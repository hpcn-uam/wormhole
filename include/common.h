#ifndef __WORM_COMMON_H__
#define __WORM_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

	enum ctrlMsgType {
		HELLO, SETUP, QUERYID, RESPONSEID, PING, PONG, DOWNLINK, OVERLOAD, UNDERLOAD
	};

	typedef struct {
		uint16_t id;
		uint16_t listenPort;
		uint16_t core;
		uint32_t IP; //TODO fix para ipv6
		uint8_t *connectionDescription; // (LISP connection description)
	} WormSetup;

	typedef struct {
		uint16_t id;
	} PongStats;

#ifdef __cplusplus
}
#endif
#endif
