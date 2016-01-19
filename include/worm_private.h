#ifndef __WORM_PRIV_H__
#define __WORM_PRIV_H__

#include "worm.h"
#include "einstein.h"

typedef struct
{
	uint16_t Port;
	uint32_t IP; //TODO fix para ipv6
	int socket;
} Worm2EinsConn;


#endif
