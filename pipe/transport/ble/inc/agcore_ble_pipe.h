#ifndef __AGCORE_BLE_PIPE_H__
#define __AGCORE_BLE_PIPE_H__

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include <stdint.h>

#include "esp_err.h"

esp_err_t agcore_ble_notify(const uint8_t *data, uint16_t len);
void agcore_ble_rx_data_handle(const uint8_t *data, uint16_t len);

#endif

#endif /* __AGCORE_BLE_PIPE_H__ */
