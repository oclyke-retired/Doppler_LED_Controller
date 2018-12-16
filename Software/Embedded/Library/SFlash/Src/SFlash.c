#include "SFlash.h"


SFLASH_Stat_e SFLASH_setUnit(SFLASH_Handle_t* pflash)
{
	if(pflash->user.setUnit == NULL){ return SFLASH_error; }
	return pflash->user.setUnit(pflash);
}

SFLASH_Stat_e SFLASH_command(SFLASH_Handle_t* pflash)
{
	if(pflash->user.command == NULL){ return SFLASH_error; }
	return pflash->user.command(pflash);
}

SFLASH_Stat_e SFLASH_transmit(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	if(pflash->user.transmit == NULL){ return SFLASH_error; }
	return pflash->user.transmit(pflash, pdata, len);
}

SFLASH_Stat_e SFLASH_receive(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	if(pflash->user.receive == NULL){ return SFLASH_error; }
	return pflash->user.receive(pflash, pdata, len);
}

SFLASH_Stat_e SFLASH_checkUnit(SFLASH_Handle_t* pflash)
{
	SFLASH_Stat_e retval = SFLASH_ok;
	if(pflash->prevUnit != pflash->unit){
		retval = SFLASH_setUnit(pflash);
	}
	if( retval != SFLASH_ok ){
		return retval;
	}
	pflash->prevUnit = pflash->unit;	// Only update the unit if the unit setting was successful
	return retval;
}

SFLASH_Stat_e SFLASH_waitNotBusy(SFLASH_Handle_t* pflash, uint32_t timeout)
{
	SFLASH_Stat_e retval = SFLASH_error;

	if(timeout == 0xFFFFFFFF){ return SFLASH_error; } // Prevent an infinite loop possibility

	uint8_t reg1;

	for(uint32_t indi = 0; indi < timeout; indi++){
		SFLASH_ReadStatusReg1(pflash, &reg1, 1);
		if(!(reg1 & 0x01)){			// Bit 0 of reg1 is the busy bit
			retval = SFLASH_ok;
			return retval;			// Return with OK status if the busy bit is not set
		}
	}
	// If you try timeout times and no luck then return the defualt fail value
	return retval;
}




// These are easier ways to execute standard-supported commands
SFLASH_Stat_e 	SFLASH_WriteEnable(SFLASH_Handle_t* pflash)
{
	/*
	The Write Enable instruction (Figure 5) sets the Write Enable Latch (WEL) bit in the Status Register to a
	1. The WEL bit must be set prior to every Page Program, Quad Page Program, Sector Erase, Block
	Erase, Chip Erase, Write Status Register and Erase/Program Security Registers instruction. The Write
	Enable instruction is entered by driving /CS low, shifting the instruction code “06h” into the Data Input (DI)
	pin on the rising edge of CLK, and then driving /CS high.
	*/
	SFLASH_Stat_e retval = SFLASH_ok;

	pflash->cmd.Instruction = 0x06;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = 0x00;				// No data
	pflash->cmd.DataLines = SFLASH_Lines_None; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	return retval;
}

SFLASH_Stat_e 	SFLASH_WriteEnableVolatileStatus(SFLASH_Handle_t* pflash)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	pflash->cmd.Instruction = 0x50;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = 0x00;				// No data
	pflash->cmd.DataLines = SFLASH_Lines_None; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	return retval;
}

SFLASH_Stat_e 	SFLASH_WriteDisable(SFLASH_Handle_t* pflash)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	pflash->cmd.Instruction = 0x04;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = 0x00;				// No data
	pflash->cmd.DataLines = SFLASH_Lines_None; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	return retval;
}

