#ifndef __AGCORE_H__
#define __AGCORE_H__

#include "sdkconfig.h"
#include "agcore_port.h"

#include "agcore_printf_log.h"
#include "app_printf_log.h"
#include "agcore_datachannel.h"
#include "agcore_utils.h"

#define AGCORE_VERSION       0x01
#define AGCORE_ID            0x01
#define AGCORE_VRESION       AGCORE_VERSION

#include "Version/agcore_version.h"
#include "pixel_driver.h"
#include "agcore_ble.h"

void agcore_init(void);
void agcore_set_device_info(const agcore_device_info_st *info);
const agcore_device_info_st *agcore_get_device_info(void);

const agcore_version_info_st *agcore_get_agcore_info(void);
uint16_t agcore_get_device_type(void);
uint16_t agcore_get_device_id(void);
uint16_t agcore_get_fw_version(void);
uint16_t agcore_get_hw_version(void);
uint16_t agcore_get_agcore_version(void);
uint16_t agcore_get_agcore_id(void);
int agcore_get_device_identifier(char *buf, size_t buf_len);
int agcore_get_agcore_identifier(char *buf, size_t buf_len);

#endif /* AGCORE_H */
