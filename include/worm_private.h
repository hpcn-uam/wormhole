#ifndef __WORM_PRIV_H__
#define __WORM_PRIV_H__

#include <worm.h>

#ifdef __cplusplus
extern "C" {
#endif

	enum wormMsgType {
		HELLO, SSLSTART, WORMINFO, SETUPWORMCONN
	};

	typedef struct {
		uint16_t Port;
		uint32_t IP; //TODO fix para ipv6
		int socket;
	} Worm2EinsConn;

	typedef struct {
		AsyncSocket socket;
		ConnectionDataType type;
	} Connection;

	typedef struct {
		uint16_t id;
		uint16_t port;
		uint32_t ip; //TODO fix para ipv6
		size_t numberOfTypes;
		ConnectionDataType *supportedTypes;
		Connection **conns;
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

	/** WH_TH_checkMsgType
	 * check the message type
	 * @return 0 if ok, 1 if some error
	 */
	int WH_TH_checkMsgType(enum wormMsgType type, SyncSocket *socket);


	/** WH_TH_hellow
	 * Process a HELLOW message
	 */
	void WH_TH_hellow(SyncSocket *socket);

	/** SETUPWORMCONN
	 * Process a HELLOW message
	 */
	void WH_TH_setupworm(SyncSocket *socket);

	/** WH_connectionPoll
	 * Poll data from some socket
	 * @return some connection with data, NULL if error/timeout.
	 */
	Connection *WH_connectionPoll(DestinationWorms *wms);

	/** WH_typesize
	 * @return the size of the tipe provided
	 */
	size_t WH_typesize(const ConnectionDataType *const type);

	/*
	 Dynamic Routing Library
	*/
	/* DUP, CAT, RR, HASH */
	enum RoutingRule {DUPLICATE, CATEGORY, ROUNDROBIN, HASH};

	/** WH_DymRoute_init
	 * Starts the Dynamic Routing Library, and setups connection configuration.
	 * Also makes connections
	 * @return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_init(const uint8_t *const routeDescription, DestinationWorms *cns);

	/** WH_DymRoute_route_create
	 * Starts the Dynamic Routing Library, and setups connection configuration.
	 * @return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_route_create(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

	/** WH_DymRoute_route_createFunc
	 * Searchs for a Function, and calls the correct function.
	 * @return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_route_createFunc(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

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

	/** WH_DymRoute_route
	 * Enrute a message
	 * @return the number of msgs sent
	 */
	uint8_t WH_DymRoute_route(const void *const data, const MessageInfo *const mi);

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
