#include "sdkconfig.h"

#if defined(CONFIG_DISPLAY_DRIVER_ENABLE) && defined(CONFIG_DISPLAY_IC_ST7789)

#include "display7789.h"

#include "agcore_printf_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "ST7789"
#define ST7789_CMD_RAMCTRL 0xB0
#define ST7789_DATA_LITTLE_ENDIAN_BIT (1 << 3)

#ifdef CONFIG_DISPLAY_ST7789_SPI2_HOST
#define ST7789_SPI_HOST SPI2_HOST
#else
#define ST7789_SPI_HOST SPI3_HOST
#endif

#ifdef CONFIG_DISPLAY_ST7789_BACKLIGHT_ACTIVE_HIGH
#define ST7789_BACKLIGHT_ACTIVE_HIGH true
#else
#define ST7789_BACKLIGHT_ACTIVE_HIGH false
#endif

static display_st7789_runtime_st s_runtime = {
    .width = CONFIG_DISPLAY_ST7789_WIDTH,
    .height = CONFIG_DISPLAY_ST7789_HEIGHT,
    .rotation = DISPLAY_ST7789_ROTATION_0,
};

/**
 * @brief SPI 颜色数据发送完成回调
 */
static bool st7789_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    (void)panel_io;
    (void)edata;
    (void)user_ctx;

    if (s_runtime.flush_done_cb != NULL) {
        s_runtime.flush_done_cb();
    }
    return false;
}

/**
 * @brief 向 ST7789 发送命令和参数
 * @param command 命令字
 * @param data 参数数据
 * @param size 参数长度
 * @return esp_err_t
 */
static esp_err_t st7789_write(uint8_t command, const void *data, size_t size)
{
    esp_err_t ret = esp_lcd_panel_io_tx_param(s_runtime.panel_io, command, data, size);
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "write cmd 0x%02X error(%d)| %s", command, ret, esp_err_to_name(ret));
    }
    return ret;
}

/**
 * @brief 设置背光输出电平
 * @param enabled true 表示开背光
 * @return esp_err_t
 */
esp_err_t display_st7789_set_backlight(bool enabled)
{
    if (CONFIG_DISPLAY_ST7789_GPIO_BACKLIGHT < 0) {
        CORE_LOGE(TAG, "backlight gpio is not configured");
        return ESP_ERR_NOT_SUPPORTED;
    }
    return gpio_set_level(CONFIG_DISPLAY_ST7789_GPIO_BACKLIGHT, enabled == ST7789_BACKLIGHT_ACTIVE_HIGH);
}

/**
 * @brief 设置屏幕旋转方向，并同步更新运行时宽高
 * @param rotation 目标方向
 * @return esp_err_t
 */
