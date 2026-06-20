#include "sdkconfig.h"

#if defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7735)

#include "display7735.h"

#include "agcore_printf_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "ST7735"

#ifdef CONFIG_DISPLAY_ST7735_SPI2_HOST
#define ST7735_SPI_HOST SPI2_HOST
#else
#define ST7735_SPI_HOST SPI3_HOST
#endif

#ifdef CONFIG_DISPLAY_ST7735_BACKLIGHT_ACTIVE_HIGH
#define ST7735_BACKLIGHT_ACTIVE_HIGH true
#else
#define ST7735_BACKLIGHT_ACTIVE_HIGH false
#endif

#ifdef CONFIG_DISPLAY_ST7735_INVERT_COLOR
#define ST7735_INVERT_COLOR true
#else
#define ST7735_INVERT_COLOR false
#endif

#ifdef CONFIG_DISPLAY_ST7735_BGR_ORDER
#define ST7735_BGR_ORDER true
#else
#define ST7735_BGR_ORDER false
#endif

static esp_lcd_panel_io_handle_t s_panel_io;
static bool s_initialized;
static TaskHandle_t s_test_task_handle;
static display_st7735_flush_done_cb_t s_flush_done_cb;

static bool st7735_color_trans_done(esp_lcd_panel_io_handle_t panel_io,
                                    esp_lcd_panel_io_event_data_t *edata,
                                    void *user_ctx)
{
    (void)panel_io;
    (void)edata;
    (void)user_ctx;

    if (s_flush_done_cb != NULL) {
        s_flush_done_cb();
    }
    return false;
}

/**
 * @brief 7735 spi写
 * @param command
 * @param data
 * @param size
 * @return esp_err_t
 */
static esp_err_t st7735_write(uint8_t command, const void *data, size_t size)
{
    esp_err_t ret = esp_lcd_panel_io_tx_param(s_panel_io, command, data, size);
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "write cmd 0x%02X error(%d)| %s", command, ret, esp_err_to_name(ret));
    }
    return ret;
}

/**
 * @brief 设置屏幕背光
 * @param enabled
 * @return esp_err_t
 */
esp_err_t display_st7735_set_backlight(bool enabled)
{
    if (CONFIG_DISPLAY_ST7735_GPIO_BACKLIGHT < 0) {
        CORE_LOGE(TAG, "backlight gpio is not configured");
        return ESP_ERR_NOT_SUPPORTED;
    }
    return gpio_set_level(CONFIG_DISPLAY_ST7735_GPIO_BACKLIGHT, enabled == ST7735_BACKLIGHT_ACTIVE_HIGH);
}

/**
 * @brief 刷新一块 RGB565 像素区域到 ST7735 显存
 *
 * 坐标采用左闭右开：
 * - x_start/y_start：起始像素
 * - x_end/y_end：结束像素的下一个位置
 */
esp_err_t display_st7735_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const void *rgb565_data)
{
    if (!s_initialized) {
        CORE_LOGE(TAG, "display is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (rgb565_data == NULL || x_start >= x_end || y_start >= y_end ||
            x_end > CONFIG_DISPLAY_ST7735_WIDTH || y_end > CONFIG_DISPLAY_ST7735_HEIGHT) {
        CORE_LOGE(TAG, "invalid bitmap region");
        return ESP_ERR_INVALID_ARG;
    }

    const uint16_t draw_w = x_end - x_start;
    const uint16_t draw_h = y_end - y_start;

    const uint16_t x0 = x_start + CONFIG_DISPLAY_ST7735_X_OFFSET;
    const uint16_t x1 = x_end - 1 + CONFIG_DISPLAY_ST7735_X_OFFSET;
    const uint16_t y0 = y_start + CONFIG_DISPLAY_ST7735_Y_OFFSET;
    const uint16_t y1 = y_end - 1 + CONFIG_DISPLAY_ST7735_Y_OFFSET;

    const uint8_t columns[] = {
        x0 >> 8, x0 & 0xFF,
           x1 >> 8, x1 & 0xFF,
    };

    const uint8_t rows[] = {
        y0 >> 8, y0 & 0xFF,
           y1 >> 8, y1 & 0xFF,
    };

    esp_err_t ret = st7735_write(LCD_CMD_CASET, columns, sizeof(columns));
    if (ret != ESP_OK) {
        return ret;
    }

    ret = st7735_write(LCD_CMD_RASET, rows, sizeof(rows));
    if (ret != ESP_OK) {
        return ret;
    }

    ret = esp_lcd_panel_io_tx_color(s_panel_io, LCD_CMD_RAMWR, rgb565_data, (size_t)draw_w * draw_h * sizeof(uint16_t));

    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "draw bitmap error(%d)| %s", ret, esp_err_to_name(ret));
    }

    return ret;
}

/**
 * @brief 7735测试模式线程
 * @param arg
 */
