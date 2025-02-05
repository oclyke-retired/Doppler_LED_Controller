
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
#include "quadspi.h"
#include "spi.h"
#include "usart.h"
//#include "usb_device.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

// ToDo: make sure you have re-enabled optimization (for size was the original, but maybe try for speed?)

#include "SFlash.h"
#include "DOPP_SD.h"
#include "DOPP_Boot.h"
#include "print.h"
#include "print_conf.h"

// ToDo: Refactor SPIDS STM32F7 driver to use interrupt transfers and block operations until the hardware is done doing stuff.
// You can do this with a simple lock variable that is set while the SD card is being accessed. Then it is un-set in the completion interrupt

#include <string.h>
#include <stdlib.h>	// Note: you need to use the full NewLib if you want floats. But we don't need or want that for the bootloader
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern SFLASH_Intfc_t SFLASH_STM32F7_QSPI_Intfc;
extern SFLASH_Handle_t hsflash;




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/



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
//	MX_GPIO_Init();
//	MX_UART4_Init();
//	MX_SPI1_Init();
//	MX_SPI6_Init();
//	MX_QUADSPI_Init();
//	MX_USART1_UART_Init();
//	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 2 */




  /* To Minimize Peripheral Configuration/Deconfig we will configure peripherals as we need them */
  firstInit(); // Inits GPIO and UART4

  PRINT_Stat_e stat;
  UNUSED(stat); // Prevent warning if we happen to not use this variable

  stat = debug_link();	// User calls this directly to link up the debug print configuration

  stat = mprintf(&debugIntfc, "Welcome to the Doppler bootloader. Here's a formatted number: 0x% 2X. Enjoy!\n", 0xAB);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  	mprint(&debugIntfc, "Jumping to application\n");
  	uint32_t count = 0;
  	while (1)
  	{

  		mprintf(&debugIntfc, "Pretend application looping, count = 0x%+4X\n", count++);
  		HAL_Delay(100);

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

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
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
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

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





/* Code Boneyard */

/*

	// Setup flash memory
	hsflash.prevUnit = 0;
	hsflash.unit = 1; 							// Doing this should force a "setUnit" call
	hsflash.user = SFLASH_STM32F7_QSPI_Intfc;		// Link in the interface
//  	  hsflash.cmd = 								// Don't set this up manually
	hsflash.configured = true;

	// Set up the SD card
  	DOPP_Begin_SD();


// Flash Test Code
//	uint8_t mandevid[2];
//	const char* hello = "So what's this?";
//	const char* busy = "busy\n";
//	uint8_t rxbuff[512];
//
//	uint8_t count = 0;
//
//	uint8_t reg1 = 0;
//
//	SFLASH_SectorErase(&hsflash, 0x205); // Erases 4k bytes from 0x00000000 to 0x00000FA0
//
//	SFLASH_ReadStatusReg1(&hsflash, &reg1, 1);
//	while(reg1 & 0x01)
//	{
////		HAL_UART_Transmit(&huart4, busy, strlen(busy), 1000);
//		SFLASH_ReadStatusReg1(&hsflash, &reg1, 1);
//	}
//
//	SFLASH_PageProgram(&hsflash, 0x200, (uint8_t*)hello, strlen(hello)+1);
//
//	for(;;)
//	{
////		SFLASH_ReadManufacturerDeviceID(&hsflash, mandevid, 2);
////		SFLASH_WriteEnable(&hsflash);
////		SFLASH_ReadStatusReg3(&hsflash, mandevid, 2);
////		SFLASH_ReadStatusReg2(&hsflash, mandevid, 2);
////		SFLASH_ReadStatusReg1(&hsflash, mandevid, 2);
////
////		SFLASH_ReadStatusReg3(&hsflash, mandevid, 2);
////		SFLASH_ReadStatusReg2(&hsflash, mandevid, 2);
////		SFLASH_ReadStatusReg1(&hsflash, mandevid, 2);
//
//
//		SFLASH_ReadStatusReg1(&hsflash, &reg1, 1);
//		while(reg1 & 0x01)
//		{
////			HAL_UART_Transmit(&huart4, busy, strlen(busy), 1000);
//			SFLASH_ReadStatusReg1(&hsflash, &reg1, 1);
//		}
//
//		SFLASH_ReadData(&hsflash, 0x200, rxbuff, 20);
////		SFLASH_FastRead(&hsflash, 0x200, rxbuff, 20);
////		SFLASH_FastReadDualOut(&hsflash, 0x200, rxbuff, 20);
////		SFLASH_FastReadQuadOut(&hsflash, 0x200, rxbuff, 20);
////		SFLASH_FastReadDualInOut(&hsflash, 0x200, rxbuff, 20);		// Failed
////		SFLASH_FastReadQuadInOut(&hsflash, 0x200, rxbuff, 20);
//
//		  HAL_UART_Transmit(&huart4, rxbuff, 20, 1000);
//		  HAL_Delay(500);
//
//		  if(count++ == 5)
//		  {
//			SFLASH_ChipErase(&hsflash);
//			count = 0;
//		  }
//	}


 SD Test Code
//	for(;;)
//	{
//		HAL_Delay(1000);
//		debug("Loop beginning");
//		debug("Mounting the SD card, result: ");
//		fresult = f_mount(&fatfs, DOPP_SDPath, 1);
////		print_fresult(fresult);
//		debug("\n");
//		if(fresult == FR_NOT_READY)
//		{
//			debug("Mounting the SD card (AGAIN), result: ");
//			fresult = f_mount(&fatfs, DOPP_SDPath, 1);
////			print_fresult(fresult);
//		}
//		if(fresult == FR_OK)
//		{
//			debug("Opening file 'test.txt', result: ");
//			fresult = f_open(&myfile, "test.txt", FA_CREATE_ALWAYS|FA_WRITE);
////			print_fresult(fresult);
//			debug("\n");
//			if(fresult == FR_OK)
//			{
//				debug("Writing data to the file...");
//
//				f_printf(&myfile, "This is line one\n");
//				f_printf(&myfile, "This is line two\n");
//				f_printf(&myfile, "Owen Lyke!!!\n");
//
//				debug("Closing the file, result: ");
//				fresult = f_close(&myfile);
////				print_fresult(fresult);
//				debug("\n");
//			}
//			else
//			{
//				debug("Ahh bummer");
//			}
//			debug("Opening file 'openme.txt', result: ");
//			fresult = f_open(&myfile, "openme.txt", FA_READ);
////			print_fresult(fresult);
//			debug("\n");
//
//			if(fresult == FR_OK)
//			{
//			  UINT bytes_read = 0;
////			  f_read(&myfile, receivebuff, 51, &bytes_read);
//			  debug("read ");
////			  serial_print_uint32_t(FTDI, (uint32_t)bytes_read, HEX, 4);
//			  debug(" bytes:");
//			  debug("Here's what I read...\n");
////			  serial_write(FTDI, receivebuff, (uint16_t)bytes_read);
//			  debug("Closing the file, result: ");
//			  fresult = f_close(&myfile);
////			  print_fresult(fresult);
//			  debug("\n");
//			}
//			else
//			{
//				debug("Didn't find the file");
//			}
//			debug("Opening root directory, result: ");
//			fresult = f_opendir(&mydir, "0:");
//			print_fresult(fresult);
//			debug("\n");
//			debug("Listing files...\n");
////			list_files();
//		}
//		else
//		{
//		debug("not ok... :(");
//		}
//		debug("Loop end");
//		debug("\n\n");
//		HAL_GPIO_TogglePin(ULED2_GPIO_Port, ULED2_Pin);
//	}

*/




