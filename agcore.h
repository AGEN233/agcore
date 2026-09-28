/**
 * @file agcore.h
 * @brief AGCORE 面向应用层的公共接口入口
 * @note 模块头按需包含其他模块头和 agcore_port.h，不反向包含本聚合头。
 */
#ifndef __AGCORE_H__
#define __AGCORE_H__

#define AGCORE_VERSION_MAJOR    0
#define AGCORE_VERSION_MINOR    9
#define AGCORE_VERSION_PATCH    5

#define AGCORE_VERSION          0x03

#include "agcore_port.h"

#include "agcore_bytes.h"
#include "agcore_check.h"
#include "agcore_version.h"
#include "agcore_console_log.h"
#include "agcore_console_shell.h"
#include "agcore_persist.h"
#include "agcore_nvs.h"
#include "agcore_fs.h"
#include "agcore_flash.h"
#include "agcore_lifecycle.h"
#include "agcore_data.h"
#include "agcore_ble.h"
#include "display_driver.h"
#include "pixel_driver.h"

#endif /* AGCORE_H */
