/***********************************************************************
文件名称：TCP_CLIENT.C
功    能：完成TCP的客户端的数据收发
编写时间：2013.4.25
编 写 人：
注    意：
***********************************************************************/
#include "main.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"
extern  struct netif netif;
struct tcp_pcb *main_pcb; //TCP 通信块
struct tcp_pcb *bus_pcb; //TCP 通信块
/**
  * @brief  通过TCP方式发送数据到TCP服务器
  * @param  buf 数据首地址
  * @param  buflen 数据长度
  * @retval 小于0表示发送失败
  */
err_t transport_sendPacketBuffer(struct tcp_pcb *cpcb,unsigned char* buf, int buflen,unsigned char socket)
{
  err_t err;
	err = tcp_write(cpcb,buf,buflen,TCP_WRITE_FLAG_COPY);	//将数据写入队列中，不会立即发送
	//err=tcp_output(cpcb);
	return err;					
}
void mqtt_disconnect(struct tcp_pcb *cpcb)
{
	uint8_t len;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	
	len = MQTTSerialize_disconnect(buf,buflen);
	transport_sendPacketBuffer(cpcb, buf, len,0);		//返回发送的长度
	//if(sysCfg.parameter.data_socket == SOCK_BUS)
	//{
		//transport_sendPacketBuffer(SOCK_BUS, buf, len);
  //	}
}	

/**
  * @brief  心跳
  * @param  无
  * @param  无
  * @retval 0 ：失败  1:成功
  */
int Heartbeat(struct tcp_pcb *cpcb,uint8_t socket)
{
	uint8_t len,rc;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	len = MQTTSerialize_pingreq(buf,buflen);
	rc = transport_sendPacketBuffer(cpcb,buf, len,socket);//返回发送的长度
	if(rc != len)
	{
//		bsp_LedOff(2);     /*蓝色*/
//		bsp_LedOff(3);     /*绿色*/
//		bsp_LedOff(4);     /*红色*/
		#if APP_DEBUG
		printf("%d:Heartbeat fail\n\r",socket);
		#endif
		return 0;	
	}
	return 1;
}	
/**
  * @brief  连接MQTT服务器
  * @param  name:用户名 
  * @param  word：密码
  * @retval 无
  */
int mqtt_connect(struct tcp_pcb *cpcb, char *name, char *word,uint8_t socket)
{
	int32_t len,rc;
	unsigned char buf[512];
	int buflen = sizeof(buf);
	  	
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

	data.clientID.cstring = (char *)sysCfg.parameter.client_id;
  data.keepAliveInterval = 120;
  data.cleansession = 1;
	data.username.cstring = name;
	data.password.cstring = word;


	len = MQTTSerialize_connect(buf, buflen, &data); 
	rc = transport_sendPacketBuffer(cpcb,buf,len,socket);	//返回发送的长度
	
  if(rc !=ERR_OK ){
		#if APP_DEBUG
    printf("Connect:connect transport_sendPacketBuffer error code:%d\n\r",rc);
		#endif
		return -1;	
  }
	return rc;
}


/**
  * @brief  向代理（服务器）发送一个消息
  * @param  pTopic 消息主题
  * @param  pMessage 消息内容
  * @retval  0表示发送失败 1:发送成功
  */
int mqtt_publish(struct tcp_pcb *cpcb,char *pTopic,char *pMessage,int msglen,uint8_t socket)
{
  int32_t len,rc;
  unsigned char buf[256];
  MQTTString topicString = MQTTString_initializer;
	
	#if 0  /*modify by pei 2018-1-29*/
  int msglen = strlen(pMessage);
	#endif
	
  int buflen = sizeof(buf);
	/*publishh*/
  topicString.cstring = pTopic;
  len= MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)pMessage, msglen); /* 2 */
	rc = transport_sendPacketBuffer(cpcb,buf,len,socket);
	if (rc == ERR_OK)
	{
		#if 0
		printf("Published %x OK\r\n",buf[len-6]);
		#endif
		return 1;
	}
	else
	{
		printf("Publish failed code:%d\r\n",rc);
		return 0;
	}
	
//如果需要发送完消息，即关闭端口，需要一定延时 QOS=1，发布消息时，服务器端会返回PUBACK
//	 if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBACK) 	
//	{
//	
//	}
	
}
/**
  * @brief  向服务器订阅一个消息，该函数会因为TCP接收数据函数而阻塞
  * @param  pTopic 消息主题，传入
  * @retval 0表示订阅消息失败，rc 发送的数据长度
  */
