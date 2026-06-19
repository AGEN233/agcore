#pragma once

#include "agcore_ble.h"
#include "os/os_cputime.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "agcore_printf_log.h"

#ifdef CONFIG_AGCORE_BLE_ENABLE

// raw data
extern agcore_ble_data_cb_t g_agcore_ble_data_cb;
esp_err_t agcore_ble_notify(const uint8_t *data, uint16_t len);

// gatt
extern uint16_t g_agcore_ble_notify_handle;
ble_uuid16_t *agcore_ble_gatt_get_uuid(uint8_t *count);
void agcore_ble_gatt_init(void);

// adv
void agcore_ble_adv_init(void);

// gap
extern volatile uint16_t g_agcore_ble_gap_mtu;
extern uint16_t g_agcore_ble_conn_handle;
int agcore_ble_gap_event_cb(struct ble_gap_event *event, void *arg);
void agcore_ble_gap_init(void);
#endif
