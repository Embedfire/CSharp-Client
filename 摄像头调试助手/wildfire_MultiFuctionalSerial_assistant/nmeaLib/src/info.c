/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: info.c 17 2008-03-11 11:56:11Z xtimor $
 *
 */

#include <string.h>

#include "nmea/info.h"

#include "nmea/context.h"
#include <stdio.h>
#include <io.h>

void trace(const char *str, int str_size)
{
	printf("Trace: ");
	write(1, str, str_size);
	printf("\n");
}
void error(const char *str, int str_size)
{
	printf("Error: ");
	write(1, str, str_size);
	printf("\n");
}

void nmea_zero_INFO(nmeaINFO *info)
{
    memset(info, 0, sizeof(nmeaINFO));
    nmea_time_now(&info->utc);
    info->sig = NMEA_SIG_BAD;
    info->fix = NMEA_FIX_BAD;

	//nmea_property()->trace_func = &trace;
	//nmea_property()->error_func = &error;
}



/********************************************************************************************************
**     ��������:            bit        IsLeapYear(uint8_t    iYear)
**    ��������:            �ж�����(�������2000�Ժ�����)
**    ��ڲ�����            iYear    ��λ����
**    ���ڲ���:            uint8_t        1:Ϊ����    0:Ϊƽ��
********************************************************************************************************/
static int IsLeapYear(int iYear)
{
	int    Year;
	Year = 2000 + iYear;
	if ((Year & 3) == 0)
	{
		return ((Year % 400 == 0) || (Year % 100 != 0));
	}
	return 0;
}

/********************************************************************************************************
**     ��������:            void    GMTconvert(uint8_t *DT,uint8_t GMT,uint8_t AREA)
**    ��������:            ��������ʱ�任�������ʱ��ʱ��
**    ��ڲ�����            *DT:    ��ʾ����ʱ������� ��ʽ YY,MM,DD,HH,MM,SS
**                        GMT:    ʱ����
**                        AREA:    1(+)���� W0(-)����
********************************************************************************************************/
void    GMTconvert(nmeaTIME *SourceTime, nmeaTIME *ConvertTime, int GMT, int AREA)
{
	int    YY, MM, DD, hh, mm, ss;        //������ʱ�����ݴ���� 

	if (GMT == 0)    return;                //�������0ʱ��ֱ�ӷ��� 
	if (GMT>12)    return;                //ʱ�����Ϊ12 �����򷵻�         

	YY = SourceTime->year;                //��ȡ�� 
	MM = SourceTime->mon;                 //��ȡ�� 
	DD = SourceTime->day;                 //��ȡ�� 
	hh = SourceTime->hour;                //��ȡʱ 
	mm = SourceTime->min;                 //��ȡ�� 
	ss = SourceTime->sec;                 //��ȡ�� 

	if (AREA)                        //��(+)ʱ������ 
	{
		if (hh + GMT<24)    hh += GMT;//������������ʱ�䴦��ͬһ�������Сʱ���� 
		else                        //����Ѿ����ڸ�������ʱ��1����������ڴ��� 
		{
			hh = hh + GMT - 24;        //�ȵó�ʱ�� 
			if (MM == 1 || MM == 3 || MM == 5 || MM == 7 || MM == 8 || MM == 10)    //���·�(12�µ�������) 
			{
				if (DD<31)    DD++;
				else
				{
					DD = 1;
					MM++;
				}
			}
			else if (MM == 4 || MM == 6 || MM == 9 || MM == 11)                //С�·�2�µ�������) 
			{
				if (DD<30)    DD++;
				else
				{
					DD = 1;
					MM++;
				}
			}
			else if (MM == 2)    //����2�·� 
			{
				if ((DD == 29) || (DD == 28 && IsLeapYear(YY) == 0))        //��������������2��29�� ���߲�����������2��28�� 
				{
					DD = 1;
					MM++;
				}
				else    DD++;
			}
			else if (MM == 12)    //����12�·� 
			{
				if (DD<31)    DD++;
				else        //�������һ�� 
				{
					DD = 1;
					MM = 1;
					YY++;
				}
			}
		}
	}
	else
	{
		if (hh >= GMT)    hh -= GMT;    //������������ʱ�䴦��ͬһ�������Сʱ���� 
		else                        //����Ѿ����ڸ�������ʱ��1����������ڴ��� 
		{
			hh = hh + 24 - GMT;        //�ȵó�ʱ�� 
			if (MM == 2 || MM == 4 || MM == 6 || MM == 8 || MM == 9 || MM == 11)    //�����Ǵ��·�(1�µ�������) 
			{
				if (DD>1)    DD--;
				else
				{
					DD = 31;
					MM--;
				}
			}
			else if (MM == 5 || MM == 7 || MM == 10 || MM == 12)                //������С�·�2�µ�������) 
			{
				if (DD>1)    DD--;
				else
				{
					DD = 30;
					MM--;
				}
			}
			else if (MM == 3)    //�����ϸ�����2�·� 
			{
				if ((DD == 1) && IsLeapYear(YY) == 0)                    //�������� 
				{
					DD = 28;
					MM--;
				}
				else    DD--;
			}
			else if (MM == 1)    //����1�·� 
			{
				if (DD>1)    DD--;
				else        //�����һ�� 
				{
					DD = 31;
					MM = 12;
					YY--;
				}
			}
		}
	}

	ConvertTime->year = YY+1900;           //������ 
	ConvertTime->mon = MM+1;                //������ 
	ConvertTime->day = DD;                //������ 
	ConvertTime->hour = hh;                //����ʱ 
	ConvertTime->min = mm;                //���·� 
	ConvertTime->sec = ss;                //������ 
}

/*��γ��ת�� �� ����nmea�� �ȷ�.xx ��ʽ����� ��.xxx */
double DegreeConvert(double sDegree)
{
	double dDegree;

	if (sDegree == 0)return 0;

	int integer = (int)sDegree;
	double decimal = sDegree - (int)sDegree;

	double min = integer % 100;
	int hour =(int) (integer / 100);

	dDegree = (double)hour + (double)(min / 60) + (double)(decimal / 60);

	

	return dDegree;
	
}

