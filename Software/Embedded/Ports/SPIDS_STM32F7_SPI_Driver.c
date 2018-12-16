/*

This is an SD interface that uses the SPI peripherals on an STM32F7 platform. 
It is designed to be used with the SPIDS interface to the fatfs library by elm-chan. 

Author: Owen Lyke
June 2018

*/


#include "SPIDS_STM32F7_SPI_Driver.h"

uint8_t ALL_HIGH = 0xFF;									// I want these to be labeled 'const' but it throws a lot of errors (qualifier discarding) so I'm skipping it for now
uint8_t ALL_HIGH_BLOCKLEN[SPIDS_STM32F7_SPI_DEF_BLOCKLEN];



/*
Example:
typedef struct
{
	uint8_t pin_to_use;
	SPIPORT * spi_to_use;
}SPI_driver_settings;

SPI_driver_settings specific_spi_settings;
specific_spi_settings.pin_to_use = specific_pin_number;
specific_spi_settings.spi_to_use = address_of_specific_SPI_port;

DSTATUS specific_driver1_initialize(BYTE pdrv)
{
	generic_spi_initialize(prdv, specific_spi_settings);
}

SPIDS_DiskioDriver_Typedef specific_SPIDS_driver;
specific_SPIDS_driver.disk_initialize = specific_driver1_initialize;

SPIDS_LinkDriver(&specific_SPIDS_driver, &buffer_for_path)

And BAM! Now the driver can use your specific settings. Rinse and repeat
for as many particular implementations of the SPI type driver that you have.
The solution is not really elegant but it will work. :)


DSTATUS specific_driver1_initialize(BYTE pdrv)
{
driver1_special_param = 1; // This might be a chip select pin or something
	generic_initialize(prdv, driver1_special_param);
}

*/


//SPIDS_DiskioDriver_Typedef SPIDS_STM32F7_SPI_Driver;


//const SPIDS_STM32F7_SPI_Settings_TypeDef hello2 = {0, 0, 0, 0};




//typedef struct{
//	DSTATUS (*disk_initialize) (BYTE);                     			/*!< Initialize Disk Drive                     */
//	DSTATUS (*disk_status)     (BYTE);                     			/*!< Get Disk Status                           */
//	DRESULT (*disk_read)       (BYTE, BYTE*, DWORD, UINT);       	/*!< Read Sector(s)                            */
//#if _USE_WRITE == 1
//  	DRESULT (*disk_write)      (BYTE, const BYTE*, DWORD, UINT); 	/*!< Write Sector(s) when _USE_WRITE = 0       */
//#endif /* _USE_WRITE == 1 */
//#if _USE_IOCTL == 1
//  	DRESULT (*disk_ioctl)      (BYTE, BYTE, void*);              	/*!< I/O control operation when _USE_IOCTL = 1 */
//#endif /* _USE_IOCTL == 1 */
//
//}SPIDS_DiskioDriver_Typedef;


void SPIDS_STM32F7_SPI_Settings_Initialize(SPIDS_STM32F7_SPI_Settings_TypeDef * hspi_settings)
{
	hspi_settings->hspi = NULL;
	hspi_settings->CS_GPIO_Port = NULL;
	hspi_settings->CS_Pin = 0;
	hspi_settings->max_freq = SPIDS_STM32F7_SPI_MAX_FREQ;
	hspi_settings->init_freq = SPIDS_STM32F7_SPI_INIT_FREQ;
	hspi_settings->Timeout = SPIDS_STM32F7_SPI_Default_Timeout;
	hspi_settings->hw_format = SPIDS_SD_HW_SC;						// Assume standard capacity
}



// These bad boys do the actual work
DSTATUS SPIDS_STM32F7_SPI_disk_initialize(BYTE pdrv, SPIDS_STM32F7_SPI_Settings_TypeDef * settings)
{
	// Okay listen up lads, this is an important one (I guess they all are...)
	// We need to get the SD card ready to work.

	uint8_t r1_resp = 0xFF;
	uint32_t r3_resp = 0x00000000;
	SPIDS_STM32F7_SPI_CodeTypeDef	result = SPIDS_STM32F7_SPI_CODE_NOM;

	for(uint32_t indi = 0; indi < SPIDS_STM32F7_SPI_DEF_BLOCKLEN; indi++)				// Fill out the giant all_high buffer for faster reads...
	{
		ALL_HIGH_BLOCKLEN[indi] = 0xFF;
	}


	SPIDS_STM32F7_SPI_assert_spi_bus(settings, 1); 										// Assert the SPI bus for the given settings, in initialization mode

	// Transmit dummy bytes - these seem to help the SD card to 'wake up' or something (copied this style from Arduino SPI SD library)
	for(uint16_t indi = 0; indi < SPIDS_STM32F7_SPI_NUM_DUMMY_BYTES; indi++)
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		HAL_SPI_Transmit(settings->hspi, &ALL_HIGH, 1, settings->Timeout);
	}


	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 0);						// Set the CS pin low

	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_GO_IDLE_STATE, 0, 148);					// Reset
	result = SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);							// Resp should be 1, 10 tries should be enough for timeout
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return STA_NOINIT;
	}

	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_GO_IDLE_STATE, 0, 148);					// Reset again (why?)
	result = SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);							// Resp should be 1
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return STA_NOINIT;
	}

	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_HS_SEND_EXT_CSD, 0x000001AA, 134);		// Send cmd8 (SD physical layer spec pg. 95)
	result = SPIDS_STM32F7_SPI_GET_R3(settings, &r1_resp, &r3_resp, 10);
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return STA_NOINIT;
	}
	if(r1_resp & SPIDS_SPI_R1_ILLEGAL_COMMAND)
	{
		// Ver1.X SD Memory Card or Not SD Memory Card
		SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_READ_OCR, 0, 0);		// Send cmd58 to check operating voltages
		result = SPIDS_STM32F7_SPI_GET_R3(settings, &r1_resp, &r3_resp, 10);
		if(result != SPIDS_STM32F7_SPI_CODE_NOM)
		{
			HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
			return STA_NOINIT;
		}
		if(r1_resp & SPIDS_SPI_R1_ILLEGAL_COMMAND)
		{
			// Not an SD card
			HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
			return STA_NOINIT;
		}
