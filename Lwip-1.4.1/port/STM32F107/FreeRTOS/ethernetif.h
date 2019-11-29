#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "main.h"
#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);


void Set_MAC_Address(uint8_t* macadd);
err_t  ethernetif_input(struct netif *netif);

#define MAC_ADDR0   02
#define MAC_ADDR1   00
#define MAC_ADDR2   00
#define MAC_ADDR3   00
#define MAC_ADDR4   00
#define MAC_ADDR5   00


#endif 
