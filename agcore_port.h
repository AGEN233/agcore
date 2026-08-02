#ifndef __AGCORE_PORT_H__
#define __AGCORE_PORT_H__

#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "stdint.h"

#define AGCORE_PLATFORM_IS_ESPIDF

#define agcore_get_timems()     ((uint32_t)(esp_timer_get_time() / 1000ULL))
#define agcore_get_timeus()     ((uint64_t)esp_timer_get_time())

#define agcore_malloc(size)     heap_caps_malloc((size), MALLOC_CAP_DEFAULT)
#define agcore_free(ptr)        do { if (ptr) { heap_caps_free((ptr)); } } while(0)

#endif /* AGCORE_PORT_H */

