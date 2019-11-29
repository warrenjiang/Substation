
#include "stm32_eth.h"

#include <stdio.h>
#include <string.h>

#define PHY_ADDRESS   0x01

void Ddelay(u32	i)
{
    for(;i>0;i--)
	{
		i++;i--;	   
	}
}
 void Ddelay_ms(u32	i)
{
    for(;i>0;i--)
	{
    	Ddelay(10000);
	}	
}
void Ddelay_s(u32	i)
{
    for(;i>0;i--)
	{
    	Ddelay_ms(1000);
	}	

}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	/* Enable the Ethernet global Interrupt 使能以太网中断 */
	NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);    
}


void ETH_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
/* ETHERNET pins configuration */
/* AF Output Push Pull: 
- ETH_MII_MDIO / ETH_RMII_MDIO: PA2 
- ETH_MII_MDC / ETH_RMII_MDC: PC1
- ETH_MII_TXD2: PC2 
- ETH_MII_TX_EN / ETH_RMII_TX_EN: PB11  
- ETH_MII_TXD0 / ETH_RMII_TXD0: PB12 
- ETH_MII_TXD1 / ETH_RMII_TXD1: PB13  
- ETH_MII_PPS_OUT / ETH_RMII_PPS_OUT: PB5
- ETH_MII_TXD3: PB8 
	RESET PA3 */ 
	
	/* Configure PA2 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PC1, PC2 and PC3 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure PB5, PB8, PB11, PB12 and PB13 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
  
/**************************************************************/
/*               For Remapped Ethernet pins                   */
/*************************************************************/
/* Input (Reset Value):  
- ETH_MII_CRS CRS: PA0 
- ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1   
- ETH_MII_COL: PA3  
- ETH_MII_RX_DV / ETH_RMII_CRS_DV: PD8
- ETH_MII_TX_CLK: PC3
- ETH_MII_RXD0 / ETH_RMII_RXD0: PD9 
- ETH_MII_RXD1 / ETH_RMII_RXD1: PD10 
- ETH_MII_RXD2: PD11
- ETH_MII_RXD3: PD12
- ETH_MII_RX_ER: PB10 */
 
	/* Configure PA0, PA1 and PA3 as input */
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PC5 as input */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 

	/* Configure PD8, PD9, PD10, PD11 and PD12 as input */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);  

    /* MCO pin configuration------------------------------------------------- */
	/* Configure MCO (PA8) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	/* Configure PC. as Output push-pull RES*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
}


void Eth_config(void)
{
	ETH_InitTypeDef ETH_InitStructure;
// RCC_ClocksTypeDef RCC_Clocks;
	/* Enable GPIOs clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |	RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
                         RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE| RCC_APB2Periph_AFIO, ENABLE);
  
	/* Enable ETHERNET clock  */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
                        RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);
	NVIC_Configuration(); 
	ETH_GPIO_Config();
	/* ETHERNET pins remapp in STM3210C-EVAL board: RX_DV and RxD[3:0] */
	GPIO_PinRemapConfig(GPIO_Remap_ETH, ENABLE); 
    
	/* MII/RMII Media interface selection */
	//  GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_MII);
	/* Get HSE clock = 25MHz on PA8 pin(MCO) */
	//  RCC_MCOConfig(RCC_MCO_HSE); 
	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);

	/* Set PLL3 clock output to 50MHz (25MHz /5 *10 =50MHz) */
	RCC_PLL3Config(RCC_PLL3Mul_10);
	/* Enable PLL3 */
	RCC_PLL3Cmd(ENABLE);
	/* Wait till PLL3 is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET)
	{}

	/* Get PLL3 clock on PA8 pin (MCO) */
	RCC_MCOConfig(RCC_MCO_PLL3CLK);
////////////////////////////////	  
	GPIO_ResetBits(GPIOA,GPIO_Pin_3);   //reset md9161
	Ddelay_ms(100);
	GPIO_SetBits(GPIOA,GPIO_Pin_3);
	Ddelay_ms(100);

	/* Reset ETHERNET on AHB Bus */
	ETH_DeInit();
  
	/* Software reset */ 
	ETH_SoftwareReset();
	/* Wait for software reset */
	while((ETH_GetSoftwareResetStatus()==SET));
	Ddelay_ms(500);
	/* ETHERNET Configuration ------------------------------------------------------*/
	/* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
	ETH_StructInit(&ETH_InitStructure);
  
/* Fill ETH_InitStructure parametrs */
/*------------------------   MAC   -----------------------------------*/
//  	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable  ;  
 	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
 	ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
	ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
	
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif
/*------------------------   DMA   -----------------------------------*/  
  
  /* When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
  the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
  if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
  
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; 
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;         
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;     
 
	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;       
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;   
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;                                                          
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;      
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;                
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;          
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;                                                                 
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	/* Configure ETHERNET */
	ETH_Init(&ETH_InitStructure, PHY_ADDRESS);
	/* Enable the Ethernet Rx Interrupt */
	ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);

}



