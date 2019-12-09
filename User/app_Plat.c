#include "main.h"

/****************************��������******************************/

uint8_t main_heart_flag=0;
uint8_t bus_heart_flag=0;

char devcfgpara_topic[]="/devid_0000000000000000/devcfgpara/set";
char gatereset_topic[]="/devid_0000000000000000/reset/gateway";
char gateupdata_topic[]="/devid_0000000000000000/update/gateway";
char rrpc_request_topic[]="/sys/0000000000000000/rrpc/request/state";
char response_request_topic[]="/sys/0000000000000000/rrpc/request/response";

/*
*********************************************************************************************************
 * ��������app_system_connect_state_flag_set
 * ����  ����ȡϵͳ����״̬
 * ����  ��socket��SOCK_MAIN = 50000��SOCK_BUS = 60000
 * ˵��  : sysCfg.parameter.connect_state��0x00������socket��δ���ӣ�0x01������socket0��0x03������socket1
 * ����  : MQTT_CONNECT,���� MQTT_DISCONNECT,δ���� 
*********************************************************************************************************
 */
MQTT_CONNECT_STATE app_system_mqtt_connect_state_get(uint16_t socket)
{
		if(socket==SOCK_MAIN)
		socket=0;
	  else if(socket==SOCK_BUS)
		socket=1;
	return((sysCfg.parameter.connect_state & (1 << socket)) ? MQTT_CONNECT : MQTT_DISCONNECT);
}

/*
*********************************************************************************************************
 * ��������app_system_connect_state_flag_set
 * ����  ������ϵͳ����״̬��־
 * ����  ��socket��SOCK_MAIN = 50000��SOCK_BUS =60000
 *       ��connect_state��
 * ˵��  : sysCfg.parameter.connect_state��0x00������socket��δ���ӣ�0x01������socket0��0x03������socket1
 * ����  : �� 
*********************************************************************************************************
 */
void app_system_mqtt_connect_state_flag_set(uint16_t socket,MQTT_CONNECT_STATE connect_state)
{
	if(socket==SOCK_MAIN)
		socket=0;
	else if(socket==SOCK_BUS)
		socket=1;
	
	if(connect_state == MQTT_CONNECT)
	{
		sysCfg.parameter.connect_state = ~sysCfg.parameter.connect_state;
		sysCfg.parameter.connect_state &= ~(1 << ((socket ? BUS_MQTT_CONNECT_FLAG_BIT : MAIN_MQTT_CONNECT_FLAG_BIT)));
		sysCfg.parameter.connect_state = ~sysCfg.parameter.connect_state;
	}
	else
	{
		sysCfg.parameter.connect_state &= ~(1 << ((socket ? BUS_MQTT_CONNECT_FLAG_BIT : MAIN_MQTT_CONNECT_FLAG_BIT)));
	}
	
	//printf("%02X \r\n",sysCfg.parameter.connect_state);	
}

/*
*********************************************************************************************************
 * ��������app_system_NetLedToggle
 * ����  ����������״ָ̬ʾ
 * ����  ����
 * ����  : ��
*********************************************************************************************************
 */
void app_system_NetLedToggle(void)
{	
	switch(sysCfg.parameter.connect_state)
	{
		case 0x01:
			/*�����˵����������� ��ֻ���������������� ����û���� ����*/
			if((sysCfg.parameter.data_socket != SOCK_MAIN) && (sysCfg.parameter.key_time_count == 0)) 
			{
				bsp_LedOff(LED_BLUE);
				bsp_LedOff(LED_GREEN);
			  bsp_LedToggle(LED_RED);
			}
			else if(sysCfg.parameter.data_socket == SOCK_MAIN) /*δ�����˵����������� ���������������� ����*/
			{
				if(sysCfg.parameter.key_time_count == 0)		
				{
					bsp_LedOff(LED_RED);
				}
				bsp_LedOff(LED_GREEN);
				bsp_LedToggle(LED_BLUE);	
			}
			break;
		
		case 0x03:  /*�����������������͵����������� ����*/
			if(sysCfg.parameter.key_time_count == 0)		
			{
				bsp_LedOff(LED_RED);
			}
			bsp_LedOff(LED_BLUE);	
			bsp_LedToggle(LED_GREEN);	
			break;
		
		default:
			break;
	}
}
/*
*********************************************************************************************************
 * ��������app_tcp_init
 * ����  ����ʼ��TCP����
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void app_tcp_init(void)
{
	/*������������*/
	TCP_Client_Init(SOCK_MAIN,SERVER_PORT,default_server_ip[0],default_server_ip[1],default_server_ip[2],default_server_ip[3]);
	if(sysCfg.parameter.data_socket == SOCK_MAIN)
	return;
  /*���ӵ�����������*/
	TCP_Client_Init(SOCK_BUS,sysCfg.parameter.server_port,sysCfg.parameter.server_ip[0],sysCfg.parameter.server_ip[1],sysCfg.parameter.server_ip[2],sysCfg.parameter.server_ip[3]); //��������
	
}
/*
*********************************************************************************************************
 * ��������app_system_LedOn
 * ����  ������ָʾ
 * ����  ����
 * ����  : ��
*********************************************************************************************************
 */
