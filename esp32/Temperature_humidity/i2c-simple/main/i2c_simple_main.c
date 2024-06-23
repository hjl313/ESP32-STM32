#include <string.h>
#include <sys/param.h>
#include "driver/i2c.h"

#define GPIO_LED_NUM 2                      /* LED引脚编号 */

esp_err_t sht3x_mode_init(void);
uint8_t sht3x_get_humiture_periodic(double *Tem_val,double *Hum_val);

void app_main(void)
{
    double Tem_val,Hum_val;
    /* 打印Hello world! */
    printf("Hello world!\n");

    ESP_ERROR_CHECK(sht3x_mode_init());

    /* 定义一个gpio配置结构体 初始化LED */
    gpio_config_t gpio_config_structure;

    /* 初始化gpio配置结构体*/
    gpio_config_structure.pin_bit_mask = (1ULL << GPIO_LED_NUM);/* 选择gpio2 */
    gpio_config_structure.mode = GPIO_MODE_OUTPUT;              /* 输出模式 */
    gpio_config_structure.pull_up_en = 0;                       /* 不上拉 */
    gpio_config_structure.pull_down_en = 0;                     /* 不下拉 */
    gpio_config_structure.intr_type = GPIO_INTR_DISABLE;    /* 禁止中断 */ 

    /* 根据设定参数初始化并使能 */  
	gpio_config(&gpio_config_structure);
    /* 默认熄灭LED */
    gpio_set_level(GPIO_LED_NUM, 0);        /* 熄灭 */

    while(1)
    {
        /* 采集温湿度数据 */
        if(sht3x_get_humiture_periodic(&Tem_val,&Hum_val) == 0)
        {
            printf("temperature:%6.2lf °C, humidity:%6.2lf %% \n",Tem_val,Hum_val);
        }
        else
            printf("Get_Humiture ERR\r\n");

        gpio_set_level(GPIO_LED_NUM, 0);        /* 熄灭 */
        vTaskDelay(500 / portTICK_PERIOD_MS);   /* 延时500ms*/
        gpio_set_level(GPIO_LED_NUM, 1);        /* 点亮 */
        vTaskDelay(500 / portTICK_PERIOD_MS);   /* 延时500ms*/
    }
}