//		if(OCR says that voltage is out of range)
//		{
//			retutn the no init flag
//		}



		uint32_t count = 0;
		while((r1_resp != 0x00) && (count++ < 10000))
		{
			SPIDS_STM32F7_SPI_CMD_SD_APP_OP_COND(settings, 0);
			result = SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);
			if(result != SPIDS_STM32F7_SPI_CODE_NOM)
			{
				// Timeout
				HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
				return STA_NOINIT;
			}
			if(r1_resp & SPIDS_SPI_R1_ILLEGAL_COMMAND)
			{
				// Not an SD card
				HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
				return STA_NOINIT;
			}
		}

		// If all these pass then it is a Version 1.X standard capacity sd card, and it is initialized
		settings->hw_format = SPIDS_SD_HW_SC;

		SPIDS_STM32F7_SPI_assert_spi_bus(settings, 0);	// Go into full speed mode

		// Set the CS pin high
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return STA_NOMINAL;
	}

	// If cmd8 is supported then it is version 2.00 or later
	// Here you could check the response to see if the voltage range is actually supported, but I will skip it for now



	// Now send ACMD41 over and over until the card is initialized
	uint32_t count = 0;
	r1_resp = 0xFF;
	while((r1_resp != 0x00) && (count++ < 10000))
	{
		SPIDS_STM32F7_SPI_CMD_SD_APP_OP_COND(settings, 1);			// The 1 here indicates 'high capacity support (HCS)'
		result = SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);
		if(result != SPIDS_STM32F7_SPI_CODE_NOM)
		{
			HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
			return STA_NOINIT;
		}
	}
	if(r1_resp != 0)
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return STA_NOINIT;	// The card timed out...
	}

	// Now use CMD58 to see if the card is high-capacity or not
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_READ_OCR, 0, 0);		// Send cmd58 to check operating voltages
	result = SPIDS_STM32F7_SPI_GET_R3(settings, &r1_resp, &r3_resp, 10);
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)
	{
		// Timeout
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return STA_NOINIT;
	}
	if(r3_resp & 0x40000000)
	{
		// High capacity
		settings->hw_format = SPIDS_SD_HW_HC;
	}
	else
	{
		// Standard capacity
		settings->hw_format = SPIDS_SD_HW_SC;
	}

	// Set the block length to 512 bytes
//	SPIDS_STM32F7_SPI_CMD_SET_BLOCKLEN(settings, 0x200);
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_SET_BLOCKLEN, 0x00000200, 0);
	result = SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);					//
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return STA_NOINIT;
	}

	SPIDS_STM32F7_SPI_assert_spi_bus(settings, 0);	// Go into full speed mode

	// Set the CS pin high
	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
	return STA_NOMINAL;
}

DSTATUS SPIDS_STM32F7_SPI_disk_status(BYTE pdrv, SPIDS_STM32F7_SPI_Settings_TypeDef * settings)
{
	uint16_t r2_resp = 0xFFFF;
	SPIDS_STM32F7_SPI_CodeTypeDef	result = SPIDS_STM32F7_SPI_CODE_NOM;

	// Set the CS pin low
	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 0);

//	SPIDS_STM32F7_SPI_CMD_SEND_STATUS(settings);
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_SEND_STATUS, 0, 0);
	SPIDS_STM32F7_SPI_GET_R2(settings, &r2_resp, 10);
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)	// Check to see if there was a timeout or something...
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return STA_NOINIT;
	}

	if(r2_resp) // Actually check the response from the SD card
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return STA_NOINIT;
	}

	// Set the CS pin high
	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
	return STA_NOMINAL;
}

