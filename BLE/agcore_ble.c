#include "agcore.h"
#include "sdkconfig.h"

#ifdef CONFIG_AGCORE_BLE_ENABLE
#include "agcore_ble.h"
#include "agcore_ble_internal.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_store.h"
#include "host/util/util.h"
#include "esp_mac.h"

#define TAG "BLE"

static TaskHandle_t s_agcore_ble_host_handle = NULL;
static uint8_t s_agcore_ble_addr_type = 0;


/**
 * @brief 获取mac类型
 * @return  #define BLE_ADDR_PUBLIC      (0x00)
            #define BLE_ADDR_RANDOM      (0x01)
            #define BLE_ADDR_PUBLIC_ID   (0x02)
            #define BLE_ADDR_RANDOM_ID   (0x03)
            #define BLE_ADDR_ANONYMOUS   (0xFF)
 */
uint8_t agcore_ble_get_addr_type(void)
{
    return s_agcore_ble_addr_type;
}

/**
 * @brief 获取蓝牙公共地址
 * @param mac
 */
void agcore_ble_get_addr(uint8_t *addr)
{
    if (!addr) return;
    esp_read_mac(addr, ESP_MAC_BT);
}

/**
 * @brief
 */
static void agcore_ble_on_stack_reset_cb(int reason)
{

}

/**
 * @brief 协议栈与控制器完成同步回调
 */
static void agcore_ble_on_stack_sync_cb(void)
{
    CORE_LOGD(TAG, "host sync success");
    agcore_ble_gap_init();
    agcore_ble_gatt_init();
    agcore_ble_adv_init();
    agcore_ble_adv_update();
    agcore_ble_adv_start();
}

/**
 * @brief
 */
static int agcore_ble_on_stack_store_status_cb(struct ble_store_status_event *event, void *arg)
{
    return 0;
}

/**
 * @brief 蓝牙主机任务
 */
static void agcore_ble_host_task(void *arg)
{
    /* This function won't return until nimble_port_stop() is executed */
    nimble_port_run();

    /* Clean up at exit */
    vTaskDelete(NULL);
}

/**
 * @brief nimble栈初始化
 */
static esp_err_t agcore_ble_nimble_init(void)
{
    /* NimBLE host stack initialization */
    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "nimble port init failed|%s", esp_err_to_name(ret));
        return ret;
    }

    /* NimBLE host configuration initialization */
    /* Set host callbacks */
    ble_hs_cfg.reset_cb = agcore_ble_on_stack_reset_cb;
    ble_hs_cfg.sync_cb = agcore_ble_on_stack_sync_cb;
    ble_hs_cfg.store_status_cb = agcore_ble_on_stack_store_status_cb;


    /* Start NimBLE host task thread and return */
    if (s_agcore_ble_host_handle == NULL) {
        xTaskCreate(agcore_ble_host_task, "AGBLEHOST", 4 * 1024, NULL, 5, &s_agcore_ble_host_handle);
        if (s_agcore_ble_host_handle == NULL) {
            CORE_LOGE(TAG, "host task create failed");
            return ESP_ERR_NO_MEM;
        }
    }

    s_agcore_ble_addr_type  = BLE_ADDR_PUBLIC;
    uint8_t mac[6];
    if (ble_hs_id_copy_addr(s_agcore_ble_addr_type, mac, NULL) != 0) {
        CORE_LOGW(TAG, "public address unavailable, fallback to random");
        s_agcore_ble_addr_type = BLE_ADDR_RANDOM;
    } else {
        CORE_LOGI(TAG, "ble address: %02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    }

    return ESP_OK;
}
#endif

/**
 * @brief 蓝牙初始化
 */
esp_err_t agcore_ble_init(void)
{
    #ifdef CONFIG_AGCORE_BLE_ENABLE
        esp_err_t ret =  agcore_ble_nimble_init();
        if (ret != ESP_OK) {
            return ESP_FAIL;
        }
        return ESP_OK;
    #else
        return ESP_FAIL;
    #endif
}
