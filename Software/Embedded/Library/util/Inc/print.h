/*

This is a file to easily print formatted strings through an arbitrary interface

*/

#ifndef __PRINT_H_
#define __PRINT_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "print_setup.h"

/* Note:
 * Not all format specifiers are enabled by default (when using newlib nano)
 * If you have trouble using certain format specifiers then as a first step make
 * sure that you're using full-featured newlib (or some equivalent note for non-stm32f7 if needed)
 */


#ifndef PRINT_ERR
#define PRINT_ERR 0 // Default to not showing errors in the stream
#endif

#ifndef PRINT_FMT
#define PRINT_FMT 0 // Don't allow formatted printing by default
#endif

#ifndef PRINT_DYN
#define PRINT_DYN 0U 	// Default to not allow dynamic memory allocation
#endif

#ifndef PRINT_DYN_MAX
#define PRINT_DYN_MAX 128U // Default to 128 bytes max
#endif

//#ifndef PRINT_EXP_BUFF_LEN
//#define PRINT_EXP_BUFF_LEN (0x100)	// Use up 128 bytes by default
//#endif

typedef char*	print_sp_t;		// These defined because you can't pass "char*" into a macro function by itself (should I use a #define instead?)
typedef int*	print_ip_t;

// Here are some type definitions because passing pointers to pointers gets a little tedious
typedef char* 		char_p;
typedef char_p* 	char_pp;
typedef uint8_t*	uint8_p;
typedef uint8_p*	uint8_pp;

typedef enum {
	PRINT_ok = 0x00,
	PRINT_error
}PRINT_Stat_e;

typedef struct _PRINT_Intfc_t PRINT_Intfc_t;

struct _PRINT_Intfc_t{
	uint8_t*	pbuff;		// Pointer to your working buffer
	uint32_t	bufflen;	// How many chars can occupy the buffer?

	volatile bool	busy;		// Is the hardware busy?

	PRINT_Stat_e	(*txBytes)(PRINT_Intfc_t* pintfc, uint8_t* pdata, uint16_t len);
	void*			(*malloc)(size_t size);

	// Not for the user...
	uint16_t 	btt;	// This is used internally to track how many bytes to transmit. It should not exceed bufflen
};

PRINT_Stat_e	mprintf(PRINT_Intfc_t* pintfc, const char* format, ...);	// Print with formatting
PRINT_Stat_e	mprint(PRINT_Intfc_t* pintfc, const char* pdata);		// Print null-term string directly


#endif /* __PRINT_H_ */
