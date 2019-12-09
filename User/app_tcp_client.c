/***********************************************************************
�ļ����ƣ�TCP_CLIENT.C
��    �ܣ����TCP�Ŀͻ��˵������շ�
��дʱ�䣺2013.4.25
�� д �ˣ�
ע    �⣺
***********************************************************************/
#include "main.h"
extern  struct netif netif;
struct tcp_pcb *main_pcb; //TCP ͨ�ſ�
struct tcp_pcb *bus_pcb; //TCP ͨ�ſ�
/*
*********************************************************************************************************
 * ��������Check_TCP_Main_Connect
 * ����  �����LwipЭ��ջ ���˿�������� ���δ���ӳ�ʼ����������
 * ����  ����
 * ����  : ��������״̬��TCP��
*********************************************************************************************************
*/
struct tcp_pcb *Check_TCP_Main_Connect(void)
{
	struct tcp_pcb *cpcb = 0;
	static unsigned char Main_connect_flag = 0;
	Main_connect_flag = 0;
	for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next)
	{
		if(cpcb->local_port == SOCK_MAIN && cpcb -> state == ESTABLISHED)//CLOSED		//���TCP_LOCAL_PORT�˿�ָ��������û�жϿ�
		{
			Main_connect_flag = 1;  						//���ӱ�־		
      break;			
		}
	}

	if(Main_connect_flag == 0)  	// TCP_LOCAL_PORTָ���Ķ˿�δ���ӻ��ѶϿ�
	{
		app_system_mqtt_connect_state_flag_set(SOCK_MAIN,MQTT_DISCONNECT);	
		TCP_Client_Init(SOCK_MAIN,SERVER_PORT,default_server_ip[0],default_server_ip[1],default_server_ip[2],default_server_ip[3]); //��������
		cpcb = 0;
	}
	return cpcb;	
}
/*
*********************************************************************************************************
 * ��������Check_TCP_Bus_Connect
 * ����  �����LwipЭ��ջ ���ݶ˿�������� ���δ���ӳ�ʼ����������
 * ����  ����
 * ����  : ��������״̬��TCP��
*********************************************************************************************************
*/
struct tcp_pcb *Check_TCP_Bus_Connect(void)
{
	struct tcp_pcb *cpcb = 0;
	static unsigned char Bus_connect_flag = 0;
	Bus_connect_flag = 0;
	for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next)
	{
		if(cpcb->local_port == SOCK_BUS && cpcb -> state == ESTABLISHED)//CLOSED		//���TCP_LOCAL_PORT�˿�ָ��������û�жϿ�
		{
			Bus_connect_flag = 1;  						//���ӱ�־
			 
			break;							   	
		}
	}

	if(Bus_connect_flag == 0)  	// TCP_LOCAL_PORTָ���Ķ˿�δ���ӻ��ѶϿ�
	{
		app_system_mqtt_connect_state_flag_set(SOCK_BUS,MQTT_DISCONNECT);
		TCP_Client_Init(SOCK_BUS,sysCfg.parameter.server_port,sysCfg.parameter.server_ip[0],sysCfg.parameter.server_ip[1],sysCfg.parameter.server_ip[2],sysCfg.parameter.server_ip[3]); //��������
		cpcb = 0;
	}
	return cpcb;	
}

