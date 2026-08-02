#include "agcore_datachannel.h"



#include "agcore_log_adapter.h"

#define TAG "AGCORE_DATAEVENT_ROUTE"

static agcore_data_cb s_agcore_data_app_cb;

void agcore_data_handler_register(agcore_data_cb cb)
{
    if (cb == NULL) {
        CORE_LOGD(TAG, "data callback invalid");
        return;
    }
    s_agcore_data_app_cb = cb;
}

/**
 * @brief 统一数据路由
 * @note  这里拿掉CORE需要的数据，其他的推送到应用层
 * @param data
 */
void agcore_data_route_handler(const agcore_data_st *data)
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
    if (s_agcore_data_app_cb != NULL) {
        s_agcore_data_app_cb(data);
    }
}
