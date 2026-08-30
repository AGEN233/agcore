#ifndef __AGCORE_BYTES_H__
#define __AGCORE_BYTES_H__

#include "stdint.h"

/**
 * @brief 字节流工具:32/16 位数值与大端字节流互转
 */

/* u16 写入 2 字节(大端) */
void agcore_put_bytes16(uint8_t *buf, uint16_t val);

/* u32 写入 4 字节(大端) */
void agcore_put_bytes32(uint8_t *buf, uint32_t val);

/* 从 2 字节(大端)读 u16 */
uint16_t agcore_get_bytes16(const uint8_t *buf);

/* 从 4 字节(大端)读 u32 */
uint32_t agcore_get_bytes32(const uint8_t *buf);

#endif /* __AGCORE_BYTES_H__ */
