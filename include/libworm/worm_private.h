#ifndef __WORM_PRIV_H__
#define __WORM_PRIV_H__

#include <worm.h>

#ifdef __cplusplus
extern "C" {
#endif

enum wormMsgType { HELLO, SSLSTART, WORMINFO, SETUPWORMCONN };

enum wormErrorType { WH_ERRNO_CLEAR = 0, WH_ERRNO_EMPTY = 1, WH_ERRNO_CLOSED = 2 };

typedef struct {
	SyncSocket *socket;
	char *ip;
	uint16_t Port;
} Worm2EinsConn;

typedef struct {
	AsyncSocket socket;
	ConnectionDataType type;
#ifdef _WORMLIB_STATISTICS_
	ConnectionStatistics stats;
#endif
} Connection;

typedef struct {
	size_t numberOfTypes;
	ConnectionDataType *supportedTypes;
	Connection **conns;
	uint16_t id;
	uint16_t port;
	char ip[INET6_ADDRSTRLEN];
} DestinationWorm;

typedef struct {
	size_t numberOfWorms;
	DestinationWorm *worms;
} DestinationWorms;

/** WH_connectWorm
 * Connect and fill the socket data.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_connectWorm(DestinationWorm *c);

/** WH_setupConnectionType
 * Setup connection type
 * @return 0 if OK, something else if error.
 */
uint8_t WH_setupConnectionType(DestinationWorm *dw, const ConnectionDataType *const type);

/** WH_getWormData
 * Gets worm data (IP+port).
 * @return 0 if OK, something else if error.
 */
uint8_t WH_getWormData(WormSetup *ws, const uint16_t wormId);

/** WH_addWorm
 * @param init Determines if the worm should be initialized or not
 * @return the created connection
 */
DestinationWorm *WH_addWorm(DestinationWorms *wms, const uint16_t wormId, const uint8_t init);

/** WH_removeWorm
 * Removes, close connections and frees all the data related to that worm.
 */
void WH_removeWorm(DestinationWorms *wms, const uint16_t wormId);

/** WH_findWorm
 * @return the worm mached (if no exists)
 */
DestinationWorm *WH_findWorm(DestinationWorms *wms, const uint16_t wormId);

/** WH_findWormIndex
 * @return the worm index in DestinationWorms
 */
size_t WH_findWormIndex(DestinationWorms *wms, const uint16_t wormId);

/** WH_thread
 * A worm Thread listening for info/petitions.
 */
void *WH_thread(void *arg);

/** WH_TH_checkCtrlMsgType
 * check a control message type from Einstein
 * @return 0 if ok, -1 if error, and 1 if socket wont receive more control data.
 */
int WH_TH_checkCtrlMsgType(enum ctrlMsgType type, SyncSocket *socket);

/** WH_TH_checkMsgType
 * check the message type
 * @return 0 if ok, 1 if some error
 */
int WH_TH_checkMsgType(enum wormMsgType type, SyncSocket *socket);

/** WH_TH_hello
 * Process a HELLOW message
 */
void WH_TH_hello(SyncSocket *socket);

/** SETUPWORMCONN
 * Process a HELLOW message
 */
void WH_TH_setupworm(SyncSocket *socket);

/** WH_connectionPoll
 * Poll data from some socket
 * @return some connection with data, NULL if error/timeout.
 */
Connection *WH_connectionPoll(DestinationWorms *wms, MessageInfo *mi);

/** WH_considerSocket
 * Check if the socket would complete the request
 * @return 1 if yes, 0 if no
 */
int WH_considerSocket(AsyncSocket *sock, MessageInfo *mi);

/** WH_typesize
 * @return the size of the tipe provided
 */
size_t WH_typesize(const ConnectionDataType *const type);

/*
 Dynamic Routing Library
*/
/* DUP, CAT, RR, HASH */
enum RoutingRule { DUPLICATE, CATEGORY, ROUNDROBIN, HASH };

/** WH_DymRoute_init
 * Starts the Dynamic Routing Library, and setups connection configuration.
 * Also makes connections
 * @return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_init(const uint8_t *const routeDescription, DestinationWorms *cns);

/** WH_DymRoute_route_create
 * Searchs for a Function, and calls the correct one.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_create(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

/** WH_DymRoute_route_createFuncDUP
 * Adds a "c code" to duplicate messages.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_createFuncDUP(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

/** WH_DymRoute_route_createFuncRR
 * Adds a "c code" for round robin.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_createFuncRR(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

/** WH_DymRoute_route_createFuncCat
 * Adds a "c code" for category splitting.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_createFuncCat(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

/** WH_DymRoute_route_createFuncHash
 * Adds a "c code" for hashing splitting.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_createFuncHash(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

/** WH_DymRoute_route_createFuncIgnore
 * Adds a "c code" for ignoring packets
 * @return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_createFuncIgnore(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

/** WH_DymRoute_route_countElems
 * Counts the following elements, for example (1 2), returns 2, but (1 (2 3)) also returns 2.
 * @return the number of elements.
 */
uint32_t WH_DymRoute_route_countElems(const uint8_t *const routeDescription);

/** WH_DymRoute_route
 * Enrute a message
 * @return the number of msgs sent
 */
uint8_t WH_DymRoute_route(const void *restrict const data, const MessageInfo *restrict const mi);

/** WH_DymRoute_send
 * Sends a message to the network
 * @return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_send(const void *const data, const MessageInfo *const mi, const DestinationWorm *const cn);

/** WH_DymRoute_invalidate
 * Invalidate the current routing system, and frees the necesary data
 */
void WH_DymRoute_invalidate();

/*
 =========================
*/
#ifdef __cplusplus
}
#endif
#endif
