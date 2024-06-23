#include <string.h>
#include <sys/param.h>
#include "driver/i2c.h"

#define WRITE_BIT 0x00                      /*!< I2C master write */
#define READ_BIT 0x01                       /*!< I2C master read  */

#define ACK_CHECK_EN 0x1                    /*!< I2C master will check ack from slave     */
#define ACK_CHECK_DIS 0x0                   /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                         /*!< I2C ack value  */
#define NACK_VAL 0x1                        /*!< I2C nack value */

#if 0
#define IIC_CTRL_NUM I2C_NUM_1              /*!< I2C port number */
#define SDA_PIN_NUM 25                      /*!< gpio number for I2C data  */
#define SCL_PIN_NUM 26                      /*!< gpio number for I2C clock */
#else
#define IIC_CTRL_NUM I2C_NUM_0              /*!< I2C port number */
#define SDA_PIN_NUM 17                      /*!< gpio number for I2C data  */
#define SCL_PIN_NUM 18                      /*!< gpio number for I2C clock */
#endif

#define SHT3X_DeviceAddr (0x44<<1)          /* SHT3X的器件地址 */

/* 枚举SHT3x命令列表 */
typedef enum
{
    /* 软件复位命令 */
    SOFT_RESET_CMD = 0x30A2,	
    /* 单次测量模式
    命名格式：Repeatability_CS_CMD
    CS： Clock stretching */
    HIGH_ENABLED_CMD    = 0x2C06,
    MEDIUM_ENABLED_CMD  = 0x2C0D,
    LOW_ENABLED_CMD     = 0x2C10,
    HIGH_DISABLED_CMD   = 0x2400,
    MEDIUM_DISABLED_CMD = 0x240B,
    LOW_DISABLED_CMD    = 0x2416,

    /* 周期测量模式
    命名格式：Repeatability_MPS_CMD
    MPS：measurement per second */
    HIGH_0_5_CMD   = 0x2032,
    MEDIUM_0_5_CMD = 0x2024,
    LOW_0_5_CMD    = 0x202F,
    HIGH_1_CMD     = 0x2130,
    MEDIUM_1_CMD   = 0x2126,
    LOW_1_CMD      = 0x212D,
    HIGH_2_CMD     = 0x2236,
    MEDIUM_2_CMD   = 0x2220,
    LOW_2_CMD      = 0x222B,
    HIGH_4_CMD     = 0x2334,
    MEDIUM_4_CMD   = 0x2322,
    LOW_4_CMD      = 0x2329,
    HIGH_10_CMD    = 0x2737,
    MEDIUM_10_CMD  = 0x2721,
    LOW_10_CMD     = 0x272A,
	/* 周期测量模式读取数据命令 */
	READOUT_FOR_PERIODIC_MODE = 0xE000,
	/* 读取传感器编号命令 */
	READ_SERIAL_NUMBER = 0x3780,
} sht3x_cmd_t;

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = IIC_CTRL_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN_NUM;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = SCL_PIN_NUM;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;     /* 标准模式(100 kbit/s) */
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

/* 描述：向SHT30发送一条16bit指令 
 * 参数cmd：SHT30指令（在SHT30_MODE中枚举定义）
 * 返回值：成功返回ESP_OK                     */
