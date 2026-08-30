#ifndef __AGCORE_LIFECYCLE_H__
#define __AGCORE_LIFECYCLE_H__

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

typedef enum {
    BOOT_NORMAL = 0,
    BOOT_BLANK,
    BOOT_FACTORY,
    BOOT_ABNORMAL,
} agcore_boot_reason;

esp_err_t agcore_lifecycle_init(void);
esp_err_t agcore_lifecycle_mark_stable(void);

agcore_boot_reason agcore_lifecycle_boot_reason_get(void);
bool agcore_lifecycle_is_factory_boot(void);
uint32_t agcore_lifecycle_boot_total_count_get(void);
uint32_t agcore_lifecycle_boot_streak_count_get(void);
uint32_t agcore_lifecycle_reset_reason_get(void);

#endif /* __AGCORE_LIFECYCLE_H__ */
