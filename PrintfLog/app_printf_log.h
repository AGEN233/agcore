#ifndef __AGCORE_APP_PRINTF_LOG_H__
#define __AGCORE_APP_PRINTF_LOG_H__

#include "agcore_port.h"
#include "sdkconfig.h"

#define APPLOG_LEVEL_NONE      (0)
#define APPLOG_LEVEL_ERROR     (1)
#define APPLOG_LEVEL_WARN      (2)
#define APPLOG_LEVEL_INFO      (3)
#define APPLOG_LEVEL_DEBUG     (4)

#ifndef CONFIG_AGCORE_APPLOG_LEVEL
#define CONFIG_AGCORE_APPLOG_LEVEL APPLOG_LEVEL_WARN
#endif

#ifndef AGCORE_APPLOG_LEVEL
#define AGCORE_APPLOG_LEVEL CONFIG_AGCORE_APPLOG_LEVEL
#endif

#define app_printf(...)        AGCORE_LOG_PORT(__VA_ARGS__)

#if (AGCORE_APPLOG_LEVEL >= APPLOG_LEVEL_ERROR)
#define APP_LOGE(tag, fmt, ...) \
    AGCORE_LOG_PORT("\033[31m[E][%s] " fmt "\033[0m\r\n", tag, ##__VA_ARGS__)
#else
#define APP_LOGE(...)
#endif

#if (AGCORE_APPLOG_LEVEL >= APPLOG_LEVEL_WARN)
#define APP_LOGW(tag, fmt, ...) \
    AGCORE_LOG_PORT("\033[33m[W][%s] " fmt "\033[0m\r\n", tag, ##__VA_ARGS__)
#else
#define APP_LOGW(...)
#endif

#if (AGCORE_APPLOG_LEVEL >= APPLOG_LEVEL_INFO)
#define APP_LOGI(tag, fmt, ...) \
    AGCORE_LOG_PORT("\033[32m[I][%s] " fmt "\033[0m\r\n", tag, ##__VA_ARGS__)
#else
#define APP_LOGI(...)
#endif

#if (AGCORE_APPLOG_LEVEL >= APPLOG_LEVEL_DEBUG)
#define APP_LOGD(tag, fmt, ...) \
    AGCORE_LOG_PORT("\033[36m[D][%s] " fmt "\033[0m\r\n", tag, ##__VA_ARGS__)
#else
#define APP_LOGD(...)
#endif

#endif /* AGCORE_APP_PRINTF_LOG_H */