DRESULT SPIDS_STM32F7_SPI_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count, SPIDS_STM32F7_SPI_Settings_TypeDef * settings)
{
	// I'll just always use a multi-sector read
	uint32_t start_address = 0x00;
	uint8_t r1_resp = 0xFF;
	uint16_t r2_resp = 0xFFFF;
	SPIDS_STM32F7_SPI_CodeTypeDef	result = SPIDS_STM32F7_SPI_CODE_NOM;
	const uint16_t blocklen = SPIDS_STM32F7_SPI_DEF_BLOCKLEN;

	// Determine the proper address
	if(settings->hw_format == SPIDS_SD_HW_SC)
	{
		// OK, relying on the initialization function to guarantee that standard capacity cards are using a 512 byte block size...
		start_address = blocklen*sector + 0;	// Can it be this simple?
	}
	else
	{
		start_address = sector + 0;	// When it is high capacity the address is in block mode
	}


	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 0);						// Set the CS pin low


	// Check that the SD card is ready:
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_SEND_STATUS, 0, 0);
	SPIDS_STM32F7_SPI_GET_R2(settings, &r2_resp, 10);
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)	// Check to see if there was a timeout or something...
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return RES_ERROR;
	}
	if(r2_resp) // Actually check the response from the SD card
	{
		// It so happens that any value in r2_resp is an error of some sort (see SD spec, r2 format)
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return RES_NOTRDY;
	}


	// Send command to read multiple blocks
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_READ_MULT_BLOCK, start_address, 0);
	result = SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);

	// Read the data packets
	for(UINT indi = 0; indi < count; indi++)
	{
		SPIDS_STM32F7_SPI_Read_Data_Packet(settings, buff+blocklen*indi, blocklen, 1000);
	}

	// Send stop command
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_STOP_TRANSMISSION, 0, 0);

	// Discard a byte
	HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &r1_resp, 1, settings->Timeout);

	// Get the response to CMD12
	r1_resp = 0xFF;
	result = SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);

	// Then wait until the card is no longer busy
	r1_resp = 0x00;
	uint32_t timeout_count = 0;
	while((r1_resp != 0xFF) && (timeout_count++ < 10000))
	{
		HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &r1_resp, 1, settings->Timeout);
	}
	if(r1_resp != 0xFF)
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);						// Set the CS pin high
		return RES_ERROR;
	}


	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);						// Set the CS pin high
	return RES_OK;
}

//#if _USE_WRITE == 1
DRESULT SPIDS_STM32F7_SPI_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count, SPIDS_STM32F7_SPI_Settings_TypeDef * settings)
{
	uint32_t start_address = 0x00;
	uint8_t r1_resp = 0xFF;
	uint16_t r2_resp = 0xFFFF;
	SPIDS_STM32F7_SPI_CodeTypeDef	result = SPIDS_STM32F7_SPI_CODE_NOM;
	const uint16_t blocklen = SPIDS_STM32F7_SPI_DEF_BLOCKLEN;

	// Determine the proper address
	if(settings->hw_format == SPIDS_SD_HW_SC)
	{
		// OK, relying on the initialization function to guarantee that standard capacity cards are using a 512 byte block size...
		start_address = blocklen*sector + 0;	// Can it be this simple?
	}
	else
	{
		start_address = sector + 0;	// When it is high capacity the address is in block mode
	}

	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 0);						// Set the CS pin low

	// Check that the SD card is ready:
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_SEND_STATUS, 0, 0);
	SPIDS_STM32F7_SPI_GET_R2(settings, &r2_resp, 10);
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)	// Check to see if there was a timeout or something...
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return RES_ERROR;
	}
	if(r2_resp) // Actually check the response from the SD card
	{
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);
		return RES_NOTRDY;
	}


	// Send command to write multiple blocks
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_WRITE_MULT_BLOCK, start_address, 0);
	result = SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);

	// Send at least one byte of all high
	for(uint8_t indi = 0; indi < 5; indi++)
	{
		HAL_SPI_Transmit(settings->hspi, &ALL_HIGH, 1, settings->Timeout);
	}

	// Then begin transmitting data packets
	for(UINT indi = 0; indi < count; indi++)
	{
		SPIDS_STM32F7_SPI_Write_Data_Packet(settings, (BYTE*)(buff+blocklen*indi), blocklen, 1000);

		// NOw get the response (if the sd card and the program disagree about blocklen then it could be problematic)
		SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);
		if((r1_resp & 0x0F) == 11)
		{
			HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);						// Set the CS pin high
			return	RES_ERROR;
		}
		else if((r1_resp & 0x0F) == 13)
		{
			HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);						// Set the CS pin high
			return	RES_ERROR;
		}

		// Then wait for it not to be busy...
		r1_resp = 0x00;
		uint32_t timeout_count = 0;
		HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &r1_resp, 1, settings->Timeout);
		while((r1_resp != 0xFF) && (timeout_count++ < 10000))
		{
			HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &r1_resp, 1, settings->Timeout);
		}
		if(r1_resp != 0xFF)
		{
			// Timeout
			HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);						// Set the CS pin high
			return RES_ERROR;
		}

		// Now send another packet or the stop
	}

	// Send the stop token
	uint8_t token = SPIDS_STM32F7_SPI_DATA_TOKEN_STOP;
	HAL_SPI_Transmit(settings->hspi, &token, 1, settings->Timeout);

	// Send/receive a garbage byte
	HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &r1_resp, 1, settings->Timeout);

	// Wait for not busy...
	r1_resp = 0x00;
	uint32_t timeout_count = 0;
	HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &r1_resp, 1, settings->Timeout);
	while((r1_resp != 0xFF) && (timeout_count++ < 10000))
	{
		HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &r1_resp, 1, settings->Timeout);
	}
	if(r1_resp != 0xFF)
	{
		// Timeout
		HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);						// Set the CS pin high
		return RES_ERROR;
	}

	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);						// Set the CS pin high
	return RES_OK;
}
//#endif /* _USE_WRITE == 1 */

