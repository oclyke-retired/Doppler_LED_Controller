#include "SFlash.h"
#include "quadspi.h"


SFLASH_Stat_e STM32F7_QSPI_setUnit(SFLASH_Handle_t* pflash); 							// Tells the lower level which unit to talk to based on the value in the handle (allowing for multiple flash controlled by one hardware driver)
SFLASH_Stat_e STM32F7_QSPI_command(SFLASH_Handle_t* pflash); 								//
SFLASH_Stat_e STM32F7_QSPI_transmit(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e STM32F7_QSPI_receive(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);

SFLASH_Intfc_t SFLASH_STM32F7_QSPI_Intfc = {
		STM32F7_QSPI_setUnit,
		STM32F7_QSPI_command,
		STM32F7_QSPI_transmit,
		STM32F7_QSPI_receive
};

SFLASH_Handle_t hsflash;








SFLASH_Stat_e STM32F7_QSPI_setUnit(SFLASH_Handle_t* pflash)
{
	// Check if the logical number is correct
	if(pflash->unit > 1){ return SFLASH_error; } // We want to allow either 1 or 0 for this

	// De-init the current one(?)

	// Re-init the next one
	if(pflash->unit == 1)
	{
		hqspi.Instance = QUADSPI;
		hqspi.Init.ClockPrescaler = 64;
		hqspi.Init.FifoThreshold = 1;
		hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
		hqspi.Init.FlashSize = 23;
		hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
		hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
		hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE; //
		hqspi.Init.FlashID = QSPI_FLASH_ID_2;
	}
	else
	{
		hqspi.Instance = QUADSPI;
		hqspi.Init.ClockPrescaler = 64;
		hqspi.Init.FifoThreshold = 1;
		hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
		hqspi.Init.FlashSize = 23;
		hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
		hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
		hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE; //
		hqspi.Init.FlashID = QSPI_FLASH_ID_1;
	}

	if (HAL_QSPI_Init(&hqspi) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	return SFLASH_ok;
}

SFLASH_Stat_e STM32F7_QSPI_command(SFLASH_Handle_t* pflash)
{
	QSPI_CommandTypeDef cmd;

	cmd.Instruction = (uint32_t)pflash->cmd.Instruction;
	switch(pflash->cmd.InstructionLines)
	{
	case SFLASH_Lines_None : cmd.InstructionMode = QSPI_INSTRUCTION_NONE; break;
	case SFLASH_Lines_One : cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE; break;
	case SFLASH_Lines_Two : cmd.InstructionMode = QSPI_INSTRUCTION_2_LINES; break;
	case SFLASH_Lines_Four : cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES; break;
	default : return SFLASH_error; break;
	}

	cmd.Address = (uint32_t)pflash->cmd.Address;
	switch(pflash->cmd.AddressBytes)
	{
	case 0 :
	case 1 : cmd.AddressSize = QSPI_ADDRESS_8_BITS; break;
	case 2 : cmd.AddressSize = QSPI_ADDRESS_16_BITS; break;
	case 3 : cmd.AddressSize = QSPI_ADDRESS_24_BITS; break;
	case 4 : cmd.AddressSize = QSPI_ADDRESS_32_BITS; break;
	default : return SFLASH_error; break;
	}
	switch(pflash->cmd.AddressLines)
	{
	case SFLASH_Lines_None : cmd.AddressMode = QSPI_ADDRESS_NONE; break;
	case SFLASH_Lines_One : cmd.AddressMode = QSPI_ADDRESS_1_LINE; break;
	case SFLASH_Lines_Two : cmd.AddressMode = QSPI_ADDRESS_2_LINES; break;
	case SFLASH_Lines_Four : cmd.AddressMode = QSPI_ADDRESS_4_LINES; break;
	default : return SFLASH_error; break;
	}

	cmd.AlternateBytes = (uint32_t)pflash->cmd.AlternateBytes;
	switch(pflash->cmd.AlternateBytesSize)
	{
	case 0 :
	case 1 : cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS; break;
	case 2 : cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_16_BITS; break;
	case 3 : cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_24_BITS; break;
	case 4 : cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_32_BITS; break;
	default : return SFLASH_error; break;
	}
	switch(pflash->cmd.AlternateBytesLines)
	{
	case SFLASH_Lines_None : cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; break;
	case SFLASH_Lines_One : cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES; break;
	case SFLASH_Lines_Two : cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES; break;
	case SFLASH_Lines_Four : cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES; break;
	default : return SFLASH_error; break;
	}

	cmd.DummyCycles = (uint32_t)pflash->cmd.DummyCycles;



	cmd.NbData = (uint32_t)pflash->cmd.DataBytes;
	switch(pflash->cmd.DataLines)
	{
	case SFLASH_Lines_None : cmd.DataMode = QSPI_DATA_NONE; break;
	case SFLASH_Lines_One : cmd.DataMode = QSPI_DATA_1_LINE; break;
	case SFLASH_Lines_Two : cmd.DataMode = QSPI_DATA_2_LINES; break;
	case SFLASH_Lines_Four : cmd.DataMode = QSPI_DATA_4_LINES; break;
	default : return SFLASH_error; break;
	}

	cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

	if(pflash->cmd.SIOO)
	{
		cmd.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;
	}
	else
	{
		cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	}

	HAL_QSPI_Command(&hqspi, &cmd, 1000);

	return SFLASH_ok;
}
SFLASH_Stat_e STM32F7_QSPI_transmit(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	HAL_StatusTypeDef HAL_stat = HAL_OK;
	SFLASH_Stat_e retval = SFLASH_ok;

	// Command is already set up
	// Make sure array length is respected
	if(len < pflash->cmd.DataBytes){ return SFLASH_error; }

	// Use HAL layer
	HAL_stat = HAL_QSPI_Transmit(&hqspi, pdata, 1000);

	switch(HAL_stat)
	{
	case HAL_OK : return retval; break;
	default : retval = SFLASH_error; break;
	}
	return retval;
}

SFLASH_Stat_e STM32F7_QSPI_receive(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	HAL_StatusTypeDef HAL_stat = HAL_OK;
	SFLASH_Stat_e retval = SFLASH_ok;

	// Command is already set up
	// Make sure array length is respected
	if(len < pflash->cmd.DataBytes){ return SFLASH_error; }

	// Use HAL layer
	HAL_stat = HAL_QSPI_Receive(&hqspi, pdata, 1000);

	switch(HAL_stat)
	{
	case HAL_OK : return retval; break;
	default : retval = SFLASH_error; break;
	}
	return retval;
}




/* Code Boneyard */

/*

////  	uint8_t spi5dat[2] = {0xAA, 0xEC};
////  //	const uint8_t spi5dat = "Hello world!";
//  	uint8_t flash_results[2] = {0xB0, 0x0B};
//  	QSPI_CommandTypeDef readManufacturerDeviceID;
//  	readManufacturerDeviceID.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
//  	readManufacturerDeviceID.Instruction = 0x90;	// pg 46
//  	readManufacturerDeviceID.InstructionMode = QSPI_INSTRUCTION_1_LINE;
////  	readManufacturerDeviceID.InstructionSize?
//  	readManufacturerDeviceID.Address = 0x000000;
//  	readManufacturerDeviceID.AddressMode = QSPI_ADDRESS_1_LINE;
//  	readManufacturerDeviceID.AddressSize = QSPI_ADDRESS_24_BITS;
//  	readManufacturerDeviceID.AlternateBytes = 0x00;
//  	readManufacturerDeviceID.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
//  	readManufacturerDeviceID.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
//  	readManufacturerDeviceID.DummyCycles = 0;
//  	readManufacturerDeviceID.DataMode = QSPI_DATA_1_LINE;
//  	readManufacturerDeviceID.NbData = 2;
//  	readManufacturerDeviceID.DdrMode = QSPI_DDR_MODE_DISABLE;
//  	readManufacturerDeviceID.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
//  	HAL_QSPI_Command(&hqspi, &readManufacturerDeviceID, 1000);
//  	for(;;)
//  	{
////  		HAL_QSPI_Transmit(&hqspi, flash_results, 1000);
//  		HAL_QSPI_Receive(&hqspi, flash_results, 1000);
//  		HAL_Delay(500);
//  	}

*/
