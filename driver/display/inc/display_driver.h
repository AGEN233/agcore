#ifndef __DISPLAY_DRIVER_H__
#define __DISPLAY_DRIVER_H__

#include "sdkconfig.h"
#include "stdbool.h"
#include "stdint.h"
#include "esp_err.h"

#if defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7735)
#define DISPLAY_DRIVER_WIDTH  CONFIG_DISPLAY_ST7735_WIDTH
#define DISPLAY_DRIVER_HEIGHT CONFIG_DISPLAY_ST7735_HEIGHT
#elif defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7789)
#define DISPLAY_DRIVER_WIDTH  CONFIG_DISPLAY_ST7789_WIDTH
#define DISPLAY_DRIVER_HEIGHT CONFIG_DISPLAY_ST7789_HEIGHT
#endif

typedef void (*display_driver_flush_done_cb_t)(void);

typedef enum {
    DISPLAY_DRIVER_ROTATION_0 = 0,
    DISPLAY_DRIVER_ROTATION_90,
    DISPLAY_DRIVER_ROTATION_180,
    DISPLAY_DRIVER_ROTATION_270,
} display_driver_rotation;

esp_err_t display_driver_init(display_driver_flush_done_cb_t cb);
esp_err_t display_driver_set_backlight(bool enabled);
esp_err_t display_driver_set_rotation(display_driver_rotation rotation);
display_driver_rotation display_driver_get_rotation(void);

uint16_t display_driver_get_width(void);
uint16_t display_driver_get_height(void);
esp_err_t display_driver_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const void *rgb565_data);
esp_err_t display_driver_test(void);

#endif /* __DISPLAY_DRIVER_H__ */
