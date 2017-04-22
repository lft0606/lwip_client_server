#include "delay.h"

static u8  fac_us=0;							//us延时倍乘数			   
static u16 fac_ms=0;							//ms延时倍乘数,在ucos下,代表每个节拍的ms数
		   
/*************************************************************************************
FunctionName: void delay_init()//初始化延迟函数
Author  		:  LFT
Date    		:	 20170419
Description	:  //nus为要延时的us数.
History			:  历史修改记录
1、LFT 2017/04/16 创建该函数
*************************************************************************************/
void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
	fac_us=SystemCoreClock/8000000;												//为系统时钟的1/8  
	fac_ms=(u16)fac_us*1000;															//非OS下,代表每个ms需要的systick时钟数   
}								    

/*************************************************************************************
FunctionName: void delay_us(u32 nus)//延时nus
Author  		:  LFT
Date    		:	 20170419
Description	:  //nus为要延时的us数.
History			:  历史修改记录
1、LFT 2017/04/16 创建该函数
*************************************************************************************/
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					//时间加载	  		 
	SysTick->VAL=0x00;        					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;      					 //清空计数器	 
}


/*************************************************************************************
FunctionName: void delay_ms(u16 nms)//延时nms
Author  		:  LFT
Date    		:	 20170419
Description	:  对72M条件下,nms<=1864
History			:  历史修改记录
1、LFT 2017/04/16 创建该函数
*************************************************************************************/
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;       					//清空计数器	  	    
} 
