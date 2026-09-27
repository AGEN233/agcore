#ifndef __AGCORE_PORT_H__
#define __AGCORE_PORT_H__

#define AGCORE_PLATFORM_IS_ESPIDF

/* C标准库 */
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "inttypes.h"

/* 平台适配层 */
#if defined(AGCORE_PLATFORM_IS_ESPIDF)
#include "agcore_port_espidf.h"
#else
#error "Unsupported AGCORE platform"
#endif

#endif /* AGCORE_PORT_H */
