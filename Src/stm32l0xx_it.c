/**
  ******************************************************************************
  * @file    stm32l0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
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
#include "stm32l0xx_hal.h"
#include "stm32l0xx.h"
#include "stm32l0xx_it.h"
#include "process.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_FS;
extern ADC_HandleTypeDef hadc;
extern DMA_HandleTypeDef hdma_lpuart1_rx;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart1;
extern RTC_HandleTypeDef hrtc;

/******************************************************************************/
/*            Cortex-M0+ Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles Non maskable Interrupt.
*/
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
	uint32_t i;

	i = sprintf((char*)&blinkMessage[0], "Hardware fault!\r\n");
	CDC_Transmit_FS(blinkMessage, i);
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
  /* USER CODE BEGIN HardFault_IRQn 1 */

  /* USER CODE END HardFault_IRQn 1 */
}

/**
* @brief This function handles System service call via SWI instruction.
*/
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
* @brief This function handles Pendable request for system service.
*/
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32L0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l0xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles RTC global interrupt through EXTI lines 17, 19 and 20 and LSE CSS interrupt through EXTI line 19.
*/
void RTC_IRQHandler(void)
{
  /* USER CODE BEGIN RTC_IRQn 0 */

  /* USER CODE END RTC_IRQn 0 */
  HAL_RTC_AlarmIRQHandler(&hrtc);
  HAL_RTCEx_WakeUpTimerIRQHandler(&hrtc);
  /* USER CODE BEGIN RTC_IRQn 1 */

  /* USER CODE END RTC_IRQn 1 */
}

/**
* @brief This function handles EXTI line 4 to 15 interrupts.
*/
void EXTI4_15_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
  
    spiSendData[0] = (0x02 << 1) | 0x01;		// C7:C1=Device ID register address, C0=read

	HAL_SPI_Transmit(&hspi1, spiSendData, 1, 50);
	HAL_SPI_Receive(&hspi1, spiReceiveData, 1, 50);

	uint8_t status = 0;
	if (spiReceiveData[0] == 0xFA)
		status = 1;
	else
		status = 0;

	uint16_t x = 0, y = 0, z = 0;

	if (status == 1)
	{
		spiSendData[0] = (0x3F << 1) | 0x00;		// C7:C1=Device ID register address, C0=read
		spiSendData[1] = 0x03;
		HAL_SPI_Transmit(&hspi1, spiSendData, 2, 50);
		HAL_Delay(370);

		spiSendData[0] = (0x08 << 1) | 0x01;		// C7:C1=Device ID register address, X value read

		HAL_SPI_Transmit(&hspi1, spiSendData, 1, 50);
		HAL_SPI_Receive(&hspi1, spiReceiveData, 2, 50);
		x = (uint16_t)(spiReceiveData[0]) << 4 + (uint16_t)(spiReceiveData[1]) >> 4;

		HAL_Delay(50);
		spiSendData[0] = (0x0A << 1) | 0x01;		// C7:C1=Device ID register address, X value read
		HAL_SPI_Transmit(&hspi1, spiSendData, 1, 50);
		HAL_SPI_Receive(&hspi1, spiReceiveData, 2, 50);
		y = (uint16_t)(spiReceiveData[0]) << 4 + (uint16_t)(spiReceiveData[1]) >> 4;

		HAL_Delay(50);
		spiSendData[0] = (0x0C << 1) | 0x01;		// C7:C1=Device ID register address, X value read
		HAL_SPI_Transmit(&hspi1, spiSendData, 1, 50);
		HAL_SPI_Receive(&hspi1, spiReceiveData, 2, 50);
		z = (uint16_t)(spiReceiveData[0]) << 4 + (uint16_t)(spiReceiveData[1]) >> 4;


		HAL_Delay(50);
		//n = sprintf((char*)&blinkMessage[0], "Motion=> X: %d, Y: %d, Z: %d\r\n", x, y, z);
		//CDC_Transmit_FS(blinkMessage, n);

	}
	else
	{
		//n = sprintf((char*)&blinkMessage[0], "The motion sensor is not installed.\r\n");
		//CDC_Transmit_FS(blinkMessage, n);
	}

	spiSendData[0] = (0x3F << 1) | 0x00;		// C7:C1=Device ID register address, C0=read
	spiSendData[1] = 0x00;
	HAL_SPI_Transmit(&hspi1, spiSendData, 2, 50);
    
    if (x > X_THRESHOLD || y > Y_THRESHOLD || z > Z_THRESHOLD)
    {
        BootBG96();
        getGPSData();
    }
	
    RTC_AlarmTypeDef sAlarm;// Add the interval to the current time
	uint8_t alarmHour = 0;
	uint32_t interval = 7;
	
	alarmHour = sTime.Minutes + (uint8_t)((interval >> 16) & 0xFF);
	// if alarmHour is larger than 23, then set to 0 and increase the weekday
	if (alarmHour > 59)
	{
		alarmHour -= 60;
		if (sTime.Hours == 23)
		{
			sAlarm.AlarmTime.Hours = 0;
			sAlarm.AlarmDateWeekDay = sDate.WeekDay + 1;
			if (sAlarm.AlarmDateWeekDay > RTC_WEEKDAY_SUNDAY)
				sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
		}
		else
			sAlarm.AlarmTime.Hours = sTime.Hours + 1;
	}
	else
	{
		sAlarm.AlarmTime.Hours = sTime.Hours;
		sAlarm.AlarmDateWeekDay = sDate.WeekDay;
	}

	// set the alarm time
	sAlarm.AlarmTime.Minutes = alarmHour;
	sAlarm.AlarmTime.Seconds = sTime.Seconds;
	sAlarm.AlarmTime.SubSeconds = sTime.SubSeconds;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_PM;
	sAlarm.Alarm = RTC_ALARM_A;

	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD);

	n = sprintf((char*)&blinkMessage[0], "=>Measurement: %d Time:%d:%d   alarm:%d:%d.\r\n", sDate.WeekDay, sTime.Hours, sTime.Minutes, sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes);
    CDC_Transmit_FS(blinkMessage, n);

	HAL_Delay(500);

	//__HAL_RCC_PWR_CLK_ENABLE(); // Enable Power Control clock
	HAL_PWREx_EnableUltraLowPower(); // Ultra low power mode
	HAL_PWREx_EnableFastWakeUp(); // Fast wake-up for ultra low power mode
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		//HAL_PWR_EnterSLEEPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	//HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	HAL_PWR_EnterSTANDBYMode();
}

