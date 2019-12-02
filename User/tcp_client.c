/***********************************************************************
�ļ����ƣ�TCP_CLIENT.C
��    �ܣ����TCP�Ŀͻ��˵������շ�
��дʱ�䣺2013.4.25
�� д �ˣ�
ע    �⣺
***********************************************************************/
#include "main.h"
#include "lwip/dhcp.h"
extern  struct netif netif;
/**
  * @brief  ͨ��TCP��ʽ�������ݵ�TCP������
  * @param  buf �����׵�ַ
  * @param  buflen ���ݳ���
  * @retval С��0��ʾ����ʧ��
  */
err_t transport_sendPacketBuffer(struct tcp_pcb *cpcb,unsigned char* buf, int buflen,unsigned char socket)
{
  err_t err;
	err = tcp_write(cpcb,buf,buflen,TCP_WRITE_FLAG_COPY);	//������д������У�������������
	err=tcp_output(cpcb);
	return err;					
}
void mqtt_disconnect(struct tcp_pcb *cpcb)
{
	uint8_t len;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	
	len = MQTTSerialize_disconnect(buf,buflen);
	transport_sendPacketBuffer(cpcb, buf, len,0);		//���ط��͵ĳ���
	//if(sysCfg.parameter.data_socket == SOCK_BUS)
	//{
		//transport_sendPacketBuffer(SOCK_BUS, buf, len);
  //	}
}	

/**
  * @brief  ����
  * @param  ��
  * @param  ��
  * @retval 0 ��ʧ��  1:�ɹ�
  */
int Heartbeat(struct tcp_pcb *cpcb,uint8_t socket)
{
	uint8_t len,rc;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	len = MQTTSerialize_pingreq(buf,buflen);
	rc = transport_sendPacketBuffer(cpcb,buf, len,socket);//���ط��͵ĳ���
	if(rc != len)
	{
//		bsp_LedOff(2);     /*��ɫ*/
//		bsp_LedOff(3);     /*��ɫ*/
//		bsp_LedOff(4);     /*��ɫ*/
		#if APP_DEBUG
		printf("%d:Heartbeat fail\n\r",socket);
		#endif
		return 0;	
	}
	return 1;
}	
/**
  * @brief  ����MQTT������
  * @param  name:�û��� 
  * @param  word������
  * @retval ��
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
	rc = transport_sendPacketBuffer(cpcb,buf,len,socket);	//���ط��͵ĳ���
	
  if(rc !=ERR_OK ){
		#if APP_DEBUG
    printf("Connect:connect transport_sendPacketBuffer error code:%d\n\r",rc);
		#endif
		return -1;	
  }
	return rc;
}


/**
  * @brief  �����������������һ����Ϣ
  * @param  pTopic ��Ϣ����
  * @param  pMessage ��Ϣ����
  * @retval  0��ʾ����ʧ�� 1:���ͳɹ�
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
	
//�����Ҫ��������Ϣ�����رն˿ڣ���Ҫһ����ʱ QOS=1��������Ϣʱ���������˻᷵��PUBACK
//	 if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBACK) 	
//	{
//	
//	}
	
}
/**
  * @brief  �����������һ����Ϣ���ú�������ΪTCP�������ݺ���������
  * @param  pTopic ��Ϣ���⣬����
  * @retval 0��ʾ������Ϣʧ�ܣ�rc ���͵����ݳ���
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
 * ��������app_system_NetPublic
 * ����  �����Ӳ�������
 * ����  ����
 * ����  : ��
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
		
  /*startstatus: 1����Դ��λ 2�������λ 3���������Ź���λ 4�����ڿ��Ź���λ 5���͹��ĸ�λ*/	
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
	mqtt_publish(pcb,SOFTVERSION_TOPIC,netparm,strlen(netparm),0);//�ϱ��汾��-
}

