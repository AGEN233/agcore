#include "agcore_log_adapter.h"
#include "agcore_console.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <time.h>

#define AGCORE_LOG_TASK_STACK   (2048)
#define AGCORE_LOG_TASK_PRIO    (5)

static SemaphoreHandle_t s_log_notify;         /* 异步输出通知(二值信号量) */
static StaticSemaphore_t s_log_notify_buf;

static void agcore_log_output_task(void *arg);

/**
 * @brief EasyLogger 最终输出(整行文本交给 console 写口, 由 console 内部持锁)
 * @param log 整行日志内容指针
 * @param size 整行日志字节数
 */
void agcore_log_port_output(const char *log, size_t size)
{
    agcore_console_write(log, size);
}

/**
 * @brief 异步输出通知
 */
void agcore_log_async_notice(void)
{
    xSemaphoreGive(s_log_notify);
}

/**
 * @brief 异步输出 task(log 专属): 等通知 → 逐行取缓冲 → 写 console
 */
static void agcore_log_output_task(void *arg)
{
    static char buf[ELOG_LINE_BUF_SIZE - 4];
    size_t size;
    (void)arg;

    while (1) {
        xSemaphoreTake(s_log_notify, portMAX_DELAY);
        while ((size = elog_async_get_line_log(buf, sizeof(buf))) > 0) {
            agcore_console_write(buf, size);
        }
    }
}

/**
 * @brief 获取当前时间字符串(含时分秒)
 * @return 静态缓冲的时间字符串
 */
const char *agcore_log_port_get_time(void)
{
    static char tbuf[24];
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(tbuf, sizeof(tbuf), "%H:%M:%S", &tm);
    return tbuf;
}

/**
 * @brief agcore log初始化
 * @note  日志必须初始化, 失败不跳过
 */
void agcore_log_init(void)
{
    s_log_notify = xSemaphoreCreateBinaryStatic(&s_log_notify_buf);
    if (!s_log_notify) {
        return;
    }

    if (elog_init() == ELOG_NO_ERR) {
        elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
        elog_set_fmt(ELOG_LVL_ERROR,  ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_FUNC | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_WARN,   ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_FUNC | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_INFO,   ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_FUNC | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_DEBUG,  ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_FUNC | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
        elog_start();

        xTaskCreate(agcore_log_output_task, "elog_out", AGCORE_LOG_TASK_STACK, NULL, AGCORE_LOG_TASK_PRIO, NULL);
    }
}
