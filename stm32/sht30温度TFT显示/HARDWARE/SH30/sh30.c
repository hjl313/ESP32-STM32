/*
设定温度阈值范围：-20℃——125℃
设定相对湿度范围：0%——100%
*/
#include"stm32f10x.h"
#include"sh30.h"
#include"stdio.h"

#define write 0 //IIC设备地址一般是7位，也有10位，本程序使用7位IIC设备地址，第8位是写命令
#define read  1 //IIC设备地址一般是7位，也有10位，本程序使用7位IIC设备地址，第8位是读命令

//IIC总线接口定义
#define SCL PBout(6)//设置时钟端口
//SHT30数据SDA传输口，有输出也有输入，所以需配置它的输入和输出
#define SDA_OUT PBout(7)//设置输出数据端口
#define SDA_IN PBin(7)//设置输入数据端口
//设置端口高8位的工作模式（具体可以参照端口位配置表），即I/O输入输出方向设置，先用与（&）对PB7引脚的四个位清0，再用或（|）置1
#define IIC_INPUT_MODE_SET()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=8<<28;}//改变PB7对应位为1000（CNF[1:0]MODE[1:0]），设置成上拉或者下拉输入
#define IIC_OUTPUT_MODE_SET() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=3<<28;}//改变PB7对应位为0011，设置成通用推挽输出
//#define IIC_INPUT_MODE_SET()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=8<<12;}//改变PB7对应位为1000（CNF[1:0]MODE[1:0]），设置成上拉或者下拉输入
//#define IIC_OUTPUT_MODE_SET() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=3<<12;}//改变PB7对应位为0011，设置成通用推挽输出

 //定义温湿度存储变量
u8 humiture_buff1[20];

float Temperature=0;//定义存放温度拼接的变量Temperature，初始化温度为0
float Humidity=0;//定义存放湿度拼接的变量Humidity，初始化湿度为0

void SHT30_Init(void)//初始化SHT30的SCL与SDA接口
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;//定于SCL连接到STM32的PB6引脚上，定于SDA_OUT/SDA_IN连接到STM32的PB7引脚上
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	SCL=1;//根据SHT30时序要求，默认时序是高电平（SDA拉低时，SCL跟着拉低，开始数据的传输）
	SDA_OUT=1;//高电平还未开始数据的传输，拉低时开始数据的传输
}	
//主机发送确认字符ACK
void IIC_ACK(void)//IIC协议，需要主机发送确认字符ACK来判断数据是否接收无误
{
  IIC_OUTPUT_MODE_SET();//配置PB7的SDA输出为推挽输出模式
  SCL=0;
  delay_us(2); 
  SDA_OUT=0;
  delay_us(2);     
  SCL=1;//数据接收完成之后，将SCL拉高，以便与后面主机发送ACK给从机
  delay_us(2);                  
  SCL=0;//数据接收完成，主机发送ACK                     
  delay_us(1);    
}
//主机不发送确认字符ACK
void IIC_NACK(void)//IIC协议，数据传输未完成或者数据传输有误，主机不发送确认字符ACK
{
  IIC_OUTPUT_MODE_SET();//配置PB7的SDA输出为推挽输出模式
  SCL=0;
  delay_us(2); 
  SDA_OUT=1;
  delay_us(2);      
  SCL=1;
  delay_us(2);                   
  SCL=0;                     
  delay_us(1);    
}
//主机等待从机的确认字符ACK
u8 IIC_wait_ACK(void)//主机等待从机发送ACK，从而判断数据是否接收完成
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
//启动IIC总线进行通讯,发送IIC通讯的起始条件
void IIC_Start(void)//启动IIC通讯
{
  IIC_OUTPUT_MODE_SET();//设置PB7的SDA为推挽输出
  SDA_OUT=1;//根据SHT30时序，启动之前，SDA配置为高电平
  SCL=1;//根据SHT30时序，启动之前，SCL配置为高电平
  delay_us(4);//延时4us，给硬件一个反应时间
  SDA_OUT=0;//SDA引脚拉低，开始数据的传输
  delay_us(4);
  SCL=0;//SCL拉低，与SDA对应，响应SDA数据的拉低，表示正式开始数据的传输
}
 
//结束IIC总线通讯，发送IIC通讯的结束条件
void IIC_Stop(void)
{
	IIC_OUTPUT_MODE_SET();
	SCL=0;//结束IIC通讯之前，查看SCL是否是拉低状态
	SDA_OUT=0;//结束IIC通讯之前，查看SDA是否是拉低状态  
	delay_us(4);	
	SCL=1;//将时钟拉高，表示已经结束IIC通讯
	delay_us(4);
	SDA_OUT=1;//将数据传输引脚拉高，表示正式结束数据的传输
	delay_us(4);
}
 