/*��������*/
void app_system_TcpsStart(struct tcp_pcb *pcb)
{
	uint8_t mac[6];
	char dev[30] = {0};
	
	memcpy(mac,sysCfg.parameter.client_mac,6); 
	
	/*������������*/
	if(0== mqtt_subscrib(pcb,gateupdata_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",gateupdata_topic);
 		#endif  
	}	
				
	/*�����豸��Ϣ����*/
	if(0 == mqtt_subscrib(pcb,devcfgpara_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",devcfgpara_topic);
 		#endif  
	}	

	/*������������*/
	if(0 == mqtt_subscrib(pcb,gatereset_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",gatereset_topic);
 		#endif  
	}	

	/*����������豸��������*/
	if(0 == mqtt_subscrib(pcb,rrpc_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",rrpc_request_topic);
 		#endif  
	}	
	
	/*���������ظ������յ�������Ϣ����*/
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
 * ��������app_system_Subscrib
 * ����  �����ĵ���������
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void app_system_Subscrib(struct tcp_pcb *pcb)
{
	/*����������豸��������*/
	if(0 == mqtt_subscrib(pcb,rrpc_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",rrpc_request_topic);
 		#endif  
	}	
	
	/*���������ظ������յ�������Ϣ����*/
	if(0 == mqtt_subscrib(pcb,response_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",response_request_topic);
 		#endif  
	}		
}
/***********************************************************************
�������ƣ�Check_TCP_Main_Connect(void)
��    �ܣ�����������˿��������
���������
���������
��дʱ�䣺2013.4.25
�� д �ˣ�
ע    �⣺for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next) 
***********************************************************************/\
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
		TCP_Client_Init(SOCK_MAIN,SERVER_PORT,default_server_ip[0],default_server_ip[1],default_server_ip[2],default_server_ip[3]); //��������
		cpcb = 0;
	}
	return cpcb;	
}
/***********************************************************************
�������ƣ�Check_TCP_Bus_Connect(void)
��    �ܣ�������ݶ˿��������
���������
���������
��дʱ�䣺2013.4.25
�� д �ˣ�
ע    �⣺for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next) 
***********************************************************************/\
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
		TCP_Client_Init(SOCK_BUS,sysCfg.parameter.server_port,sysCfg.parameter.server_ip[0],sysCfg.parameter.server_ip[1],sysCfg.parameter.server_ip[2],sysCfg.parameter.server_ip[3]); //��������
		cpcb = 0;
	}
	return cpcb;	
}

/***********************************************************************
�������ƣ�err_t TCP_Connected(void *arg,struct tcp_pcb *pcb,err_t err)
��    �ܣ�����һ���ص���������TCP�ͻ�����������ӽ���ʱ������
***********************************************************************/
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
/***********************************************************************
�������ƣ�TCP_Client_Recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
��    �ܣ�tcp�ͻ��˽������ݻص������
ע    �⣺����һ���ص���������TCP��������������ʱ����
***********************************************************************/

err_t  TCP_Client_Recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
{
	#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4	
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
	  header.byte = buf[0]; //MQTTͷ��
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
/***********************************************************************
�������ƣ�TCP_Client_Init(u16_t local_port,u16_t remote_port,unsigned char a,unsigned char b,unsigned char c,unsigned char d)
��    �ܣ�tcp�ͻ��˳�ʼ��
���������local_port���ض˿ںţ�remote_port��Ŀ��˿ںţ�a,b,c,d��������ip
���������
��дʱ�䣺2013.4.25
�� д �ˣ�
ע    �⣺
***********************************************************************/
void TCP_Client_Init(u16_t local_port,u16_t remote_port,unsigned char a,unsigned char b,unsigned char c,unsigned char d)
{

	struct ip_addr ipaddr;
	err_t err;
	struct tcp_pcb *tcp_client_pcb;
	IP4_ADDR(&ipaddr,a,b,c,d);   //������IP��ַ
	tcp_client_pcb = tcp_new(); /* ����ͨ�ŵ�TCP���ƿ�(Clipcb) */
	if (!tcp_client_pcb)
	{
		return ;
	}		
	err = tcp_bind(tcp_client_pcb,IP_ADDR_ANY,local_port); /* �󶨱���IP��ַ�Ͷ˿ں� ������ip��ַ��LwIP_Init()���Ѿ���ʼ��*/
	if(err != ERR_OK)
	{
		return ;
	}    
	tcp_connect(tcp_client_pcb,&ipaddr,remote_port,TCP_Connected);//ע��ص�����
	tcp_recv(tcp_client_pcb,TCP_Client_Recv); 				/* ����tcp���ջص����� */
}
/*UDP ��Ϣ���ջص�������*/
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
				IPaddress = netif.ip_addr.addr;//��ȡ�µ�IP��ַ
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
/***********************************************************************
�������ƣ�void Task_TCP_Client(void *pdata)
��    �ܣ�
***********************************************************************/
void Task_TCP_Client(void *pvParameters)
{
	struct tcp_pcb *main_pcb; //TCP ͨ�ſ�
	struct tcp_pcb *bus_pcb; //TCP ͨ�ſ�
	struct udp_pcb *udppcb; //UDP ͨ�ſ�
	struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
	static  uint16_t wait_bus_socket_ack_time = 0;
  static  uint16_t wait_main_socket_ack_time = 0;
	static uint8_t check_count=0;
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
	memcpy(sysCfg.parameter.ip,(char*)&netif.ip_addr.addr,4);//��ȡ�����еı���ip��ַ
	udppcb=udp_app_init();//��ʼ��UDP������ntpͨ��
  app_tcp_init();//��ʼ��tcp
	while(1)
	{	
		/*������˿�����״̬*/
		main_pcb = Check_TCP_Main_Connect();
		if(main_pcb==0)
		{
			check_count++;
			if(check_count>20)
			{
				check_count=0;
				App_Printf("Fail to connect Main_Socket...\r\n");
				NVIC_SystemReset();		
			}		
		}
    if(sysCfg.parameter.data_socket == SOCK_BUS)//�����˵�����������
		{
			bus_pcb=Check_TCP_Bus_Connect();
		}		
		if(MQTT_CONNECT == app_system_mqtt_connect_state_get(SOCK_MAIN))
		{
			app_system_NetLedToggle();//״ָ̬ʾ��
			udp_senddata(udppcb);//����udp��	
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
