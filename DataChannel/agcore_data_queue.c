#include "agcore_datachannel.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "agcore_printf_log.h"

#define TAG "AGCORE_DATA_QUEUE"

#ifndef AGCORE_DATA_QUEUE_LENGTH
#define AGCORE_DATA_QUEUE_LENGTH           8
#endif

static QueueHandle_t s_agcore_data_queue;
static TaskHandle_t s_agcore_data_task;

/**
 * @brief 统一数据队列推送任务
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
 * @brief 统一数据队列初始化
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
}

/**
 * @brief 推送数据到统一数据队列
 * @param link Data source type.
 * @param cmd Merged command id.
 * @param payload Raw payload buffer.
 * @param payload_len Raw payload length.
 */
void agcore_data_push(agcore_data_link_et link, uint16_t cmd, const uint8_t *payload, uint16_t payload_len)
{
    if (payload_len > AGCORE_DATA_CHANNEL_PAYLOAD_MAX) {
        CORE_LOGD(TAG, "payload too long|link=%d cmd=%u len=%u", link, cmd, payload_len);
        return;
    }

    if (payload_len > 0 && payload == NULL) {
        CORE_LOGD(TAG, "payload invalid|link=%d cmd=%u len=%u", link, cmd, payload_len);
        return;
    }

    if (s_agcore_data_queue == NULL) {
        CORE_LOGD(TAG, "queue not ready|link=%d cmd=%u len=%u", link, cmd, payload_len);
        return;
    }

    agcore_data_st data = {
        .link = link,
        .cmd = cmd,
        .payload_len = payload_len,
    };

    if (payload_len > 0) {
        memcpy(data.payload, payload, payload_len);
    }

    if (xQueueSend(s_agcore_data_queue, &data, 0) != pdTRUE) {
        CORE_LOGE(TAG, "queue full|link=%d cmd=%u len=%u", link, cmd, payload_len);
        return;
    }
}
