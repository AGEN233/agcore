#include "agcore_fs.h"

#include "sdkconfig.h"

#ifdef CONFIG_AGCORE_FS_ENABLE
#include "esp_spiffs.h"
#endif

static bool g_agcore_fs_mounted = false;

/**
 * @brief 挂载文件系统
 * @return ESP_OK 成功, 否则错误码; 未使能时返回 ESP_ERR_NOT_SUPPORTED
 */
esp_err_t agcore_fs_mount(void)
{
#ifdef CONFIG_AGCORE_FS_ENABLE
    if (g_agcore_fs_mounted) {
        return ESP_OK;
    }

    esp_vfs_spiffs_conf_t conf = {
        .base_path = AGCORE_FS_BASE_PATH,
        .partition_label = CONFIG_AGCORE_FS_PARTITION_LABEL,
        .max_files = 4,
        .format_if_mount_failed = false,
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret == ESP_OK) {
        g_agcore_fs_mounted = true;
    }
    return ret;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

/**
 * @brief 卸载文件系统
 * @return ESP_OK 成功, 否则错误码; 未使能时返回 ESP_ERR_NOT_SUPPORTED
 */
esp_err_t agcore_fs_unmount(void)
{
#ifdef CONFIG_AGCORE_FS_ENABLE
    if (!g_agcore_fs_mounted) {
        return ESP_OK;
    }

    esp_err_t ret = esp_vfs_spiffs_unregister(CONFIG_AGCORE_FS_PARTITION_LABEL);
    if (ret == ESP_OK) {
        g_agcore_fs_mounted = false;
    }
    return ret;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

/**
 * @brief 查询文件系统是否已挂载
 * @return 已挂载返回 true
 */
bool agcore_fs_is_mounted(void)
{
    return g_agcore_fs_mounted;
}
