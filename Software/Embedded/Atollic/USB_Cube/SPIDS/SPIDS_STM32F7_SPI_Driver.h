/*

This is intended to be a driver for the 
fatfs: http://elm-chan.org/fsw/ff/00index_e.html

Author: Owen Lyke

*/




#ifndef SPIDS_STM32F4_SPI_DRIVER_H
#define SPIDS_STM32F4_SPI_DRIVER_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "SPIDS.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_spi.h"
#include "diskio.h"

// SPI interface defaults
#define SPIDS_STM32F4_SPI_MAX_FREQ		25000000			// Hz - Maximum data transfer frequency
#define SPIDS_STM32F4_SPI_INIT_FREQ		400000				// Hz - Maximum frequency used for initialization
#define SPIDS_STM32F4_SPI_CPOL 			SPI_POLARITY_HIGH	// Idle high
#define SPIDS_STM32F4_SPI_CPHA			SPI_PHASE_2EDGE		// 2nd edge

// This is an SD spec... it REALLY should not be changed
#define SPIDS_STM32F4_SPI_CMD_FRAME_LENGTH 6
#define SPIDS_STM32F4_SPI_DATA_TOKEN_25		0xFC
#define SPIDS_STM32F4_SPI_DATA_TOKEN_READ	0xFE
#define SPIDS_STM32F4_SPI_DATA_TOKEN_STOP	0xFD

// Some rando defines
#define SPIDS_STM32F4_SPI_Default_Timeout	1000U
//#define SPIDS_STM32F4_SPI_NUM_DUMMY_BYTES 	12000
#define SPIDS_STM32F4_SPI_NUM_DUMMY_BYTES 	100
#define SPIDS_STM32F4_SPI_DEF_BLOCKLEN 		512		// This shouldnt change either

// R1 flag masks for SPI mode (different than the full SDIO spec)
#define SPIDS_SPI_R1_IN_IDLE_STATE 			0x01
#define SPIDS_SPI_R1_ERASE_RESET 			0x02
#define SPIDS_SPI_R1_ILLEGAL_COMMAND		0x04
#define SPIDS_SPI_R1_COMMAND_CRC_ERROR		0x08
#define SPIDS_SPI_R1_ERASE_SEQUENCE_ERROR	0x10
#define SPIDS_SPI_R1_ADDRESS_ERROR			0x20
#define SPIDS_SPI_R1_PARAMETER_ERROR		0x40


typedef enum{
	SPIDS_STM32F4_SPI_CODE_NOM = 0,
	SPIDS_STM32F4_SPI_CODE_TIMEOUT
}SPIDS_STM32F4_SPI_CodeTypeDef;


typedef struct{
	SPI_HandleTypeDef 		*hspi;				// Which SPI port is being used
	GPIO_TypeDef			*CS_GPIO_Port;		// Which GPIO port is used for the chip select pin
	uint16_t				CS_Pin;				// The pin number of the chip select pin on the GPIO port
	uint32_t 				max_freq;			// User can elect to
	uint32_t 				init_freq;
	uint32_t				Timeout;			// The timeout value for blocking SPI operations
	SPIDS_SD_HW_FORMAT_TypeDef	hw_format;		// Actually should not be used by the user - this is set by the driver during initialization and then referenced later
}SPIDS_STM32F4_SPI_Settings_TypeDef;			// This settings structure can be used to pass some general SPI settings into the driver (more or less)

// Since I can't have a global-scope default SPI settings object (Atollic can't find the program file if I try the following:
// const SPIDS_STM32F4_SPI_Settings_TypeDef hello2 = {0, 0, 0, 0}; ) then I will use a default initializer function
void SPIDS_STM32F4_SPI_Settings_Initialize(SPIDS_STM32F4_SPI_Settings_TypeDef * hspi_settings);