static void st7735_test_task(void *arg)
{
    uint16_t *frame_buffer = (uint16_t *)arg;
    const size_t pixel_count = (size_t)CONFIG_DISPLAY_ST7735_WIDTH * CONFIG_DISPLAY_ST7735_HEIGHT;

    /* RGB565 is sent as raw bytes, so store each color in panel byte order. */
    static const uint16_t test_colors[] = {
        0x00F8, /* Red:   0xF800 */
        0xE007, /* Green: 0x07E0 */
        0x1F00, /* Blue:  0x001F */
    };

    size_t color_index = 0;
    while (1) {
        for (size_t pixel = 0; pixel < pixel_count; ++pixel) {
            frame_buffer[pixel] = test_colors[color_index];
        }

        esp_err_t ret = display_st7735_flush(0, 0, CONFIG_DISPLAY_ST7735_WIDTH,  CONFIG_DISPLAY_ST7735_HEIGHT, frame_buffer);
        if (ret != ESP_OK) {
            CORE_LOGE(TAG, "display test flush failed(%d)| %s", ret, esp_err_to_name(ret));
        }

        color_index = (color_index + 1) % (sizeof(test_colors) / sizeof(test_colors[0]));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}

/**
 * @brief 7735测试模式
 * @return esp_err_t
 */
esp_err_t display_st7735_test(void)
{
    if (s_test_task_handle != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!s_initialized) {
        esp_err_t ret = display_st7735_init(NULL);
        if (ret != ESP_OK) {
            return ret;
        }
    }

    const size_t pixel_count = (size_t)CONFIG_DISPLAY_ST7735_WIDTH * CONFIG_DISPLAY_ST7735_HEIGHT;
    uint16_t *frame_buffer = heap_caps_malloc(pixel_count * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (frame_buffer == NULL) {
        CORE_LOGE(TAG, "failed to allocate test frame buffer from SPIRAM");
        return ESP_ERR_NO_MEM;
    }

    BaseType_t task_result = xTaskCreate(st7735_test_task, "st7735_test", (3 * 1024),  frame_buffer,  5, &s_test_task_handle);
    if (task_result != pdPASS) {
        heap_caps_free(frame_buffer);
        s_test_task_handle = NULL;
        CORE_LOGE(TAG, "failed to create display test task");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

/**
 * @brief 777735参数初始化
 */
static esp_err_t st7735_commands_init(void)
{
    /* 设置普通显示模式帧率 */
    const uint8_t frame_rate[] = {0x01, 0x2C, 0x2D};
    esp_err_t ret = st7735_write(0xB1, frame_rate, sizeof(frame_rate));
    if (ret != ESP_OK) return ret;

    /* 设置空闲显示模式帧率 */
    ret = st7735_write(0xB2, frame_rate, sizeof(frame_rate));
    if (ret != ESP_OK) return ret;

    /* 设置局部显示模式帧率 */
    const uint8_t frame_rate_partial[] = {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D};
    ret = st7735_write(0xB3, frame_rate_partial, sizeof(frame_rate_partial));
    if (ret != ESP_OK) return ret;

#if ST7735_INVERT_COLOR
    /* 设置驱动反转方式 - 列反转*/
    const uint8_t inversion_control = 0x07;
    ret = st7735_write(0xB4, &inversion_control, 1);
    if (ret != ESP_OK) return ret;
#endif

    /* 设置电源控制 1 */
    const uint8_t power_1[] = {0xA2, 0x02, 0x84};
    ret = st7735_write(0xC0, power_1, sizeof(power_1));
    if (ret != ESP_OK) return ret;

    /* 设置栅极高压和低压电压等级 */
    const uint8_t power_2 = 0xC5;
    ret = st7735_write(0xC1, &power_2, 1);
    if (ret != ESP_OK) return ret;

    /* 设置常态模式下运放电流和偏置电流 */
    const uint8_t power_3[] = {0x0A, 0x00};
    ret = st7735_write(0xC2, power_3, sizeof(power_3));
    if (ret != ESP_OK) return ret;

    /* 设置局部模式的GVDD电平 */
    const uint8_t power_4[] = {0x8A, 0x2A};
    ret = st7735_write(0xC3, power_4, sizeof(power_4));
    if (ret != ESP_OK) return ret;

    /* 设置VCOM电压 */
    const uint8_t power_5[] = {0x8A, 0xEE};
    ret = st7735_write(0xC4, power_5, sizeof(power_5));
    if (ret != ESP_OK) return ret;

    /* 设置 VCOM 电压 */
    const uint8_t vcom = 0x0E;
    ret = st7735_write(0xC5, &vcom, 1);
    if (ret != ESP_OK) return ret;

    /* 设置显存扫描方向和 RGB/BGR 顺序 */
    const uint8_t madctl = ST7735_BGR_ORDER ? LCD_CMD_BGR_BIT : 0;
    ret = st7735_write(LCD_CMD_MADCTL, &madctl, 1);
    if (ret != ESP_OK) return ret;

    /* RGB565 */
    const uint8_t color_mode = 0x05;
    ret = st7735_write(LCD_CMD_COLMOD, &color_mode, 1);
    if (ret != ESP_OK) return ret;

    return ESP_OK;
}

/**
 * @brief 7735上电初始化
 * @return esp_err_t
 */
static esp_err_t st7735_panel_init(void)
{
    /* 软复位芯片 */
    esp_err_t ret = st7735_write(LCD_CMD_SWRESET, NULL, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(150));

    /* 退出休眠模式 */
    ret = st7735_write(LCD_CMD_SLPOUT, NULL, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(120));

    /* 初始化参数 */
    ret = st7735_commands_init();
    if (ret != ESP_OK) return ret;

    /* 设置是否使用 Display Inversion 显示模式 */
    ret = st7735_write(ST7735_INVERT_COLOR ? LCD_CMD_INVON : LCD_CMD_INVOFF, NULL, 0);
    if (ret != ESP_OK) return ret;

    /* 进入普通常态显示模式 */
    ret = st7735_write(LCD_CMD_NORON, NULL, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(10));

    /* 打开显示输出 */
    ret = st7735_write(LCD_CMD_DISPON, NULL, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(100));
    return ESP_OK;
}

/**
 * @brief 7735驱动初始化
 * @param 发送完成回调
 * @return esp_err_t
 */
esp_err_t display_st7735_init(display_st7735_flush_done_cb_t cb)
{
    esp_err_t ret = ESP_OK;

    if (s_initialized) {
        CORE_LOGE(TAG, "display already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (CONFIG_DISPLAY_ST7735_GPIO_MOSI <= GPIO_NUM_NC || CONFIG_DISPLAY_ST7735_GPIO_SCLK <= GPIO_NUM_NC ||
        CONFIG_DISPLAY_ST7735_GPIO_CS   <= GPIO_NUM_NC || CONFIG_DISPLAY_ST7735_GPIO_DC   <= GPIO_NUM_NC) {

        CORE_LOGE(TAG, "display GPIO is not configured in menuconfig");
        return ESP_ERR_INVALID_ARG;
    }

    s_flush_done_cb = cb;

    #if CONFIG_DISPLAY_ST7735_GPIO_BACKLIGHT > GPIO_NUM_NC
        gpio_config_t backlight_pin_config = {
            .pin_bit_mask = 1ULL << CONFIG_DISPLAY_ST7735_GPIO_BACKLIGHT,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };

        ret = gpio_config(&backlight_pin_config);
        if (ret != ESP_OK) {
            CORE_LOGE(TAG, "backlight gpio init error(%d)| %s", ret, esp_err_to_name(ret));
            return ret;
        }

        display_st7735_set_backlight(false);
    #endif

    spi_bus_config_t bus_config = {
        .mosi_io_num = CONFIG_DISPLAY_ST7735_GPIO_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = CONFIG_DISPLAY_ST7735_GPIO_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = CONFIG_DISPLAY_ST7735_WIDTH *
        CONFIG_DISPLAY_ST7735_HEIGHT *
        sizeof(uint16_t),
    };

    ret = spi_bus_initialize(ST7735_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        CORE_LOGE(TAG, "spi bus init error(%d)| %s", ret, esp_err_to_name(ret));
        return ret;
    }

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = CONFIG_DISPLAY_ST7735_GPIO_DC,
        .cs_gpio_num = CONFIG_DISPLAY_ST7735_GPIO_CS,
        .pclk_hz = CONFIG_DISPLAY_ST7735_PIXEL_CLOCK_HZ,
        .on_color_trans_done = (cb != NULL) ? st7735_color_trans_done : NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };

    ret = esp_lcd_new_panel_io_spi(
              (esp_lcd_spi_bus_handle_t)ST7735_SPI_HOST,
              &io_config,
              &s_panel_io
          );
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "panel io init error(%d)| %s", ret, esp_err_to_name(ret));
        return ret;
    }

    #if CONFIG_DISPLAY_ST7735_GPIO_RST > GPIO_NUM_NC
        gpio_config_t reset_pin_config = {
            .pin_bit_mask = 1ULL << CONFIG_DISPLAY_ST7735_GPIO_RST,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };

        ret = gpio_config(&reset_pin_config);
        if (ret != ESP_OK) {
            CORE_LOGE(TAG, "reset gpio init error(%d)| %s", ret, esp_err_to_name(ret));
            return ret;
        }

        gpio_set_level(CONFIG_DISPLAY_ST7735_GPIO_RST, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level(CONFIG_DISPLAY_ST7735_GPIO_RST, 1);
        vTaskDelay(pdMS_TO_TICKS(120));
    #endif

    ret = st7735_panel_init();
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "st7735 panel init error(%d)| %s", ret, esp_err_to_name(ret));
        return ret;
    }

    s_initialized = true;

    #if CONFIG_DISPLAY_ST7735_GPIO_BACKLIGHT > GPIO_NUM_NC
        display_st7735_set_backlight(true);
    #endif

    return ESP_OK;
}
#endif
