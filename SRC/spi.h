#ifndef __SPI_H
#define __SPI_H
#include "sys.h"
			  	    													  
void SPI1_Init(void);			 //初始化SPI口   
u8 SPI1_ReadWriteByte(u8 TxData);//SPI总线读写一个字节
		  
#endif

