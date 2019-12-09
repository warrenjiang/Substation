#include "main.h"

/*
*********************************************************************************************************
 * 函数名：udp_ntp_recv_rb
 * 描述  ：udp 回调函数处理
 * 输入  ：udp_pcb UDP控制块 pbuf UDPbuff  ip_addr远程ip  port远程端口
 * 说明  : ntp服务器返回48字节数据 从索引40开始解析时间戳 最后更新到时间戳当中
 * 返回  : 
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
 * 函数名：udp_ntp_init
 * 描述  ：udp 初始化
 * 输入  ：无
 * 说明  : 1.创建一个新的UDP块  2.在知道远程服务器的ip和端口时 调用udp_connect 接收指定的ip和端口数据
		3.如何是接收任意ip 调用用udp_blind 只绑定端口
 * 返回  : 创建的UDP块
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
 * 函数名：udp_senddata
 * 描述  ：发送请求时间戳数据
 * 输入  ：udp块
 * 说明  : 1.创建一个新的UDP块  2.在知道远程服务器的ip和端口时 调用udp_connect 接收指定的ip和端口数据
		3.如何是接收任意ip 调用用udp_blind 只绑定端口
 * 返回  : 创建的UDP块
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


