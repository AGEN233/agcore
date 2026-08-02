#include "agcore_console.h"
#include "agcore_log_adapter.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include <stddef.h>

static SemaphoreHandle_t s_console_tx_lock;   /* 与 esh 共享的串口写锁 */
static StaticSemaphore_t s_console_lock_buf;

/**
 * @brief 初始化 console 共享件: 创建串口写锁, 并拉起日志初始化
 * @note  作为 console 层对外初始化入口, 确保锁先建好、日志再启动(日志使用该锁)
 */
void agcore_console_init(void)
{
    s_console_tx_lock = xSemaphoreCreateMutexStatic(&s_console_lock_buf);
    if (!s_console_tx_lock) {
        return;
    }
    agcore_log_init();
}

/**
 * @brief 获取共享串口写锁(阻塞等待)
 */
void agcore_console_lock(void)
{
    xSemaphoreTake(s_console_tx_lock, portMAX_DELAY);
}

/**
 * @brief 释放共享串口写锁
 */
void agcore_console_unlock(void)
{
    xSemaphoreGive(s_console_tx_lock);
}

/**
 * @brief 输出一段文本到 console(内部持共享锁, 保证 log/esh 写不互扰)
 * @param buf 文本内容指针
 * @param len 文本字节数
 */
void agcore_console_write(const char *buf, size_t len)
{
    agcore_console_lock();
    printf("%.*s", (int)len, buf);
    agcore_console_unlock();
}
