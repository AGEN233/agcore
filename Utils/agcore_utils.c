#include "agcore_utils.h"

/**
 * @brief 计算8位累加校验和
 * @param data 数据指针
 * @param len 数据长度
 * @return uint8_t 8位校验和
 */
uint8_t agcore_checksum8_calc(const uint8_t *data, uint16_t len)
{
    uint8_t checksum = 0;

    for (uint16_t i = 0; i < len; i++) {
        checksum += data[i];
    }

    return checksum;
}

/**
 * @brief 计算16位累加校验和
 * @param data 数据指针
 * @param len 数据长度
 * @return uint16_t 16位校验和
 */
uint16_t agcore_checksum16_calc(const uint8_t *data, uint16_t len)
{
    uint16_t checksum = 0;

    for (uint16_t i = 0; i < len; i++) {
        checksum += data[i];
    }

    return (uint16_t)checksum;
}
