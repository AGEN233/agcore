#include "sdkconfig.h"

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include "agcore_ble.h"
#include "agcore_ble_gap.h"
#include "agcore_ble_gatt.h"
#include "agcore_ble_pipe.h"
#include "agcore_data_protocol.h"
#include "nimble/nimble_port.h"
#include "os/os_mbuf.h"
#include <stdatomic.h>
#include "agcore_console_log.h"

#define TAG "BLE_DATA"

/* 单次发送最多等待三次 BLE 缓冲区腾出空间。 */
#define AGCORE_BLE_NOTIFY_RETRY_MAX 3

static struct ble_npl_callout g_agcore_ble_rx_timeout;
static bool g_agcore_ble_pipe_initialized;
static atomic_uint g_agcore_ble_session;
static ble_npl_time_t g_agcore_ble_rx_last_tick;

/**
 * @brief 在 host 事件上下文释放超时消息
 * @param ev 超时事件
 */
static void fragment_timeout_cb(struct ble_npl_event *ev)
{
    (void)ev;
    agcore_data_protocol_state_t *state = agcore_data_protocol_state_get(LINK_BLE);
    if (state->fragment.buf == NULL) {
        return;
    }
    ble_npl_time_t timeout;
    ble_npl_time_ms_to_ticks(CONFIG_AGCORE_BLE_FRAGMENT_TIMEOUT_MS, &timeout);
    ble_npl_time_t elapsed = ble_npl_time_get() - g_agcore_ble_rx_last_tick;
    /* 已排队的旧超时事件可能在新片到达后执行，不能提前释放新消息。 */
    if (elapsed < timeout &&
            ble_npl_callout_reset(&g_agcore_ble_rx_timeout, timeout - elapsed) == 0) {
        return;
    }
    agcore_fragment_reset(&state->fragment);
}

/** @brief 初始化 BLE 接收超时事件。 */
void agcore_ble_pipe_init(void)
{
    if (!g_agcore_ble_pipe_initialized) {
        ble_npl_callout_init(&g_agcore_ble_rx_timeout, nimble_port_get_dflt_eventq(),
                             fragment_timeout_cb, NULL);
        g_agcore_ble_pipe_initialized = true;
    }
}

/** @brief 清理未完成接收并使旧会话的发送失效。 */
void agcore_ble_pipe_reset(void)
{
    atomic_fetch_add(&g_agcore_ble_session, 1);
    if (g_agcore_ble_pipe_initialized) {
        ble_npl_callout_stop(&g_agcore_ble_rx_timeout);
    }
    agcore_data_protocol_state_t *state = agcore_data_protocol_state_get(LINK_BLE);
    agcore_fragment_reset(&state->fragment);
}

/**
 * @brief 接收 BLE 分片，完整消息不复制地转移给队列
 * @param conn_handle 活动连接句柄
 * @param data 含分包头的片段
 * @param len 片段长度
 * @return ESP_OK 接受，否则错误码
 */
esp_err_t agcore_ble_rx_data_handle(uint16_t conn_handle, const uint8_t *data, uint16_t len)
{
    if (conn_handle != g_agcore_ble_conn_handle) {
        return ESP_ERR_INVALID_STATE;
    }
    agcore_data_protocol_state_t *state = agcore_data_protocol_state_get(LINK_BLE);
    agcore_fragment_result result = agcore_fragment_decode(&state->fragment, data, len, CONFIG_AGCORE_BLE_MESSAGE_MAX_LEN);
    if (result == AGCORE_FRAGMENT_MORE) {
        ble_npl_time_t ticks;
        ble_npl_time_ms_to_ticks(CONFIG_AGCORE_BLE_FRAGMENT_TIMEOUT_MS, &ticks);
        g_agcore_ble_rx_last_tick = ble_npl_time_get();
        if (ble_npl_callout_reset(&g_agcore_ble_rx_timeout, ticks) != 0) {
            agcore_fragment_reset(&state->fragment);
            return ESP_ERR_INVALID_STATE;
        }
        return ESP_OK;
    }
    ble_npl_callout_stop(&g_agcore_ble_rx_timeout);
    if (result != AGCORE_FRAGMENT_COMPLETE) {
        return result == AGCORE_FRAGMENT_NO_MEMORY ? ESP_ERR_NO_MEM : ESP_ERR_INVALID_ARG;
    }
    esp_err_t ret = agcore_data_push(LINK_BLE, state->fragment.buf, state->fragment.total_len);
    if (ret == ESP_OK) {
        state->fragment.buf = NULL;
    }
    agcore_fragment_reset(&state->fragment);
    return ret;
}

/**
 * @brief 通过 BLE GATT 通知发送数据(按 MTU 分片)
 * @param data 待发送数据
 * @param len 数据长度
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_ble_notify(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0 || len > CONFIG_AGCORE_BLE_MESSAGE_MAX_LEN) {
        return ESP_ERR_INVALID_ARG;
    }

    unsigned session = atomic_load(&g_agcore_ble_session);
    uint16_t conn_handle = g_agcore_ble_conn_handle;
    if (conn_handle == BLE_HS_CONN_HANDLE_NONE) {
        return ESP_ERR_INVALID_STATE;
    }

    /* GAP 从默认 MTU 23 开始，只会更新为协商后的有效 MTU。 */
    uint16_t payload_len_max = g_agcore_ble_gap_mtu - 3 - AGCORE_FRAGMENT_HEADER_LEN;
    uint16_t payload_sent = 0;
    uint8_t retries = 0;

    while (payload_sent < len) {
        uint16_t chunk_len = len - payload_sent;
        if (chunk_len > payload_len_max) {
            chunk_len = payload_len_max;
        }

        uint8_t header[AGCORE_FRAGMENT_HEADER_LEN];
        agcore_fragment_encode(header, len, payload_sent);
        struct os_mbuf *om = ble_hs_mbuf_from_flat(header, sizeof(header));
        if (!om) {
            CORE_LOGE(TAG, "mbuf allocate failed");
            return ESP_ERR_NO_MEM;
        }

        if (os_mbuf_append(om, data + payload_sent, chunk_len) != 0) {
            os_mbuf_free_chain(om);
            return ESP_ERR_NO_MEM;
        }
        if (session != atomic_load(&g_agcore_ble_session)) {
            os_mbuf_free_chain(om);
            return ESP_ERR_INVALID_STATE;
        }

        /* NimBLE 无论结果如何都会接管并释放 om。 */
        int ret = ble_gatts_notify_custom(conn_handle, g_agcore_ble_notify_handle, om);
        if (ret == 0) {
            payload_sent += chunk_len;
            retries = 0;
        } else if (ret == BLE_HS_EAGAIN) {
            if (++retries >= AGCORE_BLE_NOTIFY_RETRY_MAX) {
                CORE_LOGD(TAG, "notify buffer full|sent=%u len=%u", payload_sent, len);
                return ESP_ERR_TIMEOUT;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        } else {
            CORE_LOGE(TAG, "notify failed|%d", ret);
            return ESP_FAIL;
        }
    }

    return ESP_OK;
}


#endif /* CONFIG_AGCORE_BLE_ENABLE */
