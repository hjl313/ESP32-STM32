
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

#define OTA_PARTION_SIZE 1600000

static bool check_partition_erased(const esp_partition_t *partition) 
{
    // 分配一个足够大的缓冲区来读取分区内容
    static uint8_t read_data[10240]; // 假设缓冲区大小为4KB

    // 读取分区内容
    esp_partition_read(partition, 0, read_data, sizeof(read_data));

    // 检查读取的数据是否全部为0xFF
    for (int i = 0; i < sizeof(read_data); ++i) {
        if (read_data[i] != 0xFF) {
            ESP_LOGI(TAG, "Partition is not erased.\n");
            return false; // 一旦找到不是0xFF的字节，就终止检查
        }
    }
    ESP_LOGI(TAG, "Partition is erased.\n");
    return true;
}

#define BUFFER_SIZE 4096  // 缓冲区大小（4KB）
int http_ota_start(const char *url) 
{
    esp_err_t err;

    //获取当前运行分区
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    if (running_partition == NULL) {
        ESP_LOGE(TAG, "Failed to get current running partition!");
        return -1;
    }

    bool use_custom_partion = false;//使用factory分区或custom分区来存储新固件
    if (strcmp(running_partition->label, "factory") == 0) {
        use_custom_partion = true;
    }

    // 查找OTA分区
    esp_partition_t *partition = NULL;
    char ota_partition_name[32] = {0};
    if(use_custom_partion){
        strcpy(ota_partition_name, "custom");
        partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, ota_partition_name);
    }else{
        strcpy(ota_partition_name, "factory");
        partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, ota_partition_name);
    }
    if (partition == NULL) {
        ESP_LOGE(TAG, "ota partition %s not found!", ota_partition_name);
        return -1;
    }
    ESP_LOGI(TAG, "ota partition %s found at offset: %ld, size: %ld", ota_partition_name, partition->address, partition->size);

    //擦除分区
    ESP_LOGI(TAG, "Erasing ota partition %s ...", ota_partition_name);
    err = esp_partition_erase_range(partition, 0, partition->size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase ota partition %s : %s", ota_partition_name, esp_err_to_name(err));
        return -1;
    }
    ESP_LOGI(TAG, "ota partition %s  erased successfully!", ota_partition_name);

    // 初始化 HTTP 客户端配置
    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = 5000,  // 超时时间
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // 开始 HTTP 连接
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return -1;
    }

    // 获取内容长度
    int content_length = esp_http_client_fetch_headers(client);
    if (content_length <= 0) {
        ESP_LOGE(TAG, "Invalid content length: %d", content_length);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return -1;
    }
    ESP_LOGI(TAG, "Firmware size: %d bytes", content_length);
    if(content_length >= OTA_PARTION_SIZE){
        ESP_LOGE(TAG, "The firmware is too large to download");
        return -1;
    }

    // 下载并写入数据
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
        // 写入到 OTA 分区
        err = esp_partition_write(partition, total_bytes_written, buffer, bytes_read);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write to %s partition: %s", ota_partition_name, esp_err_to_name(err));
            break;
        }
        total_bytes_written += bytes_read;
        ESP_LOGI(TAG, "Written %d bytes to %s partition", total_bytes_written, ota_partition_name);
    }

    // 关闭 HTTP 连接
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (total_bytes_written == content_length) {
        ESP_LOGI(TAG, "File downloaded and written to %s partition successfully!", ota_partition_name);
    } else {
        ESP_LOGE(TAG, "Failed to download/write the entire file");
        return -1;
    }

    // 设置 ota 分区为启动分区
    err = esp_ota_set_boot_partition(partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set boot %s partition: %s", ota_partition_name, esp_err_to_name(err));
        return -1;
    }

    ESP_LOGI(TAG, "Boot partition set to %s successfully! ", ota_partition_name);
    return 0;
}

void app_main(void)
{
    for(int i = 0; i < 3; i ++){
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

    const char *url = "http://192.168.3.238/http_request.bin";
    http_ota_start(url);

    // esp_restart(); // 重启设备

}
