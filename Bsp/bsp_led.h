/*
*********************************************************************************************************
*
*	ģ������ : LEDָʾ������ģ��
*	�ļ����� : bsp_led.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
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

/* ���ⲿ���õĺ������� */
void bsp_InitLed(void);
void bsp_LedOn(uint8_t _no);
void bsp_LedOff(uint8_t _no);
void bsp_LedToggle(uint8_t _no);
uint8_t bsp_IsLedOn(uint8_t _no);

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
