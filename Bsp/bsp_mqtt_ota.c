#include "main.h"
/***************************��������*****************************/
#define UPDATABCD_TOPIC    "/iot_gateway/update/ack"                  /*��������*/
#define SOFTVERSION_TOPIC  "/json/iot_gateway/version/report"

/*
*********************************************************************************************************
 * ��������app_plat_usMBCRC16
 * ����  ������CRCУ��
 * ����  ��pucFrame,����������
 *			 : usLen�����������ݳ���
 * ����  : CRCУ��ֵ
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
*	�� �� ��: FLASH_PagesMask
*	����˵��: �������ֽ�������Ҫ������ҳ��
*	��    �ΣSSize:���ֽ���	 
*	�� �� ֵ: ҳ��
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
*	�� �� ��: bsp_ota_ackResponse
*	����˵��: ������Ӧ
*	��    ��: asktype����Ӧ����
*	        : serialnumber��������ˮ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_ota_ackResponse( struct tcp_pcb *pcb,uint8_t asktype , uint8_t *serialnumber )
{
	uint16_t crcdata;
	uint8_t ackbuf[18]={0x0E,0x00,0x3A};
	ackbuf[2] = UPDATA;
	ackbuf[11] = asktype;
	
	memset(&ackbuf[3],0,2);                           /*�����豸ID���ֽڲ���*/
	memcpy(&ackbuf[5],sysCfg.parameter.client_mac,6); /*����ID*/
	memcpy(&ackbuf[12],serialnumber,4);     				  /*���ݰ���ˮ��*/
	
	/*CRCУ��*/
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
	
	memset(&ackbuf[3],0,2);                           /*�����豸ID���ֽڲ���*/
	memcpy(&ackbuf[5],sysCfg.parameter.client_mac,6); /*����ID*/
	memcpy(&ackbuf[12],serialnumber,4);     				  /*���ݰ���ˮ��*/
	ackbuf[12]+=1;
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(ackbuf,ackbuf[1]*256+ackbuf[0]+2);
	memcpy(&ackbuf[16],(uint8_t *)&crcdata,2);  
  mqtt_publish( pcb, UPDATABCD_TOPIC , (char *)ackbuf , ackbuf[1]*256+ackbuf[0]+4,0 );
}
/*
*********************************************************************************************************
*	�� �� ��: mqtt_firmware
*	����˵��: ��mqttЭ�����ع̼���������
*	��    �ΣSbuf��mqtt�������ݵ�ַ
            updatetopic����������
            messgelen��������Ϣ����
            payload:�������ݵ�ַ
*	�� �� ֵ:
*********************************************************************************************************
*/
void bsp_ota_UpdataGateway(struct tcp_pcb *pcb,uint16_t messgelen,uint8_t* payload)
{
  uint16_t i;
	uint16_t data;                        //��������
	uint16_t status;                      //Flash״̬		
	uint16_t count=0;											//�����ڼ��� 
	
	uint32_t n_pages=0;                   //������ҳ
	uint32_t n_erased=0;
  uint32_t firmwaretype;
	
	static uint16_t  lastpacknum = 0;        //��һ���̼����
	static uint16_t  firmwarepacknum = 0;      //�̼��ܰ���
	
	static uint32_t rxlen=0;							//���ճ���	
	static uint32_t content_len=0;				//���ֽ�
	static uint32_t flashdest=BACKUPADRR;	//��������ַ
		
	/*�����*/
	//count = payload[3] + payload[4]*256;  
	 count = payload[3];
	uint8_t device_info[4] = {GATEWAY,STM32F103RCT6,TRANS_WW5500,NOSRAM};
	uint8_t ask[4] = {ACK_DEVTYPE_ERROR,ACK_MCUTYPE_ERROR,ACK_TRSTYPE_ERROR,ACK_SRATYPE_ERROR};
	
	if(lastpacknum == count)  /*�ظ��̼���*/
	{
		bsp_ota_ackResponse( pcb,ACK_OK , &payload[messgelen-6] );	
    return;		
	}
	
	if(count == 1)       /*��һ������*/
	{ 		
		content_len = 0;   /*�����ܳ���*/
		for(i=0;i<4;i++)   /*��ȡ�̼�����*/
		{
			content_len |= (payload[7+i] << (8 * i));
		}		
					
		/*�̼����ȴ���*/
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
		n_pages = FLASH_PagesMask(content_len - FIRM_INFO_LEN);    /*����Ҫ������ҳ*/
		for(n_erased = 0; n_erased < n_pages; n_erased++)      		 /*����ҳ*/
		{			
			status=FLASH_ErasePage(BACKUPADRR + 0x800 * n_erased);
			if(status != 4)       /*����ʧ��*/
			{
				#if APP_DEBUG
				App_Printf("eraser flash fail\r\n");
				#endif
				bsp_ota_ackResponse( pcb,ACK_ERASE_ERROR , &payload[messgelen-6] );
				return;
			}
		}				
		
		rxlen = 0;                   /*���ռ�������*/
		lastpacknum=count;           /*��һ���̼����*/
		flashdest=BACKUPADRR;        /*д�뿪ʼ��ַ*/
		firmwarepacknum=payload[5]+payload[6]*256; /*�ܰ���*/
		
		#if APP_DEBUG
		App_Printf("firmware_len:%d,total package num:%d\r\n",content_len - FIRM_INFO_LEN,firmwarepacknum);     /*�̼��ܳ���*/
		App_Printf("eraser flash Sucessful\r\n");
		#endif
		
		bsp_ota_ackResponse( pcb,ACK_OK , &payload[messgelen-6] );
	}
	else if(count == 2)      
	{
		/*�豸��Ϣ���*/
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
			bsp_ota_ackResponse( pcb,ACK_FIRTYPE_ERROR , &payload[messgelen-6] ); /*�̼����ʹ���*/
			return;			
		}
		
		/*����д��flash������*/
		for(i = 0;i < messgelen - 11 - FIRM_INFO_LEN ; i += 2)
		{
			if(flashdest > FLAGADRR)        /*�ж��Ƿ�д��Խ��*/
			{
				flashdest = BACKUPADRR;       /*д�뿪ʼ��ַ*/
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
			if(status != 4)        /*д��flashʧ��*/
			{
				#if APP_DEBUG
				App_Printf("write flash fail\r\n");
				#endif
				
				bsp_ota_ackResponse( pcb,ACK_WRITE_ERROR , &payload[messgelen-6] );
				return;
			}
		} 
		
		lastpacknum = count;          /*��һ���̼����*/		
		
		#if APP_DEBUG
		App_Printf("download %6d bytes ok\r\n",rxlen);	 
		#endif		
		bsp_ota_ackResponse( pcb,ACK_OK , &payload[messgelen-6] );
	}
	else if(count > 2)       
	{
		/*����д��flash������*/
		for(i=0;i<messgelen-11;i+=2)
		{
			if(flashdest > FLAGADRR)        /*�ж��Ƿ�д��Խ��*/
			{
				flashdest = BACKUPADRR;         /*д�뿪ʼ��ַ*/
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
			if(status != 4)        /*д��flashʧ��*/
			{
				#if APP_DEBUG
				App_Printf("write flash fail\r\n");
				#endif
				
				bsp_ota_ackResponse( pcb,ACK_WRITE_ERROR , &payload[messgelen-6] );
				return;
			}
		} 
		
		lastpacknum = count;          /*��һ���̼����*/		
		
		#if APP_DEBUG
		App_Printf("download %6d bytes ok\r\n",rxlen);	 
		#endif
	
		
		if((rxlen == content_len - FIRM_INFO_LEN) && ((count-1) == firmwarepacknum))   	  /*�����������*/
		{	 		
			/*���һ����ִ�������޷�������δ�ҵ�ԭ����ʱ����else��֧�����ִ*/
			// bsp_ota_ackResponse( pcb,ACK_OK , &payload[messgelen-6] );
			/*д���ļ�����*/									 
			if(!(bsp_WriteCpuFlash(FLAGADRR,(uint8_t *)&rxlen,4)))
			{
				#if APP_DEBUG
				App_Printf("\r\nReady t o reboot.......\r\n");
				#endif
				/*�Ͽ�Mqtt����*/
				//mqtt_disconnect();
				
				delay_ms(5000);
				__set_FAULTMASK(1);
				NVIC_SystemReset();  /*����ϵͳ*/
			}
			else	/*д���־λʧ��*/
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
			  /*���һ����ִ*/
			  if((count-1) == firmwarepacknum-1)
		    {
			   bsp_ota_ackResponse1( pcb,ACK_OK , &payload[messgelen-6] );
		    }
		}			
	}		 
}



