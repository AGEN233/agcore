#include <stdio.h>

/* 用尖括号 include, 强制走 include 路径搜索, 命中 CMake 生成版(generated/agcore_version.h),
   而非同目录模板版(含 @占位符@)。 */
#include <agcore_version.h>

static agcore_device_info_t g_agcore_device_info = {
    .device_type = 0,
    .device_id = 0,
    .fw_version = 0,
    .hw_version = 0,
};

static bool g_agcore_device_info_is_set = false;

static const agcore_version_info_t g_agcore_version_info = {
    .version = AGCORE_VERSION,
    .git_hash = AGCORE_GIT_HASH,
    .git_branch = AGCORE_GIT_BRANCH,
    .build_time = AGCORE_BUILD_TIME,
};

/**
 * @brief 设置设备信息
 * @param info 设备信息指针
 */
void agcore_device_info_set(const agcore_device_info_t *info)
{
    if (info == NULL) {
        return;
    }

    g_agcore_device_info = *info;
    g_agcore_device_info_is_set = true;
}

/**
 * @brief 设备信息是否已设置
 * @return 已设置返回 true
 */
bool agcore_device_info_is_set(void)
{
    return g_agcore_device_info_is_set;
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

    if (!g_agcore_device_info_is_set || len < 8) {
        return;
    }

    buf[0] = (uint8_t)(g_agcore_device_info.device_type >> 8);
    buf[1] = (uint8_t)(g_agcore_device_info.device_type);
    agcore_put_bytes16(&buf[2], g_agcore_device_info.device_id);
    agcore_put_bytes16(&buf[4], g_agcore_device_info.fw_version);
    agcore_put_bytes16(&buf[6], g_agcore_device_info.hw_version);
}

/**
 * @brief 获取设备信息
 * @return 设备信息指针
 */
const agcore_device_info_t *agcore_device_info_get(void)
{
    return &g_agcore_device_info;
}

/**
 * @brief 获取 AGCORE 版本信息
 * @return AGCORE 版本信息指针
 */
const agcore_version_info_t *agcore_version_info_get(void)
{
    return &g_agcore_version_info;
}

/**
 * @brief 获取设备类型
 * @return 设备类型
 */
uint16_t agcore_device_info_get_type(void)
{
    return g_agcore_device_info.device_type;
}

/**
 * @brief 获取设备 ID
 * @return 设备 ID
 */
uint16_t agcore_device_info_get_id(void)
{
    return g_agcore_device_info.device_id;
}

/**
 * @brief 获取固件版本
 * @return 固件版本
 */
uint16_t agcore_device_info_get_fw_version(void)
{
    return g_agcore_device_info.fw_version;
}

/**
 * @brief 获取硬件版本
 * @return 硬件版本
 */
uint16_t agcore_device_info_get_hw_version(void)
{
    return g_agcore_device_info.hw_version;
}

/**
 * @brief 获取 AGCORE 版本
 * @return AGCORE 版本
 */
uint16_t agcore_version_info_get_version(void)
{
    return g_agcore_version_info.version;
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
                    g_agcore_device_info.device_type,
                    g_agcore_device_info.device_id,
                    g_agcore_device_info.fw_version,
                    g_agcore_device_info.hw_version);
}

/**
 * @brief 获取 AGCORE 标识字符串
 * @return VERSION
 */
int agcore_version_info_get_string(char *buf, size_t buf_len)
{
    if (buf == NULL || buf_len == 0) {
        return -1;
    }

    return snprintf(buf, buf_len, "%04X", g_agcore_version_info.version);
}
