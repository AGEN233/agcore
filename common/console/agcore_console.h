#ifndef __AGCORE_CONSOLE_H__
#define __AGCORE_CONSOLE_H__

#include "agcore_port.h"

void agcore_console_output_lock(void);
void agcore_console_output_unlock(void);
void agcore_console_output(const char *data, size_t size);
void agcore_console_flush(void);
bool agcore_console_output_init(void);


#endif /* __AGCORE_CONSOLE_H__ */
