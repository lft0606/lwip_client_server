#include "lwip_comm.h" 
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "malloc.h"
#include "delay.h"
#include "usart.h"  
#include <stdio.h>
#include "lcd.h"

//TCP ����ȫ��״̬��Ǳ���
//bit7:0,û������Ҫ����;1,������Ҫ���� 
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û�������Ϸ�����;1,�����Ϸ�������.
//bit4~0:����
u8 tcp_flag;
u8 tcp_recvbuf[TCP_RX_BUFSIZE];								 //TCP Client�������ݻ�����
u8 *tcp_sendbuf = "ENC28J60 TCP send data\r\n";//TCP������������������

__lwip_dev lwipdev;						//lwip���ƽṹ�� 
struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�

extern u32 memp_get_memorysize(void);	//��memp.c���涨��
extern u8_t *memp_memory;				//��memp.c���涨��.
extern u8_t *ram_heap;					//��mem.c���涨��.

u32 TCPTimer=0;				//TCP��ѯ��ʱ��
u32 ARPTimer=0;				//ARP��ѯ��ʱ��
u32 lwip_localtime;		//lwip����ʱ�������,��λ:ms

#if LWIP_DHCP
u32 DHCPfineTimer=0;		//DHCP��ϸ�����ʱ��
u32 DHCPcoarseTimer=0;	//DHCP�ֲڴ����ʱ��
#endif

  
/*************************************************************************************
FunctionName: u8 lwip_comm_mem_malloc(void)//lwip��mem��memp���ڴ�����
Author  		:  
Date    		:	
Description	:  ����ֵ:0,�ɹ�;����,ʧ��
History			:  ��ʷ�޸ļ�¼
*************************************************************************************/
u8 lwip_comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	mempsize=memp_get_memorysize();					//�õ�memp_memory�����С
	memp_memory=mymalloc(SRAMIN,mempsize);	//Ϊmemp_memory�����ڴ�
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//�õ�ram heap��С
	ram_heap=mymalloc(SRAMIN,ramheapsize);	//Ϊram_heap�����ڴ� 
	if(!memp_memory||!ram_heap)//������ʧ�ܵ�
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}


/*************************************************************************************
FunctionName: void lwip_comm_mem_free(void)//lwip��mem��memp�ڴ��ͷ�
Author  		:  
Date    		:	
Description	:  ����ֵ:0,�ɹ�;����,ʧ��
History			:  ��ʷ�޸ļ�¼
*************************************************************************************/
void lwip_comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
	myfree(SRAMIN,ram_heap);
}

/*************************************************************************************
FunctionName: void lwip_comm_default_ip_set(__lwip_dev *lwipx)//lwip Ĭ��Զ��IP����
Author  		:  
Date    		:	
Description	: lwipx:lwip���ƽṹ��ָ��
History			: ��ʷ�޸ļ�¼//Ĭ��Զ��IPΪ:192.168.1.100
*************************************************************************************/
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=103;
	
	//MAC��ַ����(�����ֽڹ̶�Ϊ:2.0.0,�����ֽ���STM32ΨһID)
	lwipx->mac[0]=enc28j60_dev.macaddr[0];
	lwipx->mac[1]=enc28j60_dev.macaddr[1];
	lwipx->mac[2]=enc28j60_dev.macaddr[2];
	lwipx->mac[3]=enc28j60_dev.macaddr[3];
	lwipx->mac[4]=enc28j60_dev.macaddr[4];
	lwipx->mac[5]=enc28j60_dev.macaddr[5];
	
	//Ĭ�ϱ���IPΪ:192.168.1.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=1;
	lwipx->ip[3]=30;
	
	//Ĭ����������:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	
	//Ĭ������:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//û��DHCP	
} 
    
