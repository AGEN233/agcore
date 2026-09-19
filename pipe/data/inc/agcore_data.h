#ifndef __AGCORE_DATA_H__
#define __AGCORE_DATA_H__

#include "stdint.h"
#include "stdbool.h"
#include "esp_err.h"

#ifndef AGCORE_DATA_PAYLOAD_MAX
#define AGCORE_DATA_PAYLOAD_MAX      256
#endif

typedef enum {
    LINK_NONE = 0,
    LINK_WIFI,
    LINK_BLE,
    LINK_UART,
    LINK_COUNT,
} agcore_link;

typedef struct {
    agcore_link link;
    uint8_t sn;
    uint16_t cmd;
    uint16_t payload_len;
    uint8_t payload[AGCORE_DATA_PAYLOAD_MAX];
} agcore_data_t;

typedef void (*agcore_data_cb)(const agcore_data_t *data);

/* 链路发送回调:接收已封好的顶层协议帧,由各链路实现 */
typedef esp_err_t (*agcore_data_send_cb)(const uint8_t *buf, uint16_t len);

int agcore_data_queue_init(void);
void agcore_data_push(const agcore_data_t *data);
void agcore_data_link_reset(agcore_link link);
void agcore_data_handler_register(agcore_data_cb cb);
void agcore_data_route_handler(const agcore_data_t *data);

/* 统一发送:注册链路发送回调 + 入发送队列(队列内统一封包后按链路分发) */
void agcore_data_send_register(agcore_link link, agcore_data_send_cb cb);
void agcore_data_send(const agcore_data_t *data);

/**
 * @brief CORE 层数据分发
 * @note  由 agcore_core_cmd.c 实现;返回 true 表示已消费(截留),
 *        false 表示应继续推送到应用层
 */
bool agcore_data_handle_core(const agcore_data_t *data);

#endif /* __AGCORE_DATA_H__ */
