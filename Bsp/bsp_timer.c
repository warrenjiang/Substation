/**
  ******************************************************************************
  * @�ļ���     �� bsp_timer.c
  * @�汾       �� V1.0.0
  * @����       �� 2018��11��14��
  * @ժҪ       �� TIM�ײ�Դ�ļ�
  ******************************************************************************/
/*----------------------------------------------------------------------------
  ������־:
  2018-11-14 V1.0.0:��ʼ�汾
  ----------------------------------------------------------------------------*/
/* ������ͷ�ļ� --------------------------------------------------------------*/
#include "bsp_timer.h"

/**
  * @brief  ��ʼ����ʱ���ж�
  * @param  None
  * @retval None
  */
void Timer_Interrupts_Config(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  ��ʼ����ʱ�� 
  * @param  None
  * @retval None
  */
void Key_Timer_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 
	TIM_DeInit(TIM3);                              					//��λTIM3��ʱ��	
	/* TIM3 configuration   ������ʱ��*/
	TIM_TimeBaseStructure.TIM_Period = 1000-1;        //10 				 
	TIM_TimeBaseStructure.TIM_Prescaler = 6400-1;    	//104		  	  
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  		// ʱ�ӷ�Ƶ 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//�����������ϼ���
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* Clear TIM3 update pending flag[���TIM3����жϱ�־] */
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);

	/* Enable TIM3 Update interrupt [TIM2����ж�����]*/
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); 
	/* TIM2����������*/
	TIM3->CNT=0;
	/* TIM3 enable counter [����TIM3����]*/
	TIM_Cmd(TIM3, DISABLE);  
	/*Config interrupts*/
		
	Timer_Interrupts_Config();
}

/**
  * @brief  �����ʱ���������Ĵ�������ֵ��������ʱ��
  * @param  None
  * @retval None
  */
void key_timer_Start(void)
{
	TIM3->CNT=0;//����������Ĵ�����ֵ�����Լ�С��֡�����
  /* Enable the TIM Counter */
	TIM_Cmd(TIM3, ENABLE); 
}

/**
  * @brief  ֹͣ��ʱ���������ʱ���ļ���ֵ
  * @param  None
  * @retval None
  */
void key_timer_Stop(void)
{ 
  /* Disable the TIM Counter */
	TIM_Cmd(TIM3, DISABLE);
}
/****************************************** CANOpen��ʱ ******************************************/
/************************************************
�������� �� CANOpen_TIM_Configuration
��    �� �� CANOpen��ʱ����
��    �� �� ��
�� �� ֵ �� ��
*************************************************/
void CANOpen_TIM_Configuration(void)
{
  NVIC_InitTypeDef        NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

  /* ʱ������ */
  RCC_APB1PeriphClockCmd(CANOPEN_TIM_CLK, ENABLE);

  /* NVIC���� */
  NVIC_InitStructure.NVIC_IRQChannel = CANOPEN_TIM_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CANOPEN_TIM_Priority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* ʱ������ */
  TIM_TimeBaseStructure.TIM_Prescaler = CANOPEN_TIM_PRESCALER_VALUE; //Ԥ��Ƶֵ
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;        //���ϼ���ģʽ
  TIM_TimeBaseStructure.TIM_Period = CANOPEN_TIM_PERIOD;             //������ֵ(����)
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;            //ʱ�ӷ�Ƶ����
  TIM_TimeBaseInit(CANOPEN_TIMx, &TIM_TimeBaseStructure);

  /* ʹ���ж� */
  TIM_ClearFlag(CANOPEN_TIMx, TIM_IT_Update);                        //�����־
  TIM_ITConfig(CANOPEN_TIMx, TIM_IT_Update, ENABLE);                 //ʹ�� TIMx �����ж�

  /* ��ʼ�� */
  TIM_SetCounter(CANOPEN_TIMx, 0);
  TIM_Cmd(CANOPEN_TIMx, ENABLE);
}

/************************************************
�������� �� TIM_Initializes
��    �� �� TIM��ʼ��
��    �� �� ��
�� �� ֵ �� ��
*************************************************/
void TIM_Initializes(void)
{
  CANOpen_TIM_Configuration();
}



