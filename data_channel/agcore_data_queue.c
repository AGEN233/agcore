#include "agcore_data_encode.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "agcore_port.h"

#define TAG "AGCORE_DATA_QUEUE"

#ifndef AGCORE_DATA_QUEUE_LENGTH
#define AGCORE_DATA_QUEUE_LENGTH           8
#endif

/* 接收队列 */
static QueueHandle_t s_agcore_data_queue;
static TaskHandle_t s_agcore_data_task;

/* 发送队列 */
static QueueHandle_t s_agcore_send_queue;
static TaskHandle_t s_agcore_send_task;
static agcore_data_send_cb s_agcore_send_cb[AGCORE_DATA_LINK_MAX];

/**
 * @brief 统一接收队列任务
 * @param arg
 */
static void agcore_data_task(void *arg)
{
    agcore_data_st data;

    while (1) {
        if (xQueueReceive(s_agcore_data_queue, &data, portMAX_DELAY) == pdTRUE) {
            agcore_data_route_handler(&data);
        }
    }
}

/**
 * @brief 统一发送队列任务
 */
static void agcore_data_send_task(void *arg)
{
    agcore_data_st data = {0};
    /* 封包缓冲放堆上,避免占用任务栈 */
    uint8_t *buf = (uint8_t *)agcore_malloc(AGCORE_DATA_CHANNEL_PAYLOAD_MAX + 16);

    if (buf == NULL) {
        CORE_LOGE(TAG, "send task malloc failed");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        if (xQueueReceive(s_agcore_send_queue, &data, portMAX_DELAY) == pdTRUE) {
            if (data.link <= AGCORE_DATA_LINK_NONE || data.link >= AGCORE_DATA_LINK_MAX) {
                CORE_LOGE(TAG, "send invalid link|link=%d", data.link);
                continue;
            }
            if (s_agcore_send_cb[data.link] == NULL) {
                CORE_LOGE(TAG, "no send handler|link=%d cmd=%u", data.link, data.cmd);
                continue;
            }

            /* 统一封包 */
            uint16_t len = agcore_data_encode(&data, buf);
            if (len == 0) {
                CORE_LOGE(TAG, "encode failed|link=%d cmd=%u", data.link, data.cmd);
                continue;
            }

            /* 按目标链路分发已封好的帧 */
            s_agcore_send_cb[data.link](buf, len);
        }
    }
    agcore_free(buf);
}

/**
 * @brief 统一收发队列初始化
 */
void agcore_data_queue_init(void)
{
    if (s_agcore_data_queue == NULL) {
        s_agcore_data_queue = xQueueCreate(AGCORE_DATA_QUEUE_LENGTH, sizeof(agcore_data_st));
        if (s_agcore_data_queue == NULL) {
            CORE_LOGE(TAG, "data queue create failed");
            return;
        }
    }

    if (s_agcore_data_task == NULL) {
        xTaskCreate(agcore_data_task, "AGCORE_DATA_QUEUE_TASK", (4 * 1024), NULL, 5, &s_agcore_data_task);
        if (s_agcore_data_task == NULL) {
            CORE_LOGE(TAG, "data queue task create failed");
            return;
        }
    }

    if (s_agcore_send_queue == NULL) {
        s_agcore_send_queue = xQueueCreate(AGCORE_DATA_QUEUE_LENGTH, sizeof(agcore_data_st));
        if (s_agcore_send_queue == NULL) {
            CORE_LOGE(TAG, "send queue create failed");
            return;
        }
    }

    if (s_agcore_send_task == NULL) {
        xTaskCreate(agcore_data_send_task, "AGCORE_DATA_SEND_TASK", (4 * 1024), NULL, 5, &s_agcore_send_task);
        if (s_agcore_send_task == NULL) {
            CORE_LOGE(TAG, "send queue task create failed");
            return;
        }
    }
}

/**
 * @brief 推送数据到统一接收队列
 * @param data 统一数据结构.
 */
void agcore_data_push(const agcore_data_st *data)
{
    if (data == NULL) {
        CORE_LOGD(TAG, "data invalid");
        return;
    }

    if (data->payload_len > AGCORE_DATA_CHANNEL_PAYLOAD_MAX) {
        CORE_LOGD(TAG, "payload too long|link=%d cmd=%u len=%u", data->link, data->cmd, data->payload_len);
        return;
    }

    if (s_agcore_data_queue == NULL) {
        CORE_LOGD(TAG, "queue not ready|link=%d cmd=%u len=%u", data->link, data->cmd, data->payload_len);
        return;
    }

    if (xQueueSend(s_agcore_data_queue, data, 0) != pdTRUE) {
        CORE_LOGE(TAG, "queue full|link=%d cmd=%u len=%u", data->link, data->cmd, data->payload_len);
        return;
    }
}

/**
 * @brief 注册链路发送回调
 * @param link 链路
 * @param cb   发送回调
 */
void agcore_data_send_register(agcore_data_link_et link, agcore_data_send_cb cb)
{
    if (link <= AGCORE_DATA_LINK_NONE || link >= AGCORE_DATA_LINK_MAX) {
        CORE_LOGE(TAG, "register invalid link|link=%d", link);
        return;
    }
    s_agcore_send_cb[link] = cb;
}

/**
 * @brief agcore统一数据发送接口
 */
void agcore_data_send(const agcore_data_st *data)
{
    if (data == NULL) {
        CORE_LOGD(TAG, "send invalid");
        return;
    }

    if (data->payload_len > AGCORE_DATA_CHANNEL_PAYLOAD_MAX) {
        CORE_LOGD(TAG, "send payload too long|link=%d cmd=%u len=%u", data->link, data->cmd, data->payload_len);
        return;
    }

    if (s_agcore_send_queue == NULL) {
        CORE_LOGD(TAG, "send queue not ready|link=%d cmd=%u", data->link, data->cmd);
        return;
    }

    if (xQueueSend(s_agcore_send_queue, data, 0) != pdTRUE) {
        CORE_LOGE(TAG, "send queue full|link=%d cmd=%u", data->link, data->cmd);
        return;
    }
}
