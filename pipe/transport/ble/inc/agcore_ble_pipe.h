#ifndef __AGCORE_BLE_PIPE_H__
#define __AGCORE_BLE_PIPE_H__

#ifdef CONFIG_AGCORE_BLE_ENABLE

#include <stdint.h>

#include "esp_err.h"

esp_err_t agcore_ble_notify(const uint8_t *data, uint16_t len);

/** @brief 初始化接收超时事件，必须在 NimBLE host 上下文调用。 */
void agcore_ble_pipe_init(void);

/** @brief 清理重组并使在途发送失效，必须在 NimBLE host 上下文调用。 */
void agcore_ble_pipe_reset(void);

/**
 * @brief 在 pipe 初始化后的 GATT 写回调中接收分片，收完整后转移给统一队列
 * @param conn_handle 来源连接
 * @param data 含大端分包头的片段
 * @param len 片段长度
 * @return ESP_OK 接受，其他值表示接收失败
 */
esp_err_t agcore_ble_rx_data_handle(uint16_t conn_handle, const uint8_t *data, uint16_t len);

#endif

#endif /* __AGCORE_BLE_PIPE_H__ */
