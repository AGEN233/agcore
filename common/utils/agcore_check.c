#include "agcore_check.h"

#ifdef ESP_PLATFORM
#include "esp_rom_crc.h"
#endif

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

    return checksum;
}

/**
 * @brief 计算 CRC16 校验值
 * @param seed CRC16 初始值
 * @param data 数据指针
 * @param len 数据长度
 * @return uint16_t CRC16 校验值
 */
uint16_t agcore_crc16_calc(uint16_t seed, const uint8_t *data, uint16_t len)
{
#ifdef ESP_PLATFORM
    {
        return (uint16_t)~esp_rom_crc16_be((uint16_t)~seed, data, len);
    }
#else
    {
        uint16_t crc = seed;

        while (len--)
        {
            crc ^= (uint16_t)(*data++) << 8;

            for (uint8_t i = 0; i < 8; i++) {
                if (crc & 0x8000U) {
                    crc = (crc << 1) ^ 0x1021U;
                } else {
                    crc <<= 1;
                }
            }
        }

        return crc;
    }
#endif
}

/**
 * @brief 计算 CRC32 校验值
 * @param seed CRC32 初始值
 * @param data 数据指针
 * @param len 数据长度
 * @return uint32_t CRC32 校验值
 */
uint32_t agcore_crc32_calc(uint32_t seed, const uint8_t *data, uint32_t len)
{
#ifdef ESP_PLATFORM
    {
        return ~esp_rom_crc32_le(~seed, data, len);
    }
#else
    {
        uint32_t crc = seed;

        while (len--)
        {
            crc ^= *data++;

            for (uint8_t i = 0; i < 8; i++) {
                if (crc & 0x01U) {
                    crc = (crc >> 1) ^ 0xEDB88320U;
                } else {
                    crc >>= 1;
                }
            }
        }

        return crc;
    }
#endif
}