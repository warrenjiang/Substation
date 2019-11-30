#include "main.h"

extern  struct netif netif;

/************************************************
�������� �� CANSend_Task
��    �� �� CAN����Ӧ���������,��������Can���Ͷ��������Ϣ
��    �� �� pvParameters --- ��ѡ����
�� �� ֵ �� ��
*************************************************/
static void CANSend_Task(void *pvParameters)
{
  static CanTxMsg TxMsg;

  while(1)
  {
    /* �ȴ�������Ч���ݰ� */
    if(xQueueReceive(xCANSendQueue, &TxMsg, 100) == pdTRUE)
    {
      if(CAN_Transmit(CANx, &TxMsg) == CAN_NO_MB)
      {
        vTaskDelay(1);                           //��һ�η���ʧ��, ��ʱ1���δ�, �ٴη���
        CAN_Transmit(CANx, &TxMsg);
      }
    }
  }
}
/************************************************
�������� �� CANRcv_Task
��    �� �� CAN����Ӧ�����������������Can���ն����е���Ϣ
��    �� �� pvParameters --- ��ѡ����
�� �� ֵ �� ��
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
        msg.rtr = 1;                             //Զ��֡
      else
        msg.rtr = 0;                             //����֡

      msg.len = (UNS8)RxMsg.DLC;                 //����

      for(i=0; i<RxMsg.DLC; i++)                 //����
       msg.data[i] = RxMsg.Data[i];
      TIM_ITConfig(CANOPEN_TIMx, TIM_IT_Update, DISABLE);
      canDispatch(&TestMaster_Data, &msg);       //����Э����ؽӿ�
      TIM_ITConfig(CANOPEN_TIMx, TIM_IT_Update, ENABLE);
    }
  }
}
/*********************************************
�������� �� APP_Task
��    �� �� Ӧ�ó���(��ʼ����)
��    �� �� pvParameters --- ��ѡ����
�� �� ֵ �� ��
*************************************************/
//static void APP_Task(void *pvParameters)
//{   
//  uint8_t pcWriteBuffer[250];
//  while(1)
//  {
//    vTaskDelay(5000);
//    vTaskList((char *)&pcWriteBuffer);
//    App_Printf("%s\r\n", pcWriteBuffer); 
//	//IWDG_Feed();  /*ι��*/
//  }
//}

/*
*********************************************************************************************************
*	�� �� ��: CANOpen_App_Task
*	����˵��: Canopen Ӧ�ò����
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
* �� �� ��: 2  
*********************************************************************************************************
*/
static void CANOpen_App_Task(void *pvParameters)
{
  unsigned char nodeID = 0x01;                   //�ڵ�ID
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
   /* Ӧ�ô��� */
  }
}/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ���
*	�� �� ֵ: �� ��ֵԽ�� ���ȼ�Խ��
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{

//  /* ����Ӧ�ó���(��ʼ����) */
//   xTaskCreate(APP_Task,
//							"APP_Task", 
//	             256, 
//	             NULL, 
//	             1, 
//	             NULL);

	/* �������� ����Can��Ϣ����*/
   xTaskCreate(CANSend_Task,
							"CANSend_Task", 
	             256, 
	             NULL, 
	             1, 
	             NULL);
   /* �������� ����Can��Ϣ����*/
  xTaskCreate(CANRcv_Task, 
							"CANRcv_Task", 
							 256,
							 NULL, 
	             1,
							 NULL);
  /* �������� Э���Ӧ������*/
	xTaskCreate(CANOpen_App_Task, 
							"CANOpen_App_Task", 
							256, 
							NULL, 
							1, 
							NULL); 
		/* ������̫����ʱ����*/
	xTaskCreate(Eth_Task, 
							"Eth_Task", 
							256, 
							NULL, 
							2, 
							NULL); 
		/* ������̫��Э��ջ����*/
	xTaskCreate(Task_TCP_Client, 
							"Task_TCP_Client", 
							512, 
							NULL, 
							3, 
							NULL); 
}
/*
*********************************************************************************************************
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨ�Ż���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppObjCreate (void)
{

  /* ��������  Can���Ͷ���*/
  if(xCANSendQueue == NULL)
  {
    xCANSendQueue = xQueueCreate(CANTX_QUEUE_LEN, sizeof(CanTxMsg));
    if(xCANSendQueue == NULL)
    {
      printf("CANSendQueue create failed");
      return;                                    //�������Ͷ���ʧ��
    }
  }
   /* ��������  Can���ն���*/
  if(xCANRcvQueue == NULL)
  {
    xCANRcvQueue = xQueueCreate(CANRX_QUEUE_LEN, sizeof(CanRxMsg));
    if(xCANRcvQueue == NULL)
    {
      printf("CANRcvQueue create failed");
      return;                                    //�������ն���ʧ��
    }
  }
		/* ���������ź���  ���ڴ�ӡ����*/
   xMutex = xSemaphoreCreateMutex();
	
	if(xMutex == NULL)
    {
       printf("xSemaphoreCreateMutex failed\r\n");
			 return;
    }
//	/*�����ź��� ������̫��Link����*/
//	 vSemaphoreCreateBinary( ETH_link_xSemaphore );		
//		if (ETH_link_xSemaphore == NULL)
//   {
//      printf("vSemaphoreCreateBinary failed\r\n");
//			return;
//   }
}