//#if _USE_IOCTL == 1
DRESULT SPIDS_STM32F7_SPI_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff, SPIDS_STM32F7_SPI_Settings_TypeDef * settings)
{
	WORD sector_size = 0;
	DWORD num_sectors = 0;

	switch(cmd)
	{
	case CTRL_SYNC :
			// Good news everyone! Because our write function is blocking we don't have to do anything for this command.
		return RES_OK;
		break;

	case GET_SECTOR_COUNT :
			num_sectors = (DWORD)SPIDS_STM32F7_SPI_Get_Num_Sectors(settings);
			*((DWORD *) buff) = num_sectors;
			return RES_OK;
		break;

	case GET_SECTOR_SIZE :
			sector_size = (WORD)SPIDS_STM32F7_SPI_Get_Sector_Size(settings);
			*((WORD *) buff) = sector_size;
			return RES_OK;
		break;

	case GET_BLOCK_SIZE :
			*((DWORD *) buff) = (DWORD)1;
			return RES_OK;
		break;

	case CTRL_TRIM :
			// I will ignore this for now
			return RES_OK;
		break;

	default :
		return RES_PARERR;
	}

}
//#endif /* _USE_IOCTL == 1 */

DWORD 	SPIDS_STM32F7_SPI_get_fattime(SPIDS_STM32F7_SPI_Settings_TypeDef * settings)
{
	uint8_t year = (2018 - 1980) & 0x3F;	// 6 Bits
	uint8_t month = 4;
	uint8_t day = 20;
	uint8_t hour = 4;
	uint8_t minute = 20;
	uint8_t second = 0;
	DWORD retval = (DWORD)((year << 25) | (month << 21) | (day << 16) | (hour << 11) | (minute << 5) | (second));
	return retval;
}





//


void 	SPIDS_STM32F7_SPI_CMD(SPIDS_STM32F7_SPI_Settings_TypeDef * settings, uint8_t cmd_val, uint32_t payload, uint8_t crc_val)
{
	uint8_t command_buff[SPIDS_STM32F7_SPI_CMD_FRAME_LENGTH];
	for(uint8_t indi = 0; indi < SPIDS_STM32F7_SPI_CMD_FRAME_LENGTH; indi++ ){ command_buff[indi] = 0x00; }

	command_buff[0] = (cmd_val | 0b01000000);
	command_buff[1] = ((payload & 0xFF000000) >> 24);
	command_buff[2] = ((payload & 0x00FF0000) >> 16);
	command_buff[3] = ((payload & 0x0000FF00) >> 8);
	command_buff[4] = ((payload & 0x000000FF) >> 0);
	command_buff[SPIDS_STM32F7_SPI_CMD_FRAME_LENGTH-1] = (crc_val | 0x01);

	HAL_SPI_Transmit(settings->hspi, command_buff, SPIDS_STM32F7_SPI_CMD_FRAME_LENGTH, settings->Timeout);
}


SPIDS_STM32F7_SPI_CodeTypeDef	SPIDS_STM32F7_SPI_CMD_SD_APP_OP_COND(SPIDS_STM32F7_SPI_Settings_TypeDef * settings, uint8_t HCS_flag)
{
	uint8_t rx_byt = 0xFF;
	SPIDS_STM32F7_SPI_CodeTypeDef result = SPIDS_STM32F7_SPI_CODE_NOM;


	// First send the APP_CMD
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_APP_CMD, 0, 0);
	result = SPIDS_STM32F7_SPI_GET_R1(settings, &rx_byt, 10);					// timeout = 10 should be enough
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)
	{
		return result;
	}

	// Then send 'SDMMC_CMD_SD_APP_OP_COND'
	uint32_t payload = 0x00;
	if(HCS_flag)
	{
		payload = 0x40000000;
	}
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_SD_APP_OP_COND, payload, 0);
	return SPIDS_STM32F7_SPI_CODE_NOM;
}

