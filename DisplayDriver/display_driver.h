#ifndef __DISPLAY_DRIVER_H__
#define __DISPLAY_DRIVER_H__
#include "sdkconfig.h"

#if defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7735)
#include "display7735/display7735.h"
#endif

#endif
