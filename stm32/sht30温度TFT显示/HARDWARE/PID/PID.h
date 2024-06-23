#ifndef PID_H_
#define PID_H_

typedef struct Pid
{
 	float Sv;//�û��趨ֵ
 	float Pv;
 
 	float Kp;
 	int T;  //PID��������--��������
 	float Ti;
 	float Td; 
	
 	float Ek;  //����ƫ��
	float Ek_1;//�ϴ�ƫ��
	float SEk; //��ʷƫ��֮��
	
	float Iout;
	float Pout;
	float Dout;
	
 	float OUT0;

 	float OUT;

 	int C1ms;
	
 	int pwmcycle;//pwm����
 
 	int times;
}PID;

extern PID pid;

void PID_Init(void);
void PID_Calc(void);

#endif

