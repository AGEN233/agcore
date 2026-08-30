#include "agcore.h"

#include <stdio.h>

#include "sdkconfig.h"

#include "agcore_console.h"
#include "agcore_lifecycle.h"
#include "agcore_persist.h"
#include "agcore_version.h"
#include "agcore_data.h"
#ifdef CONFIG_AGCORE_BLE_ENABLE
#include "agcore_ble.h"
#endif

/**
 * @brief AGCORE 总初始化入口: console/persist/lifecycle/数据管道/蓝牙
 */
void agcore_init(void)
{
    agcore_console_init();
    agcore_persist_init();
    agcore_lifecycle_init();

    const agcore_version_info_t *version_info = agcore_version_info_get();
    printf("--> AGCORE VERSION\n"
           "--> VERSION: %04X\n"
           "--> GIT: %s\n"
           "--> BRANCH: %s\n"
           "--> build: %s\n",
           version_info->version,
           version_info->git_hash,
           version_info->git_branch,
           version_info->build_time);

    // core pipe init


    // data pipe init
    agcore_data_queue_init();


    // ble init
    #ifdef CONFIG_AGCORE_BLE_ENABLE
    agcore_ble_init();
    #endif
}
