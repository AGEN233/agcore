#include "agcore_data_protocol.h"
#include "agcore_data_route.h"
#include "initcall.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "agcore_port.h"

#define TAG "AGCORE_DATA"

#ifndef AGCORE_DATA_QUEUE_LENGTH
#define AGCORE_DATA_QUEUE_LENGTH           8
#endif

typedef struct {
    agcore_link link;
    uint16_t len;
    uint8_t *buf;
} agcore_rawdata_t;

/* 接收队列 */
static QueueHandle_t g_agcore_data_queue;
static TaskHandle_t g_agcore_data_task;

/* 每条链路独立串行发送，避免并发发送使帧序号与提交顺序不一致。 */
typedef struct {
    SemaphoreHandle_t send_lock;
    agcore_data_send_cb send_cb;
    agcore_data_protocol_state_t protocol;
} agcore_data_link_context_t;

static agcore_data_link_context_t g_agcore_data_link[LINK_COUNT];

/**
 * @brief 获取链路协议状态，避免在各模块重复维护链路数组
 * @param link 数据链路
 * @return 状态指针，无效链路返回 NULL
 */
agcore_data_protocol_state_t *agcore_data_protocol_state_get(agcore_link link)
{
    if (link <= LINK_NONE || link >= LINK_COUNT) {
        return NULL;
    }
    return &g_agcore_data_link[link].protocol;
}

/**
 * @brief 统一接收队列任务
 * @param arg
 */
static void agcore_data_task(void *arg)
{
    agcore_rawdata_t raw;

    while (1) {
        if (xQueueReceive(g_agcore_data_queue, &raw, portMAX_DELAY) == pdTRUE) {
            agcore_data_t data = {.link = raw.link};

            if (agcore_rawdata_decode(raw.buf, raw.len, &data) != 0) {
                agcore_data_route_handler(&data);
            }
            agcore_free(raw.buf);
        }
    }
}

/**
 * @brief 将完整堆消息推送到接收队列，成功后由接收任务释放
 * @param link 数据来源链路
 * @param raw 完整消息，失败时所有权仍属于调用方
 * @param raw_len 完整消息长度
 * @return ESP_OK 成功，其他值表示未接管缓冲区
 */
esp_err_t agcore_data_push(agcore_link link, uint8_t *raw, uint16_t raw_len)
{
    if (raw == NULL || raw_len == 0 || link <= LINK_NONE || link >= LINK_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }
    if (g_agcore_data_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    agcore_rawdata_t item = {.link = link, .len = raw_len, .buf = raw};
    if (xQueueSend(g_agcore_data_queue, &item, 0) != pdTRUE) {
        CORE_LOGE(TAG, "queue full|link=%d len=%u", link, raw_len);
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

/**
 * @brief 注册链路发送回调
 * @param link 链路
 * @param cb   发送回调
 */
void agcore_data_send_register(agcore_link link, agcore_data_send_cb cb)
{
    if (link <= LINK_NONE || link >= LINK_COUNT) {
        CORE_LOGE(TAG, "register invalid link|link=%d", link);
        return;
    }
    g_agcore_data_link[link].send_cb = cb;
}

/**
 * @brief 在当前任务中封包并提交到目标链路
 * @param data 待发送数据，payload 仅在函数调用期间借用
 * @return ESP_OK 表示链路接受数据，否则返回错误码
 */
esp_err_t agcore_data_send(const agcore_data_t *data)
{
    if (data == NULL || (data->payload_len > 0 && data->payload == NULL)) {
        CORE_LOGD(TAG, "send invalid");
        return ESP_ERR_INVALID_ARG;
    }

    if (data->link <= LINK_NONE || data->link >= LINK_COUNT || data->payload_len > AGCORE_RAWDATA_PAYLOAD_MAX_LEN) {
        CORE_LOGD(TAG, "send invalid|link=%d len=%u", data->link, data->payload_len);
        return ESP_ERR_INVALID_ARG;
    }

    SemaphoreHandle_t lock = g_agcore_data_link[data->link].send_lock;
    if (lock == NULL) {
        CORE_LOGD(TAG, "send not ready|link=%d", data->link);
        return ESP_ERR_INVALID_STATE;
    }

    uint16_t frame_len = data->payload_len + AGCORE_RAWDATA_OVERHEAD;
    uint8_t *buf = agcore_malloc(frame_len);
    if (buf == NULL) {
        CORE_LOGE(TAG, "send buffer malloc failed|link=%d", data->link);
        return ESP_ERR_NO_MEM;
    }

    xSemaphoreTake(lock, portMAX_DELAY);
    agcore_data_send_cb cb = g_agcore_data_link[data->link].send_cb;
    esp_err_t ret;
    if (cb) {
        uint16_t len = agcore_rawdata_encode(data, buf);
        ret = ESP_ERR_INVALID_STATE;
        ret = len == 0 ? ESP_FAIL : cb(buf, len);
    } else {
        ret = ESP_ERR_INVALID_STATE;
        CORE_LOGD(TAG, "no send handler|link=%d", data->link);
    }

    xSemaphoreGive(lock);
    agcore_free(buf);
    return ret;
}

/**
 * @brief 初始化接收队列和各链路发送锁
 * @return ESP_OK 成功，否则返回错误码
 */
int agcore_data_queue_init(void)
{
    for (agcore_link link = LINK_NONE + 1; link < LINK_COUNT; link++) {
        if (g_agcore_data_link[link].send_lock == NULL) {
            g_agcore_data_link[link].send_lock = xSemaphoreCreateMutex();
            if (g_agcore_data_link[link].send_lock == NULL) {
                CORE_LOGE(TAG, "send lock create failed|link=%d", link);
                return ESP_ERR_NO_MEM;
            }
        }
    }

    if (g_agcore_data_queue == NULL) {
        g_agcore_data_queue = xQueueCreate(AGCORE_DATA_QUEUE_LENGTH, sizeof(agcore_rawdata_t));
        if (g_agcore_data_queue == NULL) {
            CORE_LOGE(TAG, "data queue create failed");
            return ESP_ERR_NO_MEM;
        }
    }

    if (g_agcore_data_task == NULL) {
        if (xTaskCreate(agcore_data_task, "AGCORE_DATA_QUEUE_TASK", (4 * 1024), NULL, 5,
                        &g_agcore_data_task) != pdPASS) {
            g_agcore_data_task = NULL;
            CORE_LOGE(TAG, "data queue task create failed");
            return ESP_ERR_NO_MEM;
        }
    }

    return ESP_OK;
}

AGCORE_SERVICE_INITCALL(agcore_data_queue_init);
