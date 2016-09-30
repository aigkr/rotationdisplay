/******************************************************************
*功能：旋转显示加计时器
*作者：宋超
*日期：2016年6月
******************************************************************/
#include <reg51.h>	   //此文件中定义了51的一些特殊功能寄存器

#include "character.c"

#define uchar unsigned char
#define uint unsigned int

#define BITLEN 32                 //32位接收码

//起始测低电平时间用计时器时间：1ms
#define TH0OF1MS 252                             //(65536 - 1000) / 256
#define TL0OF1MS 24                              //(65536 - 1000) % 256

//扫描用计时:0.25ms
#define TH1OFDISPLAY 255       //(65536 - 250) / 256
#define TL1OFDISPLAY 6        //(65536 - 250) % 256

//防抖用计时：0.75ms
#define TH1OFDELAY 253         //(65536 - 750) / 256
#define TL1OFDELAY 15          //(65536 - 750) % 256

/****************************************************************************
起始时判断后面是否有发送码的高电位电位时间：3.4ms
有发送码时高电位时间：4.5ms，连续码的高电位时间：2.3ms
3400/256=13; 4500/256=17; 2300/256=8
只需取出TH0中的数据即可做出判断
****************************************************************************/
#define IFRECIIVE 13

/********************************************************************************
区别接收到的数据码的0和1用的时间：1.6875ms
数据0的总时间：1.125ms 数据1的总时间：2.25ms
1688/256=6; 1125/256=4; 2250/256=8
也只需取出TH0中的数据即可做出判断
********************************************************************************/
#define IS1OR0 6

sbit IRIN = P3^2;
sbit position = P3^3;

unsigned char IrValue[4] = {0};                  //用来存放读取到的红外值
unsigned char int0State = 0;
unsigned char lenOfLow = 0;                      //用来记录起始低电位的长度(9ms)以判断是否接收到正确的信号
unsigned char tempOfTH0 = 0;                     //暂存32位接收码计时器的TH0值
unsigned char receivedBitLen = 0;
unsigned char kInIrValue = 0;                    //表示第几组数据
unsigned char g_disPlayStart = 0;
unsigned char g_characterLen = 0;
unsigned char g_mode = 0;
unsigned int g_n = 0;
unsigned int g_startPosition = 0;

void IntConfiguration();
void Display(void);

/*演示主程序*/
void main(void)
{
    IntConfiguration();
    while (1)
    {
		if (int0State == 2)
		{
			if (receivedBitLen == BITLEN)        //32位数据接收完毕
			{
				TR0 = 0;                         //关闭计时器0
				int0State = 0;
				receivedBitLen = 0;
				TH0 = TH0OF1MS;
				TL0 = TL0OF1MS;                  //定时器0初始化
				if (IrValue[2] = ~IrValue[3])   //数据码与其反码不符，有误
			    {
					switch (IrValue[2])
		            {
						case 0x0C:
			            case 0x11: {g_startPosition = 128; g_mode = 1; g_characterLen = 64;}; break;
						case 0x18:
			            case 0x12: {g_startPosition = 192; g_mode = 1; g_characterLen = 64;}; break;
						case 0x5E:
			            case 0x13: {g_startPosition = 256; g_mode = 1; g_characterLen = 64;}; break;
			            case 0x08:
						case 0x14: {g_startPosition = 320; g_mode = 1; g_characterLen = 64;}; break;
			            case 0x1C:
						case 0x15: {g_startPosition = 384; g_mode = 1; g_characterLen = 64;}; break;
			            case 0x5A:
						case 0x16: {g_startPosition = 448; g_mode = 1; g_characterLen = 64;}; break;
						
						case 0x42: {g_startPosition = 0; g_mode = 2; g_characterLen = 128; IrValue[2] = 0;}; break;
						
						case 0x52: {g_startPosition = 512; g_mode = 3; g_characterLen = 128; IrValue[2] = 0;}; break;
						
						case 0x4a: {g_startPosition = 2224; g_mode = 4; g_characterLen = 128; IrValue[2] = 0;}; break;
		            }
			    }
			}
		}
		
	    if (g_n == g_characterLen)
		{
			//一圈扫描结束
			TR1 = 0;                                        //关闭定时器中断1
			g_n = 0;
		    P2 = 0xff;
            P0 = 0xff;
		
		    //重新装载定时器 时间：0.75ms
		    TH1 = TH1OFDELAY;
	        TL1 = TL1OFDELAY;
			if (g_mode == 2)
			{
				g_startPosition++;
			    if (g_startPosition == 512)
			    {
				    g_startPosition = 0;
			    }
			}
			else if (g_mode == 3)
			{
				g_startPosition++;
			    if (g_startPosition == 2224)
			    {
				    g_startPosition = 512;
			    }
			}
			else if (g_mode == 4)
			{
				g_startPosition++;
			    if (g_startPosition == 4768)
			    {
				    g_startPosition = 2224;
			    }
			}
		}
		
    }
}

