#include "agcore_console.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_rom_sys.h"
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>

static SemaphoreHandle_t g_console_output_lock;

/**
 * @brief 获取 Console 公共输出锁
 */
void agcore_console_output_lock(void)
{
    xSemaphoreTake(g_console_output_lock, portMAX_DELAY);
}

/**
 * @brief 释放 Console 公共输出锁
 */
void agcore_console_output_unlock(void)
{
    xSemaphoreGive(g_console_output_lock);
}

/**
 * @brief console输出
 * @param data
 * @param size
 */
void agcore_console_output(const char *data, size_t size)
{
    int fd = fileno(stdout);
    size_t offset = 0;

    if (fd < 0) {
        return;
    }

    while (offset < size) {
        ssize_t written = write(fd, data + offset, size - offset);
        if (written <= 0) {
            return;
        }
        offset += (size_t)written;
    }

}

/**
 * @brief 刷新 Console backend 的待发送数据。
 */
void agcore_console_flush(void)
{
    const int fd = fileno(stdout);

    if (fd >= 0) {
        (void)fsync(fd);
    }
}

/**
 * @brief 初始化 Console 公共输出资源。
 * @return 初始化成功返回 true，否则返回 false
 */
bool agcore_console_output_init(void)
{
    g_console_output_lock = xSemaphoreCreateMutex();
    return g_console_output_lock != NULL;
}
