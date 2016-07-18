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
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <strings.h>
#include <errno.h>
#include <poll.h>

#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

#include <hptl.h>
#include <netlib.h>

#define WORMVERSION 0
#define EINSTEINVERSION 0

#ifndef uint128_t
typedef unsigned __int128 uint128_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

	enum ctrlMsgType {
		HELLOEINSTEIN, STARTSSL, SETUP, QUERYID, RESPONSEID, PING, PONG, CHANGEROUTE, DOWNLINK, OVERLOAD, UNDERLOAD, CTRL_OK, CTRL_ERROR, HALT, ABORT, PRINTMSG, TIMEOUT
	};

	char *ctrlMsgType2str(enum ctrlMsgType msg);

	typedef struct { //__attribute__(packet)??
		uint128_t IP;
		uint16_t listenPort;
		//128+16
		uint16_t id;
		//128+32
		uint8_t isSSLNode : 1;
		uint8_t einsteinSSL : 1;
		uint8_t isIPv6 : 1; //if false, ipv4
		uint8_t reservedBitFlag0 : 1; //Prevents valgrind unitialized errors. It is also used for memory alignament. It do not stores anyting (yet)
		uint8_t reservedBitFlag1 : 1; //Prevents valgrind unitialized errors. It is also used for memory alignament. It do not stores anyting (yet)
		uint8_t reservedBitFlag2 : 1; //Prevents valgrind unitialized errors. It is also used for memory alignament. It do not stores anyting (yet)
		uint8_t reservedBitFlag3 : 1; //Prevents valgrind unitialized errors. It is also used for memory alignament. It do not stores anyting (yet)
		uint8_t reservedBitFlag4 : 1; //Prevents valgrind unitialized errors. It is also used for memory alignament. It do not stores anyting (yet)
		uint8_t reservedFlag1; 		  //Prevents valgrind unitialized errors. It is also used for memory alignament. It do not stores anyting (yet)
		uint8_t einsteinVersion;
		uint8_t wormVersion;
		//128+64
		int64_t core; //affinity
		//128+128
		uint8_t *connectionDescription; // (LISP connection description)
		uint32_t connectionDescriptionLength;
	} WormSetup;

	typedef struct {
		uint16_t id;
	} PongStats;
#define WH_PREFETCHING
#ifdef WH_PREFETCHING //default: nope
	/**
	* Prefetch a cache line into all cache levels.
	* @param p
	*   Address to prefetch
	*/
	static inline void WH_prefetch0(volatile void *p)
	{
		asm volatile("prefetcht0 %[p]" : [p] "+m"(*(volatile char *)p));
	}

	/**
	* Prefetch a cache line into all cache levels except the 0th cache level.
	* @param p
	*   Address to prefetch
	*/
	static inline void WH_prefetch1(volatile void *p)
	{
		asm volatile("prefetcht1 %[p]" : [p] "+m"(*(volatile char *)p));
	}

	/**
	* Prefetch a cache line into all cache levels except the 0th and 1th cache
	* levels.
	* @param p
	*   Address to prefetch
	*/
	static inline void WH_prefetch2(volatile void *p)
	{
		asm volatile("prefetcht2 %[p]" : [p] "+m"(*(volatile char *)p));
	}
#endif

#ifdef __cplusplus
}
#endif
#endif
