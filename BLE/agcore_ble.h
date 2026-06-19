#ifndef __AGCORE_BLE_H__
#define __AGCORE_BLE_H__
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

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

// callback
typedef void (*agcore_ble_data_cb_t)(const uint8_t *data, uint16_t len);

// raw data
esp_err_t agcore_ble_send_data(const uint8_t *data, uint16_t len);
void agcore_ble_register_data_callback(agcore_ble_data_cb_t cb);

// advertising
void agcore_ble_adv_start(void);
void agcore_ble_adv_stop(void);
void agcore_ble_adv_update(void);

// link state
bool agcore_ble_is_connected(void);
uint16_t agcore_ble_get_mtu(void);

// local address
uint8_t agcore_ble_get_addr_type(void);
void agcore_ble_get_addr(uint8_t *addr);

esp_err_t agcore_ble_init(void);
#endif
