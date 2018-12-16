
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
  * Copyright (c) 2018 STMicroelectronics International N.V. 
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
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

#include "SPIDS.h"
#include "SPIDS_STM32F7_SPI_Driver.h"
#include "serial.h"

#define 	FTDI		&serial1

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

uint8_t receivebuff[200];

SPIDS_STM32F4_SPI_Settings_TypeDef 	AES_SPIDS_SPI_Settings;
SPIDS_DiskioDriver_Typedef			AES_SPIDS_SPI_Driver;

FATFS fatfs;
FIL myfile;
FRESULT fresult;
DIR mydir;
FILINFO fileinfo;

char AES_SDPath[4];   /* SD logical drive path */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

TaskHandle_t Heartbeet_Task_Handle = NULL;
void Heartbeet_Task(void* pvParameters )
{
	uint8_t rx_byte = 0;

	TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

//	HAL_UART_Receive_IT(&huart7, &(rx_byte), 1);

	for(;;)
	{
//		HAL_UART_Receive_IT(&huart7, &(rx_byte), 1);
//		vTaskSuspend(NULL); // Task will suspend itself (NULL) until resumed externally (for example when UART ISR is called)
//		HAL_UART_Transmit_IT(&huart7, &(rx_byte), 1);
//		//		HAL_GPIO_TogglePin(ULED2_GPIO_Port, ULED2_Pin);

		vTaskDelay(xDelay);
		serial_println(FTDI, "And I am still alive!");

	}
}

TaskHandle_t SD_Test_Task_Handle = NULL;
void SD_Test_Task(void* pvParameters)
{

	TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

	for(;;)
	{
		serial_println(FTDI, "Loop beginning");

		  serial_print(FTDI, "Mounting the SD card, result: ");
		  fresult = f_mount(&fatfs, AES_SDPath, 1);
		  print_fresult(fresult);
		  serial_println(FTDI,"");

		  if(fresult == FR_NOT_READY)
		  {
			  serial_print(FTDI, "Mounting the SD card (AGAIN), result: ");
			  fresult = f_mount(&fatfs, AES_SDPath, 1);
			  print_fresult(fresult);
		  }

		  if(fresult == FR_OK)
		  {
			  serial_print(FTDI, "Opening file 'test.txt', result: ");
			  fresult = f_open(&myfile, "test.txt", FA_CREATE_ALWAYS|FA_WRITE);
			  print_fresult(fresult);
			  serial_println(FTDI,"");

			  if(fresult == FR_OK)
			  {
				  serial_println(FTDI, "Writing data to the file...");

				  f_printf(&myfile, "This is line one\n");
				  f_printf(&myfile, "This is line two\n");
				  f_printf(&myfile, "Owen Lyke!!!\n");

				  serial_print(FTDI, "Closing the file, result: ");
				  fresult = f_close(&myfile);
				  print_fresult(fresult);
				  serial_println(FTDI, "");

			  }
			  else
			  {
				  serial_println(FTDI, "Ahh bummer");
			  }

			  serial_print(FTDI, "Opening file 'openme.txt', result: ");
			  fresult = f_open(&myfile, "openme.txt", FA_READ);
			  print_fresult(fresult);
			  serial_println(FTDI,"");

			  if(fresult == FR_OK)
			  {
				  UINT bytes_read = 0;

				  f_read(&myfile, receivebuff, 51, &bytes_read);

				  serial_print(FTDI, "read ");
				  serial_print_uint32_t(FTDI, (uint32_t)bytes_read, HEX, 4);
				  serial_println(FTDI, " bytes:");
				  serial_println(FTDI, "Here's what I read...\n");

				  serial_write(FTDI, receivebuff, (uint16_t)bytes_read);

				  serial_print(FTDI, "Closing the file, result: ");
				  fresult = f_close(&myfile);
				  print_fresult(fresult);
				  serial_println(FTDI, "");

			  }
			  else
			  {
				  serial_println(FTDI, "Didn't find the file");
			  }

			  serial_print(FTDI, "Opening root directory, result: ");
			  fresult = f_opendir(&mydir, "0:");
			  print_fresult(fresult);
			  serial_println(FTDI,"");

			  serial_println(FTDI, "Listing files...");

			  list_files();

		  }
		  else
		  {
			  serial_println(FTDI, "not ok... :(");
		  }

		  serial_println(FTDI, "Loop end");
		  serial_println(FTDI, "\n\n");

		  HAL_GPIO_TogglePin(ULED2_GPIO_Port, ULED2_Pin);

		  vTaskDelay(xDelay);
	}
}


