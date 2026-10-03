#include "agcore_console_shell.h"
#include "agcore_console_log.h"
#include "letter_shell/shell.h"
#include "initcall.h"

#define TAG "Shell"

static Shell g_shell = {0};
static char *g_shell_buffer = NULL;
static bool g_shell_ready = false;

/**
 * @brief shell输出接口
 */
static signed short agcore_shell_write(char *data, unsigned short len)
{
    agcore_console_output_lock();
    agcore_console_output(data, len);
    agcore_console_output_unlock();
    return (signed short)len;
}

/**
 * @brief shell就绪
 * @return true
 * @return false
 */
bool agcore_console_shell_is_ready(void)
{
    return g_shell_ready;
}

/**
 * @brief shell外部输出接口
 * @param data
 * @param size
 */
void agcore_console_shell_write_external(const char *data, size_t size)
{
    if (!g_shell_ready) {
        return;
    }

    shellWriteEndLine(&g_shell, (char *)data, size);

    agcore_console_output_lock();
    agcore_console_flush();
    agcore_console_output_unlock();
}

/**
 * @brief shell task
 * @param arg
 */
static void agcore_shell_task(void *arg)
{
    char ch;

    (void)arg;

    while (1) {
        if (fread(&ch, 1, 1, stdin) == 1) {
            shellHandler(&g_shell, ch);
            agcore_console_output_lock();
            agcore_console_flush();
            agcore_console_output_unlock();
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

/**
 * @brief 初始化 console shell。
 */
int agcore_console_shell_init(void)
{
    g_shell.write = agcore_shell_write;

    g_shell_buffer = agcore_malloc_pram(CONFIG_AGCORE_CONSOLE_SHELL_BUF_SIZE);
    if (!g_shell_buffer) return -1;

    shellInit(&g_shell, g_shell_buffer, CONFIG_AGCORE_CONSOLE_SHELL_BUF_SIZE);
    agcore_console_output_lock();
    agcore_console_flush();
    agcore_console_output_unlock();

    g_shell_ready = true;

    /* 创建 Shell RX task */
    xTaskCreate(agcore_shell_task, "shell", 2048, NULL, 2, NULL);
    return 0;
}

#ifdef CONFIG_AGCORE_CONSOLE_SHELL_ENABLE
AGCORE_SERVICE_INITCALL(agcore_console_shell_init);
#endif
