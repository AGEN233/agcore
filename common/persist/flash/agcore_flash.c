#include "agcore_flash.h"

#include "sdkconfig.h"

#include "esp_partition.h"

/**
 * @brief 获取数据分区句柄(按 Kconfig 配置的分区标签)
 * @return 分区指针, 未找到返回 NULL
 */
static const esp_partition_t *agcore_flash_get_partition(void)
{
    return esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                    ESP_PARTITION_SUBTYPE_ANY,
                                    CONFIG_AGCORE_FLASH_PARTITION_LABEL);
}

/**
 * @brief 从数据分区读取数据
 * @param offset 分区内偏移
 * @param buf 输出缓冲
 * @param len 读取长度
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_flash_read(size_t offset, void *buf, size_t len)
{
    const esp_partition_t *partition = agcore_flash_get_partition();
    if (!partition) {
        return ESP_ERR_NOT_FOUND;
    }
    return esp_partition_read(partition, offset, buf, len);
}

/**
 * @brief 向数据分区写入数据
 * @param offset 分区内偏移
 * @param buf 数据缓冲
 * @param len 写入长度
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_flash_write(size_t offset, const void *buf, size_t len)
{
    const esp_partition_t *partition = agcore_flash_get_partition();
    if (!partition) {
        return ESP_ERR_NOT_FOUND;
    }
    return esp_partition_write(partition, offset, buf, len);
}

/**
 * @brief 擦除数据分区指定范围
 * @param offset 分区内偏移
 * @param len 擦除长度
 * @return ESP_OK 成功, 否则错误码
 */
esp_err_t agcore_flash_erase(size_t offset, size_t len)
{
    const esp_partition_t *partition = agcore_flash_get_partition();
    if (!partition) {
        return ESP_ERR_NOT_FOUND;
    }
    return esp_partition_erase_range(partition, offset, len);
}
