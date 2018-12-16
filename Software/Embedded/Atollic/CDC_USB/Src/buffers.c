#include "buffers.h"


void initBuffMan( buffer_manager_t* pman, uint8_t* pdata, uint32_t len )
{
	pman->pbuff = pdata;
	pman->len = len;
	pman->writeOffset = 0;
	pman->readOffset = 0;
	pman->readOk = false;
	pman->writeOk = true;
}

uint32_t available( buffer_manager_t* pman )
{
	if( pman->writeOffset == pman->readOffset )
	{
		if(pman->readOk)
		{
			return pman->len;
		}
		else
		{
			return 0;
		}
	}
	else if( pman->writeOffset > pman->readOffset )
	{
		return ((pman->writeOffset) - (pman->readOffset));
	}
	else
	{
		return ((pman->len - pman->readOffset) + (pman->writeOffset));
	}
}

void insert( buffer_manager_t* pman, uint8_t c )
{
	if( pman->writeOk )
	{
		*(pman->pbuff + pman->writeOffset) = c;
		pman->writeOffset++;
		if(pman->writeOffset >= pman->len)
		{
			pman->writeOffset = 0;
		}
		if(pman->writeOffset == pman->readOffset)
		{
			pman->writeOk = false;
		}
		pman->readOk = true;
	}
}

uint8_t read( buffer_manager_t* pman )
{
	uint8_t retval = 0x00;
	if(pman->readOk)
	{
		retval = *(pman->pbuff + pman->readOffset);
		pman->readOffset++;
		if(pman->readOffset >= pman->len)
		{
			pman->readOffset = 0;
		}
		if( pman->readOffset == pman->writeOffset )
		{
			pman->readOk = false;
		}
		pman->writeOk = true;
	}
	return retval;
}
