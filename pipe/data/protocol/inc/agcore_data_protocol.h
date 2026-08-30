#ifndef __AGCORE_DATA_PROTOCOL_H__
#define __AGCORE_DATA_PROTOCOL_H__

#include "agcore_data.h"
#include "agcore_check.h"
#include "agcore_bytes.h"
#include "agcore_log.h"

/**
 * @brief 封装AGCORE顶层协议帧
 *
 * @param data 输入统一数据结构
 * @param buf 输出完整顶层协议包: Header + Ver + SN + PayloadLen + Cmd + Payload + CheckSum
 * @return uint16_t 完整顶层协议包长度，失败返回0
 */
uint16_t agcore_data_encode(agcore_data_t *data, uint8_t *buf);

/**
 * @brief 解析AGCORE顶层协议帧
 *
 * @param buf 输入完整顶层协议包
 * @param buf_len 完整顶层协议包长度
 * @param data 输出统一数据结构；调用前需要先设置data->link
 * @return uint16_t 顶层协议包长度，失败返回0
 */
uint16_t agcore_data_decode(const uint8_t *buf, uint16_t buf_len, agcore_data_t *data);

#endif /* __AGCORE_DATA_PROTOCOL_H__ */
