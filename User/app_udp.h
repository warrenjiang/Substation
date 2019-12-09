#ifndef APP_UDP_H
#define APP_UDP_H



extern struct udp_pcb * udp_ntp_init(void);
extern void udp_senddata(struct udp_pcb *upcb);



#endif



