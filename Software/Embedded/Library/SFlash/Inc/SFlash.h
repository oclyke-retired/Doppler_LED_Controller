/*

This is the generic header for SPI Flash Memory

Things that need to happen:

- Configure a command 
	- Which flash, 
	- Which phases,
	- Etc
- Send/receive data (blocking or interrupt)


*/ 


#ifndef __SFLASH_H_
#define __SFLASH_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define SFLASH_MIN(a,b) (a <= b ? a : b)

#define SFLASH_DEFAULT_TIMEOUT 1000

typedef uint8_t SFLASH_unit_t;		// Defines the maximum number of unique flash devices that the library supports

typedef enum{						// A type to represent how many lines to use to send a command phase
	SFLASH_Lines_None = 0x00,
	SFLASH_Lines_One,
	SFLASH_Lines_Two,
	SFLASH_Lines_Four
}SFLASH_Lines_e;		

typedef enum{
	SFLASH_ok = 0x00,
	SFLASH_error
}SFLASH_Stat_e;

typedef struct _SFLASH_Handle_t SFLASH_Handle_t; 	// Forward declaration of the handle type

typedef struct { 		// Pointers to the user's functions for interfacing with the hardware
	SFLASH_Stat_e (*setUnit)(SFLASH_Handle_t* pflash); 							// Tells the lower level which unit to talk to based on the value in the handle (allowing for multiple flash controlled by one hardware driver)
	SFLASH_Stat_e (*command)(SFLASH_Handle_t* pflash); 								//
	SFLASH_Stat_e (*transmit)(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
	SFLASH_Stat_e (*receive)(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
}SFLASH_Intfc_t;
	
typedef struct {
	uint8_t 			Instruction;		// The value of the instruction
	SFLASH_Lines_e		InstructionLines;	// How many lines to use for this phase
	// uint8_t 			InstructionBytes;	// How many bytes the instruction is // Note: this is always one byte

	uint32_t 			Address;			// Value of the address
	SFLASH_Lines_e		AddressLines;		// How many lines to use for this phase
	uint8_t 			AddressBytes;		// How many bytes long the address is

	uint32_t 			AlternateBytes;
	SFLASH_Lines_e 		AlternateBytesLines;	// How many lines to use for this phase
	uint8_t				AlternateBytesSize;

	uint8_t				DummyCycles;

	SFLASH_Lines_e 		DataLines;				// How many lines to use for this phase
	uint32_t 			DataBytes;

	bool 				SIOO;				// If true then only send the instruction once per command setup
}SFLASH_Command_t;

struct _SFLASH_Handle_t{			// You declare a handle for each flash that you want to interface with
	// Settings that change with program execution
	SFLASH_Command_t 	cmd;	// Current command settings to use for transmit or receive

	// Static conditions set at the beginning and then left alone
	SFLASH_Intfc_t 		user;	// The user's functions for this handle
	SFLASH_unit_t		unit;	// Which flash unit you want to talk to

	// Stuff that the user doesn't change
	SFLASH_unit_t		prevUnit;	// Used to determine if we need to set the unit before a command
	bool 				configured;	// Is the handle properly configured to accept commands?
};	


/* Here is an idea I have: allow users to refefine the commands used for a particular operation for each given device handle. */

// struct SFLASH_UnitCmd_t{
// 	SFLASH_Command_t 	cmd;
// 	bool 				valid;
// }

// typedef enum {					// The value of the enum here indicates the index of the command within the std command structure
// 	SFLASH_WriteEnable = 0x00,
// 	SFLASH_WriteEnableVolatileStatus,
// 	SFLASH_WriteDisable,
// 	SFLASH_ReadStatusReg1,
// 	SFLASH_ReadStatusReg2,
// 	SFLASH_ReadStatusReg3,
// 	SFLASH_WriteStatusReg1,
// 	SFLASH_WriteStatusReg2,
// 	SFLASH_WriteStatusReg3,
// 	SFLASH_ReadData,
// 	SFLASH_FastRead,
// 	SFLASH_FastReadDualOut,
// 	SFLASH_FastReadQuadOut,
// 	SFLASH_FastReadDualInOUt,
// 	SFLASH_FastReadQuadInOut,
// 	SFLASH_SetBurstWWrap,
// 	SFLASH_PageProgram,
// 	SFLASH_QuadInPageProgram,
// 	SFLASH_SectorErase,
// 	SFLASH_32KBBlockErase,
// 	SFLASH_64KBBlockErase,
// 	SFLASH_ChipErase,
// 	SFLASH_EraseProgramSuspend,
// 	SFLASH_EraseProgramResume,
// 	SFLASH_PowerDown,
// 	SFLASH_ReadManufacturerDeviceID,
// 	SFLASH_ReadManufacturerDeviceIDDualInOut,
// 	SFLASH_ReadManufacturerDeviceIDQuadInOut,
// 	SFLASH_ReadUniqueIDNumber,
// 	SFLASH_ReadJEDECID,
// 	SFLASH_ReadSFDPRegister,
// 	SFLASH_EraseSecurityRegisters,
// 	SFLASH_ProgramSecurityRegisters,
// 	SFLASH_ReadSecurityRegisters,
// 	SFLASH_IndividualBlockSectorLock,
// 	SFLASH_IndividualBlockSectorUnlock,
// 	SFLASH_ReadBlockSectorLock,
// 	SFLASH_GlobalBlockSectorLock,
// 	SFLASH_GlovalBlockSectorUnlock,
// 	SFLASH_EnableReset,
// 	SFLASH_ResetDevice
// }SFLASH_StdCmd_e;

// struct SFLASH_StdCmd_t{
// 	SFLASH_UnitCmd_t*	WriteEnable;
// 	SFLASH_UnitCmd_t	WriteEnableVolatileStatus;
// 	SFLASH_UnitCmd_t	WriteDisable;
// 	SFLASH_UnitCmd_t	ReadStatusReg1;
// 	SFLASH_UnitCmd_t	ReadStatusReg2;
// 	SFLASH_UnitCmd_t	ReadStatusReg3;
// 	SFLASH_UnitCmd_t	WriteStatusReg1;
// 	SFLASH_UnitCmd_t	WriteStatusReg2;
// 	SFLASH_UnitCmd_t	WriteStatusReg3;
// 	SFLASH_UnitCmd_t	ReadData;
// 	SFLASH_UnitCmd_t	FastRead;
// 	SFLASH_UnitCmd_t	FastReadDualOut;
// 	SFLASH_UnitCmd_t	FastReadQuadOut;
// 	SFLASH_UnitCmd_t	FastReadDualInOUt;
// 	SFLASH_UnitCmd_t	FastReadQuadInOut;
// 	SFLASH_UnitCmd_t	SetBurstWWrap;
// 	SFLASH_UnitCmd_t	PageProgram;
// 	SFLASH_UnitCmd_t	QuadInPageProgram;
// 	SFLASH_UnitCmd_t	SectorErase;
// 	SFLASH_UnitCmd_t	32KBBlockErase;
// 	SFLASH_UnitCmd_t	64KBBlockErase;
// 	SFLASH_UnitCmd_t	ChipErase;
// 	SFLASH_UnitCmd_t	EraseProgramSuspend;
// 	SFLASH_UnitCmd_t	EraseProgramResume;
// 	SFLASH_UnitCmd_t	PowerDown;
// 	SFLASH_UnitCmd_t	ReadManufacturerDeviceID;
// 	SFLASH_UnitCmd_t	ReadManufacturerDeviceIDDualInOut;
// 	SFLASH_UnitCmd_t	ReadManufacturerDeviceIDQuadInOut;
// 	SFLASH_UnitCmd_t	ReadUniqueIDNumber;
// 	SFLASH_UnitCmd_t	ReadJEDECID;
// 	SFLASH_UnitCmd_t	ReadSFDPRegister;
// 	SFLASH_UnitCmd_t	EraseSecurityRegisters;
// 	SFLASH_UnitCmd_t	ProgramSecurityRegisters;
// 	SFLASH_UnitCmd_t	ReadSecurityRegisters;
// 	SFLASH_UnitCmd_t	IndividualBlockSectorLock;
// 	SFLASH_UnitCmd_t	IndividualBlockSectorUnlock;
// 	SFLASH_UnitCmd_t	ReadBlockSectorLock;
// 	SFLASH_UnitCmd_t	GlobalBlockSectorLock;
// 	SFLASH_UnitCmd_t	GlovalBlockSectorUnlock;
// 	SFLASH_UnitCmd_t	EnableReset;
// 	SFLASH_UnitCmd_t	ResetDevice;
// }



// These are the building blocks that other functions can be made out of
// It is possible to use these functions directly to implement non-standard commands
SFLASH_Stat_e SFLASH_setUnit(SFLASH_Handle_t* pflash);									// Called if the desired unit number for a handle has changed
SFLASH_Stat_e SFLASH_command(SFLASH_Handle_t* pflash);								// Called to set the low level driver's command definition
SFLASH_Stat_e SFLASH_transmit(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);	// Called to transmit up to len bytes from pdata according to the currently set command structure
SFLASH_Stat_e SFLASH_receive(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);	// Called to receive up to len bytes into pdata according to the current command structure. Keep in mind that the number of data bytes is set in the command structure too

// Some utility functions for the library
SFLASH_Stat_e SFLASH_checkUnit(SFLASH_Handle_t* pflash);
SFLASH_Stat_e SFLASH_waitNotBusy(SFLASH_Handle_t* pflash, uint32_t timeout);


// These are easier ways to execute standard-supported commands
SFLASH_Stat_e 	SFLASH_WriteEnable(SFLASH_Handle_t* pflash);
SFLASH_Stat_e 	SFLASH_WriteEnableVolatileStatus(SFLASH_Handle_t* pflash);
SFLASH_Stat_e 	SFLASH_WriteDisable(SFLASH_Handle_t* pflash);
SFLASH_Stat_e 	SFLASH_ReadStatusReg1(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_ReadStatusReg2(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_ReadStatusReg3(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_WriteStatusReg1(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_WriteStatusReg2(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_WriteStatusReg3(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_ReadData(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_FastRead(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_FastReadDualOut(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_FastReadQuadOut(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_FastReadDualInOut(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_FastReadQuadInOut(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len);
//SFLASH_Stat_e 	SFLASH_SetBurstWWrap(SFLASH_Handle_t* pflash);
SFLASH_Stat_e 	SFLASH_PageProgram(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_QuadInPageProgram(SFLASH_Handle_t* pflash, uint32_t addr, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_SectorErase(SFLASH_Handle_t* pflash, uint32_t addr);
SFLASH_Stat_e 	SFLASH_32KBBlockErase(SFLASH_Handle_t* pflash, uint32_t addr);
SFLASH_Stat_e 	SFLASH_64KBBlockErase(SFLASH_Handle_t* pflash, uint32_t addr);
SFLASH_Stat_e 	SFLASH_ChipErase(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_EraseProgramSuspend(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_EraseProgramResume(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_PowerDown(SFLASH_Handle_t* pflash);
SFLASH_Stat_e 	SFLASH_ReadManufacturerDeviceID(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_ReadManufacturerDeviceIDDualInOut(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_ReadManufacturerDeviceIDQuadInOut(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
SFLASH_Stat_e 	SFLASH_ReadUniqueIDNumber(SFLASH_Handle_t* pflash, uint8_t* pdata, uint32_t len);
//SFLASH_Stat_e 	SFLASH_ReadJEDECID(SFLASH_Handle_t* pflash);
SFLASH_Stat_e 	SFLASH_ReadSFDPRegister(SFLASH_Handle_t* pflash, uint8_t offset, uint8_t* pdata, uint32_t len);
//SFLASH_Stat_e 	SFLASH_EraseSecurityRegisters(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_ProgramSecurityRegisters(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_ReadSecurityRegisters(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_IndividualBlockSectorLock(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_IndividualBlockSectorUnlock(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_ReadBlockSectorLock(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_GlobalBlockSectorLock(SFLASH_Handle_t* pflash);
//SFLASH_Stat_e 	SFLASH_GlovalBlockSectorUnlock(SFLASH_Handle_t* pflash);
SFLASH_Stat_e 	SFLASH_EnableReset(SFLASH_Handle_t* pflash);
SFLASH_Stat_e 	SFLASH_ResetDevice(SFLASH_Handle_t* pflash);






#endif /* __SFLASH_H_ */
