#include "spi.h"
	  
/****************************************************************************
FunctionName:  SPI1_Init(void)//SPI1初始化
Author  		:  LFT
Date    		:	 20170419
Description	:  TxData:要写入的字节;返回值:读取到的字节
History			:  历史修改记录
1、LFT 2017/04/16 创建该函数
*****************************************************************************/
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;											//SPI主机
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;									//发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;												//时钟悬空低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;											//数据捕获于第1个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;													//NSS信号由软件控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//必须为8分频，否则ping不通
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;								//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;													//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  				
	
	SPI_Cmd(SPI1, ENABLE);
}   

/*************************************************************************************
FunctionName:  SPI1_ReadWriteByte(u8 TxData)//读写一个字节
Author  		:  LFT
Date    		:	 20170419
Description	:  TxData:要写入的字节;返回值:读取到的字节
History			:  历史修改记录
1、LFT 2017/04/16 创建该函数
*************************************************************************************/
u8 SPI1_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
		retry++;
		if(retry>200)
		{
			return 0;
		}
	}			  
	SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个数据
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
	{
		retry++;
		if(retry>200)
		{
			return 0;
		}
	}	  						    
	return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据					    
}
