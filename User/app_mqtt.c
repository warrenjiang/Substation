#include "main.h"

/*
*********************************************************************************************************
 * ��������transport_sendPacketBuffer
 * ����  ������TCP���� 
 * ����  ��cpcb TCP��  buf ����Դ buflen ���ݳ��� socket �˿�
 * ˵��  : ����tcp_writeֻ�ǽ�����д�뻺������У���Eth_Task ������ÿ250ms�����
					tcp_fasttmr-->tcp_outputȻ���Ϳ��Է��͵�����
					Ҳ�����ڵ���tcp_write֮��ֱ�ӵ���tcp_output �������� ǰ��Ч�ʸ���Щ
 * ����  : 
*********************************************************************************************************
 */
err_t transport_sendPacketBuffer(struct tcp_pcb *cpcb,unsigned char* buf, int buflen,unsigned char socket)
{
  err_t err;
	err = tcp_write(cpcb,buf,buflen,TCP_WRITE_FLAG_COPY);	//������д������У�������������
	//tcp_output(cpcb);
	return err;					
}
/*
*********************************************************************************************************
 * ��������mqtt_disconnect
 * ����  ���Ͽ�MQTT����
 * ����  ��cpcb TCP��  
 * ˵��  : 
 * ����  : 
*********************************************************************************************************
 */
void mqtt_disconnect(struct tcp_pcb *cpcb)
{
	uint8_t len;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	
	len = MQTTSerialize_disconnect(buf,buflen);
	transport_sendPacketBuffer(cpcb, buf, len,0);		//���ط��͵ĳ���
}	

/*
*********************************************************************************************************
 * ��������Heartbeat
 * ����  ����MQTT������֮�������
 * ����  ��cpcb TCP��  
 * ˵��  : 
 * ����  : 
*********************************************************************************************************
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
		return -1;	
	}
	return 0;
}	
/*
*********************************************************************************************************
 * ��������mqtt_connect
 * ����  ������MQTT������ 
 * ����  ��cpcb TCP��   name �û��� word ���� socket �˿ں�
 * ˵��  : 
 * ����  : ʧ�ܷ���-1 �ɹ����� 0
*********************************************************************************************************
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


/*
*********************************************************************************************************
 * ��������mqtt_publish
 * ����  ��������Ϣ
 * ����  ��cpcb TCP��   pTopic �������� pMessage ��Ϣ���� msglen ��Ϣ���� socket �˿ں�
 * ˵��  : 
 * ����  : ʧ�ܷ���-1 �ɹ����� 0
*********************************************************************************************************
 */
int mqtt_publish(struct tcp_pcb *cpcb,char *pTopic,char *pMessage,int msglen,uint8_t socket)
{
  int32_t len,rc;
  unsigned char buf[256];
  MQTTString topicString = MQTTString_initializer;
  int buflen = sizeof(buf);
  topicString.cstring = pTopic;
  len= MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)pMessage, msglen); /* 2 */
	rc = transport_sendPacketBuffer(cpcb,buf,len,socket);
	if (rc == ERR_OK)
	{
		#if 0
		printf("Published %x OK\r\n",buf[len-6]);
		#endif
		return 0;
	}
	else
	{
		printf("Publish failed code:%d\r\n",rc);
		return -1;
	}
	#if 0
  /*�����Ҫ��������Ϣ�����رն˿ڣ���Ҫһ����ʱ QOS=1��������Ϣʱ���������˻᷵��PUBACK*/
	 if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBACK) 	
	{
	
	}
	#endif
	
}
/*
*********************************************************************************************************
 * ��������mqtt_subscrib
 * ����  ��������Ϣ
 * ����  ��cpcb TCP��   pTopic �������� socket �˿ں� reg_qos ��������
 * ˵��  : 
 * ����  : ʧ�ܷ���-1 �ɹ����� 0
*********************************************************************************************************
 */
int mqtt_subscrib(struct tcp_pcb *cpcb,char *pTopic,uint8_t socket,int req_qos)
{
	MQTTString topicString = MQTTString_initializer;
	unsigned char buf[128];
	int buflen = sizeof(buf);
	int rc = 0;
	int msgid = 1;
	int len = 0;
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
 * ����  ���������Ӳ��� 
   {"gid":"0000306101411199","bid":"0000d882494f73c7","cip":"192.168.0.100","mac":"30:61:01:41:11:99",
   "connect":"139.159.133.76:1883","sver":"BS_V100R010C01B140_0409","hver":"MCBR02_V1.7","sta":"iwdrst"}
 * ����  ��cpcb TCP��   
 * ˵��  : 
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
/*
*********************************************************************************************************
 * ��������app_system_TcpsStart
 * ����  ��������Ϣ
 * ����  ��cpcb TCP��   
 * ˵��  : 
 * ����  : ��
*********************************************************************************************************
 */
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


