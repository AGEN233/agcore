#include "agcore_nvs.h"

#include <string.h>

#include "nvs.h"

#define AGCORE_NVS_CORE_NAMESPACE "agcore"
#define AGCORE_NVS_USER_NAMESPACE "userdata"
#define AGCORE_NVS_KEY_MAX_LEN    15

/**
 * @brief 校验 NVS 读写参数是否合法
 * @param key 键名
 * @param buf 数据缓冲
 * @param len 数据长度
 * @return ESP_OK 合法, 否则 ESP_ERR_INVALID_ARG
 */
static esp_err_t agcore_nvs_check_args(const char *key, const void *buf, size_t len)
{
    if (!key || !buf || len == 0 || strlen(key) > AGCORE_NVS_KEY_MAX_LEN) {
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

/**
 * @brief 从指定命名空间读取 blob 数据
 * @param namespace 命名空间
 * @param key 键名
 * @param buf 输出缓冲
 * @param len 读取长度
 * @return ESP_OK 成功, 否则错误码
 */
static esp_err_t agcore_nvs_read_blob(const char *namespace, const char *key, void *buf, size_t len)
{
    esp_err_t ret = agcore_nvs_check_args(key, buf, len);
    if (ret != ESP_OK) {
        return ret;
    }

    nvs_handle_t handle;
    ret = nvs_open(namespace, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        return ret;
    }

    size_t required_len = len;
    ret = nvs_get_blob(handle, key, buf, &required_len);
    nvs_close(handle);
    return ret;
}

/**
 * @brief 向指定命名空间写入 blob 数据并提交
 * @param namespace 命名空间
 * @param key 键名
 * @param buf 数据缓冲
 * @param len 数据长度
 * @return ESP_OK 成功, 否则错误码
 */
static esp_err_t agcore_nvs_write_blob(const char *namespace, const char *key, const void *buf, size_t len)
{
    esp_err_t ret = agcore_nvs_check_args(key, buf, len);
    if (ret != ESP_OK) {
        return ret;
    }

    nvs_handle_t handle;
    ret = nvs_open(namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_set_blob(handle, key, buf, len);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
    }

    nvs_close(handle);
    return ret;
}

/**
 * @brief 擦除指定命名空间下的键并提交
 * @param namespace 命名空间
 * @param key 键名
 * @return ESP_OK 成功, 否则错误码
 */
static esp_err_t agcore_nvs_erase_key(const char *namespace, const char *key)
{
    if (!key || strlen(key) > AGCORE_NVS_KEY_MAX_LEN) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_erase_key(handle, key);
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
    }

    nvs_close(handle);
    return ret;
}

/**
 * @brief 读取 CORE 命名空间的 blob 数据
 * @param key 键名
 * @param buf 输出缓冲
 * @param len 读取长度
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_coredata_read(const char *key, void *buf, size_t len)
{
    return agcore_nvs_read_blob(AGCORE_NVS_CORE_NAMESPACE, key, buf, len);
}

/**
 * @brief 写入 CORE 命名空间的 blob 数据
 * @param key 键名
 * @param buf 数据缓冲
 * @param len 数据长度
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_coredata_write(const char *key, const void *buf, size_t len)
{
    return agcore_nvs_write_blob(AGCORE_NVS_CORE_NAMESPACE, key, buf, len);
}

/**
 * @brief 擦除 CORE 命名空间下的键
 * @param key 键名
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_coredata_erase(const char *key)
{
    return agcore_nvs_erase_key(AGCORE_NVS_CORE_NAMESPACE, key);
}

/**
 * @brief 读取用户命名空间的 blob 数据
 * @param key 键名
 * @param buf 输出缓冲
 * @param len 读取长度
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_userdata_read(const char *key, void *buf, size_t len)
{
    return agcore_nvs_read_blob(AGCORE_NVS_USER_NAMESPACE, key, buf, len);
}

/**
 * @brief 写入用户命名空间的 blob 数据
 * @param key 键名
 * @param buf 数据缓冲
 * @param len 数据长度
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_userdata_write(const char *key, const void *buf, size_t len)
{
    return agcore_nvs_write_blob(AGCORE_NVS_USER_NAMESPACE, key, buf, len);
}

/**
 * @brief 擦除用户命名空间下的键
 * @param key 键名
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_userdata_erase(const char *key)
{
    return agcore_nvs_erase_key(AGCORE_NVS_USER_NAMESPACE, key);
}
