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
#include "bsp_led.h"
#include "bsp_iwdg.h"
#include "bsp_can.h"
#include "bsp_cpu_flash.h"
#include "bsp_mqtt_ota.h"
#include "bsp_timer.h"
#include "bsp_usart.h"
#include "tcp_client.h"
#include "TestMaster.h"
#include "MQTTPacket.h"

#include "app_flash.h"
#include "app_Plat.h"
#include "app_ntp.h"


#define  APP_DEBUG  1

/*本机IP*/
#define MyIP1  192
#define MyIP2  168
#define MyIP3  0
#define MyIP4  198

/* 宏定义 --------------------------------------------------------------------*/
#define CANTX_QUEUE_LEN           10                       //CAN队列长度(队列的数量)(发送)
#define CANTX_QUEUE_SIZE          19                       //CAN队列大小(一个队列长度) - sizeof(CanTxMsg)
#define CANRX_QUEUE_LEN           10                       //CAN队列长度(队列的数量)(接收)
#define CANRX_QUEUE_SIZE          20                       //CAN队列大小(一个队列长度) - sizeof(CanRxMsg)
/* 宏定义 --------------------------------------------------------------------*/
#define APP_STACK_SIZE            128                      //开始任务堆栈大小
#define APP_TASK_PRIORITY         1                        //开始任务优先级

#define CANOPEN_STACK_SIZE        128                      //CANOPEN任务堆栈大小
#define CANOPEN_TASK_PRIORITY     2                        //CANOPEN任务优先级

#define CANTX_STACK_SIZE          256                      //CAN通信任务堆栈大小(发送)
#define CANTX_TASK_PRIORITY       3                        //CAN通信任务优先级

#define CANRX_STACK_SIZE          256                      //CAN通信任务堆栈大小(接收)
#define CANRX_TASK_PRIORITY       3                        //CAN通信任务优先级
/* 静态变量 ------------------------------------------------------------------*/
static xQueueHandle xCANSendQueue = NULL;        //CAN发送队列
static xQueueHandle xCANRcvQueue = NULL;         //CAN接收队列
static SemaphoreHandle_t  xMutex = NULL;        //互斥信号量
/* 定时器TIM相关变量 */
static TIMEVAL last_counter_val = 0;
static TIMEVAL elapsed_time = 0;

/* 函数申明 ------------------------------------------------------------------*/
void CANOpen_App_Init(void);
void CANOpen_Driver_Init(void);
void CANSend_Date(CanTxMsg TxMsg);
void CANRcv_DateFromISR(CanRxMsg *RxMsg);
void TIMx_DispatchFromISR(void);
/* 静态申明 ------------------------------------------------------------------*/
static void CANSend_Task(void *pvParameters);
static void CANRcv_Task(void *pvParameters);
static void CANOpen_App_Task(void *pvParameters);
void  App_Printf(char *format, ...);

extern void LwIP_Init(void);
extern void Eth_config(void);
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


