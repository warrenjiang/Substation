/***********************************************************************
文件名称：TCP_CLIENT.C
功    能：完成TCP的客户端的数据收发
编写时间：2013.4.25
编 写 人：
注    意：
***********************************************************************/
#include "main.h"
#include "lwip/dhcp.h"
extern  struct netif netif;
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
	err=tcp_output(cpcb);
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
/*UDP 消息接收回调处理函数*/
void udp_recv_rb(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
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
struct udp_pcb * udp_app_init(void)
{
    err_t err;
    struct udp_pcb *udppcb;
    struct ip_addr rmtipaddr;   
    udppcb = udp_new();
    if(udppcb){
        IP4_ADDR(&rmtipaddr,120,25,108,11);
        err=udp_connect(udppcb,&rmtipaddr,123);
        err=udp_bind(udppcb,IP_ADDR_ANY,3000);
        udp_recv(udppcb , udp_recv_rb , NULL);
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
	struct tcp_pcb *Main_pcb; //TCP 通信块
	struct tcp_pcb *Bus_pcb; //TCP 通信块
	struct udp_pcb *udppcb; //UDP 通信块
	struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
	static uint16_t wait_bus_socket_ack_time = 0;
	static uint16_t wait_main_socket_ack_time = 0;
	
	if(sysCfg.parameter.dhcp == STATIC)
	{
		IP4_ADDR(&ipaddr, sysCfg.parameter.ip[0] ,sysCfg.parameter.ip[1],sysCfg.parameter.ip[2],sysCfg.parameter.ip[3]  );
		IP4_ADDR(&netmask, sysCfg.parameter.sub[0], sysCfg.parameter.sub[1], sysCfg.parameter.sub[2], sysCfg.parameter.sub[3]);
		IP4_ADDR(&gw, sysCfg.parameter.gw[0], sysCfg.parameter.gw[1], sysCfg.parameter.gw[2], sysCfg.parameter.gw[3]);
		netif_set_addr(&netif, &ipaddr , &netmask, &gw);
		App_Printf("Static IP address:%d.%d.%d.%dr\n",sysCfg.parameter.ip[0] ,sysCfg.parameter.ip[1],sysCfg.parameter.ip[2],sysCfg.parameter.ip[3]);
	}
	else
	{
		App_Printf("Ready to run DHCP...\r\n");
		DHCP_run();
	}
	memcpy(sysCfg.parameter.ip,(char*)&netif.ip_addr.addr,4);//获取网卡中的本地ip地址
	udppcb=udp_app_init();//初始化UDP，用于ntp通信
  app_tcp_init();//初始化tcp
	while(1)
	{
		Main_pcb = Check_TCP_Main_Connect();
    if(sysCfg.parameter.data_socket == SOCK_BUS)//配置了第三方服务器
		{
			Bus_pcb=Check_TCP_Bus_Connect();
		}	
			/*PHY状态正常且已经连接上服务器*/
		if(MQTT_CONNECT == app_system_mqtt_connect_state_get(SOCK_MAIN))
		{ 
			/*未接收到服务器通信信息时间累加*/
			wait_main_socket_ack_time++;
			if(sysCfg.parameter.data_socket != SOCK_MAIN)
			{
				wait_bus_socket_ack_time++;
			}
			
			/*与服务器保持通讯超时 超过 HEART_OUTTIME_TIMES 时间未接收到服务器通信信息 指示红灯*/
			if((wait_main_socket_ack_time > HEART_OUTTIME_TIMES) || (wait_bus_socket_ack_time > HEART_OUTTIME_TIMES)) 
			{
				bsp_LedOn(LED_RED);
				bsp_LedOff(LED_BLUE);
				bsp_LedOff(LED_GREEN);
			}
			else  /*网关指示灯正常指示*/
			{
				app_system_NetLedToggle();
        udp_senddata(udppcb);//发送udp包				
			}
			
			if(main_heart_flag==RECEIVE_HEART)        /*收到主服务器的PING响应*/
			{
				main_heart_flag=RECEIVE_CLEAR;
				wait_main_socket_ack_time = 0;     /*清零等待时间*/
			}
			if(bus_heart_flag == RECEIVE_HEART)  	 /*收到第三方服务器的PING响应*/
			{
				bus_heart_flag=RECEIVE_CLEAR;
				wait_bus_socket_ack_time = 0;      /*清零等待时间*/
			}
			/*主服务器 PING 超出MQTT_RECONNECT_TIME秒无响应 重连*/
			if(wait_main_socket_ack_time > MQTT_RECONNECT_TIME)  
			{
				#if APP_DEBUG
				printf("ping main socket outtime\r\n");
				#endif
				wait_main_socket_ack_time = 0;
				app_system_mqtt_connect_state_flag_set(SOCK_MAIN,MQTT_DISCONNECT);			
				tcp_close(Main_pcb); 
			}	
			/*第三方服务器 PING 超出MQTT_RECONNECT_TIME秒无响应 重连*/
			if(wait_bus_socket_ack_time > MQTT_RECONNECT_TIME)
			{
				#if APP_DEBUG
				printf("ping bus socket outtime\r\n");
				#endif
				wait_bus_socket_ack_time = 0;	
				app_system_mqtt_connect_state_flag_set(SOCK_BUS,MQTT_DISCONNECT); 
				tcp_close(Bus_pcb); 
			}
		}
		vTaskDelay(1000);
	}
}
