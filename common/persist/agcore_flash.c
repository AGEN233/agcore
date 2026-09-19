#include "agcore_flash.h"

#include "sdkconfig.h"

#include "esp_partition.h"

static const esp_partition_t *agcore_flash_get_partition(void)
{
    return esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                    ESP_PARTITION_SUBTYPE_ANY,
                                    CONFIG_AGCORE_FLASH_PARTITION_LABEL);
}

esp_err_t agcore_flash_read(size_t offset, void *buf, size_t len)
{
    const esp_partition_t *partition = agcore_flash_get_partition();
    if (!partition) {
        return ESP_ERR_NOT_FOUND;
    }
    return esp_partition_read(partition, offset, buf, len);
}

esp_err_t agcore_flash_write(size_t offset, const void *buf, size_t len)
{
    const esp_partition_t *partition = agcore_flash_get_partition();
    if (!partition) {
        return ESP_ERR_NOT_FOUND;
    }
    return esp_partition_write(partition, offset, buf, len);
}

esp_err_t agcore_flash_erase(size_t offset, size_t len)
{
    const esp_partition_t *partition = agcore_flash_get_partition();
    if (!partition) {
        return ESP_ERR_NOT_FOUND;
    }
    return esp_partition_erase_range(partition, offset, len);
}
