#ifndef __DISPLAY7735_H__
#define __DISPLAY7735_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "sdkconfig.h"

#if defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7735)
#include "stdbool.h"
#include "stdint.h"
#include "esp_err.h"

typedef void (*display_st7735_flush_done_cb_t)(void);
esp_err_t display_st7735_init(display_st7735_flush_done_cb_t cb);
esp_err_t display_st7735_set_backlight(bool enabled);
esp_err_t display_st7735_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const void *rgb565_data);
esp_err_t display_st7735_test(void);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __DISPLAY7735_H__ */
