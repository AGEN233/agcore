#include "sdkconfig.h"
#if defined(CONFIG_PIXEL_DRIVER_ENABLE) && defined(CONFIG_PIXEL_TYPE_WS2812B_RMT)
#include "pixel_driver.h"
#include <assert.h>
#include "driver/rmt_tx.h"
#include "esp_attr.h"
#include "esp_err.h"
#define TAG "WS2812B(RMT)"

#define WS2812B_RMT_HZ 10000000
static uint8_t ws2812b_rgb_buf[CONFIG_LED_COUNT_MAX * 3] = {0};
static SemaphoreHandle_t ws2812b_rmt_tx_done_sem;
static TaskHandle_t ws2812b_rmt_drv_task_handle = NULL;
static rmt_channel_handle_t led_chan = NULL;
static rmt_encoder_handle_t simple_encoder = NULL;
static rmt_tx_channel_config_t tx_chan_config;
static uint16_t led_num = CONFIG_DEFAULT_LED_NUM;

rmt_transmit_config_t tx_config = {
    // no transfer loop
    .loop_count = 0,
};

static const rmt_symbol_word_t ws2812b_bit0 = {
    .level0 = 1,
    .duration0 = 3,
    .level1 = 0,
    .duration1 = 9
};

static const rmt_symbol_word_t ws2812b_bit1 = {
    .level0 = 1,
    .duration0 = 9,
    .level1 = 0,
    .duration1 = 3
};

static const  rmt_symbol_word_t ws2812b_reset = {
    .duration0 = 250,
    .level0 = 0,
    .duration1 = 250,
    .level1 = 0
};

/**
 * @brief 设置LED点数
 */
void pixel_drv_set_led_len(uint16_t len)
{
    if (len == 0 || len > CONFIG_LED_COUNT_MAX) {
        CORE_LOGE(TAG, "LED LEN ERROR, USE DEFAULT %d| current: %d", CONFIG_DEFAULT_LED_NUM, len);
        led_num = CONFIG_DEFAULT_LED_NUM;
    } else {
        led_num = len;
    }
}

/**
 * @brief 复制buf到发送线程
 */
void pixel_drv_sendata(const uint8_t *color_data, uint16_t len)
{
    if (len > led_num) len = led_num;
    memcpy(ws2812b_rgb_buf, color_data, (len * 3));
}

/**
 * @brief RMT编码回调
 */
static size_t encoder_callback(const void *data, size_t data_size, size_t symbols_written,  size_t symbols_free, rmt_symbol_word_t *symbols, bool *done, void *arg)
{
    if (symbols_free < 8) {
        return 0;
    }
    size_t data_pos = symbols_written / 8;
    uint8_t *data_bytes = (uint8_t *)data;
    if (data_pos < data_size) {
        // Encode a byte
        size_t symbol_pos = 0;
        for (int bitmask = 0x80; bitmask != 0; bitmask >>= 1) {
            if (data_bytes[data_pos]&bitmask) {
                symbols[symbol_pos++] = ws2812b_bit1;
            } else {
                symbols[symbol_pos++] = ws2812b_bit0;
            }
        }
        return symbol_pos;
    } else {
        symbols[0] = ws2812b_reset;
        *done = 1;
        return 1;
    }
}

/**
 * @brief RMT TXDONE
 */
static bool rmt_tx_done_callback(rmt_channel_handle_t channel, const rmt_tx_done_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup = pdFALSE;

    xSemaphoreGiveFromISR(ws2812b_rmt_tx_done_sem, &high_task_wakeup);

    return high_task_wakeup == pdTRUE;
}

/**
 * @brief RMT发送线程
 */
static void ws2812b_rmt_drv_task(void *arg)
{
    while (1) {
        ESP_ERROR_CHECK(rmt_transmit(led_chan, simple_encoder, ws2812b_rgb_buf, (led_num * 3), &tx_config));
        xSemaphoreTake(ws2812b_rmt_tx_done_sem, portMAX_DELAY);
    }
}

/**
 * @brief RMT初始化
 */
