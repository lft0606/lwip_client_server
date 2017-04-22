#ifndef _LWIP_COMM_H
#define _LWIP_COMM_H 
#include "enc28j60.h" 

extern u8 tcp_flag;
extern u8 tcp_recvbuf[];								 //TCP Client接收数据缓冲区
extern u8 *tcp_sendbuf;//TCP服务器发送数据内容
extern u32 lwip_localtime;	//lwip本地时间计数器,单位:ms

#define TCP_RX_BUFSIZE	1500	//定义tcp client最大接收数据长度
#define TCP_TX_BUFSIZE	200		//定义tcp client最大发送数据长度

#define LWIP_MAX_DHCP_TRIES		4   //DHCP服务器最大重试次数

//lwip控制结构体
typedef struct  
{
	u8 mac[6];      //MAC地址
	u8 remoteip[4];	//远端主机IP地址 
	u8 ip[4];       //本机IP地址
	u8 netmask[4]; 	//子网掩码
	u8 gateway[4]; 	//默认网关的IP地址
	
	vu8 dhcpstatus;	//dhcp状态 
					//0,未获取DHCP地址;
					//1,进入DHCP获取状态
					//2,成功获取DHCP地址
					//0XFF,获取失败.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip控制结构体

u8 lwip_comm_mem_malloc(void);
void lwip_comm_mem_free(void);
void lwip_comm_default_ip_set(__lwip_dev *lwipx);
u8 lwip_comm_init(void);
void lwip_pkt_handle(void);
void lwip_periodic_handle(void);
void lwip_dhcp_process_handle(void);
void load_app_gui(u8 mode);
#endif













