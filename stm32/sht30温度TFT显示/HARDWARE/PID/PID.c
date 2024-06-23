#include "PID.h"
#include "stm32f10x.h"

PID pid;

void PID_Init()
{
    pid.Sv=38;//�û��趨�¶�
	pid.Kp=30;
	pid.T=500;//PID��������
    pid.Ti=5000000;//����ʱ��
	pid.Td=1000;//΢��ʱ��
	pid.pwmcycle=200;//pwm����200
	pid.OUT0=1;
	pid.C1ms=0;
}

void PID_Calc()  //pid����
{
 	float DelEk;
	float ti,ki;
	float td;
	float kd;
	float out;
 	if(pid.C1ms<(pid.T))  //��������δ��
 	{
    	return ;
 	}
 
 	pid.Ek=pid.Sv-pid.Pv;   //�õ���ǰ��ƫ��ֵ
 	pid.Pout=pid.Kp*pid.Ek;      //�������
 
 	pid.SEk+=pid.Ek;        //��ʷƫ���ܺ�
 
 	DelEk=pid.Ek-pid.Ek_1;  //�������ƫ��֮��
 
 	ti=pid.T/pid.Ti;
 	ki=ti*pid.Kp;

    pid.Iout=ki*pid.SEk;  //�������

 	td=pid.Td/pid.T;
 
 	kd=pid.Kp*td;
 
    pid.Dout=kd*DelEk;    //΢�����
 
 	out= pid.Pout+ pid.Iout+ pid.Dout;
 
 
 	if(out>pid.pwmcycle)
 	{
  		pid.OUT=pid.pwmcycle;
 	}
 	else if(out<=0)
 	{
		pid.OUT=pid.OUT0; 
 	}
 	else 
 	{
  		pid.OUT=out;
 	}
 	pid.Ek_1=pid.Ek;  //����ƫ��
 	pid.C1ms=0;
}