static esp_err_t SHT3x_Send_Cmd(sht3x_cmd_t sht3x_cmd)
{
    uint8_t cmd_buffer[2];
    cmd_buffer[0] = sht3x_cmd >> 8;
    cmd_buffer[1] = sht3x_cmd;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT3X_DeviceAddr | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, cmd_buffer[0], ACK_CHECK_EN);
    i2c_master_write_byte(cmd, cmd_buffer[1], ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(IIC_CTRL_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
	
	return ret;
}

/* 描述：从SHT3x读取数据 
 * 参数data_len：读取多少个字节数据
 * 参数data_arr：读取的数据存放在一个数组里
 * 返回值：读取成功返回ESP_OK 
*/
static esp_err_t SHT3x_Recv_Data(size_t data_len, uint8_t* data_arr)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT3X_DeviceAddr | READ_BIT, ACK_CHECK_EN);
    if (data_len > 1) {
        i2c_master_read(cmd, data_arr, data_len - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_arr + data_len - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(IIC_CTRL_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

	return ret;
}

/* 描述：读取传感器编号
 * 参数：存储编号数据的指针
 * 返回值：0-读取成功，1-读取失败 */
uint8_t SHT3x_ReadSerialNumber(uint32_t* serialNumber)
{ 
	uint8_t Num_buf[4] = {0xFF,0xFF,0xFF,0xFF};
	
	SHT3x_Send_Cmd(READ_SERIAL_NUMBER);
	vTaskDelay(500 / portTICK_PERIOD_MS);   /* 延时50ms，有问题时需要适当延长！！！！！！*/
	esp_err_t ret = SHT3x_Recv_Data(4,Num_buf);
	
	*serialNumber = ((Num_buf[0] << 24) | (Num_buf[1] << 16) |(Num_buf[2] << 8) |(Num_buf[3]));
	if(0xFFFFFFFF == *serialNumber) return 1;
    else if(ret == ESP_OK) return 0;
    printf("SHT3x_ReadSerialNumber ERR :%s\n",esp_err_to_name(ret)); 
    return ret;
}

/* 描述：SHT3x初始化函数，并将其设置为周期测量模式
 * 参数：无
 * 返回值：初始化成功返回ESP_OK */
esp_err_t sht3x_mode_init(void)
{
    /* 初始化IIC控制器 */
    ESP_ERROR_CHECK(i2c_master_init());
    /* 设置为周期测量模式 */
    esp_err_t ret = SHT3x_Send_Cmd(MEDIUM_2_CMD);
	return ret;
}

/* 描述：数据CRC校验
 * 参数message：需要校验的数据
 * 参数initial_value：crc初始值
 * 返回值：计算得到的CRC码 */
#define CRC8_POLYNOMIAL 0x131
static uint8_t CheckCrc8(uint8_t* const message, uint8_t initial_value)
{
    uint8_t  remainder;	    //余数
    uint8_t  i = 0, j = 0;  //循环变量

    /* 初始化 */
    remainder = initial_value;
    for(j = 0; j < 2;j++)
    {
        remainder ^= message[j];
        /* 从最高位开始依次计算  */
        for (i = 0; i < 8; i++)
        {
            if (remainder & 0x80)
                remainder = (remainder << 1)^CRC8_POLYNOMIAL;
            else
                remainder = (remainder << 1);
        }
    }
    /* 返回计算的CRC码 */
    return remainder;
}

/* 描述：温湿度数据获取函数,周期读取，注意，需要提前设置周期模式   
 * 参数Tem_val：存储温度数据的指针, 温度单位为°C
 * 参数Hum_val：存储湿度数据的指针, 温度单位为%
 * 返回值：0-读取成功，1-读取失败 **********************************/
uint8_t sht3x_get_humiture_periodic(double *Tem_val,double *Hum_val)
{
	uint8_t ret=0;
	uint8_t buff[6];
	uint16_t tem,hum;
	double Temperature=0;
	double Humidity=0;

	ret = SHT3x_Send_Cmd(READOUT_FOR_PERIODIC_MODE);	
	ret = SHT3x_Recv_Data(6,buff);
	
	/* 校验温度数据和湿度数据是否接收正确 */
	if(CheckCrc8(buff, 0xFF) != buff[2] || CheckCrc8(&buff[3], 0xFF) != buff[5])
	{	
		printf("CRC_ERROR,ret = 0x%x\r\n",ret);
		return 1;
	}
		
	/* 转换温度数据 */
	tem = (((uint16_t)buff[0]<<8) | buff[1]);//温度数据拼接
	Temperature= (175.0*(double)tem/65535.0-45.0) ;	// T = -45 + 175 * tem / (2^16-1)
	
	/* 转换湿度数据 */
	hum = (((uint16_t)buff[3]<<8) | buff[4]);//湿度数据拼接
	Humidity= (100.0*(double)hum/65535.0);			// RH = hum*100 / (2^16-1)
	
	/* 过滤错误数据 */
	if((Temperature>=-20)&&(Temperature<=125)&&(Humidity>=0)&&(Humidity<=100))
	{
		*Tem_val = Temperature;
		*Hum_val = Humidity;
		return 0;
	}
	else
		return 1;
}
