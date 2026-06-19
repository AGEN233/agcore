#include "sdkconfig.h"

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include "agcore_ble.h"
#include "agcore_ble_internal.h"

#define TAG "BLE_DATA"

agcore_ble_data_cb_t g_agcore_ble_data_cb = NULL;

esp_err_t agcore_ble_notify(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (g_agcore_ble_conn_handle == BLE_HS_CONN_HANDLE_NONE) {
        return ESP_ERR_INVALID_STATE;
    }

    uint16_t payload_len_max = g_agcore_ble_gap_mtu - 3;
    uint16_t payload_sent = 0;

    while (payload_sent < len) {
        uint16_t chunk_len = len - payload_sent;
        if (chunk_len > payload_len_max) {
            chunk_len = payload_len_max;
        }

        struct os_mbuf *om = ble_hs_mbuf_from_flat(data + payload_sent, chunk_len);
        if (!om) {
            CORE_LOGE(TAG, "mbuf allocate failed");
            return ESP_ERR_NO_MEM;
        }

        int ret = ble_gattc_notify_custom(g_agcore_ble_conn_handle, g_agcore_ble_notify_handle, om);
        if (ret == 0) {
            payload_sent += chunk_len;
        } else if (ret == BLE_HS_EAGAIN) {
            os_mbuf_free(om);
            CORE_LOGD(TAG, "notify buffer full, retrying");
            vTaskDelay(pdMS_TO_TICKS(10));
        } else {
            os_mbuf_free(om);
            CORE_LOGE(TAG, "notify failed|%d", ret);
            return ESP_FAIL;
        }
    }

    return ESP_OK;
}

esp_err_t agcore_ble_send_data(const uint8_t *data, uint16_t len)
{
    return agcore_ble_notify(data, len);
}

void agcore_ble_register_data_callback(agcore_ble_data_cb_t cb)
{
    g_agcore_ble_data_cb = cb;
}

#endif
