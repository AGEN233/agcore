#ifndef __AGCORE_DATA_H__
#define __AGCORE_DATA_H__

#include "agcore_port.h"
#include "esp_err.h"

#ifndef AGCORE_DATA_PAYLOAD_MAX
/* 顶层协议 payload 容量；不规定其中的业务格式。 */
#define AGCORE_DATA_PAYLOAD_MAX      258
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
    uint16_t payload_len;
    /* 接收回调中借用原始帧缓冲区；字节内容由使用方解释。 */
    uint8_t *payload;
} agcore_data_t;

/* 回调同步收到完整 payload，不能保留 payload 指针。 */
typedef void (*agcore_data_cb)(const agcore_data_t *data);

/* 链路发送回调:接收已封好的顶层协议帧,由各链路实现 */
typedef esp_err_t (*agcore_data_send_cb)(const uint8_t *buf, uint16_t len);

int agcore_data_queue_init(void);
/* 入队前复制 raw；调用方返回后可释放原始缓冲区。 */
void agcore_data_push(agcore_link link, const uint8_t *raw, uint16_t raw_len);
void agcore_data_link_reset(agcore_link link);
/* 在开始接收数据前注册；重复注册同一回调不会创建新节点。 */
void agcore_data_handler_register(agcore_data_cb cb);
void agcore_data_route_handler(const agcore_data_t *data);

/* 统一发送: payload 为不透明字节串，入队前复制。 */
void agcore_data_send_register(agcore_link link, agcore_data_send_cb cb);
void agcore_data_send(const agcore_data_t *data);

#endif /* __AGCORE_DATA_H__ */