// These bad boys do the actual work, using both the standard fatfs parameters as well as the spi settings parameter.
DSTATUS SPIDS_STM32F4_SPI_disk_initialize(BYTE pdrv, SPIDS_STM32F4_SPI_Settings_TypeDef * settings);                     							/*!< Initialize Disk Drive                     */
DSTATUS SPIDS_STM32F4_SPI_disk_status(BYTE pdrv, SPIDS_STM32F4_SPI_Settings_TypeDef * settings);                     								/*!< Get Disk Status                           */
DRESULT SPIDS_STM32F4_SPI_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count, SPIDS_STM32F4_SPI_Settings_TypeDef * settings);   /*!< Read Sector(s)                            */
//#if _USE_WRITE == 1
	DRESULT SPIDS_STM32F4_SPI_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count, SPIDS_STM32F4_SPI_Settings_TypeDef * settings); 		/*!< Write Sector(s) when _USE_WRITE = 0       */
//#endif /* _USE_WRITE == 1 */
//#if _USE_IOCTL == 1
	DRESULT SPIDS_STM32F4_SPI_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff, SPIDS_STM32F4_SPI_Settings_TypeDef * settings);              					/*!< I/O control operation when _USE_IOCTL = 1 */
//#endif /* _USE_IOCTL == 1 */
DWORD 	SPIDS_STM32F4_SPI_get_fattime(SPIDS_STM32F4_SPI_Settings_TypeDef * settings);


// These are functions that are directly used to talk to the SD card
void 								SPIDS_STM32F4_SPI_CMD(SPIDS_STM32F4_SPI_Settings_TypeDef * settings, uint8_t cmd_val, uint32_t payload, uint8_t crc_val);
SPIDS_STM32F4_SPI_CodeTypeDef		SPIDS_STM32F4_SPI_CMD_SD_APP_OP_COND(SPIDS_STM32F4_SPI_Settings_TypeDef * settings, uint8_t HCS_flag);
void 								SPIDS_STM32F4_SPI_CMD_READ_OCR(SPIDS_STM32F4_SPI_Settings_TypeDef * settings);

// These functions wait for a particular response format from the card, or time out
SPIDS_STM32F4_SPI_CodeTypeDef 	SPIDS_STM32F4_SPI_GET_R1(SPIDS_STM32F4_SPI_Settings_TypeDef * settings, uint8_t * p_r1_rx, uint32_t timeout);
SPIDS_STM32F4_SPI_CodeTypeDef 	SPIDS_STM32F4_SPI_GET_R2(SPIDS_STM32F4_SPI_Settings_TypeDef * settings, uint16_t *  p_r2_rx, uint32_t timeout);
SPIDS_STM32F4_SPI_CodeTypeDef 	SPIDS_STM32F4_SPI_GET_R3(SPIDS_STM32F4_SPI_Settings_TypeDef * settings, uint8_t *  p_r1_rx, uint32_t * p_r3_rx, uint32_t timeout);

// For getting data from the SD card in blocks
void SPIDS_STM32F4_SPI_Read_Data_Packet(SPIDS_STM32F4_SPI_Settings_TypeDef * settings, BYTE* buff, uint16_t blocklen, uint16_t timeout);

// FOr writing data to the SD card in blocks
void SPIDS_STM32F4_SPI_Write_Data_Packet(SPIDS_STM32F4_SPI_Settings_TypeDef * settings, BYTE* buff, uint16_t blocklen, uint16_t timeout);

uint32_t SPIDS_STM32F4_SPI_Get_Num_Sectors(SPIDS_STM32F4_SPI_Settings_TypeDef * settings);
uint16_t SPIDS_STM32F4_SPI_Get_Sector_Size(SPIDS_STM32F4_SPI_Settings_TypeDef * settings);

void 	SPIDS_STM32F4_SPI_assert_spi_bus(SPIDS_STM32F4_SPI_Settings_TypeDef * settings, uint8_t initialization);




#ifdef __cplusplus
}
#endif

#endif
