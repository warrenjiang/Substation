#include "main.h"

static  DateTime nowdate;
static  uint8_t time_zone = 39;																			/*NTP�������˿ں�*/
static  uint8_t ntp_message[48]={0};
uint8_t ntp_port = 123; 	
uint8_t ntp_server_ip[4]={120,25,108,11}; 										  /*������NTP������*/

/*
*********************************************************************************************************
 * ��������app_plat_IsLeapYear
 * ����  ���ж��Ƿ�Ϊ����
 * ����  ��year,��
 * ����  : 1,�� 0,����
*********************************************************************************************************
*/
static uint8_t app_nrf_IsLeapYear(uint16_t year)
{			  
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}
/*
*********************************************************************************************************
 * ��������app_plat_TimeTosec
 * ����  ���ж��Ƿ�Ϊ����
 * ����  ��syear,smon,sday,hour,min,min,sec
 * ����  : seccount,
*********************************************************************************************************
*/
uint32_t app_nrf_TimeTosec(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;

	uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//��������ݵ��������
	{
		if(1 == app_nrf_IsLeapYear(t))seccount+=31622400;//�����������
		else seccount+=31536000;			  //ƽ���������
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //��ǰ���·ݵ����������
	{
		seccount+=(uint32_t)mon_table[t]*86400;//�·����������
		if(app_nrf_IsLeapYear(syear)&&t==1)seccount+=86400;//����2�·�����һ���������	   
	}
	
	seccount+=(uint32_t)(sday-1)*86400;//��ǰ�����ڵ���������� 
	seccount+=(uint32_t)hour*3600;//Сʱ������
  seccount+=(uint32_t)min*60;	 //����������
	seccount+=sec;//�������Ӽ���ȥ
  
	return seccount;	    
}
/**
*@brief			��������ʱ��
*@param			seconds�UUTC �����׼ʱ�� 
*@return		��
*/
static void calc_date_time(tstamp seconds)
{
	uint8_t yf=0;
	uint32_t p_year_total_sec;
	uint32_t r_year_total_sec;
	tstamp n=0,d=0,total_d=0,rz=0;
	uint16_t y=0,r=0,yr=0;
	signed long long yd=0;
	
	n = seconds;
	total_d = seconds/(SECS_PERDAY);
	d=0;
	p_year_total_sec=SECS_PERDAY*365;
	r_year_total_sec=SECS_PERDAY*366;
	while(n>=p_year_total_sec) 
	{
		if((EPOCH+r)%400==0 || ((EPOCH+r)%100!=0 && (EPOCH+r)%4==0))
		{
			n = n -(r_year_total_sec);
			d = d + 366;
		}
		else
		{
			n = n - (p_year_total_sec);
			d = d + 365;
		}
		r+=1;
		y+=1;

	}

	y += EPOCH;

	nowdate.time.year[1] = (uint8_t)((y & 0xff00)>>8);
	nowdate.time.year[0] = (uint8_t)(y & 0xff);

	yd=0;
	yd = total_d - d;

	yf=1;
	while(yd>=28) 
	{
	
		if(yf==1 || yf==3 || yf==5 || yf==7 || yf==8 || yf==10 || yf==12)
		{
			yd -= 31;
			if(yd<0)break;
			rz += 31;
		}
		
		if (yf==2)
		{
			if (y%400==0 || (y%100!=0 && y%4==0)) 
			{
				yd -= 29;
				if(yd<0)break;
				rz += 29;
			}
			else 
			{
				yd -= 28;
				if(yd<0)break;
				rz += 28;
			}
		} 
		if(yf==4 || yf==6 || yf==9 || yf==11 )
		{
			yd -= 30;
			if(yd<0)break;
			rz += 30;
		}
		yf += 1;

	}
	nowdate.time.month=yf;
	yr = total_d-d-rz;
	
	yr += 1;
	
	nowdate.time.day=yr;
	
	//calculation for time
	seconds = seconds%SECS_PERDAY;
	nowdate.time.hour = seconds/3600;
	nowdate.time.minute = (seconds%3600)/60;
	nowdate.time.second = (seconds%3600)%60;
}

/**
*@brief			�ı�����ʱ��Ϊ��
*@param		  seconds�� 
*@return		��
*/
static tstamp change_datetime_to_seconds(void) 
{
	tstamp seconds=0;
	uint32_t total_day=0;
	uint16_t i=0,run_year_cnt=0,l=0;
	
	l = nowdate.time.year[0];//high
	
	l = (l<<8);
	
	l = l + nowdate.time.year[1];//low
	
	
	for(i=EPOCH;i<l;i++)
	{
		if((i%400==0) || ((i%100!=0) && (i%4==0))) 
		{
			run_year_cnt += 1;
		}
	}
	
	total_day=(l-EPOCH-run_year_cnt)*365+run_year_cnt*366;
	
	for(i=1;i<=nowdate.time.month;i++)
	{
		if(i==5 || i==7 || i==10 || i==12)
		{
			total_day += 30;
		}
		if (i==3)
		{
			if (l%400==0 && l%100!=0 && l%4==0) 
			{
				total_day += 29;
			}
			else 
			{
				total_day += 28;
			}
		} 
		if(i==2 || i==4 || i==6 || i==8 || i==9 || i==11)
		{
			total_day += 31;
		}
	}
	
	seconds = (total_day+nowdate.time.day-1)*24*3600;  
	seconds += nowdate.time.second;
	seconds += nowdate.time.minute*60;
	seconds += nowdate.time.hour*3600;
	
	return seconds;
}

/**
*@brief		��NTP��������ȡʱ��
*@param		buf����Ż���
*@param		idx��������������ʼλ��
*@return	��
*/
void get_seconds_from_ntp_server(uint8_t* buf,uint16_t idx)
{
	tstamp seconds = 0;
	uint8_t i=0;
	for (i = 0; i < 4; i++)
	{
		seconds = (seconds << 8) | buf[idx + i];
	}
	switch (time_zone)
	{
		case 0:
			seconds -=  12*3600;
		break;
		case 1:
			seconds -=  11*3600;
		break;
		case 2:
			seconds -=  10*3600;
		break;
		case 3:
			seconds -=  (9*3600+30*60);
		break;
		case 4:
			seconds -=  9*3600;
		break;
		case 5:
		case 6:
			seconds -=  8*3600;
		break;
		case 7:
		case 8:
			seconds -=  7*3600;
		break;
		case 9:
		case 10:
			seconds -=  6*3600;
		break;
		case 11:
		case 12:
		case 13:
			seconds -= 5*3600;
		break;
		case 14:
			seconds -=  (4*3600+30*60);
		break;
		case 15:
		case 16:
			seconds -=  4*3600;
		break;
		case 17:
			seconds -=  (3*3600+30*60);
		break;
		case 18:
			seconds -=  3*3600;
		break;
		case 19:
			seconds -=  2*3600;
		break;
		case 20:
			seconds -=  1*3600;
		break;
		case 21:                           
		case 22:
		break;
		case 23:
		case 24:
		case 25:
			seconds +=  1*3600;
		break;
		case 26:
		case 27:
			seconds +=  2*3600;
		break;
		case 28:
		case 29:
			seconds +=  3*3600;
		break;
		case 30:
			seconds +=  (3*3600+30*60);
		break;
		case 31:
			seconds +=  4*3600;
		break;
		case 32:
			seconds +=  (4*3600+30*60);
		break;
		case 33:
			seconds +=  5*3600;
		break;
		case 34:
			seconds +=  (5*3600+30*60);
		break;
		case 35:
			seconds +=  (5*3600+45*60);
		break;
		case 36:
			seconds +=  6*3600;
		break;
		case 37:
			seconds +=  (6*3600+30*60);
		break;
		case 38:
			seconds +=  7*3600;
		break;
		case 39:
			seconds +=  8*3600;
		break;
		case 40:
			seconds +=  9*3600;
		break;
		case 41:
			seconds +=  (9*3600+30*60);
		break;
		case 42:
			seconds +=  10*3600;
		break;
		case 43:
			seconds +=  (10*3600+30*60);
		break;
		case 44:
			seconds +=  11*3600;
		break;
		case 45:
			seconds +=  (11*3600+30*60);
		break;
		case 46:
			seconds +=  12*3600;
		break;
		case 47:
			seconds +=  (12*3600+45*60);
		break;
		case 48:
			seconds +=  13*3600;
		break;
		case 49:
			seconds +=  14*3600;
		break;
	}
	  calc_date_time(seconds); 																									/*��UTCʱ���������*/
	
}


/*��ȡntp����ʱ��*/
void get_ntp_time(DateTime *time)
{
	memcpy(time->data,nowdate.data,sizeof(nowdate.data));
}