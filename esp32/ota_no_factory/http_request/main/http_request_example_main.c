
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


//切换分区
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


#define BUFFER_SIZE 4096  // 缓冲区大小（4KB）
void download_and_write_to_custom_partition(const char *url) {
    esp_err_t err;

    // 1. 获取当前运行分区
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    if (running_partition == NULL) {
        ESP_LOGE(TAG, "Failed to get current running partition!");
        return;
    }

    // 2. 查找 custom 分区
    const esp_partition_t *custom_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, "custom");

    if (custom_partition == NULL) {
        ESP_LOGE(TAG, "Custom partition not found!");
        return;
    }


    ESP_LOGI(TAG, "Custom partition found at offset: %ld, size: %ld",custom_partition->address, custom_partition->size);

    // 检查是否正在运行 custom 分区
    if (strcmp(running_partition->label, "custom") == 0) {
        ESP_LOGE(TAG, "Cannot erase the currently running partition (custom)!");
        return;
    }

    // 3. 擦除 custom 分区
    ESP_LOGI(TAG, "Erasing custom partition...");
    err = esp_partition_erase_range(custom_partition, 0, custom_partition->size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase custom partition: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "Custom partition erased successfully!");

    // 3. 初始化 HTTP 客户端配置
    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = 5000,  // 超时时间
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // 4. 开始 HTTP 连接
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }

    // 5. 获取内容长度
    int content_length = esp_http_client_fetch_headers(client);
    if (content_length <= 0) {
        ESP_LOGE(TAG, "Invalid content length: %d", content_length);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return;
    }

    ESP_LOGI(TAG, "Content length: %d bytes", content_length);

    // 6. 下载并写入数据
    char buffer[BUFFER_SIZE];
    int total_bytes_written = 0;

    while (1) {
        int bytes_read = esp_http_client_read(client, buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            ESP_LOGE(TAG, "Failed to read data from HTTP stream");
            break;
        } else if (bytes_read == 0) {
            ESP_LOGI(TAG, "Download complete");
            break;
        }

        // 写入到 custom 分区
        err = esp_partition_write(custom_partition, total_bytes_written, buffer, bytes_read);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write to custom partition: %s", esp_err_to_name(err));
            break;
        }

        total_bytes_written += bytes_read;
        ESP_LOGI(TAG, "Written %d bytes to custom partition", total_bytes_written);
    }

    // 7. 关闭 HTTP 连接
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (total_bytes_written == content_length) {
        ESP_LOGI(TAG, "File downloaded and written to custom partition successfully!");
    } else {
        ESP_LOGE(TAG, "Failed to download/write the entire file");
        return;
    }

    // 8. 设置 custom 分区为启动分区
    err = esp_ota_set_boot_partition(custom_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set boot partition: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Boot partition set to custom successfully! Rebooting...");
    esp_restart(); // 重启设备
}

void app_main(void)
{
    for(int i = 0; i < 5; i ++){
        vTaskDelay(100);
        ESP_LOGI(TAG, "wait %d s...", i);
    }

    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());


    const char *url = "http://hearit.ai/d/HA-TOY/HA-TOY01.bin";
    download_and_write_to_custom_partition(url);
}
