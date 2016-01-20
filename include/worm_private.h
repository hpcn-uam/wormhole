#ifndef __WORM_PRIV_H__
#define __WORM_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "worm.h"

	typedef struct
{
		uint16_t Port;
		uint32_t IP; //TODO fix para ipv6
		int socket;
	} Worm2EinsConn;

	typedef struct {
		uint16_t port;
		uint16_t id;
		uint32_t ip; //TODO fix para ipv6
		int socket;
		ConnectionDataType type;
		uint8_t *buffPosition;
		uint8_t *buff;
	} Connection;

	typedef struct {
		size_t numberConnections;
		Connection *conns;
	} Connections;



	/*
	 Dynamic Routing Library
	*/
	enum RoutingRule {DUPLICATE, CATEGORY, ROUNDROBIN, HASH};

	/* Name WH_DymRoute_init
	 * Starts the Dynamic Routing Library
	 * Return 0 if OK, something else if error.
	 */
	uint8_t WH_DymRoute_init (const uint8_t * const routeDescription, Connections** cns);

	/* Name WH_DymRoute_route
	 * Enrute a message
	 * Return output connection
	 */
	Connection * WH_DymRoute_route (const MessageInfo * const mi, Connections* const cns);

	void getDestinationInfo(const uint16_t id, uint32_t *ip, uint16_t *port);

	/*
	 =========================
	*/
#ifdef __cplusplus
}
#endif
#endif
