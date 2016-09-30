/******************************************************************
*���ܣ���ת��ʾ�Ӽ�ʱ��
*���ߣ��γ�
*���ڣ�2016��6��
******************************************************************/
#include <reg51.h>	   //���ļ��ж�����51��һЩ���⹦�ܼĴ���

#include "character.c"

#define uchar unsigned char
#define uint unsigned int

#define BITLEN 32                 //32λ������

//��ʼ��͵�ƽʱ���ü�ʱ��ʱ�䣺1ms
#define TH0OF1MS 252                             //(65536 - 1000) / 256
#define TL0OF1MS 24                              //(65536 - 1000) % 256

//ɨ���ü�ʱ:0.25ms
#define TH1OFDISPLAY 255       //(65536 - 250) / 256
#define TL1OFDISPLAY 6        //(65536 - 250) % 256

//�����ü�ʱ��0.75ms
#define TH1OFDELAY 253         //(65536 - 750) / 256
#define TL1OFDELAY 15          //(65536 - 750) % 256

/****************************************************************************
��ʼʱ�жϺ����Ƿ��з�����ĸߵ�λ��λʱ�䣺3.4ms
�з�����ʱ�ߵ�λʱ�䣺4.5ms��������ĸߵ�λʱ�䣺2.3ms
3400/256=13; 4500/256=17; 2300/256=8
ֻ��ȡ��TH0�е����ݼ��������ж�
****************************************************************************/
#define IFRECIIVE 13

/********************************************************************************
������յ����������0��1�õ�ʱ�䣺1.6875ms
����0����ʱ�䣺1.125ms ����1����ʱ�䣺2.25ms
1688/256=6; 1125/256=4; 2250/256=8
Ҳֻ��ȡ��TH0�е����ݼ��������ж�
********************************************************************************/
#define IS1OR0 6

sbit IRIN = P3^2;
sbit position = P3^3;

unsigned char IrValue[4] = {0};                  //������Ŷ�ȡ���ĺ���ֵ
unsigned char int0State = 0;
unsigned char lenOfLow = 0;                      //������¼��ʼ�͵�λ�ĳ���(9ms)���ж��Ƿ���յ���ȷ���ź�
unsigned char tempOfTH0 = 0;                     //�ݴ�32λ�������ʱ����TH0ֵ
unsigned char receivedBitLen = 0;
unsigned char kInIrValue = 0;                    //��ʾ�ڼ�������
unsigned char g_disPlayStart = 0;
unsigned char g_characterLen = 0;
unsigned char g_mode = 0;
unsigned int g_n = 0;
unsigned int g_startPosition = 0;

void IntConfiguration();
void Display(void);

