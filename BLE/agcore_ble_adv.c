#include "sdkconfig.h"

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include <stdio.h>
#include <string.h>

#include "agcore.h"
#include "agcore_ble.h"
#include "agcore_ble_internal.h"
#include "esp_bt.h"
#include "host/ble_hs.h"

#define TAG "BLE_ADV"

#define AGCORE_BLE_ADV_DEVICE_NAME      "AGOS"
#define AGCORE_BLE_ADV_DEVICE_NAME_MAX  8
#define AGCORE_BLE_ADV_DEVICE_MAC_LEN   6
#define AGCORE_BLE_ADV_DEVICE_MAX       (AGCORE_BLE_ADV_DEVICE_NAME_MAX + AGCORE_BLE_ADV_DEVICE_MAC_LEN)

static agcore_ble_adv_data_st s_agcore_ble_adv_data;
static const struct ble_gap_adv_params s_agcore_ble_adv_params = {
    .conn_mode = BLE_GAP_CONN_MODE_UND,
    .disc_mode = BLE_GAP_DISC_MODE_GEN,
    .itvl_min = (uint16_t)(CONFIG_AGCORE_BLE_ADV_ITVL_MIN_MS * 16 / 10),
    .itvl_max = (uint16_t)(CONFIG_AGCORE_BLE_ADV_ITVL_MAX_MS * 16 / 10),
    .channel_map = 0,
    .filter_policy = 0,
    .high_duty_cycle = 0,
};

/**
 * @brief 填充蓝牙广播设备字段；默认 00 FF 交替，已设置设备信息时覆盖为真实值。
 */
static void agcore_ble_adv_fill_device_info(uint8_t *buf)
{
    for (uint16_t i = 0; i < 8; i++) {
        buf[i] = (i % 2 == 0) ? 0x00 : 0xFF;
    }

    if (!agcore_device_info_is_set()) {
        return;
    }

    const agcore_device_info_st *info = agcore_get_device_info();
    buf[0] = (uint8_t)(info->device_type >> 8);
    buf[1] = (uint8_t)(info->device_type);
    buf[2] = (uint8_t)(info->device_id >> 8);
    buf[3] = (uint8_t)(info->device_id);
    buf[4] = (uint8_t)(info->fw_version >> 8);
    buf[5] = (uint8_t)(info->fw_version);
    buf[6] = (uint8_t)(info->hw_version >> 8);
    buf[7] = (uint8_t)(info->hw_version);
}

/**
 * @brief 将 MAC 地址转换为 ASCII HEX 字符串。
 */
