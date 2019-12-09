#include "main.h"

/****************************订阅主题******************************/

uint8_t main_heart_flag=0;
uint8_t bus_heart_flag=0;

char devcfgpara_topic[]="/devid_0000000000000000/devcfgpara/set";
char gatereset_topic[]="/devid_0000000000000000/reset/gateway";
char gateupdata_topic[]="/devid_0000000000000000/update/gateway";
char rrpc_request_topic[]="/sys/0000000000000000/rrpc/request/state";
char response_request_topic[]="/sys/0000000000000000/rrpc/request/response";

/*
*********************************************************************************************************
 * 函数名：app_system_connect_state_flag_set
 * 描述  ：获取系统连接状态
 * 输入  ：socket：SOCK_MAIN = 50000，SOCK_BUS = 60000
 * 说明  : sysCfg.parameter.connect_state：0x00，两个socket都未连接；0x01，连接socket0；0x03，连接socket1
 * 返回  : MQTT_CONNECT,连接 MQTT_DISCONNECT,未连接 
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
 * 函数名：app_system_connect_state_flag_set
 * 描述  ：设置系统连接状态标志
 * 输入  ：socket：SOCK_MAIN = 50000，SOCK_BUS =60000
 *       ：connect_state：
 * 说明  : sysCfg.parameter.connect_state：0x00，两个socket都未连接；0x01，连接socket0；0x03，连接socket1
 * 返回  : 无 
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
 * 函数名：app_system_NetLedToggle
 * 描述  ：网络连接状态指示
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
 */