/*******************************************************************************
* 函数名         : IntConfiguration()
* 函数功能		   : 设置外部中断
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/
void IntConfiguration()
{
	//外部中断0设置
    IT0 = 1;                                     //下降沿触发
    EX0 = 1;                                     //打开中断0允许
    IRIN = 1;                                    //初始化端口
	
	//定时器/计数器0设置
	TH0 = TH0OF1MS;
	TL0 = TL0OF1MS;
	ET0 = 1;                                     //打开定时器0中断
	
	//设置定时器1
	TMOD = 0x11;         //设置定时器0和1为工作方式1
	TH1 = TH1OFDELAY;
	TL1 = TL1OFDELAY;
	ET1 = 1;             //打开定时器中断1
	
	//设置INT1
	IT1 = 1;             //跳变沿触发方式（下降沿）
	EX1 = 1;
	
	EA = 1;              //打开总中断
}

/*******************************************************************************
/函数名          ：StartReadIr()
/函数功能        ：由外部中断0控制开始读取红外数值
/输入            ：无
/输出            ：无
/******************************************************************************/
void StartReadIr() interrupt 0
{
	if (IRIN == 0)
	{
		switch (int0State)
		{
			case 0:
			{
				TR0 = 1;                         //启动定时器0
				int0State++;
			};break;
			case 1:
			{
				TR0 = 0;                        //首先关闭计时器0
				if (TH0 < IFRECIIVE)            //后面没有发送码
				{
					int0State = 0;              //状态机初始化
					TH0 = TH0OF1MS;
	                TL0 = TL0OF1MS;
				}
				else                            //后面有32位的发送码
				{
					TH0 = 0;
					TL0 = 0;                    //计时器0初始化
					TR0 = 1;                    //开启计时器0
					int0State++;
				}
			};break;
			case 2:
			{
				TR0 = 0;                        //关闭计时器0
				tempOfTH0 = TH0;                //暂存TH0内的值
				TH0 = 0;
				TL0 = 0;                        //计时器0初始化
				TR0 = 1;                        //开启计时器0
				kInIrValue = receivedBitLen / 8;
			    IrValue[kInIrValue] >>= 1;
			    if (tempOfTH0 > 20)
				{
					TR0 = 0;
					TH0 = TH0OF1MS;
	                TL0 = TL0OF1MS;
					int0State = 0;
					receivedBitLen = 0;
					IRIN = 1;                        //初始化端口
				}
			    else if (tempOfTH0 > IS1OR0)         //此位是1
			    {
				    IrValue[kInIrValue] |= 0x80;
			    }
				receivedBitLen++;
			};break;
		}
	}
}

/*************************************************************************
函数名           ：T0_Timer()
函数功能         ：定时1ms，共触发9次，判断是否进入起始9ms的低电位
输入             ：无
输出             ：无
*************************************************************************/
void T0_Timer() interrupt 1
{
	if (lenOfLow < 8)
	{
		if (IRIN == 0)                          //正确处于低电位
	    {
		    TH0 = TH0OF1MS;
	        TL0 = TL0OF1MS;
			TR0 = 1;
			lenOfLow++;
	    }
		else                                   //错误的触发
		{
			TH0 = TH0OF1MS;
			TL0 = TL0OF1MS;
			lenOfLow = 0;
		}
	}
	else                                       //最后一次触发定时器0，接下来就要测高电位的时间了
	{
			TH0 = 0;
			TL0 = 0;                           //计时器模式
			TR0 = 1;                           //开启计时器
			lenOfLow = 0;                      //计数器初始化，为下一次接收做准备
	}
}

/*******************************************************************************
* 函数名         : PositionCheck()
* 函数功能		 : 外部中断1的中断函数
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/
void PositionCheck() interrupt 2		   //外部中断1的中断函数
{	
	TR1 = 1;               //开启定时器中断1
    g_disPlayStart = 1;
}

/******************************************************************************
*函数名       ：Timer1() interrupt 3
*函数功能     ：定时器中断1的函数
*输入         ：无
*输出         ：无
******************************************************************************/
void TimerofDisplay() interrupt 3
{
	if (g_disPlayStart)
	{
		//重新开启定时器 时间：0.25ms
		TH1 = TH1OFDISPLAY;
	    TL1 = TL1OFDISPLAY;
		
		g_disPlayStart = 0;    //标明非起始状态
	}
	else
	{
		//重新开启定时器 时间：0.25ms
		TH1 = TH1OFDISPLAY;
	    TL1 = TL1OFDISPLAY;
		Display();
	}
}
void Display(void)
{
    if (g_n < g_characterLen)
	{
		P2 = upside[g_startPosition + g_n];
		P0 = underpart[g_startPosition + g_n++];
	}
}  
