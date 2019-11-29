#ifndef __APP_PLAT_H
#define __APP_PLAT_H

#include "stm32f10x.h"
#include "MQTTPacket.h"
#include "lwip/tcp.h"
/***************************��������*****************************/
#define HIS_SPORT_TOPIC    "/bracelet/report/history_sport_data"/*��ʷ�˶���������*/
#define SPORT_TOPIC        "/bracelet/report/sport_data"     	  /*�˶���������*/
#define HEALTH_TOPIC       "/bracelet/report/heartrate_data" 	  /*������������*/
#define LOCATION_TOPIC     "/bracelet/report/location"        	/*λ����������*/
#define BASEINFOR_TOPIC    "/bracelet/report/bracelet_info"   	/*�ֻ�������������*/
#define ATTENDANCE_TOPIC   "/bracelet/attendance/report"      	/*������������*/
#define BLE_ATT_TOPIC      "/iot_gateway/ble_attendance/report" /*�����޸п�������*/
#define SLEEP_TOPIC        "/bracelet/report/sleep_data"        /*˯������*/

#define RESETACK_TOPIC     "/iot_gateway/reset/ack"             /*�������û�ִ����*/ 
#define SMSBCD_TOPIC       "/iot_gateway/notice/sms"          	/*��Ϣ��ִ����*/
#define ADDRMANNABCD_TOPIC "/iot_gateway/management/addr"     	/*485��ַ�����ִ����*/
#define UPDATABCD_TOPIC    "/iot_gateway/update/ack"          	/*���������ִ����*/
#define UPDATABLE_TOPIC    "/iot_gateway/braupdate/ack"         /*���������ִ����*/
#define BLEUPDATABCD_TOPIC "/iot_gateway/bleupdate/ack"         /*�������������ִ����*/
#define BAECON_TOPIC       "/iot_gateway/management/beacon"  	  /*�ű�����ִ*/
#define USERINFO_TOPIC     "/iot_gateway/user_info/ack"       	/*�û���Ϣ���û�ִ*/

#define SOFTVERSION_TOPIC  "/json/iot_gateway/version/report"  	/*�豸����汾�ϱ�*/
#define ALARM_TOPIC        "/iot_gateway/device_alarm/report"   /*�豸�澯����*/
#define SHT_TOPIC					 "/iot_gateway/temp_hum/report"  			/*��ʪ�������ϱ�*/

//#define WILL_TOPIC         "/iot_gateway/offline_will/report"   /*������Ϣ����*/

/***************************��ƽ̨Э��***************************/
#define SPORT              0x51        //�˶�����Msg_id
#define HEALTH             0x52        //��������Msg_id
#define LOCATION           0x53        //λ������Msg_id
#define BASEINFOR          0x54        //�ֻ���ϢMsg_id
#define SLEEP_DATA         0x55        //˯������Msg_id
#define SHT_DATA           0x56        //��ʪ������Msg_id
#define VERSION            0x57        //�汾��ϢMsg_id
#define DEVRESTET          0xB0        //�豸����Msg_id

#define RESTETACK          0x0B        /*�������û�ִMsg_id*/
#define BANDDATABACK       0x1A        /*�ֻ���Ϣ��ִMsg_id*/
#define BEACONBACK         0x4A        /*�����ű��ִMsg_id*/
#define USERINFOACK        0x6A        /*�ֻ�������Ϣ��ִMsg_id*/
#define BLEUPDATACK        0x8A        /*����������ִMsg_id*/

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
