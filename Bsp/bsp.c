/**
  ******************************************************************************
  * @�ļ���     �� bsp.c
  * @�汾       �� V1.0.0
  * @����       �� 2018��11��14��
  * @ժҪ       �� �ײ�����Դ�ļ�
  ******************************************************************************/
/*----------------------------------------------------------------------------
  ������־:
  2018-11-14 V1.0.0:��ʼ�汾
  ----------------------------------------------------------------------------*/
/* ������ͷ�ļ� --------------------------------------------------------------*/
#include "bsp.h"

/************************************************
�������� �� LED_Initializes
��    �� �� LED��ʼ��
��    �� �� ��
�� �� ֵ �� ��
*************************************************/
void LED_Initializes(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* ʹ��ʱ�� */
  RCC_APB2PeriphClockCmd(LED_GPIO_CLK, ENABLE);

  /* �������� */
  GPIO_InitStructure.GPIO_Pin   = LED_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);

  LED_OFF();
}
/*����ʱ*/
void delay_ms(u16 time)
{    
	 uint16_t i=0;  
	 while(time--)
	 {
			i=12000;  
			while(i--);    
	 }
}
/************************************************
�������� �� BSP_Init
��    �� �� �ײ�������ʼ��
��    �� �� ��
�� �� ֵ �� ��
*************************************************/
void BSP_Init(void)
{
	uint32_t ota_flag=0xFFFFFFFF;
	RCC_ClocksTypeDef RCC_Clocks;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  bsp_InitLed();	                               /*��ʼLEDָʾ�ƶ˿� */
	bsp_Initkey();
	Key_Timer_Config();
	//bsp_InitIwdg(0x0FFF);        /*26.208s���Ź��쳣��λʱ��*/
  CAN_Initializes();                             //CAN�ײ��ʼ��
  TIM_Initializes();                             //TIM�ײ��ʼ��
  USART_Initializes();                           //USART�ײ��ʼ��                                  
	RCC_GetClocksFreq(&RCC_Clocks);
	printf("\r\nSYSCLK_Frequency:%dHz\r\n",RCC_Clocks.SYSCLK_Frequency);	
  #if 1
		/*���������־λ*/
	if(1 == bsp_WriteCpuFlash(FLAGADRR,(uint8_t *)&ota_flag,4))
	{
		#if APP_DEBUG
		printf("\r\nclear flag Fail\r\n");
		#endif
	}
	#endif

}

