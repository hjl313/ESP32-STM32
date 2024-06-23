// https://blog.csdn.net/boybs/article/details/125952565
/*------------------------TFT3.5接线--------------------------

//              GND   电源地
//              VCC   3.3v电源
//              SCL   PA0（SCLK）
//              SDA   PA1（MOSI）
//              RES   PA2
//              DC    PA3
//							BLK   PA4 控制背光
//              MISO  PA5
//              CS1   PA6 
//              CS2   PA7
----------------------------------------------------------------*/

#include "delay.h"
#include "sys.h"
#include "led.h"
#include "lcd_init.h"
#include "lcd.h"
#include "pic.h"
#include "usart.h"
#include "SH30.h"

#include <usart.h>


extern float Temperature;
extern float Humidity;

char buf[2][24] = {0};

int main(void)
{
	// u8 i,j;
	// float t=0;
	// LED_Init();//LED初始化
	// LCD_Init();//LCD初始化
	USART_CONFIG();
	
	SHT30_Init();
	// LCD_Fill(0,0,LCD_W,LCD_H,GREEN);

	while(1)
	{		

		SHT30_read_result(0x44);
		sprintf((char *)buf[0],"Current Temp:%.2f\r\n",Temperature);
		sprintf((char *)buf[1],"Current Humi:%.2f\r\n",Humidity);	

		Usart_SendStr(USART1,buf[0]);
		Usart_SendStr(USART1,buf[1]);
		// LCD_ShowString(40,120,buf[0],WHITE,RED,32,0);
		// LCD_ShowString(40,160,buf[1],WHITE,RED,32,0);
		delay_ms(2000);
	}
}

