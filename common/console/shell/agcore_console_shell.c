#include "agcore_console_shell.h"
#include "agcore_console_log.h"
#include "letter_shell/shell.h"
#include "initcall.h"

#define TAG "Shell"

static Shell g_shell = {0};
static char g_shell_buffer[512];

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

static void agcore_shell_task(void *arg)
{
    char ch;

    (void)arg;

    while (1) {
        if (fread(&ch, 1, 1, stdin) == 1) {
            // printf("RX: 0x%02X\r\n", (unsigned char)ch);
            // fflush(stdout);
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

    shellInit(&g_shell, g_shell_buffer, sizeof(g_shell_buffer));
    agcore_console_output_lock();
    agcore_console_flush();
    agcore_console_output_unlock();

    /* 创建 Shell RX task */
    xTaskCreate(agcore_shell_task, "shell", 4096, NULL, 2, NULL);
    return 0;
}

AGCORE_SERVICE_INITCALL(agcore_console_shell_init);