// These functions here are wrappers for the SPI interface functions. They are defined by the user to pass in the desired spi settings in addition to normal fatfs parameters
DSTATUS AES_SPI_disk_initialize(BYTE pdrv);                     							/*!< Initialize Disk Drive                     */
DSTATUS AES_SPI_disk_status(BYTE pdrv);                     								/*!< Get Disk Status                           */
DRESULT AES_SPI_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);       			/*!< Read Sector(s)                            */
DRESULT AES_SPI_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count); 			/*!< Write Sector(s) when _USE_WRITE = 0       */
DRESULT AES_SPI_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);              					/*!< I/O control operation when _USE_IOCTL = 1 */
DWORD AES_SPI_get_fattime (void);

void print_fresult(FRESULT result);
void list_files();

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	// Define SPI interface settings
	SPIDS_STM32F4_SPI_Settings_Initialize(&AES_SPIDS_SPI_Settings);		// Set the SPI settings to default for safety
	AES_SPIDS_SPI_Settings.hspi = &hspi1;								// Use hspi1
	AES_SPIDS_SPI_Settings.CS_GPIO_Port = CS_SD_GPIO_Port;				// Use the sd_cs port
	AES_SPIDS_SPI_Settings.CS_Pin = CS_SD_Pin;							// Use the sd_cs pin
	AES_SPIDS_SPI_Settings.max_freq = 20000000;							// Only 20 MHz
	// leave init freq unchanged
	// Leave timeout unchanged



	// Link driver functions
	AES_SPIDS_SPI_Driver.disk_initialize = AES_SPI_disk_initialize;                     							/*!< Initialize Disk Drive                     */
	AES_SPIDS_SPI_Driver.disk_status = AES_SPI_disk_status;
	AES_SPIDS_SPI_Driver.disk_read = AES_SPI_disk_read;
	#if _USE_WRITE == 1
		AES_SPIDS_SPI_Driver.disk_write = AES_SPI_disk_write;
	#endif /* _USE_WRITE == 1 */
	#if _USE_IOCTL == 1
		AES_SPIDS_SPI_Driver.disk_ioctl = AES_SPI_disk_ioctl;
	#endif /* _USE_IOCTL == 1 */


	// Now link the driver into the disk manager(?)
		SPIDS_LinkDriver(&AES_SPIDS_SPI_Driver, AES_SDPath); // The first drive linked in should be at path "0:/\0"

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_UART7_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  HAL_SPI_MspInit( &hspi1 );	// Configure SPI hardware

  MX_USB_DEVICE_Init();





  xTaskCreate(Heartbeet_Task,
  		  (const char* const)"Heartbeet",
  		  configMINIMAL_STACK_SIZE,
  		  0,
  		  3,
  		  &Heartbeet_Task_Handle);


//    xTaskCreate(SD_Test_Task,
//  		  (const char* const)"SD_Test",
//  		  4*configMINIMAL_STACK_SIZE,
//  		  0,
//  		  1,
//  		  &SD_Test_Task_Handle);


  serial_initialize(FTDI, &huart7);

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Activate the Over-Drive mode 
    */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART7|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Uart7ClockSelection = RCC_UART7CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* USER CODE BEGIN 4 */

DSTATUS AES_SPI_disk_initialize(BYTE pdrv)
{
	return SPIDS_STM32F4_SPI_disk_initialize(pdrv, &AES_SPIDS_SPI_Settings);
}

