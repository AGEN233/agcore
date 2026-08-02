#ifndef __AGCORE_DATACHANNEL_H__
#define __AGCORE_DATACHANNEL_H__

#include "stdint.h"
#include "stdbool.h"
#include "esp_err.h"

#ifndef AGCORE_DATA_CHANNEL_PAYLOAD_MAX
#define AGCORE_DATA_CHANNEL_PAYLOAD_MAX      256
#endif

typedef enum {
    AGCORE_DATA_LINK_NONE = 0,
    AGCORE_DATA_LINK_WIFI,
    AGCORE_DATA_LINK_BLE,
    AGCORE_DATA_LINK_UART,
    AGCORE_DATA_LINK_MAX,
} agcore_data_link_et;

typedef struct {
    agcore_data_link_et link;
    uint8_t sn;
    uint16_t cmd;
    uint16_t payload_len;
    uint8_t payload[AGCORE_DATA_CHANNEL_PAYLOAD_MAX];
} agcore_data_st;

typedef void (*agcore_data_cb)(const agcore_data_st *data);

/* 链路发送回调:接收已封好的顶层协议帧,由各链路实现 */
typedef esp_err_t (*agcore_data_send_cb)(const uint8_t *buf, uint16_t len);

void agcore_data_queue_init(void);
void agcore_data_push(const agcore_data_st *data);
void agcore_data_link_reset(agcore_data_link_et link);
void agcore_data_handler_register(agcore_data_cb cb);
void agcore_data_route_handler(const agcore_data_st *data);

/* 统一发送:注册链路发送回调 + 入发送队列(队列内统一封包后按链路分发) */
void agcore_data_send_register(agcore_data_link_et link, agcore_data_send_cb cb);
void agcore_data_send(const agcore_data_st *data);

/**
 * @brief CORE 层数据分发
 * @note  由 agcore_datevent_handle.c 实现;返回 true 表示已消费(截留),
 *        false 表示应继续推送到应用层
 */
bool agcore_data_handle_core(const agcore_data_st *data);

#endif /* __AGCORE_DATACHANNEL_H__ */