int main(void)
{
	  /*����������֮ǰ�ȹر������ж�*/
    __set_PRIMASK(1); 	
	  /*Ӳ����ʼ�� */
	  BSP_Init(); 
	  /*��ȡ���ò���*/
	  app_flash_LoadSysConfig(); 
    /*STM32F107 ETH��ʼ��*/
	  Eth_config();	
	
	  /*LWIP��ʼ��*/
  	LwIP_Init();	
    /*��������ͨ�Ż��� */
    AppObjCreate(); 
    /*������̫���ں�����*/	
    AppTaskCreate();
	
		vTaskStartScheduler();

		/* We should never get here as control is now taken by the scheduler */
		for( ;; );	
}
/************************************************
�������� �� CANRcv_DateFromISR
��    �� �� CAN�������ݣ���Ӳ���жϵ��ã�����Can��Ϣ�������뵽���ն�����
��    �� �� RxMsg --- ��������(����)
�� �� ֵ �� ��
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
�������� �� setTimer
��    �� �� Э��ջ������������
��    �� �� ��
�� �� ֵ �� ��
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
�������� �� getElapsedTime
��    �� �� �������ŵ�ʱ��
��    �� �� ��
�� �� ֵ �� ���ŵ�ʱ��
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
�������� �� TIMx_DispatchFromISR
��    �� �� ��ʱ����(�Ӷ�ʱ���ж�)
��    �� �� ��
�� �� ֵ �� ��
*************************************************/
void TIMx_DispatchFromISR(void)
{
  last_counter_val = 0;
  elapsed_time = 0;
  TimeDispatch(); //�������� Э��ջ���е�ʱ��
}
/*
�������� �� canSend
��    �� �� CAN���ͣ�����Ϣ�ӵ����Ͷ�����
��    �� �� notused --- δʹ�ò���
            m --------- ��Ϣ����
�� �� ֵ �� 0:ʧ��  1:�ɹ�
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

  /* �ж��Ƿ���ִ���ж� */
  if(0 == __get_CONTROL())
  {
    if(xQueueSendFromISR(xCANSendQueue, &TxMsg, &xHigherPriorityTaskWoken) != pdPASS)
    {                                            //�������ʧ��
      return 0xFF;
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
  }
  else
  {
    if(xQueueSend(xCANSendQueue, &TxMsg, 100) != pdPASS)
    {                                            //�������ʧ��
      return 0xFF;
    }
  }

  return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ		  			  
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
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

	/* �����ź��� */
	  xSemaphoreTake(xMutex, portMAX_DELAY);

    printf("%s", buf_str);

   	xSemaphoreGive(xMutex);
}
/*******************************************************************************
* ������  : EXTI9_5_IRQHandler
* ����    : �������жϺ���
*         ���ж���5�жϷ������������жϷ�������
* ����    : ��
* ����ֵ  : ��
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
   struct tcp_pcb *Main_pcb; //TCP ͨ�ſ�
	 struct tcp_pcb *Bus_pcb; //TCP ͨ�ſ�
   portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line5);
   
		delay_ms(20);  /*Ӳ��ʱ ������*/

    if(0 == GPIO_ReadInputDataBit( GPIOE , GPIO_Pin_5 ))	     /*��������*/
		{			
			bsp_LedOn(LED_RED);                                    /*���¿�ʼ������ʱ*/
			key_timer_Start();    
			sysCfg.parameter.key_time_count = 0;
			bsp_Initkey_Triggertype(EXTI_Trigger_Rising);            /*���������*/
		}
		else if(1 == GPIO_ReadInputDataBit( GPIOE , GPIO_Pin_5 )) /*�����ɿ�*/ 	
		{
			bsp_LedOff(LED_RED);                                       /*�رհ�����ʱ*/
			key_timer_Stop();  
			sysCfg.parameter.key_time_count = 0;
			bsp_Initkey_Triggertype(EXTI_Trigger_Falling);           /*����½���*/
		}
	}	
  if(EXTI_GetITStatus(EXTI_Line6) != RESET)
  {
		
		EXTI_ClearITPendingBit(EXTI_Line6);
		if(((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_MISR)) & PHY_LINK_STATUS) != 0)
      {
        if((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_SR) & 1))
        {
           bsp_LedOff(LED_RED);                                       /*�رհ�����ʱ*/
		       key_timer_Stop(); 
           app_system_mqtt_connect_state_flag_set(SOCK_MAIN,MQTT_CONNECT);
           if(sysCfg.parameter.data_socket == SOCK_BUS)					
           app_system_mqtt_connect_state_flag_set(SOCK_BUS,MQTT_CONNECT);	 					
        }
        else
        {
					  bsp_LedOn(LED_RED);                                    /*���¿�ʼ������ʱ*/
			      key_timer_Start();  						
        }
      }
				 
  }
}
/*������ʱ�� 100ms*/
void TIM3_IRQHandler(void)
{		
  if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
		TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);
		
		sysCfg.parameter.key_time_count++;           
		 
		if(sysCfg.parameter.key_time_count == 50)    /*����5s*/
		{
	    sysCfg.parameter.config_hold_flag = 0xff;  /*�ָ���������*/
			sysCfg.parameter.dhcp = DHCP;			 /*Ĭ��Ϊ����DHCP*/
			if(0 == bsp_WriteCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN))
			{
				delay_ms(100);
				NVIC_SystemReset();		
			}
		}
		if(sysCfg.parameter.key_time_count>150)    /*����5s*/
		{
			app_system_mqtt_connect_state_flag_set(SOCK_MAIN,MQTT_DISCONNECT);			
      app_system_mqtt_connect_state_flag_set(SOCK_BUS,MQTT_DISCONNECT);		
			
		}
	}
}


