#include "DOPP_Boot.h"

BOOT_bmInit_e initTracker;


BOOT_bStatus_e firstInit( void )
{
	// This function initializes the peripherals that are needed immediately when the bootloader starts
	// - UART4 (For status transmissions to the user via the ESP32)
	// - GPIO (For LED blinkies)

	MX_GPIO_Init();
	MX_UART4_Init();

	HAL_UART_MspInit(&huart4);

	if(1) // If the initialization was successful then record that in the initTracker
	{
		initTracker |= BOOT_Init_First;
	}

	return BOOT_ok;
}
