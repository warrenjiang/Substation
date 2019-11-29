#include "main.h"
/***************************发布主题*****************************/
#define UPDATABCD_TOPIC    "/iot_gateway/update/ack"                  /*升级主题*/
#define SOFTVERSION_TOPIC  "/json/iot_gateway/version/report"

/*
*********************************************************************************************************
 * 函数名：app_plat_usMBCRC16
 * 描述  ：数据CRC校验
 * 输入  ：pucFrame,待处理数据
 *			 : usLen，待处理数据长度
 * 返回  : CRC校验值
*********************************************************************************************************
*/
uint16_t app_plat_usMBCRC16( uint8_t * pucFrame, uint16_t usLen )
{
	uint8_t ucCRCHi = 0xFF;
	uint8_t ucCRCLo = 0xFF;
	uint16_t iIndex;
	while( usLen-- )
	{
		iIndex = ucCRCLo ^ *( pucFrame++ );
		ucCRCLo = ( uint8_t )( ucCRCHi ^ aucCRCHi[iIndex] );
		ucCRCHi = aucCRCLo[iIndex];
	}
	return ( uint16_t )( ucCRCHi << 8 | ucCRCLo );
}
/*
*********************************************************************************************************
*	函 数 名: FLASH_PagesMask
*	功能说明: 根据总字节数计算要擦除的页数
*	形    参Size:总字节数	 
*	返 回 值: 页数
*********************************************************************************************************
*/
static uint32_t FLASH_PagesMask(uint32_t Size)
{
  uint32_t pagenumber = 0x0;
  uint32_t size = Size;

  if((size % 0x800) != 0)
  {
    pagenumber = (size / 0x800) + 1;
  }
  else
  {
    pagenumber = size / 0x800;
  }
  return pagenumber;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ota_ackResponse
*	功能说明: 升级响应
*	形    参: asktype，响应类型
*	        : serialnumber，数据流水号
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_ota_ackResponse( struct tcp_pcb *pcb,uint8_t asktype , uint8_t *serialnumber )
{
	uint16_t crcdata;
	uint8_t ackbuf[18]={0x0E,0x00,0x3A};
	ackbuf[2] = UPDATA;
	ackbuf[11] = asktype;
	
	memset(&ackbuf[3],0,2);                           /*网关设备ID低字节补齐*/
	memcpy(&ackbuf[5],sysCfg.parameter.client_mac,6); /*网关ID*/
	memcpy(&ackbuf[12],serialnumber,4);     				  /*数据包流水号*/
	
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(ackbuf,ackbuf[1]*256+ackbuf[0]+2);
	memcpy(&ackbuf[16],(uint8_t *)&crcdata,2);  
  mqtt_publish( pcb, UPDATABCD_TOPIC , (char *)ackbuf , ackbuf[1]*256+ackbuf[0]+4,0 );
}
static void bsp_ota_ackResponse1( struct tcp_pcb *pcb,uint8_t asktype , uint8_t *serialnumber )
{
	uint16_t crcdata;
	uint8_t ackbuf[18]={0x0E,0x00,0x3A};
	ackbuf[2] = UPDATA;
	ackbuf[11] = asktype;
	
	memset(&ackbuf[3],0,2);                           /*网关设备ID低字节补齐*/
	memcpy(&ackbuf[5],sysCfg.parameter.client_mac,6); /*网关ID*/
	memcpy(&ackbuf[12],serialnumber,4);     				  /*数据包流水号*/
	ackbuf[12]+=1;
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(ackbuf,ackbuf[1]*256+ackbuf[0]+2);
	memcpy(&ackbuf[16],(uint8_t *)&crcdata,2);  
  mqtt_publish( pcb, UPDATABCD_TOPIC , (char *)ackbuf , ackbuf[1]*256+ackbuf[0]+4,0 );
}
/*
*********************************************************************************************************
*	函 数 名: mqtt_firmware
*	功能说明: 由mqtt协议下载固件到备份区
*	形    参buf：mqtt接收数据地址
            updatetopic：升级主题
            messgelen：负载消息长度
            payload:负载内容地址
*	返 回 值:
*********************************************************************************************************
*/
void bsp_ota_UpdataGateway(struct tcp_pcb *pcb,uint16_t messgelen,uint8_t* payload)
{
  uint16_t i;
	uint16_t data;                        //接收数据
	uint16_t status;                      //Flash状态		
	uint16_t count=0;											//计数第几包 
	
	uint32_t n_pages=0;                   //擦除的页
	uint32_t n_erased=0;
  uint32_t firmwaretype;
	
	static uint16_t  lastpacknum = 0;        //上一包固件序号
	static uint16_t  firmwarepacknum = 0;      //固件总包数
	
	static uint32_t rxlen=0;							//接收长度	
	static uint32_t content_len=0;				//总字节
	static uint32_t flashdest=BACKUPADRR;	//备份区地址
		
	/*包序号*/
	//count = payload[3] + payload[4]*256;  
	 count = payload[3];
	uint8_t device_info[4] = {GATEWAY,STM32F103RCT6,TRANS_WW5500,NOSRAM};
	uint8_t ask[4] = {ACK_DEVTYPE_ERROR,ACK_MCUTYPE_ERROR,ACK_TRSTYPE_ERROR,ACK_SRATYPE_ERROR};
	
	if(lastpacknum == count)  /*重复固件包*/
	{
		bsp_ota_ackResponse( pcb,ACK_OK , &payload[messgelen-6] );	
    return;		
	}
	
	if(count == 1)       /*第一包数据*/
	{ 		
		content_len = 0;   /*清零总长度*/
		for(i=0;i<4;i++)   /*获取固件长度*/
		{
			content_len |= (payload[7+i] << (8 * i));
		}		
					
		/*固件长度错误*/
		if(BACKUPADRR + content_len - FIRM_INFO_LEN > FLAGADRR)
		{
			#if APP_DEBUG
			App_Printf("firmware len error.\r\n");	 
			#endif
			bsp_ota_ackResponse( pcb,ACK_LEN_ERROR , &payload[messgelen-6] );
			return;
		}
		
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		n_pages = FLASH_PagesMask(content_len - FIRM_INFO_LEN);    /*计算要擦除的页*/
		for(n_erased = 0; n_erased < n_pages; n_erased++)      		 /*擦除页*/
		{			
			status=FLASH_ErasePage(BACKUPADRR + 0x800 * n_erased);
			if(status != 4)       /*擦除失败*/
			{
				#if APP_DEBUG
				App_Printf("eraser flash fail\r\n");
				#endif
				bsp_ota_ackResponse( pcb,ACK_ERASE_ERROR , &payload[messgelen-6] );
				return;
			}
		}				
		
		rxlen = 0;                   /*接收计数清零*/
		lastpacknum=count;           /*上一包固件序号*/
		flashdest=BACKUPADRR;        /*写入开始地址*/
		firmwarepacknum=payload[5]+payload[6]*256; /*总包数*/
		
		#if APP_DEBUG
		App_Printf("firmware_len:%d,total package num:%d\r\n",content_len - FIRM_INFO_LEN,firmwarepacknum);     /*固件总长度*/
		App_Printf("eraser flash Sucessful\r\n");
		#endif
		
		bsp_ota_ackResponse( pcb,ACK_OK , &payload[messgelen-6] );
	}
	else if(count == 2)      
	{
		/*设备信息检查*/
		for(uint8_t i = 0; i < 4;i++)
		{
			
			if(payload[5+i] != device_info[i])
			{
				bsp_ota_ackResponse( pcb,ask[i] , &payload[messgelen-6] );
				#if APP_DEBUG
				App_Printf("firmware type error code:%d\r\n",i);
				#endif
				return;
			}
		}
		
		if(payload[12] < SOFTVERSION)
		{                      
			bsp_ota_ackResponse( pcb,ACK_VERTYPE_ERROR , &payload[messgelen-6] );
			#if APP_DEBUG
			App_Printf("firmware version error\r\n");
			#endif
			return;			
		}
		
		memcpy((uint8_t *)&firmwaretype,&payload[13],4);
		if((firmwaretype & 0x2FFE0000) != 0x20000000)
		{
			#if APP_DEBUG
			App_Printf("firmware type error.\r\n");	 
			#endif
			bsp_ota_ackResponse( pcb,ACK_FIRTYPE_ERROR , &payload[messgelen-6] ); /*固件类型错误*/
			return;			
		}
		
		/*数据写入flash备份区*/
		for(i = 0;i < messgelen - 11 - FIRM_INFO_LEN ; i += 2)
		{
			if(flashdest > FLAGADRR)        /*判断是否写入越界*/
			{
				flashdest = BACKUPADRR;       /*写入开始地址*/
				#if APP_DEBUG
				App_Printf("write firmware over backupadrr.\r\n");
				#endif 
				bsp_ota_ackResponse( pcb,ACK_ADDROV_ERROR , &payload[messgelen-6] );
				return;            
			}
			data = payload[i+6+FIRM_INFO_LEN];	
			data = (data<<8) + payload[i+5+FIRM_INFO_LEN];
			status = FLASH_ProgramHalfWord(flashdest, data);									      
			flashdest += 2;	 
			rxlen += 2;
			if(status != 4)        /*写入flash失败*/
			{
				#if APP_DEBUG
				App_Printf("write flash fail\r\n");
				#endif
				
				bsp_ota_ackResponse( pcb,ACK_WRITE_ERROR , &payload[messgelen-6] );
				return;
			}
		} 
		
		lastpacknum = count;          /*上一包固件序号*/		
		
		#if APP_DEBUG
		App_Printf("download %6d bytes ok\r\n",rxlen);	 
		#endif		
		bsp_ota_ackResponse( pcb,ACK_OK , &payload[messgelen-6] );
	}
	else if(count > 2)       
	{
		/*数据写入flash备份区*/
		for(i=0;i<messgelen-11;i+=2)
		{
			if(flashdest > FLAGADRR)        /*判断是否写入越界*/
			{
				flashdest = BACKUPADRR;         /*写入开始地址*/
				#if APP_DEBUG
				App_Printf("write firmware over backupadrr.\r\n");
				#endif
				bsp_ota_ackResponse( pcb,ACK_ADDROV_ERROR , &payload[messgelen-6] );
				return;            
			}
			data = payload[i+6];	
			data = (data<<8) + payload[i+5];
			status = FLASH_ProgramHalfWord(flashdest, data);									      
			flashdest += 2;	 
			rxlen += 2;
			if(status != 4)        /*写入flash失败*/
			{
				#if APP_DEBUG
				App_Printf("write flash fail\r\n");
				#endif
				
				bsp_ota_ackResponse( pcb,ACK_WRITE_ERROR , &payload[messgelen-6] );
				return;
			}
		} 
		
		lastpacknum = count;          /*上一包固件序号*/		
		
		#if APP_DEBUG
		App_Printf("download %6d bytes ok\r\n",rxlen);	 
		#endif
	
		
		if((rxlen == content_len - FIRM_INFO_LEN) && ((count-1) == firmwarepacknum))   	  /*接收数据完成*/
		{	 		
			/*最后一包回执在这里无法发出，未找到原因，暂时改在else分支下面回执*/
			// bsp_ota_ackResponse( pcb,ACK_OK , &payload[messgelen-6] );
			/*写入文件长度*/									 
			if(!(bsp_WriteCpuFlash(FLAGADRR,(uint8_t *)&rxlen,4)))
			{
				#if APP_DEBUG
				App_Printf("\r\nReady t o reboot.......\r\n");
				#endif
				/*断开Mqtt连接*/
				//mqtt_disconnect();
				
				delay_ms(5000);
				__set_FAULTMASK(1);
				NVIC_SystemReset();  /*重启系统*/
			}
			else	/*写入标志位失败*/
			{
				#if APP_DEBUG
				App_Printf("write flag fail\r\n");
        #endif			  
				bsp_ota_ackResponse( pcb,ACK_FLAG_ERROR , &payload[messgelen-6] );
				return;
			}
		}			
    else
		{
				bsp_ota_ackResponse( pcb,ACK_OK , &payload[messgelen-6] );
			  /*最后一包回执*/
			  if((count-1) == firmwarepacknum-1)
		    {
			   bsp_ota_ackResponse1( pcb,ACK_OK , &payload[messgelen-6] );
		    }
		}			
	}		 
}



