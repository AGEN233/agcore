#ifndef __AGCORE_H__
#define __AGCORE_H__

#include "sdkconfig.h"
#include "agcore_port.h"

#include "agcore_printf_log.h"
#include "app_printf_log.h"
#include "agcore_datachannel.h"
#include "agcore_utils.h"

#define AGCORE_VERSION       0x01
#define AGCORE_ID            0x01
#define AGCORE_VRESION       AGCORE_VERSION

#include "agcore_version.h"
#include "pixel_driver.h"
#include "agcore_ble.h"

void agcore_init(void);

#endif /* AGCORE_H */
