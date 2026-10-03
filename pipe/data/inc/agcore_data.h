#ifndef __AGCORE_DATA_H__
#define __AGCORE_DATA_H__

#include "agcore_port.h"
#include "esp_err.h"

typedef enum {
    LINK_NONE = 0,
    LINK_BLE,
    LINK_UART,
    LINK_COUNT,
} agcore_link;

typedef struct {
    agcore_link link;
    uint8_t sn;
    uint16_t payload_len;
    uint8_t *payload;
} agcore_data_t;

typedef esp_err_t (*agcore_data_send_cb)(const uint8_t *buf, uint16_t len);

int agcore_data_queue_init(void);
/**
 * @brief 将完整堆消息入队；成功转移所有权，失败仍由调用方释放
 * @param link 来源链路
 * @param raw 可由 agcore_free 释放的完整消息
 * @param raw_len 完整消息长度
 * @return ESP_OK 成功，否则错误码
 */
esp_err_t agcore_data_push(agcore_link link, uint8_t *raw, uint16_t raw_len);
void agcore_data_link_reset(agcore_link link);
void agcore_data_send_register(agcore_link link, agcore_data_send_cb cb);
esp_err_t agcore_data_send(const agcore_data_t *data);

#endif /* __AGCORE_DATA_H__ */
