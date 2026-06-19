#include "agcore_datachannel.h"



#include "agcore_printf_log.h"

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

    switch (data->cmd) {
        default: {
            if (s_agcore_data_app_cb != NULL) {
                s_agcore_data_app_cb(data);
            }
            break;
        }
    }

}
