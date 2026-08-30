#ifndef __AGCORE_CORE_CMD_H__
#define __AGCORE_CORE_CMD_H__

#include "agcore_data.h"
#include "agcore_version.h"
#include "agcore_bytes.h"
#include "agcore_log.h"
#include <inttypes.h>

/* CORE 公共命令号 */
#define CORE_CMD_GET_VERINFO        0xAC00      /* 查询设备版本信息 */
#define CORE_CMD_GET_DEVICETIME     0xAC01      /* 查询设备时间戳  */
#define CORE_CMD_SET_DEVICETIME     0xAC02      /* 设置设备时间    */

#endif /* __AGCORE_CORE_CMD_H__ */