/*��ʾ������*/
void main(void)
{
    IntConfiguration();
    while (1)
    {
		if (int0State == 2)
		{
			if (receivedBitLen == BITLEN)        //32λ���ݽ������
			{
				TR0 = 0;                         //�رռ�ʱ��0
				int0State = 0;
				receivedBitLen = 0;
				TH0 = TH0OF1MS;
				TL0 = TL0OF1MS;                  //��ʱ��0��ʼ��
				if (IrValue[2] = ~IrValue[3])   //���������䷴�벻��������
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
			//һȦɨ�����
			TR1 = 0;                                        //�رն�ʱ���ж�1
			g_n = 0;
		    P2 = 0xff;
            P0 = 0xff;
		
		    //����װ�ض�ʱ�� ʱ�䣺0.75ms
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
* ������         : IntConfiguration()
* ��������		   : �����ⲿ�ж�
* ����           : ��
* ���         	 : ��
*******************************************************************************/
void IntConfiguration()
{
	//�ⲿ�ж�0����
    IT0 = 1;                                     //�½��ش���
    EX0 = 1;                                     //���ж�0����
    IRIN = 1;                                    //��ʼ���˿�
	
	//��ʱ��/������0����
	TH0 = TH0OF1MS;
	TL0 = TL0OF1MS;
	ET0 = 1;                                     //�򿪶�ʱ��0�ж�
	
	//���ö�ʱ��1
	TMOD = 0x11;         //���ö�ʱ��0��1Ϊ������ʽ1
	TH1 = TH1OFDELAY;
	TL1 = TL1OFDELAY;
	ET1 = 1;             //�򿪶�ʱ���ж�1
	
	//����INT1
	IT1 = 1;             //�����ش�����ʽ���½��أ�
	EX1 = 1;
	
	EA = 1;              //�����ж�
}

/*******************************************************************************
/������          ��StartReadIr()
/��������        �����ⲿ�ж�0���ƿ�ʼ��ȡ������ֵ
/����            ����
/���            ����
/******************************************************************************/
void StartReadIr() interrupt 0
{
	if (IRIN == 0)
	{
		switch (int0State)
		{
			case 0:
			{
				TR0 = 1;                         //������ʱ��0
				int0State++;
			};break;
			case 1:
			{
				TR0 = 0;                        //���ȹرռ�ʱ��0
				if (TH0 < IFRECIIVE)            //����û�з�����
				{
					int0State = 0;              //״̬����ʼ��
					TH0 = TH0OF1MS;
	                TL0 = TL0OF1MS;
				}
				else                            //������32λ�ķ�����
				{
					TH0 = 0;
					TL0 = 0;                    //��ʱ��0��ʼ��
					TR0 = 1;                    //������ʱ��0
					int0State++;
				}
			};break;
			case 2:
			{
				TR0 = 0;                        //�رռ�ʱ��0
				tempOfTH0 = TH0;                //�ݴ�TH0�ڵ�ֵ
				TH0 = 0;
				TL0 = 0;                        //��ʱ��0��ʼ��
				TR0 = 1;                        //������ʱ��0
				kInIrValue = receivedBitLen / 8;
			    IrValue[kInIrValue] >>= 1;
			    if (tempOfTH0 > 20)
				{
					TR0 = 0;
					TH0 = TH0OF1MS;
	                TL0 = TL0OF1MS;
					int0State = 0;
					receivedBitLen = 0;
					IRIN = 1;                        //��ʼ���˿�
				}
			    else if (tempOfTH0 > IS1OR0)         //��λ��1
			    {
				    IrValue[kInIrValue] |= 0x80;
			    }
				receivedBitLen++;
			};break;
		}
	}
}

/*************************************************************************
������           ��T0_Timer()
��������         ����ʱ1ms��������9�Σ��ж��Ƿ������ʼ9ms�ĵ͵�λ
����             ����
���             ����
*************************************************************************/
void T0_Timer() interrupt 1
{
	if (lenOfLow < 8)
	{
		if (IRIN == 0)                          //��ȷ���ڵ͵�λ
	    {
		    TH0 = TH0OF1MS;
	        TL0 = TL0OF1MS;
			TR0 = 1;
			lenOfLow++;
	    }
		else                                   //����Ĵ���
		{
			TH0 = TH0OF1MS;
			TL0 = TL0OF1MS;
			lenOfLow = 0;
		}
	}
	else                                       //���һ�δ�����ʱ��0����������Ҫ��ߵ�λ��ʱ����
	{
			TH0 = 0;
			TL0 = 0;                           //��ʱ��ģʽ
			TR0 = 1;                           //������ʱ��
			lenOfLow = 0;                      //��������ʼ����Ϊ��һ�ν�����׼��
	}
}

/*******************************************************************************
* ������         : PositionCheck()
* ��������		 : �ⲿ�ж�1���жϺ���
* ����           : ��
* ���         	 : ��
*******************************************************************************/
void PositionCheck() interrupt 2		   //�ⲿ�ж�1���жϺ���
{	
	TR1 = 1;               //������ʱ���ж�1
    g_disPlayStart = 1;
}

/******************************************************************************
*������       ��Timer1() interrupt 3
*��������     ����ʱ���ж�1�ĺ���
*����         ����
*���         ����
******************************************************************************/
void TimerofDisplay() interrupt 3
{
	if (g_disPlayStart)
	{
		//���¿�����ʱ�� ʱ�䣺0.25ms
		TH1 = TH1OFDISPLAY;
	    TL1 = TL1OFDISPLAY;
		
		g_disPlayStart = 0;    //��������ʼ״̬
	}
	else
	{
		//���¿�����ʱ�� ʱ�䣺0.25ms
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