//将byte数据发送出去
void  IIC_SendByte(u8 byte)
{
	u8  Count;
	IIC_OUTPUT_MODE_SET();
	SCL=0;//将时钟拉低，开始数据的传输
	for(Count=0;Count<8;Count++)//要传送的数据长度为8位
	{
		if(byte&0x80) SDA_OUT=1;//判断发送位，发送位为1，则还未发送数据
		else SDA_OUT=0; //判断发送位为0，则开始数据的发送
		byte<<=1;
		delay_us(2); 
		SCL=1;
		delay_us(2);
		SCL=0;
		delay_us(2);
	}
}

// 用来接收从器件传来的数据    
u8 IIC_RcvByte(void)
{
  u8 retc;
  u8 Count;
  retc=0; 
  IIC_INPUT_MODE_SET();//配置数据线为输入方式
  delay_us(1);                    
  for(Count=0;Count<8;Count++)
  {  
	SCL=0;//配置时钟线为低，准备接收数据位
	delay_us(2);               
	SCL=1;//配置时钟线为高使数据线上数据有效                
	retc=retc<<1;
	if(SDA_IN) retc |=1;//读数据位,接收的数据位放入retc中 
	delay_us(1);
  }
  SCL=0;    
  return(retc);
}
//用来接收从器件采集并合成温湿度 
void SHT30_read_result(u8 addr)
{
	//SHT30有两种读取数值的方法，分别是状态查询和数值查询，这里我使用的是数值查询，发送指令为0x2C06
	u16 tem,hum;//定义存放温湿度计算公式的变量
	u16 buff[6];//定义6个字节的数组，存放温度高、低八位，湿度高、低八位，两个字节的CRC校验位

	//发送指令为0x2C06（默认）
	IIC_Start();
	IIC_SendByte(addr<<1 | write);//写7位I2C设备地址加0作为写取位,1为写取位
	IIC_wait_ACK();
//	delay_us(1);
	IIC_SendByte(0x2C);//前半段发送指令为0x2C
	IIC_wait_ACK();
//	delay_us(1);
	IIC_SendByte(0x06);//后半段发送指令为0x06
	IIC_wait_ACK();
//	delay_us(1);
	IIC_Stop();
	delay_ms(50);
	IIC_Start();
	IIC_SendByte(addr<<1 | read);//写7位I2C设备地址加1作为读取位,1为读取位
//SHT30返回的数值是6个元素的数组
	if(IIC_wait_ACK()==0)
	{
//  	delay_us(1);
		buff[0]=IIC_RcvByte();//返回温度高8位
		IIC_ACK();
		buff[1]=IIC_RcvByte();//返回温度低8位
		IIC_ACK();
		buff[2]=IIC_RcvByte();//温度crc校验位
		IIC_ACK();
		buff[3]=IIC_RcvByte();//返回湿度高8位
		IIC_ACK();
		buff[4]=IIC_RcvByte();//返回湿度低8位
		IIC_ACK();
		buff[5]=IIC_RcvByte();//湿度crc校验位
		IIC_NACK();
		IIC_Stop();
	}
//	tem = (buff[0]<<8) + buff[1];
//	hum = (buff[3]<<8) + buff[4];
	tem = ((buff[0]<<8) | buff[1]);//将buff[0]采集到的温度8位数据与buff[1]采集到的低8位数据相或，实现温度拼接
	hum = ((buff[3]<<8) | buff[4]);//将buff[3]采集到的湿度8位数据与buff[4]采集到的低8位数据相或，实现湿度拼接

//查询SHT30数据手册可知，温湿度的计算方法如下	
	Temperature= 175.0*(float)tem/65535.0-45.0 ;// T = -45 + 175 * tem / (2^16-1)
	Humidity= 100.0*(float)hum/65535.0;// RH = hum*100 / (2^16-1)
	
	if((Temperature>=-20)&&(Temperature<=125)&&(Humidity>=0)&&(Humidity<=100))//设定温度和湿度的阈值，超过这个阈值则返回错误提示
	{
		printf("温度：%6.2f℃\r\n",Temperature);
		printf("湿度：%6.2f%%\r\n",Humidity);
	}
	else
	{
		printf("温湿度超过给定阈值！\r\n");
	}
	hum=0;
	tem=0;	

}
