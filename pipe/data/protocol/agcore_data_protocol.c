#include "agcore_data_protocol.h"

#include <stdbool.h>
#include <stdatomic.h>
#include <string.h>

#define TAG "AGCORE_DATA_ENCODE"

#define AGCORE_DATA_TOP_HEADER1       0x0A
#define AGCORE_DATA_TOP_HEADER2       0x1B
#define AGCORE_DATA_TOP_HEADER3       0x2C
#define AGCORE_DATA_TOP_HEADER4       0x3D
#define AGCORE_DATA_TOP_VERSION       0x00
#define AGCORE_DATA_TOP_MAGIC_LEN     4
#define AGCORE_DATA_TOP_HEAD_LEN      8
#define AGCORE_DATA_TOP_CHECKSUM_LEN  1
#define AGCORE_DATA_TOP_OVERHEAD      (AGCORE_DATA_TOP_HEAD_LEN + AGCORE_DATA_TOP_CHECKSUM_LEN)

/**
 * @brief 重置指定链路的收发状态(清空序号计数)
 * @param link 链路
 */
void agcore_data_link_reset(agcore_link link)
{
    agcore_data_protocol_state_t *state = agcore_data_protocol_state_get(link);
    if (state == NULL) {
        CORE_LOGD(TAG, "reset invalid link|link=%d", link);
        return;
    }

    atomic_store(&state->rx_sn, 0);
    atomic_store(&state->tx_sn, 0);
    CORE_LOGD(TAG, "reset link|link=%d", link);
}

/**
 * @brief 在缓冲区中查找顶层协议帧头
 * @param buf 数据缓冲
 * @param buf_len 缓冲长度
 * @return 帧头位置, 未找到返回 UINT16_MAX
 */
static uint16_t agcore_data_top_find_header(const uint8_t *buf, uint16_t buf_len)
{
    if (buf == NULL || buf_len < AGCORE_DATA_TOP_MAGIC_LEN) {
        return UINT16_MAX;
    }

    for (uint16_t i = 0; i <= buf_len - AGCORE_DATA_TOP_MAGIC_LEN; i++) {
        if (buf[i] == AGCORE_DATA_TOP_HEADER1 &&
                buf[i + 1] == AGCORE_DATA_TOP_HEADER2 &&
                buf[i + 2] == AGCORE_DATA_TOP_HEADER3 &&
                buf[i + 3] == AGCORE_DATA_TOP_HEADER4) {
            return i;
        }
    }

    return UINT16_MAX;
}

/**
 * @brief agcore_data_t -> 顶层协议包
 */
uint16_t agcore_rawdata_encode(const agcore_data_t *data, uint8_t *buf)
{
    agcore_data_protocol_state_t *state;
    uint16_t top_payload_len;
    uint16_t packet_len;

    if (data == NULL || buf == NULL) {
        CORE_LOGD(TAG, "encode invalid arg");
        return 0;
    }

    state = agcore_data_protocol_state_get(data->link);
    if (state == NULL) {
        CORE_LOGD(TAG, "encode invalid link|link=%d", data->link);
        return 0;
    }

    if (data->payload_len > AGCORE_RAWDATA_PAYLOAD_MAX_LEN ||
            (data->payload_len > 0 && data->payload == NULL)) {
        CORE_LOGD(TAG, "encode payload invalid|link=%d len=%u", data->link, data->payload_len);
        return 0;
    }

    top_payload_len = data->payload_len;
    packet_len = AGCORE_DATA_TOP_OVERHEAD + top_payload_len;
    buf[0] = AGCORE_DATA_TOP_HEADER1;
    buf[1] = AGCORE_DATA_TOP_HEADER2;
    buf[2] = AGCORE_DATA_TOP_HEADER3;
    buf[3] = AGCORE_DATA_TOP_HEADER4;
    buf[4] = AGCORE_DATA_TOP_VERSION;
    buf[5] = state->tx_sn++;

    agcore_put_bytes16(&buf[6], top_payload_len);
    if (data->payload_len > 0) {
        memcpy(&buf[AGCORE_DATA_TOP_HEAD_LEN], data->payload, data->payload_len);
    }

    buf[packet_len - 1] = agcore_checksum8_calc(buf, packet_len - AGCORE_DATA_TOP_CHECKSUM_LEN);
    CORE_LOGD(TAG, "encode ok|link=%d sn=%02X payload=%u packet=%u checksum=%02X", data->link, buf[5], data->payload_len, packet_len, buf[packet_len - 1]);

    return packet_len;
}

/**
 * @brief 原始顶层协议包 -> agcore_data_t
 * @param buf 可写的原始帧缓冲区
 * @param buf_len 原始帧长度
 * @param data 输出数据，调用前需设置链路
 * @return 成功解析的协议包长度，失败返回 0
 */
