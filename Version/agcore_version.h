#ifndef __AGCORE_VERSION_H__
#define __AGCORE_VERSION_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define AGCORE_GIT_HASH      "@AGCORE_GIT_HASH@"
#define AGCORE_GIT_BRANCH    "@AGCORE_GIT_BRANCH@"
#define AGCORE_BUILD_TIME    "@AGCORE_BUILD_TIME@"

#ifndef AGCORE_VERSION
#define AGCORE_VERSION       0x01
#endif

#ifndef AGCORE_ID
#define AGCORE_ID            0x01
#endif

#ifndef AGCORE_VRESION
#define AGCORE_VRESION       AGCORE_VERSION
#endif

typedef struct {
    uint16_t device_type;
    uint16_t device_id;
    uint16_t fw_version;
    uint16_t hw_version;
} agcore_device_info_st;

typedef struct {
    uint16_t version;
    uint16_t id;
    const char *git_hash;
    const char *git_branch;
    const char *build_time;
} agcore_version_info_st;

void agcore_device_info_set(const agcore_device_info_st *info);
bool agcore_device_info_is_set(void);
const agcore_device_info_st *agcore_device_info_get(void);
const agcore_version_info_st *agcore_version_info_get(void);
void agcore_device_info_get_bytes(uint8_t *buf, uint16_t len);

uint16_t agcore_device_info_get_type(void);
uint16_t agcore_device_info_get_id(void);
uint16_t agcore_device_info_get_fw_version(void);
uint16_t agcore_device_info_get_hw_version(void);
uint16_t agcore_version_info_get_version(void);
uint16_t agcore_version_info_get_id(void);

int agcore_device_info_get_string(char *buf, size_t buf_len);
int agcore_version_info_get_string(char *buf, size_t buf_len);

#endif /* __AGCORE_VERSION_H__ */