SFLASH_Stat_e 	SFLASH_ReadStatusReg1(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	if(len < 1){ return SFLASH_error; }
	len = SFLASH_MIN(len, 1);

	pflash->cmd.Instruction = 0x05;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_ReadStatusReg2(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	if(len < 1){ return SFLASH_error; }
	len = SFLASH_MIN(len, 1);

	pflash->cmd.Instruction = 0x35;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_ReadStatusReg3(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	if(len < 1){ return SFLASH_error; }
	len = SFLASH_MIN(len, 1);

	pflash->cmd.Instruction = 0x15;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_WriteStatusReg1(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	if(len < 1){ return SFLASH_error; }
	len = SFLASH_MIN(len, 1);

	pflash->cmd.Instruction = 0x01;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_transmit( pflash, pdata, len);
	return retval;
}
SFLASH_Stat_e 	SFLASH_WriteStatusReg2(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	if(len < 1){ return SFLASH_error; }
	len = SFLASH_MIN(len, 1);

	pflash->cmd.Instruction = 0x31;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_transmit( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_WriteStatusReg3(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	if(len < 1){ return SFLASH_error; }
	len = SFLASH_MIN(len, 1);

	pflash->cmd.Instruction = 0x11;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_transmit( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_ReadData(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0x03;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_FastRead(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0x0B;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x08;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_FastReadDualOut(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0x3B;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x08;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_Two;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}
SFLASH_Stat_e 	SFLASH_FastReadQuadOut(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0x6B;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x08;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_Four;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_FastReadDualInOut(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0xBB;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_Two;
	pflash->cmd.AlternateBytes = 0xFF;
	pflash->cmd.AlternateBytesSize = 1;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_Two; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_Two;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_FastReadQuadInOut(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0xEB;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_Four;
	pflash->cmd.AlternateBytes = 0xFF;
	pflash->cmd.AlternateBytesSize = 1;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_Four; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x04;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_Four;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

//SFLASH_Stat_e 	SFLASH_SetBurstWWrap(SFLASH_Handle_t* pflash);

SFLASH_Stat_e 	SFLASH_PageProgram(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	len = SFLASH_MIN(len, 256);
	uint16_t leftInPage = ((uint16_t)0x00000100 - (uint8_t)(addr & 0x000000FF));
	if( len > leftInPage ){ return SFLASH_error; } // Don't allow over writing a page by wrapping

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_WriteEnable(pflash);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0x02;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_transmit( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_QuadInPageProgram(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	len = SFLASH_MIN(len, 256);
	uint16_t leftInPage = ((uint16_t)0x00000100 - (uint8_t)(addr & 0x000000FF));
	if( len > leftInPage ){ return SFLASH_error; } // Don't allow over writing a page by wrapping

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_WriteEnable(pflash);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0x32;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_Four;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_transmit( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_SectorErase(SFLASH_Handle_t* pflash, uint32_t addr)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_WriteEnable(pflash);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0x20;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = 0x00;
	pflash->cmd.DataLines = SFLASH_Lines_None;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

//	retval = SFLASH_transmit( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_32KBBlockErase(SFLASH_Handle_t* pflash, uint32_t addr)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_WriteEnable(pflash);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0x52;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = 0x00;
	pflash->cmd.DataLines = SFLASH_Lines_None;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

//	retval = SFLASH_transmit( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_64KBBlockErase(SFLASH_Handle_t* pflash, uint32_t addr)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_WriteEnable(pflash);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0xD8;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = addr;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = 0x00;
	pflash->cmd.DataLines = SFLASH_Lines_None;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

//	retval = SFLASH_transmit( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_ChipErase(SFLASH_Handle_t* pflash)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_waitNotBusy(pflash, SFLASH_DEFAULT_TIMEOUT);
	if(retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_WriteEnable(pflash);
	if(retval != SFLASH_ok ){ return retval; }

	pflash->cmd.Instruction = 0xC7; // or 0x60
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = 0x00;
	pflash->cmd.DataLines = SFLASH_Lines_None;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

//	retval = SFLASH_transmit( pflash, pdata, len);
	return retval;
}

//SFLASH_Stat_e 	SFLASH_EraseProgramSuspend(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_EraseProgramResume(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_PowerDown(SFLASH_Handle_t* pflash);

SFLASH_Stat_e 	SFLASH_ReadManufacturerDeviceID(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	len = SFLASH_MIN(len, 2);

	pflash->cmd.Instruction = 0x90;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = 0x00000000;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_One;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_ReadManufacturerDeviceIDDualInOut(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	len = SFLASH_MIN(len, 2);

	pflash->cmd.Instruction = 0x92;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = 0x00000000;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_Two;
	pflash->cmd.AlternateBytes = 0xFF;
	pflash->cmd.AlternateBytesSize = 1;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_Two; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_Two;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_ReadManufacturerDeviceIDQuadInOut(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	len = SFLASH_MIN(len, 2);

	pflash->cmd.Instruction = 0x94;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = 0x00000000;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_Four;
	pflash->cmd.AlternateBytes = 0xFF;
	pflash->cmd.AlternateBytesSize = 1;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_Four; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x04;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_Four;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_ReadUniqueIDNumber(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	len = SFLASH_MIN(len, 8);

	pflash->cmd.Instruction = 0x4B;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 32;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_None;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

//SFLASH_Stat_e 	SFLASH_ReadJEDECID(SFLASH_Handle_t* pflash);

SFLASH_Stat_e 	SFLASH_ReadSFDPRegister(SFLASH_Handle_t* pflash, uint8_t offset, uint8_t* pdata, uint32_t len)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	len = SFLASH_MIN(len, 256);

	pflash->cmd.Instruction = 0x5A;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.Address = (uint32_t)offset;
	pflash->cmd.AddressBytes = 3;
	pflash->cmd.AddressLines = SFLASH_Lines_One; 			// No address
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x08;
	pflash->cmd.DataBytes = len;
	pflash->cmd.DataLines = SFLASH_Lines_One; 	// No data

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_receive( pflash, pdata, len );
	return retval;
}

//SFLASH_Stat_e 	SFLASH_EraseSecurityRegisters(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_ProgramSecurityRegisters(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_ReadSecurityRegisters(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_IndividualBlockSectorLock(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_IndividualBlockSectorUnlock(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_ReadBlockSectorLock(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_GlobalBlockSectorLock(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_GlovalBlockSectorUnlock(SFLASH_Handle_t* pflash);

SFLASH_Stat_e 	SFLASH_EnableReset(SFLASH_Handle_t* pflash)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	pflash->cmd.Instruction = 0x66;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = 0;
	pflash->cmd.DataLines = SFLASH_Lines_None;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

//	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}

SFLASH_Stat_e 	SFLASH_ResetDevice(SFLASH_Handle_t* pflash)
{
	SFLASH_Stat_e retval = SFLASH_ok;

	retval = SFLASH_EnableReset(pflash);
	if(retval != SFLASH_ok){ return retval; }

	pflash->cmd.Instruction = 0x99;
	pflash->cmd.InstructionLines = SFLASH_Lines_One;
	pflash->cmd.AddressLines = SFLASH_Lines_None;
	pflash->cmd.AlternateBytesLines = SFLASH_Lines_None; 	// No alternate bytes
	pflash->cmd.DummyCycles = 0x00;
	pflash->cmd.DataBytes = 0;
	pflash->cmd.DataLines = SFLASH_Lines_None;

	retval = SFLASH_checkUnit(pflash); // Make sure the correct driver is selected
	if( retval != SFLASH_ok ){ return retval; }

	retval = SFLASH_command( pflash );
	if( retval != SFLASH_ok ){ return retval; }

//	retval = SFLASH_receive( pflash, pdata, len);
	return retval;
}
