#include "agcore_lifecycle.h"
#include "initcall.h"

#include <stddef.h>
#include <string.h>

#include "sdkconfig.h"

#include "agcore_check.h"
#include "agcore_log.h"
#include "agcore_nvs.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"

#define TAG "lifecycle"

#define AGCORE_LIFECYCLE_KEY       "life"
typedef struct {
    uint8_t magic[4];
    uint32_t boot_total_count;
    uint32_t boot_streak_count;
    uint32_t reset_reason;
    agcore_boot_reason boot_reason;
    uint8_t reserved;
    uint16_t crc16;
} agcore_lifecycle_data_t;

static agcore_lifecycle_data_t g_agcore_lifecycle_data;
static agcore_boot_reason g_agcore_boot_reason = BOOT_NORMAL;
static TaskHandle_t g_agcore_lifecycle_stable_task;

/**
 * @brief 判断持久化启动原因是否可恢复
 * @param boot_reason 启动原因
 * @return 可恢复返回 true
 */
static bool agcore_lifecycle_boot_reason_is_valid(agcore_boot_reason boot_reason)
{
    return boot_reason == BOOT_NORMAL ||
           boot_reason == BOOT_BLANK ||
           boot_reason == BOOT_FACTORY ||
           boot_reason == BOOT_ABNORMAL;
}

/**
 * @brief 计算生命周期数据的 CRC16(不含 crc16 字段)
 * @param data 生命周期数据
 * @return CRC16 校验值
 */
static uint16_t agcore_lifecycle_crc16(const agcore_lifecycle_data_t *data)
{
    const uint8_t *bytes = (const uint8_t *)data;
    size_t len = offsetof(agcore_lifecycle_data_t, crc16);

    return agcore_crc16_calc(0xFFFFU, bytes, (uint16_t)len);
}

/**
 * @brief 重新计算并刷新数据的 crc16 字段
 * @param data 生命周期数据
 */
static void agcore_lifecycle_crc_refresh(agcore_lifecycle_data_t *data)
{
    data->crc16 = agcore_lifecycle_crc16(data);
}

/**
 * @brief 校验生命周期数据: magic 与 CRC16 是否匹配
 * @param data 生命周期数据
 * @return 有效返回 true
 */
static bool agcore_lifecycle_data_is_valid(const agcore_lifecycle_data_t *data)
{
    if (memcmp(data->magic, AGCORE_LIFECYCLE_KEY, sizeof(data->magic)) != 0) {
        return false;
    }

    return data->crc16 == agcore_lifecycle_crc16(data);
}

/**
 * @brief 将生命周期数据填充为出厂默认值
 * @param data 生命周期数据
 */
static void agcore_lifecycle_data_default(agcore_lifecycle_data_t *data)
{
    memset(data, 0, sizeof(*data));
    memcpy(data->magic, AGCORE_LIFECYCLE_KEY, sizeof(data->magic));
    data->boot_reason = BOOT_BLANK;
    agcore_lifecycle_crc_refresh(data);
}

/**
 * @brief 将当前生命周期数据写入 NVS
 * @return ESP_OK 成功, 否则错误码
 */
static esp_err_t agcore_lifecycle_save(void)
{
    g_agcore_lifecycle_data.boot_reason = (uint8_t)g_agcore_boot_reason;
    agcore_lifecycle_crc_refresh(&g_agcore_lifecycle_data);
    return agcore_coredata_write(AGCORE_LIFECYCLE_KEY, &g_agcore_lifecycle_data, sizeof(g_agcore_lifecycle_data));
}

/**
 * @brief 稳定标记延时任务: 达到稳定时间后标记启动稳定
 */
static void agcore_lifecycle_stable_task(void *arg)
{
    (void)arg;

    vTaskDelay(pdMS_TO_TICKS(CONFIG_AGCORE_LIFECYCLE_STABLE_CLEAR_MS));
    agcore_lifecycle_mark_stable();
    g_agcore_lifecycle_stable_task = NULL;
    vTaskDelete(NULL);
}

