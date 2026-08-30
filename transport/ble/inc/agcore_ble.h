#ifndef __AGCORE_BLE_H__
#define __AGCORE_BLE_H__
#include <stdbool.h>
#include "esp_err.h"
#include "agcore_data.h"

// link state
bool agcore_ble_is_ready(void);
bool agcore_ble_is_connected(void);
uint16_t agcore_ble_get_mtu(void);

// local address
uint8_t agcore_ble_get_addr_type(void);
void agcore_ble_get_addr(uint8_t *addr);

esp_err_t agcore_ble_init(void);
#endif
