/*
*********************************************************************************************************
*
*	ģ������ : LEDָʾ������ģ��
*	�ļ����� : bsp_led.c
*	��    �� : V1.0
*	˵    �� : ����LEDָʾ��
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-02-01 armfly  ��ʽ����
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp_led.h"

/*
	�ó��������ڰ�����STM32-V4 ������

	�����������Ӳ�������޸�GPIO����
	
	����û���LEDָʾ�Ƹ���С��4�������Խ������LEDȫ������Ϊ�͵�1��LEDһ��������Ӱ�������
*/


/* �����ڶ�Ӧ��RCCʱ�� */

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitLed
*	����˵��: ����LEDָʾ����ص�GPIO,  �ú����� bsp_Init() ���á�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ��GPIOʱ�� */
	RCC_APB2PeriphClockCmd(RCC_ALL_LED, ENABLE);
	/*
		�������е�LEDָʾ��GPIOΪ�������ģʽ
		���ڽ�GPIO����Ϊ���ʱ��GPIO����Ĵ�����ֵȱʡ��0����˻�����LED����.
		�����Ҳ�ϣ���ģ�����ڸı�GPIOΪ���ǰ���ȹر�LEDָʾ��
	*/
	bsp_LedOff(LED_BLUE);     /*��ɫ*/
	bsp_LedOff(LED_GREEN);     /*��ɫ*/
	bsp_LedOff(LED_RED);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	/* �������ģʽ */
	
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED1;
	GPIO_Init(GPIO_PORT_LED1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED_BLUE;
	GPIO_Init(GPIO_PORT_LED_BLUE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED_GREEN;
	GPIO_Init(GPIO_PORT_LED_GREEN, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_RED;
	GPIO_Init(GPIO_PORT_LED_RED, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedOn
*	����˵��: ����ָ����LEDָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_LedOn(uint8_t _no)
{
	_no--;

	if (_no == 0)
	{
		GPIO_PORT_LED1->BRR = GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED_BLUE->BRR = GPIO_PIN_LED_BLUE;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED_GREEN->BRR = GPIO_PIN_LED_GREEN;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED_RED->BRR = GPIO_PIN_RED;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedOff
*	����˵��: Ϩ��ָ����LEDָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_LedOff(uint8_t _no)
{
	_no--;

	if (_no == 0)
	{
		GPIO_PORT_LED1->BSRR = GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED_BLUE->BSRR = GPIO_PIN_LED_BLUE;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED_GREEN->BSRR = GPIO_PIN_LED_GREEN;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED_RED->BSRR = GPIO_PIN_RED;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_LedToggle
*	����˵��: ��תָ����LEDָʾ�ơ�
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
void bsp_LedToggle(uint8_t _no)
{
	if (_no == 1)
	{
		GPIO_PORT_LED1->ODR ^= GPIO_PIN_LED1;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED_BLUE->ODR ^= GPIO_PIN_LED_BLUE;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED_GREEN->ODR ^= GPIO_PIN_LED_GREEN;
	}
	else if (_no == 4)
	{
		GPIO_PORT_LED_RED->ODR ^= GPIO_PIN_RED;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_IsLedOn
*	����˵��: �ж�LEDָʾ���Ƿ��Ѿ�������
*	��    ��:  _no : ָʾ����ţ���Χ 1 - 4
*	�� �� ֵ: 1��ʾ�Ѿ�������0��ʾδ����
*********************************************************************************************************
*/
uint8_t bsp_IsLedOn(uint8_t _no)
{
	if (_no == 1)
	{
		if ((GPIO_PORT_LED1->ODR & GPIO_PIN_LED1) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 2)
	{
		if ((GPIO_PORT_LED_BLUE->ODR & GPIO_PIN_LED_BLUE) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 3)
	{
		if ((GPIO_PORT_LED_GREEN->ODR & GPIO_PIN_LED_GREEN) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 4)
	{
		if ((GPIO_PORT_LED_RED->ODR & GPIO_PIN_RED) == 0)
		{
			return 1;
		}
		return 0;
	}

	return 0;
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
