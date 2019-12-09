#ifndef __APP_TCP_CLIENT_H
#define __APP_TCP_CLIENT_H

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/snmp.h"
#include "lwip/tcp.h"
#include "lwip/tcp_impl.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"
#include <stdio.h>
#include <string.h>
typedef enum 
{ 
  DHCP_START=0,
  DHCP_WAIT_ADDRESS,
  DHCP_ADDRESS_ASSIGNED,
  DHCP_TIMEOUT
} 
DHCP_State_TypeDef;

#define STATIC 1
#define DHCP   2
#define MAX_DHCP_TRIES 4 
#define HEART_OUTTIME_TIMES  150 
#define MQTT_RECONNECT_TIME  200

void TCP_Client_Init(u16_t local_port,u16_t remote_port,unsigned char a,unsigned char b,unsigned char c,unsigned char d);
err_t TCP_Client_Send_Data(struct tcp_pcb *cpcb,unsigned char *buff,unsigned int length);
err_t  TCP_Client_Recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err);

#endif

