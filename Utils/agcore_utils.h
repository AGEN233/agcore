#ifndef __AGCORE_UTILS_H__
#define __AGCORE_UTILS_H__

#include "stdint.h"

/**
 * @brief checksum
 */
uint8_t agcore_checksum8_calc(const uint8_t *data, uint16_t len);
uint16_t agcore_checksum16_calc(const uint8_t *data, uint16_t len);
#endif