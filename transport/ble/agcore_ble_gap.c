#include "sdkconfig.h"

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include "agcore_ble_adv.h"
#include "agcore_ble_gap.h"
#include "agcore_data.h"
#include "agcore_log.h"
#include "nimble/nimble_port.h"
#include "os/os_cputime.h"
#include "services/gap/ble_svc_gap.h"
#define TAG "AGCORE_BLE_GAP"

static struct ble_npl_callout g_agcore_ble_mtu_callout;
static bool g_agcore_ble_connected = false;
volatile uint16_t g_agcore_ble_gap_mtu = 23;
uint16_t g_agcore_ble_conn_handle = BLE_HS_CONN_HANDLE_NONE;

/**
 * @brief 主动发起mtu协商
 * @param ev
 */
static void agcore_ble_gap_mtu_exchange_initiate(struct ble_npl_event *ev)
{
    if (g_agcore_ble_gap_mtu == 23 && g_agcore_ble_conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        int ret = ble_gattc_exchange_mtu(g_agcore_ble_conn_handle, NULL, NULL);
        if (ret != 0) {
            CORE_LOGE(TAG, "mtu exchange failed|%d", ret);
        }
    }
}

/**
 * @brief GAP连接处理
 */
/**
 * @brief GAP 连接事件处理: 更新连接状态并触发 MTU 协商
 * @param event GAP 事件
 */
static void agcore_ble_gap_connect_handle(struct ble_gap_event *event)
{
    if (event->connect.status == 0) {
        g_agcore_ble_connected = true;
        g_agcore_ble_conn_handle = event->connect.conn_handle;
        agcore_data_link_reset(LINK_BLE);

        struct ble_gap_conn_desc conn_desc;
        if (ble_gap_conn_find(g_agcore_ble_conn_handle, &conn_desc) == 0) {
            const uint8_t *addr = conn_desc.peer_id_addr.val;
            CORE_LOGD(TAG, "peer address: %02X:%02X:%02X:%02X:%02X:%02X", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
        } else {
            CORE_LOGD(TAG, "peer address unavailable");
        }

        ble_npl_time_t ticks;
        ble_npl_time_ms_to_ticks(500, &ticks);
        ble_npl_callout_reset(&g_agcore_ble_mtu_callout, ticks);

        CORE_LOGD(TAG, "connected");
    }
}

/**
 * @brief GAP断开连接处理
 */
/**
 * @brief GAP 断开事件处理: 复位链路并恢复广播
 */
static void agcore_ble_gap_disconnect_handle(void)
{
    ble_npl_callout_stop(&g_agcore_ble_mtu_callout);
    g_agcore_ble_connected = false;
    g_agcore_ble_conn_handle = BLE_HS_CONN_HANDLE_NONE;
    agcore_data_link_reset(LINK_BLE);
    agcore_ble_adv_update();
    agcore_ble_adv_start();
    CORE_LOGD(TAG, "disconnected");
}

/**
 * @brief MTU更新处理
 */
/**
 * @brief MTU 更新事件处理
 * @param event GAP 事件
 */
static void agcore_ble_gap_mtu_handle(struct ble_gap_event *event)
{
    g_agcore_ble_gap_mtu = event->mtu.value;
    ble_npl_callout_stop(&g_agcore_ble_mtu_callout);
    CORE_LOGD(TAG, "mtu updated|%d", g_agcore_ble_gap_mtu);
}

/**
 * @brief 获取MTU
 * @return uint16_t
 */
uint16_t agcore_ble_get_mtu(void)
{
    return g_agcore_ble_gap_mtu;
}

/**
 * @brief GAP 事件总回调
 * @return 0
 */
int agcore_ble_gap_event_cb(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT: {
            agcore_ble_gap_connect_handle(event);
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT: {
            agcore_ble_gap_disconnect_handle();
            break;
        }
        case BLE_GAP_EVENT_MTU: {
            agcore_ble_gap_mtu_handle(event);
            break;
        }
        default: {
            break;
        }
    }
    return 0;
}

/**
 * @brief 获取蓝牙链接状态
 * @return true
 * @return false
 */
bool agcore_ble_is_connected(void)
{
    return g_agcore_ble_connected;
}

/**
 * @brief GAP 参数设置与初始化
 */
void agcore_ble_gap_init(void)
{
    ble_svc_gap_init();
    ble_npl_callout_init(&g_agcore_ble_mtu_callout, nimble_port_get_dflt_eventq(), agcore_ble_gap_mtu_exchange_initiate, NULL);
}

#endif
