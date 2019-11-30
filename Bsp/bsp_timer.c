/**
  ******************************************************************************
  * @文件名     ： bsp_timer.c
  * @版本       ： V1.0.0
  * @日期       ： 2018年11月14日
  * @摘要       ： TIM底层源文件
  ******************************************************************************/
/*----------------------------------------------------------------------------
  更新日志:
  2018-11-14 V1.0.0:初始版本
  ----------------------------------------------------------------------------*/
/* 包含的头文件 --------------------------------------------------------------*/
#include "bsp_timer.h"

/**
  * @brief  初始化定时器中断
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
  * @brief  初始化定时器 
  * @param  None
  * @retval None
  */
void Key_Timer_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 
	TIM_DeInit(TIM3);                              					//复位TIM3定时器	
	/* TIM3 configuration   按键定时器*/
	TIM_TimeBaseStructure.TIM_Period = 1000-1;        //10 				 
	TIM_TimeBaseStructure.TIM_Prescaler = 6400-1;    	//104		  	  
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  		// 时钟分频 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//计数方向向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* Clear TIM3 update pending flag[清除TIM3溢出中断标志] */
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);

	/* Enable TIM3 Update interrupt [TIM2溢出中断允许]*/
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); 
	/* TIM2计数器清零*/
	TIM3->CNT=0;
	/* TIM3 enable counter [允许TIM3计数]*/
	TIM_Cmd(TIM3, DISABLE);  
	/*Config interrupts*/
		
	Timer_Interrupts_Config();
}

/**
  * @brief  清除定时器计数器寄存器计数值并启动定时器
  * @param  None
  * @retval None
  */
void key_timer_Start(void)
{
	TIM3->CNT=0;//清除计数器寄存器的值，可以减小丢帧的情况
  /* Enable the TIM Counter */
	TIM_Cmd(TIM3, ENABLE); 
}

/**
  * @brief  停止定时器并清除定时器的计数值
  * @param  None
  * @retval None
  */
void key_timer_Stop(void)
{ 
  /* Disable the TIM Counter */
	TIM_Cmd(TIM3, DISABLE);
}
/****************************************** CANOpen定时 ******************************************/
/************************************************
函数名称 ： CANOpen_TIM_Configuration
功    能 ： CANOpen定时配置
参    数 ： 无
返 回 值 ： 无
*************************************************/
void CANOpen_TIM_Configuration(void)
{
  NVIC_InitTypeDef        NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

  /* 时钟配置 */
  RCC_APB1PeriphClockCmd(CANOPEN_TIM_CLK, ENABLE);

  /* NVIC配置 */
  NVIC_InitStructure.NVIC_IRQChannel = CANOPEN_TIM_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CANOPEN_TIM_Priority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* 时基配置 */
  TIM_TimeBaseStructure.TIM_Prescaler = CANOPEN_TIM_PRESCALER_VALUE; //预分频值
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;        //向上计数模式
  TIM_TimeBaseStructure.TIM_Period = CANOPEN_TIM_PERIOD;             //最大计数值(周期)
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;            //时钟分频因子
  TIM_TimeBaseInit(CANOPEN_TIMx, &TIM_TimeBaseStructure);

  /* 使能中断 */
  TIM_ClearFlag(CANOPEN_TIMx, TIM_IT_Update);                        //清除标志
  TIM_ITConfig(CANOPEN_TIMx, TIM_IT_Update, ENABLE);                 //使能 TIMx 更新中断

  /* 初始化 */
  TIM_SetCounter(CANOPEN_TIMx, 0);
  TIM_Cmd(CANOPEN_TIMx, ENABLE);
}

/************************************************
函数名称 ： TIM_Initializes
功    能 ： TIM初始化
参    数 ： 无
返 回 值 ： 无
*************************************************/
void TIM_Initializes(void)
{
  CANOpen_TIM_Configuration();
}



