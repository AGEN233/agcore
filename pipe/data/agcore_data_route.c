#include "agcore_data.h"
#include "agcore_console_log.h"

#define TAG "AGCORE_DATA_ROUTE"

typedef struct agcore_data_handler_node {
    agcore_data_cb cb;
    struct agcore_data_handler_node *next;
} agcore_data_handler_node_t;

static agcore_data_handler_node_t *g_agcore_data_handler_head;
static agcore_data_handler_node_t *g_agcore_data_handler_tail;

/**
 * @brief 注册统一数据接收回调
 * @param cb 接收回调，重复注册时保持原有顺序
 */
void agcore_data_handler_register(agcore_data_cb cb)
{
    if (cb == NULL) {
        CORE_LOGD(TAG, "data callback invalid");
        return;
    }

    for (agcore_data_handler_node_t *node = g_agcore_data_handler_head; node != NULL; node = node->next) {
        if (node->cb == cb) {
            return;
        }
    }

    agcore_data_handler_node_t *node = agcore_malloc(sizeof(*node));
    if (node == NULL) {
        CORE_LOGE(TAG, "data callback malloc failed");
        return;
    }
    node->cb = cb;
    node->next = NULL;

    if (g_agcore_data_handler_tail == NULL) {
        g_agcore_data_handler_head = node;
    } else {
        g_agcore_data_handler_tail->next = node;
    }
    g_agcore_data_handler_tail = node;
}

/**
 * @brief 将解码后的数据同步分发给所有已注册回调
 * @param data 解码数据；回调返回后缓冲区失效
 */
void agcore_data_route_handler(const agcore_data_t *data)
{
    if (data == NULL || (data->payload_len > 0 && data->payload == NULL)) {
        CORE_LOGD(TAG, "data invalid");
        return;
    }

    agcore_data_handler_node_t *last = g_agcore_data_handler_tail;
    for (agcore_data_handler_node_t *node = g_agcore_data_handler_head; node != NULL; node = node->next) {
        node->cb(data);
        if (node == last) {
            break;
        }
    }
}
