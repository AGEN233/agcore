#ifndef __PIXEL_DRIVER_H__
#define __PIXEL_DRIVER_H__

#include "stdint.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "agcore_printf_log.h"

void pixel_drv_set_led_len(uint16_t len);
void pixel_drv_sendata(const uint8_t *color_data, uint16_t len);
void pixel_driver_init(void);

#endif
