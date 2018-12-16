



#ifndef __DOPP_SD_H_
#define __DOPP_SD_H_

#include "SPIDS_STM32F7_SPI_Driver.h"
#include "spi.h"


extern SPIDS_STM32F7_SPI_Settings_TypeDef  	DOPP_STM32F7_SPI_Settings;
extern SPIDS_DiskioDriver_Typedef			DOPP_STM32F7_SPI_Driver;
extern char 								DOPP_SDPath[4];   			/* SD logical drive path */

// These functions here are wrappers for the SPI interface functions. They are defined by the user to pass in the desired spi settings in addition to normal fatfs parameters
DSTATUS DOPP_SD_disk_initialize(BYTE pdrv);                     							/*!< Initialize Disk Drive                     */
DSTATUS DOPP_SD_disk_status(BYTE pdrv);                     								/*!< Get Disk Status                           */
DRESULT DOPP_SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);       			/*!< Read Sector(s)                            */
DRESULT DOPP_SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count); 			/*!< Write Sector(s) when _USE_WRITE = 0       */
DRESULT DOPP_SD_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);              					/*!< I/O control operation when _USE_IOCTL = 1 */
DWORD DOPP_SD_get_fattime (void);


void DOPP_Begin_SD( void );


#endif /* __DOPP_SD_H_ */
