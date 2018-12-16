/*

This is written as the media access layer for "fatfs" 
Intended to be used on STM32 devices with SD cards 
connected to a SPI bus. 

fatfs: http://elm-chan.org/fsw/ff/00index_e.html

Author: Owen Lyke

*/


/*

The crash-course in how to use SPIDS:

What is SPIDS?
	SPIDS is a library aimed at simplifying the experience of using 
	SD cards with embedded systems. It allows users to easily define
	their own method of accessing the SD card and allows that method 
	to be used with the FatFS library that is already developed.
	Furthermore it provides simplified management of multiple drives
	and the ability to use several different interface drivers, for
	example SPI, SDIO 1bit, SDIO 4bit, etc. 
	Like in the FatFS library the actual physical interface to the SD
	card is left up to the user to create.
	The library uses techniques observed in original code by 
	STMicroelectronics in the STM32CubeMX software.


Why was it created?
	SPIDS was created to help demystify the use of SD cards and the 
	FatFS library. A common concensus among peers agreed that 
	implementing a custom SD solution was challenging. This project
	was undertaken to solve a specific problem and provide a basis 
	of knowledge that can be continued into the future.


How does SPIDS work?
	SPIDS will create a global structure called 'systemDisks.' This 
	structure is used to manage up to 10 physical SD (or MMC) cards
	on your system at a time. (The limit of 10 is imposed by FatFS)
	This structure keeps track of only a few really important things:
	1) whether or not a disk has been initialized
	2) how to actually talk to the disk
	3) unique identifiers for each disk (lun and path name)
	4) the number of disks that have active drivers

	Normally when FatFS uses the Media Access Interface it will call one of five
	functions that are defined by the user in "diskio.h" When using SPIDS a
	diskio.h is provided for FatFS to use, and it redirects calls to these basic
	functions as needed to call the appropriate driver to interact with
	the desired disk.


What is the bare minimum I need to do to use SPIDS?
	You need to write the appropriate Media Access Interface functions
	for YOUR particular use case. They of course must have the correct
	signature (return types and parameter types). 
	Then you will create a "SPIDS_DiskioDriver_Typedef" structure. The 
	elements of the structure will be the appropriate function pointers
	that can be used to call your specific interface.
	Finally you use the "SPIDS_LinkDriver_EX" function to link your driver
	to a disk. It will be added to the systemDisks structure to help 
	manage everything.

The bottom line:
	SPIDS includes the diskio.h file implementation that you need to get
	started writing your own SD interface. It also makes multi-disk 
	capability easier AND it has this nice explanation in the header file!
	What other SD library have you seen that does this??? 

*/






#ifndef SPIDS_H
#define SPIDS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "ff.h"
#include "stdint.h"
#include "SPIDS_sd_reference_vals.h"

#define _USE_WRITE	1	/* 1: Enable disk_write function */
#define _USE_IOCTL	1	/* 1: Enable disk_ioctl function */

//extern uint8_t retSD; /* Return value for SD */
//extern char SDPath[4]; /* SD logical drive path */
//extern FATFS SDFatFS; /* File system object for SD logical drive */
//extern FIL SDFile; /* File object for SD */
//
//void MX_FATFS_Init(void);

#define _VOLUMES FF_VOLUMES

// Some needed typedefs (usually found in diskio.h)
 /* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


// SD Cardstate
typedef enum
{
  SPIDS_SD_CARD_READY                  = 0x00000001U,  /*!< Card state is ready                     */
  SPIDS_SD_CARD_IDENTIFICATION         = 0x00000002U,  /*!< Card is in identification state         */
  SPIDS_SD_CARD_STANDBY                = 0x00000003U,  /*!< Card is in standby state                */
  SPIDS_SD_CARD_TRANSFER               = 0x00000004U,  /*!< Card is in transfer state               */
  SPIDS_SD_CARD_SENDING                = 0x00000005U,  /*!< Card is sending an operation            */
  SPIDS_SD_CARD_RECEIVING              = 0x00000006U,  /*!< Card is receiving operation information */
  SPIDS_SD_CARD_PROGRAMMING            = 0x00000007U,  /*!< Card is in programming state            */
  SPIDS_SD_CARD_DISCONNECTED           = 0x00000008U,  /*!< Card is disconnected                    */
  SPIDS_SD_CARD_ERROR                  = 0x000000FFU   /*!< Card response Error                     */
}SPIDS_SD_CardStateTypeDef;


