/***********************************************************************
文件名称：TCP_CLIENT.C
功    能：完成TCP的客户端的数据收发
编写时间：2013.4.25
编 写 人：
注    意：
***********************************************************************/
#include "main.h"
extern  struct netif netif;
struct tcp_pcb *main_pcb; //TCP 通信块
struct tcp_pcb *bus_pcb; //TCP 通信块
/*
*********************************************************************************************************
 * 函数名：Check_TCP_Main_Connect
 * 描述  ：检查Lwip协议栈 主端口连接情况 如果未连接初始化重新连接
 * 输入  ：无
 * 返回  : 处于连接状态的TCP块
*********************************************************************************************************
*/
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
		app_system_mqtt_connect_state_flag_set(SOCK_MAIN,MQTT_DISCONNECT);	
		TCP_Client_Init(SOCK_MAIN,SERVER_PORT,default_server_ip[0],default_server_ip[1],default_server_ip[2],default_server_ip[3]); //重新连接
		cpcb = 0;
	}
	return cpcb;	
}
/*
*********************************************************************************************************
 * 函数名：Check_TCP_Bus_Connect
 * 描述  ：检查Lwip协议栈 数据端口连接情况 如果未连接初始化重新连接
 * 输入  ：无
 * 返回  : 处于连接状态的TCP块
*********************************************************************************************************
*/
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
		app_system_mqtt_connect_state_flag_set(SOCK_BUS,MQTT_DISCONNECT);
		TCP_Client_Init(SOCK_BUS,sysCfg.parameter.server_port,sysCfg.parameter.server_ip[0],sysCfg.parameter.server_ip[1],sysCfg.parameter.server_ip[2],sysCfg.parameter.server_ip[3]); //重新连接
		cpcb = 0;
	}
	return cpcb;	
}

/*
*********************************************************************************************************
 * 函数名：TCP_Connected
 * 描述  ：当TCP连接时会调用此函数
 * 输入  ：pcb TCP块
 * 返回  : 
*********************************************************************************************************
*/
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
/*
*********************************************************************************************************
 * 函数名：TCP_Client_Recv
 * 描述  ：当TCP接收到数据时 会调用该函数
 * 输入  ：pcb TCP块  p 数据Buf 
 * 返回  : 
*********************************************************************************************************
*/
err_t  TCP_Client_Recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
{
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
	  header.byte = buf[0]; /*MQTT头部*/
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
/*
*********************************************************************************************************
 * 函数名：TCP_Client_Init
 * 描述  ：初始化TCP客户端  
						1.建立新的tcp块
						2.绑定
						3.连接
 * 输入  ：local_port 本地端口 remote_port 远程端口 a b c d 远程ip
 * 返回  : 
*********************************************************************************************************
*/
void TCP_Client_Init(u16_t local_port,u16_t remote_port,unsigned char a,unsigned char b,unsigned char c,unsigned char d)
{

	struct ip_addr ipaddr;
	err_t err;
	struct tcp_pcb *tcp_client_pcb;
	/*服务器IP地址*/
	IP4_ADDR(&ipaddr,a,b,c,d); 
 /* 建立通信的TCP控制块(Clipcb) */  
	tcp_client_pcb = tcp_new(); 
	if (!tcp_client_pcb)
	{
		return ;
	}
  /* 绑定本地IP地址和端口号 IP_ADDR_ANY 表示网卡中的地址 在这儿之前已经设置好*/	
	err = tcp_bind(tcp_client_pcb,IP_ADDR_ANY,local_port); 
	if(err != ERR_OK)
	{
		return ;
	} 
  /*连接服务器 并注册回调函数*/
	tcp_connect(tcp_client_pcb,&ipaddr,remote_port,TCP_Connected);
	/* 设置tcp接收回调函数 */
	tcp_recv(tcp_client_pcb,TCP_Client_Recv); 				
}
/*
*********************************************************************************************************
 * 函数名：DHCP_run
 * 描述  ：动态获取ip地址 
 * 输入  ：
 * 返回  : 
*********************************************************************************************************
*/
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
				/*读取新的IP地址*/
				IPaddress = netif.ip_addr.addr;
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
extern UNS32 ID_Update(CO_Data* d, const indextable *indextable, UNS8 bSubindex);
/*
*********************************************************************************************************
 * 函数名：Task_TCP_Client
 * 描述  ：处理TCP 任务
 * 输入  ：
 * 返回  : 
*********************************************************************************************************
*/
void Task_TCP_Client(void *pvParameters)
{
	struct udp_pcb *udppcb; //UDP 通信块
	struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
	static  uint16_t wait_bus_socket_ack_time = 0;
  static  uint16_t wait_main_socket_ack_time = 0;
	volatile uint8_t flag=1;
	setNodeId(&TestMaster_Data, 0);
  setState(&TestMaster_Data, Initialisation);
  setState(&TestMaster_Data, Operational);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2000, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2001, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2002, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2003, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2004, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2005, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2006, 0x00, ID_Update);
	RegisterSetODentryCallBack(&TestMaster_Data, 0x2007, 0x00, ID_Update);
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
	//udppcb=udp_ntp_init();//初始化UDP，用于ntp通信
  app_tcp_init();//初始化tcp
	while(1)
	{	
		/*检测主端口连接状态*/

			 main_pcb = Check_TCP_Main_Connect();
			if(sysCfg.parameter.data_socket == SOCK_BUS)//配置了第三方服务器
			{
				bus_pcb=Check_TCP_Bus_Connect();
			}	
		if(MQTT_CONNECT == app_system_mqtt_connect_state_get(SOCK_MAIN))
		{
			app_system_NetLedToggle();//状态指示灯
			//udp_senddata(udppcb);//发送udp包	
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