static void ws2812b_rmt_init(void)
{
    tx_chan_config.clk_src = RMT_CLK_SRC_DEFAULT;
    tx_chan_config.gpio_num = CONFIG_ARGB_OUT_PIN;
    tx_chan_config.mem_block_symbols = 64;
    tx_chan_config.resolution_hz = WS2812B_RMT_HZ;
    tx_chan_config.trans_queue_depth = 4;
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    rmt_tx_event_callbacks_t cbs = {
        .on_trans_done = rmt_tx_done_callback,
    };
    ESP_ERROR_CHECK(rmt_tx_register_event_callbacks(led_chan, &cbs, NULL));
    ws2812b_rmt_tx_done_sem = xSemaphoreCreateBinary();

    const rmt_simple_encoder_config_t simple_encoder_cfg = {
        .callback = encoder_callback
    };
    ESP_ERROR_CHECK(rmt_new_simple_encoder(&simple_encoder_cfg, &simple_encoder));

    ESP_ERROR_CHECK(rmt_enable(led_chan));

    memset(ws2812b_rgb_buf, 0, CONFIG_LED_COUNT_MAX * 3);

    BaseType_t ret = xTaskCreate(ws2812b_rmt_drv_task, "ws2812b_rmt_drv_task", CONFIG_WS2812B_STACK_SIZE, NULL, 5, &ws2812b_rmt_drv_task_handle);
    assert(ret == pdPASS);
}

/**
 * @brief 驱动初始化
 */
void pixel_driver_init(void)
{
    ws2812b_rmt_init();
}

#elif defined(CONFIG_PIXEL_DRIVER_ENABLE) && defined(CONFIG_PIXEL_TYPE_WS2812B_SPI)
#include "pixel_driver.h"

#include "esp_attr.h"
#include "esp_err.h"
#include "driver/spi_master.h"
#define TAG "WS2812B(SPI)"

#define SPI_BITS_PER_WS2812_BIT      4
#define BYTES_PER_LED               ((3 * 8 * SPI_BITS_PER_WS2812_BIT) / 8)

// WS2812B Driver buff
static DMA_ATTR uint8_t ws2812b_spi_buf[CONFIG_LED_COUNT_MAX * BYTES_PER_LED];
static uint8_t ws2812b_rgb_buf[CONFIG_LED_COUNT_MAX * 3];

// WS2812B RGB - > SPI LUT
static const uint16_t ws2812b_half_lut_buf[16] = {
    0x8888,  // 0b0000 → 1000 1000 1000 1000
    0x888E,  // 0b0001 → 1000 1000 1000 1110
    0x88E8,  // 0b0010 → 1000 1000 1110 1000
    0x88EE,  // 0b0011 → 1000 1000 1110 1110
    0x8E88,  // 0b0100 → 1000 1110 1000 1000
    0x8E8E,  // 0b0101 → 1000 1110 1000 1110
    0x8EE8,  // 0b0110 → 1000 1110 1110 1000
    0x8EEE,  // 0b0111 → 1000 1110 1110 1110
    0xE888,  // 0b1000 → 1110 1000 1000 1000
    0xE88E,  // 0b1001 → 1110 1000 1000 1110
    0xE8E8,  // 0b1010 → 1110 1000 1110 1000
    0xE8EE,  // 0b1011 → 1110 1000 1110 1110
    0xEE88,  // 0b1100 → 1110 1110 1000 1000
    0xEE8E,  // 0b1101 → 1110 1110 1000 1110
    0xEEE8,  // 0b1110 → 1110 1110 1110 1000
    0xEEEE   // 0b1111 → 1110 1110 1110 1110
};

static bool ws2812b_init_done = false;
static spi_device_handle_t spi_handle = NULL;               // SPIHandle
static TaskHandle_t ws2812b_spi_drv_handle = NULL;          // TaskHandle
static uint16_t led_num = CONFIG_DEFAULT_LED_NUM;

/**
 * @brief 设置发送灯光数据
 * @param color_data
 * @param len
 */
void pixel_drv_set_led_len(uint16_t len)
{
    if (len == 0 || len > CONFIG_LED_COUNT_MAX) {
        CORE_LOGE(TAG, "LED LEN ERROR, USE DEFAULT %d| current: %d", CONFIG_DEFAULT_LED_NUM, len);
        led_num = CONFIG_DEFAULT_LED_NUM;
    } else {
        led_num = len;
    }
}

