#include "agcore.h"

#include "sdkconfig.h"
#ifdef CONFIG_AGCORE_BLE_ENABLE
#include "agcore_ble.h"
#endif

void agcore_init(void)
{
    // core chanel init


    // data chanel_init
    agcore_data_queue_init();


    // ble init
    agcore_ble_init();
}
