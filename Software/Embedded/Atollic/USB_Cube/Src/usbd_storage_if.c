/**
  ******************************************************************************
  * @file           : usbd_storage_if.c
  * @version        : v1.0_Cube
  * @brief          : Memory management layer.
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
#include "usbd_storage_if.h"

/* USER CODE BEGIN INCLUDE */

#include "SPIDS.h"
#include "SPIDS_STM32F7_SPI_Driver.h"

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

extern SPIDS_STM32F4_SPI_Settings_TypeDef 	AES_SPIDS_SPI_Settings;
extern SPIDS_DiskioDriver_Typedef			AES_SPIDS_SPI_Driver;

extern FATFS fatfs;
extern FIL myfile;
extern FRESULT fresult;
extern DIR mydir;
extern FILINFO fileinfo;

extern char AES_SDPath[4];   /* SD logical drive path */




#define memBlkSize (512)
#define memNumBlk	(0x300)
#define memTotalBytes (memNumBlk * memBlkSize)

uint8_t mem[memBlkSize*memNumBlk];




//// These functions here are wrappers for the SPI interface functions. They are defined by the user to pass in the desired spi settings in addition to normal fatfs parameters
//extern DSTATUS AES_SPI_disk_initialize(BYTE pdrv);                     							/*!< Initialize Disk Drive                     */
//extern DSTATUS AES_SPI_disk_status(BYTE pdrv);                     								/*!< Get Disk Status                           */
//extern DRESULT AES_SPI_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);       			/*!< Read Sector(s)                            */
//extern DRESULT AES_SPI_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count); 			/*!< Write Sector(s) when _USE_WRITE = 0       */
//extern DRESULT AES_SPI_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);              					/*!< I/O control operation when _USE_IOCTL = 1 */
//extern DWORD AES_SPI_get_fattime (void);

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_STORAGE
  * @brief Usb mass storage device module
  * @{
  */

/** @defgroup USBD_STORAGE_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Defines
  * @brief Private defines.
  * @{
  */

#define STORAGE_LUN_NBR                  1
//#define STORAGE_BLK_NBR                  0x10000
#define STORAGE_BLK_NBR                  0x39b7
#define STORAGE_BLK_SIZ                  0x200

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN INQUIRY_DATA_FS */
/** USB Mass storage Standard Inquiry Data. */
const int8_t STORAGE_Inquirydata_FS[] = {/* 36 */
  
  /* LUN 0 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,	
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0' ,'1'                      /* Version      : 4 Bytes */
}; 
/* USER CODE END INQUIRY_DATA_FS */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t STORAGE_Init_FS(uint8_t lun);
static int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady_FS(uint8_t lun);
static int8_t STORAGE_IsWriteProtected_FS(uint8_t lun);
static int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun_FS(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_StorageTypeDef USBD_Storage_Interface_fops_FS =
{
  STORAGE_Init_FS,
  STORAGE_GetCapacity_FS,
  STORAGE_IsReady_FS,
  STORAGE_IsWriteProtected_FS,
  STORAGE_Read_FS,
  STORAGE_Write_FS,
  STORAGE_GetMaxLun_FS,
  (int8_t *)STORAGE_Inquirydata_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes over USB FS IP
  * @param  lun:
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Init_FS(uint8_t lun)
{
  /* USER CODE BEGIN 2 */

  fresult = f_mount(&fatfs, AES_SDPath, 1);
  if(fresult == FR_NOT_READY)
  {
	  fresult = f_mount(&fatfs, AES_SDPath, 1);
  }

  if(fresult == FR_NOT_READY)
  {
	  return (USBD_FAIL);
  }

  return (USBD_OK);

  /* USER CODE END 2 */
}

/**
  * @brief  .
  * @param  lun: .
  * @param  block_num: .
  * @param  block_size: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  /* USER CODE BEGIN 3 */


  uint32_t numSecs = SPIDS_STM32F4_SPI_Get_Num_Sectors(&AES_SPIDS_SPI_Settings);
  uint16_t secSize = SPIDS_STM32F4_SPI_Get_Sector_Size(&AES_SPIDS_SPI_Settings);

  *(block_size) = secSize;
  *(block_num) = numSecs;

  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsReady_FS(uint8_t lun)
{
  /* USER CODE BEGIN 4 */

	DSTATUS stat = SPIDS_STM32F4_SPI_disk_status(lun, &AES_SPIDS_SPI_Settings);

	if( stat != STA_NOMINAL ){ return (USBD_FAIL); }

//#define STA_NOMINAL		0x00	/* Nominal status */
//#define STA_NOINIT		0x01	/* Drive not initialized */
//#define STA_NODISK		0x02	/* No medium in the drive */
//#define STA_PROTECT		0x04	/* Write protected */


  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
  /* USER CODE BEGIN 5 */


	DSTATUS stat = SPIDS_STM32F4_SPI_disk_status(lun, &AES_SPIDS_SPI_Settings);

		if( stat == STA_PROTECT ){ return (USBD_FAIL); }

	//#define STA_NOMINAL		0x00	/* Nominal status */
	//#define STA_NOINIT		0x01	/* Drive not initialized */
	//#define STA_NODISK		0x02	/* No medium in the drive */
	//#define STA_PROTECT		0x04	/* Write protected */


  return (USBD_OK);

//	return 0;

  /* USER CODE END 5 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  /* USER CODE BEGIN 6 */
	DRESULT result = SPIDS_STM32F4_SPI_disk_read(lun, buf, blk_addr, blk_len, &AES_SPIDS_SPI_Settings);

	if( result != RES_OK){ return (USBD_FAIL); }


////	if(lun != 0)
////	{
////		return (USBD_FAIL);
////	}
//
//
////	uint32_t bytePointer = blk_addr*memBlkSize;
//	uint32_t bufferOffset = 0;
//
//	for(uint16_t indi = 0; indi < (blk_len); indi++)
//	{
//		// Loop through all the desired blocks to read
//		if( indi < memNumBlk )
//		{
//			for(uint32_t indj = 0; indj < memBlkSize; indj++)
//			{
//				// Loop through all the bytes of the block and write to the buffer
//				*(buf + bufferOffset++) = *(mem + ((blk_addr + indi) * memBlkSize) + indj);
//			}
//		}
//	}

  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  /* USER CODE BEGIN 7 */
	DRESULT result = SPIDS_STM32F4_SPI_disk_write(lun, buf, blk_addr, blk_len, &AES_SPIDS_SPI_Settings);
	if( result != RES_OK){ return (USBD_FAIL); }

////	if(lun != 0)
////	{
////		return (USBD_FAIL);
////	}
//
//	uint32_t bufferOffset = 0;
//
//	for(uint16_t indi = 0; indi < (blk_len); indi++)
//	{
//		// Loop through all the desired blocks to write
//		if( indi < memNumBlk )
//		{
//			for(uint32_t indj = 0; indj < memBlkSize; indj++)
//			{
//				// Loop through all the bytes of the block and write from the buffer
//				*(mem + ((blk_addr + indi) * memBlkSize) + indj) = *(buf + bufferOffset++);
//			}
//		}
//	}

  return (USBD_OK);
  /* USER CODE END 7 */
}

/**
  * @brief  .
  * @param  None
  * @retval .
  */
int8_t STORAGE_GetMaxLun_FS(void)
{
  /* USER CODE BEGIN 8 */

//	return 1;

  return (STORAGE_LUN_NBR - 1);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
