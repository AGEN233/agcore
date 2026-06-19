#include "sdkconfig.h"

#ifdef CONFIG_AGCORE_BLE_ENABLE
#include "agcore_ble.h"
#include "agcore_ble_internal.h"
#include "services/gatt/ble_svc_gatt.h"
#define TAG "BLE_GATT"

// CONTROL -> C -> 0x43
static const ble_uuid16_t s_agcore_ble_svc_control          = BLE_UUID16_INIT(0x4300);
static const ble_uuid16_t s_agcore_ble_chr_control_write    = BLE_UUID16_INIT(0x4301);
static const ble_uuid16_t s_agcore_ble_chr_control_notify   = BLE_UUID16_INIT(0x4302);
uint16_t g_agcore_ble_notify_handle;

/**
 * @brief 获取GATT的UUID
 * @param uuid_list 储存返回的uuid
 * @param count 储存返回的uuid数量
 */
ble_uuid16_t *agcore_ble_gatt_get_uuid(uint8_t *count)
{
    static const ble_uuid16_t uuid_list[] = {
        s_agcore_ble_svc_control,
    };

    if (count) {
        *count = sizeof(uuid_list) / sizeof(uuid_list[0]);
    }

    return (ble_uuid16_t *)uuid_list;
}

/**
 * @brief 空回调
 */
static int agcore_ble_gatt_dummy_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    return 0;
}

/**
 * @brief GATT数据接收回调
 */
static int agcore_ble_gatt_write_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint8_t *data = ctxt->om->om_data;
    uint16_t len = ctxt->om->om_len;

    if (g_agcore_ble_data_cb) {
        g_agcore_ble_data_cb(data, len);
    }

    return 0;
}

/**
 * @brief GATT服务表
 */
static const struct ble_gatt_svc_def s_agcore_ble_gatt_svcs[] = {
    /*** Service: CONTROL. */
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &s_agcore_ble_svc_control.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                .uuid = &s_agcore_ble_chr_control_write.u,
                .access_cb = agcore_ble_gatt_write_cb,
                .flags = BLE_GATT_CHR_F_WRITE,
            }, {
                .uuid = &s_agcore_ble_chr_control_notify.u,
                .access_cb = agcore_ble_gatt_dummy_cb,
                .val_handle = &g_agcore_ble_notify_handle,
                .flags = BLE_GATT_CHR_F_NOTIFY,
            },
            {0}
        }
    },
    {0}
};

/**
 * @brief GATT初始化
 */
void agcore_ble_gatt_init(void)
{
    int ret;
    ble_svc_gatt_init();

    ret = ble_gatts_count_cfg(s_agcore_ble_gatt_svcs);
    if (ret != 0) {
        CORE_LOGE(TAG, "count service config failed|%d", ret);
        return;
    }

    ret = ble_gatts_add_svcs(s_agcore_ble_gatt_svcs);
    if (ret != 0) {
        CORE_LOGE(TAG, "add service failed|%d", ret);
        return;
    }

    ret = ble_gatts_start();
    if (ret != 0) {
        CORE_LOGE(TAG, "start failed|%d", ret);
        return;
    }
}
#endif
