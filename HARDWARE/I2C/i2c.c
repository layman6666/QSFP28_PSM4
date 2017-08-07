#include "i2c.h"


extern I2C_HandleTypeDef I2cHandle;

void I2C_Init(void)
{  
  I2cHandle.Instance             = I2Cx;
  I2cHandle.Init.Timing          = I2C_TIMING;
  I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2cHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  I2cHandle.Init.OwnAddress2     = 0xFF;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;  

  if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
  {

  }

  /* Enable the Analog I2C Filter */
  //HAL_I2CEx_ConfigAnalogFilter(&I2cHandle,I2C_ANALOGFILTER_ENABLE);

  if (HAL_I2CEx_ConfigAnalogFilter(&I2cHandle, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {

  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&I2cHandle, 0) != HAL_OK)
  {
    
  }
}


