#ifndef __AGCORE_BLE_GAP_H__
#define __AGCORE_BLE_GAP_H__

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include <stdbool.h>
#include <stdint.h>

#include "host/ble_gap.h"

extern volatile uint16_t g_agcore_ble_gap_mtu;
extern uint16_t g_agcore_ble_conn_handle;

void agcore_ble_gap_init(void);
int agcore_ble_gap_event_cb(struct ble_gap_event *event, void *arg);
bool agcore_ble_is_connected(void);
uint16_t agcore_ble_get_mtu(void);

#endif

#endif /* __AGCORE_BLE_GAP_H__ */
