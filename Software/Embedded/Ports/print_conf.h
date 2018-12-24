#ifndef __PRINT_CONF_H_
#define __PRINT_CONF_H_

#include "print.h"
#include "usart.h"


#define DEBUG_PRINT_BUFF_LEN 5

extern PRINT_Intfc_t debugIntfc;

PRINT_Stat_e debug_link( void );	// User calls this directly

// These are used by the print header file
PRINT_Stat_e debug_txBytes(PRINT_Intfc_t* pintfc, uint8_t* pdata, uint16_t len);
void* debug_malloc(size_t size);

#endif /* __PRINT_CONF_H_ */
