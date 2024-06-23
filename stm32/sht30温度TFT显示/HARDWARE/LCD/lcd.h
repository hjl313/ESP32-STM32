#ifndef __LCD_H
#define __LCD_H		
#include "sys.h"


void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u32 color);//指定区域填充颜色
void LCD_DrawPoint(u16 x,u16 y,u32 color);//在指定位置画一个点
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u32 color);//在指定位置画一条线
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u32 color);//在指定位置画一个矩形
void Draw_Circle(u16 x0,u16 y0,u8 r,u32 color);//在指定位置画一个圆

void LCD_ShowChinese(u16 x,u16 y,u8 *s,u32 fc,u32 bc,u8 sizey,u8 mode);//显示汉字串
void LCD_ShowChinese12x12(u16 x,u16 y,u8 *s,u32 fc,u32 bc,u8 sizey,u8 mode);//显示单个12x12汉字
void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u32 fc,u32 bc,u8 sizey,u8 mode);//显示单个16x16汉字
void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u32 fc,u32 bc,u8 sizey,u8 mode);//显示单个24x24汉字
void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u32 fc,u32 bc,u8 sizey,u8 mode);//显示单个32x32汉字

void LCD_ShowChar(u16 x,u16 y,u8 num,u32 fc,u32 bc,u8 sizey,u8 mode);//显示一个字符
void LCD_ShowString(u16 x,u16 y,const u8 *p,u32 fc,u32 bc,u8 sizey,u8 mode);//显示字符串
u32 mypow(u8 m,u8 n);//求幂
void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u32 fc,u32 bc,u8 sizey);//显示整数变量
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u32 fc,u32 bc,u8 sizey);//显示两位小数变量

void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[]);//显示图片

//字库函数
void ZK_command(u8 dat);
u8 get_data_from_ROM(void);
void get_n_bytes_data_from_ROM(u8 AddrHigh,u8 AddrMid,u8 AddrLow,u8 *pBuff,u8 DataLen);
void Display_GB2312(u16 x,u16 y,u8 zk_num,u32 fc,u32 bc);
void Display_GB2312_String(u16 x,u16 y,u8 zk_num,u8 text[],u32 fc,u32 bc);
void Display_Asc(u16 x,u16 y,u8 zk_num,u32 fc,u32 bc);
void Display_Asc_String(u16 x,u16 y,u16 zk_num,u8 text[],u32 fc,u32 bc);
void Display_Arial_TimesNewRoman(u16 x,u16 y,u8 zk_num,u32 fc,u32 bc);
void Display_Arial_String(u16 x,u16 y,u16 zk_num,u8 text[],u32 fc,u32 bc);
void Display_TimesNewRoman_String(u16 x,u16 y,u16 zk_num,u8 text[],u32 fc,u32 bc);

	
//画笔颜色
#define WHITE         	 0xFCFCFC
#define BLACK            0X000000
#define RED           	 0xFC0000
#define GREEN            0x00FC00
#define BLUE             0x0000FC

#endif





