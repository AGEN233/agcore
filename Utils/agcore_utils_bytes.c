#include "agcore_utils_bytes.h"

/**
 * @brief 将16位数值写入2字节大端缓冲
 * @param buf 输出缓冲(至少2字节)
 * @param val 待写入的16位数值
 */
void agcore_put_bytes16(uint8_t *buf, uint16_t val)
{
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)val;
}

/**
 * @brief 将32位数值写入4字节大端缓冲
 * @param buf 输出缓冲(至少4字节)
 * @param val 待写入的32位数值
 */
void agcore_put_bytes32(uint8_t *buf, uint32_t val)
{
    buf[0] = (uint8_t)(val >> 24);
    buf[1] = (uint8_t)(val >> 16);
    buf[2] = (uint8_t)(val >> 8);
    buf[3] = (uint8_t)val;
}

/**
 * @brief 从2字节大端缓冲读取16位数值
 * @param buf 输入缓冲(至少2字节)
 * @return 读取到的16位数值
 */
uint16_t agcore_get_bytes16(const uint8_t *buf)
{
    return (uint16_t)(((uint16_t)buf[0] << 8) | (uint16_t)buf[1]);
}

/**
 * @brief 从4字节大端缓冲读取32位数值
 * @param buf 输入缓冲(至少4字节)
 * @return 读取到的32位数值
 */
uint32_t agcore_get_bytes32(const uint8_t *buf)
{
    return ((uint32_t)buf[0] << 24) |
           ((uint32_t)buf[1] << 16) |
           ((uint32_t)buf[2] << 8)  |
           ((uint32_t)buf[3]);
}
