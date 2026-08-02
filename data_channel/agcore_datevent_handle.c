#include "agcore_datevent_handle.h"

#include <time.h>
#include <sys/time.h>
#include <string.h>

#define TAG "AGCORE_DATAEVENT_HANDLE"

/**
 * @brief 处理 CORE_CMD_GET_VERINFO:查询设备版本信息
 * @param data 请求数据
 */
static void agcore_handle_get_verinfo_handle(const agcore_data_st *data)
{
    const agcore_version_info_st *ver = agcore_version_info_get();
    agcore_data_st rsp = {
        .link = data->link,   /* 从哪来回哪去 */
        .cmd = CORE_CMD_GET_VERINFO,
        .payload_len = 17,
    };

    agcore_device_info_get_bytes(rsp.payload, 8);   /* device_type+id+fw+hw 共8B */
    agcore_put_bytes16(&rsp.payload[8], ver->version); /* agcore_version 2B */
    memcpy(&rsp.payload[10], ver->git_hash, 7);              /* git hash 7B */

    agcore_data_send(&rsp);
}

/**
 * @brief 处理 CORE_CMD_GET_DEVICETIME:查询设备时间戳
 * @param data 请求数据
 */
static void agcore_handle_get_devicetime_handle(const agcore_data_st *data)
{
    uint32_t ts = (uint32_t)time(NULL);
    agcore_data_st rsp = {
        .link = data->link,   /* 从哪来回哪去 */
        .cmd = CORE_CMD_GET_DEVICETIME,
        .payload_len = 4,
    };

    agcore_put_bytes32(rsp.payload, ts);

    agcore_data_send(&rsp);
}

/**
 * @brief 处理 CORE_CMD_SET_DEVICETIME:设置设备时间戳
 * @param data 请求数据(payload: 4Byte 时间戳)
 */
static void agcore_handle_set_devicetime_handle(const agcore_data_st *data)
{
    uint32_t ts = agcore_get_bytes32(data->payload);
    struct timeval tv = {.tv_sec = ts, .tv_usec = 0};

    agcore_data_st rsp = {
        .link = data->link,
        .cmd = CORE_CMD_SET_DEVICETIME,
        .payload_len = 1,
        .payload[0] = (settimeofday(&tv, NULL) == 0),
    };

    CORE_LOGI(TAG, "set devicetime: %"PRIu32" %s", ts, (rsp.payload[0] ? "success" : "failed"));

    agcore_data_send(&rsp);
}

/**
 * @brief CORE 层数据分发
 * @note  处理 CORE 需要的数据,返回 true 表示已消费(截留);
 *        其余返回 false,交给路由推送到应用层
 */
bool agcore_data_handle_core(const agcore_data_st *data)
{
    if (data == NULL) {
        return false;
    }

    switch (data->cmd) {
        case CORE_CMD_GET_VERINFO: {
            agcore_handle_get_verinfo_handle(data);
            break;
        }
        case CORE_CMD_GET_DEVICETIME: {
            agcore_handle_get_devicetime_handle(data);
            break;
        }
        case CORE_CMD_SET_DEVICETIME: {
            agcore_handle_set_devicetime_handle(data);
            break;
        }
        default: {
            return false;
        }
    }
    return true;
}