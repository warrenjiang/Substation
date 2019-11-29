/*
*********************************************************************************************************
*
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_LED_H
#define __BSP_LED_H

#define LED_BLUE      2
#define LED_GREEN		  3
#define LED_RED			  4

#define RCC_ALL_LED 	(RCC_APB2Periph_GPIOE)

#define GPIO_PORT_LED1  		 	 GPIOE
#define GPIO_PIN_LED1					 GPIO_Pin_0

#define GPIO_PORT_LED_BLUE     GPIOE
#define GPIO_PIN_LED_BLUE	     GPIO_Pin_3   

#define GPIO_PORT_LED_GREEN		 GPIOE
#define GPIO_PIN_LED_GREEN	   GPIO_Pin_1

#define GPIO_PORT_LED_RED  		 GPIOE
#define GPIO_PIN_RED					 GPIO_Pin_2

/* 供外部调用的函数声明 */
void bsp_InitLed(void);
void bsp_LedOn(uint8_t _no);
void bsp_LedOff(uint8_t _no);
void bsp_LedToggle(uint8_t _no);
uint8_t bsp_IsLedOn(uint8_t _no);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
