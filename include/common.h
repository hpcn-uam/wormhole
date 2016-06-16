#ifndef __WORM_COMMON_H__
#define __WORM_COMMON_H__

#include <arpa/inet.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <strings.h>
#include <errno.h>

#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

#include <hptl.h>
#include <netlib.h>

#ifdef __cplusplus
extern "C" {
#endif

	enum ctrlMsgType {
		HELLOEINSTEIN, STARTSSL, SETUP, QUERYID, RESPONSEID, PING, PONG, CHANGEROUTE, DOWNLINK, OVERLOAD, UNDERLOAD, CTRL_OK, CTRL_ERROR, HALT, ABORT, PRINTMSG
	};

	typedef struct { //__attribute__(packet)??
		uint16_t id;
		uint16_t listenPort;
		uint32_t IP; //TODO fix para ipv6
		uint8_t isSSLNode;
		uint8_t reservedFlag4; //Prevents valgrind unitialized errors. Is not used at all
		uint8_t reservedFlag2; //Prevents valgrind unitialized errors. Is not used at all
		uint8_t reservedFlag3; //Prevents valgrind unitialized errors. Is not used at all
		uint32_t connectionDescriptionLength;
		uint8_t *connectionDescription; // (LISP connection description)
		int64_t core;
	} WormSetup;

	typedef struct {
		uint16_t id;
	} PongStats;

#ifdef __cplusplus
}
#endif
#endif