void app_system_LedOn(void)
{	
	if(sysCfg.parameter.data_socket == SOCK_MAIN)
	{
		bsp_LedOff(LED_GREEN);	
		bsp_LedOn(LED_BLUE);				
	}
	else
	{
		bsp_LedOff(LED_BLUE);	
		bsp_LedOn(LED_GREEN);	
	}	
}

/*
*********************************************************************************************************
 * ��������app_system_CheckID
 * ����  �����ڲ�ѯ�豸ID
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
//void app_system_CheckID(void)
//{
//	uint8_t i = 0;
//	char recebuf[100];
//	
//	/*���˿�����*/
//	while(comGetChar(COM1,(uint8_t *)&recebuf[i++]))
//  {
//		i %= 100;
//	}
//	recebuf[i] = '\0';
//	
//	if(NULL != strstr(recebuf,"ID=?"))
//	{
//		if(sysCfg.parameter.register_flag == NRF_REGISTER)
//		{
//			printf("ID=0000%02X%02X%02X%02X%02X%02X:0000%02X%02X%02X%02X%02X%02X:\r\n",sysCfg.parameter.client_mac[0],
//			sysCfg.parameter.client_mac[1],sysCfg.parameter.client_mac[2],sysCfg.parameter.client_mac[3],sysCfg.parameter.client_mac[4],
//			sysCfg.parameter.client_mac[5],sysCfg.parameter.nrfstation[2],sysCfg.parameter.nrfstation[3],sysCfg.parameter.nrfstation[4],
//			sysCfg.parameter.nrfstation[5],sysCfg.parameter.nrfstation[6],sysCfg.parameter.nrfstation[7]);
//		}
//	}
//}
/*
*********************************************************************************************************
 * ��������app_plat_GetRespone
 * ����  ����ȡ��Ϣ����
 * ����  ��topic,��Ϣ����
 *			 : pRespone�������Ϣ����
 * ����  : 
 *       ��SUCCESS,�Ǵ��������Ϣ���ͣ�
 *       ��ERROR�����Ǵ��������Ϣ����
*********************************************************************************************************
*/
static ErrorStatus app_plat_GetRespone(uint8_t *TopicInfo,char *topic,app_plat_topic* pRespone)
{ 
	#if 0
	App_Printf("Receive topic:%s\r\n",topic);
	#endif
	
		/*����*/
	 if( strstr( topic, "/rrpc/request/state"))
	{
		*pRespone = PLAT_RESPONSE_RRPC;
		return SUCCESS;
	}
	/*�����豸����*/
	 else if( strstr( topic, "/update/gateway"))
	{
		*pRespone = PLAT_RESPONSE_DEVUPDATE;
		return SUCCESS;
	}

	/*�����豸��������*/
	else if( strstr( topic, "/devcfgpara/set"))
	{
		*pRespone = PLAT_RESPONSE_DEVCFGSET;
		return SUCCESS;
	}

	/*����Ӳ����λ*/
	else if( strstr( topic, "/reset/gateway"))
	{
		*pRespone = PLAT_RESPONSE_RESET;
		return SUCCESS;
	}
	
	else
	{
		return ERROR;
	}
}


