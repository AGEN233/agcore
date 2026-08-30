#include "agcore_check.h"

#include "esp_rom_crc.h"

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

/**
 * @brief 计算 CRC16 校验值
 * @param data 数据指针
 * @param len 数据长度
 * @return uint16_t CRC16 校验值
 */
uint16_t agcore_crc16_calc(const uint8_t *data, uint16_t len)
{
    return (uint16_t)~esp_rom_crc16_be((uint16_t)~0xFFFFU, data, len);
}