static void mac_to_ascii_suffix(uint8_t mac[6], char buf[13])
{
    snprintf(buf, 13, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**
 * @brief 获取广播发射功率。
 * @return dBm 功率值
 */
static int8_t agcore_ble_get_tx_power_dbm(void)
{
    esp_power_level_t level = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_ADV);

    switch (level) {
        case ESP_PWR_LVL_N24:
            return -24;
        case ESP_PWR_LVL_N21:
            return -21;
        case ESP_PWR_LVL_N18:
            return -18;
        case ESP_PWR_LVL_N15:
            return -15;
        case ESP_PWR_LVL_N12:
            return -12;
        case ESP_PWR_LVL_N9:
            return -9;
        case ESP_PWR_LVL_N6:
            return -6;
        case ESP_PWR_LVL_N3:
            return -3;
        case ESP_PWR_LVL_N0:
            return 0;
        case ESP_PWR_LVL_P3:
            return 3;
        case ESP_PWR_LVL_P6:
            return 6;
        case ESP_PWR_LVL_P9:
            return 9;
        case ESP_PWR_LVL_P12:
            return 12;
        case ESP_PWR_LVL_P15:
            return 15;
        case ESP_PWR_LVL_P18:
            return 18;
        case ESP_PWR_LVL_P20:
            return 20;
        default:
            CORE_LOGD(TAG, "invalid tx power level|%d", level);
            return 0;
    }
}

/**
 * @brief 启动蓝牙广播。
 */
void agcore_ble_adv_start(void)
{
    int ret = ble_gap_adv_start(agcore_ble_get_addr_type(), NULL, BLE_HS_FOREVER, &s_agcore_ble_adv_params, agcore_ble_gap_event_cb, NULL);
    if (ret) {
        CORE_LOGE(TAG, "start failed|%d", ret);
    } else {
        CORE_LOGD(TAG, "started");
    }
}

/**
 * @brief 停止蓝牙广播。
 */
void agcore_ble_adv_stop(void)
{
    int rc = ble_gap_adv_stop();
    if (rc) {
        CORE_LOGW(TAG, "stop failed|%d", rc);
    }
}

/**
 * @brief 更新蓝牙广播内容。
 */
void agcore_ble_adv_update(void)
{
    struct ble_hs_adv_fields adv_fields;
    struct ble_hs_adv_fields rsp_fields;

    memset(&adv_fields, 0, sizeof(adv_fields));
    memset(&rsp_fields, 0, sizeof(rsp_fields));

    /*
     * Advertising Data
     * --------------------------------------------------
     * Flags
     * TX Power
     * Complete Local Name
     *
     * AGIOT + MAC
     * Example:
     * AGIOTAABBCCDDEEFF
     */
    uint8_t mac[6];
    char name[18];

    agcore_ble_get_addr(mac);

    memcpy(name, "AGIOT", 5);
    mac_to_ascii_suffix(mac, &name[5]);
    name[17] = '\0';

    adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    adv_fields.tx_pwr_lvl = agcore_ble_get_tx_power_dbm();
    adv_fields.tx_pwr_lvl_is_present = 1;

    adv_fields.name = (const uint8_t *)name;
    adv_fields.name_len = 17;
    adv_fields.name_is_complete = 1;

    int ret = ble_gap_adv_set_fields(&adv_fields);
    if (ret) {
        CORE_LOGE(TAG, "set adv fields failed|%d", ret);
        return;
    }

    /*
     * Scan Response
     * --------------------------------------------------
     * UUID16 List
     * Manufacturer Data:
     * AGCORE + device_type + device_id + fw_version + hw_version
     */
    uint8_t mfg_data[14];

    memcpy(mfg_data, "AGCORE", 6);
    agcore_ble_adv_fill_device_info(&mfg_data[6]);

    rsp_fields.mfg_data = mfg_data;
    rsp_fields.mfg_data_len = sizeof(mfg_data);

    uint8_t uuid_num = 0;
    const ble_uuid16_t *uuid_list = agcore_ble_gatt_get_uuid(&uuid_num);

    if (uuid_list && uuid_num > 0) {
        rsp_fields.uuids16 = (ble_uuid16_t *)uuid_list;
        rsp_fields.num_uuids16 = uuid_num;
        rsp_fields.uuids16_is_complete = 1;
    }

    ret = ble_gap_adv_rsp_set_fields(&rsp_fields);
    if (ret) {
        CORE_LOGE(TAG, "set scan response failed|%d", ret);
        return;
    }

    CORE_LOGI(TAG, "BLE ADV UPDATE");
    CORE_LOGD(TAG, "ADV DATA|name:%s tx:%ddBm device:%02X%02X%02X%02X%02X%02X%02X%02X",
              name,
              agcore_ble_get_tx_power_dbm(),
              mfg_data[6],
              mfg_data[7],
              mfg_data[8],
              mfg_data[9],
              mfg_data[10],
              mfg_data[11],
              mfg_data[12],
              mfg_data[13]);
}

/**
 * @brief 初始化蓝牙广播数据。
 */
void agcore_ble_adv_init(void)
{
    memset(&s_agcore_ble_adv_data, 0, sizeof(s_agcore_ble_adv_data));
}

#endif