/*
*********************************************************************************************************
 * ��������TCP_Connected
 * ����  ����TCP����ʱ����ô˺���
 * ����  ��pcb TCP��
 * ����  : 
*********************************************************************************************************
*/
err_t TCP_Connected(void *arg,struct tcp_pcb *pcb,err_t err)
{
	if(pcb->local_port==SOCK_MAIN)
	{
  mqtt_connect(pcb, default_server_user, default_server_pass,0);
	app_system_NetPublic(pcb);//�ϵ���Ϣ
	app_system_TcpsStart(pcb);//��������
	}
	if(pcb->local_port==SOCK_BUS)
	{
		mqtt_connect(pcb, (char *)sysCfg.parameter.server_user, (char *)sysCfg.parameter.server_pass,0);
		app_system_Subscrib(pcb);//���ĵ���������
		mqtt_subscrib(pcb,"test",0,0);
	}
	return ERR_OK;
}
/*
*********************************************************************************************************
 * ��������TCP_Client_Recv
 * ����  ����TCP���յ�����ʱ ����øú���
 * ����  ��pcb TCP��  p ����Buf 
 * ����  : 
*********************************************************************************************************
*/
err_t  TCP_Client_Recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
{
	struct pbuf *p_temp;
  MQTTHeader header = {0};//MQTT ͷ��
	int len = 0;
	uint8_t buf[512]; //����buf
	vTaskSuspendAll();//��������������ֹ����
	memset(buf,0,sizeof(buf));
  p_temp = p;//TCP�������ݣ�����ṹ
	if(p_temp != NULL)
	{	
		tcp_recved(pcb, p_temp->tot_len);//��ȡ���ݳ��� tot_len��tcp���ݿ�ĳ���
		#if 0
		App_Printf("p_temp->tot_len is %d\r\n",p_temp->tot_len);//��ǰ�����ܳ���
 		App_Printf("p_temp->len is %d\r\n",p_temp->len);//��ǰ�������ݳ���
		#endif
		/*���������������ַ ������������buf�� ����MQTT����������δ֪*/
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
	  header.byte = buf[0]; /*MQTTͷ��*/
		switch(header.bits.type)
		{
			case PUBLISH:   /*���ն�����������*/
				app_palt_Receicedata(buf,pcb,pcb->local_port);
				break;	
			case CONNACK:   /*���ӻ�ִ*/
				 app_palt_Connectack(buf,pcb->local_port);
				break;		
			case PINGRESP:  /*��������ping��ִ*/
				break;
			default:
				break;
		}	  		
	}
	else
	{
		tcp_close(pcb); 											/* ��ΪTCP��������Ӧ�����ر�������ӣ� */
	}
	/* �ͷŸ�TCP�� */
	pbuf_free(p); 	
	err = ERR_OK;
	xTaskResumeAll();//�ָ��������
	return err;
}
/*
*********************************************************************************************************
 * ��������TCP_Client_Init
 * ����  ����ʼ��TCP�ͻ���  
						1.�����µ�tcp��
						2.��
						3.����
 * ����  ��local_port ���ض˿� remote_port Զ�̶˿� a b c d Զ��ip
 * ����  : 
*********************************************************************************************************
*/
void TCP_Client_Init(u16_t local_port,u16_t remote_port,unsigned char a,unsigned char b,unsigned char c,unsigned char d)
{

	struct ip_addr ipaddr;
	err_t err;
	struct tcp_pcb *tcp_client_pcb;
	/*������IP��ַ*/
	IP4_ADDR(&ipaddr,a,b,c,d); 
 /* ����ͨ�ŵ�TCP���ƿ�(Clipcb) */  
	tcp_client_pcb = tcp_new(); 
	if (!tcp_client_pcb)
	{
		return ;
	}
  /* �󶨱���IP��ַ�Ͷ˿ں� �IIP_ADDR_ANY ��ʾ�����еĵ�ַ �����֮ǰ�Ѿ����ú�*/	
	err = tcp_bind(tcp_client_pcb,IP_ADDR_ANY,local_port); 
	if(err != ERR_OK)
	{
		return ;
	} 
  /*���ӷ����� ��ע��ص�����*/
	tcp_connect(tcp_client_pcb,&ipaddr,remote_port,TCP_Connected);
	/* ����tcp���ջص����� */
	tcp_recv(tcp_client_pcb,TCP_Client_Recv); 				
}
/*
*********************************************************************************************************
 * ��������DHCP_run
 * ����  ����̬��ȡip��ַ 
 * ����  ��
 * ����  : 
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
				/*��ȡ�µ�IP��ַ*/
				IPaddress = netif.ip_addr.addr;
				if (IPaddress!=0) 
				{
					DHCP_state = DHCP_ADDRESS_ASSIGNED;					
					dhcp_stop(&netif);//�ر�DHCP  
					App_Printf("IP address assigned by a DHCP server :%d.%d.%d.%d\r\n",(uint8_t)(IPaddress),(uint8_t)(IPaddress >> 8),(uint8_t)(IPaddress >> 16),(uint8_t)(IPaddress >> 24));
					IP_STAUS=1;//IP��ַ������
				}
				else
				{          
					if (netif.dhcp->tries > MAX_DHCP_TRIES)//��ʱδ���IP��ַ
					{
						DHCP_state = DHCP_TIMEOUT;
					 
						dhcp_stop(&netif);//�ر�DHCP
						/*ʹ��Ĭ�Ͼ�̬IP*/
						IP4_ADDR(&ipaddr, MyIP1 ,MyIP2 , MyIP3 ,MyIP4 );
						IP4_ADDR(&netmask, 255, 255, 255, 0);
						IP4_ADDR(&gw, 192, 168, 0, 1);
						netif_set_addr(&netif, &ipaddr , &netmask, &gw);
						App_Printf("DHCP timeout!\r\nStatic IP address:%d.%d.%d.%d\r\n",MyIP1,MyIP2,MyIP3,MyIP4);
						sysCfg.parameter.dhcp=STATIC;
						IP_STAUS=1;//IP��ַ������
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
 * ��������Task_TCP_Client
 * ����  ������TCP ����
 * ����  ��
 * ����  : 
*********************************************************************************************************
*/
void Task_TCP_Client(void *pvParameters)
{
	struct udp_pcb *udppcb; //UDP ͨ�ſ�
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
	memcpy(sysCfg.parameter.ip,(char*)&netif.ip_addr.addr,4);//��ȡ�����еı���ip��ַ
	memcpy(sysCfg.parameter.gw,(char*)&netif.gw.addr,4); //��ȡ�����е�����
	memcpy(sysCfg.parameter.sub,(char*)&netif.netmask.addr,4);//��ȡ�����е�����
	udp_pc_init();//��ʼ��UDP,��������λ��ͨѶ
	//udppcb=udp_ntp_init();//��ʼ��UDP������ntpͨ��
  app_tcp_init();//��ʼ��tcp
	while(1)
	{	
		/*������˿�����״̬*/

			 main_pcb = Check_TCP_Main_Connect();
			if(sysCfg.parameter.data_socket == SOCK_BUS)//�����˵�����������
			{
				bus_pcb=Check_TCP_Bus_Connect();
			}	
		if(MQTT_CONNECT == app_system_mqtt_connect_state_get(SOCK_MAIN))
		{
			app_system_NetLedToggle();//״ָ̬ʾ��
			//udp_senddata(udppcb);//����udp��	
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
			if(main_heart_flag==RECEIVE_HEART)        /*�յ�����������PING��Ӧ*/
			{
				main_heart_flag=RECEIVE_CLEAR;
				wait_main_socket_ack_time = 0;     /*����ȴ�ʱ��*/
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
			if(bus_heart_flag == RECEIVE_HEART)  	 /*�յ���������������PING��Ӧ*/
			{
				bus_heart_flag=RECEIVE_CLEAR;
				wait_bus_socket_ack_time = 0;      /*����ȴ�ʱ��*/
			}
		}
		vTaskDelay(1000);
	}
}
