#include "main.h"

SysCfg sysCfg = {0};

#if 0
char default_server_user[] = "test";
char default_server_pass[] = "123456";
uint8_t default_server_ip[4]  = {139,159,213,146};
#else
char default_server_user[] = "CdGateway";
char default_server_pass[] = "0d2ff5970d1c4759a6ade32da30866ab";
uint8_t default_server_ip[4]  = {139,159,133,76};
#endif
static uint32_t idAddr[]={0x1FFFF7AC,
												  0x1FFFF7E8, 
												  0x1FFF7A10,  
												  0x1FFFF7AC,  
												  0x1FFF7A10,  
												  0x1FF0F420,  
												  0x1FF80050,  
												  0x1FF80050,  
												  0x1FFF7590, 
												  0x1FF0F420}; 

/*
*********************************************************************************************************
 * 函数名：app_flash_GetSTM32MCUID
 * 描述  ：获取设备ID
 * 输入  ：id，存放id的数组
 *       ：type，MCU类型
 * 返回  : 无
*********************************************************************************************************
 */
static void app_flash_GetSTM32MCUID(uint32_t *id,MCUTypedef type)
{
	 if(id!=NULL)
	 {
		 id[0]=*(uint32_t*)(idAddr[type]);
		 id[1]=*(uint32_t*)(idAddr[type]+4);
		 id[2]=*(uint32_t*)(idAddr[type]+8);
	 } 
}
/*
*********************************************************************************************************
 * 函数名：app_flash_ReadSysConfig
 * 描述  ：加载主机配置信息
 * 输入  ：无

 * 返回  : 
 *       ：SUCCESS,成功；ERROR，失败
*********************************************************************************************************
 */
ErrorStatus app_flash_LoadSysConfig(void)
{ 
	uint8_t tmp;
	uint8_t errcode;

	uint32_t addr[3] = {0};	
	uint8_t mac_id[12] = {0};
	uint8_t client_mac[6] = {0};
	
	char DeviceID[16]={0};
	
	#if APP_DEBUG
	printf("Load system config...%d\r\n",SYSCFG_DATA_LEN);
	#endif
	
	/*读取配置参数*/
	errcode = bsp_ReadCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN);
	if(1 == errcode)
	{
		#if APP_DEBUG
		printf("ReadSysConfig:Read list fail.\r\n");
		#endif
		return ERROR;  
	} 

	app_flash_GetSTM32MCUID(addr,STM32F1);
	memcpy(mac_id,(uint8_t *)addr,sizeof(mac_id));
		
	/*获取设备ID*/
	#ifdef SYS_64M   
	memcpy(client_mac,mac_id,6);
	for(uint8_t i = 0; i < 6; i++ )
	{
		client_mac[i] ^= mac_id[6+i];
	}
	#else
	memcpy(client_mac,&mac_id[8],3);
	client_mac[3] = mac_id[5];
	
  tmp = mac_id[0];
	tmp <<= 4;
	tmp &= 0xf0;
	mac_id[2] &= 0x0f;
	tmp |= mac_id[2];
	client_mac[4] = tmp;

  tmp = mac_id[6];
	tmp <<= 4;
	tmp &= 0xf0;
	mac_id[7] &= 0x0f;
	tmp |= mac_id[7];
	client_mac[5] = tmp;	
	#endif
	
	client_mac[0] &= 0xfe;   /*第一个字节为偶数*/
	memcpy(sysCfg.parameter.client_mac,client_mac,6);
	sysCfg.parameter.key_time_count=0;
  sysCfg.parameter.connect_state = MQTT_INIT_CONNECT_STATE;
	if(  (0 == memcmp(sysCfg.parameter.server_ip,default_server_ip,sizeof(default_server_ip)))
		&& (0 == strncmp((char *)sysCfg.parameter.server_user,default_server_user,strlen(default_server_user))) 
		&& (0 == strncmp((char *)sysCfg.parameter.server_user,default_server_user,strlen((char *)sysCfg.parameter.server_user))) 
		&& (0 == strncmp((char *)sysCfg.parameter.server_pass,default_server_pass,strlen(default_server_pass)))
		&& (0 == strncmp((char *)sysCfg.parameter.server_pass,default_server_pass,strlen((char *)sysCfg.parameter.server_pass))))
	{
		sysCfg.parameter.config_hold_flag = 0xff;
	}
	
	/*没有被修改过，使用默认配置参数*/
	if(sysCfg.parameter.config_hold_flag != CFG_HOLDER)   
	{
		/*拷贝默认配置*/
		memcpy(sysCfg.parameter.server_ip,default_server_ip,4);
		sysCfg.parameter.server_port = SERVER_PORT;		
		
		memcpy(sysCfg.parameter.server_user,default_server_user,strlen(default_server_user));
		memcpy(sysCfg.parameter.server_pass,default_server_pass,strlen(default_server_pass));
		
		sysCfg.parameter.server_user[strlen(default_server_user)] = '\0';
		sysCfg.parameter.server_pass[strlen(default_server_pass)] = '\0';
			 
		sysCfg.parameter.data_socket = SOCK_MAIN;
		
		#if APP_DEBUG 
		printf("Use default config.\r\n");
		#endif
	}
	else
	{
		#if APP_DEBUG
		printf("Use flash config.\r\n");
		#endif
		sysCfg.parameter.data_socket = SOCK_BUS;
	}
	sprintf(DeviceID,"%02x%02x%02x%02x%02x%02x%02x%02x",0,0,sysCfg.parameter.client_mac[0],sysCfg.parameter.client_mac[1],
	sysCfg.parameter.client_mac[2],sysCfg.parameter.client_mac[3],sysCfg.parameter.client_mac[4],sysCfg.parameter.client_mac[5]);
	
	memcpy(sysCfg.parameter.client_id,DeviceID,sizeof(DeviceID));	
	sysCfg.parameter.client_id[sizeof(DeviceID)] = '\0';
	
	/*订阅消息中加入设备信息*/  	        
	
	memcpy(&gateupdata_topic[7],DeviceID,sizeof(DeviceID));
	memcpy(&devcfgpara_topic[7],DeviceID,sizeof(DeviceID));
	memcpy(&gatereset_topic[7],DeviceID,sizeof(DeviceID));
	memcpy(&rrpc_request_topic[5],DeviceID,sizeof(DeviceID));
	memcpy(&response_request_topic[5],DeviceID,sizeof(DeviceID));
	
	#if APP_DEBUG
	/*打印设置参数*/
	printf("server IP:");
	for(uint8_t i=0;i<3;i++)
	{
		printf("%d",sysCfg.parameter.server_ip[i]);
		printf(".");
	}
	printf("%d",sysCfg.parameter.server_ip[3]);
	
	printf("      PORT:%d",sysCfg.parameter.server_port);
	printf("\r\n");
	
	printf("server username:%s\r\n",sysCfg.parameter.server_user);
	printf("server password:%s\r\n",sysCfg.parameter.server_pass);
	
	printf("Client 	MAC:");
	for(uint8_t i=0;i<6;i++)
	{
		printf("%02x ",sysCfg.parameter.client_mac[i]);
	}
	printf("\r\n");
	
	printf("Client id:%s\r\n",sysCfg.parameter.client_id);
	printf("Hardware version:%s\r\n",HARDWARE_VERSION);
	printf("Software version:%s\r\n",SOFTWARE_VERSION);
	
	printf("Ready to init net...\r\n");
	#endif

	return SUCCESS;
}


