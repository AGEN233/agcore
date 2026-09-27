#ifndef __AGCORE_SHELL_CFG_USER_H__
#define __AGCORE_SHELL_CFG_USER_H__

#include "agcore_console_shell.h"

/* 尾行模式 */
#define SHELL_SUPPORT_END_LINE              1

/* 不使用 Companion */
#define SHELL_USING_COMPANION               0

/* 禁止执行未导出的函数地址 */
#define SHELL_EXEC_UNDEF_FUNC               0

/* 不使用普通函数签名调用，仅使用 argc / argv CMD_MAIN */
#define SHELL_USING_FUNC_SIGNATURE          0

/* 不使用数组参数 */
#define SHELL_SUPPORT_ARRAY_PARAM           0

/* help 中不显示用户 */
#define SHELL_HELP_LIST_USER                0

/* help 中不显示变量 */
#define SHELL_HELP_LIST_VAR                 0

/* 时间戳接口  */
#define SHELL_GET_TICK                      agcore_get_timems

/* shell名  */
#define SHELL_DEFAULT_USER                  "AGCORE"

/* 关闭启动log */
#define SHELL_SHOW_INFO                     0
#endif /* __AGCORE_SHELL_CFG_USER_H__ */