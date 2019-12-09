#include "main.h"

/*
*********************************************************************************************************
 * ��������udp_pc_recv_rb
 * ����  �����ڴ�����λ���·�����
 * ����  ����
 * ˵��  : 
 * ����  : ������UDP��
*********************************************************************************************************
 */
 /*
 ���̣�
 1.��λ������ FF 01 01 02 HOST->255.255.255.255:1500
	  01 ����Ϣ���ͣ� 
 2.�豸����first ��ʱ��λ������ʾ���������豸 Local ip ->HOST
 3.����������ĸ��豸 ��λ���·�
   FF 13 03 24 43 18 56 07 38 61 64 6D 69 6E 00 61 64 6D 69 6E 00 3C
	 03����Ϣ���ͣ�  24 43 18 56 07 38:6�ֽ�MAC��ַ 61 64 6D 69 6E ��admin
 4.�豸����secend third  ���ұ߼��ɿ��������б� Local ip ->HOST 
 5.�޸���ز������������ ��λ���·�
   FF 56 05 24 43 18 56 07 38....��������ip ����ģʽ
	 FF 52 06 24 43 18 56 07 38....����������ip
	 FF 68 21 24 43 18 56 07 38....�����Զ���ע���
	 FF 13 02 24 43 18 56 07 38....�豸��������
	 HOST->255.255.255.255:1500
	 
	 �豸��Ӧ����
	 FF 01 05 4B
	 FF 01 06 4B
	 FF 01 21 4B
	 FF 01 02 4B
	 Local ip ->HOST
	6.�ָ��������� ��λ���·�
	 FF 01 0B......
 */