esp_err_t display_st7789_set_rotation(display_st7789_rotation_et rotation)
{
    if (!s_runtime.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    /* 当前板载 ST7789 实物按 RGB 正常色序工作。
     * 这里固定从 RGB 基础值开始，避免 menuconfig 中残留的
     * BGR/Invert 组合继续把纯色显示成黄/粉/青。 */
    uint8_t madctl = 0;
    switch (rotation) {
        case DISPLAY_ST7789_ROTATION_0: {
            break;
        }
        case DISPLAY_ST7789_ROTATION_90: {
            madctl |= LCD_CMD_MV_BIT | LCD_CMD_MX_BIT;
            break;
        }
        case DISPLAY_ST7789_ROTATION_180: {
            madctl |= LCD_CMD_MX_BIT | LCD_CMD_MY_BIT;
            break;
        }
        case DISPLAY_ST7789_ROTATION_270: {
            madctl |= LCD_CMD_MV_BIT | LCD_CMD_MY_BIT;
            break;
        }
        default: {
            return ESP_ERR_INVALID_ARG;
        }
    }

    esp_err_t ret = st7789_write(LCD_CMD_MADCTL, &madctl, sizeof(madctl));
    if (ret == ESP_OK) {
        s_runtime.rotation = rotation;
        if (rotation == DISPLAY_ST7789_ROTATION_90 || rotation == DISPLAY_ST7789_ROTATION_270) {
            s_runtime.width = CONFIG_DISPLAY_ST7789_HEIGHT;
            s_runtime.height = CONFIG_DISPLAY_ST7789_WIDTH;
        } else {
            s_runtime.width = CONFIG_DISPLAY_ST7789_WIDTH;
            s_runtime.height = CONFIG_DISPLAY_ST7789_HEIGHT;
        }
        CORE_LOGD(TAG, "rotation=%d madctl=0x%02X size=%ux%u", rotation, madctl, s_runtime.width, s_runtime.height);
    }
    return ret;
}

/**
 * @brief 获取当前运行时方向
 * @return display_st7789_rotation_et
 */
display_st7789_rotation_et display_st7789_get_rotation(void)
{
    return s_runtime.rotation;
}

/**
 * @brief 获取当前运行时屏幕宽度
 * @return uint16_t
 */
uint16_t display_st7789_get_width(void)
{
    return s_runtime.width;
}

/**
 * @brief 获取当前运行时屏幕高度
 * @return uint16_t
 */
uint16_t display_st7789_get_height(void)
{
    return s_runtime.height;
}

/**
 * @brief 刷新一块 RGB565 像素区域到 ST7789 显存
 *
 * 坐标采用左闭右开：
 * - x_start/y_start：起始像素
 * - x_end/y_end：结束像素的下一个位置
 */
esp_err_t display_st7789_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const void *rgb565_data)
{
    if (!s_runtime.initialized) {
        CORE_LOGE(TAG, "display is not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (rgb565_data == NULL || x_start >= x_end || y_start >= y_end ||
            x_end > display_st7789_get_width() || y_end > display_st7789_get_height()) {
        CORE_LOGE(TAG, "invalid bitmap region");
        return ESP_ERR_INVALID_ARG;
    }

    const uint16_t draw_w = x_end - x_start;
    const uint16_t draw_h = y_end - y_start;

    uint16_t x_offset = CONFIG_DISPLAY_ST7789_X_OFFSET;
    uint16_t y_offset = CONFIG_DISPLAY_ST7789_Y_OFFSET;
    if (s_runtime.rotation == DISPLAY_ST7789_ROTATION_90 ||
            s_runtime.rotation == DISPLAY_ST7789_ROTATION_270) {
        x_offset = CONFIG_DISPLAY_ST7789_Y_OFFSET;
        y_offset = CONFIG_DISPLAY_ST7789_X_OFFSET;
    }

    const uint16_t x0 = x_start + x_offset;
    const uint16_t x1 = x_end - 1 + x_offset;
    const uint16_t y0 = y_start + y_offset;
    const uint16_t y1 = y_end - 1 + y_offset;

    const uint8_t columns[] = {
        x0 >> 8, x0 & 0xFF,
        x1 >> 8, x1 & 0xFF,
    };

    const uint8_t rows[] = {
        y0 >> 8, y0 & 0xFF,
        y1 >> 8, y1 & 0xFF,
    };

    esp_err_t ret = st7789_write(LCD_CMD_CASET, columns, sizeof(columns));
    if (ret != ESP_OK) {
        return ret;
    }

    ret = st7789_write(LCD_CMD_RASET, rows, sizeof(rows));
    if (ret != ESP_OK) {
        return ret;
    }

    ret = esp_lcd_panel_io_tx_color(s_runtime.panel_io, LCD_CMD_RAMWR, rgb565_data, (size_t)draw_w * draw_h * sizeof(uint16_t));
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "draw bitmap error(%d)| %s", ret, esp_err_to_name(ret));
    }

    return ret;
}

/**
 * @brief ST7789 测试线程，循环刷纯色画面
 * @param arg 外部分配的测试帧缓冲
 */
