
#include <stdio.h>
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt_main.h"
#include "freertos/task.h"

#include <nvs_flash.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_netif_ip_addr.h>


#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_system.h"
#include "esp_spiffs.h"

/* 初始化SPIFFS分区*/
void initSPIFFS(void)
{
    printf("%s\n", "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf =
        {
            .base_path = "/spiffs",
            .partition_label = NULL,
            .max_files = 5,
            .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            printf("%s", "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            printf("%s", "Failed to find SPIFFS partition");
        }
        else
        {
            printf("Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total, used;
    if (esp_spiffs_info(NULL, &total, &used) == ESP_OK)
    {
        printf("Partition size: total: %d, used: %d", total, used);
    }
}

#define BASE_PATH "/spiffs/"
/* 将保存的文本文件内容全部读取完毕打印出来*/
int read_file(const char *file_name)
{
    char file_path[64] = {0};
    sprintf(file_path, "%s%s", BASE_PATH, file_name);

    FILE *file = fopen(file_path, "r");
    if (file == NULL)
    {
        printf("无法打开文件 %s\n", file_path);
        return -1;
    }

    printf("\n\n\n");
    size_t bytesRead;
    char buffer[100] = {0};
    while ((bytesRead = fread(buffer, sizeof(char), 100, file)) > 0)
    {
        printf("%s", buffer);
    }
    printf("\n\n\n");
    fclose(file);

    return 0;
}

/* 将扫描结果追加到spiff文本文件中 */
int append_file(const char *file_name, char *buffer)
{
    char file_path[64] = {0};
    sprintf(file_path, "%s%s", BASE_PATH, file_name);

    FILE *file = fopen(file_path, "a+");
    if (file == NULL)
    {
        printf("无法打开文件 %s\n", file_path);
        return -1;
    }
    fprintf(file, "%s\n", buffer);
    fclose(file);

    return 0;
}

/* 是否已经添加过 */
int is_appended(const char *file_name, char *dev_name)
{
    char file_path[64] = {0};
    sprintf(file_path, "%s%s", BASE_PATH, file_name);

    FILE *file = fopen(file_path, "r");
    if (file == NULL)
    {
        printf("无法打开文件 %s\n", file_path);
        return 0;
    }
    char line[100] = {0};
    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, dev_name) != NULL)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

/* 定义回调函数以处理扫描结果 */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    if (event == ESP_GAP_BLE_SCAN_RESULT_EVT)
    {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        if (scan_result->scan_rst.adv_data_len > 0)
        {
            // 在广播数据中查找设备名称
            char dev_name[32] = {0};
            uint8_t *adv_data = scan_result->scan_rst.ble_adv;
            int adv_data_len = scan_result->scan_rst.adv_data_len;
            for (int i = 0; i < adv_data_len; ++i)
            {
                if (adv_data[i + 1] == ESP_BLE_AD_TYPE_NAME_CMPL ||
                    adv_data[i + 1] == ESP_BLE_AD_TYPE_NAME_SHORT)
                {
                    memcpy(dev_name, &adv_data[i + 2], adv_data[i] - 1);
                    dev_name[adv_data[i] - 1] = '\0';
                    break;
                }
            }
            // 剔除没有设备名的蓝牙设备
            if (0 == strlen(dev_name))
                return;

            char buffer[128] = {0};
            sprintf(buffer, "addr:%02X:%02X:%02X:%02X:%02X:%02X name:%s", scan_result->scan_rst.bda[0], scan_result->scan_rst.bda[1], scan_result->scan_rst.bda[2],
                    scan_result->scan_rst.bda[3], scan_result->scan_rst.bda[4], scan_result->scan_rst.bda[5], dev_name);

            printf("ble device:\n");
            printf("%s\n\n", buffer);

            // 已经保存过的设备不再保存
            if (!is_appended("ble.txt", dev_name))
                append_file("ble.txt", buffer);
        }
    }
}

void Bluetooth_test()
{
    esp_err_t ret;

    /* 初始化 ESP-IDF 的 Bluetooth 子系统 */
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK)
    {
        printf("初始化 Bluetooth 控制器失败\n");
        return;
    }

    /* 配置 Bluetooth 控制器并启动 */
    ret = esp_bt_controller_enable(1);
    if (ret != ESP_OK)
    {
        printf("启用 Bluetooth 控制器失败\n");
        return;
    }

    /* 初始化 Bluedroid 栈 */
    ret = esp_bluedroid_init();
    if (ret != ESP_OK)
    {
        printf("初始化 Bluedroid 栈失败\n");
        return;
    }

    /* 启用 Bluedroid 栈 */
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK)
    {
        printf("启用 Bluedroid 栈失败\n");
        return;
    }

    /* 设置 GAP 回调函数以处理扫描结果 */
    ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret != ESP_OK)
    {
        printf("注册 GAP 回调函数失败\n");
        return;
    }

    while (1) //循环会造成重启，虽然一直重启，但重启后也在接着扫描
    {
        /* 开始蓝牙扫描 */
        ret = esp_ble_gap_start_scanning(0);
        if (ret != ESP_OK)
        {
            printf("启动蓝牙扫描失败\n");
            return;
        }

        /* 休眠一段时间，以便执行扫描操作 */
        vTaskDelay(50);

        /* 停止蓝牙扫描 */
        esp_ble_gap_stop_scanning();
        vTaskDelay(100);
    }

    /* 程序结束 */
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
}

void app_main(void)
{
    initSPIFFS();
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        return;
    }

    printf("\n已经保存的数据：\n");
    read_file("ble.txt"); // 读取保存的文件打印到串口上

    printf("\n本次开机扫描的数据：\n");
    Bluetooth_test();
}
