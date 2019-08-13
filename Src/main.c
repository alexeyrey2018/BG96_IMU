
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l0xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_lpuart1_rx;
DMA_HandleTypeDef hdma_usart1_rx;

RTC_HandleTypeDef hrtc;

int main(void)
{
/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USB_DEVICE_Init();

	uint32_t status = PWR->CSR;

	//if ((status & PWR_CSR_WUF_Msk) == 0x00)


	// Check the status of the standby flag
	//if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == RESET)

	if (__HAL_PWR_GET_FLAG(PWR_FLAG_WU) == RESET)
	{
 		MX_RTC_Init();
		IoTCommunicationCase = 1;
		SensorMeasurementCase = 0;

		MX_SPI1_Init();
		MX_ADC_Init();
		// Initialization
		HAL_GPIO_WritePin(UC_SENS_SPI_SS0_GPIO_Port, UC_SENS_SPI_SS0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(UC_SENS_SPI_SS1_GPIO_Port, UC_SENS_SPI_SS1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(UC_SENS_SPI_SS2_GPIO_Port, UC_SENS_SPI_SS2_Pin, GPIO_PIN_SET);

		HAL_Delay(500);
		spiSendData[0] = 0x08;		// C7:0, C6:0,C5C4C3:0x01, C2C1C0:0x00
		spiSendData[1] |= 0x60;
		HAL_SPI_Transmit(&hspi1, spiSendData, 2, 50);

		HAL_GPIO_WritePin(UC_SENS_SPI_SS0_GPIO_Port, UC_SENS_SPI_SS0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(UC_SENS_SPI_SS1_GPIO_Port, UC_SENS_SPI_SS1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(UC_SENS_SPI_SS2_GPIO_Port, UC_SENS_SPI_SS2_Pin, GPIO_PIN_SET);
		HAL_Delay(10);
		spiSendData[0] = (0x3F << 1) | 0x00;		// C7:C1=Device ID register address, C0=read
		spiSendData[1] = 0x00;
		HAL_SPI_Transmit(&hspi1, spiSendData, 2, 50);
		HAL_GPIO_WritePin(UC_SENS_SPI_SS1_GPIO_Port, UC_SENS_SPI_SS1_Pin, GPIO_PIN_SET);

		HAL_Delay(500);

		HAL_ADC_DeInit(&hadc);
		HAL_SPI_DeInit(&hspi1);
	}
	else
	{
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

		//if(__HAL_RTC_ALARM_GET_FLAG(&hrtc, RTC_FLAG_ALRAF) != RESET)
		if ((RTC->ISR & RTC_ISR_ALRAF_Msk) == RTC_ISR_ALRAF_Msk)
		{
			SensorMeasurementCase = 1;
		}
		else if ((RTC->ISR & RTC_ISR_ALRBF_Msk) == RTC_ISR_ALRBF_Msk)
		{
			IoTCommunicationCase = 1;
		}

		MX_RTC_Wakeup_Init();
	}

	/*
	HAL_Delay(1000);
    n = sprintf((char*)&blinkMessage[0], "PWR->CSR = %d %d\r\n", status, PWR_CSR_WUF_Msk);
    CDC_Transmit_FS(blinkMessage, n);

	status = RTC->ISR;
	HAL_Delay(1000);
    n = sprintf((char*)&blinkMessage[0], "hrtc->ISR = %x\r\n", status);
    CDC_Transmit_FS(blinkMessage, n);
	*/

	HAL_Delay(1000);
	// n = sprintf((char*)&blinkMessage[0], "Case = %d %d\r\n", IoTCommunicationCase, SensorMeasurementCase);
	//CDC_Transmit_FS(blinkMessage, n);

	HAL_Delay(1000);

	while (1)
	{
		
		if (IoTCommunicationCase == 1)
		{
			IoTCommunicationCase = 0;
			IoTCommunicationProcess();
		}
	}

}

/* USER CODE BEGIN 4 */
char error[100];
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
