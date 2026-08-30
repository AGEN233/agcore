#ifndef __AGCORE_CONSOLE_H__
#define __AGCORE_CONSOLE_H__

#include <stddef.h>     /* size_t */

#ifdef __cplusplus
extern "C" {
#endif

void agcore_console_init(void);
void agcore_console_lock(void);
void agcore_console_unlock(void);

/**
 * @brief 输出一段文本到 console(内部持共享锁, 保证 log/esh 写不互扰)
 * @param buf 文本内容指针
 * @param len 文本字节数
 */
void agcore_console_write(const char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __AGCORE_CONSOLE_H__ */
