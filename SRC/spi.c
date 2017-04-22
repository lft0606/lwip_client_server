#include "spi.h"
	  
/****************************************************************************
FunctionName:  SPI1_Init(void)//SPI1��ʼ��
Author  		:  LFT
Date    		:	 20170419
Description	:  TxData:Ҫд����ֽ�;����ֵ:��ȡ�����ֽ�
History			:  ��ʷ�޸ļ�¼
1��LFT 2017/04/16 �����ú���
*****************************************************************************/
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;											//SPI����
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;									//���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;												//ʱ�����յ�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;											//���ݲ����ڵ�1��ʱ����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;													//NSS�ź����������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//����Ϊ8��Ƶ������ping��ͨ
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;								//���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;													//CRCֵ����Ķ���ʽ
	SPI_Init(SPI1, &SPI_InitStructure);  				
	
	SPI_Cmd(SPI1, ENABLE);
}   

/*************************************************************************************
FunctionName:  SPI1_ReadWriteByte(u8 TxData)//��дһ���ֽ�
Author  		:  LFT
Date    		:	 20170419
Description	:  TxData:Ҫд����ֽ�;����ֵ:��ȡ�����ֽ�
History			:  ��ʷ�޸ļ�¼
1��LFT 2017/04/16 �����ú���
*************************************************************************************/
u8 SPI1_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		retry++;
		if(retry>200)
		{
			return 0;
		}
	}			  
	SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		retry++;
		if(retry>200)
		{
			return 0;
		}
	}	  						    
	return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����					    
}
