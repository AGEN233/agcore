#include "agcore_console_log.h"
#include "agcore_console.h"

#ifdef CONFIG_AGCORE_CONSOLE_SHELL_ENABLE
#include "agcore_console_shell.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <time.h>
#include <stdio.h>

#define AGCORE_LOG_TASK_STACK   (2048)
#define AGCORE_LOG_TASK_PRIO    (5)

static SemaphoreHandle_t g_log_notify;
static StaticSemaphore_t g_log_notify_buf;

static void agcore_log_output_task(void *arg);

/**
 * @brief EasyLogger 最终输出到 Console。
 * @param log 整行日志内容指针
 * @param size 整行日志字节数
 */
void agcore_log_port_output(const char *log, size_t size)
{
#ifdef CONFIG_AGCORE_CONSOLE_SHELL_ENABLE
    if (agcore_console_shell_is_ready()) {
        agcore_console_shell_write_external(log, size);
        return;
    }
#endif
    agcore_console_output_lock();
    agcore_console_output(log, size);
    agcore_console_output_unlock();
}

/**
 * @brief 异步输出通知。
 */
void agcore_log_async_notice(void)
{
    xSemaphoreGive(g_log_notify);
}

/**
 * @brief Log 专属异步输出 task: 等通知，逐行取缓冲并写入 Console。
 */
static void agcore_log_output_task(void *arg)
{
    static char buf[ELOG_LINE_BUF_SIZE - 4];
    size_t size;
    (void)arg;

    while (1) {
        xSemaphoreTake(g_log_notify, portMAX_DELAY);
        while ((size = elog_async_get_line_log(buf, sizeof(buf))) > 0) {
            agcore_log_port_output(buf, size);
        }
    }
}

/**
 * @brief 获取当前时间字符串(含时分秒)。
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
 * @brief agcore Log 初始化。
 * @note 日志必须初始化，失败不跳过。
 */
void agcore_log_init(void)
{
    const size_t log_fmt = ELOG_FMT_LVL | ELOG_FMT_TAG;
    bool console_output_init_ok;

    g_log_notify = xSemaphoreCreateBinaryStatic(&g_log_notify_buf);
    console_output_init_ok = agcore_console_output_init();
    if (!g_log_notify || !console_output_init_ok) {
        return;
    }

    if (elog_init() == ELOG_NO_ERR) {
        elog_set_fmt(ELOG_LVL_ASSERT,  log_fmt);
        elog_set_fmt(ELOG_LVL_ERROR,   log_fmt);
        elog_set_fmt(ELOG_LVL_WARN,    log_fmt);
        elog_set_fmt(ELOG_LVL_INFO,    log_fmt);
        elog_set_fmt(ELOG_LVL_DEBUG,   log_fmt);
        elog_set_fmt(ELOG_LVL_VERBOSE, log_fmt);
        elog_start();

        xTaskCreate(agcore_log_output_task, "elog_out", AGCORE_LOG_TASK_STACK, NULL, AGCORE_LOG_TASK_PRIO, NULL);
    }
}
