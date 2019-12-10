#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm32f10x.h"
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <ctype.h>


#include "Crc_data.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "bsp.h"
#include "TestMaster.h"

#include "app_tcp_client.h"
#include "app_flash.h"
#include "app_Plat.h"
#include "app_ntp.h"
#include "app_udp.h"
#include "app_udp_pc.h"
#include "app_mqtt.h"

#define  MAX_HASH_LEN 1000
#define  FILTER     0
#define  APP_DEBUG  1

/*����IP*/
#define MyIP1  192
#define MyIP2  168
#define MyIP3  0
#define MyIP4  198

/* �궨�� --------------------------------------------------------------------*/
#define CANTX_QUEUE_LEN           10                       //CAN���г���(���е�����)(����)
#define CANTX_QUEUE_SIZE          19                       //CAN���д�С(һ�����г���) - sizeof(CanTxMsg)
#define CANRX_QUEUE_LEN           100                       //CAN���г���(���е�����)(����)
#define CANRX_QUEUE_SIZE          20                       //CAN���д�С(һ�����г���) - sizeof(CanRxMsg)

#define MQTT_QUEUE_LEN           200                       
#define MQTT_QUEUE_SIZE          5                      
/* �궨�� --------------------------------------------------------------------*/
#define APP_STACK_SIZE            128                      //��ʼ�����ջ��С
#define APP_TASK_PRIORITY         1                        //��ʼ�������ȼ�

#define CANOPEN_STACK_SIZE        128                      //CANOPEN�����ջ��С
#define CANOPEN_TASK_PRIORITY     2                        //CANOPEN�������ȼ�

#define CANTX_STACK_SIZE          256                      //CANͨ�������ջ��С(����)
#define CANTX_TASK_PRIORITY       3                        //CANͨ���������ȼ�

#define CANRX_STACK_SIZE          256                      //CANͨ�������ջ��С(����)
#define CANRX_TASK_PRIORITY       3                        //CANͨ���������ȼ�
/* ��̬���� ------------------------------------------------------------------*/
static xQueueHandle xCANSendQueue = NULL;        //CAN���Ͷ���
static xQueueHandle xCANRcvQueue = NULL;         //CAN���ն���
static xQueueHandle xMQTTSendQueue = NULL;         //MQTT���Ͷ���
static SemaphoreHandle_t  xMutex = NULL;        //�����ź���
static SemaphoreHandle_t  Id_Updata = NULL;      //ID����
/* ��ʱ��TIM��ر��� */
static TIMEVAL last_counter_val = 0;
static TIMEVAL elapsed_time = 0;

/* �������� ------------------------------------------------------------------*/
void CANOpen_App_Init(void);
void CANOpen_Driver_Init(void);
void CANSend_Date(CanTxMsg TxMsg);
void CANRcv_DateFromISR(CanRxMsg *RxMsg);
void TIMx_DispatchFromISR(void);
/* ��̬���� ------------------------------------------------------------------*/
static void CANSend_Task(void *pvParameters);
static void CANRcv_Task(void *pvParameters);
static void CANOpen_App_Task(void *pvParameters);
void  App_Printf(char *format, ...);

extern void LwIP_Init(void);
extern void Eth_Task( void *pvParameters);
extern void Task_TCP_Client(void *pvParameters);
#define LED1_ON     GPIO_ResetBits(GPIOD, GPIO_Pin_11);
#define LED1_OFF    GPIO_SetBits(GPIOD, GPIO_Pin_11);

#define LED2_ON     GPIO_ResetBits(GPIOD, GPIO_Pin_12);
#define LED2_OFF    GPIO_SetBits(GPIOD, GPIO_Pin_12);

#define LED3_ON     GPIO_ResetBits(GPIOD, GPIO_Pin_13);
#define LED3_OFF    GPIO_SetBits(GPIOD, GPIO_Pin_13);

#define LED4_ON     GPIO_ResetBits(GPIOD, GPIO_Pin_14);
#define LED4_OFF    GPIO_SetBits(GPIOD, GPIO_Pin_14);


#endif 


