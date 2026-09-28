#ifndef __AGCORE_CONSOLE_SHELL_H__
#define __AGCORE_CONSOLE_SHELL_H__

#include "agcore_console.h"

bool agcore_console_shell_is_ready(void);
void agcore_console_shell_write_external(const char *data, size_t size);
int agcore_console_shell_init(void);


#endif /* __AGCORE_CONSOLE_SHELL_H__ */
