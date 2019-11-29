/**
  ******************************************************************************
  * @�ļ���     �� bsp.h
  * @����       �� strongerHuang
  * @�汾       �� V1.0.0
  * @����       �� 2018��11��14��
  * @ժҪ       �� �ײ�����ͷ�ļ�
  ******************************************************************************/

/* �����ֹ�ݹ���� ----------------------------------------------------------*/
#ifndef _BSP_H
#define _BSP_H

/* ������ͷ�ļ� --------------------------------------------------------------*/
#include "stm32f10x.h"


/* �궨�� --------------------------------------------------------------------*/
#define LED_GPIO_CLK              RCC_APB2Periph_GPIOE
#define LED_PIN                   GPIO_Pin_1
#define LED_GPIO_PORT             GPIOE

/* LED���� */
#define LED_ON()                  GPIO_SetBits(LED_GPIO_PORT, LED_PIN)
#define LED_OFF()                 GPIO_ResetBits(LED_GPIO_PORT, LED_PIN)
#define LED_TOGGLE()              (LED_GPIO_PORT->ODR ^= LED_PIN)


/* �������� ------------------------------------------------------------------*/
void BSP_Init(void);
/*����ʱ*/
void delay_ms(u16 time);

#endif /* _BSP_H */

/**** Copyright (C)2018 strongerHuang. All Rights Reserved **** END OF FILE ****/