uint16_t agcore_rawdata_decode(uint8_t *buf, uint16_t buf_len, agcore_data_t *data)
{
    agcore_data_protocol_state_t *state;
    uint16_t header_pos;
    uint16_t top_payload_len;
    uint16_t packet_len;
    uint8_t checksum;
    uint8_t sn;

    if (buf == NULL || data == NULL) {
        CORE_LOGD(TAG, "decode invalid arg");
        return 0;
    }

    state = agcore_data_protocol_state_get(data->link);
    if (state == NULL) {
        CORE_LOGD(TAG, "decode invalid link|link=%d len=%u", data->link, buf_len);
        return 0;
    }

    header_pos = agcore_data_top_find_header(buf, buf_len);
    if (header_pos == UINT16_MAX) {
        CORE_LOGD(TAG, "decode no header|link=%d len=%u", data->link, buf_len);
        return 0;
    }

    if (buf_len - header_pos < AGCORE_DATA_TOP_OVERHEAD) {
        CORE_LOGD(TAG, "decode short head|link=%d pos=%u len=%u", data->link, header_pos, buf_len);
        return 0;
    }

    if (buf[header_pos + 4] != AGCORE_DATA_TOP_VERSION) {
        CORE_LOGD(TAG, "decode version mismatch|link=%d pos=%u ver=%02X", data->link, header_pos, buf[header_pos + 4]);
        return 0;
    }

    top_payload_len = agcore_get_bytes16(&buf[header_pos + 6]);
    if (top_payload_len > AGCORE_RAWDATA_PAYLOAD_MAX_LEN) {
        return 0;
    }
    packet_len = AGCORE_DATA_TOP_OVERHEAD + top_payload_len;
    if (buf_len - header_pos < packet_len) {
        CORE_LOGD(TAG, "decode packet incomplete|link=%d pos=%u len=%u need=%u", data->link, header_pos, buf_len, packet_len);
        return 0;
    }

    checksum = agcore_checksum8_calc(&buf[header_pos], packet_len - AGCORE_DATA_TOP_CHECKSUM_LEN);
    if (checksum != buf[header_pos + packet_len - 1]) {
        CORE_LOGD(TAG, "decode checksum mismatch|link=%d pos=%u calc=%02X recv=%02X packet=%u",
                  data->link, header_pos, checksum, buf[header_pos + packet_len - 1], packet_len);
        return 0;
    }

    sn = buf[header_pos + 5];

    if (sn != 0 && atomic_load(&state->rx_sn) == (uint16_t)(0x100 | sn)) {
        return 0;
    }

    if (sn != 0) {
        atomic_store(&state->rx_sn, (uint16_t)(0x100 | sn));
    }

    data->sn = sn;
    data->payload_len = top_payload_len;
    data->payload = top_payload_len > 0 ? &buf[header_pos + AGCORE_DATA_TOP_HEAD_LEN] : NULL;

    return packet_len;
}

/**
 * @brief 释放重组缓冲区并清空状态
 * @param state 重组状态
 */
void agcore_fragment_reset(agcore_fragment_state_t *state)
{
    if (state != NULL) {
        agcore_free(state->buf);
        memset(state, 0, sizeof(*state));
    }
}

/**
 * @brief 校验并追加一个分片，完成后缓冲区仍归状态持有
 * @param state 重组状态
 * @param buf 分包头及数据
 * @param len 片段长度
 * @param max_len 完整消息长度上限
 * @return 重组结果
 */
agcore_fragment_result agcore_fragment_decode(agcore_fragment_state_t *state,
                                             const uint8_t *buf, uint16_t len,
                                             uint16_t max_len)
{
    if (state == NULL) {
        return AGCORE_FRAGMENT_ERROR;
    }
    if (buf == NULL || len <= AGCORE_FRAGMENT_HEADER_LEN) {
        agcore_fragment_reset(state);
        return AGCORE_FRAGMENT_ERROR;
    }
    uint16_t total_len = agcore_get_bytes16(buf);
    uint16_t offset = agcore_get_bytes16(buf + 2);
    uint16_t chunk_len = len - AGCORE_FRAGMENT_HEADER_LEN;
    if (total_len == 0 || total_len > max_len || offset >= total_len ||
            chunk_len > total_len - offset) {
        agcore_fragment_reset(state);
        return AGCORE_FRAGMENT_ERROR;
    }
    if (offset == 0) {
        agcore_fragment_reset(state);
        state->buf = agcore_malloc(total_len);
        if (state->buf == NULL) {
            return AGCORE_FRAGMENT_NO_MEMORY;
        }
        state->total_len = total_len;
    }
    if (state->buf == NULL || state->total_len != total_len ||
            state->received_len != offset) {
        agcore_fragment_reset(state);
        return AGCORE_FRAGMENT_ERROR;
    }
    memcpy(state->buf + offset, buf + AGCORE_FRAGMENT_HEADER_LEN, chunk_len);
    state->received_len += chunk_len;
    return state->received_len == total_len ? AGCORE_FRAGMENT_COMPLETE : AGCORE_FRAGMENT_MORE;
}

/**
 * @brief 编码大端分包头
 * @param header 4 字节输出缓冲区
 * @param total_len 完整消息长度
 * @param offset 当前偏移
 * @param chunk_len 本片数据长度
 * @return 参数是否合法
 */
bool agcore_fragment_encode(uint8_t *header, uint16_t total_len,
                           uint16_t offset, uint16_t chunk_len)
{
    if (header == NULL || total_len == 0 || offset >= total_len ||
            chunk_len == 0 || chunk_len > total_len - offset) {
        return false;
    }
    agcore_put_bytes16(header, total_len);
    agcore_put_bytes16(header + 2, offset);
    return true;
}