void 	SPIDS_STM32F7_SPI_CMD_READ_OCR(SPIDS_STM32F7_SPI_Settings_TypeDef * settings)
{
	uint8_t command_buff[SPIDS_STM32F7_SPI_CMD_FRAME_LENGTH];
	uint8_t crc_val = 0;

	for(uint8_t indi = 0; indi < SPIDS_STM32F7_SPI_CMD_FRAME_LENGTH; indi++ ){ command_buff[indi] = 0x00; }
	command_buff[0] = (58 | 0b01000000);
	command_buff[SPIDS_STM32F7_SPI_CMD_FRAME_LENGTH-1] = (crc_val | 0x01);

	HAL_SPI_Transmit(settings->hspi, command_buff, SPIDS_STM32F7_SPI_CMD_FRAME_LENGTH, settings->Timeout);
}





/*
 *
 * Response getters
 *
 */

SPIDS_STM32F7_SPI_CodeTypeDef SPIDS_STM32F7_SPI_GET_R1(SPIDS_STM32F7_SPI_Settings_TypeDef * settings, uint8_t * p_r1_rx, uint32_t timeout)
{
	uint32_t count = 0;
	HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH,  p_r1_rx, 1, settings->Timeout);
	while((*( p_r1_rx) == 0xFF) && (count++ < timeout))
	{
		HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH,  p_r1_rx, 1, settings->Timeout);
	}
	if(*( p_r1_rx) == 0xFF)
	{
		return SPIDS_STM32F7_SPI_CODE_TIMEOUT;
	}
	return SPIDS_STM32F7_SPI_CODE_NOM;
}

SPIDS_STM32F7_SPI_CodeTypeDef SPIDS_STM32F7_SPI_GET_R2(SPIDS_STM32F7_SPI_Settings_TypeDef * settings, uint16_t *  p_r2_rx, uint32_t timeout)
{
	SPIDS_STM32F7_SPI_CodeTypeDef result = SPIDS_STM32F7_SPI_CODE_NOM;
	uint8_t r2_buff[2];

	result = SPIDS_STM32F7_SPI_GET_R1(settings, r2_buff, timeout);
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)
	{
		return result;
	}
	HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &(r2_buff[1]), 1, settings->Timeout);

	*(p_r2_rx) = ((r2_buff[0] << 8) | (r2_buff[1] << 0));

	return SPIDS_STM32F7_SPI_CODE_NOM;
}

SPIDS_STM32F7_SPI_CodeTypeDef SPIDS_STM32F7_SPI_GET_R3(SPIDS_STM32F7_SPI_Settings_TypeDef * settings, uint8_t *  p_r1_rx, uint32_t * p_r3_rx, uint32_t timeout)
{
	SPIDS_STM32F7_SPI_CodeTypeDef result = SPIDS_STM32F7_SPI_CODE_NOM;
	uint8_t tx_buff[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t r3_buff[4];

	result = SPIDS_STM32F7_SPI_GET_R1(settings, p_r1_rx, timeout);
	if(result != SPIDS_STM32F7_SPI_CODE_NOM)
	{
		return result;
	}
	HAL_SPI_TransmitReceive(settings->hspi, tx_buff, r3_buff, 4, settings->Timeout);

	*(p_r3_rx) = ((r3_buff[0] << 24) | (r3_buff[1] << 16) | (r3_buff[2] << 8) | (r3_buff[3] << 0));

	return SPIDS_STM32F7_SPI_CODE_NOM;
}







/*
 *
 * Intercepting data packets
 *
 */

void SPIDS_STM32F7_SPI_Read_Data_Packet(SPIDS_STM32F7_SPI_Settings_TypeDef * settings, BYTE* buff, uint16_t blocklen, uint16_t timeout)
{
	uint32_t count = 0;
	uint8_t temp_buff[2];
	uint8_t tx_buff[2] = {0xFF, 0xFF};

	if(blocklen > SPIDS_STM32F7_SPI_DEF_BLOCKLEN)
	{
		return;	// Can't have any buffer overruns... and for now ALL_HIGH_BLOCKLEN is of length SPIDS_STM32F7_SPI_DEF_BLOCKLEN
	}

	HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH,  temp_buff, 1, settings->Timeout);
//	while((*(temp_buff) == SPIDS_STM32F7_SPI_DATA_TOKEN_READ) && (count++ < timeout))
	while((*(temp_buff) != SPIDS_STM32F7_SPI_DATA_TOKEN_READ) && (count++ < timeout))
	{
		HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH,  temp_buff, 1, settings->Timeout);
	}
	// As soon as a non 0xFF byte was received it should be the data token (0xFE)
	// THe next 'blocklen' bytes should be the data...
//	for(uint16_t indi = 0; indi < blocklen; indi++)
//	{
//		HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &buff[0]+indi, 1, settings->Timeout);
//	}
	HAL_SPI_TransmitReceive(settings->hspi, ALL_HIGH_BLOCKLEN, buff, blocklen, settings->Timeout);	// I wish I could use this method (full blocklen read... but I need a guaranteed buffer of 0xFF values so that I don't accidentally transmit a bad command

	// Then there should be 2 CRC bytes
	HAL_SPI_TransmitReceive(settings->hspi, tx_buff, temp_buff, 2, settings->Timeout);
}

