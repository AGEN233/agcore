#ifndef __DISPLAY7789_H__
#define __DISPLAY7789_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sdkconfig.h"

#if defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7789)

#include "stdbool.h"
#include "stdint.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef void (*display_st7789_flush_done_cb_t)(void);

typedef enum {
    DISPLAY_ST7789_ROTATION_0 = 0,
    DISPLAY_ST7789_ROTATION_90,
    DISPLAY_ST7789_ROTATION_180,
    DISPLAY_ST7789_ROTATION_270,
} display_st7789_rotation;

typedef struct {
    uint16_t width;
    uint16_t height;
    display_st7789_rotation rotation;

    bool initialized;
    display_st7789_flush_done_cb_t flush_done_cb;

    esp_lcd_panel_io_handle_t panel_io;
    TaskHandle_t test_task_handle;
} display_st7789_runtime_t;

esp_err_t display_st7789_init(display_st7789_flush_done_cb_t cb);
esp_err_t display_st7789_set_backlight(bool enabled);
esp_err_t display_st7789_set_rotation(display_st7789_rotation rotation);
display_st7789_rotation display_st7789_get_rotation(void);

uint16_t display_st7789_get_width(void);
uint16_t display_st7789_get_height(void);
esp_err_t display_st7789_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const void *rgb565_data);
esp_err_t display_st7789_test(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY7789_H__ */
