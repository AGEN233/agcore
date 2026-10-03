#ifndef __AGCORE_CONSOLE_SHELL_H__
#define __AGCORE_CONSOLE_SHELL_H__

#include "agcore_console.h"
#include "letter_shell/shell.h"

#define AGCORE_SHELL_CMD(name, func, desc) \
    SHELL_EXPORT_CMD( \
        SHELL_CMD_PERMISSION(0) | \
        SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | \
        SHELL_CMD_DISABLE_RETURN, \
        name, func, desc)
#define AGCORE_SHELL_CMD_FUNC(name, func, desc) \
    SHELL_EXPORT_CMD( \
        SHELL_CMD_PERMISSION(0) | \
        SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC) | \
        SHELL_CMD_DISABLE_RETURN, \
        name, func, desc)

bool agcore_console_shell_is_ready(void);
void agcore_console_shell_write_external(const char *data, size_t size);
int agcore_console_shell_init(void);


#endif /* __AGCORE_CONSOLE_SHELL_H__ */