int mqtt_subscrib(struct tcp_pcb *cpcb,char *pTopic,uint8_t socket,int req_qos)
{
	MQTTString topicString = MQTTString_initializer;
	unsigned char buf[128];
	int buflen = sizeof(buf);
	int rc = 0;
	int msgid = 1;
	int len = 0;
	/* subscribe */
	topicString.cstring = pTopic;
	len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);
	rc = transport_sendPacketBuffer(cpcb,buf, len,socket);
  if(rc != ERR_OK)
	{
		#if APP_DEBUG
    printf("Subscrib:connect transport_sendPacketBuffer error\n\r");
		printf("rc:%d\r\n",rc);
		#endif
		
		return -1;
  }
	
  return rc;
}
/*
*********************************************************************************************************
 * 函数名：app_system_NetPublic
 * 描述  ：连接参数发布
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
 */
void app_system_NetPublic(struct tcp_pcb *pcb)
{
	uint8_t mac[6];
	uint8_t cip[4];
	uint8_t sip[4];
	uint8_t nrf[6];
  uint8_t startstatus = 0;
	
	char dev[30] = {0};
	char ble[30] = {0};
	char link[30] = {0};
	char netparm[500] = {0};
	char *state[6] = {"unrst","porrst","sftrst","iwdrst","wwdrst","lpwrst"};
	uint32_t CSR = RCC->CSR;
	uint32_t reset_state[5] = {PORRSTF_MASK,SFTRST_MASK,IWDGRST_MASK,WWDGRST_MASK,LPWRRSTF_MASK};
		
  /*startstatus: 1：电源复位 2：软件复位 3：独立看门狗复位 4：窗口看门狗复位 5：低功耗复位*/	
	for(uint8_t i = 0; i < 5; i++)
	{
		if(CSR & reset_state[i])
		{
			startstatus = i + 1;
			break;
		}
	}
  
	#if APP_DEBUG
	printf("startstatus:%s\r\n",state[startstatus]);
	#endif
	
	RCC_ClearFlag();
	
	memcpy(sip,sysCfg.parameter.server_ip,4);  
	memcpy(cip,sysCfg.parameter.ip,4);  
	memcpy(mac,sysCfg.parameter.client_mac,6); 
	memcpy(nrf,&sysCfg.parameter.nrfstation[2],6);
	
	sprintf(dev,"0000%02x%02x%02x%02x%02x%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	sprintf(ble,"0000%02x%02x%02x%02x%02x%02x",nrf[0],nrf[1],nrf[2],nrf[3],nrf[4],nrf[5]);
	sprintf(link,"%d.%d.%d.%d:%d",sip[0],sip[1],sip[2],sip[3],sysCfg.parameter.server_port);
	
	sprintf(netparm,"{\"gid\":\"%s\",\"bid\":\"%s\",\"cip\":\"%d.%d.%d.%d\",\"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"connect\":\"%s\",\"sver\":\"%s\",\"hver\":\"%s\",\"sta\":\"%s\"}",
	dev,ble,cip[0],cip[1],cip[2],cip[3],mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],link,SOFTWARE_VERSION,HARDWARE_VERSION,state[startstatus]);		
	mqtt_publish(pcb,SOFTVERSION_TOPIC,netparm,strlen(netparm),0);//上报版本号-
}

