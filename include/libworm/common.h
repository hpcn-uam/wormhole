#ifndef __WORM_COMMON_H__
#define __WORM_COMMON_H__

#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <hptl.h>
#include <netlib.h>

#include <wh_config.h>

// API VERSION
#define WORMVERSION 0
#define ZEUSVERSION 0

#ifndef uint128_t
#ifdef __clang__
#pragma message "WARNING: This compiler does not support uint128_t: DO NOT USE IPv6!!!"
#define uint128_t uint64_t;
#else
typedef unsigned __int128 uint128_t;
#endif
#endif

#ifdef __cplusplus
#define restrict __restrict__  // to solve some compilation bugs
extern "C" {
#endif

enum ctrlMsgType {
	HELLOZEUS,
	STARTSSL,
	SETUP,
	QUERYID,
	RESPONSEID,
	PING,
	PONG,
	CHANGEROUTE,
	DOWNLINK,
	OVERLOAD,
	UNDERLOAD,
	CTRL_OK,
	CTRL_ERROR,
	HALT,
	ABORT,
	PRINTMSG,
	TIMEOUT
};

enum queryType { qUNSUPORTED, qSTATISTICS_IN, qSTATISTICS_OUT };

char *ctrlMsgType2str(enum ctrlMsgType msg);

typedef struct {  //__attribute__(packet)??
	uint128_t IP;
	uint16_t listenPort;
	// 128+16
	uint16_t id;
	// 128+32
	uint8_t isSSLNode : 1;
	uint8_t zeusSSL : 1;
	uint8_t isIPv6 : 1;            // if false, ipv4
	uint8_t reservedBitFlag0 : 1;  // Prevents valgrind unitialized errors. It is also used for memory alignament. It do not
	                               // stores anyting (yet)
	uint8_t reservedBitFlag1 : 1;  // Prevents valgrind unitialized errors. It is also used for memory alignament. It do not
	                               // stores anyting (yet)
	uint8_t reservedBitFlag2 : 1;  // Prevents valgrind unitialized errors. It is also used for memory alignament. It do not
	                               // stores anyting (yet)
	uint8_t reservedBitFlag3 : 1;  // Prevents valgrind unitialized errors. It is also used for memory alignament. It do not
	                               // stores anyting (yet)
	uint8_t reservedBitFlag4 : 1;  // Prevents valgrind unitialized errors. It is also used for memory alignament. It do not
	                               // stores anyting (yet)
	uint8_t reservedFlag1;  // Prevents valgrind unitialized errors. It is also used for memory alignament. It do not stores
	                        // anyting (yet)
	uint8_t zeusVersion;
	uint8_t holeVersion;
	// 128+64
	int64_t core;  // affinity
	// 128+128
	uint8_t *connectionDescription;  // (LISP connection description)
	uint32_t connectionDescriptionLength;
} HoleSetup;

typedef struct {
	uint16_t id;
} PongStats;
#ifdef WH_PREFETCHING  // default: nope
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

#ifdef WH_STATISTICS
typedef struct {
	uint64_t holeId;
	uint64_t totalIO;
	uint64_t lastMinIO_tmp;
	uint64_t lastMinIO;
	uint64_t lastCheck;  // hptl_t
} ConnectionStatistics;
#endif

#ifdef __cplusplus
}
#endif
#endif
