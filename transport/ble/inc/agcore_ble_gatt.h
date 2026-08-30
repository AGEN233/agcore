#ifndef __AGCORE_BLE_GATT_H__
#define __AGCORE_BLE_GATT_H__

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include <stdint.h>

#include "host/ble_gatt.h"
#include "host/ble_uuid.h"

extern uint16_t g_agcore_ble_notify_handle;

void agcore_ble_gatt_init(void);
ble_uuid16_t *agcore_ble_gatt_get_uuid(uint8_t *count);

#endif

#endif /* __AGCORE_BLE_GATT_H__ */
