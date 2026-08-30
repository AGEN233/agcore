#ifndef __AGCORE_H__
#define __AGCORE_H__

#define AGCORE_VERSION_MAJOR    0
#define AGCORE_VERSION_MINOR    7
#define AGCORE_VERSION_PATCH    2

#define AGCORE_VERSION       0x03

#include "agcore_public.h"
#include "agcore_data.h"
#include "agcore_version.h"
#include "agcore_log.h"
#include "agcore_lifecycle.h"
#include "agcore_nvs.h"
#include "agcore_flash.h"
#include "agcore_fs.h"
#include "agcore_ble.h"
#include "pixel_driver.h"
#include "display_driver.h"

void agcore_init(void);

#endif /* AGCORE_H */
