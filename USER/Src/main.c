/**
  ******************************************************************************
  * @file    DAC/DAC_SimpleConversion/Src/main.c
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    25-November-2016
  * @brief   This example provides a short description of how to use the DAC
  *          peripheral to do a simple conversation.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l0xx_hal.h"
#include "i2c.h"
#include "led_driver.h"
#include "vcc_1v8.h"
#include "spi.h"

I2C_HandleTypeDef I2cHandle;
IWDG_HandleTypeDef hiwdg;

/* Buffer used for transmission */
uint8_t aTxBuffer[RXBUFFERSIZE];

/* Buffer used for reception */
uint8_t aRxBuffer[RXBUFFERSIZE];
__IO uint16_t regAddress = 0;
uint8_t bTransferRequest = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* Private functions ---------------------------------------------------------*/

static void MX_IWDG_Init(void);
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength);
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  HAL_Init();

  /* Configure the system clock to 24 MHz */
  SystemClock_Config();	
  
  I2C_Init();
  VCC_1v8_Init();
  LED_Driver_Init(); 
  
  MX_IWDG_Init();
  
  /* Infinite loop */
  while(1)
  {
    regAddress = 0;

    //    /*##-2- Slave receive request from master ################################*/
    while(HAL_I2C_Slave_Receive_IT(&I2cHandle, (uint8_t*)&bTransferRequest, 1)!= HAL_OK){ }
    
    while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
    {                /* Refresh IWDG: reload counter */
      if(HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
      {
        /* Refresh Error */
      }
    }

    /* If master request write operation #####################################*/
    if (bTransferRequest == MASTER_REQ_WRITE)
    {
      /*##-3- Slave receive number of data to be read ########################*/
      while(HAL_I2C_Slave_Receive_IT(&I2cHandle, (uint8_t*)&regAddress, 1)!= HAL_OK);      
      while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY){ };
      
      /*##-4- Slave receives aRxBuffer from master ###########################*/
      while(HAL_I2C_Slave_Receive_IT(&I2cHandle, (uint8_t*)aRxBuffer, 1)!= HAL_OK);
      while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY){ };
            
      if(regAddress<=0x04)
      {
        LED_Driver_SetValue(regAddress, aRxBuffer[0]);
      }
      
      if(regAddress==0x05)
      {
        aTxBuffer[0]=LED_Driver_GetValue(1);
        while(HAL_I2C_Slave_Transmit_IT(&I2cHandle, (uint8_t*)aTxBuffer, 1)!= HAL_OK);
        while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY){ } 
      }
      
      if((regAddress>=0x06)&&(regAddress<=0x0D))
      {
        AD5317R_DAC_Write(regAddress, aRxBuffer[0]);
      }      
      
      if((regAddress>=0x0E)&&(regAddress<=0x15))
      {
        aTxBuffer[0] = AD5317R_DAC_Read(regAddress);    
        while(HAL_I2C_Slave_Transmit_IT(&I2cHandle, (uint8_t*)aTxBuffer, 1)!= HAL_OK);
        while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY){ } 
      }      
    }
    
    /* Flush Rx buffers */
    Flush_Buffer((uint8_t*)aRxBuffer,RXBUFFERSIZE); 
  }  
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = MSI
  *            SYSCLK(Hz)                     = 2000000
  *            HCLK(Hz)                       = 2000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            Flash Latency(WS)              = 0
  *            Main regulator output voltage  = Scale3 mode
  * @retval None
  */
void SystemClock_Config1(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  
  /* Enable MSI Oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.MSICalibrationValue=0x00;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
  
  /* Select MSI as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  
  /* Disable Power Control clock */
  __HAL_RCC_PWR_CLK_DISABLE();
  
}

void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct ={0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Disable Power Control clock */
  __HAL_RCC_PWR_CLK_DISABLE();
  
  /* Enable HSE Oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue=16;
  RCC_OscInitStruct.HSI48State      = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLSource   = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLMUL      = RCC_PLL_MUL4;
  RCC_OscInitStruct.PLL.PLLDIV      = RCC_PLL_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
}

static void MX_IWDG_Init(void)
{
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Window = 0xfff;
  hiwdg.Init.Reload = 0xfff;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {

  }
}

/**
  * @brief  Flushes the buffer
  * @param  pBuffer: buffers to be flushed.
  * @param  BufferLength: buffer's length
  * @retval None
  */
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    *pBuffer = 0;

    pBuffer++;
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
//static void Error_Handler(void)
//{
//  /* Error if LED2 is slowly blinking (1 sec. period) */
//  while(1)
//  {    
//    //BSP_LED_Toggle(LED2); 
//    HAL_Delay(1000);
//  } 
//}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif
