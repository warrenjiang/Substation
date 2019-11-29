#include "main.h"



/************************************************
函数名称 ： CANSend_Task
功    能 ： CAN发送应用任务程序,阻塞接收Can发送队列里的消息
参    数 ： pvParameters --- 可选参数
返 回 值 ： 无
*************************************************/
static void CANSend_Task(void *pvParameters)
{
  static CanTxMsg TxMsg;

  while(1)
  {
    /* 等待接收有效数据包 */
    if(xQueueReceive(xCANSendQueue, &TxMsg, 100) == pdTRUE)
    {
      if(CAN_Transmit(CANx, &TxMsg) == CAN_NO_MB)
      {
        vTaskDelay(1);                           //第一次发送失败, 延时1个滴答, 再次发送
        CAN_Transmit(CANx, &TxMsg);
      }
    }
  }
}
/************************************************
函数名称 ： CANRcv_Task
功    能 ： CAN接收应用任务程序，阻塞接收Can接收队列中的消息
参    数 ： pvParameters --- 可选参数
返 回 值 ： 无
*************************************************/
static void CANRcv_Task(void *pvParameters)
{
  static CanRxMsg RxMsg;
  static Message msg;

  uint8_t i = 0;

  while(1)
  {
    if(xQueueReceive(xCANRcvQueue, &RxMsg, 100) == pdTRUE)
    {
      msg.cob_id = RxMsg.StdId;                  //CAN-ID
      if(CAN_RTR_REMOTE == RxMsg.RTR)
        msg.rtr = 1;                             //远程帧
      else
        msg.rtr = 0;                             //数据帧

      msg.len = (UNS8)RxMsg.DLC;                 //长度

      for(i=0; i<RxMsg.DLC; i++)                 //数据
       msg.data[i] = RxMsg.Data[i];
      TIM_ITConfig(CANOPEN_TIMx, TIM_IT_Update, DISABLE);
      canDispatch(&TestMaster_Data, &msg);       //调用协议相关接口
      TIM_ITConfig(CANOPEN_TIMx, TIM_IT_Update, ENABLE);
    }
  }
}
/*********************************************
函数名称 ： APP_Task
功    能 ： 应用程序(开始任务)
参    数 ： pvParameters --- 可选参数
返 回 值 ： 无
*************************************************/
//static void APP_Task(void *pvParameters)
//{   
//  uint8_t pcWriteBuffer[250];
//  while(1)
//  {
//    vTaskDelay(5000);
//    vTaskList((char *)&pcWriteBuffer);
//    App_Printf("%s\r\n", pcWriteBuffer); 
//	//IWDG_Feed();  /*喂狗*/
//  }
//}
/*
*********************************************************************************************************
*	函 数 名: CANOpen_App_Task
*	功能说明: Canopen 应用层代码
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级: 2  
*********************************************************************************************************
*/
static void CANOpen_App_Task(void *pvParameters)
{
  unsigned char nodeID = 0x01;                   //节点ID
  setNodeId(&TestMaster_Data, nodeID);
  setState(&TestMaster_Data, Initialisation);
  setState(&TestMaster_Data, Operational);
//	RegisterSetODentryCallBack(&TestMaster_Data, 0x2000, 0x00, &ID1_Update);
//	RegisterSetODentryCallBack(&TestMaster_Data, 0x2001, 0x00, &ID2_Update);
//	RegisterSetODentryCallBack(&TestMaster_Data, 0x2002, 0x00, &ID3_Update);
//	RegisterSetODentryCallBack(&TestMaster_Data, 0x2003, 0x00, &ID4_Update);
//	RegisterSetODentryCallBack(&TestMaster_Data, 0x2004, 0x00, &ID5_Update);
//	RegisterSetODentryCallBack(&TestMaster_Data, 0x2005, 0x00, &ID6_Update);
//	RegisterSetODentryCallBack(&TestMaster_Data, 0x2006, 0x00, &ID7_Update);
//	RegisterSetODentryCallBack(&TestMaster_Data, 0x2007, 0x00, &ID8_Update);
  while(1)
  {
    vTaskDelay(1000);
   /* 应用代码 */
  }
}/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：无
*	返 回 值: 无 数值越大 优先级越高
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{

//  /* 创建应用程序(开始任务) */
//   xTaskCreate(APP_Task,
//							"APP_Task", 
//	             256, 
//	             NULL, 
//	             1, 
//	             NULL);

	/* 创建任务 发送Can消息任务*/
   xTaskCreate(CANSend_Task,
							"CANSend_Task", 
	             256, 
	             NULL, 
	             1, 
	             NULL);
   /* 创建任务 接收Can消息任务*/
  xTaskCreate(CANRcv_Task, 
							"CANRcv_Task", 
							 256,
							 NULL, 
	             1,
							 NULL);
  /* 创建任务 协议层应用任务*/
	xTaskCreate(CANOpen_App_Task, 
							"CANOpen_App_Task", 
							256, 
							NULL, 
							1, 
							NULL); 
		/* 创建以太网定时任务*/
	xTaskCreate(Eth_Task, 
							"Eth_Task", 
							256, 
							NULL, 
							2, 
							NULL); 
		/* 创建以太网协议栈任务*/
	xTaskCreate(Task_TCP_Client, 
							"Task_TCP_Client", 
							512, 
							NULL, 
							3, 
							NULL); 
}
/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate (void)
{

  /* 创建队列  Can发送队列*/
  if(xCANSendQueue == NULL)
  {
    xCANSendQueue = xQueueCreate(CANTX_QUEUE_LEN, sizeof(CanTxMsg));
    if(xCANSendQueue == NULL)
    {
      printf("CANSendQueue create failed");
      return;                                    //创建发送队列失败
    }
  }
   /* 创建队列  Can接收队列*/
  if(xCANRcvQueue == NULL)
  {
    xCANRcvQueue = xQueueCreate(CANRX_QUEUE_LEN, sizeof(CanRxMsg));
    if(xCANRcvQueue == NULL)
    {
      printf("CANRcvQueue create failed");
      return;                                    //创建接收队列失败
    }
  }
		/* 创建互斥信号量 */
   xMutex = xSemaphoreCreateMutex();
	
	if(xMutex == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}
int main(void)
{
	  /*在任务启动之前先关闭所有中断*/
    __set_PRIMASK(1); 	
	  /*硬件初始化 */
	  BSP_Init(); 
	  /*读取配置参数*/
	  app_flash_LoadSysConfig(); 
    /*STM32F107 ETH初始化*/
	  Eth_config();	
	  /*LWIP初始化*/
  	LwIP_Init();	
    /*创建任务通信机制 */
    AppObjCreate(); 
    /*创建以太网内核任务*/	
    AppTaskCreate();
	
		vTaskStartScheduler();

		/* We should never get here as control is now taken by the scheduler */
		for( ;; );	
}
/************************************************
函数名称 ： CANRcv_DateFromISR
功    能 ： CAN接收数据，由硬件中断调用，接收Can消息，并加入到接收队列中
参    数 ： RxMsg --- 接收数据(队列)
返 回 值 ： 无
*************************************************/
void CANRcv_DateFromISR(CanRxMsg *RxMsg)
{
  static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  if(NULL != xCANRcvQueue)
  {
    xQueueSendFromISR(xCANRcvQueue, RxMsg, &xHigherPriorityTaskWoken);
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
  }
}
/************************************************
函数名称 ： setTimer
功    能 ： 协议栈用于设置闹钟
参    数 ： 无
返 回 值 ： 无
*************************************************/
void setTimer(TIMEVAL value)
{
  uint32_t timer = TIM_GetCounter(CANOPEN_TIMx); // Copy the value of the running timer

  elapsed_time += timer - last_counter_val;
  last_counter_val = CANOPEN_TIM_PERIOD - value;
  TIM_SetCounter(CANOPEN_TIMx, CANOPEN_TIM_PERIOD - value);
  TIM_Cmd(CANOPEN_TIMx, ENABLE);
}

/************************************************
函数名称 ： getElapsedTime
功    能 ： 返回消逝的时间
参    数 ： 无
返 回 值 ： 消逝的时间
*************************************************/
TIMEVAL getElapsedTime(void)
{
  uint32_t timer = TIM_GetCounter(CANOPEN_TIMx); // Copy the value of the running timer

  if(timer < last_counter_val)
    timer += CANOPEN_TIM_PERIOD;

  TIMEVAL elapsed = timer - last_counter_val + elapsed_time;

  return elapsed;
}

/************************************************
函数名称 ： TIMx_DispatchFromISR
功    能 ： 定时调度(从定时器中断)
参    数 ： 无
返 回 值 ： 无
*************************************************/
void TIMx_DispatchFromISR(void)
{
  last_counter_val = 0;
  elapsed_time = 0;
  TimeDispatch(); //驱动整个 协议栈运行的时间
}
/*
函数名称 ： canSend
功    能 ： CAN发送，将消息加到发送队列中
参    数 ： notused --- 未使用参数
            m --------- 消息参数
返 回 值 ： 0:失败  1:成功
*************************************************/
unsigned char canSend(CAN_PORT notused, Message *m)
{
  uint8_t i;
  static CanTxMsg TxMsg;
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  TxMsg.StdId = m->cob_id;

  if(m->rtr)
      TxMsg.RTR = CAN_RTR_REMOTE;
  else
      TxMsg.RTR = CAN_RTR_DATA;

  TxMsg.IDE = CAN_ID_STD;
  TxMsg.DLC = m->len;
  for(i=0; i<m->len; i++)
      TxMsg.Data[i] = m->data[i];

  /* 判断是否在执行中断 */
  if(0 == __get_CONTROL())
  {
    if(xQueueSendFromISR(xCANSendQueue, &TxMsg, &xHigherPriorityTaskWoken) != pdPASS)
    {                                            //加入队列失败
      return 0xFF;
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
  }
  else
  {
    if(xQueueSend(xCANSendQueue, &TxMsg, 100) != pdPASS)
    {                                            //加入队列失败
      return 0xFF;
    }
  }

  return 0;
}

/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式		  			  
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
void  App_Printf(char *format, ...)
{
    char  buf_str[200 + 1];
    va_list   v_args;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* 互斥信号量 */
	  xSemaphoreTake(xMutex, portMAX_DELAY);

    printf("%s", buf_str);

   	xSemaphoreGive(xMutex);
}
