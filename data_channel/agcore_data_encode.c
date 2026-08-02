#include "agcore_data_encode.h"

#include <stdbool.h>
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
#define AGCORE_DATA_CMD_LEN           2

typedef struct {
    bool rx_sn_valid;
    uint8_t rx_last_sn;
    uint8_t tx_sn;
} agcore_data_link_state_st;

static agcore_data_link_state_st s_agcore_data_link_state[AGCORE_DATA_LINK_MAX];

static agcore_data_link_state_st *agcore_data_link_state_get(agcore_data_link_et link)
{
    if (link <= AGCORE_DATA_LINK_NONE || link >= AGCORE_DATA_LINK_MAX) {
        return NULL;
    }

    return &s_agcore_data_link_state[link];
}

void agcore_data_link_reset(agcore_data_link_et link)
{
    agcore_data_link_state_st *state = agcore_data_link_state_get(link);
    if (state == NULL) {
        CORE_LOGD(TAG, "reset invalid link|link=%d", link);
        return;
    }

    memset(state, 0, sizeof(*state));
    CORE_LOGD(TAG, "reset link|link=%d", link);
}

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
 * @brief agcore_data_st -> 顶层协议包
 */
uint16_t agcore_data_encode(agcore_data_st *data, uint8_t *buf)
{
    agcore_data_link_state_st *state;
    uint16_t top_payload_len;
    uint16_t packet_len;

    if (data == NULL || buf == NULL) {
        CORE_LOGD(TAG, "encode invalid arg");
        return 0;
    }

    state = agcore_data_link_state_get(data->link);
    if (state == NULL) {
        CORE_LOGD(TAG, "encode invalid link|link=%d", data->link);
        return 0;
    }

    if (data->payload_len > AGCORE_DATA_CHANNEL_PAYLOAD_MAX) {
        CORE_LOGD(TAG, "encode payload too long|link=%d cmd=%04X len=%u", data->link, data->cmd, data->payload_len);
        return 0;
    }

    top_payload_len = AGCORE_DATA_CMD_LEN + data->payload_len;
    packet_len = AGCORE_DATA_TOP_OVERHEAD + top_payload_len;

    data->sn = state->tx_sn++;

    buf[0] = AGCORE_DATA_TOP_HEADER1;
    buf[1] = AGCORE_DATA_TOP_HEADER2;
    buf[2] = AGCORE_DATA_TOP_HEADER3;
    buf[3] = AGCORE_DATA_TOP_HEADER4;
    buf[4] = AGCORE_DATA_TOP_VERSION;
    buf[5] = data->sn;
    agcore_put_bytes16(&buf[6], top_payload_len);
    agcore_put_bytes16(&buf[8], data->cmd);

    if (data->payload_len > 0) {
        memcpy(&buf[AGCORE_DATA_TOP_HEAD_LEN + AGCORE_DATA_CMD_LEN], data->payload, data->payload_len);
    }

    buf[packet_len - 1] = agcore_checksum8_calc(buf, packet_len - AGCORE_DATA_TOP_CHECKSUM_LEN);
    CORE_LOGD(TAG, "encode ok|link=%d sn=%02X cmd=%04X payload=%u packet=%u checksum=%02X",
              data->link, data->sn, data->cmd, data->payload_len, packet_len, buf[packet_len - 1]);

    return packet_len;
}

/**
 * @brief 顶层协议包 -> agcore_data_st
 */
uint16_t agcore_data_decode(const uint8_t *buf, uint16_t buf_len, agcore_data_st *data)
{
    agcore_data_link_state_st *state;
    uint16_t header_pos;
    uint16_t top_payload_len;
    uint16_t app_payload_len;
    uint16_t packet_len;
    uint8_t checksum;
    uint8_t sn;

    if (buf == NULL || data == NULL) {
        CORE_LOGD(TAG, "decode invalid arg");
        return 0;
    }

    state = agcore_data_link_state_get(data->link);
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
    if (top_payload_len < AGCORE_DATA_CMD_LEN) {
        CORE_LOGD(TAG, "decode payload too short|link=%d pos=%u top_payload=%u", data->link, header_pos, top_payload_len);
        return 0;
    }

    app_payload_len = top_payload_len - AGCORE_DATA_CMD_LEN;
    if (app_payload_len > AGCORE_DATA_CHANNEL_PAYLOAD_MAX) {
        CORE_LOGD(TAG, "decode payload too long|link=%d pos=%u app_payload=%u", data->link, header_pos, app_payload_len);
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
    if (sn != 0 && state->rx_sn_valid && state->rx_last_sn == sn) {
        uint16_t cmd = agcore_get_bytes16(&buf[header_pos + 8]);
        CORE_LOGD(TAG, "decode duplicate sn|link=%d sn=%02X cmd=%04X payload=%u packet=%u", data->link, sn, cmd, app_payload_len, packet_len);
        return 0;
    }

    if (sn != 0) {
        state->rx_last_sn = sn;
        state->rx_sn_valid = true;
    }

    data->sn = sn;
    data->cmd = agcore_get_bytes16(&buf[header_pos + 8]);
    data->payload_len = app_payload_len;

    if (app_payload_len > 0) {
        memcpy(data->payload, &buf[header_pos + AGCORE_DATA_TOP_HEAD_LEN + AGCORE_DATA_CMD_LEN], app_payload_len);
    }

    return packet_len;
}
