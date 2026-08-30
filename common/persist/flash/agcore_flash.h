#ifndef __AGCORE_FLASH_H__
#define __AGCORE_FLASH_H__

#include <stddef.h>

#include "esp_err.h"

esp_err_t agcore_flash_read(size_t offset, void *buf, size_t len);
esp_err_t agcore_flash_write(size_t offset, const void *buf, size_t len);
esp_err_t agcore_flash_erase(size_t offset, size_t len);

#endif /* __AGCORE_FLASH_H__ */
