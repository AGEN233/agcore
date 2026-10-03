#ifndef __AGCORE_DATA_PROTOCOL_H__
#define __AGCORE_DATA_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "agcore_data.h"
#include "agcore_check.h"
#include "agcore_bytes.h"
#include "agcore_console_log.h"

#define AGCORE_RAWDATA_OVERHEAD 9

/* 完整帧长度受 uint16_t 限制，payload 需扣除帧开销。 */
#define AGCORE_RAWDATA_PAYLOAD_MAX_LEN (UINT16_MAX - AGCORE_RAWDATA_OVERHEAD)

/** @brief 分包头长度：2 字节总长度和 2 字节偏移，均为大端。 */
#define AGCORE_FRAGMENT_HEADER_LEN 4

typedef struct {
    uint8_t *buf;
    uint16_t total_len;
    uint16_t received_len;
} agcore_fragment_state_t;

typedef struct {
    atomic_uint_least16_t rx_sn;
    atomic_uchar tx_sn;
    agcore_fragment_state_t fragment;
} agcore_data_protocol_state_t;

typedef enum {
    AGCORE_FRAGMENT_ERROR = -1,
    AGCORE_FRAGMENT_MORE,
    AGCORE_FRAGMENT_COMPLETE,
    AGCORE_FRAGMENT_NO_MEMORY,
} agcore_fragment_result;

/**
 * @brief 获取链路的协议状态；重组状态只能由该链路的串行接收上下文操作
 * @param link 数据链路
 * @return 状态指针，无效链路返回 NULL
 */
agcore_data_protocol_state_t *agcore_data_protocol_state_get(agcore_link link);

/**
 * @brief 释放未转移的重组缓冲区并清空状态，可重复调用
 * @param state 重组状态
 */
void agcore_fragment_reset(agcore_fragment_state_t *state);

/**
 * @brief 接收一片不透明消息；错误时清理重组，完成后缓冲区仍由 state 持有
 * @param state 调用方独立持有的重组状态，首次使用前需清零
 * @param buf 含分包头的完整片段
 * @param len 片段长度
 * @param max_len 允许的完整消息长度上限
 * @return 未完成、完成、格式错误或内存不足
 */
agcore_fragment_result agcore_fragment_decode(agcore_fragment_state_t *state,
                                             const uint8_t *buf, uint16_t len,
                                             uint16_t max_len);

/**
 * @brief 写入大端分包头，参数合法性由调用方保证
 * @param header 至少 4 字节的输出缓冲区
 * @param total_len 完整消息长度
 * @param offset 本片数据偏移
 */
void agcore_fragment_encode(uint8_t *header, uint16_t total_len, uint16_t offset);

/**
 * @brief 封装AGCORE顶层协议帧
 *
 * @param data 输入统一数据结构，payload 为不透明字节串
 * @param buf 输出完整顶层协议包: Header + Ver + SN + PayloadLen + Payload + CheckSum
 * @return 完整顶层协议包长度，失败返回 0
 */
uint16_t agcore_rawdata_encode(const agcore_data_t *data, uint8_t *buf);

/**
 * @brief 解析AGCORE原始顶层协议帧
 *
 * @param buf 输入完整顶层协议包
 * @param buf_len 完整顶层协议包长度
 * @param data 输出统一数据结构；调用前需要先设置data->link；payload 借用 buf
 * @return 顶层协议包长度，失败返回 0
 */
uint16_t agcore_rawdata_decode(uint8_t *buf, uint16_t buf_len, agcore_data_t *data);

#endif /* __AGCORE_DATA_PROTOCOL_H__ */
