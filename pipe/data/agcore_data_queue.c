#include "agcore_data_protocol.h"
#include "initcall.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "agcore_port.h"
#include <string.h>

#define TAG "AGCORE_DATA_QUEUE"

#ifndef AGCORE_DATA_QUEUE_LENGTH
#define AGCORE_DATA_QUEUE_LENGTH           8
#endif

/* 帧头和校验共 9 字节。 */
#define AGCORE_DATA_RAW_MAX_LEN            (AGCORE_DATA_PAYLOAD_MAX + 9)

typedef struct {
    agcore_link link;
    uint16_t len;
    uint8_t *buf;
} agcore_rawdata_t;

/* 接收队列 */
static QueueHandle_t g_agcore_data_queue;
static TaskHandle_t g_agcore_data_task;

/* 发送队列 */
static QueueHandle_t g_agcore_send_queue;
static TaskHandle_t g_agcore_send_task;
static agcore_data_send_cb g_agcore_send_cb[LINK_COUNT];

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
 * @brief 统一发送队列任务
 */
static void agcore_data_send_task(void *arg)
{
    agcore_data_t data = {0};
    /* 封包缓冲放堆上,避免占用任务栈 */
    uint8_t *buf = (uint8_t *)agcore_malloc(AGCORE_DATA_RAW_MAX_LEN);

    if (buf == NULL) {
        CORE_LOGE(TAG, "send task malloc failed");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        if (xQueueReceive(g_agcore_send_queue, &data, portMAX_DELAY) == pdTRUE) {
            if (data.link <= LINK_NONE || data.link >= LINK_COUNT) {
                CORE_LOGE(TAG, "send invalid link|link=%d", data.link);
            } else if (g_agcore_send_cb[data.link] == NULL) {
                CORE_LOGE(TAG, "no send handler|link=%d", data.link);
            } else {
                /* 统一封包 */
                uint16_t len = agcore_data_encode(&data, buf);
                if (len == 0) {
                    CORE_LOGE(TAG, "encode failed|link=%d", data.link);
                } else {
                    /* 按目标链路分发已封好的帧 */
                    g_agcore_send_cb[data.link](buf, len);
                }
            }
            agcore_free(data.payload);
        }
    }
    agcore_free(buf);
}

/**
 * @brief 统一收发队列初始化
 */
int agcore_data_queue_init(void)
{
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

    if (g_agcore_send_queue == NULL) {
        g_agcore_send_queue = xQueueCreate(AGCORE_DATA_QUEUE_LENGTH, sizeof(agcore_data_t));
        if (g_agcore_send_queue == NULL) {
            CORE_LOGE(TAG, "send queue create failed");
            return ESP_ERR_NO_MEM;
        }
    }

    if (g_agcore_send_task == NULL) {
        if (xTaskCreate(agcore_data_send_task, "AGCORE_DATA_SEND_TASK", (4 * 1024), NULL, 5,
                        &g_agcore_send_task) != pdPASS) {
            g_agcore_send_task = NULL;
            CORE_LOGE(TAG, "send queue task create failed");
            return ESP_ERR_NO_MEM;
        }
    }

    return ESP_OK;
}

AGCORE_SERVICE_INITCALL(agcore_data_queue_init);

/**
 * @brief 复制原始帧并推送到统一接收队列
 * @param link 数据来源链路
 * @param raw 原始帧，调用结束后可由调用方释放
 * @param raw_len 原始帧长度
 */
void agcore_data_push(agcore_link link, const uint8_t *raw, uint16_t raw_len)
{
    if (raw == NULL || raw_len == 0 || link <= LINK_NONE || link >= LINK_COUNT) {
        CORE_LOGD(TAG, "raw data invalid|link=%d len=%u", link, raw_len);
        return;
    }

    if (raw_len > AGCORE_DATA_RAW_MAX_LEN) {
        CORE_LOGD(TAG, "raw data too long|link=%d len=%u", link, raw_len);
        return;
    }

    if (g_agcore_data_queue == NULL) {
        CORE_LOGD(TAG, "queue not ready|link=%d len=%u", link, raw_len);
        return;
    }

    agcore_rawdata_t item = {
        .link = link,
        .len = raw_len,
        .buf = agcore_malloc(raw_len),
    };
    if (item.buf == NULL) {
        CORE_LOGE(TAG, "raw data malloc failed|link=%d len=%u", link, raw_len);
        return;
    }
    memcpy(item.buf, raw, raw_len);

    if (xQueueSend(g_agcore_data_queue, &item, 0) != pdTRUE) {
        agcore_free(item.buf);
        CORE_LOGE(TAG, "queue full|link=%d len=%u", link, raw_len);
        return;
    }
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
    g_agcore_send_cb[link] = cb;
}

/**
 * @brief 复制数据后入发送队列
 * @param data 待发送数据
 */
void agcore_data_send(const agcore_data_t *data)
{
    if (data == NULL || (data->payload_len > 0 && data->payload == NULL)) {
        CORE_LOGD(TAG, "send invalid");
        return;
    }

    if (data->payload_len > AGCORE_DATA_PAYLOAD_MAX) {
        CORE_LOGD(TAG, "send payload too long|link=%d len=%u", data->link, data->payload_len);
        return;
    }

    if (g_agcore_send_queue == NULL) {
        CORE_LOGD(TAG, "send queue not ready|link=%d", data->link);
        return;
    }

    agcore_data_t item = *data;
    if (data->payload_len > 0) {
        item.payload = agcore_malloc(data->payload_len);
        if (item.payload == NULL) {
            CORE_LOGE(TAG, "send payload malloc failed|link=%d len=%u", data->link, data->payload_len);
            return;
        }
        memcpy(item.payload, data->payload, data->payload_len);
    } else {
        item.payload = NULL;
    }

    if (xQueueSend(g_agcore_send_queue, &item, 0) != pdTRUE) {
        agcore_free(item.payload);
        CORE_LOGE(TAG, "send queue full|link=%d", data->link);
        return;
    }
}