/*************************************************************************************
FunctionName: u8 lwip_comm_init(void)//LWIP��ʼ��(LWIP������ʱ��ʹ��)
Author  		:  
Date    		:	
Description	: ����ֵ:0,�ɹ���1,�ڴ����2,DM9000��ʼ��ʧ�ܣ�3,�������ʧ��.
History			: ��ʷ�޸ļ�¼
*************************************************************************************/
u8 lwip_comm_init(void)
{
	struct netif *Netif_Init_Flag;		//����netif_add()����ʱ�ķ���ֵ,�����ж������ʼ���Ƿ�ɹ�
	struct ip_addr ipaddr;  					//ip��ַ
	struct ip_addr netmask; 					//��������
	struct ip_addr gw;      					//Ĭ������ 
	
	if(lwip_comm_mem_malloc())
	{
		return 1;		//�ڴ�����ʧ��
	}
	if(ENC28J60_Init())
	{
		return 2;		//��ʼ��ENC28J60
	}
	
	lwip_init();												//��ʼ��LWIP�ں�
	lwip_comm_default_ip_set(&lwipdev);	//����Ĭ��IP����Ϣ

#if LWIP_DHCP		//ʹ�ö�̬IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//ʹ�þ�̬IP
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&ethernet_input);//�������б������һ������
	
#if LWIP_DHCP			//���ʹ��DHCP�Ļ�
	lwipdev.dhcpstatus=0;			//DHCP���Ϊ0
	dhcp_start(&lwip_netif);	//����DHCP����
#endif
	
	if(Netif_Init_Flag==NULL)
	{
		return 3;//�������ʧ�� 
	}
	else//������ӳɹ���,����netifΪĬ��ֵ,���Ҵ�netif����
	{
		netif_set_default(&lwip_netif); //����netifΪĬ������
		netif_set_up(&lwip_netif);			//��netif����
	}
	return 0;//����OK.
}   

/*************************************************************************************
FunctionName: void lwip_pkt_handle(void)//�����绺�����ж�ȡ���յ������ݰ������䷢�͸�LWIP����
Author  		:  
Date    		:	
Description	: �����յ����ݺ����
History			: ��ʷ�޸ļ�¼
*************************************************************************************/
void lwip_pkt_handle(void)
{	 
	ethernetif_input(&lwip_netif);
}

