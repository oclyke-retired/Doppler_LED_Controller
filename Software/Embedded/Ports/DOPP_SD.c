#include "DOPP_SD.h"


SPIDS_STM32F7_SPI_Settings_TypeDef 	DOPP_STM32F7_SPI_Settings;
SPIDS_DiskioDriver_Typedef			DOPP_STM32F7_SPI_Driver;
char DOPP_SDPath[4];   /* SD logical drive path */


// Wrappers
DSTATUS DOPP_SD_disk_initialize(BYTE pdrv){ return SPIDS_STM32F7_SPI_disk_initialize(pdrv, &DOPP_STM32F7_SPI_Settings); }
DSTATUS DOPP_SD_disk_status(BYTE pdrv){ return SPIDS_STM32F7_SPI_disk_status(pdrv, &DOPP_STM32F7_SPI_Settings); }
DRESULT DOPP_SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count){ return SPIDS_STM32F7_SPI_disk_read( pdrv, buff, sector, count, &DOPP_STM32F7_SPI_Settings); }
#if _USE_WRITE == 1
DRESULT DOPP_SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count){ return SPIDS_STM32F7_SPI_disk_write( pdrv, buff, sector, count, &DOPP_STM32F7_SPI_Settings); }
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
DRESULT DOPP_SD_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff){ return SPIDS_STM32F7_SPI_disk_ioctl( pdrv, cmd, buff, &DOPP_STM32F7_SPI_Settings); }
#endif /* _USE_IOCTL == 1 */
DWORD DOPP_SD_get_fattime (void){ return SPIDS_STM32F7_SPI_get_fattime(&DOPP_STM32F7_SPI_Settings); }




// Initialization function
void DOPP_Begin_SD( void )
{
	// Define SPI interface settings
	SPIDS_STM32F7_SPI_Settings_Initialize(&DOPP_STM32F7_SPI_Settings);		// Set the SPI settings to default for safety
	DOPP_STM32F7_SPI_Settings.hspi = &hspi1;								// Use hspi1
	DOPP_STM32F7_SPI_Settings.CS_GPIO_Port = CS_SD_GPIO_Port;				// Use the sd_cs port
	DOPP_STM32F7_SPI_Settings.CS_Pin = CS_SD_Pin;							// Use the sd_cs pin
	DOPP_STM32F7_SPI_Settings.max_freq = 20000000;							// Only 20 MHz
	// leave init freq unchanged
	// Leave timeout unchanged

	// Link driver functions
	DOPP_STM32F7_SPI_Driver.disk_initialize 	= DOPP_SD_disk_initialize;                     							/*!< Initialize Disk Drive                     */
	DOPP_STM32F7_SPI_Driver.disk_status 		= DOPP_SD_disk_status;
	DOPP_STM32F7_SPI_Driver.disk_read 			= DOPP_SD_disk_read;
	#if _USE_WRITE == 1
	DOPP_STM32F7_SPI_Driver.disk_write 		= DOPP_SD_disk_write;
	#endif /* _USE_WRITE == 1 */
	#if _USE_IOCTL == 1
	DOPP_STM32F7_SPI_Driver.disk_ioctl 		= DOPP_SD_disk_ioctl;
	#endif /* _USE_IOCTL == 1 */

	// Now link the driver into the disk manager(?)
	SPIDS_LinkDriver(&DOPP_STM32F7_SPI_Driver, DOPP_SDPath); // The first drive linked in should be at path "0:/\0"

	HAL_SPI_MspInit(&hspi1);
}
