//tcpip.c
#include "main.h"
#include "stm32_eth.h"
#include "ethernetif.h"
#include "lwip/ip_addr.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "netif/etharp.h"

#include "lwip/tcp.h"
#include "lwip/dhcp.h"
#include "lwip/tcp_impl.h"

#include <stdio.h>
#include <string.h>


/* Private typedef -----------------------------------------------------------*/
#define MAX_DHCP_TRIES        4
#define SELECTED              1
#define NOT_SELECTED		  (!SELECTED)
#define CLIENTMAC6            2


//typedef enum 
//{ 
//  DHCP_START=0,
//  DHCP_WAIT_ADDRESS,
//  DHCP_ADDRESS_ASSIGNED,
//  DHCP_TIMEOUT
//} 
//DHCP_State_TypeDef;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern u32 LocalTime; /* this variable is used to create a time reference incremented by 1ms */

struct netif netif;
uint32_t TCPTimer = 0;
uint32_t ARPTimer = 0;

#ifdef LWIP_DHCP
uint32_t DHCPfineTimer = 0;
uint32_t DHCPcoarseTimer = 0;
#endif

void LwIP_DHCP_Process_Handle();
/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
	
// 	extern err_t
// ethernet_input(struct pbuf *p, struct netif *netif);
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();


#if LWIP_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#else
  IP4_ADDR(&ipaddr,MyIP1,MyIP2,MyIP3,MyIP4);
  IP4_ADDR(&netmask, 255,255,255,0);
  IP4_ADDR(&gw, 192, 168, 0, 1);
#endif

  Set_MAC_Address(sysCfg.parameter.client_mac);

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))
    
   Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /*  Registers the default network interface.*/
  netif_set_default(&netif);


#if LWIP_DHCP
  /*  Creates a new DHCP client for this interface on the first call.
  Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
  the predefined regular intervals after starting the client.
  You can peek in the netif->dhcp struct for the actual DHCP status.*/
  //dhcp_start(&netif);
#endif

  /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&netif);
}



void LwIP_Pkt_Handle(void)
{
  /* Read a received packet from the Ethernet buffers 
                and send it to the lwIP for handling */
  ethernetif_input(&netif);
}


void Eth_Task( void *pvParameters )
{
		for( ;; )
		{
        vTaskDelay(10/portTICK_RATE_MS);
			
				TCPTimer += 10;
				if (TCPTimer>= 250)
				{
					TCPTimer =  0;
					tcp_tmr();
				}
				// ARP periodic process every 5s 
				ARPTimer += 10;
				if (ARPTimer >= 5000)
				{
					ARPTimer =  0;
					etharp_tmr();
				} 
				/*DHCP超时处理*/
        DHCPfineTimer+=10;
        if(DHCPfineTimer>=500)
				{
					DHCPfineTimer=0;
					dhcp_fine_tmr();
				}	
				/*DHCP租用情况到期处理*/
        DHCPcoarseTimer+=10;
				if(DHCPcoarseTimer>60000)
				{
				 DHCPcoarseTimer=0;
				 dhcp_coarse_tmr();
				}
		}
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
