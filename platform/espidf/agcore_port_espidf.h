#ifndef __AGCORE_PORT_ESPIDF_H__
#define __AGCORE_PORT_ESPIDF_H__

/* FREERTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* ESPIDF 相关*/
#include "sdkconfig.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"

#define agcore_get_timems()         ((uint32_t)(esp_timer_get_time() / 1000ULL))
#define agcore_get_timeus()         ((uint64_t)esp_timer_get_time())

#define agcore_malloc(size)         heap_caps_malloc((size), MALLOC_CAP_DEFAULT)
#define agcore_malloc_pram(size)    agcore_malloc(size)
#define agcore_free(ptr)            do { if (ptr) { heap_caps_free((ptr)); } } while (0)

/**
 * @brief 下面是一些逻辑判断
 */
#ifdef CONFIG_SPIRAM
#undef agcore_malloc_pram
#define agcore_malloc_pram(size) heap_caps_malloc((size), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#endif

#endif /* __AGCORE_PORT_ESPIDF_H__ */
