/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */
 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <elog.h>

#include "agcore_console.h"
#include "agcore_log_adapter.h"

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    /* add your code here */
    /* 日志必须初始化: 建锁/建异步 task 已由 adapter 的 agcore_log_init 内联完成,
       此处按 EasyLogger 约定总是返回成功(枚举仅有 ELOG_NO_ERR), 不引入失败跳过路径 */
    return ELOG_NO_ERR;
}

/**
 * EasyLogger port deinitialize
 *
 */
void elog_port_deinit(void) {
    /* add your code here */
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    /* add your code here */
    agcore_log_port_output(log, size);
}

/**
 * output lock
 * @note 锁集中于 agcore_console_write 内部统一持共享锁, 此处不重复加锁,
 *       以避免同一把非递归 mutex 在(外层 elog 锁 + write 内层锁)下重入死锁。
 */
void elog_port_output_lock(void) {
    /* add your code here */
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    /* add your code here */
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    /* add your code here */
    return agcore_log_port_get_time();
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    /* add your code here */
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    /* add your code here */
    return pcTaskGetName(NULL);
}

/* elog_async.c 在非 pthread 模式下 extern 引用该符号, 转调 adapter */
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
void elog_async_output_notice(void) {
    agcore_log_async_notice();
}
#endif