/*
*********************************************************************************************************
 * ��������app_palt_devcfgset
 * ����  �����ӵڵ�������������Ϣ����
 * ����  ��
 *       : TopicInfo,������Ϣ
 *       : len,������Ϣ
 * ����  : ��
*********************************************************************************************************
*/
static void app_palt_devcfgset(uint8_t *TopicInfo , uint16_t len )
{
	uint16_t usTemp;
	uint32_t _ulFlashAddr;
	
	uint8_t err_code = 0;
	
	if(TopicInfo[9] > sizeof(sysCfg.parameter.server_user) || TopicInfo[10+TopicInfo[9]] > sizeof(sysCfg.parameter.server_pass))
	{
		#if APP_DEBUG   
		App_Printf("\r\nset parameter over limit!\r\n");
		#endif
		return;			
	}
		
	memcpy(sysCfg.parameter.server_ip,&TopicInfo[3],4);
	memcpy((uint8_t *)&sysCfg.parameter.server_port,&TopicInfo[7],2);
	
	memcpy(sysCfg.parameter.server_user,&TopicInfo[10],TopicInfo[9]);
	memcpy(sysCfg.parameter.server_pass,&TopicInfo[11+TopicInfo[9]],TopicInfo[10+TopicInfo[9]]);

	sysCfg.parameter.server_user[TopicInfo[9]] = '\0';
	sysCfg.parameter.server_pass[TopicInfo[10+TopicInfo[9]]] = '\0';
	
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
	
	App_Printf("server username:%s\r\n",sysCfg.parameter.server_user);
	App_Printf("server password:%s\r\n",sysCfg.parameter.server_pass);
	#endif
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
  err_code = FLASH_ErasePage( SYSCFG_ADDR & 0xFFFFFFFF);     
	if(err_code != 4)       /*����ʧ��*/
	{
		#if APP_DEBUG
		App_Printf("erase flash error\r\n");
		#endif	
	}
	
	_ulFlashAddr = SYSCFG_ADDR;
	
	for (uint8_t i = 0; i < SYSCFG_DATA_LEN / 2; i++)
	{	
		usTemp = sysCfg.data[2 * i];
		usTemp |= (sysCfg.data[2 * i + 1] << 8);
		err_code = FLASH_ProgramHalfWord(_ulFlashAddr, usTemp);
		if (err_code != FLASH_COMPLETE)
		{
			break;
		}
		
		_ulFlashAddr += 2;
	}
	
	if (err_code == FLASH_COMPLETE)
	{
		//mqtt_disconnect();
		vTaskDelay(1000);
		NVIC_SystemReset();
	}
	
	FLASH_Lock();
	
	#if APP_DEBUG
	printf("write flash error\r\n");
	#endif
}

/*
*********************************************************************************************************
 * ��������app_palt_reset
 * ����  ���豸����
 * ����  ��TopicInfo,������Ϣ
 *			 : len�����ⳤ��
 * ����  : ��
*********************************************************************************************************
*/
static void app_palt_reset(uint8_t *TopicInfo , uint16_t len )
{
	#ifdef MCBR03
	uint16_t crcdata;
	uint8_t deal_result;
	uint8_t topicdata[18]={0x0e,0x00,0x4B};

	if(TopicInfo[3] != 0x01 || TopicInfo[2] != 0xB4)
	{
		return;
	}
	
	deal_result = 0x01; 
  memset(&topicdata[3],0,2);                            /*�����豸ID���ֽڲ���*/
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);  /*����ID*/
	memcpy(&topicdata[11],&deal_result,1);  							/*����ɹ�*/
  memcpy(&topicdata[12],&TopicInfo[len-6],4);           /**/

	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[16],(uint8_t *)&crcdata,2);  
	
	/*�ϱ�����*/
  if(mqtt_publish( SOCK_MAIN, RESETACK_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
		
	}	
	
	vTaskDelay(1000);
	
	POW_RESET();
	#endif
}
/*
*********************************************************************************************************
 * ��������app_system_Heart
 * ����  ����������
 * ����  ��socket���������Ͷ˿�
 * ����  : ��
*********************************************************************************************************
 */
