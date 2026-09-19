#ifndef __AGCORE_LOG_ADAPTER_H__
#define __AGCORE_LOG_ADAPTER_H__

#include "stddef.h"
#include <elog.h>
#include "agcore_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_AGCORE_CORELOG_LEVEL
#define CONFIG_AGCORE_CORELOG_LEVEL 2
#endif
#ifndef CONFIG_AGCORE_APPLOG_LEVEL
#define CONFIG_AGCORE_APPLOG_LEVEL 2
#endif

#if (CONFIG_AGCORE_CORELOG_LEVEL >= 1)
#define CORE_LOGE(tag, fmt, ...) elog_error("CORE:" tag, fmt, ##__VA_ARGS__)
#else
#define CORE_LOGE(...) ((void)0)
#endif

#if (CONFIG_AGCORE_CORELOG_LEVEL >= 2)
#define CORE_LOGW(tag, fmt, ...) elog_warn("CORE:" tag, fmt, ##__VA_ARGS__)
#else
#define CORE_LOGW(...) ((void)0)
#endif

#if (CONFIG_AGCORE_CORELOG_LEVEL >= 3)
#define CORE_LOGI(tag, fmt, ...) elog_info("CORE:" tag, fmt, ##__VA_ARGS__)
#else
#define CORE_LOGI(...) ((void)0)
#endif

#if (CONFIG_AGCORE_CORELOG_LEVEL >= 4)
#define CORE_LOGD(tag, fmt, ...) elog_debug("CORE:" tag, fmt, ##__VA_ARGS__)
#else
#define CORE_LOGD(...) ((void)0)
#endif

#if (CONFIG_AGCORE_APPLOG_LEVEL >= 1)
#define APP_LOGE(tag, fmt, ...) elog_error("APP:" tag, fmt, ##__VA_ARGS__)
#else
#define APP_LOGE(...) ((void)0)
#endif

#if (CONFIG_AGCORE_APPLOG_LEVEL >= 2)
#define APP_LOGW(tag, fmt, ...) elog_warn("APP:" tag, fmt, ##__VA_ARGS__)
#else
#define APP_LOGW(...) ((void)0)
#endif

#if (CONFIG_AGCORE_APPLOG_LEVEL >= 3)
#define APP_LOGI(tag, fmt, ...) elog_info("APP:" tag, fmt, ##__VA_ARGS__)
#else
#define APP_LOGI(...) ((void)0)
#endif

#if (CONFIG_AGCORE_APPLOG_LEVEL >= 4)
#define APP_LOGD(tag, fmt, ...) elog_debug("APP:" tag, fmt, ##__VA_ARGS__)
#else
#define APP_LOGD(...) ((void)0)
#endif

void agcore_log_init(void);
void agcore_log_port_output(const char *log, size_t size);
const char *agcore_log_port_get_time(void);
void agcore_log_port_lock(void);
void agcore_log_port_unlock(void);
void agcore_log_async_notice(void);

#ifdef __cplusplus
}
#endif

#endif /* __AGCORE_LOG_ADAPTER_H__ */
