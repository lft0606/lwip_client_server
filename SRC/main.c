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
		  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	
	delay_init();	    	 			
	uart_init(115200);	 			
	LED_Init();		  					
	KEY_Init();								
	LCD_Init();			   			  
	TIM3_Init(1000,719);			//��ʱ��3Ƶ��Ϊ100hz
	my_mem_init(SRAMIN);			//��ʼ���ڲ��ڴ��
  
	while(lwip_comm_init()) 	//lwip��ʼ��������IP��ַ��MAC��ַ����Ϣ
	{
		LCD_ShowString(30,110,200,20,16,"LWIP Init Falied!");
		delay_ms(1200);
		LCD_Fill(30,110,230,130,WHITE); //�����ʾ
		LCD_ShowString(30,130,200,16,16,"Retrying...");  
	}

#if LWIP_DHCP   //ʹ��DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//�ȴ�DHCP��ȡ�ɹ�/��ʱ���
	{
		lwip_periodic_handle();	//LWIP�ں���Ҫ��ʱ����ĺ���
	}
#endif
	
	load_app_gui(0);//���س�ʼ����
	
 	while(1)
	{	
		key = KEY_Scan(0);
		if(key == KEY0_PRES)		//��KEY1����������
		{
				load_app_gui(1);
				tcp_client_App();		//���Ͽ����Ӻ�,��tcp_client_test()����
		}
		else if(key == KEY1_PRES)		//��KEY1����������
		{
				load_app_gui(2);
				tcp_server_App();		//���Ͽ����Ӻ�,��tcp_client_test()����
		}
		delay_ms(10);
	}
}


