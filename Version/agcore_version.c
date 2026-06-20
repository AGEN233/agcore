#include "agcore.h"

#include <stdio.h>

static agcore_device_info_st s_agcore_device_info = {
    .device_type = 0,
    .device_id = 0,
    .fw_version = 0,
    .hw_version = 0,
};

static bool s_agcore_device_info_is_set = false;

static const agcore_version_info_st s_agcore_version_info = {
    .version = AGCORE_VERSION,
    .id = AGCORE_ID,
    .git_hash = AGCORE_GIT_HASH,
    .git_branch = AGCORE_GIT_BRANCH,
    .build_time = AGCORE_BUILD_TIME,
};

/**
 * @brief 设置设备信息
 * @param info 设备信息指针
 */
void agcore_device_info_set(const agcore_device_info_st *info)
{
    if (info == NULL) {
        return;
    }

    s_agcore_device_info = *info;
    s_agcore_device_info_is_set = true;
}

bool agcore_device_info_is_set(void)
{
    return s_agcore_device_info_is_set;
}

/**
 * @brief 填充设备信息字节为字节流
 * @param buf 输出缓存
 * @param len 输出缓存长度
 */
void agcore_device_info_get_bytes(uint8_t *buf, uint16_t len)
{
    if (buf == NULL) {
        return;
    }

    for (uint16_t i = 0; i < len; i++) {
        buf[i] = (i % 2 == 0) ? 0x00 : 0xFF;
    }

    if (!s_agcore_device_info_is_set || len < 8) {
        return;
    }

    buf[0] = (uint8_t)(s_agcore_device_info.device_type >> 8);
    buf[1] = (uint8_t)(s_agcore_device_info.device_type);
    buf[2] = (uint8_t)(s_agcore_device_info.device_id >> 8);
    buf[3] = (uint8_t)(s_agcore_device_info.device_id);
    buf[4] = (uint8_t)(s_agcore_device_info.fw_version >> 8);
    buf[5] = (uint8_t)(s_agcore_device_info.fw_version);
    buf[6] = (uint8_t)(s_agcore_device_info.hw_version >> 8);
    buf[7] = (uint8_t)(s_agcore_device_info.hw_version);
}

/**
 * @brief 获取设备信息
 * @return 设备信息指针
 */
const agcore_device_info_st *agcore_device_info_get(void)
{
    return &s_agcore_device_info;
}

/**
 * @brief 获取 AGCORE 版本信息
 * @return AGCORE 版本信息指针
 */
const agcore_version_info_st *agcore_version_info_get(void)
{
    return &s_agcore_version_info;
}

/**
 * @brief 获取设备类型
 * @return 设备类型
 */
uint16_t agcore_device_info_get_type(void)
{
    return s_agcore_device_info.device_type;
}

/**
 * @brief 获取设备 ID
 * @return 设备 ID
 */
uint16_t agcore_device_info_get_id(void)
{
    return s_agcore_device_info.device_id;
}

/**
 * @brief 获取固件版本
 * @return 固件版本
 */
uint16_t agcore_device_info_get_fw_version(void)
{
    return s_agcore_device_info.fw_version;
}

/**
 * @brief 获取硬件版本
 * @return 硬件版本
 */
uint16_t agcore_device_info_get_hw_version(void)
{
    return s_agcore_device_info.hw_version;
}

/**
 * @brief 获取 AGCORE 版本
 * @return AGCORE 版本
 */
uint16_t agcore_version_info_get_version(void)
{
    return s_agcore_version_info.version;
}

/**
 * @brief 获取 AGCORE ID
 * @return AGCORE ID
 */
uint16_t agcore_version_info_get_id(void)
{
    return s_agcore_version_info.id;
}

/**
 * @brief 获取设备标识字符串
 * @return TYPE_ID_FW_HW
 */
int agcore_device_info_get_string(char *buf, size_t buf_len)
{
    if (buf == NULL || buf_len == 0) {
        return -1;
    }

    return snprintf(buf, buf_len, "%04X_%04X_%04X_%04X",
                    s_agcore_device_info.device_type,
                    s_agcore_device_info.device_id,
                    s_agcore_device_info.fw_version,
                    s_agcore_device_info.hw_version);
}

/**
 * @brief 获取 AGCORE 标识字符串
 * @return VERSION_ID
 */
int agcore_version_info_get_string(char *buf, size_t buf_len)
{
    if (buf == NULL || buf_len == 0) {
        return -1;
    }

    return snprintf(buf, buf_len, "%04X_%04X", s_agcore_version_info.version, s_agcore_version_info.id);
}
