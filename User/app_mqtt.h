#ifndef APP_MQTT_H
#define APP_MQTT_H

#include "stm32f10x.h"
#include "lwip/tcp.h"

err_t transport_sendPacketBuffer(struct tcp_pcb *cpcb,unsigned char* buf, int buflen,unsigned char socket);
void mqtt_disconnect(struct tcp_pcb *cpcb);
int Heartbeat(struct tcp_pcb *cpcb,uint8_t socket);
int mqtt_connect(struct tcp_pcb *cpcb, char *name, char *word,uint8_t socket);
int mqtt_publish(struct tcp_pcb *cpcb,char *pTopic,char *pMessage,int msglen,uint8_t socket);
int mqtt_subscrib(struct tcp_pcb *cpcb,char *pTopic,uint8_t socket,int req_qos);
void app_system_NetPublic(struct tcp_pcb *pcb);
void app_system_TcpsStart(struct tcp_pcb *pcb);
void app_system_Subscrib(struct tcp_pcb *pcb);

#endif

