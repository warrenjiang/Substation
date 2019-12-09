#include "main.h"

/*
*********************************************************************************************************
 * ��������udp_ntp_recv_rb
 * ����  ��udp �ص���������
 * ����  ��udp_pcb UDP���ƿ� pbuf UDPbuff  ip_addrԶ��ip  portԶ�̶˿�
 * ˵��  : ntp����������48�ֽ����� ������40��ʼ����ʱ��� �����µ�ʱ�������
 * ����  : 
*********************************************************************************************************
 */
static void udp_ntp_recv_rb(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	  uint8_t data_buf[48]={0};
	  uint16_t  startindex = 40;
		memcpy(data_buf,(char *)(p->payload),p->len);
		#if 0
	  for(uint8_t i=0;i<p->len;i++)
	  {
			 App_Printf("%x",data_buf[i]);
		}
		App_Printf("\r\n");	
    #endif		
	  get_seconds_from_ntp_server(data_buf,startindex);	
    pbuf_free(p);
}
/*
*********************************************************************************************************
 * ��������udp_ntp_init
 * ����  ��udp ��ʼ��
 * ����  ����
 * ˵��  : 1.����һ���µ�UDP��  2.��֪��Զ�̷�������ip�Ͷ˿�ʱ ����udp_connect ����ָ����ip�Ͷ˿�����
		3.����ǽ�������ip ������udp_blind ֻ�󶨶˿�
 * ����  : ������UDP��
*********************************************************************************************************
 */
struct udp_pcb * udp_ntp_init(void)
{
    err_t err;
    struct udp_pcb *udppcb;
    struct ip_addr rmtipaddr;   
    udppcb = udp_new();
    if(udppcb){
        IP4_ADDR(&rmtipaddr,120,25,108,11);
        err=udp_connect(udppcb,&rmtipaddr,123);
        udp_recv(udppcb , udp_ntp_recv_rb , NULL);
    }
		return udppcb;
}
/*
*********************************************************************************************************
 * ��������udp_senddata
 * ����  ����������ʱ�������
 * ����  ��udp��
 * ˵��  : 1.����һ���µ�UDP��  2.��֪��Զ�̷�������ip�Ͷ˿�ʱ ����udp_connect ����ָ����ip�Ͷ˿�����
		3.����ǽ�������ip ������udp_blind ֻ�󶨶˿�
 * ����  : ������UDP��
*********************************************************************************************************
 */
void udp_senddata(struct udp_pcb *upcb)
{
    struct pbuf *ptr;
	  uint8_t ntp_message[48]={0};
		ntp_message[0]=0xA3;
    ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ntp_message),PBUF_POOL); 
    if(ptr)
    {
			  memcpy(ptr->payload,ntp_message,sizeof(ntp_message));
        udp_send(upcb,ptr); 
        pbuf_free(ptr);
    } 
} 