void app_system_Heart(struct tcp_pcb *pcb,uint16_t socket)
{
	uint8_t mac[6];
	DateTime nowtime;
	uint32_t nowsec=0;
	char dev[17] = {0};
	char heart[50] = {0};
	char rrpc_ack_topic[100] = {0};
	
	get_ntp_time(&nowtime);
	nowsec = app_nrf_TimeTosec(nowtime.time.year[0]+nowtime.time.year[1]*256,nowtime.time.month,
	nowtime.time.day,nowtime.time.hour,nowtime.time.minute,nowtime.time.second);
	
	sprintf(rrpc_ack_topic,"/json%s/ack",rrpc_request_topic);
	memcpy(mac,sysCfg.parameter.client_mac,6); 
	sprintf(dev,"0000%02x%02x%02x%02x%02x%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	
	sprintf(heart,"{\"id\":\"%s\",\"time\":\"%d\"}",dev,nowsec);
	
	#if APP_DEBUG
	App_Printf("send heart %d\r\n",socket);
	#endif
	mqtt_publish(pcb,rrpc_ack_topic,heart,strlen(heart),socket);	//������������
}
/*
*********************************************************************************************************
 * ��������app_palt_Receicedata
 * ����  ������ƽ̨����
 * ����  ��plat_report_t,ƽ̨��Ϣ�ṹ��
 * ����  : ��
*********************************************************************************************************
*/
void app_palt_Receicedata(uint8_t * buff,struct tcp_pcb *pcb,uint16_t socket)
{
  uint8_t  len;
	uint8_t  dup;					//�ظ���־
  uint8_t  retained;		//������־
  uint8_t  *payload_in; //��������
  uint8_t  ack[30];			//��ִ
	uint16_t msgid;				//��ϢID
  uint16_t crcdata; 
	int qos;							//��Ϣ����
	int payloadlen_in;		//���س���
	MQTTString receivedTopic;
	app_plat_topic nState;
	
	/*������Ϣ����*/
	MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
	&payload_in, &payloadlen_in, buff , PLAT_MAX_DATA_LEN,socket);				
  
	/*��Ϣ��ִ ���ĵ�QOS=1��Ҫ��ִ*/
	if(1==qos)
	{
	len = MQTTSerialize_puback(ack, sizeof(ack),msgid);
	transport_sendPacketBuffer(pcb,ack,len,0);
	}
	/*����������ʼָ��*/
	if(0 == strncmp(receivedTopic.lenstring.data,gateupdata_topic,strlen(gateupdata_topic))
	&&(0x01 == payload_in[3]+payload_in[4]*256))
	{
		//report_t->gateway_updata_flag = GAT_UPDATA_START;
	}
	
	/*���յ�����������*/
	if(0 == strncmp(receivedTopic.lenstring.data,response_request_topic,strlen(response_request_topic)))
	{
		if(pcb->local_port==SOCK_MAIN)
		{
		  App_Printf("Receive SOCK_MAIN Heart\r\n");
			main_heart_flag=RECEIVE_HEART;
		}
		if(pcb->local_port==SOCK_BUS)
		{
			App_Printf("Receive SOCK_BUS Heart\r\n");
			bus_heart_flag=RECEIVE_HEART;
		}
	}					
  /*���ݳ��ȼ��*/	
	if((payload_in[0]+payload_in[1]*256)!=payloadlen_in-4)
	{
		#if APP_DEBUG
		App_Printf("Topic len error");
		App_Printf("TopicInfo[0]+TopicInfo[1]*256=%d,len=%d\r\n",payload_in[0]+payload_in[1]*256,payloadlen_in);
		App_Printf("\r\n");
	  #endif
		return;
	}
	
	/*CRCУ����*/
	crcdata=app_plat_usMBCRC16(payload_in,payloadlen_in-2);
	if((payload_in[payloadlen_in-1]!=crcdata/256)||(payload_in[payloadlen_in-2]!=crcdata%256))
	{
		#if APP_DEBUG
		App_Printf("CRC error\r\n");
		#endif
		return;
	}
	if(app_plat_GetRespone(payload_in , receivedTopic.lenstring.data , &nState ))
	{
		switch(nState)
		{	
				/*�豸�������*/
			case PLAT_RESPONSE_RRPC:
				 app_system_Heart(pcb,socket);
				break;
			/*�����豸����*/
			case PLAT_RESPONSE_DEVUPDATE:
				//IWDG_Feed();
			  bsp_ota_UpdataGateway(pcb,payloadlen_in,payload_in);  
				break;	
			/*�����豸������Ϣ*/
			case PLAT_RESPONSE_DEVCFGSET:
				app_palt_devcfgset( payload_in , payloadlen_in );
				break;
			/*�豸Ӳ������*/
			case PLAT_RESPONSE_RESET:
				app_palt_reset( payload_in , payloadlen_in );
				break;	
			default:
				break;
		}
	}
}

void app_palt_Connectack(uint8_t *buff,uint16_t socket )
{
	uint8_t sessionPresent, connack_rc;
	
	if (MQTTDeserialize_connack(&sessionPresent, &connack_rc,  buff , PLAT_MAX_DATA_LEN, socket) != 1 || (connack_rc != 0) )
	{		
		/*δ������*/
	}
	else
	{
		#if APP_DEBUG
		printf("Port %d MQTT connect OK\r\n",socket);	
		#endif
		/*д���ӱ�־λ*/
		app_system_mqtt_connect_state_flag_set(socket,MQTT_CONNECT);
	}
}

