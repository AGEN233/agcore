#ifndef __AGCORE_DATA_ROUTE_H__
#define __AGCORE_DATA_ROUTE_H__

#include "agcore_data.h"

/* 回调同步借用只读 payload，不得释放或在返回后保留；应答另行构造。 */
typedef void (*agcore_data_cb)(const agcore_data_t *data);

/* 在开始接收数据前注册；重复注册同一回调不会创建新节点。 */
void agcore_data_handler_register(agcore_data_cb cb);
void agcore_data_route_handler(const agcore_data_t *data);

#endif /* __AGCORE_DATA_ROUTE_H__ */
