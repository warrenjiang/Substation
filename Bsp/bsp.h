/**
  ******************************************************************************
  * @文件名     ： bsp.h
  * @作者       ： strongerHuang
  * @版本       ： V1.0.0
  * @日期       ： 2018年11月14日
  * @摘要       ： 底层驱动头文件
  ******************************************************************************/

/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _BSP_H
#define _BSP_H

/* 包含的头文件 --------------------------------------------------------------*/
#include "stm32f10x.h"
#include "bsp_led.h"
#include "bsp_iwdg.h"
#include "bsp_can.h"
#include "bsp_cpu_flash.h"
#include "bsp_mqtt_ota.h"
#include "bsp_timer.h"
#include "bsp_usart.h"
#include "bsp_key.h"

/* 宏定义 --------------------------------------------------------------------*/
#define LED_GPIO_CLK              RCC_APB2Periph_GPIOE
#define LED_PIN                   GPIO_Pin_1
#define LED_GPIO_PORT             GPIOE

/* LED开关 */
#define LED_ON()                  GPIO_SetBits(LED_GPIO_PORT, LED_PIN)
#define LED_OFF()                 GPIO_ResetBits(LED_GPIO_PORT, LED_PIN)
#define LED_TOGGLE()              (LED_GPIO_PORT->ODR ^= LED_PIN)


/* 函数申明 ------------------------------------------------------------------*/
void BSP_Init(void);
/*软延时*/
void delay_ms(u16 time);

#endif /* _BSP_H */

/**** Copyright (C)2018 strongerHuang. All Rights Reserved **** END OF FILE ****/
