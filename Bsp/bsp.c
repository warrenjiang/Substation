/**
  ******************************************************************************
  * @文件名     ： bsp.c
  * @版本       ： V1.0.0
  * @日期       ： 2018年11月14日
  * @摘要       ： 底层驱动源文件
  ******************************************************************************/
/*----------------------------------------------------------------------------
  更新日志:
  2018-11-14 V1.0.0:初始版本
  ----------------------------------------------------------------------------*/
/* 包含的头文件 --------------------------------------------------------------*/
#include "bsp.h"

/************************************************
函数名称 ： LED_Initializes
功    能 ： LED初始化
参    数 ： 无
返 回 值 ： 无
*************************************************/
void LED_Initializes(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 使能时钟 */
  RCC_APB2PeriphClockCmd(LED_GPIO_CLK, ENABLE);

  /* 引脚配置 */
  GPIO_InitStructure.GPIO_Pin   = LED_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);

  LED_OFF();
}
/*软延时*/
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
函数名称 ： BSP_Init
功    能 ： 底层驱动初始化
参    数 ： 无
返 回 值 ： 无
*************************************************/
void BSP_Init(void)
{
	uint32_t ota_flag=0xFFFFFFFF;
	RCC_ClocksTypeDef RCC_Clocks;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  bsp_InitLed();	                               /*初始LED指示灯端口 */
	bsp_Initkey();
	Key_Timer_Config();
	//bsp_InitIwdg(0x0FFF);        /*26.208s看门狗异常复位时间*/
  CAN_Initializes();                             //CAN底层初始化
  TIM_Initializes();                             //TIM底层初始化
  USART_Initializes();                           //USART底层初始化                                  
	RCC_GetClocksFreq(&RCC_Clocks);
	printf("\r\nSYSCLK_Frequency:%dHz\r\n",RCC_Clocks.SYSCLK_Frequency);	
  #if 1
		/*清除升级标志位*/
	if(1 == bsp_WriteCpuFlash(FLAGADRR,(uint8_t *)&ota_flag,4))
	{
		#if APP_DEBUG
		printf("\r\nclear flag Fail\r\n");
		#endif
	}
	#endif

}

