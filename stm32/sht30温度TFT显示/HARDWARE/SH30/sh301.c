#include "SH301.h"


/*----------------------SHT30----------------------
							黄线（SCL） -PB4
							白线（SDA)  -PB5
							黑线（GND)
							红线（VCC)
----------------------------------------------------*/

#define write 0
#define read  1
//IIC总线地址接口定义
#define SCL PAout(4)
#define SDA_OUT PAout(5)
#define SDA_IN PAin(5)
#define IIC_INPUT_MODE_SET()  {GPIOA->CRL&=0xFF0FFFFF;GPIOA->CRL|=0x00800000;}
#define IIC_OUTPUT_MODE_SET() {GPIOA->CRL&=0xFF0FFFFF;GPIOA->CRL|=0x00300000;}
 
float humiture[4];
u8 Refresh_SHT30_Data=0;
u8 humiture_buff1[20];
u8 humiture_buff2[20];
 
u8 Temperature_L=15;
u8 Humidity_L=50;
u8 Temperature_H=30;
u8 Humidity_H=80;

float Temperature = 0;
float Humidity = 0;
 
	
	
void SHT30_Init(void)
{
  /*1.开时钟*/
  RCC->APB2ENR |= 1<<2;
  /*2.设置GPIO模式*/
  GPIOA->CRL &= 0x0000FFFF;
  GPIOA->CRL |= 0x33330000;
  /*3.设置GPIO空闲电平*/
  GPIOA->ODR |= 0xF<<4;	
  //printf("SHT30_Init OK！\n");
}	
 /*主机发送ACK*/
void IIC_ACK(void)
{
  IIC_OUTPUT_MODE_SET();
  SCL=0;
  delay_us(2); 
  SDA_OUT=0;
  delay_us(2);     
  SCL=1;
  delay_us(2);                  
  SCL=0;                     
  delay_us(1);    
}
 /*主机不发送ACK*/
void IIC_NACK(void)
{
  IIC_OUTPUT_MODE_SET();
  SCL=0;
  delay_us(2); 
  SDA_OUT=1;
  delay_us(2);      
  SCL=1;
  delay_us(2);                   
  SCL=0;                     
  delay_us(1);    
}
 /*主机等待从机的ACK*/
u8 IIC_wait_ACK(void)
{
    u8 t = 200;
    IIC_OUTPUT_MODE_SET();
    SDA_OUT=1;//8位发送完后释放数据线，准备接收应答位 
    delay_us(1);
    SCL=0;
    delay_us(1); 
    IIC_INPUT_MODE_SET();
    delay_us(1); 
    while(SDA_IN)//等待SHT30应答
    {
	t--;
	delay_us(1); 
	if(t==0)
	{
	  SCL=0;
	  return 1;
	}
	delay_us(1); 
    }
    delay_us(1);      
    SCL=1;
    delay_us(1);
    SCL=0;             
    delay_us(1);    
    return 0;	
}
/*******************************************************************
功能:启动I2C总线,即发送I2C起始条件.  
********************************************************************/
void IIC_Start(void)
{
  IIC_OUTPUT_MODE_SET();
  SDA_OUT=1;
  SCL=1;
  delay_us(4);	
  SDA_OUT=0;
  delay_us(4); 
  SCL=0;
}
 
/*******************************************************************
功能:结束I2C总线,即发送I2C结束条件.  
********************************************************************/
void IIC_Stop(void)
{
	IIC_OUTPUT_MODE_SET();
	SCL=0;
	SDA_OUT=0;  
	delay_us(4);	
	SCL=1;
	delay_us(4);
	SDA_OUT=1;
	delay_us(4);
}
 
/*******************************************************************
字节数据发送函数               
函数原型: void  SendByte(UCHAR c);
功能:将数据c发送出去,可以是地址,也可以是数据
********************************************************************/
void  IIC_SendByte(u8 byte)
{
	u8  BitCnt;
	IIC_OUTPUT_MODE_SET();
	SCL=0;
	for(BitCnt=0;BitCnt<8;BitCnt++)//要传送的数据长度为8位
	{
		if(byte&0x80) SDA_OUT=1;//判断发送位
		else SDA_OUT=0; 
		byte<<=1;
		delay_us(2); 
		SCL=1;
		delay_us(2);
		SCL=0;
		delay_us(2);
	}
}
/*******************************************************************
 字节数据接收函数               
函数原型: UCHAR  RcvByte();
功能: 用来接收从器件传来的数据  
********************************************************************/    
u8 IIC_RcvByte(void)
{
  u8 retc;
  u8 BitCnt;
  retc=0; 
  IIC_INPUT_MODE_SET();//置数据线为输入方式
  delay_us(1);                    
  for(BitCnt=0;BitCnt<8;BitCnt++)
  {  
	SCL=0;//置时钟线为低，准备接收数据位
	delay_us(2);               
	SCL=1;//置时钟线为高使数据线上数据有效                
	retc=retc<<1;
	if(SDA_IN) retc |=1;//读数据位,接收的数据位放入retc中 
	delay_us(1);
  }
  SCL=0;    
  return(retc);
}
/*******************************************************************
 温湿度获取函数               
函数原型: SHT30_read_result(u8 addr);
功能: 用来接收从器件采集并合成温湿度
********************************************************************/ 
void SHT30_read_result(u8 addr)
{
	u16 tem,hum;
	u16 buff[6];	
	IIC_Start();
	IIC_SendByte(addr<<1 | write);//写7位I2C设备地址加0作为写取位,1为读取位
	IIC_wait_ACK();
	IIC_SendByte(0x2C);
	IIC_wait_ACK();
	IIC_SendByte(0x06);
	IIC_wait_ACK();
	IIC_Stop();
	delay_ms(50);
	IIC_Start();
	IIC_SendByte(addr<<1 | read);//写7位I2C设备地址加0作为写取位,1为读取位
	if(IIC_wait_ACK()==0)
	{
		buff[0]=IIC_RcvByte();
		IIC_ACK();
		buff[1]=IIC_RcvByte();
		IIC_ACK();
		buff[2]=IIC_RcvByte();
		IIC_ACK();
		buff[3]=IIC_RcvByte();
		IIC_ACK();
		buff[4]=IIC_RcvByte();
		IIC_ACK();
		buff[5]=IIC_RcvByte();
		IIC_NACK();
		IIC_Stop();
	}
	
	tem = ((buff[0]<<8) | buff[1]);//温度拼接
	hum = ((buff[3]<<8) | buff[4]);//湿度拼接
	
	/*转换实际温度*/
	Temperature= (175.0*(float)tem/65535.0-45.0) ;// T = -45 + 175 * tem / (2^16-1)
	Humidity= (100.0*(float)hum/65535.0);// RH = hum*100 / (2^16-1)
	
	if((Temperature>=-20)&&(Temperature<=125)&&(Humidity>=0)&&(Humidity<=100))//过滤错误数据
	{
		humiture[0]=Temperature;
		humiture[2]=Humidity;
		sprintf(humiture_buff1,"%6.2f*C %6.2f%%",Temperature,Humidity);//111.01*C 100.01%（保留2位小数）
	}
	printf("温湿度：%s\n",humiture_buff1);
	hum=0;
	tem=0;
}