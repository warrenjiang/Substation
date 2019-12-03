/*******************************************************************************
** �ļ���: 		mian.c
** �汾��  		1.0
** ��������: 	RealView MDK-ARM 4.14
** ����: 		wuguoyana
** ��������: 	2011-04-28
** ����:		USART��ʼ����RCC���ã�Ȼ���common.c��ִ�����˵�
** ����ļ�:	stm32f10x.h
** �޸���־��	2011-04-29   �����ĵ�
*******************************************************************************/
/* ����ͷ�ļ� *****************************************************************/
#include "common.h"
#include "stdio.h"
/* ���� ----------------------------------------------------------------------*/
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;

/* �������� ------------------------------------------------------------------*/
static void IAP_Init(void);
void GPIO_Configuration(void);
void USART_Configuration(void);
/* �������� ------------------------------------------------------------------*/

/*******************************************************************************
  * @��������	main
  * @����˵��   ������
  * @�������   ��
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
int main(void)
{
	   uint32_t fw_len=0xffffffff;
		 uint32_t i=0;
	   uint8_t status;
     uint32_t nPage;
	   uint32_t nErasedPage;
    //Flash ����
    FLASH_Unlock();
    IAP_Init();
    if(!(bsp_ReadCpuFlash(0x0803F000,(uint8_t *)&fw_len,4)))
			;
		printf("\r\nEnter Bootloader Sucessful\r\n");
		if (fw_len!=0xffffffff)
			{	
				  SerialPutString("\r\nReady to update\r\n");
					nPage=FLASH_PagesMask(fw_len);
					// Clear All pending flags
					FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
					//erase required pages
					for(nErasedPage=0; nErasedPage<nPage; nErasedPage++)
					{
						status=FLASH_ErasePage(ApplicationAddress + 0x800*nErasedPage);
					}
					//copy app from backup address to real address
					for(i=0; i<fw_len; i+=2)
					{
						FLASH_ProgramHalfWord(ApplicationAddress+i, *(uint16_t*)(AppBackupAddress+i));
					}
					FLASH_Lock();			
				  if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
          {
            printf("enter user Program\r\n");
            //��ת���û�����
            JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
            Jump_To_Application = (pFunction) JumpAddress;
            //��ʼ���û�����Ķ�ջָ��
            __set_MSP(*(__IO uint32_t*) ApplicationAddress);
            Jump_To_Application(); //ִ���û�����
        }
			}
   
    else
    {
        //�ж��û��Ƿ��Ѿ����س�����Ϊ��������´˵�ַ��ջ��ַ��
        //��û����һ��Ļ�����ʹû�����س���Ҳ�����������ܷɡ�
        if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
        {
            SerialPutString("Enter user Program\r\n");
            //��ת���û�����
            JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
            Jump_To_Application = (pFunction) JumpAddress;

            //��ʼ���û�����Ķ�ջָ��
            __set_MSP(*(__IO uint32_t*) ApplicationAddress);
            Jump_To_Application(); //ִ���û�����
        }
 
    }
	

    while (1)
    {
    }
}

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
///�ض���c�⺯��printf��USART1
int fputc(int ch, FILE *f)
{
			
	  USART1->DR = (u8) ch;  
		while((USART1->SR&0X40)==0);	
	
		return (ch);
}
#endif

/*******************************************************************************
  * @��������	GPIO_Configuration
  * @����˵��   ����ʹ��USART1�����IO�ܽ�
  * @�������   ��
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    // ���� USART1 Tx (PA.09) ��Ϊ�������Ų��������ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //���� USART1 Tx (PA.10) ��Ϊ�������Ų��Ǹ�������ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************************
  * @��������	IAP_Init
  * @����˵��   ����ʹ��IAP
  * @�������   ��
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
void IAP_Init(void)
{
    USART_InitTypeDef USART_InitStructure;

    /* USART1 ���� ------------------------------------------------------------
         USART1 ��������:
          - ������      = 115200 baud
          - �ֳ�        = 8 Bits
          - һ��ֹͣλ
          - ��У��
          - ��Ӳ��������
          - ���ܺͷ���ʹ��
    --------------------------------------------------------------------------*/
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    GPIO_Configuration();
    USART_Init(USART1, &USART_InitStructure);
    // ʹ�� USART1
    USART_Cmd(USART1, ENABLE);
}



/*******************************�ļ�����***************************************/