void udp_pc_recv_rb(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{   
	  uint8_t first[36]={0xFF,0x24,0x01,0x00,0x00,0xc0,0xA8,0x00,0x94,0x9C,0xA5,0x25,0x9A,0x90,0x1B,\
		0xB2,0x0F,0x00,0x00,0x53,0x53,0x57,0x2D,0x43,0x41,0x4E,0x4F,0x50,0x45,0x2D,0x30,0x30,0x31,0x00,\
		0x00,0x09};
		uint8_t secend[130]={0x95,0x63,0x03,0x40,0x00,0x00,0x50,0x00,0x00,0x07,0x00,0xA8,0xC0,0x01,0x00,0xA8,0xC0,0x00,\
		0xFF,0xFF,0xFF,0x53,0x53,0x57,0x2D,0x43,0x41,0x4E,0x4F,0x50,0x45,0x2D,0x30,0x30,0x31,0x00,0x00,0x61,0x64,0x6D,\
		0x69,0x6E,0x00,0x61,0x64,0x6D,0x69,0x6E,0x00,0x00,0x01,0x00,0xA0,0x9C,0xA5,0x25,0x9A,0x90,0x1B,0xDE,0xDE,0x43,\
		0xD0,0x03,0x00,0x00,0x00,0x00,0xC2,0x01,0x00,0x08,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x2A,0x20,0x31,\
		0x39,0x32,0x2E,0x31,0x36,0x38,0x2E,0x30,0x2E,0x32,0x30,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC9,0x00,0xA8,0xC0,0x20,0x01,0x00,0x04,0x10,0x0E,0x00,0x00,0x00,0x00,0x00,\
		0x00,0x00};
		uint8_t third[85]={0x00,0x1E,0x00,0x0A,0x0D,0x77,0x77,0x77,0x2E,0x75,0x73,0x72,0x2E,0x63,0x6E,0x00,0x00,0x00,\
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
		0x00,0x00,0x00,0x00,0x00,0x77,0x77,0x77,0x2E,0x75,0x73,0x72,0x2E,0x63,0x6E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		uint8_t ack[4]={0xFF,0x01,0x00,0x4B};
		uint8_t mac[6]={0};
		char strip[20]={0};//��λ���·�ip Ϊ�ַ�����ʽ
		uint32_t ipval=0;
		char password[41]={0};//(�û���+��,��+����)����֮�Ͳ��ܳ���40 
		char *pr=NULL;
		uint8_t i;
		struct pbuf *ptr;
	  uint8_t type=0;//��Ϣ����
	  struct ip_addr my_ipaddr;
	  unsigned char *temp = (unsigned char *)addr;
		IP4_ADDR(&my_ipaddr, temp[0], temp[1], temp[2], temp[3]); 
	  type=*((char*)p->payload+2);
	  switch(type)
		{
			case 0x01:
				
					memcpy(&first[5],sysCfg.parameter.ip,6);
				  memcpy(&first[9],sysCfg.parameter.client_mac,6); 
				  ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(first),PBUF_POOL);
				  if(ptr)
				  {
           memcpy(ptr->payload,first,sizeof(first));
           //ptr->payload=(void*)first;	//����ɱ�����			 
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }					    			
				break;
			case 0x02:
				memcpy(mac,(char*)p->payload+3,6);
			  if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
					break;
				ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
			  ack[2]=0x02;
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }					
				if(0 == bsp_WriteCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN))
			    {
				  delay_ms(100);
				  NVIC_SystemReset();		
			    } 
				break;
			case 0x03:
				  memcpy(mac,(char*)p->payload+3,6);
			    if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
						break;
					App_Printf("search:");
					for(i=0;i<6;i++)
			    App_Printf("%02x",mac[i]);
			    App_Printf("\r\n");
					/*���ù���̬ip,0xc0 STATIC 0x40:DHCP*/
          if(sysCfg.parameter.dhcp==STATIC)
						secend[3]=0xc0;
					/*����ip,ת��ΪС��*/
					 for(i=0;i<4;i++)
					 {
						 secend[9+i]=sysCfg.parameter.ip[3-i];
					 }
					 /*��������*/
					 for(i=0;i<4;i++)
					 {
						 secend[13+i]=sysCfg.parameter.gw[3-i];
					 }
					 /*��������*/
					  for(i=0;i<4;i++)
					 {
						 secend[17+i]=sysCfg.parameter.sub[3-i];
					 }
					 /*����Զ�̷�����ip*/
					 sprintf((char*)&secend[83],"%d.%d.%d.%d",sysCfg.parameter.server_ip[0],sysCfg.parameter.server_ip[1],sysCfg.parameter.server_ip[2],sysCfg.parameter.server_ip[3]);
					 secend[81]=sysCfg.parameter.server_port&0xFF;
					 secend[82]=sysCfg.parameter.server_port/256;
					 memcpy(&secend[53],sysCfg.parameter.client_mac,6);
					 ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(secend),PBUF_POOL);
				  if(ptr)
				  {
           memcpy(ptr->payload,secend,sizeof(secend));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
					memcpy(&third[45],sysCfg.parameter.server_user,strlen(sysCfg.parameter.server_user));
					third[45+strlen(sysCfg.parameter.server_user)]=',';
					memcpy(&third[45+1+strlen(sysCfg.parameter.server_user)],sysCfg.parameter.server_pass,strlen(sysCfg.parameter.server_pass));
					ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(third),PBUF_POOL);
				  if(ptr)
				  {
           memcpy(ptr->payload,third,sizeof(third));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
							
			break;
			
			case 0x05:
				memcpy(mac,(char*)p->payload+3,6);
				if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
					break;
				App_Printf("save:");
				for(i=0;i<6;i++)
				App_Printf("%02x",mac[i]);
				App_Printf("\r\n");
				 if(0xc0==*((unsigned char*)p->payload+24))
				 {
					 sysCfg.parameter.dhcp=STATIC;
					 App_Printf("Static\r\n");
			/*����ip,ת��ΪС��*/
				 for(i=0;i<4;i++)
				 {
					sysCfg.parameter.ip[3-i]=*((unsigned char*)p->payload+30+i);
				 }
				 /*��������*/
				 for(i=0;i<4;i++)
				 {
					 sysCfg.parameter.gw[3-i]=*((unsigned char*)p->payload+34+i);
				 }
				 /*��������*/
					for(i=0;i<4;i++)
				 {
					 sysCfg.parameter.sub[3-i]=*((unsigned char*)p->payload+38+i);
				 }
			   }
				 else
				 {
					 sysCfg.parameter.dhcp=DHCP;
					 App_Printf("Dhcp\r\n");
				  }
				  ack[2]=0x05;
				  ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack)); 
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	 
				break;
			case 0x06:
				
				memcpy(mac,(char*)p->payload+3,6);
				if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
					break;
			 /*����Զ�̷�����ip*/
			  strcpy(strip,(const char*)p->payload+37);
				App_Printf("ip:%s\r\n",strip);
				ipval=inet_addr(strip);
				sysCfg.parameter.server_ip[0]=(uint8_t)(ipval);
				sysCfg.parameter.server_ip[1]=(uint8_t)(ipval >> 8);
				sysCfg.parameter.server_ip[2]=(uint8_t)(ipval >> 16);
				sysCfg.parameter.server_ip[3]=(uint8_t)(ipval >> 24);
			  sysCfg.parameter.server_port=(uint16_t)(*((unsigned char*)p->payload+35)|(*((unsigned char*)p->payload+36)<<8));
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
				#endif
				ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
			  ack[2]=0x06;
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
				break;
			case 0x0B:
				  memcpy(mac,(char*)p->payload+3,6);
			    if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
						break;
					ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
			    ack[2]=0x0B;
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
				sysCfg.parameter.config_hold_flag = 0xff;  /*�ָ���������*/
			  sysCfg.parameter.dhcp = DHCP;			 /*Ĭ��Ϊ����DHCP*/
				if(0 == bsp_WriteCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN))
				{
					delay_ms(100);
					NVIC_SystemReset();		
				}
				break;
			case 0x21:
				memcpy(mac,(char*)p->payload+3,6);
				if(0!=memcmp(mac,sysCfg.parameter.client_mac,6))
					break;
				strncpy(password,(const char*)p->payload+66,40);
				App_Printf("password:%s\r\n",password);
				pr=strstr((const char*)password,",");
				if(NULL!=pr)
				{
					memcpy(sysCfg.parameter.server_user,password,pr-password);
					sysCfg.parameter.server_user[pr-password]='\0';
					strncpy(sysCfg.parameter.server_pass,pr+1,40-(pr+1-password));
					sysCfg.parameter.server_pass[40-(pr+1-password)]='\0';
					App_Printf("server username:%s\r\n",sysCfg.parameter.server_user);
				  App_Printf("server password:%s\r\n",sysCfg.parameter.server_pass);
				}
				else
				{
					App_Printf("Format is not correct\r\n");
				}
				
				ptr=pbuf_alloc(PBUF_TRANSPORT,sizeof(ack),PBUF_POOL);
			  ack[2]=0x21;
				  if(ptr)
				  {
           memcpy(ptr->payload,ack,sizeof(ack));
           udp_sendto(upcb,ptr,&my_ipaddr, port);
           pbuf_free(ptr);					 
				  }	
				break;
					
			default:
				break;			
		}	
    pbuf_free(p);
}
/*
*********************************************************************************************************
 * ��������udp_pc_init
 * ����  ��udp ��ʼ�� ��������λ��ͨѶ
 * ����  ����
 * ˵��  : 1.����һ���µ�UDP��  2.��֪��Զ�̷�������ip�Ͷ˿�ʱ ����udp_connect ����ָ����ip�Ͷ˿�����
		3.����ǽ�������ip ������udp_blind ֻ�󶨶˿�
 * ����  : ������UDP��
*********************************************************************************************************
 */
struct udp_pcb * udp_pc_init(void)
{
    err_t err;
    struct udp_pcb *udppcb;
    udppcb = udp_new();
    if(udppcb){
        err=udp_bind(udppcb,IP_ADDR_ANY,1500);
        udp_recv(udppcb , udp_pc_recv_rb , NULL);
    }
		return udppcb;
}

