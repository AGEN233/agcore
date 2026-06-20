#include "agcore.h"

#include <stdio.h>

#include "sdkconfig.h"
#ifdef CONFIG_AGCORE_BLE_ENABLE
#include "agcore_ble.h"
#endif

void agcore_init(void)
{
    const agcore_version_info_st *version_info = agcore_version_info_get();
    printf("--> AGCORE VERSION\n"
           "--> VERSION: %04X_%04X\n"
           "--> GIT: %s\n"
           "--> BRANCH: %s\n"
           "--> build: %s\n",
           version_info->version,
           version_info->id,
           version_info->git_hash,
           version_info->git_branch,
           version_info->build_time);

    // core chanel init


    // data chanel_init
    agcore_data_queue_init();


    // ble init
    agcore_ble_init();
}
