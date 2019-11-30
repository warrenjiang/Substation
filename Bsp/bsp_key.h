/*
*********************************************************************************************************
*
*	模块名称 : 按键驱动模块
*	文件名称 : bsp_key.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "stm32f10x.h"

#define RCC_APB_KEY 	 RCC_APB2Periph_GPIOE
#define GPIO_PORT_KEY  GPIOE
#define GPIO_PIN_KEY	 GPIO_Pin_5

#define KEY_EXTI_LINE        EXTI_Line5
#define KEY_EXTI_PortSource  GPIO_PortSourceGPIOE
#define KEY_EXTI_PinSource   GPIO_PinSource5

void bsp_Initkey(void);
void bsp_Initkey_Triggertype(EXTITrigger_TypeDef Trigger_TypeDef);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