void SPIDS_STM32F7_SPI_Write_Data_Packet(SPIDS_STM32F7_SPI_Settings_TypeDef * settings, BYTE* buff, uint16_t blocklen, uint16_t timeout)
{
	uint8_t token = 0;
	uint8_t tx_buff[2] = {0xCA, 0xA3};

	if(blocklen > SPIDS_STM32F7_SPI_DEF_BLOCKLEN)
	{
		return;	// Can't have any buffer overruns... and for now ALL_HIGH_BLOCKLEN is of length SPIDS_STM32F7_SPI_DEF_BLOCKLEN
	}

	// Transmit the data token
	token = SPIDS_STM32F7_SPI_DATA_TOKEN_25;
	HAL_SPI_Transmit(settings->hspi, &token, 1, settings->Timeout);

	// Then transmit the data
	// THe next 'blocklen' bytes should be the data...
//	for(uint16_t indi = 0; indi < blocklen; indi++)
//	{
//		HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &buff[0]+indi, 1, settings->Timeout);
//	}
	HAL_SPI_Transmit(settings->hspi, buff, blocklen, settings->Timeout);	// I wish I could use this method (full blocklen read... but I need a guaranteed buffer of 0xFF values so that I don't accidentally transmit a bad command

	// Then there should be 2 CRC bytes
	HAL_SPI_Transmit(settings->hspi, tx_buff, 2, settings->Timeout);
}

uint32_t SPIDS_STM32F7_SPI_Get_Num_Sectors(SPIDS_STM32F7_SPI_Settings_TypeDef * settings)
{
	uint8_t r1_resp = 0xFF;
	uint8_t CSD[16];
	uint8_t trash[4];

	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 0);						// Set the CS pin low

	// Send the command to get the CSD register
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_SEND_CSD, 0, 0);
	SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);

	// Now we are expecting the data token 'SPIDS_STM32F7_SPI_DATA_TOKEN_25'
	HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &r1_resp, 1, settings->Timeout);
	if(r1_resp != SPIDS_STM32F7_SPI_DATA_TOKEN_25)
	{
		// This would be a 'not good' situation. Not sure what the proper thing to do would be though (error? try again? ehh)
	}

	// Now read 16 bytes of CSD
	HAL_SPI_TransmitReceive(settings->hspi, ALL_HIGH_BLOCKLEN, CSD, 16, settings->Timeout);

	// Now read the CRC values
	HAL_SPI_TransmitReceive(settings->hspi, ALL_HIGH_BLOCKLEN, trash, 2, settings->Timeout);


	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);						// Set the CS pin high

	// CSD bits 0-7 are CSD[15]
	// CSD bits 8-15 are CSD[14]
	// CSD bits 16-23 are CSD[13]
	// CSD bits 24-31 are CSD[12]
	// CSD bits 32-39 are CSD[11]
	// CSD bits 40-47 are CSD[10]
	// CSD bits 48-55 are CSD[9]
	// CSD bits 56-63 are CSD[8]
	// CSD bits 64-71 are CSD[7]
	// CSD bits 72-79 are CSD[6]
	// CSD bits 80-87 are CSD[5]
	// CSD bits 88-95 are CSD[4]
	// CSD bits 96-103 are CSD[3]
	// CSD bits 104-111 are CSD[2]
	// CSD bits 112-119 are CSD[1]
	// CSD bits 120-127 are CSD[0]


	// Now use the values to compute the number of sectors!!
	// Keep in mind that HC cards have different CSD fields:

	uint32_t num_sectors = 0;
	if(settings->hw_format == SPIDS_SD_HW_SC)
	{
		uint8_t read_bl_len = (CSD[5] & 0x0F);	// read_bl_len is bits 80-83, or the lower nibble of CSD[5]
		uint16_t c_size = (  ((uint16_t)(CSD[6] & 0x03) << 10) | ((uint16_t)(CSD[7] & 0xFF) << 2 )  |  ((uint16_t)(CSD[8] & 0xC0) >> 6)  );// c_size is bits 62-73, or
		uint8_t c_size_mult =  (((CSD[9] & 0x03) << 1) | ((CSD[10] & 0x80) >> 7));   	// c_size_mult is bits 47-49, or

		if(read_bl_len > 11)
		{
			read_bl_len = 11;		// See pg 81 of sd spec.
		}

		if(c_size_mult > 7)
		{
			c_size_mult = 7;		// See pg 81 of sd spec.
		}

//		uint16_t BLOCK_LEN = ((uint16_t)0x01 << read_bl_len);
		uint16_t MULT = ((uint16_t)0x01 << (c_size_mult+2));
		uint32_t BLOCKNR = (c_size + 1) * MULT;

		return BLOCKNR;
	}
	else
	{
		// In the case of a high capacity card the number of sectors is given (nearly) directly
		num_sectors = ( ((((uint32_t)CSD[7]) & 0x3F) << 16 ) | (((uint32_t)CSD[8]) << 8 ) | (((uint32_t)CSD[9]) << 0 ) );
//		num_sectors += 1; // Maybe... according to the SD spec page 87... but seems fishy. Can't hurt to tell fatfs one less....
		return num_sectors;
	}
}


