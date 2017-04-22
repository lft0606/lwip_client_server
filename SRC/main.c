#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "malloc.h"
#include "enc28j60.h" 	 
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "timer.h"
#include "tcp_client_demo.h"
#include "tcp_server_demo.h"

int main(void)
{	
	u8 key;	
		  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	
	delay_init();	    	 			
	uart_init(115200);	 			
	LED_Init();		  					
	KEY_Init();								
	LCD_Init();			   			  
	TIM3_Init(1000,719);			//定时器3频率为100hz
	my_mem_init(SRAMIN);			//初始化内部内存池
  
	while(lwip_comm_init()) 	//lwip初始化，包括IP地址、MAC地址等信息
	{
		LCD_ShowString(30,110,200,20,16,"LWIP Init Falied!");
		delay_ms(1200);
		LCD_Fill(30,110,230,130,WHITE); //清除显示
		LCD_ShowString(30,130,200,16,16,"Retrying...");  
	}

#if LWIP_DHCP   //使用DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//等待DHCP获取成功/超时溢出
	{
		lwip_periodic_handle();	//LWIP内核需要定时处理的函数
	}
#endif
	
	load_app_gui(0);//加载初始界面
	
 	while(1)
	{	
		key = KEY_Scan(0);
		if(key == KEY0_PRES)		//按KEY1键建立连接
		{
				load_app_gui(1);
				tcp_client_App();		//当断开连接后,调tcp_client_test()函数
		}
		else if(key == KEY1_PRES)		//按KEY1键建立连接
		{
				load_app_gui(2);
				tcp_server_App();		//当断开连接后,调tcp_client_test()函数
		}
		delay_ms(10);
	}
}