// SD card format (standard capacity, HC, XC etc)
typedef enum{
	SPIDS_SD_HW_SC = 0,
	SPIDS_SD_HW_HC,
	SPIDS_SD_HW_XC
}SPIDS_SD_HW_FORMAT_TypeDef;



/**

// Driver Structure

*/
typedef struct{
	DSTATUS (*disk_initialize) (BYTE);                     			/*!< Initialize Disk Drive                     */
	DSTATUS (*disk_status)     (BYTE);                     			/*!< Get Disk Status                           */
	DRESULT (*disk_read)       (BYTE, BYTE*, DWORD, UINT);       	/*!< Read Sector(s)                            */
#if _USE_WRITE == 1
  	DRESULT (*disk_write)      (BYTE, const BYTE*, DWORD, UINT); 	/*!< Write Sector(s) when _USE_WRITE = 0       */
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  	DRESULT (*disk_ioctl)      (BYTE, BYTE, void*);              	/*!< I/O control operation when _USE_IOCTL = 1 */
#endif /* _USE_IOCTL == 1 */

}SPIDS_DiskioDriver_Typedef;




/**

// Disk Manager

*/
/**
  	* @brief  Disk IO Driver structure definition
	*
	* Used to keep track of what drivers are used
	* for each physical drive, as well as to create
	* drive numbers
  */
typedef struct
{
  uint8_t                 			is_initialized[_VOLUMES];	// Indicates whether each disk is initialized
  const SPIDS_DiskioDriver_Typedef 	*drv[_VOLUMES];				// Holds a pointer to a driver for each disk
  uint8_t                 			lun[_VOLUMES];				// Holds lun for a disk if it is used with USB Key Disk
  volatile uint8_t        			nbr;						// Number of active linked drivers
}SPIDS_DiskManager_Typedef;

uint8_t SPIDS_LinkDriver_EX(const SPIDS_DiskioDriver_Typedef *drv, char *path, uint8_t lun);
uint8_t SPIDS_LinkDriver(const SPIDS_DiskioDriver_Typedef *drv, char *path);
uint8_t SPIDS_UnLinkDriver_EX(char *path, uint8_t lun);
uint8_t SPIDS_UnLinkDriver(char *path);
uint8_t SPIDS_GetAttachedDriversNbr(void);


// Here is the interface defined by fatfs:

//DSTATUS disk_status (
//	BYTE pdrv     /* [IN] Physical drive number */
//);
//
//DSTATUS disk_initialize (
//	BYTE pdrv           /* [IN] Physical drive number */
//);
//
//DRESULT disk_read (
//	BYTE pdrv,     /* [IN] Physical drive number */
//	BYTE* buff,    /* [OUT] Pointer to the read data buffer */
//	DWORD sector,  /* [IN] Start sector number */
//	UINT count     /* [IN] Number of sectros to read */
//);
//
//DRESULT disk_write (
//	BYTE pdrv,        /* [IN] Physical drive number */
//	const BYTE* buff, /* [IN] Pointer to the data to be written */
//	DWORD sector,     /* [IN] Sector number to write from */
//	UINT count        /* [IN] Number of sectors to write */
//);
//
//DRESULT disk_ioctl (
//	BYTE pdrv,     /* [IN] Drive number */
//	BYTE cmd,      /* [IN] Control command code */
//	void* buff     /* [I/O] Parameter and data buffer */
//);
//
//DWORD get_fattime (void);

#ifdef __cplusplus
}
#endif
#endif
