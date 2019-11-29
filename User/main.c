#include "main.h"



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
		/* ���������ź��� */
   xMutex = xSemaphoreCreateMutex();
	
	if(xMutex == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }
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
