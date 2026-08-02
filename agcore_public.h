/**
 * @file agcore_public.h
 * @brief AGCORE 最底层公共基础头文件
 * @note  仅平铺「所有模块都依赖的最底层基础设施」:ESP-IDF 配置、基础类型、平台适配。
 *        模块级头文件(日志/数据通道/工具/版本/外设驱动/ble)不在此平铺,
 *        由各使用方按需显式 include,模块边界层层嵌套、拒绝跨模块平铺。
 */
#ifndef __AGCORE_PUBLIC_H__
#define __AGCORE_PUBLIC_H__

/* ESP-IDF 配置 */
#include "sdkconfig.h"

/* 基础类型 */
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "inttypes.h"

/* 平台适配层(内存/时间/日志宏) */
#include "agcore_port.h"

#endif /* __AGCORE_PUBLIC_H__ */
