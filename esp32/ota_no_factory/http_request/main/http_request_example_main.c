
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "protocol_examples_common.h"
#include "string.h"

#include "esp_partition.h"
#include "esp_ota_ops.h"

static const char *TAG = "simple_ota_example";



void switch_partition() {
    // 获取当前运行的分区
    const esp_partition_t *current_partition = esp_ota_get_running_partition();

    if (current_partition == NULL) {
        ESP_LOGE(TAG, "Failed to get current running partition!");
        return;
    }

    ESP_LOGI(TAG, "Currently running partition: %s", current_partition->label);

    // 查找目标分区
    const esp_partition_t *target_partition = NULL;

    if (current_partition->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY) {
        ESP_LOGI(TAG, "Currently running from factory partition. Switching to custom partition...");

        // 查找 custom 分区（假定为 OTA 0 分区）
        target_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, "custom");

        if (target_partition == NULL) {
            ESP_LOGE(TAG, "Custom partition not found!");
            return;
        }
    } else {
        ESP_LOGI(TAG, "Currently running from custom partition. Switching to factory partition...");

        // 查找 factory 分区
        target_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);

        if (target_partition == NULL) {
            ESP_LOGE(TAG, "Factory partition not found!");
            return;
        }
    }

    // 设置目标分区为下次启动分区
    esp_err_t err = esp_ota_set_boot_partition(target_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set boot partition: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Boot partition set successfully! Rebooting...");
    esp_restart(); // 重启设备
}


void app_main(void)
{
    for(int i = 0; i < 5; i ++){
        vTaskDelay(100);
        ESP_LOGI(TAG, "wait %d s...", i);
    }
    ESP_LOGI(TAG, "Starting partition switch...");
    switch_partition();

}
