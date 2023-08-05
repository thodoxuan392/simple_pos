/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Hal/clock.h"
#include "Hal/gpio.h"
#include "Hal/timer.h"
#include "Hal/i2c.h"
#include "Hal/uart.h"
#include "Device/eeprom.h"
#include "Device/keypad.h"
#include "Device/billacceptor.h"
#include "Device/rtc.h"
#include "Device/lcd.h"
#include "Device/tcd.h"
#include "DeviceManager/billacceptormanager.h"
#include "DeviceManager/tcdmanager.h"
#include "DeviceManager/keypadmanager.h"
#include "App/commandhandler.h"
#include "App/schedulerport.h"
#include "App/statusreporter.h"
#include "App/statemachine.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  // Hal init
  CLOCK_init();
  GPIO_init();
  TIMER_init();
  UART_init();
  I2C_init();
//  WATCHDOG_init();
  // Init
  CONFIG_init();
  SCHEDULERPORT_init();

  // Device Init
  BILLACCEPTOR_init();
  EEPROM_init();
  TCD_init();
  KEYPAD_init();
  LCD_init();
  RTC_init();
  // Device Manager Init
  BILLACCEPTORMNG_init();
  TCDMNG_init();
  KEYPADMNG_init();
  LCDMNG_init();
  // App Init
  MQTT_init();
  COMMANDHANDLER_init();
  SCHEDULERPORT_init();
  STATUSREPORTER_init();
  STATEMACHINE_init();
  /* USER CODE END Init */
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  RTC_test();
//  EEPROM_test();
//  CONFIG_test();
//  JSMNG_test();
//  CONFIG_clear();
//  WATCHDOG_test();
  while (1)
  {
//	  WATCHDOG_refresh();
	  STATEMACHINE_run();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
