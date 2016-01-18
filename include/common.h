#ifndef __WORM_COMMON_H__
#define __WORM_COMMON_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

enum {HELLO, SETUP, PING, PONG, DOWNLINK, OVERLOAD, UNDERLOAD} ctrlMsgType;

typedef struct {
	uint16_t id;
	uint16_t listenPort;
	uint16_t core;
	uint8_t *connectionDescription; // (LISP connection description)
} WormSetup;

typedef struct {
	uint16_t id;
} PongStats;


#endif
