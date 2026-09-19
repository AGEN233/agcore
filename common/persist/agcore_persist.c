#include "agcore_persist.h"
#include "agcore_initcall.h"

/**
 * @brief 持久化模块初始化(当前无额外初始化工作)
 * @return ESP_OK
 */
esp_err_t agcore_persist_init(void)
{
    return ESP_OK;
}

AGCORE_CORE_INITCALL(agcore_persist_init);
