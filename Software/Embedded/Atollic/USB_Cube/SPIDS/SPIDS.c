/*

This is written as the media access layer for "fatfs"
Intended to be used on STM32 devices with SD cards
connected to a SPI bus.

Author: Owen Lyke
June 2018

*/


#include "SPIDS.h"


/*

// Disk Manager

*/
SPIDS_DiskManager_Typedef systemDisks = {{0},{0},{0},0};	// This is the global physical drive (disk) manager


/**
  * @brief  Links a compatible diskio driver/lun id and increments the number of active
  *         linked drivers.
  * @note   The number of linked drivers (volumes) is up to 10 due to FatFs limits.
  * @param  drv: pointer to the disk IO Driver structure
  * @param  path: pointer to the logical drive path
  * @param  lun : only used for USB Key Disk to add multi-lun management
            else the parameter must be equal to 0
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t SPIDS_LinkDriver_EX(const SPIDS_DiskioDriver_Typedef *drv, char *path, uint8_t lun)
{
  uint8_t ret = 1;
  uint8_t DiskNum = 0;

  if(systemDisks.nbr < _VOLUMES)
  {
    systemDisks.is_initialized[systemDisks.nbr] = 0;
    systemDisks.drv[systemDisks.nbr] = drv;
    systemDisks.lun[systemDisks.nbr] = lun;
    DiskNum = systemDisks.nbr++;
    path[0] = DiskNum + '0';
    path[1] = ':';
    path[2] = '/';
    path[3] = 0;
    ret = 0;
  }

  return ret;
}

/**
  * @brief  Links a compatible diskio driver and increments the number of active
  *         linked drivers.
  * @note   The number of linked drivers (volumes) is up to 10 due to FatFs limits
  * @param  drv: pointer to the disk IO Driver structure
  * @param  path: pointer to the logical drive path
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t SPIDS_LinkDriver(const SPIDS_DiskioDriver_Typedef *drv, char *path)
{
  return SPIDS_LinkDriver_EX(drv, path, 0);
}

/**
  * @brief  Unlinks a diskio driver and decrements the number of active linked
  *         drivers.
  * @param  path: pointer to the logical drive path
  * @param  lun : not used
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t SPIDS_UnLinkDriver_EX(char *path, uint8_t lun)
{
  uint8_t DiskNum = 0;
  uint8_t ret = 1;

  if(systemDisks.nbr >= 1)
  {
    DiskNum = path[0] - '0';
    if(systemDisks.drv[DiskNum] != 0)
    {
      systemDisks.drv[DiskNum] = 0;
      systemDisks.lun[DiskNum] = 0;
      systemDisks.nbr--;
      ret = 0;
    }
  }

  return ret;
}

/**
  * @brief  Unlinks a diskio driver and decrements the number of active linked
  *         drivers.
  * @param  path: pointer to the logical drive path
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t SPIDS_UnLinkDriver(char *path)
{
  return SPIDS_UnLinkDriver_EX(path, 0);
}

/**
  * @brief  Gets number of linked drivers to the FatFs module.
  * @param  None
  * @retval Number of attached drivers.
  */
uint8_t SPIDS_GetAttachedDriversNbr(void)
{
  return systemDisks.nbr;
}



/*

// Driver Structure

*/
// Actually its fairly nice: there is no required code for this section
// Here's an example of how to link up your Media Access Interface functions:
// FatFS Name 		: 		Your corresponding function
// ----------------------------------------------------
// disk_status		:		my_disk_status
// disk_initialize	:		my_disk_initialize
// disk_read		:		my_disk_read
// disk_write		:		my_disk_write
// disk_ioctl		:		my_disk_ioctl		* optional implementation
// get_fattime		:		my_get_fattime


/*
SPIDS was created with the ability to make generic drivers in mind.
For a case study imagine the desire to make a SPI based driver that
allows one to choose which SPI hardware and which CS pin are used:

One might think to store these values in the the driver structure like so:

typedef struct
{
	SPIPORT * spi_to_use;
	PINNUM 		pin_to_use;

	DSTATUS (*disk_initialize) (BYTE);                     			!< Initialize Disk Drive
	DSTATUS (*disk_status)     (BYTE);                     			// !< Get Disk Status
	DRESULT (*disk_read)       (BYTE, BYTE*, DWORD, UINT);       	// !< Read Sector(s)
//#if _USE_WRITE == 1
  	DRESULT (*disk_write)      (BYTE, const BYTE*, DWORD, UINT); 	// !< Write Sector(s) when _USE_WRITE = 0
//#endif // _USE_WRITE == 1
//#if _USE_IOCTL == 1
  	DRESULT (*disk_ioctl)      (BYTE, BYTE, void*);              	// !< I/O control operation when _USE_IOCTL = 1
//#endif // _USE_IOCTL == 1

//}FancyCoolDriverWSPI;

Regrettably one cannot make a new typedef like this and expect to link it
into the systemDisks structure becuase it is the wrong type! (Here's where
you COULD go ahead and re-write a lot of SPIDS to make it work with the new
structure you want... for example you modify systemDisks to take
'FancyCoolDriverWSPI' types and the driver functions look at the parameters
stored in the driver to determine which SPI port and CS pin to use. Hey, that
would be pretty cool!)

But to maintain a high level of generality there is another OK solution (not ideal,
but OK). You can simply declare a global variable that contains the desired
parameters as well as a 'generic' SPI driver function that takes the desired
parameters as well. Then when you link the driver for a specific device you can
just make a small wrapper function that is specific to that device. This solution
does not lend itself well to dynamically variable interfaces, but hey when will
that ever happen??

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

*/


/*
DSTATUS specific_driver1_initialize(BYTE pdrv)
{
driver1_special_param = 1; // This might be a chip select pin or something
	generic_initialize(prdv, driver1_special_param);
}

// yadda yadda yadda
*/
