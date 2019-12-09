#ifndef __APP_PLAT_H
#define __APP_PLAT_H

#include "stm32f10x.h"
#include "MQTTPacket.h"
#include "lwip/tcp.h"
/***************************发布主题*****************************/

#define UPDATABCD_TOPIC    "/iot_gateway/update/ack"          	/*升级主题回执主题*/
#define SOFTVERSION_TOPIC  "/json/iot_gateway/version/report"  	/*设备软件版本上报*/
#define ALARM_TOPIC        "/iot_gateway/device_alarm/report"   /*设备告警主题*/


//#define WILL_TOPIC         "/iot_gateway/offline_will/report"   /*遗嘱消息主题*/


#define VALID_SOCKT_NUM    3

#define PLAT_MAX_DATA_LEN  512

#define RECEIVE_HEART      1
#define RECEIVE_CLEAR      0

#define BLE_UPDATA_START   1
#define BRA_UPDATA_START   1
#define GAT_UPDATA_START   1
#define BLE_UPDATA_RESET   0
#define BRA_UPDATA_RESET   0
#define GAT_UPDATA_RESET   0

#define MQTT_INIT_CONNECT_STATE     0x00
#define BUS_MQTT_CONNECT_FLAG_BIT   1
#define MAIN_MQTT_CONNECT_FLAG_BIT  0

typedef enum 
{
	PLAT_RESPONSE_SEND_MESSAGE = 0, 
	PLAT_RESPONSE_ADDRMANAGEMENT,
	PLAT_RESPONSE_BEAMANAGEMENT,
	PLAT_RESPONSE_BRAUPDATE,
	PLAT_RESPONSE_DEVUPDATE,
	PLAT_RESPONSE_BLEUPDATE,
	PLAT_RESPONSE_USERINFOSET,
	PLAT_RESPONSE_DEVCFGSET,
	PLAT_RESPONSE_RESET,
	PLAT_RESPONSE_RRPC,
}app_plat_topic;
typedef enum 
{
	MQTT_CONNECT = 0,
	MQTT_DISCONNECT
}MQTT_CONNECT_STATE;

#define PORRSTF_MASK 				 (1ul<<27)
#define SFTRST_MASK  				 (1ul<<28)
#define IWDGRST_MASK			   (1ul<<29)
#define WWDGRST_MASK  			 (1ul<<30)
#define LPWRRSTF_MASK 			 (1ul<<31)

extern char gatereset_topic[];
extern char gateupdata_topic[];
extern char devcfgpara_topic[];
extern char rrpc_request_topic[];
extern char response_request_topic[];
extern uint8_t main_heart_flag;
extern uint8_t bus_heart_flag;

MQTT_CONNECT_STATE app_system_mqtt_connect_state_get(uint16_t socket);
void app_system_mqtt_connect_state_flag_set(uint16_t socket,MQTT_CONNECT_STATE connect_state);
void app_system_TcpsStart(struct tcp_pcb *pcb);
void app_palt_Connectack(uint8_t * buff,uint16_t socket);
void app_palt_Receicedata(uint8_t * buff,struct tcp_pcb *pcb,uint16_t socket);
void app_system_NetLedToggle(void);
void app_tcp_init(void);
#endif
