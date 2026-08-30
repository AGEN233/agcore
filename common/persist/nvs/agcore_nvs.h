#ifndef __AGCORE_NVS_H__
#define __AGCORE_NVS_H__

#include <stddef.h>

#include "esp_err.h"

esp_err_t agcore_coredata_read(const char *key, void *buf, size_t len);
esp_err_t agcore_coredata_write(const char *key, const void *buf, size_t len);
esp_err_t agcore_coredata_erase(const char *key);

esp_err_t agcore_userdata_read(const char *key, void *buf, size_t len);
esp_err_t agcore_userdata_write(const char *key, const void *buf, size_t len);
esp_err_t agcore_userdata_erase(const char *key);

#endif /* __AGCORE_NVS_H__ */
