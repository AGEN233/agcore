#ifndef __AGCORE_CHECK_H__
#define __AGCORE_CHECK_H__

#include "stdint.h"

/**
 * @brief checksum
 */
uint8_t agcore_checksum8_calc(const uint8_t *data, uint16_t len);
uint16_t agcore_checksum16_calc(const uint8_t *data, uint16_t len);
uint16_t agcore_crc16_calc(uint16_t seed, const uint8_t *data, uint16_t len);
uint32_t agcore_crc32_calc(uint32_t seed, const uint8_t *data, uint32_t len);

#endif /* __AGCORE_CHECK_H__ */
