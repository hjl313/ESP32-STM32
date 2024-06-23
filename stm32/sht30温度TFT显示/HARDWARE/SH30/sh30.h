#ifndef _SH30_H
#define _SH30_H

#include "delay.h"
#include "sys.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"
 
extern u8 humiture_buff1[20];

void SHT30_Init(void);
void IIC_ACK(void);//确认字符，表示发来的数据已经确认接收无误
void IIC_NACK(void);//无确认字符，表示数据接收有误或者还未接收完成
u8 IIC_wait_ACK(void);//等待接收确认字符
void IIC_Start(void);//等待IIC传输数据开始
void IIC_Stop(void);//IIC传输数据停止
void IIC_SendByte(u8 byte);//IIC发送数据
u8 IIC_RcvByte(void);//IIC接收数据
void SHT30_read_result(u8 addr);//SHT30温湿度传感器开始读取数据

#endif

