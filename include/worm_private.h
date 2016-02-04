#ifndef __WORM_PRIV_H__
#define __WORM_PRIV_H__

#include <worm.h>
#include <common.h>

#ifdef __cplusplus
extern "C" {
#endif

	enum wormMsgType {
		HELLO, WORMINFO, SETUPWORMCONN
	};

	typedef struct {
		uint16_t Port;
		uint32_t IP; //TODO fix para ipv6
		int socket;
	} Worm2EinsConn;

	typedef struct {
		uint16_t port;
		uint32_t ip; //TODO fix para ipv6
		int socket;
		ConnectionDataType type;
		uint8_t *buffPosition;
		uint8_t *buff;
	} Connection;

	typedef struct {
		uint16_t id;
		size_t numberOfTypes;
		/*TODO: fix para multiples conexiones con un tipo por conexion*/
		ConnectionDataType *supportedTypes;
		Connection conns;
	} DestinationWorm;

	typedef struct {
		size_t numberOfWorms;
		DestinationWorm *worms;
	} DestinationWorms;


	/* Name WH_connectWorm
	 * Connect and fill the socket data.
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_connectWorm(DestinationWorm *c);

	/* Name WH_setupConnectionType
	 * Setup connection type
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_setupConnectionType(Connection *c, const ConnectionDataType *const type);

	/* Name WH_getWormData
	 * Gets worm data (IP+port).
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_getWormData(WormSetup *ws, const uint16_t wormId);

	/* Name WH_addWormConnection
	 * Return the created connection
	 */
	Connection *WH_addWormConnection(DestinationWorm *cns);

	/* Name WH_addWorm
	 * Return the created connection
	 */
	DestinationWorm *WH_addWorm(DestinationWorms *wms, const uint16_t wormId);

	/* Name WH_findWorm
	 * Return the worm mached (if no exists)
	 */
	DestinationWorm *WH_findWorm(DestinationWorms *wms, const uint16_t wormId);

	/* Name WH_findWormIndex
	 * Return the worm index in DestinationWorms
	 */
	size_t WH_findWormIndex(DestinationWorms *wms, const uint16_t wormId);

	/* Name WH_thread
	 * A worm Thread listening for info/petitions.
	 */
	void *WH_thread(void *arg);

	/*
	 Dynamic Routing Library
	*/
	/* DUP, CAT, RR, HASH */
	enum RoutingRule {DUPLICATE, CATEGORY, ROUNDROBIN, HASH};

	/* Name WH_DymRoute_init
	 * Starts the Dynamic Routing Library, and setups connection configuration.
	 * Also makes connections
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_init(const uint8_t *const routeDescription, DestinationWorms *cns);

	/* Name WH_DymRoute_route_create
	 * Starts the Dynamic Routing Library, and setups connection configuration.
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_route_create(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

	/* Name WH_DymRoute_route_createFunc
	 * Searchs for a Function, and calls the correct function.
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_route_createFunc(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

	/* Name WH_DymRoute_route_createFuncRR
	 * Adds a "c code" for round robin.
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_route_createFuncRR(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

	/* Name WH_DymRoute_route_createFuncCat
	 * Adds a "c code" for category splitting.
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_route_createFuncCat(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms);

	/* Name WH_DymRoute_route
	 * Enrute a message
	 * Return the number of msgs sent
	 */
	uint8_t WH_DymRoute_route(const void *const data, const MessageInfo *const mi, DestinationWorms *wms);

	/* Name WH_DymRoute_send
	 * Sends a message to the network
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_send(const void *const data, const MessageInfo *const mi, const DestinationWorm *const cn);


	/*
	 =========================
	*/
#ifdef __cplusplus
}
#endif
#endif