void app_system_NetLedToggle(void)
{	
	switch(sysCfg.parameter.connect_state)
	{
		case 0x01:
			/*配置了第三方服务器 但只连接上了主服务器 按键没按下 红闪*/
			if((sysCfg.parameter.data_socket != SOCK_MAIN) && (sysCfg.parameter.key_time_count == 0)) 
			{
				bsp_LedOff(LED_BLUE);
				bsp_LedOff(LED_GREEN);
			  bsp_LedToggle(LED_RED);
			}
			else if(sysCfg.parameter.data_socket == SOCK_MAIN) /*未配置了第三方服务器 连接上了主服务器 蓝闪*/
			{
				if(sysCfg.parameter.key_time_count == 0)		
				{
					bsp_LedOff(LED_RED);
				}
				bsp_LedOff(LED_GREEN);
				bsp_LedToggle(LED_BLUE);	
			}
			break;
		
		case 0x03:  /*连接上了主服务器和第三方服务器 绿闪*/
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
 * 函数名：app_tcp_init
 * 描述  ：初始化TCP连接
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
*/
void app_tcp_init(void)
{
	/*连接主服务器*/
	TCP_Client_Init(SOCK_MAIN,SERVER_PORT,default_server_ip[0],default_server_ip[1],default_server_ip[2],default_server_ip[3]);
	if(sysCfg.parameter.data_socket == SOCK_MAIN)
	return;
  /*连接第三方服务器*/
	TCP_Client_Init(SOCK_BUS,sysCfg.parameter.server_port,sysCfg.parameter.server_ip[0],sysCfg.parameter.server_ip[1],sysCfg.parameter.server_ip[2],sysCfg.parameter.server_ip[3]); //重新连接
	
}
/*
*********************************************************************************************************
 * 函数名：app_system_LedOn
 * 描述  ：升级指示
 * 输入  ：无
 * 返回  : 无
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
 * 函数名：app_system_CheckID
 * 描述  ：串口查询设备ID
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
*/
//void app_system_CheckID(void)
//{
//	uint8_t i = 0;
//	char recebuf[100];
//	
//	/*读端口数据*/
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
 * 函数名：app_plat_GetRespone
 * 描述  ：获取消息类型
 * 输入  ：topic,消息主题
 *			 : pRespone，存放消息类型
 * 返回  : 
 *       ：SUCCESS,是待处理的消息类型；
 *       ：ERROR，不是待处理的消息类型
*********************************************************************************************************
*/
static ErrorStatus app_plat_GetRespone(uint8_t *TopicInfo,char *topic,app_plat_topic* pRespone)
{ 
	#if 0
	App_Printf("Receive topic:%s\r\n",topic);
	#endif
	
		/*心跳*/
	 if( strstr( topic, "/rrpc/request/state"))
	{
		*pRespone = PLAT_RESPONSE_RRPC;
		return SUCCESS;
	}
	/*网关设备升级*/
	 else if( strstr( topic, "/update/gateway"))
	{
		*pRespone = PLAT_RESPONSE_DEVUPDATE;
		return SUCCESS;
	}

	/*配置设备联网参数*/
	else if( strstr( topic, "/devcfgpara/set"))
	{
		*pRespone = PLAT_RESPONSE_DEVCFGSET;
		return SUCCESS;
	}

	/*网关硬件复位*/
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
 * 函数名：app_palt_devcfgset
 * 描述  ：连接第第三方服务器信息设置
 * 输入  ：
 *       : TopicInfo,主题信息
 *       : len,主题信息
 * 返回  : 无
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
	if(err_code != 4)       /*擦除失败*/
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
 * 函数名：app_palt_reset
 * 描述  ：设备重启
 * 输入  ：TopicInfo,主题信息
 *			 : len，主题长度
 * 返回  : 无
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
  memset(&topicdata[3],0,2);                            /*网关设备ID低字节补齐*/
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);  /*网关ID*/
	memcpy(&topicdata[11],&deal_result,1);  							/*处理成功*/
  memcpy(&topicdata[12],&TopicInfo[len-6],4);           /**/

	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[16],(uint8_t *)&crcdata,2);  
	
	/*上报数据*/
  if(mqtt_publish( SOCK_MAIN, RESETACK_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
		
	}	
	
	vTaskDelay(1000);
	
	POW_RESET();
	#endif
}
/*
*********************************************************************************************************
 * 函数名：app_system_Heart
 * 描述  ：连接心跳
 * 输入  ：socket，心跳发送端口
 * 返回  : 无
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
	mqtt_publish(pcb,rrpc_ack_topic,heart,strlen(heart),socket);	//发送在线心跳
}
/*
*********************************************************************************************************
 * 函数名：app_palt_Receicedata
 * 描述  ：接收平台数据
 * 输入  ：plat_report_t,平台信息结构体
 * 返回  : 无
*********************************************************************************************************
*/
void app_palt_Receicedata(uint8_t * buff,struct tcp_pcb *pcb,uint16_t socket)
{
  uint8_t  len;
	uint8_t  dup;					//重复标志
  uint8_t  retained;		//保留标志
  uint8_t  *payload_in; //负载内容
  uint8_t  ack[30];			//回执
	uint16_t msgid;				//消息ID
  uint16_t crcdata; 
	int qos;							//消息质量
	int payloadlen_in;		//负载长度
	MQTTString receivedTopic;
	app_plat_topic nState;
	
	/*解析消息内容*/
	MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
	&payload_in, &payloadlen_in, buff , PLAT_MAX_DATA_LEN,socket);				
  
	/*消息回执 订阅的QOS=1需要回执*/
	if(1==qos)
	{
	len = MQTTSerialize_puback(ack, sizeof(ack),msgid);
	transport_sendPacketBuffer(pcb,ack,len,0);
	}
	/*网关升级开始指令*/
	if(0 == strncmp(receivedTopic.lenstring.data,gateupdata_topic,strlen(gateupdata_topic))
	&&(0x01 == payload_in[3]+payload_in[4]*256))
	{
		//report_t->gateway_updata_flag = GAT_UPDATA_START;
	}
	
	/*接收到服务器心跳*/
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
  /*数据长度检查*/	
	if((payload_in[0]+payload_in[1]*256)!=payloadlen_in-4)
	{
		#if APP_DEBUG
		App_Printf("Topic len error");
		App_Printf("TopicInfo[0]+TopicInfo[1]*256=%d,len=%d\r\n",payload_in[0]+payload_in[1]*256,payloadlen_in);
		App_Printf("\r\n");
	  #endif
		return;
	}
	
	/*CRC校验检查*/
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
				/*设备心跳检测*/
			case PLAT_RESPONSE_RRPC:
				 app_system_Heart(pcb,socket);
				break;
			/*网关设备升级*/
			case PLAT_RESPONSE_DEVUPDATE:
				//IWDG_Feed();
			  bsp_ota_UpdataGateway(pcb,payloadlen_in,payload_in);  
				break;	
			/*配置设备联网信息*/
			case PLAT_RESPONSE_DEVCFGSET:
				app_palt_devcfgset( payload_in , payloadlen_in );
				break;
			/*设备硬件重启*/
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
		/*未连接上*/
	}
	else
	{
		#if APP_DEBUG
		printf("Port %d MQTT connect OK\r\n",socket);	
		#endif
		/*写连接标志位*/
		app_system_mqtt_connect_state_flag_set(socket,MQTT_CONNECT);
	}
}

