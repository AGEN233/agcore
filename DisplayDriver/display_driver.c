#include "display_driver.h"

#if defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7735)

#include "display7735/display7735.h"

esp_err_t display_driver_init(display_driver_flush_done_cb_t cb)
{
    return display_st7735_init((display_st7735_flush_done_cb_t)cb);
}

esp_err_t display_driver_set_backlight(bool enabled)
{
    return display_st7735_set_backlight(enabled);
}

esp_err_t display_driver_set_rotation(display_driver_rotation_et rotation)
{
    return display_st7735_set_rotation((display_st7735_rotation_et)rotation);
}

display_driver_rotation_et display_driver_get_rotation(void)
{
    return (display_driver_rotation_et)display_st7735_get_rotation();
}

uint16_t display_driver_get_width(void)
{
    return display_st7735_get_width();
}

uint16_t display_driver_get_height(void)
{
    return display_st7735_get_height();
}

esp_err_t display_driver_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const void *rgb565_data)
{
    return display_st7735_flush(x_start, y_start, x_end, y_end, rgb565_data);
}

esp_err_t display_driver_test(void)
{
    return display_st7735_test();
}

#elif defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7789)

#include "display7789/display7789.h"

esp_err_t display_driver_init(display_driver_flush_done_cb_t cb)
{
    return display_st7789_init((display_st7789_flush_done_cb_t)cb);
}

esp_err_t display_driver_set_backlight(bool enabled)
{
    return display_st7789_set_backlight(enabled);
}

esp_err_t display_driver_set_rotation(display_driver_rotation_et rotation)
{
    return display_st7789_set_rotation((display_st7789_rotation_et)rotation);
}

display_driver_rotation_et display_driver_get_rotation(void)
{
    return (display_driver_rotation_et)display_st7789_get_rotation();
}

uint16_t display_driver_get_width(void)
{
    return display_st7789_get_width();
}

uint16_t display_driver_get_height(void)
{
    return display_st7789_get_height();
}

esp_err_t display_driver_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const void *rgb565_data)
{
    return display_st7789_flush(x_start, y_start, x_end, y_end, rgb565_data);
}

esp_err_t display_driver_test(void)
{
    return display_st7789_test();
}

#endif
