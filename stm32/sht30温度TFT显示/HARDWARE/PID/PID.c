#include "PID.h"
#include "stm32f10x.h"

PID pid;

void PID_Init()
{
    pid.Sv=38;//用户设定温度
	pid.Kp=30;
	pid.T=500;//PID计算周期
    pid.Ti=5000000;//积分时间
	pid.Td=1000;//微分时间
	pid.pwmcycle=200;//pwm周期200
	pid.OUT0=1;
	pid.C1ms=0;
}

void PID_Calc()  //pid计算
{
 	float DelEk;
	float ti,ki;
	float td;
	float kd;
	float out;
 	if(pid.C1ms<(pid.T))  //计算周期未到
 	{
    	return ;
 	}
 
 	pid.Ek=pid.Sv-pid.Pv;   //得到当前的偏差值
 	pid.Pout=pid.Kp*pid.Ek;      //比例输出
 
 	pid.SEk+=pid.Ek;        //历史偏差总和
 
 	DelEk=pid.Ek-pid.Ek_1;  //最近两次偏差之差
 
 	ti=pid.T/pid.Ti;
 	ki=ti*pid.Kp;

    pid.Iout=ki*pid.SEk;  //积分输出

 	td=pid.Td/pid.T;
 
 	kd=pid.Kp*td;
 
    pid.Dout=kd*DelEk;    //微分输出
 
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
 	pid.Ek_1=pid.Ek;  //更新偏差
 	pid.C1ms=0;
}
