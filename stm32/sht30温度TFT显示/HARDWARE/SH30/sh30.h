#ifndef _SH30_H
#define _SH30_H

#include "delay.h"
#include "sys.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"
 
extern u8 humiture_buff1[20];

void SHT30_Init(void);
void IIC_ACK(void);//ȷ���ַ�����ʾ�����������Ѿ�ȷ�Ͻ�������
void IIC_NACK(void);//��ȷ���ַ�����ʾ���ݽ���������߻�δ�������
u8 IIC_wait_ACK(void);//�ȴ�����ȷ���ַ�
void IIC_Start(void);//�ȴ�IIC�������ݿ�ʼ
void IIC_Stop(void);//IIC��������ֹͣ
void IIC_SendByte(u8 byte);//IIC��������
u8 IIC_RcvByte(void);//IIC��������
void SHT30_read_result(u8 addr);//SHT30��ʪ�ȴ�������ʼ��ȡ����

#endif