static void st7789_test_task(void *arg)
{
    uint16_t *frame_buffer = (uint16_t *)arg;
    const size_t pixel_count = (size_t)CONFIG_DISPLAY_ST7789_WIDTH * CONFIG_DISPLAY_ST7789_HEIGHT;

    /* ST7789 当前按常规 RGB565 解析颜色数据。 */
    static const uint16_t test_colors[] = {
        0xF800,
        0x07E0,
        0x001F,
    };

    size_t color_index = 0;
    while (1) {
        for (size_t pixel = 0; pixel < pixel_count; ++pixel) {
            frame_buffer[pixel] = test_colors[color_index];
        }

        esp_err_t ret = display_st7789_flush(0, 0, CONFIG_DISPLAY_ST7789_WIDTH, CONFIG_DISPLAY_ST7789_HEIGHT, frame_buffer);
        if (ret != ESP_OK) {
            CORE_LOGE(TAG, "display test flush failed(%d)| %s", ret, esp_err_to_name(ret));
        }

        color_index = (color_index + 1) % (sizeof(test_colors) / sizeof(test_colors[0]));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief 创建 ST7789 测试任务
 * @return esp_err_t
 */
esp_err_t display_st7789_test(void)
{
    if (s_runtime.test_task_handle != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!s_runtime.initialized) {
        esp_err_t ret = display_st7789_init(NULL);
        if (ret != ESP_OK) {
            return ret;
        }
    }

    const size_t pixel_count = (size_t)CONFIG_DISPLAY_ST7789_WIDTH * CONFIG_DISPLAY_ST7789_HEIGHT;
    uint16_t *frame_buffer = heap_caps_malloc(pixel_count * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (frame_buffer == NULL) {
        CORE_LOGE(TAG, "failed to allocate test frame buffer from SPIRAM");
        return ESP_ERR_NO_MEM;
    }

    BaseType_t task_result = xTaskCreate(st7789_test_task, "st7789_test", (3 * 1024), frame_buffer, 5, &s_runtime.test_task_handle);
    if (task_result != pdPASS) {
        heap_caps_free(frame_buffer);
        s_runtime.test_task_handle = NULL;
        CORE_LOGE(TAG, "failed to create display test task");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

/**
 * @brief 初始化 ST7789 常用寄存器配置
 *
 * 这里采用常见的 16bit RGB565 初始化序列，适配大多数 SPI 接口的
 * ST7789 面板；如果后续实物存在偏色、抖动或灰阶异常，再按具体屏幕
 * 手册微调这组参数即可。
 */
static esp_err_t st7789_commands_init(void)
{
    const uint8_t porch_setting[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    esp_err_t ret = st7789_write(0xB2, porch_setting, sizeof(porch_setting));
    if (ret != ESP_OK) return ret;

    const uint8_t gate_control = 0x35;
    ret = st7789_write(0xB7, &gate_control, 1);
    if (ret != ESP_OK) return ret;

    const uint8_t vcom_setting = 0x19;
    ret = st7789_write(0xBB, &vcom_setting, 1);
    if (ret != ESP_OK) return ret;

    const uint8_t lcm_control = 0x2C;
    ret = st7789_write(0xC0, &lcm_control, 1);
    if (ret != ESP_OK) return ret;

    const uint8_t vdv_vrh_enable[] = {0x01, 0xFF};
    ret = st7789_write(0xC2, vdv_vrh_enable, sizeof(vdv_vrh_enable));
    if (ret != ESP_OK) return ret;

    const uint8_t vrh_setting = 0x12;
    ret = st7789_write(0xC3, &vrh_setting, 1);
    if (ret != ESP_OK) return ret;

    const uint8_t vdv_setting = 0x20;
    ret = st7789_write(0xC4, &vdv_setting, 1);
    if (ret != ESP_OK) return ret;

    const uint8_t frame_rate = 0x0F;
    ret = st7789_write(0xC6, &frame_rate, 1);
    if (ret != ESP_OK) return ret;

    const uint8_t power_control[] = {0xA4, 0xA1};
    ret = st7789_write(0xD0, power_control, sizeof(power_control));
    if (ret != ESP_OK) return ret;

    const uint8_t madctl = 0;
    ret = st7789_write(LCD_CMD_MADCTL, &madctl, 1);
    if (ret != ESP_OK) return ret;

    /* ST7789 的 16bit/pixel 格式值为 0x55。 */
    const uint8_t color_mode = 0x55;
    ret = st7789_write(LCD_CMD_COLMOD, &color_mode, 1);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(10));

    /* LVGL 这里输出常规 RGB565，ESP32 按内存字节序发 SPI 数据时会先发低字节，
     * 因此 ST7789 侧要打开 little-endian 像素解释，避免 16bit 颜色位段错位。 */
    const uint8_t ramctrl[] = {0x00, (uint8_t)(0xF0 | ST7789_DATA_LITTLE_ENDIAN_BIT)};
    ret = st7789_write(ST7789_CMD_RAMCTRL, ramctrl, sizeof(ramctrl));
    if (ret != ESP_OK) return ret;

    const uint8_t positive_gamma[] = {
        0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F,
        0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23,
    };
    ret = st7789_write(0xE0, positive_gamma, sizeof(positive_gamma));
    if (ret != ESP_OK) return ret;

    const uint8_t negative_gamma[] = {
        0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F,
        0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23,
    };
    ret = st7789_write(0xE1, negative_gamma, sizeof(negative_gamma));
    if (ret != ESP_OK) return ret;

    return ESP_OK;
}

/**
 * @brief ST7789 上电初始化流程
 * @return esp_err_t
 */
static esp_err_t st7789_panel_init(void)
{
    esp_err_t ret = st7789_write(LCD_CMD_SWRESET, NULL, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(150));

    ret = st7789_write(LCD_CMD_SLPOUT, NULL, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(120));

    ret = st7789_commands_init();
    if (ret != ESP_OK) return ret;

    /* 当前实拍结果表现为典型反色：红->青、绿->粉、蓝->黄。
     * 这里切换到 INVON，验证这块 ST7789 模组实际需要 inversion。 */
    ret = st7789_write(LCD_CMD_INVON, NULL, 0);
    if (ret != ESP_OK) return ret;

    ret = st7789_write(LCD_CMD_NORON, NULL, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(10));

    ret = st7789_write(LCD_CMD_DISPON, NULL, 0);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

/**
 * @brief ST7789 驱动初始化
 * @param cb 刷屏发送完成回调
 * @return esp_err_t
 */
esp_err_t display_st7789_init(display_st7789_flush_done_cb_t cb)
{
    esp_err_t ret = ESP_OK;

    if (s_runtime.initialized) {
        CORE_LOGE(TAG, "display already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (CONFIG_DISPLAY_ST7789_GPIO_MOSI <= GPIO_NUM_NC || CONFIG_DISPLAY_ST7789_GPIO_SCLK <= GPIO_NUM_NC ||
            CONFIG_DISPLAY_ST7789_GPIO_CS   <= GPIO_NUM_NC || CONFIG_DISPLAY_ST7789_GPIO_DC   <= GPIO_NUM_NC) {
        CORE_LOGE(TAG, "display GPIO is not configured in menuconfig");
        return ESP_ERR_INVALID_ARG;
    }

    s_runtime.flush_done_cb = cb;

#if CONFIG_DISPLAY_ST7789_GPIO_BACKLIGHT > GPIO_NUM_NC
    gpio_config_t backlight_pin_config = {
        .pin_bit_mask = 1ULL << CONFIG_DISPLAY_ST7789_GPIO_BACKLIGHT,
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

    display_st7789_set_backlight(false);
#endif

    spi_bus_config_t bus_config = {
        .mosi_io_num = CONFIG_DISPLAY_ST7789_GPIO_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = CONFIG_DISPLAY_ST7789_GPIO_SCLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = CONFIG_DISPLAY_ST7789_WIDTH *
        CONFIG_DISPLAY_ST7789_HEIGHT *
        sizeof(uint16_t),
    };

    ret = spi_bus_initialize(ST7789_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        CORE_LOGE(TAG, "spi bus init error(%d)| %s", ret, esp_err_to_name(ret));
        return ret;
    }

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = CONFIG_DISPLAY_ST7789_GPIO_DC,
        .cs_gpio_num = CONFIG_DISPLAY_ST7789_GPIO_CS,
        .pclk_hz = CONFIG_DISPLAY_ST7789_PIXEL_CLOCK_HZ,
        .on_color_trans_done = (cb != NULL) ? st7789_color_trans_done : NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };

    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)ST7789_SPI_HOST, &io_config, &s_runtime.panel_io);
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "panel io init error(%d)| %s", ret, esp_err_to_name(ret));
        return ret;
    }

#if CONFIG_DISPLAY_ST7789_GPIO_RST > GPIO_NUM_NC
    gpio_config_t reset_pin_config = {
        .pin_bit_mask = 1ULL << CONFIG_DISPLAY_ST7789_GPIO_RST,
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

    gpio_set_level(CONFIG_DISPLAY_ST7789_GPIO_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(CONFIG_DISPLAY_ST7789_GPIO_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
#endif

    ret = st7789_panel_init();
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "st7789 panel init error(%d)| %s", ret, esp_err_to_name(ret));
        return ret;
    }

    s_runtime.initialized = true;
    s_runtime.width = CONFIG_DISPLAY_ST7789_WIDTH;
    s_runtime.height = CONFIG_DISPLAY_ST7789_HEIGHT;
    s_runtime.rotation = DISPLAY_ST7789_ROTATION_0;

#if CONFIG_DISPLAY_ST7789_GPIO_BACKLIGHT > GPIO_NUM_NC
    display_st7789_set_backlight(true);
#endif

    return ESP_OK;
}

#endif
