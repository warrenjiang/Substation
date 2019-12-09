#include "main.h"

/*
*********************************************************************************************************
 * 函数名：transport_sendPacketBuffer
 * 描述  ：发送TCP数据 
 * 输入  ：cpcb TCP块  buf 数据源 buflen 数据长度 socket 端口
 * 说明  : 调用tcp_write只是将数据写入缓存队列中，在Eth_Task 任务中每250ms会调用
					tcp_fasttmr-->tcp_output然后发送可以发送的数据
					也可以在调用tcp_write之后直接调用tcp_output 发送数据 前者效率更高些
 * 返回  : 
*********************************************************************************************************
 */
err_t transport_sendPacketBuffer(struct tcp_pcb *cpcb,unsigned char* buf, int buflen,unsigned char socket)
{
  err_t err;
	err = tcp_write(cpcb,buf,buflen,TCP_WRITE_FLAG_COPY);	//将数据写入队列中，不会立即发送
	//tcp_output(cpcb);
	return err;					
}
/*
*********************************************************************************************************
 * 函数名：mqtt_disconnect
 * 描述  ：断开MQTT连接
 * 输入  ：cpcb TCP块  
 * 说明  : 
 * 返回  : 
*********************************************************************************************************
 */
void mqtt_disconnect(struct tcp_pcb *cpcb)
{
	uint8_t len;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	
	len = MQTTSerialize_disconnect(buf,buflen);
	transport_sendPacketBuffer(cpcb, buf, len,0);		//返回发送的长度
}	

/*
*********************************************************************************************************
 * 函数名：Heartbeat
 * 描述  ：与MQTT服务器之间的心跳
 * 输入  ：cpcb TCP块  
 * 说明  : 
 * 返回  : 
*********************************************************************************************************
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
		return -1;	
	}
	return 0;
}	
/*
*********************************************************************************************************
 * 函数名：mqtt_connect
 * 描述  ：连接MQTT服务器 
 * 输入  ：cpcb TCP块   name 用户名 word 密码 socket 端口号
 * 说明  : 
 * 返回  : 失败返回-1 成功返回 0
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
	rc = transport_sendPacketBuffer(cpcb,buf,len,socket);	//返回发送的长度
	
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
 * 函数名：mqtt_publish
 * 描述  ：发布消息
 * 输入  ：cpcb TCP块   pTopic 发布主题 pMessage 消息内容 msglen 消息长度 socket 端口号
 * 说明  : 
 * 返回  : 失败返回-1 成功返回 0
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
  /*如果需要发送完消息，即关闭端口，需要一定延时 QOS=1，发布消息时，服务器端会返回PUBACK*/
	 if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBACK) 	
	{
	
	}
	#endif
	
}
/*
*********************************************************************************************************
 * 函数名：mqtt_subscrib
 * 描述  ：订阅消息
 * 输入  ：cpcb TCP块   pTopic 订阅主题 socket 端口号 reg_qos 订阅质量
 * 说明  : 
 * 返回  : 失败返回-1 成功返回 0
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
 * 函数名：app_system_NetPublic
 * 描述  ：发布连接参数 
   {"gid":"0000306101411199","bid":"0000d882494f73c7","cip":"192.168.0.100","mac":"30:61:01:41:11:99",
   "connect":"139.159.133.76:1883","sver":"BS_V100R010C01B140_0409","hver":"MCBR02_V1.7","sta":"iwdrst"}
 * 输入  ：cpcb TCP块   
 * 说明  : 
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
/*
*********************************************************************************************************
 * 函数名：app_system_TcpsStart
 * 描述  ：订阅消息
 * 输入  ：cpcb TCP块   
 * 说明  : 
 * 返回  : 无
*********************************************************************************************************
 */
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


