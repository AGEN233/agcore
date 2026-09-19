#ifndef __AGCORE_PORT_H__
#define __AGCORE_PORT_H__

#define AGCORE_PLATFORM_IS_ESPIDF

#if defined(AGCORE_PLATFORM_IS_ESPIDF)
#include "agcore_port_espidf.h"
#else
#error "Unsupported AGCORE platform"
#endif

#endif /* AGCORE_PORT_H */