void pixel_drv_sendata(const uint8_t *color_data, uint16_t len)
{
    if (len > led_num) len = led_num;
    memcpy(ws2812b_rgb_buf, color_data, (len * 3));
}

/**
 * @brief RGB ->ws2812b(查表) -> SPIBIT
 * @param rgb_buf
 * @param spi_buf
 */
static inline void ws2812b_data_rgb_to_spi(const uint8_t *rgb_buf, uint8_t *spi_buf)
{
    uint32_t spi_index = 0;
    for (uint16_t i = 0; i < led_num; i++) {
        uint8_t R = rgb_buf[i * 3];
        uint8_t G = rgb_buf[(i * 3) + 1];
        uint8_t B = rgb_buf[(i * 3) + 2];

        uint8_t colors[3] = {G, R, B};

        for (uint8_t i = 0; i < 3; i++) {
            uint8_t color_val = colors[i];
            uint8_t high = color_val >> 4;
            uint8_t low = color_val & 0x0F;

            uint16_t spi_high = ws2812b_half_lut_buf[high];
            uint16_t spi_low = ws2812b_half_lut_buf[low];

            spi_buf[spi_index++] = (spi_high >> 8) & 0xFF;
            spi_buf[spi_index++] = spi_high & 0xFF;
            spi_buf[spi_index++] = (spi_low >> 8) & 0xFF;
            spi_buf[spi_index++] = spi_low & 0xFF;
        }
    }
}

/**
 * @brief 驱动线程
 */
static void ws2812b_spi_drv_task(void *arg)
{
    static spi_transaction_t t;
    static spi_transaction_t *ret_trans;
    while (1) {
        if (!ws2812b_init_done) {
            vTaskDelay(pdMS_TO_TICKS(40));
            continue;
        }

        ws2812b_data_rgb_to_spi(ws2812b_rgb_buf, ws2812b_spi_buf);

        // SPI发送
        memset(&t, 0, sizeof(t));
        t.length = led_num * BYTES_PER_LED * 8;
        t.tx_buffer = ws2812b_spi_buf;

        spi_device_queue_trans(spi_handle, &t, portMAX_DELAY);
        spi_device_get_trans_result(spi_handle, &ret_trans, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/**
 * @brief 驱动初始化
 */
static void ws2812b_spi_init(void)
{
    esp_err_t ret;
    const static spi_bus_config_t buscfg = {
        .mosi_io_num = CONFIG_ARGB_OUT_PIN,
        .sclk_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = CONFIG_LED_COUNT_MAX *  BYTES_PER_LED,
    };
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "spi bus init error(%d)| %s", ret, esp_err_to_name(ret));
        return;
    }

    const static spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 2500000, // 2.5M 波特率
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
    };
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);
    if (ret != ESP_OK) {
        CORE_LOGE(TAG, "spi device init error(%d)| %s", ret, esp_err_to_name(ret));
        return;
    }

    memset(ws2812b_spi_buf, 0, CONFIG_LED_COUNT_MAX * BYTES_PER_LED);
    memset(ws2812b_rgb_buf, 0, CONFIG_LED_COUNT_MAX * 3);

    if (ws2812b_spi_drv_handle == NULL) {
        xTaskCreate(ws2812b_spi_drv_task, "ws2812b_spi_drv_task", CONFIG_WS2812B_STACK_SIZE, NULL, 5, &ws2812b_spi_drv_handle);

        if (ws2812b_spi_drv_handle == NULL) {
            CORE_LOGI(TAG, "driver task create error\n");
            return;
        }
    }

    ws2812b_init_done = true;
    CORE_LOGI(TAG, "driver init success");
}

/**
 * @brief 驱动初始化
 */
void pixel_driver_init(void)
{
    ws2812b_spi_init();
}
#else
#include "pixel_driver.h"

void pixel_drv_set_led_len(uint16_t len)
{
    (void)len;
}

void pixel_drv_sendata(const uint8_t *color_data, uint16_t len)
{
    (void)color_data;
    (void)len;
}

void pixel_driver_init(void)
{
}
#endif
