/**
 * @file agcore.h
 * @brief AGCORE 最底层公共基础头文件
 * @note 仅包含所有模块依赖的基础设施；模块接口由使用方按需包含。
 */
#ifndef AGCORE_H
#define AGCORE_H


#define AGCORE_VERSION_MAJOR    0
#define AGCORE_VERSION_MINOR    8
#define AGCORE_VERSION_PATCH    0

#define AGCORE_VERSION 0x03

/* ESP-IDF 配置 */
#include "sdkconfig.h"

/* 基础类型 */
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "inttypes.h"

/* 平台适配层（内存、时间、日志宏） */
#include "agcore_port.h"

#endif /* AGCORE_H */
