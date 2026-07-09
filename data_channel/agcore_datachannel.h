#ifndef __AGCORE_DATACHANNEL_H__
#define __AGCORE_DATACHANNEL_H__

#include "stdint.h"
#include "stdbool.h"

#ifndef AGCORE_DATA_CHANNEL_PAYLOAD_MAX
#define AGCORE_DATA_CHANNEL_PAYLOAD_MAX      256
#endif

typedef enum {
    AGCORE_DATA_LINK_NONE = 0,
    AGCORE_DATA_LINK_WIFI,
    AGCORE_DATA_LINK_BLE,
    AGCORE_DATA_LINK_UART,
} agcore_data_link_et;

typedef struct {
    agcore_data_link_et link;
    uint16_t cmd;
    uint16_t payload_len;
    uint8_t payload[AGCORE_DATA_CHANNEL_PAYLOAD_MAX];
} agcore_data_st;

typedef void (*agcore_data_cb)(const agcore_data_st *data);

void agcore_data_queue_init(void);
void agcore_data_push(agcore_data_link_et link,  uint16_t cmd, const uint8_t *payload, uint16_t payload_len);
void agcore_data_handler_register(agcore_data_cb cb);
void agcore_data_route_handler(const agcore_data_st *data);

#endif /* __AGCORE_DATACHANNEL_H__ */
