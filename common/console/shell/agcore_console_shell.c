#include "agcore_console_shell.h"
#include "agcore_initcall.h"

/**
 * @brief 初始化 console shell。
 *
 * 当前尚无 shell 后端；保留独立初始化入口供后续接入，并使其可由
 * 安全校验模块单独控制。
 */
int agcore_console_shell_init(void)
{
    return 0;
}

AGCORE_SERVICE_INITCALL(agcore_console_shell_init);
