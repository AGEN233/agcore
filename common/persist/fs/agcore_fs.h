#ifndef __AGCORE_FS_H__
#define __AGCORE_FS_H__

#include <stdbool.h>

#include "esp_err.h"

#define AGCORE_FS_BASE_PATH "/fs"

esp_err_t agcore_fs_mount(void);
esp_err_t agcore_fs_unmount(void);
bool agcore_fs_is_mounted(void);

#endif /* __AGCORE_FS_H__ */
