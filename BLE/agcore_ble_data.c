#include "sdkconfig.h"

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include "agcore_ble.h"
#include "agcore_ble_internal.h"
#include "agcore_data_encode.h"

#define TAG "BLE_DATA"

/**
 * @brief AGCORE BLE NOTIFY
 */
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

/**
 * @brief AGCORE BLE数据接收
 */
void agcore_ble_rx_data_handle(const uint8_t *data, uint16_t len)
{
    agcore_data_st item = {
        .link = AGCORE_DATA_LINK_BLE,
    };

    if (data == NULL) {
        CORE_LOGD(TAG, "rx invalid|len=%u", len);
        return;
    }

    if (agcore_data_decode(data, len, &item) == 0) {
        CORE_LOGD(TAG, "decode failed|len=%u", len);
        return;
    }

    agcore_data_push(&item);
}

#endif
