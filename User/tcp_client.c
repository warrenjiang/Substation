/***********************************************************************
ÎÄ¼þÃû³Æ£ºTCP_CLIENT.C
¹¦    ÄÜ£ºÍê³ÉTCPµÄ¿Í»§¶ËµÄÊý¾ÝÊÕ·¢
±àÐ´Ê±¼ä£º2013.4.25
±à Ð´ ÈË£º
×¢    Òâ£º
***********************************************************************/
#include "main.h"
#include "lwip/dhcp.h"
extern  struct netif netif;
/**
  * @brief  Í¨¹ýTCP·½Ê½·¢ËÍÊý¾Ýµ½TCP·þÎñÆ÷
  * @param  buf Êý¾ÝÊ×µØÖ·
  * @param  buflen Êý¾Ý³¤¶È
  * @retval Ð¡ÓÚ0±íÊ¾·¢ËÍÊ§°Ü
  */
err_t transport_sendPacketBuffer(struct tcp_pcb *cpcb,unsigned char* buf, int buflen,unsigned char socket)
{
  err_t err;
	err = tcp_write(cpcb,buf,buflen,TCP_WRITE_FLAG_COPY);	//½«Êý¾ÝÐ´Èë¶ÓÁÐÖÐ£¬²»»áÁ¢¼´·¢ËÍ
	err=tcp_output(cpcb);
	return err;					
}
void mqtt_disconnect(struct tcp_pcb *cpcb)
{
	uint8_t len;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	
	len = MQTTSerialize_disconnect(buf,buflen);
	transport_sendPacketBuffer(cpcb, buf, len,0);		//·µ»Ø·¢ËÍµÄ³¤¶È
	//if(sysCfg.parameter.data_socket == SOCK_BUS)
	//{
		//transport_sendPacketBuffer(SOCK_BUS, buf, len);
  //	}
}	

/**
  * @brief  ÐÄÌø
  * @param  ÎÞ
  * @param  ÎÞ
  * @retval 0 £ºÊ§°Ü  1:³É¹¦
  */
int Heartbeat(struct tcp_pcb *cpcb,uint8_t socket)
{
	uint8_t len,rc;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	len = MQTTSerialize_pingreq(buf,buflen);
	rc = transport_sendPacketBuffer(cpcb,buf, len,socket);//·µ»Ø·¢ËÍµÄ³¤¶È
	if(rc != len)
	{
//		bsp_LedOff(2);     /*À¶É«*/
//		bsp_LedOff(3);     /*ÂÌÉ«*/
//		bsp_LedOff(4);     /*ºìÉ«*/
		#if APP_DEBUG
		printf("%d:Heartbeat fail\n\r",socket);
		#endif
		return 0;	
	}
	return 1;
}	
/**
  * @brief  Á¬½ÓMQTT·þÎñÆ÷
  * @param  name:ÓÃ»§Ãû 
  * @param  word£ºÃÜÂë
  * @retval ÎÞ
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
	rc = transport_sendPacketBuffer(cpcb,buf,len,socket);	//·µ»Ø·¢ËÍµÄ³¤¶È
	
  if(rc !=ERR_OK ){
		#if APP_DEBUG
    printf("Connect:connect transport_sendPacketBuffer error code:%d\n\r",rc);
		#endif
		return -1;	
  }
	return rc;
}


/**
  * @brief  Ïò´úÀí£¨·þÎñÆ÷£©·¢ËÍÒ»¸öÏûÏ¢
  * @param  pTopic ÏûÏ¢Ö÷Ìâ
  * @param  pMessage ÏûÏ¢ÄÚÈÝ
  * @retval  0±íÊ¾·¢ËÍÊ§°Ü 1:·¢ËÍ³É¹¦
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
	
//Èç¹ûÐèÒª·¢ËÍÍêÏûÏ¢£¬¼´¹Ø±Õ¶Ë¿Ú£¬ÐèÒªÒ»¶¨ÑÓÊ± QOS=1£¬·¢²¼ÏûÏ¢Ê±£¬·þÎñÆ÷¶Ë»á·µ»ØPUBACK
//	 if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBACK) 	
//	{
//	
//	}
	
}
/**
  * @brief  Ïò·þÎñÆ÷¶©ÔÄÒ»¸öÏûÏ¢£¬¸Ãº¯Êý»áÒòÎªTCP½ÓÊÕÊý¾Ýº¯Êý¶ø×èÈû
  * @param  pTopic ÏûÏ¢Ö÷Ìâ£¬´«Èë
  * @retval 0±íÊ¾¶©ÔÄÏûÏ¢Ê§°Ü£¬rc ·¢ËÍµÄÊý¾Ý³¤¶È
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
 * º¯ÊýÃû£ºapp_system_NetPublic
 * ÃèÊö  £ºÁ¬½Ó²ÎÊý·¢²¼
 * ÊäÈë  £ºÎÞ
 * ·µ»Ø  : ÎÞ
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
		
  /*startstatus: 1£ºµçÔ´¸´Î» 2£ºÈí¼þ¸´Î» 3£º¶ÀÁ¢¿´ÃÅ¹·¸´Î» 4£º´°¿Ú¿´ÃÅ¹·¸´Î» 5£ºµÍ¹¦ºÄ¸´Î»*/	
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
	mqtt_publish(pcb,SOFTVERSION_TOPIC,netparm,strlen(netparm),0);//ÉÏ±¨°æ±¾ºÅ-
}

