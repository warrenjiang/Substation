#include "main.h"

static uint16_t hash[MAX_HASH_LEN]={0};
extern struct tcp_pcb *main_pcb; //TCP 通信块
extern struct tcp_pcb *bus_pcb; //TCP 通信块
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
/************************************************
函数名称 ： ID_Update
功    能 ： 从站PDO发送到主站 主站参数改变后回调 将数据放入消息队列中
参    数 ： pvParameters --- 可选参数
返 回 值 ： 无
*************************************************/
UNS32 ID_Update(CO_Data* d, const indextable *indextable, UNS8 bSubindex)
{
   portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	 uint8_t buf[5]={0};
	 uint32_t ID=0;
	 uint8_t  ReaderID=indextable->index+1;
	 ID=*(uint32_t*)indextable->pSubindex->pObject;
	 buf[0]=ReaderID;
	 memcpy(&buf[1],(char*)&ID,4);
     /* 判断是否在执行中断 */
  if(0 == __get_CONTROL())
  {
    if(xQueueSendFromISR(xMQTTSendQueue, buf, &xHigherPriorityTaskWoken) != pdPASS)
    {                                            //加入队列失败
      return 0xFF;
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
  }
  else
  {
    if(xQueueSend(xMQTTSendQueue, buf, 100) != pdPASS)
    {                                            //加入队列失败
      return 0xFF;
    }
  }
  return 0;
}
/************************************************
函数名称 ： MQTT_Task
功    能 ： MQTT推送应用任务程序，阻塞接收Can接收队列中的消息
参    数 ： pvParameters --- 可选参数
						格式：1600 B1 0000244318560738 02 2200 0100 736CEF5D 00000000 FDF0
返 回 值 ： 无
*************************************************/
static void MQTT_Task(void *pvParameters)
{
  /*CanId[0]   读卡器ID
	  CanId[1-2]  人员ID 
	  CanId[3-4]  位置ID 
	*/
  uint8_t CanId[5]={0};
	DateTime nowtime;
	uint32_t nowsec=0;
	uint32_t serialnumber=0;
	uint8_t IdHex[26]={0x16,0x00,0xB1,0x00,0x00};
	uint16_t crcdata;
	uint16_t PersonID=0;
	uint16_t LocationID=0;
	struct tcp_pcb *pcb=NULL;
	/*等待连接*/
	while(MQTT_DISCONNECT == app_system_mqtt_connect_state_get(sysCfg.parameter.data_socket))
	{
		vTaskDelay(1000);
	}
	vTaskDelay(1000);
	/*确定数据端口*/
	pcb=(sysCfg.parameter.data_socket==SOCK_MAIN?main_pcb:bus_pcb);
	/*设置主节点ID*/
	setNodeId(&TestMaster_Data, 0);
	/*设置状态，启动Canopen协议栈*/
  setState(&TestMaster_Data, Initialisation);
  setState(&TestMaster_Data, Operational);
	/*注册字典参数更新后的回调函数*/
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2000, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2001, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2002, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2003, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2004, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2005, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2006, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2007, 0x00, ID_Update);
  while(1)
  {
			if(xQueueReceive(xMQTTSendQueue, CanId, 10) == pdTRUE)
			{
				 /*关闭所有中断*/
				 __set_PRIMASK(1); 
				 #if FILTER
				 /*当前人员*/
				 PersonID=CanId[1]|CanId[2]<<8;
				/*当前位置*/
				 LocationID=CanId[3]|CanId[4]<<8;
				/*表中未记录 */
				 if(LocationID!=hash[PersonID])
				 {		
					 /*更新列表中的位置*/
					 hash[PersonID]=LocationID;
					 //App_Printf("hash[%d]=%d\r\n",PersonID,LocationID);
				#endif
					 /*获取时间*/			
					 get_ntp_time(&nowtime);
					 nowsec = app_nrf_TimeTosec(nowtime.time.year[0]+nowtime.time.year[1]*256,nowtime.time.month,
					 nowtime.time.day,nowtime.time.hour,nowtime.time.minute,nowtime.time.second);                   
					 memcpy(&IdHex[5],sysCfg.parameter.client_mac,6); /*网关ID*/
					 IdHex[11]=CanId[0];
					 memcpy(&IdHex[12],&CanId[1],4); 
					 memcpy(&IdHex[16],(uint8_t*)&nowsec,4);	
					 memcpy(&IdHex[20],(uint8_t*)&serialnumber,4);     /*数据包流水号*/
					 /*CRC校验*/
					 crcdata=app_plat_usMBCRC16(IdHex,IdHex[1]*256+IdHex[0]+2);
					 memcpy(&IdHex[24],(uint8_t *)&crcdata,2);  
					 mqtt_publish( pcb, "ID" , (char *)IdHex , 26,0 );
				 #if FILTER
				 }
				 #endif
				 /*开启所有中断*/
				__set_PRIMASK(0); 	
			}
  }
}
/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：无
*	返 回 值: 无 数值越大 优先级越高
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
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
		/* 创建以太网定时任务*/
	xTaskCreate(Eth_Task, 
							"Eth_Task", 
							256, 
							NULL, 
							4, 
							NULL); 
		/* 创建以太网协议栈任务*/
	xTaskCreate(Task_TCP_Client, 
							"Task_TCP_Client", 
							512, 
							NULL, 
							3, 
							NULL); 
		/* 创建MQTT推送任务*/
	xTaskCreate( MQTT_Task, 
							"MQTT_Task", 
							256, 
							NULL, 
							2, 
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
	  /* 创建队列  消息发送队列*/
  if(xMQTTSendQueue == NULL)
  {
    xMQTTSendQueue = xQueueCreate(MQTT_QUEUE_LEN, MQTT_QUEUE_SIZE);
    if(xMQTTSendQueue == NULL)
    {
      printf("CANSendQueue create failed");
      return;                                    //创建发送队列失败
    }
  }
		/* 创建互斥信号量  用于打印互斥*/
   xMutex = xSemaphoreCreateMutex();
	
	if(xMutex == NULL)
    {
       printf("xSemaphoreCreateMutex failed\r\n");
			 return;
    }
				/* 创建互斥信号量  */
   Id_Updata = xSemaphoreCreateMutex();
	
	if(Id_Updata == NULL)
    {
       printf("xSemaphoreCreateMutex failed\r\n");
			 return;
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
    /*创建任务通信机制 */
    AppObjCreate(); 
    /*创建以太网内核任务*/	
    AppTaskCreate();
	
		vTaskStartScheduler();

		/* We should never get here as control is now taken by the scheduler */
		while(1);	
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
/*******************************************************************************
* 函数名  : EXTI9_5_IRQHandler
* 描述    : 按键的中断函数
*         ：中断线5中断服务函数（按键中断服务函数）
* 输入    : 无
* 返回值  : 无
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
  
	
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line5);
   
		delay_ms(20);  /*硬延时 防抖动*/

    if(0 == GPIO_ReadInputDataBit( GPIOE , GPIO_Pin_5 ))	     /*按键按下*/
		{			
			bsp_LedOn(LED_RED);                                    /*按下开始按键计时*/
			key_timer_Start();    
			sysCfg.parameter.key_time_count = 0;
			bsp_Initkey_Triggertype(EXTI_Trigger_Rising);            /*检测上升沿*/
		}
		else if(1 == GPIO_ReadInputDataBit( GPIOE , GPIO_Pin_5 )) /*按键松开*/ 	
		{
			bsp_LedOff(LED_RED);                                       /*关闭按键计时*/
			key_timer_Stop();  
			sysCfg.parameter.key_time_count = 0;
			bsp_Initkey_Triggertype(EXTI_Trigger_Falling);           /*检测下降沿*/
		}
	}	
  if(EXTI_GetITStatus(EXTI_Line6) != RESET)
  {
		
		EXTI_ClearITPendingBit(EXTI_Line6);
		if(((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_MISR)) & PHY_LINK_STATUS) != 0)
      {
        if(0==(ETH_ReadPHYRegister(PHY_ADDRESS, PHY_SR) & 1))//网线未插上
        {  
						 delay_ms(100);
				     NVIC_SystemReset();	
        }
      }
				 
  }
}
/*按键定时器 100ms*/
void TIM3_IRQHandler(void)
{		
  if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
		TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);
		
		sysCfg.parameter.key_time_count++;           
		 
		if(sysCfg.parameter.key_time_count == 50)    /*长按5s*/
		{
	    sysCfg.parameter.config_hold_flag = 0xff;  /*恢复出厂设置*/
			sysCfg.parameter.dhcp = DHCP;			 /*默认为开启DHCP*/
			if(0 == bsp_WriteCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN))
			{
				delay_ms(100);
				NVIC_SystemReset();		
			}
		}
	}
}