uint16_t SPIDS_STM32F7_SPI_Get_Sector_Size(SPIDS_STM32F7_SPI_Settings_TypeDef * settings)
{
	uint8_t r1_resp = 0xFF;
	uint8_t CSD[16];
	uint8_t trash[4];

	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 0);						// Set the CS pin low

	// Send the command to get the CSD register
	SPIDS_STM32F7_SPI_CMD(settings, SDMMC_CMD_SEND_CSD, 0, 0);
	SPIDS_STM32F7_SPI_GET_R1(settings, &r1_resp, 10);

	// Now we are expecting the data token 'SPIDS_STM32F7_SPI_DATA_TOKEN_25'
	HAL_SPI_TransmitReceive(settings->hspi, &ALL_HIGH, &r1_resp, 1, settings->Timeout);
	if(r1_resp != SPIDS_STM32F7_SPI_DATA_TOKEN_25)
	{
		// This would be a 'not good' situation. Not sure what the proper thing to do would be though (error? try again? ehh)
	}

	// Now read 16 bytes of CSD
	HAL_SPI_TransmitReceive(settings->hspi, ALL_HIGH_BLOCKLEN, CSD, 16, settings->Timeout);

	// Now read the CRC values
	HAL_SPI_TransmitReceive(settings->hspi, ALL_HIGH_BLOCKLEN, trash, 2, settings->Timeout);


	HAL_GPIO_WritePin(settings->CS_GPIO_Port, settings->CS_Pin, 1);						// Set the CS pin high


	if(settings->hw_format == SPIDS_SD_HW_SC)
	{
		uint8_t read_bl_len = (CSD[5] & 0x0F);	// read_bl_len is bits 80-83, or the lower nibble of CSD[5]

		if(read_bl_len > 11)
		{
			read_bl_len = 11;		// See pg 81 of sd spec.
		}

		uint16_t BLOCK_LEN = ((uint16_t)0x01 << read_bl_len);

		return BLOCK_LEN;
	}
	else
	{
		// In the case of a high capacity card the block length is fixed to 512
		return 512;
	}
}






/*
 *
 * SPI bus assertion!
 *
 */


