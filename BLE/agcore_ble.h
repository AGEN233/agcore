#ifndef __AGCORE_BLE_H__
#define __AGCORE_BLE_H__
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "agcore_datachannel.h"

/**
 * @brief 广播数据
 */
typedef struct {
    uint8_t  device_type;
    uint16_t fw_version;

    bool     dual_ic;
    uint8_t  ic2_device_type;
    uint16_t  ic2_fw_version;
} agcore_ble_adv_data_st;

// data
void agcore_ble_rx_data_handle(const uint8_t *data, uint16_t len);

// advertising
void agcore_ble_adv_start(void);
void agcore_ble_adv_stop(void);
void agcore_ble_adv_update(void);

// link state
bool agcore_ble_is_ready(void);
bool agcore_ble_is_connected(void);
uint16_t agcore_ble_get_mtu(void);

// local address
uint8_t agcore_ble_get_addr_type(void);
void agcore_ble_get_addr(uint8_t *addr);

esp_err_t agcore_ble_init(void);
#endif
