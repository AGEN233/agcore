#include "display_driver.h"

#if defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7735)

#include "display_st7735.h"

/**
 * @brief 显示驱动初始化(ST7735)
 * @param cb 刷屏发送完成回调
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_init(display_driver_flush_done_cb_t cb)
{
    return display_st7735_init((display_st7735_flush_done_cb_t)cb);
}

/**
 * @brief 设置背光
 * @param enabled true 开背光
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_set_backlight(bool enabled)
{
    return display_st7735_set_backlight(enabled);
}

/**
 * @brief 设置屏幕旋转方向
 * @param rotation 目标方向
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_set_rotation(display_driver_rotation rotation)
{
    return display_st7735_set_rotation((display_st7735_rotation)rotation);
}

/**
 * @brief 获取当前旋转方向
 * @return 旋转方向
 */
display_driver_rotation display_driver_get_rotation(void)
{
    return (display_driver_rotation)display_st7735_get_rotation();
}

/**
 * @brief 获取屏幕宽度
 * @return 宽度(像素)
 */
uint16_t display_driver_get_width(void)
{
    return display_st7735_get_width();
}

/**
 * @brief 获取屏幕高度
 * @return 高度(像素)
 */
uint16_t display_driver_get_height(void)
{
    return display_st7735_get_height();
}

/**
 * @brief 刷新一块 RGB565 像素区域到屏幕(左闭右开坐标)
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const void *rgb565_data)
{
    return display_st7735_flush(x_start, y_start, x_end, y_end, rgb565_data);
}

/**
 * @brief 启动显示测试模式
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_test(void)
{
    return display_st7735_test();
}

#elif defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7789)

#include "display_st7789.h"

/**
 * @brief 显示驱动初始化(ST7789)
 * @param cb 刷屏发送完成回调
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_init(display_driver_flush_done_cb_t cb)
{
    return display_st7789_init((display_st7789_flush_done_cb_t)cb);
}

/**
 * @brief 设置背光
 * @param enabled true 开背光
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_set_backlight(bool enabled)
{
    return display_st7789_set_backlight(enabled);
}

/**
 * @brief 设置屏幕旋转方向
 * @param rotation 目标方向
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_set_rotation(display_driver_rotation rotation)
{
    return display_st7789_set_rotation((display_st7789_rotation)rotation);
}

/**
 * @brief 获取当前旋转方向
 * @return 旋转方向
 */
display_driver_rotation display_driver_get_rotation(void)
{
    return (display_driver_rotation)display_st7789_get_rotation();
}

/**
 * @brief 获取屏幕宽度
 * @return 宽度(像素)
 */
uint16_t display_driver_get_width(void)
{
    return display_st7789_get_width();
}

/**
 * @brief 获取屏幕高度
 * @return 高度(像素)
 */
uint16_t display_driver_get_height(void)
{
    return display_st7789_get_height();
}

/**
 * @brief 刷新一块 RGB565 像素区域到屏幕(左闭右开坐标)
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const void *rgb565_data)
{
    return display_st7789_flush(x_start, y_start, x_end, y_end, rgb565_data);
}

/**
 * @brief 启动显示测试模式
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t display_driver_test(void)
{
    return display_st7789_test();
}

#endif