DSTATUS AES_SPI_disk_status(BYTE pdrv)
{
	return SPIDS_STM32F4_SPI_disk_status(pdrv, &AES_SPIDS_SPI_Settings);
}

DRESULT AES_SPI_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
	return SPIDS_STM32F4_SPI_disk_read( pdrv, buff, sector, count, &AES_SPIDS_SPI_Settings);
}

#if _USE_WRITE == 1
DRESULT AES_SPI_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
	return SPIDS_STM32F4_SPI_disk_write( pdrv, buff, sector, count, &AES_SPIDS_SPI_Settings);
}
#endif /* _USE_WRITE == 1 */

#if _USE_IOCTL == 1
DRESULT AES_SPI_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
	return SPIDS_STM32F4_SPI_disk_ioctl( pdrv, cmd, buff, &AES_SPIDS_SPI_Settings);
}
#endif /* _USE_IOCTL == 1 */

DWORD AES_SPI_get_fattime (void)
{
	return SPIDS_STM32F4_SPI_get_fattime(&AES_SPIDS_SPI_Settings);
}


void print_fresult(FRESULT result)
{
	switch(result)
	{
		case FR_DISK_ERR : serial_print(FTDI, "FR_DISK_ERR"); break;
		case FR_INT_ERR : serial_print(FTDI, "FR_INT_ERR"); break;
		case FR_NOT_READY : serial_print(FTDI, "FR_NOT_READY"); break;
		case FR_NO_FILE : serial_print(FTDI, "FR_NO_FILE"); break;
		case FR_NO_PATH : serial_print(FTDI, "FR_NO_PATH"); break;
		case FR_INVALID_NAME : serial_print(FTDI, "FR_INVALID_NAME"); break;
		case FR_DENIED : serial_print(FTDI, "FR_DENIED"); break;
		case FR_EXIST : serial_print(FTDI, "FR_EXIST"); break;
		case FR_INVALID_OBJECT : serial_print(FTDI, "FR_INVALID_OBJECT"); break;
		case FR_WRITE_PROTECTED : serial_print(FTDI, "FR_WRITE_PROTECTED"); break;
		case FR_INVALID_DRIVE : serial_print(FTDI, "FR_INVALID_DRIVE"); break;
		case FR_NOT_ENABLED : serial_print(FTDI, "FR_NOT_ENABLED"); break;
		case FR_NO_FILESYSTEM : serial_print(FTDI, "FR_NO_FILESYSTEM"); break;
		case FR_MKFS_ABORTED : serial_print(FTDI, "FR_MKFS_ABORTED"); break;
		case FR_TIMEOUT : serial_print(FTDI, "FR_TIMEOUT"); break;
		case FR_LOCKED : serial_print(FTDI, "FR_LOCKED"); break;
		case FR_NOT_ENOUGH_CORE : serial_print(FTDI, "FR_NOT_ENOUGH_CORE"); break;
		case FR_TOO_MANY_OPEN_FILES : serial_print(FTDI, "FR_TOO_MANY_OPEN_FILES"); break;
		case FR_INVALID_PARAMETER : serial_print(FTDI, "FR_INVALID_PARAMETER"); break;
	}
}


void list_files()
{
	uint8_t count = 0;
	uint8_t max_reps = 100;
	uint8_t firstchar = 'a';

	while((firstchar != 0) && (count++ < max_reps))
	{
		serial_print(FTDI, "Reading a directory object, result: ");
		fresult = f_readdir(&mydir, &fileinfo);
		print_fresult(fresult);
		if(fresult == FR_OK)
		{
			serial_print(FTDI, ", Filename: ");
			serial_print(FTDI, fileinfo.fname);
			serial_println(FTDI, "");
			firstchar = fileinfo.fname[0];
		}
		else
		{
			// Error-break loop
			firstchar = 0;
		}
	}
}

// UART Callbacks!
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
//{
//	BaseType_t checkIfYieldRequired = xTaskResumeFromISR( UART_RX_Task_Handle ); // Check if need to yield
//	portYIELD_FROM_ISR( checkIfYieldRequired );
//}
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef * huart)
//{
//
//}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
