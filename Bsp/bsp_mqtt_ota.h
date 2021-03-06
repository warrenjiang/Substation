#ifndef _BSP_MQTT_OTA_H_
#define _BSP_MQTT_OTA_H_

#include "stm32f10x.h"
#include "lwip/tcp.h"
/********************FLASH分区*****************************
Bootloder:   0x08000000--0x08007FFF        32K
Application: 0x08008000--0x08023FFF        112K
Backup:      0x08024000--0x0802EFFF        108k
Flag:        0x0803f000--0x0803FFFF        4K
**********************************************************/

#define FIRM_INFO_LEN       8             /*固件头部有效信息字节数*/

/*设备类型*/
#define GATEWAY             0x01          /*网关设备*/
#define RFID                0x02					/*刷卡机设备*/

/*主控类型*/
#define STM32F103ZET6       0x01   
#define STM32F103RCT6       0x02
#define STM32F030C8T6       0x03  

/*传输类型*/
#define TRANS_WW5500        0x01
#define TRANS_GSM           0x02

/*SRAM 类型*/                 
#define NOSRAM              0x01

#define SRAM                0x02

/*软件版本号*/
#define SOFTVERSION         1

#define UPDATA     			 		0x3A          /*升级响应Msg_id*/
#define BLE_UPDATA     			0x9A          /*升级响应Msg_id*/
#define ACK_FAIL     			 	0x00          /*操作失败*/
#define ACK_OK     			 		0x01          /*操作成功*/
#define OTA_STA    			 		0x02          
#define KEY_OK    			 		0x03          

#define ACK_ERASE_ERROR  		0x02          /*擦除Flash失败*/
#define ACK_WRITE_ERROR  		0x03          /*写Flash失败*/
#define ACK_LEN_ERROR    		0x04					/*固件长度异常*/
#define ACK_FLAG_ERROR   		0x05					/*写入升级标志位失败*/
#define ACK_CRC16_ERROR 		0x06					/*CRC校验失败*/
#define ACK_ADDROV_ERROR 	  0x07          /*写地址越界*/

#define ACK_FIRTYPE_ERROR   0x08          /*固件类型错误*/	
#define ACK_DEVTYPE_ERROR   0x09          /*设备类型错误*/
#define ACK_MCUTYPE_ERROR   0x0A          /*主控类型错误*/
#define ACK_TRSTYPE_ERROR   0x0B          /*传输类型错误*/
#define ACK_SRATYPE_ERROR   0x0C          /*SRAM类型错误*/
#define ACK_VERTYPE_ERROR   0x0D   				/*软件版本错误*/

#define APPADRR          0x08008000    /*APP程序地址*/
#define BACKUPADRR 			 0x08024000    /*程序备份区地址*/
#define FLAGADRR   			 0x0803F000    /*标志位地址*/

void bsp_ota_UpdataGateway(struct tcp_pcb *pcb,uint16_t messgelen,uint8_t* payload);
uint16_t app_plat_usMBCRC16( uint8_t * pucFrame, uint16_t usLen );
#endif