/**
* @brief This function handles DMA1 channel 2 and channel 3 interrupts.
*/
void DMA1_Channel2_3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_3_IRQn 0 */

  /* USER CODE END DMA1_Channel2_3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_lpuart1_rx);
  /* USER CODE BEGIN DMA1_Channel2_3_IRQn 1 */

  /* USER CODE END DMA1_Channel2_3_IRQn 1 */
}

/**
* @brief This function handles DMA1 channel 4, channel 5, channel 6 and channel 7 interrupts.
*/
void DMA1_Channel4_5_6_7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel4_5_6_7_IRQn 0 */

  /* USER CODE END DMA1_Channel4_5_6_7_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA1_Channel4_5_6_7_IRQn 1 */

  /* USER CODE END DMA1_Channel4_5_6_7_IRQn 1 */
}

/**
* @brief This function handles ADC1, COMP1 and COMP2 interrupts (COMP interrupts through EXTI lines 21 and 22).
*/
void ADC1_COMP_IRQHandler(void)
{
  /* USER CODE BEGIN ADC1_COMP_IRQn 0 */

  /* USER CODE END ADC1_COMP_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc);
  /* USER CODE BEGIN ADC1_COMP_IRQn 1 */

  /* USER CODE END ADC1_COMP_IRQn 1 */
}

/**
* @brief This function handles USART1 global interrupt / USART1 wake-up interrupt through EXTI line 25.
*/
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
* @brief This function handles AES, RNG and LPUART1 interrupts / LPUART1 wake-up interrupt through EXTI line 28.
*/
void AES_RNG_LPUART1_IRQHandler(void)
{
  /* USER CODE BEGIN AES_RNG_LPUART1_IRQn 0 */

  /* USER CODE END AES_RNG_LPUART1_IRQn 0 */
  HAL_UART_IRQHandler(&hlpuart1);
  /* USER CODE BEGIN AES_RNG_LPUART1_IRQn 1 */

 	receiveIndexEnd ++;
	flag = 0;
	if (receiveIndexEnd == 1000)
	    receiveIndexEnd = 0;
  /* USER CODE END AES_RNG_LPUART1_IRQn 1 */
}

/**
* @brief This function handles USB event interrupt / USB wake-up interrupt through EXTI line 18.
*/
void USB_IRQHandler(void)
{
  /* USER CODE BEGIN USB_IRQn 0 */

  /* USER CODE END USB_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
  /* USER CODE BEGIN USB_IRQn 1 */

  /* USER CODE END USB_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
