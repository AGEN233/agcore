#ifndef __AGCORE_CONSOLE_SHELL_H__
#define __AGCORE_CONSOLE_SHELL_H__

#include "agcore_console.h"

/**
 * @brief 初始化 console shell。
 *
 * shell 与 console/log 初始化分离，以便调用方按需决定是否启动 shell。
 */
int agcore_console_shell_init(void);


#endif /* __AGCORE_CONSOLE_SHELL_H__ */
