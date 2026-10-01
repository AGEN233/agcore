#include "agcore_core_cmd.h"
#include "initcall.h"

#include <time.h>
#include <sys/time.h>
#include <string.h>

#define TAG "AGCORE_CORE_CMD"

/**
 * @brief 处理 CORE_CMD_GET_VERINFO:查询设备版本信息
 * @param data 请求数据
 */
static void agcore_handle_get_verinfo_handle(const agcore_data_t *data)
{
    const agcore_version_info_t *ver = agcore_version_info_get();
    uint8_t payload[19];
    agcore_data_t rsp = {
        .link = data->link,   /* 从哪来回哪去 */
        .payload_len = sizeof(payload),
        .payload = payload,
    };

    agcore_put_bytes16(payload, CORE_CMD_GET_VERINFO);
    agcore_device_info_get_bytes(&payload[2], 8);   /* device_type+id+fw+hw 共8B */
    agcore_put_bytes16(&payload[10], ver->version); /* agcore_version 2B */
    memcpy(&payload[12], ver->git_hash, 7);         /* git hash 7B */

    agcore_data_send(&rsp);
}

/**
 * @brief 处理 CORE_CMD_GET_DEVICETIME:查询设备时间戳
 * @param data 请求数据
 */
static void agcore_handle_get_devicetime_handle(const agcore_data_t *data)
{
    uint32_t ts = (uint32_t)time(NULL);
    uint8_t payload[6];
    agcore_data_t rsp = {
        .link = data->link,   /* 从哪来回哪去 */
        .payload_len = sizeof(payload),
        .payload = payload,
    };

    agcore_put_bytes16(payload, CORE_CMD_GET_DEVICETIME);
    agcore_put_bytes32(&payload[2], ts);

    agcore_data_send(&rsp);
}

/**
 * @brief 处理 CORE_CMD_SET_DEVICETIME:设置设备时间戳
 * @param data 请求数据(payload: 4Byte 时间戳)
 */
static void agcore_handle_set_devicetime_handle(const agcore_data_t *data)
{
    if (data->payload_len < 4) {
        CORE_LOGD(TAG, "set devicetime payload too short|len=%u", data->payload_len);
        return;
    }

    uint32_t ts = agcore_get_bytes32(data->payload);
    struct timeval tv = {.tv_sec = ts, .tv_usec = 0};
    uint8_t payload[3];

    agcore_data_t rsp = {
        .link = data->link,
        .payload_len = sizeof(payload),
        .payload = payload,
    };
    agcore_put_bytes16(payload, CORE_CMD_SET_DEVICETIME);
    payload[2] = (settimeofday(&tv, NULL) == 0);

    CORE_LOGI(TAG, "set devicetime: %"PRIu32" %s", ts, (payload[2] ? "success" : "failed"));

    agcore_data_send(&rsp);
}

/**
 * @brief CORE 命令接收回调
 * @param data 完整业务 payload，由 CORE 解释命令格式
 */
static void agcore_core_data_handle(const agcore_data_t *data)
{
    if (data == NULL || data->payload == NULL || data->payload_len < 2) {
        return;
    }

    uint16_t cmd = agcore_get_bytes16(data->payload);
    agcore_data_t body = *data;
    body.payload += 2;
    body.payload_len -= 2;

    switch (cmd) {
        case CORE_CMD_GET_VERINFO: {
            agcore_handle_get_verinfo_handle(&body);
            break;
        }
        case CORE_CMD_GET_DEVICETIME: {
            agcore_handle_get_devicetime_handle(&body);
            break;
        }
        case CORE_CMD_SET_DEVICETIME: {
            agcore_handle_set_devicetime_handle(&body);
            break;
        }
        default: {
            return;
        }
    }
}

/**
 * @brief 注册 CORE 命令接收回调
 * @return 初始化结果
 */
static int agcore_core_data_handler_init(void)
{
    agcore_data_handler_register(agcore_core_data_handle);
    return ESP_OK;
}

AGCORE_CORE_INITCALL(agcore_core_data_handler_init);