/*订阅主题*/
void app_system_TcpsStart(struct tcp_pcb *pcb)
{
	uint8_t mac[6];
	char dev[30] = {0};
	
	memcpy(mac,sysCfg.parameter.client_mac,6); 
	
	/*主控升级主题*/
	if(0== mqtt_subscrib(pcb,gateupdata_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",gateupdata_topic);
 		#endif  
	}	
				
	/*配置设备信息主题*/
	if(0 == mqtt_subscrib(pcb,devcfgpara_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",devcfgpara_topic);
 		#endif  
	}	

	/*网关重启主题*/
	if(0 == mqtt_subscrib(pcb,gatereset_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",gatereset_topic);
 		#endif  
	}	

	/*服务器检测设备在线主题*/
	if(0 == mqtt_subscrib(pcb,rrpc_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",rrpc_request_topic);
 		#endif  
	}	
	
	/*服务器返回给网关收到心跳信息主题*/
	if(0 == mqtt_subscrib(pcb,response_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",response_request_topic);
 		#endif  
	}		
	
	sprintf(dev,"system start:0000%02x%02x%02x%02x%02x%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	mqtt_publish( pcb , "ssw" , dev , strlen(dev),0);
	
}
/*
*********************************************************************************************************
 * 函数名：app_system_Subscrib
 * 描述  ：订阅第三方主题
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
*/
void app_system_Subscrib(struct tcp_pcb *pcb)
{
	/*服务器检测设备在线主题*/
	if(0 == mqtt_subscrib(pcb,rrpc_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",rrpc_request_topic);
 		#endif  
	}	
	
	/*服务器返回给网关收到心跳信息主题*/
	if(0 == mqtt_subscrib(pcb,response_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",response_request_topic);
 		#endif  
	}		
}
/***********************************************************************
函数名称：Check_TCP_Main_Connect(void)
功    能：检查连接主端口连接情况
输入参数：
输出参数：
编写时间：2013.4.25
编 写 人：
注    意：for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next) 
***********************************************************************/\
struct tcp_pcb *Check_TCP_Main_Connect(void)
{
	struct tcp_pcb *cpcb = 0;
	static unsigned char Main_connect_flag = 0;
	Main_connect_flag = 0;
	for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next)
	{
		if(cpcb->local_port == SOCK_MAIN && cpcb -> state == ESTABLISHED)//CLOSED		//如果TCP_LOCAL_PORT端口指定的连接没有断开
		{
			Main_connect_flag = 1;  						//连接标志		
      break;			
		}
	}

	if(Main_connect_flag == 0)  	// TCP_LOCAL_PORT指定的端口未连接或已断开
	{
		TCP_Client_Init(SOCK_MAIN,SERVER_PORT,default_server_ip[0],default_server_ip[1],default_server_ip[2],default_server_ip[3]); //重新连接
		cpcb = 0;
	}
	return cpcb;	
}
/***********************************************************************
函数名称：Check_TCP_Bus_Connect(void)
功    能：检查数据端口连接情况
输入参数：
输出参数：
编写时间：2013.4.25
编 写 人：
注    意：for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next) 
***********************************************************************/\
struct tcp_pcb *Check_TCP_Bus_Connect(void)
{
	struct tcp_pcb *cpcb = 0;
	static unsigned char Bus_connect_flag = 0;
	Bus_connect_flag = 0;
	for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next)
	{
		if(cpcb->local_port == SOCK_BUS && cpcb -> state == ESTABLISHED)//CLOSED		//如果TCP_LOCAL_PORT端口指定的连接没有断开
		{
			Bus_connect_flag = 1;  						//连接标志
			 
			break;							   	
		}
	}

	if(Bus_connect_flag == 0)  	// TCP_LOCAL_PORT指定的端口未连接或已断开
	{
		TCP_Client_Init(SOCK_BUS,sysCfg.parameter.server_port,sysCfg.parameter.server_ip[0],sysCfg.parameter.server_ip[1],sysCfg.parameter.server_ip[2],sysCfg.parameter.server_ip[3]); //重新连接
		cpcb = 0;
	}
	return cpcb;	
}

/***********************************************************************
函数名称：err_t TCP_Connected(void *arg,struct tcp_pcb *pcb,err_t err)
功    能：这是一个回调函数，当TCP客户端请求的连接建立时被调用
***********************************************************************/
err_t TCP_Connected(void *arg,struct tcp_pcb *pcb,err_t err)
{
	if(pcb->local_port==SOCK_MAIN)
	{
  mqtt_connect(pcb, default_server_user, default_server_pass,0);
	app_system_NetPublic(pcb);//上电信息
	app_system_TcpsStart(pcb);//订阅主题
	}
	if(pcb->local_port==SOCK_BUS)
	{
		mqtt_connect(pcb, (char *)sysCfg.parameter.server_user, (char *)sysCfg.parameter.server_pass,0);
		app_system_Subscrib(pcb);//订阅第三方主题
		mqtt_subscrib(pcb,"test",0,0);
	}
	return ERR_OK;
}
/***********************************************************************
函数名称：TCP_Client_Recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
功    能：tcp客户端接收数据回调函数�
注    意：这是一个回调函数，当TCP服务器发来数据时调用
***********************************************************************/

err_t  TCP_Client_Recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
{
	#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4	
	struct pbuf *p_temp;
  MQTTHeader header = {0};//MQTT 头部
	int len = 0;
	uint8_t buf[512]; //接收buf
	vTaskSuspendAll();//开启任务锁，防止调度
	memset(buf,0,sizeof(buf));
  p_temp = p;//TCP接收数据，链表结构
	if(p_temp != NULL)
	{	
		tcp_recved(pcb, p_temp->tot_len);//获取数据长度 tot_len：tcp数据块的长度
		#if 0
		App_Printf("p_temp->tot_len is %d\r\n",p_temp->tot_len);//当前数据总长度
 		App_Printf("p_temp->len is %d\r\n",p_temp->len);//当前块中数据长度
		#endif
		/*将不连续的链表地址 拷贝到连续的buf中 方便MQTT解析，风险未知*/
		while(p_temp != NULL)	
		{
			   memcpy(&buf[len],(uint8_t *)(p_temp->payload),p_temp->len);
		     len+=p_temp->len;
			   p_temp = p_temp->next;	
		}
		#if 0
		App_Printf("receive data:");
		for(uint16_t i=0;i<len;i++)
		App_Printf("%c",buf[i]);
		App_Printf("\r\n");
		#endif
	  header.byte = buf[0]; //MQTT头部
		switch(header.bits.type)
		{
			case PUBLISH:   /*接收订阅主题数据*/
				app_palt_Receicedata(buf,pcb,pcb->local_port);
				break;	
			case CONNACK:   /*连接回执*/
				 app_palt_Connectack(buf,pcb->local_port);
				break;		
			case PINGRESP:  /*到服务器ping回执*/
				break;
			
			default:
				break;
		}	  		
	}
	else
	{
		tcp_close(pcb); 											/* 作为TCP服务器不应主动关闭这个连接？ */
	}
	/* 释放该TCP段 */
	pbuf_free(p); 	
	err = ERR_OK;
	xTaskResumeAll();//恢复任务调度
	return err;
}
/***********************************************************************
函数名称：TCP_Client_Init(u16_t local_port,u16_t remote_port,unsigned char a,unsigned char b,unsigned char c,unsigned char d)
功    能：tcp客户端初始化
输入参数：local_port本地端口号；remote_port：目标端口号；a,b,c,d：服务器ip
输出参数：
编写时间：2013.4.25
编 写 人：
注    意：
***********************************************************************/
void TCP_Client_Init(u16_t local_port,u16_t remote_port,unsigned char a,unsigned char b,unsigned char c,unsigned char d)
{

	struct ip_addr ipaddr;
	err_t err;
	struct tcp_pcb *tcp_client_pcb;
	IP4_ADDR(&ipaddr,a,b,c,d);   //服务器IP地址
	tcp_client_pcb = tcp_new(); /* 建立通信的TCP控制块(Clipcb) */
	if (!tcp_client_pcb)
	{
		return ;
	}		
	err = tcp_bind(tcp_client_pcb,IP_ADDR_ANY,local_port); /* 绑定本地IP地址和端口号 ，本地ip地址在LwIP_Init()中已经初始化*/
	if(err != ERR_OK)
	{
		return ;
	}    
	tcp_connect(tcp_client_pcb,&ipaddr,remote_port,TCP_Connected);//注册回调函数
	tcp_recv(tcp_client_pcb,TCP_Client_Recv); 				/* 设置tcp接收回调函数 */
}
/*UDP ntp 消息接收回调处理函数*/
void udp_ntp_recv_rb(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
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
void udp_senddata(struct udp_pcb *upcb)
{
    struct pbuf *ptr;
	  uint8_t ntp_message[48]={0};
		ntp_message[0]=0xA3;
    ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ntp_message),PBUF_POOL); 
    if(ptr)
    {
        ptr->payload=(void*)ntp_message; 
        udp_send(upcb,ptr); 
        pbuf_free(ptr);
    } 
} 
/*UDP pc 消息接收回调处理函数*/
void udp_pc_recv_rb(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{   
	  uint8_t first[36]={0xFF,0x24,0x01,0x00,0x00,0xc0,0xA8,0x00,0x94,0x9C,0xA5,0x25,0x9A,0x90,0x1B,\
		0xB2,0x0F,0x00,0x00,0x53,0x53,0x57,0x2D,0x43,0x41,0x4E,0x4F,0x50,0x45,0x2D,0x30,0x30,0x31,0x00,\
		0x00,0x09};
		uint8_t secend[130]={0x95,0x63,0x03,0x40,0x00,0x00,0x50,0x00,0x00,0x07,0x00,0xA8,0xC0,0x01,0x00,0xA8,0xC0,0x00,\
		0xFF,0xFF,0xFF,0x53,0x53,0x57,0x2D,0x43,0x41,0x4E,0x4F,0x50,0x45,0x2D,0x30,0x30,0x31,0x00,0x00,0x61,0x64,0x6D,\
		0x69,0x6E,0x00,0x61,0x64,0x6D,0x69,0x6E,0x00,0x00,0x01,0x00,0xA0,0x9C,0xA5,0x25,0x9A,0x90,0x1B,0xDE,0xDE,0x43,\
		0xD0,0x03,0x00,0x00,0x00,0x00,0xC2,0x01,0x00,0x08,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x2A,0x20,0x31,\
		0x39,0x32,0x2E,0x31,0x36,0x38,0x2E,0x30,0x2E,0x32,0x30,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC9,0x00,0xA8,0xC0,0x20,0x01,0x00,0x04,0x10,0x0E,0x00,0x00,0x00,0x00,0x00,\
		0x00,0x00};
		uint8_t third[85]={0x00,0x1E,0x00,0x0A,0x0D,0x77,0x77,0x77,0x2E,0x75,0x73,0x72,0x2E,0x63,0x6E,0x00,0x00,0x00,\
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
		0x00,0x00,0x00,0x00,0x00,0x77,0x77,0x77,0x2E,0x75,0x73,0x72,0x2E,0x63,0x6E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		uint8_t ack[4]={0xFF,0x01,0x00,0x4B};
		uint8_t mac[6]={0};
		char strip[20]={0};//上位机下发ip 为字符串格式
		uint32_t ipval=0;
		char password[41]={0};//(用户名+’,‘+密码)长度之和不能超过40 
		char *pr=NULL;
		uint8_t i;
		struct pbuf *ptr;
	  uint8_t type=0;//消息类型
	  struct ip_addr my_ipaddr;
	  unsigned char *temp = (unsigned char *)addr;
		IP4_ADDR(&my_ipaddr, temp[0], temp[1], temp[2], temp[3]); 
	  type=*((char*)p->payload+2);
	  switch(type)
		{
			case 0x01:
				
					memcpy(&first[5],sysCfg.parameter.ip,6);
				  memcpy(&first[9],sysCfg.parameter.client_mac,6); 
				  ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(first),PBUF_POOL);
				  if(ptr)
				  {
           memcpy(ptr->payload,first,sizeof(first));
           //ptr->payload=(void*)first;	//会造成崩溃？			 
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }					    			
				break;
			case 0x02:
				memcpy(mac,(char*)p->payload+3,6);
			  if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
					break;
				ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
			  ack[2]=0x02;
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }					
				if(0 == bsp_WriteCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN))
			    {
				  delay_ms(100);
				  NVIC_SystemReset();		
			    } 
				break;
			case 0x03:
				  memcpy(mac,(char*)p->payload+3,6);
			    if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
						break;
					App_Printf("search:");
					for(i=0;i<6;i++)
			    App_Printf("%02x",mac[i]);
			    App_Printf("\r\n");
					/*配置过静态ip,0xc0 STATIC 0x40:DHCP*/
          if(sysCfg.parameter.dhcp==STATIC)
						secend[3]=0xc0;
					/*拷贝ip,转化为小端*/
					 for(i=0;i<4;i++)
					 {
						 secend[9+i]=sysCfg.parameter.ip[3-i];
					 }
					 /*拷贝网关*/
					 for(i=0;i<4;i++)
					 {
						 secend[13+i]=sysCfg.parameter.gw[3-i];
					 }
					 /*拷贝掩码*/
					  for(i=0;i<4;i++)
					 {
						 secend[17+i]=sysCfg.parameter.sub[3-i];
					 }
					 /*拷贝远程服务器ip*/
					 sprintf((char*)&secend[83],"%d.%d.%d.%d",sysCfg.parameter.server_ip[0],sysCfg.parameter.server_ip[1],sysCfg.parameter.server_ip[2],sysCfg.parameter.server_ip[3]);
					 secend[81]=sysCfg.parameter.server_port&0xFF;
					 secend[82]=sysCfg.parameter.server_port/256;
					 memcpy(&secend[53],sysCfg.parameter.client_mac,6);
					 ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(secend),PBUF_POOL);
				  if(ptr)
				  {
           memcpy(ptr->payload,secend,sizeof(secend));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
					memcpy(&third[45],sysCfg.parameter.server_user,strlen(sysCfg.parameter.server_user));
					third[45+strlen(sysCfg.parameter.server_user)]=',';
					memcpy(&third[45+1+strlen(sysCfg.parameter.server_user)],sysCfg.parameter.server_pass,strlen(sysCfg.parameter.server_pass));
					ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(third),PBUF_POOL);
				  if(ptr)
				  {
           memcpy(ptr->payload,third,sizeof(third));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
							
			break;
			
			case 0x05:
				memcpy(mac,(char*)p->payload+3,6);
				if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
					break;
				App_Printf("save:");
				for(i=0;i<6;i++)
				App_Printf("%02x",mac[i]);
				App_Printf("\r\n");
				 if(0xc0==*((unsigned char*)p->payload+24))
				 {
					 sysCfg.parameter.dhcp=STATIC;
					 App_Printf("Static\r\n");
			/*拷贝ip,转化为小端*/
				 for(i=0;i<4;i++)
				 {
					sysCfg.parameter.ip[3-i]=*((unsigned char*)p->payload+30+i);
				 }
				 /*拷贝网关*/
				 for(i=0;i<4;i++)
				 {
					 sysCfg.parameter.gw[3-i]=*((unsigned char*)p->payload+34+i);
				 }
				 /*拷贝掩码*/
					for(i=0;i<4;i++)
				 {
					 sysCfg.parameter.sub[3-i]=*((unsigned char*)p->payload+38+i);
				 }
			   }
				 else
				 {
					 sysCfg.parameter.dhcp=DHCP;
					 App_Printf("Dhcp\r\n");
				  }
				  ack[2]=0x05;
				  ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack)); 
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	 
				break;
			case 0x06:
				
				memcpy(mac,(char*)p->payload+3,6);
				if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
					break;
			 /*拷贝远程服务器ip*/
			  strcpy(strip,(const char*)p->payload+37);
				App_Printf("ip:%s\r\n",strip);
				ipval=inet_addr(strip);
				sysCfg.parameter.server_ip[0]=(uint8_t)(ipval);
				sysCfg.parameter.server_ip[1]=(uint8_t)(ipval >> 8);
				sysCfg.parameter.server_ip[2]=(uint8_t)(ipval >> 16);
				sysCfg.parameter.server_ip[3]=(uint8_t)(ipval >> 24);
			  sysCfg.parameter.server_port=(uint16_t)(*((unsigned char*)p->payload+35)|(*((unsigned char*)p->payload+36)<<8));
				if(  (0 == memcmp(sysCfg.parameter.server_ip,default_server_ip,sizeof(default_server_ip)))
					&& (0 == strncmp((char *)sysCfg.parameter.server_user,default_server_user,strlen(default_server_user))) 
					&& (0 == strncmp((char *)sysCfg.parameter.server_user,default_server_user,strlen((char *)sysCfg.parameter.server_user))) 
					&& (0 == strncmp((char *)sysCfg.parameter.server_pass,default_server_pass,strlen(default_server_pass)))
					&& (0 == strncmp((char *)sysCfg.parameter.server_pass,default_server_pass,strlen((char *)sysCfg.parameter.server_pass))))
				{
					sysCfg.parameter.config_hold_flag = 0xff;
				}
				else
				{
					sysCfg.parameter.config_hold_flag = CFG_HOLDER;
				}

				#if APP_DEBUG   
				App_Printf("\r\nserver IP:");
				for(uint8_t i=0;i<3;i++)
				{
					App_Printf("%d",sysCfg.parameter.server_ip[i]);
					App_Printf(".");
				}
				App_Printf("%d",sysCfg.parameter.server_ip[3]);

				App_Printf("      PORT:%d",sysCfg.parameter.server_port);
				App_Printf("\r\n");
				#endif
				ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
			  ack[2]=0x06;
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
				break;
			case 0x0B:
				  memcpy(mac,(char*)p->payload+3,6);
			    if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
						break;
					ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
			    ack[2]=0x0B;
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
				sysCfg.parameter.config_hold_flag = 0xff;  /*恢复出厂设置*/
			  sysCfg.parameter.dhcp = DHCP;			 /*默认为开启DHCP*/
				if(0 == bsp_WriteCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN))
				{
					delay_ms(100);
					NVIC_SystemReset();		
				}
				break;
			case 0x21:
				memcpy(mac,(char*)p->payload+3,6);
				if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
					break;
				strncpy(password,(const char*)p->payload+66,40);
				App_Printf("password:%s\r\n",password);
				pr=strstr((const char*)password,",");
				if(NULL!=pr)
				{
					memcpy(sysCfg.parameter.server_user,password,pr-password);
					sysCfg.parameter.server_user[pr-password]='\0';
					strncpy(sysCfg.parameter.server_pass,pr+1,40-(pr+1-password));
					sysCfg.parameter.server_pass[40-(pr+1-password)]='\0';
					App_Printf("server username:%s\r\n",sysCfg.parameter.server_user);
				  App_Printf("server password:%s\r\n",sysCfg.parameter.server_pass);
				}
				else
				{
					App_Printf("Format is not correct\r\n");
				}
				
				ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
			  ack[2]=0x21;
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
				break;
					
			default:
				break;			
		}	
    pbuf_free(p);
}
struct udp_pcb * udp_pc_init(void)
{
    err_t err;
    struct udp_pcb *udppcb;
    struct ip_addr rmtipaddr;   
    udppcb = udp_new();
    if(udppcb){
        err=udp_bind(udppcb,IP_ADDR_ANY,1500);
        udp_recv(udppcb , udp_pc_recv_rb , NULL);
    }
		return udppcb;
}
void DHCP_run(void)
{
	struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  uint32_t IPaddress;
  uint8_t DHCP_state;  
	int IP_STAUS = 0;
	DHCP_state = DHCP_START;
  do{
		switch (DHCP_state)
		{
			case DHCP_START:
			{
				dhcp_start(&netif);
				IPaddress = 0;
				DHCP_state = DHCP_WAIT_ADDRESS;
				App_Printf("Please wait...\r\n");
			}
      break;
			case DHCP_WAIT_ADDRESS:
			{	
				IPaddress = netif.ip_addr.addr;//读取新的IP地址
				if (IPaddress!=0) 
				{
					DHCP_state = DHCP_ADDRESS_ASSIGNED;					
					dhcp_stop(&netif);//关闭DHCP  
					App_Printf("IP address assigned by a DHCP server :%d.%d.%d.%d\r\n",(uint8_t)(IPaddress),(uint8_t)(IPaddress >> 8),(uint8_t)(IPaddress >> 16),(uint8_t)(IPaddress >> 24));
					IP_STAUS=1;//IP地址就绪后
				}
				else
				{          
					if (netif.dhcp->tries > MAX_DHCP_TRIES)//超时未获得IP地址
					{
						DHCP_state = DHCP_TIMEOUT;
					 
						dhcp_stop(&netif);//关闭DHCP
						/*使用默认静态IP*/
						IP4_ADDR(&ipaddr, MyIP1 ,MyIP2 , MyIP3 ,MyIP4 );
						IP4_ADDR(&netmask, 255, 255, 255, 0);
						IP4_ADDR(&gw, 192, 168, 0, 1);
						netif_set_addr(&netif, &ipaddr , &netmask, &gw);
						App_Printf("DHCP timeout!\r\nStatic IP address:%d.%d.%d.%d\r\n",MyIP1,MyIP2,MyIP3,MyIP4);
						sysCfg.parameter.dhcp=STATIC;
						IP_STAUS=1;//IP地址就绪后
					}				
				}
			}
			break;
			
			default:
			break;
		}
		vTaskDelay(250);
	}while(!IP_STAUS); 
	
}
/***********************************************************************
函数名称：void Task_TCP_Client(void *pdata)
功    能：
***********************************************************************/
void Task_TCP_Client(void *pvParameters)
{
	struct udp_pcb *udppcb; //UDP 通信块
	struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
	static  uint16_t wait_bus_socket_ack_time = 0;
  static  uint16_t wait_main_socket_ack_time = 0;
	volatile uint8_t flag=1;
	if(sysCfg.parameter.dhcp == STATIC)
	{
		IP4_ADDR(&ipaddr, sysCfg.parameter.ip[0] ,sysCfg.parameter.ip[1],sysCfg.parameter.ip[2],sysCfg.parameter.ip[3]  );
		IP4_ADDR(&netmask, sysCfg.parameter.sub[0], sysCfg.parameter.sub[1], sysCfg.parameter.sub[2], sysCfg.parameter.sub[3]);
		IP4_ADDR(&gw, sysCfg.parameter.gw[0], sysCfg.parameter.gw[1], sysCfg.parameter.gw[2], sysCfg.parameter.gw[3]);
		netif_set_addr(&netif, &ipaddr , &netmask, &gw);
		App_Printf("Static IP address:%d.%d.%d.%d\r\n",sysCfg.parameter.ip[0] ,sysCfg.parameter.ip[1],sysCfg.parameter.ip[2],sysCfg.parameter.ip[3]);
	}
	else
	{
		App_Printf("Ready to run DHCP...\r\n");
		DHCP_run();
	}
	memcpy(sysCfg.parameter.ip,(char*)&netif.ip_addr.addr,4);//获取网卡中的本地ip地址
	memcpy(sysCfg.parameter.gw,(char*)&netif.gw.addr,4); //获取网卡中的网关
	memcpy(sysCfg.parameter.sub,(char*)&netif.netmask.addr,4);//获取网卡中的掩码
	udp_pc_init();//初始化UDP,用于与上位机通讯
	udppcb=udp_ntp_init();//初始化UDP，用于ntp通信
  app_tcp_init();//初始化tcp
	while(1)
	{	
		/*检测主端口连接状态*/
		if(flag==1)
		{
				 main_pcb = Check_TCP_Main_Connect();
				if(sysCfg.parameter.data_socket == SOCK_BUS)//配置了第三方服务器
				{
					bus_pcb=Check_TCP_Bus_Connect();
				}	
				if((main_pcb!=NULL)&&(bus_pcb!=NULL))
					flag=0;	
	  }
		if(MQTT_CONNECT == app_system_mqtt_connect_state_get(SOCK_MAIN))
		{
			app_system_NetLedToggle();//状态指示灯
			udp_senddata(udppcb);//发送udp包	
			wait_main_socket_ack_time++;
			if(wait_main_socket_ack_time >=HEART_OUTTIME_TIMES)
			{
				#if APP_DEBUG
				App_Printf("ping main socket outtime\r\n");
				#endif
				bsp_LedOff(LED_BLUE);
				bsp_LedOff(LED_GREEN);
				bsp_LedOn(LED_RED);
				wait_main_socket_ack_time = 0;
				app_system_mqtt_connect_state_flag_set(SOCK_MAIN,MQTT_DISCONNECT);			
				tcp_close(main_pcb);
			}
			if(main_heart_flag==RECEIVE_HEART)        /*收到主服务器的PING响应*/
			{
				main_heart_flag=RECEIVE_CLEAR;
				wait_main_socket_ack_time = 0;     /*清零等待时间*/
			}
		}					 
		if(MQTT_CONNECT == app_system_mqtt_connect_state_get(SOCK_BUS))
		{

			wait_bus_socket_ack_time++;
			if(wait_bus_socket_ack_time >= HEART_OUTTIME_TIMES)
			{
				
				#if APP_DEBUG
				App_Printf("ping bus socket outtime\r\n");
				#endif
				bsp_LedOff(LED_BLUE);
				bsp_LedOff(LED_GREEN);
				bsp_LedOn(LED_RED);
				wait_bus_socket_ack_time = 0;
				app_system_mqtt_connect_state_flag_set(SOCK_BUS,MQTT_DISCONNECT);			
				tcp_close(bus_pcb);
			}
			if(bus_heart_flag == RECEIVE_HEART)  	 /*收到第三方服务器的PING响应*/
			{
				bus_heart_flag=RECEIVE_CLEAR;
				wait_bus_socket_ack_time = 0;      /*清零等待时间*/
			}
		}
		vTaskDelay(1000);
	}
}
