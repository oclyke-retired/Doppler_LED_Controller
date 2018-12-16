#ifndef BUFFERS_H
#define BUFFERS_H

#include <stdint.h>
#include <stdbool.h>


#define BUFF_LEN 256



typedef struct buffman{
	uint8_t* pbuff;
	uint32_t len;
	uint32_t readOffset;
	uint32_t writeOffset;
	bool readOk;
	bool writeOk;
}buffer_manager_t;


void initBuffMan( buffer_manager_t* pman, uint8_t* pdata, uint32_t len );
uint32_t available( buffer_manager_t* pman );
void insert( buffer_manager_t* pman, uint8_t c );
uint8_t read( buffer_manager_t* pman );


#endif
