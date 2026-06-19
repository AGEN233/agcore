#include "agcore.h"

#include "sdkconfig.h"
#ifdef CONFIG_AGCORE_BLE_ENABLE
#include "agcore_ble.h"
#endif

void agcore_init(void)
{
    // core chanel init


    // data chanel_init
    agcore_data_queue_init();

    agcore_ble_init();
}

/**
 * @brief 设置设备信息，并拉起core流程
 * @param info
 */
void agcore_set_device_info(const agcore_device_info_st *info)
{
    if (info == NULL) {
        CORE_LOGD("AGCORE", "device info invalid");
        return;
    }

    agcore_device_info_set(info);

#ifdef CONFIG_AGCORE_BLE_ENABLE
    if (!agcore_ble_is_connected()) {
        agcore_ble_adv_stop();
        agcore_ble_adv_update();
        agcore_ble_adv_start();
    }
#endif
}

const agcore_device_info_st *agcore_get_device_info(void)
{
    return agcore_device_info_get();
}

const agcore_version_info_st *agcore_get_agcore_info(void)
{
    return agcore_version_info_get();
}

uint16_t agcore_get_device_type(void)
{
    return agcore_device_info_get_type();
}

uint16_t agcore_get_device_id(void)
{
    return agcore_device_info_get_id();
}

uint16_t agcore_get_fw_version(void)
{
    return agcore_device_info_get_fw_version();
}

uint16_t agcore_get_hw_version(void)
{
    return agcore_device_info_get_hw_version();
}

uint16_t agcore_get_agcore_version(void)
{
    return agcore_version_info_get_version();
}

uint16_t agcore_get_agcore_id(void)
{
    return agcore_version_info_get_id();
}

int agcore_get_device_identifier(char *buf, size_t buf_len)
{
    return agcore_device_info_get_string(buf, buf_len);
}

int agcore_get_agcore_identifier(char *buf, size_t buf_len)
{
    return agcore_version_info_get_string(buf, buf_len);
}
