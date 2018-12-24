#include "print_conf.h"






uint8_t debug_buff[DEBUG_PRINT_BUFF_LEN];
PRINT_Intfc_t debugIntfc;

PRINT_Stat_e debug_link( void )
{
	PRINT_Stat_e stat = PRINT_ok;

	debugIntfc.bufflen = DEBUG_PRINT_BUFF_LEN;
	debugIntfc.pbuff = debug_buff;
	debugIntfc.busy = false;
	debugIntfc.txBytes = debug_txBytes;
	debugIntfc.malloc = debug_malloc;

	return stat;
}



PRINT_Stat_e debug_txBytes(PRINT_Intfc_t* pintfc, uint8_t* pdata, uint16_t len){
	HAL_StatusTypeDef hal_result = HAL_OK;
	PRINT_Stat_e print_result = PRINT_ok;

	pintfc->busy = true;
	hal_result = HAL_UART_Transmit(&huart4, pdata, len, 1000);
	pintfc->busy = false;

	switch(hal_result){
	case HAL_OK :
		print_result = PRINT_ok;
		break;
	default :
		print_result = PRINT_error;
		break;
	}
	return print_result;
}

void* debug_malloc(size_t size){
	return malloc(size);
}
