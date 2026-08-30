#ifndef __AGCORE_BLE_ADV_H__
#define __AGCORE_BLE_ADV_H__

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 广播数据
 */
typedef struct {
    uint8_t device_type;
    uint16_t fw_version;
    bool dual_ic;
    uint8_t ic2_device_type;
    uint16_t ic2_fw_version;
} agcore_ble_adv_data_t;

void agcore_ble_adv_init(void);
void agcore_ble_adv_start(void);
void agcore_ble_adv_stop(void);
void agcore_ble_adv_update(void);

#endif

#endif /* __AGCORE_BLE_ADV_H__ */
