#ifndef __AGCORE_CORE_PRINTF_LOG_H__
#define __AGCORE_CORE_PRINTF_LOG_H__

#include "agcore_port.h"
#include "sdkconfig.h"

#define CORELOG_LEVEL_NONE      (0)
#define CORELOG_LEVEL_ERROR     (1)
#define CORELOG_LEVEL_WARN      (2)
#define CORELOG_LEVEL_INFO      (3)
#define CORELOG_LEVEL_DEBUG     (4)

#ifndef CONFIG_AGCORE_CORELOG_LEVEL
#define CONFIG_AGCORE_CORELOG_LEVEL CORELOG_LEVEL_WARN
#endif

#ifndef AGCORE_CORELOG_LEVEL
#define AGCORE_CORELOG_LEVEL CONFIG_AGCORE_CORELOG_LEVEL
#endif

#define core_printf(...)        AGCORE_LOG_PORT(__VA_ARGS__)

#if (AGCORE_CORELOG_LEVEL >= CORELOG_LEVEL_ERROR)
#define CORE_LOGE(tag, fmt, ...) \
    AGCORE_LOG_PORT("\033[35m[E][CORE]%s: " fmt "\033[0m\r\n", tag, ##__VA_ARGS__)
#else
#define CORE_LOGE(...)
#endif

#if (AGCORE_CORELOG_LEVEL >= CORELOG_LEVEL_WARN)
#define CORE_LOGW(tag, fmt, ...) \
    AGCORE_LOG_PORT("\033[34m[W][CORE]%s: " fmt "\033[0m\r\n", tag, ##__VA_ARGS__)
#else
#define CORE_LOGW(...)
#endif

#if (AGCORE_CORELOG_LEVEL >= CORELOG_LEVEL_INFO)
#define CORE_LOGI(tag, fmt, ...) \
    AGCORE_LOG_PORT("\033[36m[I][CORE]%s: " fmt "\033[0m\r\n", tag, ##__VA_ARGS__)
#else
#define CORE_LOGI(...)
#endif

#if (AGCORE_CORELOG_LEVEL >= CORELOG_LEVEL_DEBUG)
#define CORE_LOGD(tag, fmt, ...) \
    AGCORE_LOG_PORT("[D][CORE]%s:  " fmt "\r\n", tag, ##__VA_ARGS__)
#else
#define CORE_LOGD(...)
#endif

#endif /* AGCORE_CORE_PRINTF_LOG_H */
