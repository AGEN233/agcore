#include "agcore_data.h"
#include "agcore_log.h"

#define TAG "AGCORE_DATA_ROUTE"

static agcore_data_cb g_agcore_data_app_cb;

/**
 * @brief 注册应用层数据回调
 * @param cb 应用层回调
 */
void agcore_data_handler_register(agcore_data_cb cb)
{
    if (cb == NULL) {
        CORE_LOGD(TAG, "data callback invalid");
        return;
    }
    g_agcore_data_app_cb = cb;
}

/**
 * @brief 统一数据路由
 * @note  这里拿掉CORE需要的数据，其他的推送到应用层
 * @param data
 */
void agcore_data_route_handler(const agcore_data_t *data)
{
    if (data == NULL) {
        CORE_LOGD(TAG, "data invalid");
        return;
    }

    /* 先让 CORE 处理自己需要的命令,命中则截留,不再推给应用层 */
    if (agcore_data_handle_core(data)) {
        return;
    }

    /* 剩余数据推送到应用层 */
    if (g_agcore_data_app_cb != NULL) {
        g_agcore_data_app_cb(data);
    }
}
