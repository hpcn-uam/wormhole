#ifndef __EINSTEIN_H__
#define __EINSTEIN_H__

#include "common.h"

typedef struct
{
	WormSetup ws;
	uint32_t IP; //TODO fix para ipv6
	int socket;
} Eins2WormConn;


#endif