/*¶©ÔÄÖ÷Ìâ*/
void app_system_TcpsStart(struct tcp_pcb *pcb)
{
	uint8_t mac[6];
	char dev[30] = {0};
	
	memcpy(mac,sysCfg.parameter.client_mac,6); 
	
	/*Ö÷¿ØÉý¼¶Ö÷Ìâ*/
	if(0== mqtt_subscrib(pcb,gateupdata_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",gateupdata_topic);
 		#endif  
	}	
				
	/*ÅäÖÃÉè±¸ÐÅÏ¢Ö÷Ìâ*/
	if(0 == mqtt_subscrib(pcb,devcfgpara_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",devcfgpara_topic);
 		#endif  
	}	

	/*Íø¹ØÖØÆôÖ÷Ìâ*/
	if(0 == mqtt_subscrib(pcb,gatereset_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",gatereset_topic);
 		#endif  
	}	

	/*·þÎñÆ÷¼ì²âÉè±¸ÔÚÏßÖ÷Ìâ*/
	if(0 == mqtt_subscrib(pcb,rrpc_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",rrpc_request_topic);
 		#endif  
	}	
	
	/*·þÎñÆ÷·µ»Ø¸øÍø¹ØÊÕµ½ÐÄÌøÐÅÏ¢Ö÷Ìâ*/
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
 * º¯ÊýÃû£ºapp_system_Subscrib
 * ÃèÊö  £º¶©ÔÄµÚÈý·½Ö÷Ìâ
 * ÊäÈë  £ºÎÞ
 * ·µ»Ø  : ÎÞ
*********************************************************************************************************
*/
void app_system_Subscrib(struct tcp_pcb *pcb)
{
	/*·þÎñÆ÷¼ì²âÉè±¸ÔÚÏßÖ÷Ìâ*/
	if(0 == mqtt_subscrib(pcb,rrpc_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",rrpc_request_topic);
 		#endif  
	}	
	
	/*·þÎñÆ÷·µ»Ø¸øÍø¹ØÊÕµ½ÐÄÌøÐÅÏ¢Ö÷Ìâ*/
	if(0 == mqtt_subscrib(pcb,response_request_topic,0,0))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",response_request_topic);
 		#endif  
	}		
}
/***********************************************************************
º¯ÊýÃû³Æ£ºCheck_TCP_Main_Connect(void)
¹¦    ÄÜ£º¼ì²éÁ¬½ÓÖ÷¶Ë¿ÚÁ¬½ÓÇé¿ö
ÊäÈë²ÎÊý£º
Êä³ö²ÎÊý£º
±àÐ´Ê±¼ä£º2013.4.25
±à Ð´ ÈË£º
×¢    Òâ£ºfor(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next) 
***********************************************************************/\
struct tcp_pcb *Check_TCP_Main_Connect(void)
{
	struct tcp_pcb *cpcb = 0;
	static unsigned char Main_connect_flag = 0;
	Main_connect_flag = 0;
	for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next)
	{
		if(cpcb->local_port == SOCK_MAIN && cpcb -> state == ESTABLISHED)//CLOSED		//Èç¹ûTCP_LOCAL_PORT¶Ë¿ÚÖ¸¶¨µÄÁ¬½ÓÃ»ÓÐ¶Ï¿ª
		{
			Main_connect_flag = 1;  						//Á¬½Ó±êÖ¾		
      break;			
		}
	}

	if(Main_connect_flag == 0)  	// TCP_LOCAL_PORTÖ¸¶¨µÄ¶Ë¿ÚÎ´Á¬½Ó»òÒÑ¶Ï¿ª
	{
		TCP_Client_Init(SOCK_MAIN,SERVER_PORT,default_server_ip[0],default_server_ip[1],default_server_ip[2],default_server_ip[3]); //ÖØÐÂÁ¬½Ó
		cpcb = 0;
	}
	return cpcb;	
}
/***********************************************************************
º¯ÊýÃû³Æ£ºCheck_TCP_Bus_Connect(void)
¹¦    ÄÜ£º¼ì²éÊý¾Ý¶Ë¿ÚÁ¬½ÓÇé¿ö
ÊäÈë²ÎÊý£º
Êä³ö²ÎÊý£º
±àÐ´Ê±¼ä£º2013.4.25
±à Ð´ ÈË£º
×¢    Òâ£ºfor(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next) 
***********************************************************************/\
struct tcp_pcb *Check_TCP_Bus_Connect(void)
{
	struct tcp_pcb *cpcb = 0;
	static unsigned char Bus_connect_flag = 0;
	Bus_connect_flag = 0;
	for(cpcb = tcp_active_pcbs;cpcb != NULL; cpcb = cpcb->next)
	{
		if(cpcb->local_port == SOCK_BUS && cpcb -> state == ESTABLISHED)//CLOSED		//Èç¹ûTCP_LOCAL_PORT¶Ë¿ÚÖ¸¶¨µÄÁ¬½ÓÃ»ÓÐ¶Ï¿ª
		{
			Bus_connect_flag = 1;  						//Á¬½Ó±êÖ¾
			 
			break;							   	
		}
	}

	if(Bus_connect_flag == 0)  	// TCP_LOCAL_PORTÖ¸¶¨µÄ¶Ë¿ÚÎ´Á¬½Ó»òÒÑ¶Ï¿ª
	{
		TCP_Client_Init(SOCK_BUS,sysCfg.parameter.server_port,sysCfg.parameter.server_ip[0],sysCfg.parameter.server_ip[1],sysCfg.parameter.server_ip[2],sysCfg.parameter.server_ip[3]); //ÖØÐÂÁ¬½Ó
		cpcb = 0;
	}
	return cpcb;	
}

/***********************************************************************
º¯ÊýÃû³Æ£ºerr_t TCP_Connected(void *arg,struct tcp_pcb *pcb,err_t err)
¹¦    ÄÜ£ºÕâÊÇÒ»¸ö»Øµ÷º¯Êý£¬µ±TCP¿Í»§¶ËÇëÇóµÄÁ¬½Ó½¨Á¢Ê±±»µ÷ÓÃ
***********************************************************************/
err_t TCP_Connected(void *arg,struct tcp_pcb *pcb,err_t err)
{
	if(pcb->local_port==SOCK_MAIN)
	{
  mqtt_connect(pcb, default_server_user, default_server_pass,0);
	app_system_NetPublic(pcb);//ÉÏµçÐÅÏ¢
	app_system_TcpsStart(pcb);//¶©ÔÄÖ÷Ìâ
	}
	if(pcb->local_port==SOCK_BUS)
	{
		mqtt_connect(pcb, (char *)sysCfg.parameter.server_user, (char *)sysCfg.parameter.server_pass,0);
		app_system_Subscrib(pcb);//¶©ÔÄµÚÈý·½Ö÷Ìâ
		mqtt_subscrib(pcb,"test",0,0);
	}
	return ERR_OK;
}
/***********************************************************************
º¯ÊýÃû³Æ£ºTCP_Client_Recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
¹¦    ÄÜ£ºtcp¿Í»§¶Ë½ÓÊÕÊý¾Ý»Øµ÷º¯Êýº
×¢    Òâ£ºÕâÊÇÒ»¸ö»Øµ÷º¯Êý£¬µ±TCP·þÎñÆ÷·¢À´Êý¾ÝÊ±µ÷ÓÃ
***********************************************************************/

err_t  TCP_Client_Recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
{
	#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4	
	struct pbuf *p_temp;
  MQTTHeader header = {0};//MQTT Í·²¿
	int len = 0;
	uint8_t buf[512]; //½ÓÊÕbuf
	vTaskSuspendAll();//¿ªÆôÈÎÎñËø£¬·ÀÖ¹µ÷¶È
	memset(buf,0,sizeof(buf));
  p_temp = p;//TCP½ÓÊÕÊý¾Ý£¬Á´±í½á¹¹
	if(p_temp != NULL)
	{	
		tcp_recved(pcb, p_temp->tot_len);//»ñÈ¡Êý¾Ý³¤¶È tot_len£ºtcpÊý¾Ý¿éµÄ³¤¶È
		#if 0
		App_Printf("p_temp->tot_len is %d\r\n",p_temp->tot_len);//µ±Ç°Êý¾Ý×Ü³¤¶È
 		App_Printf("p_temp->len is %d\r\n",p_temp->len);//µ±Ç°¿éÖÐÊý¾Ý³¤¶È
		#endif
		/*½«²»Á¬ÐøµÄÁ´±íµØÖ· ¿½±´µ½Á¬ÐøµÄbufÖÐ ·½±ãMQTT½âÎö£¬·çÏÕÎ´Öª*/
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
	  header.byte = buf[0]; //MQTTÍ·²¿
		switch(header.bits.type)
		{
			case PUBLISH:   /*½ÓÊÕ¶©ÔÄÖ÷ÌâÊý¾Ý*/
				app_palt_Receicedata(buf,pcb,pcb->local_port);
				break;	
			case CONNACK:   /*Á¬½Ó»ØÖ´*/
				 app_palt_Connectack(buf,pcb->local_port);
				break;		
			case PINGRESP:  /*µ½·þÎñÆ÷ping»ØÖ´*/
				break;
			
			default:
				break;
		}	  		
	}
	else
	{
		tcp_close(pcb); 											/* ×÷ÎªTCP·þÎñÆ÷²»Ó¦Ö÷¶¯¹Ø±ÕÕâ¸öÁ¬½Ó£¿ */
	}
	/* ÊÍ·Å¸ÃTCP¶Î */
	pbuf_free(p); 	
	err = ERR_OK;
	xTaskResumeAll();//»Ö¸´ÈÎÎñµ÷¶È
	return err;
}
/***********************************************************************
º¯ÊýÃû³Æ£ºTCP_Client_Init(u16_t local_port,u16_t remote_port,unsigned char a,unsigned char b,unsigned char c,unsigned char d)
¹¦    ÄÜ£ºtcp¿Í»§¶Ë³õÊ¼»¯
ÊäÈë²ÎÊý£ºlocal_port±¾µØ¶Ë¿ÚºÅ£»remote_port£ºÄ¿±ê¶Ë¿ÚºÅ£»a,b,c,d£º·þÎñÆ÷ip
Êä³ö²ÎÊý£º
±àÐ´Ê±¼ä£º2013.4.25
±à Ð´ ÈË£º
×¢    Òâ£º
***********************************************************************/
void TCP_Client_Init(u16_t local_port,u16_t remote_port,unsigned char a,unsigned char b,unsigned char c,unsigned char d)
{

	struct ip_addr ipaddr;
	err_t err;
	struct tcp_pcb *tcp_client_pcb;
	IP4_ADDR(&ipaddr,a,b,c,d);   //·þÎñÆ÷IPµØÖ·
	tcp_client_pcb = tcp_new(); /* ½¨Á¢Í¨ÐÅµÄTCP¿ØÖÆ¿é(Clipcb) */
	if (!tcp_client_pcb)
	{
		return ;
	}		
	err = tcp_bind(tcp_client_pcb,IP_ADDR_ANY,local_port); /* °ó¶¨±¾µØIPµØÖ·ºÍ¶Ë¿ÚºÅ £¬±¾µØipµØÖ·ÔÚLwIP_Init()ÖÐÒÑ¾­³õÊ¼»¯*/
	if(err != ERR_OK)
	{
		return ;
	}    
	tcp_connect(tcp_client_pcb,&ipaddr,remote_port,TCP_Connected);//×¢²á»Øµ÷º¯Êý
	tcp_recv(tcp_client_pcb,TCP_Client_Recv); 				/* ÉèÖÃtcp½ÓÊÕ»Øµ÷º¯Êý */
}
/*UDP ÏûÏ¢½ÓÊÕ»Øµ÷´¦Àíº¯Êý*/
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
				IPaddress = netif.ip_addr.addr;//¶ÁÈ¡ÐÂµÄIPµØÖ·
				if (IPaddress!=0) 
				{
					DHCP_state = DHCP_ADDRESS_ASSIGNED;					
					dhcp_stop(&netif);//¹Ø±ÕDHCP  
					App_Printf("IP address assigned by a DHCP server :%d.%d.%d.%d\r\n",(uint8_t)(IPaddress),(uint8_t)(IPaddress >> 8),(uint8_t)(IPaddress >> 16),(uint8_t)(IPaddress >> 24));
					IP_STAUS=1;//IPµØÖ·¾ÍÐ÷ºó
				}
				else
				{          
					if (netif.dhcp->tries > MAX_DHCP_TRIES)//³¬Ê±Î´»ñµÃIPµØÖ·
					{
						DHCP_state = DHCP_TIMEOUT;
					 
						dhcp_stop(&netif);//¹Ø±ÕDHCP
						/*Ê¹ÓÃÄ¬ÈÏ¾²Ì¬IP*/
						IP4_ADDR(&ipaddr, MyIP1 ,MyIP2 , MyIP3 ,MyIP4 );
						IP4_ADDR(&netmask, 255, 255, 255, 0);
						IP4_ADDR(&gw, 192, 168, 0, 1);
						netif_set_addr(&netif, &ipaddr , &netmask, &gw);
						App_Printf("DHCP timeout!\r\nStatic IP address:%d.%d.%d.%d\r\n",MyIP1,MyIP2,MyIP3,MyIP4);
						IP_STAUS=1;//IPµØÖ·¾ÍÐ÷ºó
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
º¯ÊýÃû³Æ£ºvoid Task_TCP_Client(void *pdata)
¹¦    ÄÜ£º
***********************************************************************/
void Task_TCP_Client(void *pvParameters)
{
	struct tcp_pcb *main_pcb; //TCP Í¨ÐÅ¿é
	struct tcp_pcb *bus_pcb; //TCP Í¨ÐÅ¿é
	struct udp_pcb *udppcb; //UDP Í¨ÐÅ¿é
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
	memcpy(sysCfg.parameter.ip,(char*)&netif.ip_addr.addr,4);//»ñÈ¡Íø¿¨ÖÐµÄ±¾µØipµØÖ·
	udppcb=udp_app_init();//³õÊ¼»¯UDP£¬ÓÃÓÚntpÍ¨ÐÅ
  app_tcp_init();//³õÊ¼»¯tcp
	while(1)
	{	
		/*¼ì²âÖ÷¶Ë¿ÚÁ¬½Ó×´Ì¬*/
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
    if(sysCfg.parameter.data_socket == SOCK_BUS)//ÅäÖÃÁËµÚÈý·½·þÎñÆ÷
		{
			bus_pcb=Check_TCP_Bus_Connect();
		}		
		if(MQTT_CONNECT == app_system_mqtt_connect_state_get(SOCK_MAIN))
		{
			app_system_NetLedToggle();//×´Ì¬Ö¸Ê¾µÆ
			udp_senddata(udppcb);//·¢ËÍudp°ü	
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
			if(main_heart_flag==RECEIVE_HEART)        /*ÊÕµ½Ö÷·þÎñÆ÷µÄPINGÏìÓ¦*/
			{
				main_heart_flag=RECEIVE_CLEAR;
				wait_main_socket_ack_time = 0;     /*ÇåÁãµÈ´ýÊ±¼ä*/
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
			if(bus_heart_flag == RECEIVE_HEART)  	 /*ÊÕµ½µÚÈý·½·þÎñÆ÷µÄPINGÏìÓ¦*/
			{
				bus_heart_flag=RECEIVE_CLEAR;
				wait_bus_socket_ack_time = 0;      /*ÇåÁãµÈ´ýÊ±¼ä*/
			}
		}
		vTaskDelay(1000);
	}
}
