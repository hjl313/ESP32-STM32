#ifndef SH301_H
#define SH301_H
#include "delay.h"
#include "sys.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"
#include "sys.h"
 
extern u8 humiture_buff1[20];
extern u8 humiture_buff2[20];
extern u8 Refresh_SHT30_Data;
extern u8 send_data_fleg;
extern u8 Temperature_L;
extern u8 Humidity_L;
extern u8 Temperature_H;
extern u8 Humidity_H;
 
void SHT30_Init(void);
void IIC_ACK(void);
void IIC_NACK(void);
u8 IIC_wait_ACK(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_SendByte(u8 byte);
u8 IIC_RcvByte(void);
void SHT30_read_result(u8 addr);
 
#endif
 