#ifndef PID_H_
#define PID_H_

typedef struct Pid
{
 	float Sv;//用户设定值
 	float Pv;
 
 	float Kp;
 	int T;  //PID计算周期--采样周期
 	float Ti;
 	float Td; 
	
 	float Ek;  //本次偏差
	float Ek_1;//上次偏差
	float SEk; //历史偏差之和
	
	float Iout;
	float Pout;
	float Dout;
	
 	float OUT0;

 	float OUT;

 	int C1ms;
	
 	int pwmcycle;//pwm周期
 
 	int times;
}PID;

extern PID pid;

void PID_Init(void);
void PID_Calc(void);

#endif

