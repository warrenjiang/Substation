#ifndef	__APP_NTP_H__
#define	__APP_NTP_H__
#include "stm32f10x.h"
/* for ntpclient */ 

#define SECS_PERDAY     	    86400UL             	  /* seconds in a day = 60*60*24 */
#define UTC_ADJ_HRS         	9              	        /* SEOUL : GMT+9 */
#define EPOCH                 1900                    /* NTP start year */

extern uint8_t ntp_server_ip[4];
extern uint8_t ntp_port;
typedef signed char s_char;
typedef unsigned long long tstamp;
typedef unsigned int tdist;



#pragma pack(1)
typedef struct _DateTime	/*�˽ṹ�嶨����NTPʱ��ͬ������ر���*/
{
  volatile uint8_t year[2];																			  /*��Ϊ2�ֽ� ��Ϊ�ߵ���λ */
  volatile uint8_t month;																				  /*	�� */
  volatile uint8_t day;																					  /*	�� */
  volatile uint8_t hour;																					/*	ʱ */
  volatile uint8_t minute;																				/*	�� */
  volatile uint8_t second;																				/*	�� */
} timepara;
typedef union 
{
	timepara time;
	uint8_t data[7];
}DateTime;
#pragma pack()

void get_ntp_time(DateTime *time);
void get_seconds_from_ntp_server(uint8_t* buf,uint16_t idx);
uint32_t app_nrf_TimeTosec(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);

#endif