/*************************************************************************************
FunctionName: void lwip_periodic_handle()//LWIP������ѯ����
Author  		:  
Date    		:	
Description	: 
History			: ��ʷ�޸ļ�¼
*************************************************************************************/
void lwip_periodic_handle()
{	
#if LWIP_TCP
	//ÿ250ms����һ��tcp_tmr()����
	if (lwip_localtime - TCPTimer >= TCP_TMR_INTERVAL)
	{
		TCPTimer =  lwip_localtime;
		tcp_tmr();
	}
#endif
	//ARPÿ5s�����Ե���һ��
	if ((lwip_localtime - ARPTimer) >= ARP_TMR_INTERVAL)
	{
		ARPTimer =  lwip_localtime;
		etharp_tmr();
	}

#if LWIP_DHCP //���ʹ��DHCP�Ļ�
	//ÿ500ms����һ��dhcp_fine_tmr()
	if (lwip_localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
	{
		DHCPfineTimer =  lwip_localtime;
		dhcp_fine_tmr();
		if ((lwipdev.dhcpstatus != 2)&&(lwipdev.dhcpstatus != 0XFF))
		{ 
			lwip_dhcp_process_handle();  //DHCP����
		}
	}

	//ÿ60sִ��һ��DHCP�ֲڴ���
	if (lwip_localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
	{
		DHCPcoarseTimer =  lwip_localtime;
		dhcp_coarse_tmr();
	}  
#endif
}


#if LWIP_DHCP			//���ʹ����DHCP
/*************************************************************************************
FunctionName: void lwip_dhcp_process_handle(void)//DHCP��������
Author  		:  
Date    		:	
Description	: 
History			: ��ʷ�޸ļ�¼
*************************************************************************************/
void lwip_dhcp_process_handle(void)
{
	u32 ip=0,netmask=0,gw=0;
	switch(lwipdev.dhcpstatus)
	{
		case 0: 	//����DHCP
			dhcp_start(&lwip_netif);
			lwipdev.dhcpstatus = 1;		//�ȴ�ͨ��DHCP��ȡ���ĵ�ַ
			printf("���ڲ���DHCP������,���Ե�...........\r\n");  
			break;
		case 1:		//�ȴ���ȡ��IP��ַ
		{
			ip=lwip_netif.ip_addr.addr;		//��ȡ��IP��ַ
			netmask=lwip_netif.netmask.addr;//��ȡ��������
			gw=lwip_netif.gw.addr;			//��ȡĬ������ 
			
			if(ip!=0)			//��ȷ��ȡ��IP��ַ��ʱ��
			{
				lwipdev.dhcpstatus=2;	//DHCP�ɹ�
				printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				//������ͨ��DHCP��ȡ����IP��ַ
				lwipdev.ip[3]=(uint8_t)(ip>>24); 
				lwipdev.ip[2]=(uint8_t)(ip>>16);
				lwipdev.ip[1]=(uint8_t)(ip>>8);
				lwipdev.ip[0]=(uint8_t)(ip);
				printf("ͨ��DHCP��ȡ��IP��ַ..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				//����ͨ��DHCP��ȡ�������������ַ
				lwipdev.netmask[3]=(uint8_t)(netmask>>24);
				lwipdev.netmask[2]=(uint8_t)(netmask>>16);
				lwipdev.netmask[1]=(uint8_t)(netmask>>8);
				lwipdev.netmask[0]=(uint8_t)(netmask);
				printf("ͨ��DHCP��ȡ����������............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				//������ͨ��DHCP��ȡ����Ĭ������
				lwipdev.gateway[3]=(uint8_t)(gw>>24);
				lwipdev.gateway[2]=(uint8_t)(gw>>16);
				lwipdev.gateway[1]=(uint8_t)(gw>>8);
				lwipdev.gateway[0]=(uint8_t)(gw);
				printf("ͨ��DHCP��ȡ����Ĭ������..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			}
			else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
			{
				lwipdev.dhcpstatus=0XFF;//DHCP��ʱʧ��.
				//ʹ�þ�̬IP��ַ
				IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
				printf("DHCP����ʱ,ʹ�þ�̬IP��ַ!\r\n");
				printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
				printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
				printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
				printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			}
		}
		break;
		default : break;
	}
}
#endif

void load_app_gui(u8 mode)
{
	switch(mode)
	{
		case 0:
			LCD_Clear(WHITE);	//����
			POINT_COLOR=RED; 	//��ɫ����
			LCD_ShowString(30,30,200,16,16,"ENC28J60+STM32");
			LCD_ShowString(30,50,200,20,16,"LWIP Init Success!!!");
			LCD_ShowString(30,70,200,16,16,"KEY0: TCP Client Test");
			LCD_ShowString(30,90,200,16,16,"KEY1: TCP Server Test");    
			LCD_ShowString(30,110,200,16,16,"KEY_UP:Quit"); 
			break;
		
		case 1:
			LCD_Clear(WHITE);	//����
			POINT_COLOR=RED; 	//��ɫ����
			LCD_ShowString(30,30,200,16,16,"ENC28J60+STM32");
			LCD_ShowString(30,50,200,20,16,"LWIP Init Success!!!");
			LCD_ShowString(30,70,200,16,16,"TCP Client is connecting...;");
			LCD_ShowString(30,90,200,16,16,"KEY0: Client send data to Server;");    
			LCD_ShowString(30,110,200,16,16,"KEY_UP:Quit");
			break;
		
		case 2:
			LCD_Clear(WHITE);	//����
			POINT_COLOR=RED; 	//��ɫ����
			LCD_ShowString(30,30,200,16,16,"ENC28J60+STM32");
			LCD_ShowString(30,50,200,20,16,"LWIP Init Success!!!");
			LCD_ShowString(30,70,200,16,16,"TCP Server is connecting...;");
			LCD_ShowString(30,90,200,16,16,"KEY1: Server send data to Client;");    
			LCD_ShowString(30,110,200,16,16,"KEY_UP:Quit");
			break;
		
		default:
			break;
	}
}
