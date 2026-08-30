#ifndef __AGCORE_PORT_ESPIDF_H__
#define __AGCORE_PORT_ESPIDF_H__

#include <stdint.h>

#include "esp_heap_caps.h"
#include "esp_timer.h"

#define agcore_get_timems()     ((uint32_t)(esp_timer_get_time() / 1000ULL))
#define agcore_get_timeus()     ((uint64_t)esp_timer_get_time())

#define agcore_malloc(size)     heap_caps_malloc((size), MALLOC_CAP_DEFAULT)
#define agcore_free(ptr)        do { if (ptr) { heap_caps_free((ptr)); } } while (0)

#endif /* __AGCORE_PORT_ESPIDF_H__ */
