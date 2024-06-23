#include "bsp_usart.h"

static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_COFIG;
  
  /* 嵌套向量中断控制器组选择 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* 配置USART为中断源 */
  NVIC_COFIG.NVIC_IRQChannel = DEBUG_USART_IRQ;
  /* 抢断优先级*/
  NVIC_COFIG.NVIC_IRQChannelPreemptionPriority = 1;
  /* 子优先级 */
  NVIC_COFIG.NVIC_IRQChannelSubPriority = 1;
  /* 使能中断 */
  NVIC_COFIG.NVIC_IRQChannelCmd = ENABLE;
  /* 初始化配置NVIC */
  NVIC_Init(&NVIC_COFIG);
}

void USART_CONFIG()
{
	/*定义结构体*/
	USART_InitTypeDef USART_CONFIG;
	GPIO_InitTypeDef 	GPIO_CONFIG;
	//打开GPIO时钟
	RCC_APB2PeriphClockCmd(DEBUG_USART_GPIO_CLK, ENABLE);
	//打开外设串口时钟
	RCC_APB2PeriphClockCmd(DEBUG_USART_CLK, ENABLE);
	// 将USART Tx 的 GPIO 配置为推挽复用模式
	GPIO_CONFIG.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_CONFIG.GPIO_Pin  	= DEBUG_USART_TX_GPIO_PIN;
	GPIO_CONFIG.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_CONFIG);
	// 将USART Rx的GPIO配置为浮空输入模式
	GPIO_CONFIG.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_CONFIG.GPIO_Pin  	= DEBUG_USART_RX_GPIO_PIN;
	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_CONFIG);
	
	/*.......配置串口的工作参数......*/
	// 配置波特率
	USART_CONFIG.USART_BaudRate = DEBUG_USART_BAUDRATE;
	// 配置 针数据字长
	USART_CONFIG.USART_WordLength = USART_WordLength_8b;;
	// 配置停止位
	USART_CONFIG.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_CONFIG.USART_Parity = USART_Parity_No;
	// 配置硬件流控制
	USART_CONFIG.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_CONFIG.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(DEBUG_USARTx,&USART_CONFIG);
	// 串口中断优先级配置
	NVIC_Configuration();
	// 使能串口接收中断
	USART_ITConfig(DEBUG_USARTx, USART_IT_RXNE, ENABLE);
	// 使能串口
	USART_Cmd(DEBUG_USARTx, ENABLE);
}
//半字
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t data)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,data);
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}
void Usart_SendHalfWrold( USART_TypeDef * pUSARTx, uint16_t data)
{
	uint8_t temp_h,temp_l;
	//取高八位
	temp_h = (data&0xff00) >> 8;
	//低八位
	temp_l = (data&0xff);
	/* 发送高八位 */
	USART_SendData(pUSARTx,temp_h);
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	/* 发送低八位 */
	USART_SendData(pUSARTx,temp_l );
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);		
}
//数组
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array,uint8_t num)
{
	uint8_t i;
	for(i = 0;i < num; i++)
	{
		Usart_SendByte(pUSARTx,array[i]);
	}
	//这里发送的多个字符，USART_FLAG_TC（TXE改为TC）
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) == RESET);
}
/*
void Usart_SendStr(USART_TypeDef * pUSARTx,char *Str)
{

	do 
  {
		//Str++字符串往后移动
		unsigned int i = 0;
    Usart_SendByte( pUSARTx, *(Str+i));
		i++;
  } while(*(Str + i)!= '\0');
//乱码问题。。
	do 
  {
		//Str++字符串往后移动
    Usart_SendByte( pUSARTx, *(Str++));
  } while( *(Str++)!= '\0');
  
	等待发送完成
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {}
}
*/
void Usart_SendStr( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
  do 
  {
      Usart_SendByte( pUSARTx, *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* 等待发送完成 */
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {}
}
///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}


