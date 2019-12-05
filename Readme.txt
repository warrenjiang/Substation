2019.11.28
1.Canopen 通信正常
2.Lwip运行正常,与云服务器MQTT通信正常（运行较长时间出现硬件中断？）
3.实现DHCP功能
4.获取NTP时间戳，UDP通信正常
5.远程升级固件正常
2019.11.29
1.实现两个端口通信 50000端口连云服务器  60000连第三方服务器
2.完善指示灯状态，心跳超时重连
3.可通过云平台配置连接第三方服务器参数

2019.12.02
1.ip正常获取的情况下，20秒内连不上主服务器，设备重启
check_count++;
			if(check_count>20)
			{
				check_count=0;
				App_Printf("Fail to connect Main_Socket...\r\n");
				NVIC_SystemReset();		
			}		
			
2.检测到网线被拔掉，重启设备，重新获取ip		
if(EXTI_GetITStatus(EXTI_Line6) != RESET)
  {
		
		EXTI_ClearITPendingBit(EXTI_Line6);
		if(((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_MISR)) & PHY_LINK_STATUS) != 0)
      {
        if(0==(ETH_ReadPHYRegister(PHY_ADDRESS, PHY_SR) & 1))//ÍøÏßÎ´²åÉÏ
        {  
						 delay_ms(100);
				     NVIC_SystemReset();	
        }
      }
				 
  }
2019.12.05
1.删除12.02号第一点
2.兼容有人上位机工具 目前支持搜索局域网内所有网关设备，查看配置参数，下发配置参数（DHCP/STATIC 本地ip, 远程ip）  支持设备重启，恢复出厂设置
3.增加了Lwip heap空间
 #define MEM_SIZE                (10*1024)
4.上报Can数据 每秒推送100条 26字节 格式2+1+8+1+2+2+4+4+2 目前较稳定


