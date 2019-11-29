#ifndef __APP_PLAT_H
#define __APP_PLAT_H

#include "stm32f10x.h"
#include "MQTTPacket.h"
#include "lwip/tcp.h"
/***************************发布主题*****************************/
#define HIS_SPORT_TOPIC    "/bracelet/report/history_sport_data"/*历史运动数据主题*/
#define SPORT_TOPIC        "/bracelet/report/sport_data"     	  /*运动数据主题*/
#define HEALTH_TOPIC       "/bracelet/report/heartrate_data" 	  /*健康数据主题*/
#define LOCATION_TOPIC     "/bracelet/report/location"        	/*位置数据主题*/
#define BASEINFOR_TOPIC    "/bracelet/report/bracelet_info"   	/*手环基本数据主题*/
#define ATTENDANCE_TOPIC   "/bracelet/attendance/report"      	/*考勤数据主题*/
#define BLE_ATT_TOPIC      "/iot_gateway/ble_attendance/report" /*蓝牙无感考勤主题*/
#define SLEEP_TOPIC        "/bracelet/report/sleep_data"        /*睡眠数据*/

#define RESETACK_TOPIC     "/iot_gateway/reset/ack"             /*网关重置回执主题*/ 
#define SMSBCD_TOPIC       "/iot_gateway/notice/sms"          	/*消息回执主题*/
#define ADDRMANNABCD_TOPIC "/iot_gateway/management/addr"     	/*485地址管理回执主题*/
#define UPDATABCD_TOPIC    "/iot_gateway/update/ack"          	/*升级主题回执主题*/
#define UPDATABLE_TOPIC    "/iot_gateway/braupdate/ack"         /*升级主题回执主题*/
#define BLEUPDATABCD_TOPIC "/iot_gateway/bleupdate/ack"         /*蓝牙升级主题回执主题*/
#define BAECON_TOPIC       "/iot_gateway/management/beacon"  	  /*信标管理回执*/
#define USERINFO_TOPIC     "/iot_gateway/user_info/ack"       	/*用户信息设置回执*/

#define SOFTVERSION_TOPIC  "/json/iot_gateway/version/report"  	/*设备软件版本上报*/
#define ALARM_TOPIC        "/iot_gateway/device_alarm/report"   /*设备告警主题*/
#define SHT_TOPIC					 "/iot_gateway/temp_hum/report"  			/*温湿度数据上报*/

//#define WILL_TOPIC         "/iot_gateway/offline_will/report"   /*遗嘱消息主题*/

/***************************到平台协议***************************/
#define SPORT              0x51        //运动数据Msg_id
#define HEALTH             0x52        //健康数据Msg_id
#define LOCATION           0x53        //位置数据Msg_id
#define BASEINFOR          0x54        //手环信息Msg_id
#define SLEEP_DATA         0x55        //睡眠数据Msg_id
#define SHT_DATA           0x56        //温湿度数据Msg_id
#define VERSION            0x57        //版本信息Msg_id
#define DEVRESTET          0xB0        //设备重置Msg_id

#define RESTETACK          0x0B        /*网关重置回执Msg_id*/
#define BANDDATABACK       0x1A        /*手环消息回执Msg_id*/
#define BEACONBACK         0x4A        /*考勤信标回执Msg_id*/
#define USERINFOACK        0x6A        /*手环个人信息回执Msg_id*/
#define BLEUPDATACK        0x8A        /*升级蓝牙回执Msg_id*/

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
extern char bracelet_topic[];
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
