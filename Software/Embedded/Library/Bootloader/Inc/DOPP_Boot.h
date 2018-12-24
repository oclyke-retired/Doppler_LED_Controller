/*



*/

#ifndef __DOPP_BOOT_H_
#define __DOPP_BOOT_H_


#include "gpio.h"	// You'd include the headers that you need here. For me it is the STM32CubeMX generated code
#include "usart.h"	//


typedef enum {
	BOOT_Init_None		= 0x00,
	BOOT_Init_First 	= 0x01,

}BOOT_bmInit_e;

typedef enum {
	BOOT_ok,
	BOOT_error
}BOOT_bStatus_e;


BOOT_bStatus_e firstInit( void );



// extern BOOT_InitT_e initTracker;







#endif /* __DOPP_BOOT_H_ */