void 	SPIDS_STM32F7_SPI_assert_spi_bus(SPIDS_STM32F7_SPI_Settings_TypeDef * settings, uint8_t initialization)
{
	// initialization == 1 will set the frequency to the lower initialization frequency
	// Use the HAL RCC library to get the status of the system clocks in order to adjust clock speed if necessary

	uint8_t re_init = 0;

	RCC_OscInitTypeDef OscSettings;				// Declare a type to hold the oscillator settings
	RCC_ClkInitTypeDef ClkSettings;				// Declare a type to hold clock settings
	uint32_t FL = __HAL_FLASH_GET_LATENCY();	// Determine the flash latency in order to use the "GetClockConfig" function

	HAL_RCC_GetOscConfig(&OscSettings);			// Read the oscillator settings
	HAL_RCC_GetClockConfig(&ClkSettings, &FL);	// Read the clock settings

	uint32_t SourceFreq = 0;					// Determine the source frequency (skip LSE and LSI because they do not drive the SYSCLK)
	if(OscSettings.HSEState == RCC_HSE_ON){ SourceFreq = HSE_VALUE; }
	else if(OscSettings.HSIState == RCC_HSI_ON){ SourceFreq = HSI_VALUE; }
	else{ _Error_Handler(__FILE__, __LINE__); }

	// Now use the source frequency and the oscillator/clock/spi settings to determine the SCLK frequency
	if(OscSettings.PLL.PLLState == RCC_PLL_ON){
		SourceFreq *= (OscSettings.PLL.PLLN / (OscSettings.PLL.PLLM * OscSettings.PLL.PLLP));
	}

	switch(ClkSettings.AHBCLKDivider){
		case RCC_SYSCLK_DIV1 : SourceFreq /= 1; break;
		case RCC_SYSCLK_DIV2 : SourceFreq /= 2; break;
		case RCC_SYSCLK_DIV4 : SourceFreq /= 4; break;
		case RCC_SYSCLK_DIV8 : SourceFreq /= 8; break;
		case RCC_SYSCLK_DIV16 : SourceFreq /= 16; break;
		case RCC_SYSCLK_DIV64 : SourceFreq /= 64; break;
		case RCC_SYSCLK_DIV128 : SourceFreq /= 128; break;
		case RCC_SYSCLK_DIV256 : SourceFreq /= 256; break;
		case RCC_SYSCLK_DIV512 : SourceFreq /= 512; break;
		default : _Error_Handler(__FILE__, __LINE__);
	}

	if((settings->hspi->Instance == SPI1) || (settings->hspi->Instance == SPI4) || (settings->hspi->Instance == SPI5))
	{
		// These ports use the APB2 clock
		switch(ClkSettings.APB2CLKDivider){
			case RCC_HCLK_DIV1 : SourceFreq /= 1; break;
			case RCC_HCLK_DIV2 : SourceFreq /= 2; break;
			case RCC_HCLK_DIV4 : SourceFreq /= 4; break;
			case RCC_HCLK_DIV8 : SourceFreq /= 8; break;
			case RCC_HCLK_DIV16 : SourceFreq /= 16; break;
			default : _Error_Handler(__FILE__, __LINE__);
		}
	}
	else if((settings->hspi->Instance == SPI2) || (settings->hspi->Instance == SPI3))
	{
		// These ports use the APB1 clock
		switch(ClkSettings.APB1CLKDivider){
			case RCC_HCLK_DIV1 : SourceFreq /= 1; break;
			case RCC_HCLK_DIV2 : SourceFreq /= 2; break;
			case RCC_HCLK_DIV4 : SourceFreq /= 4; break;
			case RCC_HCLK_DIV8 : SourceFreq /= 8; break;
			case RCC_HCLK_DIV16 : SourceFreq /= 16; break;
			default : _Error_Handler(__FILE__, __LINE__);
		}
	}
	else{ _Error_Handler(__FILE__, __LINE__); } // Oops, looks like you don't have a SPI port?

	switch(settings->hspi->Init.BaudRatePrescaler){
		case SPI_BAUDRATEPRESCALER_2 : SourceFreq /= 2; break;
		case SPI_BAUDRATEPRESCALER_4 : SourceFreq /= 4; break;
		case SPI_BAUDRATEPRESCALER_8 : SourceFreq /= 8; break;
		case SPI_BAUDRATEPRESCALER_16 : SourceFreq /= 16; break;
		case SPI_BAUDRATEPRESCALER_32: SourceFreq /= 32; break;
		case SPI_BAUDRATEPRESCALER_64 : SourceFreq /= 64; break;
		case SPI_BAUDRATEPRESCALER_128 : SourceFreq /= 128; break;
		case SPI_BAUDRATEPRESCALER_256 : SourceFreq /= 256; break;
		default : _Error_Handler(__FILE__, __LINE__);
	} // SourceFreq finally contains the SPI clock frequency

	if(initialization == 1)
	{
		// This loop will try to increase the source frequency as long as it is below the desired (initialization) frequency and it CAN be increased
		while((SourceFreq < settings->init_freq) && (settings->hspi->Init.BaudRatePrescaler > 0x07))	// 0x07 here could very well be 0x00 too... I just feel more comfortable this way
		{
			settings->hspi->Init.BaudRatePrescaler -= 0x08;
			SourceFreq *= 2;
			re_init = 1;
		}
		// The above loop will stop when the freq exceeds the limit OR it cannot be increased any more
		// So we use the anti-loop to go to at least one setting below the acceptable frequency
		while((SourceFreq > settings->init_freq) && (settings->hspi->Init.BaudRatePrescaler < 0x38)) 	// 0x38 is the code for the maximum prescaler of /512
		{
			settings->hspi->Init.BaudRatePrescaler += 0x08;
			SourceFreq /= 2;
			re_init = 1;
		}
		// This loop will end when the frequency was brought within the acceptable range
	}
	else
	{
		// This loop will try to increase the source frequency as long as it is below the desired (maximum) frequency and it CAN be increased
		while((SourceFreq < settings->max_freq) && (settings->hspi->Init.BaudRatePrescaler > 0x07))	// 0x07 here could very well be 0x00 too... I just feel more comfortable this way
		{
			settings->hspi->Init.BaudRatePrescaler -= 0x08;
			SourceFreq *= 2;
			re_init = 1;
		}
		// The above loop will stop when the freq exceeds the limit OR it cannot be increased any more
		// So we use the anti-loop to go to at least one setting below the acceptable frequency
		while((SourceFreq > settings->max_freq) && (settings->hspi->Init.BaudRatePrescaler < 0x38)) 	// 0x38 is the code for the maximum prescaler of /512
		{
			settings->hspi->Init.BaudRatePrescaler += 0x08;
			SourceFreq /= 2;
			re_init = 1;
		}
		// This loop will end when the frequency was brought within the acceptable range
	}


	// Check if the CPOL and CPHA are correct
	if(settings->hspi->Init.CLKPolarity != SPIDS_STM32F7_SPI_CPOL)
	{
		settings->hspi->Init.CLKPolarity = SPIDS_STM32F7_SPI_CPOL;
		re_init = 1;
	}
	if(settings->hspi->Init.CLKPhase != SPIDS_STM32F7_SPI_CPHA)
	{
		settings->hspi->Init.CLKPhase = SPIDS_STM32F7_SPI_CPHA;
		re_init = 1;
	}

	if(re_init)
	{
		if (HAL_SPI_Init(settings->hspi) != HAL_OK)		// Attempt to re-initialize the new SPI settings
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		HAL_SPI_Transmit(settings->hspi, &re_init, 1, settings->Timeout);	// Send a transmission to "fix" the SPI lines
	}
}