/**
 * @brief 生命周期初始化: 读取/校验/重建数据并累计启动统计
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_lifecycle_init(void)
{
    agcore_lifecycle_data_t data = {0};
    esp_err_t ret = agcore_coredata_read(AGCORE_LIFECYCLE_KEY, &data, sizeof(data));

    if (ret == ESP_ERR_NVS_NOT_FOUND || !agcore_lifecycle_data_is_valid(&data)) {
        agcore_lifecycle_data_default(&data);
        g_agcore_boot_reason = BOOT_BLANK;
        CORE_LOGW(TAG, "data invalid|read_ret=%s, reset to default", esp_err_to_name(ret));
    } else if (ret != ESP_OK) {
        CORE_LOGE(TAG, "read failed|%s", esp_err_to_name(ret));
        return ret;
    } else {
        g_agcore_boot_reason = data.boot_reason;
        if (!agcore_lifecycle_boot_reason_is_valid(g_agcore_boot_reason)) {
            g_agcore_boot_reason = BOOT_NORMAL;
        }
    }

    data.boot_total_count++;
    data.boot_streak_count++;
    data.reset_reason = (uint32_t)esp_reset_reason();

    if (data.boot_streak_count >= CONFIG_AGCORE_LIFECYCLE_FACTORY_BOOT_COUNT) {
        g_agcore_boot_reason = BOOT_FACTORY;
        CORE_LOGI(TAG, "reach factory boot count|streak=%u", data.boot_streak_count);
    }

    g_agcore_lifecycle_data = data;
    ret = agcore_lifecycle_save();
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "save failed|%s", esp_err_to_name(ret));
        return ret;
    }

    if (g_agcore_lifecycle_stable_task == NULL) {
        BaseType_t task_ret = xTaskCreate(agcore_lifecycle_stable_task, "aglife", 2048, NULL, 5, &g_agcore_lifecycle_stable_task);
        if (task_ret != pdPASS) {
            g_agcore_lifecycle_stable_task = NULL;
            CORE_LOGE(TAG, "stable task create failed");
        }
    }

    CORE_LOGI(TAG, "boot_reason:%d total:%u streak:%u reset_reason:%04X",
              g_agcore_boot_reason,
              g_agcore_lifecycle_data.boot_total_count,
              g_agcore_lifecycle_data.boot_streak_count,
              g_agcore_lifecycle_data.reset_reason);

    return ESP_OK;
}

AGCORE_CORE_INITCALL(agcore_lifecycle_init);

/**
 * @brief 标记启动稳定并清零连续启动计数
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_lifecycle_mark_stable(void)
{
    g_agcore_lifecycle_data.boot_streak_count = 0;
    g_agcore_boot_reason = BOOT_NORMAL;
    CORE_LOGI(TAG, "boot streak cleaned");
    return agcore_lifecycle_save();
}

/**
 * @brief 获取本次启动原因
 * @return 启动原因枚举
 */
agcore_boot_reason agcore_lifecycle_boot_reason_get(void)
{
    return g_agcore_boot_reason;
}

/**
 * @brief 是否为出厂首启(空白或达到出厂启动次数)
 * @return 是返回 true
 */
bool agcore_lifecycle_is_factory_boot(void)
{
    return g_agcore_boot_reason == BOOT_FACTORY || g_agcore_boot_reason == BOOT_BLANK;
}

/**
 * @brief 获取累计启动次数
 * @return 累计启动次数
 */
uint32_t agcore_lifecycle_boot_total_count_get(void)
{
    return g_agcore_lifecycle_data.boot_total_count;
}

/**
 * @brief 获取连续启动次数
 * @return 连续启动次数
 */
uint32_t agcore_lifecycle_boot_streak_count_get(void)
{
    return g_agcore_lifecycle_data.boot_streak_count;
}

/**
 * @brief 获取上次复位原因
 * @return 复位原因
 */
uint32_t agcore_lifecycle_reset_reason_get(void)
{
    return g_agcore_lifecycle_data.reset_reason;
}
