#include "spi.h"
#include "main.h"
#include "stm32l0xx_hal.h"

void SPI_Init(void)
{  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  SPI_SCK_GPIO_CLK_ENABLE();
  SPI_MISO_GPIO_CLK_ENABLE();
  SPI_MOSI_GPIO_CLK_ENABLE();
  SPI_NSS_GPIO_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* SPI SCK GPIO pin configuration  */
  GPIO_InitTypeDef GPIO_InitStruct;
  
  GPIO_InitStruct.Pin       = SPI_SCK_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  //GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI_SCK_GPIO_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(SPI_SCK_GPIO_PORT, SPI_SCK_PIN, GPIO_PIN_SET);

  /* SPI MISO GPIO pin configuration  */
  GPIO_InitStruct.Pin = SPI_MISO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(SPI_MISO_GPIO_PORT, &GPIO_InitStruct);

  /* SPI MOSI GPIO pin configuration  */
  GPIO_InitStruct.Pin = SPI_MOSI_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(SPI_MOSI_GPIO_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(SPI_MOSI_GPIO_PORT, SPI_MOSI_PIN, GPIO_PIN_SET);
  
  GPIO_InitStruct.Pin = SPI_NSS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(SPI_NSS_GPIO_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(SPI_NSS_GPIO_PORT, SPI_NSS_PIN, GPIO_PIN_SET);
}

uint8_t SPI_ReadWrite_Byte(uint8_t byte)
{  
  uint8_t temp=0;
  NSS_L;  
  SCK_H;    
  __NOP();             /*��ȡ��һbit���� �ȴ������ȶ� ����ʵ��ʱ�ӵ���*/
  for(uint8_t i=0;i<8;i++)
  {    
    if(byte&0x80) 
    {
      MOSI_H;          /*�����λΪ�ߣ��������*/
    }
    else
    {
      MOSI_L;          /*�����λΪ�ͣ��������*/    
    }    
    __NOP();
    byte <<= 1;
    SCK_L;
    temp <<= 1;        /*��������*/
    if(MISO)
    {
      temp++;          /*���Ӵӻ����յ��ߵ�ƽ�������Լ�һ*/
    }
    SCK_H;
  }
  NSS_H;
  return temp;
}