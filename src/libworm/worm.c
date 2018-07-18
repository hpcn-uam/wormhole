#define _GNU_SOURCE  // TODO find a better and standar way of set affinity
#include <libworm/worm_private.h>
#include <wh_config.h>

#include <sched.h>
//#undef _GNU_SOURCE

#include <netlib_inline.c>
/*
 *Global variables
 */
Worm2EinsConn WH_einsConn;
uint16_t WH_myId;
HoleSetup WH_mySetup;

DestinationHoles WH_myDstWorms = {0, 0};
DestinationHoles WH_myRcvWorms = {0, 0};

WormConfig WH_myConfig = {0, 0};

pthread_t WH_wormThread;

__thread enum wormErrorType WH_errno = 0;

volatile uint8_t WH_bussy   = 0;
volatile uint8_t WH_halting = 0;
extern uint32_t WH_load;
/*
 *===============
 */

/* Name WH_setup_types
 * Setups the available types of this Worm.
 * 	If this function is never called, only UINT8-Array would be supported.
 * Return 0 if OK, something else if error.
 */
uint8_t WH_setup_types(size_t nTypes, ConnectionDataType *types)
{
	ConnectionDataType *sTypes = malloc(sizeof(ConnectionDataType) * nTypes);

	if (!sTypes) {
		return 1;
	}

	memcpy(sTypes, types, sizeof(ConnectionDataType) * nTypes);

	if (WH_myConfig.inputTypes != NULL) {
		ConnectionDataType *tmp;
		tmp = WH_myConfig.inputTypes;

		if (WH_myConfig.numInputTypes > nTypes) {
			WH_myConfig.numInputTypes = nTypes;
		}

		WH_myConfig.inputTypes = sTypes;
		free(tmp);
	}

	WH_myConfig.inputTypes    = sTypes;
	WH_myConfig.numInputTypes = nTypes;

	return 0;
}

/* Name WH_init
 * Starts the WormHole Library
 * Return 0 if OK, something else if error.
 */
uint8_t WH_init(void)
{
	WH_myDstWorms.numberOfWorms = 0;
	WH_myDstWorms.worms         = malloc(sizeof(DestinationHole));

	WH_myRcvWorms.numberOfWorms = 0;
	WH_myRcvWorms.worms         = malloc(sizeof(DestinationHole));

	WH_myId = atoi(getenv("WORM_ID"));

	WH_einsConn.Port = atoi(getenv("ZEUS_PORT"));
	WH_einsConn.ip   = strdup(getenv("ZEUS_IP"));

	int ZeusSocket = tcp_connect_to(getenv("ZEUS_IP"), WH_einsConn.Port);

	if (ZeusSocket == -1) {
#ifdef LIBWORM_DEBUG
		perror("[WH]: Error connecting to Zeus");
#endif
		return 1;
	}

	WH_einsConn.socket = tcp_upgrade2syncSocket(ZeusSocket, NOSSL, NULL);

	if (WH_einsConn.socket == NULL) {
#ifdef LIBWORM_DEBUG
		perror("[WH]: Error connecting to Zeus");
#endif
		close(ZeusSocket);
		return 1;
	}

	if (hptl_init(NULL)) {
#ifdef LIBWORM_DEBUG
		perror("[WH]: Error Initializing hptl");
#endif
		tcp_sclose(WH_einsConn.socket);
		return 1;
	}

	// Fill hello message with worm id
	size_t hellomsgSize = sizeof(enum ctrlMsgType) + sizeof(uint16_t);
	uint8_t hellomsg[hellomsgSize];
	enum ctrlMsgType *msgType                            = (enum ctrlMsgType *)hellomsg;
	*msgType                                             = HELLOZEUS;
	*((uint16_t *)(hellomsg + sizeof(enum ctrlMsgType))) = htons(WH_myId);

	// Send hello message
	if (tcp_message_ssend(WH_einsConn.socket, hellomsg, hellomsgSize) != 0) {
#ifdef LIBWORM_DEBUG
		perror("[WH]: Error sending HELLO to Zeus");
#endif
		return 1;
	}

	// Receive HoleSetup

	// Check ctrl msg
	if (tcp_message_srecv(WH_einsConn.socket, msgType, sizeof(enum ctrlMsgType), 1) != sizeof(enum ctrlMsgType)) {
#ifdef LIBWORM_DEBUG
		perror("[WH]: Error recv myconfig.response from Zeus");
#endif
		return 1;
	}

#ifdef WH_SSL
	if (*msgType == STARTSSL) {
		if (syncSocketStartSSL(WH_einsConn.socket, CLISSL, NULL)) {
#ifdef LIBWORM_DEBUG
			perror("[WH]: Error stablishing SSL with Zeus");
#endif
			return 1;
		}

		if (tcp_message_srecv(WH_einsConn.socket, msgType, sizeof(enum ctrlMsgType), 1) != sizeof(enum ctrlMsgType)) {
#ifdef LIBWORM_DEBUG
			perror("[WH]: Error recv myconfig.response from Zeus");
#endif
			return 1;
		}
	}
#endif

	if (*msgType != SETUP) {
#ifdef LIBWORM_DEBUG
		perror("[WH]: Unexpected message from zeus");
#endif
		return 1;
	}

	uint8_t *HoleSetupMsg = (uint8_t *)&WH_mySetup;

	if (tcp_message_srecv(WH_einsConn.socket, HoleSetupMsg, sizeof(HoleSetup), 1) != sizeof(HoleSetup)) {
#ifdef LIBWORM_DEBUG
		perror("[WH]: Error recv myconfig from Zeus");
#endif
		return 1;
	}

	// Receive connectionDescription
	WH_mySetup.connectionDescription = calloc(WH_mySetup.connectionDescriptionLength + 1, sizeof(char));

	if (WH_mySetup.connectionDescriptionLength > 0) {
		if (tcp_message_srecv(
		        WH_einsConn.socket, WH_mySetup.connectionDescription, WH_mySetup.connectionDescriptionLength, 1) !=
		    WH_mySetup.connectionDescriptionLength) {
#ifdef LIBWORM_DEBUG
			perror("[WH]: Error recv connectionDescription from Zeus");
#endif
			return 1;
		}
	}

	// TypeSetup
	if (WH_myConfig.inputTypes == NULL) {
		ConnectionDataType tmpType = {.type = ARRAY, .ext.arrayType = UINT8};
		WH_setup_types(1, &tmpType);
	}

	// Establecer afinidad
	if (WH_mySetup.core != 0) {
		if (sched_setaffinity(0, sizeof(WH_mySetup.core), (cpu_set_t *)&WH_mySetup.core)) {
			perror("[WH]: Error setting up process affinity");
		}
	}

	// Lanzar hilo de recepción de conexiones
	WH_bussy = 1;

	if (pthread_create(&WH_wormThread, NULL, WH_thread, NULL)) {
#ifdef LIBWORM_DEBUG
		perror("[WH]: Error creating thread");
#endif
		return 1;
	}

	// Creamos el enrutado dinámico
	if (WH_DymRoute_init(WH_mySetup.connectionDescription, &WH_myDstWorms)) {
#ifdef LIBWORM_DEBUG
		fprintf(stderr, "[WH]: Error creating DymRoute\n");
#endif
		return 1;
	}

	WH_bussy = 0;

	struct timeval ts;
	ts.tv_sec  = 0;
	ts.tv_usec = 50000;

	if (setsockopt(WH_einsConn.socket->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&ts, sizeof(struct timeval)) < 0) {
		fputs("[TH] setsockopt failed [1]\n", stderr);
	}

	if (setsockopt(WH_einsConn.socket->sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&ts, sizeof(struct timeval)) < 0) {
		fputs("[TH] setsockopt failed [2]\n", stderr);
	}

	// fprintf(stdout, "[WH]: %s Online\n", wh_VERSION);
	return 0;
}

/** WH_halt
 * Stops and free the WormHole Library
 * @return 0 if OK, something else if error.
 */
uint8_t WH_halt(void)
{
	// TODO fix this function, in order to orderly shutdown
	enum ctrlMsgType type = HALT;

	if (tcp_message_ssend(WH_einsConn.socket, &type, sizeof(type))) {
		return 1;
	}

	WH_halting = 1;

	struct timespec ts;
	ts.tv_sec  = 1;
	ts.tv_nsec = 0;

	// close all output connections
	size_t numworms = WH_myDstWorms.numberOfWorms;

	for (size_t i = 0; i < numworms; i++) {
#ifdef LIBWORM_DEBUG
		fprintf(stderr, "[WH]: asking for free worm data...%lu %lu\n", WH_myDstWorms.numberOfWorms, i);
#endif

		WH_removeWorm(&WH_myDstWorms, WH_myDstWorms.worms[WH_myDstWorms.numberOfWorms - 1].id);
	}

	while (WH_halting) {
		nanosleep(&ts, 0);
	}

	return 0;
}

/* Name WH_thread
 * A worm Thread listening for info/petitions.
 */
void *WH_thread(void *arg)
{
	UNUSED(arg);

	int listeningSocket = tcp_listen_on_port(WH_mySetup.listenPort);
	struct timeval ts;
	ts.tv_sec = 0;
	// ts.tv_usec = 250000;
	ts.tv_usec = 250000;

	if (listeningSocket == -1) {
		perror("Error opening socket");
		exit(-1);
	}

	while (1) {
		// poll for incomming connections/requests.
		int tmpsock = tcp_accept(listeningSocket, &ts);  // TODO Optimizar para no reconfigurar constantemente el socket
		// int socket = tcp_accept(listeningSocket, NULL);

		if (tmpsock < 0) {
			enum ctrlMsgType type;

			// Lectura desde Zeus
			if (!WH_bussy) {
				ssize_t ret = tcp_message_srecv(WH_einsConn.socket, &type, sizeof(type), 0);

				if (ret != sizeof(type)) {
					if (ret == -1) {
						// TODO: connection lost with Zeus, Reconnect!!
						fputs("ZEUS connection lost...! Forced Shutdown (1)\n", stderr);
						exit(1);
					}

					continue;

				} else {
					if (WH_TH_checkCtrlMsgType(type, WH_einsConn.socket) > 0) {
						// TODO: connection lost with Zeus, Reconnect!!
						fputs("ZEUS connection lost...! Forced Shutdown (2)\n", stderr);
						exit(1);
					}
				}
			}

		} else {
			SyncSocket *socket = tcp_upgrade2syncSocket(tmpsock, NOSSL, NULL);
			enum wormMsgType type;
			int retNum = 0;

			while (tcp_message_srecv(socket, &type, sizeof(type), 1) == sizeof(type)) {
				retNum = WH_TH_checkMsgType(type, socket);

				if (retNum) {
					break;
				}
			}

			if (retNum != 1) {  // in case of stop without closing
				tcp_sclose(socket);
			}
		}
	}

	return NULL;
}

/** WH_TH_checkCtrlMsgType
 * check a control message type from Zeus
 * @return 0 if ok, -1 if error, and 1 if socket wont receive more control data.
 */
int WH_TH_checkCtrlMsgType(enum ctrlMsgType type, SyncSocket *socket)
{
	int ret = 0;
	enum ctrlMsgType response;

	switch (type) {
		case PING:
			response = PONG;

			if (tcp_message_ssend(socket, &response, sizeof(response))) {
				perror("[WH]: Pong error\n");
			}

			break;

		case HALT:
			fputs("[WH]: Halting Worm by ZEUS...\n", stderr);

			if (WH_halting) {
				WH_halting = 0;

			} else {
				exit(0);
			}

			break;

		case CHANGEROUTE: {
			uint32_t routesize;
			uint8_t *newroute = NULL;

			if (tcp_message_srecv(socket, &routesize, sizeof(routesize), 1) != sizeof(routesize)) {
				ret = -1;
			}

			if (!ret) {
				newroute = malloc(routesize);
			}

			if (newroute)
				if (tcp_message_srecv(socket, newroute, routesize, 1) != routesize) {
					ret = -1;
				}

			if (!ret) {
				fprintf(stderr, "[WH]: Setting up new Route: %s\n", newroute);
			}

			fflush(stderr);

			if (!ret) {
				ret = WH_DymRoute_init(newroute, &WH_myDstWorms);
			}

			if (ret) {
				type = CTRL_ERROR;
				ret  = -1;
				fprintf(stderr, "Faling changing route..!\n");

			} else {
				type = CTRL_OK;
				fprintf(stderr, "Route succesfully changed!\n");
			}

			tcp_message_ssend(socket, &type, sizeof(type));
			break;
		}

		case QUERYID: {
			enum queryType qtype;
			response = RESPONSEID;
			if (tcp_message_srecv(socket, &qtype, sizeof(qtype), 1) != sizeof(qtype)) {
				ret = -1;
				break;
			}

			tcp_message_ssend(socket, &response, sizeof(response));

			switch (qtype) {
#ifdef WH_STATISTICS

				case qSTATISTICS_IN: {
					tcp_message_ssend(socket, &qtype, sizeof(qtype));

					// Enviamos cuantos worms tenemos de entrada
					tcp_message_ssend(socket, &WH_myRcvWorms.numberOfWorms, sizeof(size_t));

					// Enviamos las estadisticas de cada uno de ellos
					for (size_t i = 0; i < WH_myRcvWorms.numberOfWorms; i++) {
						// estadisticas acumuladas
						ConnectionStatistics stats = {0};
						stats.holeId               = WH_myRcvWorms.worms[i].id;
						for (size_t j = 0; j < WH_myRcvWorms.worms[i].numberOfTypes; j++) {
							if (WH_myRcvWorms.worms[i].conns[j]) {
								stats.totalIO += WH_myRcvWorms.worms[i].conns[j]->stats.totalIO;
								stats.lastMinIO += WH_myRcvWorms.worms[i].conns[j]->stats.lastMinIO;
								if (stats.lastCheck < WH_myRcvWorms.worms[i].conns[j]->stats.lastCheck)
									stats.lastCheck = WH_myRcvWorms.worms[i].conns[j]->stats.lastCheck;
							}
						}
						tcp_message_ssend(socket, &stats, sizeof(stats));
					}
					break;
				}

				case qSTATISTICS_OUT: {
					tcp_message_ssend(socket, &qtype, sizeof(qtype));

					// Enviamos cuantos worms tenemos de entrada
					tcp_message_ssend(socket, &WH_myDstWorms.numberOfWorms, sizeof(size_t));

					// Enviamos las estadisticas de cada uno de ellos
					for (size_t i = 0; i < WH_myDstWorms.numberOfWorms; i++) {
						// estadisticas acumuladas
						ConnectionStatistics stats = {0};
						stats.holeId               = WH_myDstWorms.worms[i].id;
						for (size_t j = 0; j < WH_myDstWorms.worms[i].numberOfTypes; j++) {
							if (WH_myDstWorms.worms[i].conns[j]) {
								stats.totalIO += WH_myDstWorms.worms[i].conns[j]->stats.totalIO;
								stats.lastMinIO += WH_myDstWorms.worms[i].conns[j]->stats.lastMinIO;
								if (stats.lastCheck < WH_myDstWorms.worms[i].conns[j]->stats.lastCheck)
									stats.lastCheck = WH_myDstWorms.worms[i].conns[j]->stats.lastCheck;
							}
						}
						tcp_message_ssend(socket, &stats, sizeof(stats));
					}
					break;
				}
#endif
				default: {
					qtype = qUNSUPORTED;
					tcp_message_ssend(socket, &qtype, sizeof(qtype));

					ret = -1;
					break;
				}
			}
			break;
		}

		default:
			fprintf(stderr, "Unsupported Zeus Message (%d)!\n", (int)type);
			ret = 1;  // Error!
			break;
	}

	return ret;
}

/** WH_TH_checkMsgType
 * check the message type
 * @return 0 if ok, -1 if error, and 1 if socket wont receive more control data.
 */
int WH_TH_checkMsgType(enum wormMsgType type, SyncSocket *socket)
{
	int ret = 0;

	switch (type) {
		case HELLO:  // Contestamos a Hello
		default:
			WH_TH_hello(socket);  // TODO check if ret should be -1
			// return -1;
			break;

		case SETUPWORMCONN:  // Establecemos una conexión completa con otro worm
			WH_TH_setupworm(socket);
			return 1;

		case SSLSTART:
#ifdef LIBWORM_DEBUG
			fprintf(stderr, "[WH]: Starting SSL session\n");
#endif
#ifdef WH_SSL
			ret = syncSocketStartSSL(socket, SRVSSL, NULL);
#else
			WH_abort("Can't Handle SSL session. I'm not compiled for it!");
#endif
			break;
	}

	return ret;
}

/** WH_TH_hello
 * Process a HELLO message
 */
inline void WH_TH_hello(SyncSocket *socket)
{
	enum wormMsgType type;
	type = WORMINFO;  // Con Worm Info

	// fputs("Worm Info Pedida\n", stderr);

	if (tcp_message_ssend(socket, &type, sizeof(type))) {
		perror("Error contestando socket [1]\n");
		return;
	}

	if (tcp_message_ssend(socket, &WH_mySetup, sizeof(WH_mySetup))) {  // Con HoleSetup
		perror("Error contestando socket [2]\n");
		return;
	}

	if (tcp_message_ssend(socket, &WH_myConfig, sizeof(WH_myConfig))) {  // Con wormConfig
		perror("Error contestando socket [3]\n");
		return;
	}

	// fprintf(stderr, "Enviando config con size=%zu\n", WH_myConfig.numInputTypes);

	if (tcp_message_ssend(socket,
	                      WH_myConfig.inputTypes,
	                      sizeof(ConnectionDataType) * WH_myConfig.numInputTypes)) {  // Con wormConfig
		perror("Error contestando socket [4]");
		return;
	}

	// fputs("Worm Info Respondida\n", stderr);
}

/** SETUPWORMCONN
 * Process a HELLO message
 */
inline void WH_TH_setupworm(SyncSocket *socket)
{
	DestinationHole tmpDestWorm;

	// Recibimos un DestinationHole
	if (tcp_message_srecv(socket, &tmpDestWorm, sizeof(DestinationHole), 1) != sizeof(DestinationHole)) {
		fputs("Error configurando socket", stderr);
		tcp_sclose(socket);  // cerramos el socket
		return;
	}

	DestinationHole *tmpDestWormPtr = WH_addWorm(&WH_myRcvWorms, tmpDestWorm.id, 0);

	strncpy(tmpDestWormPtr->ip, tmpDestWorm.ip, INET6_ADDRSTRLEN);
	tmpDestWormPtr->port = tmpDestWorm.port;
	tmpDestWormPtr->supportedTypes =
	    realloc(tmpDestWormPtr->supportedTypes, (tmpDestWormPtr->numberOfTypes + 1) * sizeof(ConnectionDataType));

	// TODO fix para en caso de que se reduzca/reordene la lista, no se quede memoria perdida...
	if (tmpDestWormPtr->conns)
		tmpDestWormPtr->conns = realloc(tmpDestWormPtr->conns,
		                                (tmpDestWormPtr->numberOfTypes + 1) * sizeof(Connection *));  // TODO test ==NULL

	else {
		tmpDestWormPtr->conns = calloc(sizeof(Connection *), tmpDestWormPtr->numberOfTypes + 1);
	}

	tmpDestWormPtr->conns[tmpDestWormPtr->numberOfTypes] = calloc(sizeof(Connection), 1);  // TODO test ==NULL

	if (tcp_message_srecv(socket, tmpDestWormPtr->conns[tmpDestWormPtr->numberOfTypes], sizeof(Connection), 1) !=
	    sizeof(Connection)) {
		fputs("Error configurando socket", stderr);
		tcp_sclose(socket);  // cerramos el socket
		return;
	}

	socket_sync_to_async_recv(&(tmpDestWormPtr->conns[tmpDestWormPtr->numberOfTypes]->socket), socket);

	tmpDestWormPtr->numberOfTypes++;
#ifdef LIBWORM_DEBUG
	fprintf(stderr, "[WH]: Input Connection: %d\n", tmpDestWormPtr->id);
#endif
}

/* Name WH_connectionPoll
 * Poll data from some socket
 * Return some connection with data, NULL if error/timeout.
 */
Connection *WH_connectionPoll(DestinationHoles *wms, MessageInfo *mi)
{
	static uint32_t wormIndex = 0;
	static uint32_t connIndex = 0;

	uint32_t closedWorms   = 0;
	uint32_t unclosedWorms = 0;

	uint32_t startingWormIndex = wormIndex;
	uint32_t startingConnIndex = connIndex;

WH_connectionPoll_loop:

	for (; wormIndex < wms->numberOfWorms; wormIndex++, connIndex = 0) {
		for (; connIndex < wms->worms[wormIndex].numberOfTypes; connIndex++) {
			Connection *conn;
			conn = wms->worms[wormIndex].conns[connIndex];

			if (WH_considerSocket(&(conn->socket), mi)) {
				connIndex++;
				WH_errno = WH_ERRNO_CLEAR;
				return conn;

			} else if (conn->socket.closed) {
				closedWorms++;

			} else {
				unclosedWorms++;
			}
		}
	}

	if (startingWormIndex == 0 && startingConnIndex == 0) {  // begin from the start
		if (!unclosedWorms) {                                // there is no open worms
			if (!closedWorms) {                              // there is no close worms
				WH_errno = WH_ERRNO_EMPTY;
				return NULL;

			} else {
				WH_errno = WH_ERRNO_CLOSED;
				return NULL;
			}

		} else {
			wormIndex = 0;               // RST
			connIndex = 0;               // RST
			WH_errno  = WH_ERRNO_CLEAR;  // TODO check when this call happens
			return NULL;
		}

	} else {
		wormIndex         = 0;  // RST
		connIndex         = 0;  // RST
		startingWormIndex = 0;
		startingConnIndex = 0;
		goto WH_connectionPoll_loop;
		// return WH_connectionPoll(wms, mi);
	}
}

/** WH_considerSocket
 * Check if the socket would complete the request
 * @return 1 if yes, 0 if no
 */
int WH_considerSocket(AsyncSocket *sock, MessageInfo *mi)
{
	if (tcp_async_numbuf(sock) == 2) {
		return 1;
	}

	uint32_t availableBytes = tcp_async_availableBytes(sock);

	if (availableBytes < 8) {
		return 0;
	}

	if (mi->type && mi->type->type == ARRAY && availableBytes < WH_typesize(mi->type) * tcp_async_peakInt64(sock)) {
		return 0;
	}

	return 1;
}

/* Name WH_connectWorm
 * Connect and fill the socket data.
 * Return 0 if OK, something else if error.
 */
uint8_t WH_connectWorm(DestinationHole *c)
{
	int socket = 0;

	// fprintf(stderr, "[DEBUG:%d ; %s:%d]\n", __LINE__, inet_ntoa(ip_addr), c->port);

#ifdef LIBWORM_DEBUG
	int retryflag = 0;
#endif
	do {
		// Force keep-trying
		socket = tcp_connect_to(c->ip, c->port);
		hptl_waitns(100000);

#ifdef LIBWORM_DEBUG
		if (socket < 0 && retryflag != errno) {
			fprintf(stderr, "[WH]: Worm connection to %s:%d failed: ", c->ip, c->port);
			perror("");
			fprintf(stderr, "[WH]: Keep trying... ");
			fflush(stderr);
			retryflag = errno;
		}
#endif
	} while (socket < 0);

#ifdef LIBWORM_DEBUG
	if (retryflag) {
		fprintf(stderr, "CONNECTED\n");
		fflush(stderr);
	}
#endif

	// Solicitamos datos...
	enum wormMsgType type = HELLO;  // Con Worm Info

	HoleSetup HoleSetup;
	WormConfig wormConfig = {0, 0};

	if (tcp_message_send(socket, &type, sizeof(type))) {
		perror("Error solicitando información del Worm [1.s]");
		close(socket);
		return 1;
	}

	if (tcp_message_recv(socket, &type, sizeof(type), 1) != sizeof(type)) {
		perror("Error solicitando información del Worm [1.r]");
		close(socket);
		return WH_connectWorm(c);  // retry?
		                           // return 1;
	}

	if (type != WORMINFO) {
		fprintf(stderr, "Respuesta de worm no esperada...\n");
		close(socket);
		return 1;
	}

	if (tcp_message_recv(socket, &HoleSetup, sizeof(HoleSetup), 1) != sizeof(HoleSetup)) {  // Con HoleSetup
		perror("Error solicitando información del Worm [2]");
		close(socket);
		return 1;
	}

	if (tcp_message_recv(socket, &wormConfig, sizeof(wormConfig), 1) != sizeof(wormConfig)) {  // Con wormConfig
		perror("Error solicitando información del Worm [3]");
		close(socket);
		return 1;
	}

	wormConfig.inputTypes = realloc(c->supportedTypes, sizeof(ConnectionDataType) * wormConfig.numInputTypes);

	if (tcp_message_recv(socket,
	                     wormConfig.inputTypes,
	                     sizeof(ConnectionDataType) * wormConfig.numInputTypes,
	                     1) != (ssize_t)(sizeof(ConnectionDataType) * wormConfig.numInputTypes)) {  // Con wormConfig
		perror("Error solicitando información del Worm [4]");
		close(socket);
		return 1;
	}

	close(socket);

	// Rellenamos el worm entrante
	c->id             = HoleSetup.id;
	c->numberOfTypes  = wormConfig.numInputTypes;
	c->supportedTypes = wormConfig.inputTypes;

	// TODO en caso de que se reduzca el numero de conexiones problemas
	if (c->conns) {
		c->conns = realloc(c->conns, sizeof(Connection *) * wormConfig.numInputTypes);

	} else {
		c->conns = calloc(sizeof(Connection *), wormConfig.numInputTypes);
	}

	return 0;
}

/* Name WH_setupConnectionType
 * Setup connection type
 * Return 0 if OK, something else if error.
 */
uint8_t WH_setupConnectionType(DestinationHole *dw, const ConnectionDataType *const type)
{
	WH_connectWorm(dw);

	int tmpsock = tcp_connect_to(dw->ip, dw->port);

	if (tmpsock < 0) {
		return 1;
	}

	SyncSocket *socket = tcp_upgrade2syncSocket(tmpsock, NOSSL, NULL);

// setup SSL
#ifdef WH_SSL
	if (WH_mySetup.isSSLNode) {
		enum wormMsgType msgtype = SSLSTART;  // Con Worm config

#ifdef LIBWORM_DEBUG
		fprintf(stderr, "[WH]: Sending SSLSTART to worm\n");
#endif

		if (tcp_message_ssend(socket, &msgtype, sizeof(msgtype))) {
			fputs("Error configurando Worm externo [SSL]", stderr);
			return 1;
		}

		if (syncSocketStartSSL(socket, CLISSL, NULL)) {
			fputs("Error configurando Worm externo [SSL 2]", stderr);
			return 1;
		}
	}
#endif

#ifdef LIBWORM_DEBUG
	fprintf(stderr, "[WH]: Connection established with worm.id=%d\n", dw->id);
#endif

	int8_t flag = 0;

	for (uint32_t i = 0; i < dw->numberOfTypes; i++) {
		if (!WH_connectionDataTypecmp(dw->supportedTypes + i, type)) {
			flag = 1;
			break;
		}
	}

	if (!flag) {
#ifdef LIBWORM_DEBUG
		fprintf(stderr, "No se puede abrir un socket con el datatype solicitado!\n");
#endif
		tcp_sclose(socket);
		return 1;
	}

	// Informamos del tipo de conexion...
	enum wormMsgType msgtype = SETUPWORMCONN;  // Con Worm config

	if (tcp_message_ssend(socket, &msgtype, sizeof(msgtype))) {
		fputs("Error configurando Worm externo", stderr);
		return 1;
	}

	DestinationHole dwtmp;
	bzero(&dwtmp, sizeof(dwtmp));
	dwtmp.id = WH_mySetup.id;

	if (WH_mySetup.isIPv6) {
		strdup(inet_ntop(AF_INET6, &WH_mySetup.IP, dwtmp.ip, INET6_ADDRSTRLEN));

	} else {
		strdup(inet_ntop(AF_INET, &WH_mySetup.IP, dwtmp.ip, INET_ADDRSTRLEN));
	}

	dwtmp.port          = WH_mySetup.listenPort;
	dwtmp.numberOfTypes = 1;

	if (tcp_message_ssend(socket, &dwtmp, sizeof(dwtmp))) {  // DstWorm
		fprintf(stderr, "Error configurando Worm externo %d\n", dw->id);
		return 1;
	}

	Connection conntmp = {.type = *type,
//.socket = {0}, //error in some compilers (Clang)
#ifdef WH_STATISTICS
	                      .stats = {.totalIO = 0, .lastMinIO = 0, .lastMinIO_tmp = 0, .lastCheck = hptl_fget()}
#endif
	};
	bzero(&conntmp.socket, sizeof(conntmp.socket));  // replaces socket={0}

	if (tcp_message_ssend(socket, &conntmp, sizeof(conntmp))) {  // DstWorm
		fprintf(stderr, "Error configurando Worm externo %d\n", dw->id);
		return 1;
	}

	flag = 0;

	for (uint32_t i = 0; i < dw->numberOfTypes; i++) {
		if (!WH_connectionDataTypecmp(dw->supportedTypes + i, type)) {
			dw->conns[i] = calloc(sizeof(Connection), 1);
			socket_sync_to_async_send(&(dw->conns[i]->socket), socket);
			dw->conns[i]->type = *type;
			flag               = 1;
			break;
		}
	}

	if (!flag) {
		tcp_sclose(socket);
		return 1;
	}

#ifdef LIBWORM_DEBUG
	fprintf(stderr, "[WH]: Output Connection: %d\n", dw->id);
#endif

	return 0;
}

uint8_t WH_getWormData(HoleSetup *ws, const uint16_t holeId)
{
	enum ctrlMsgType ctrlMsg = QUERYID;

	if (tcp_message_ssend(WH_einsConn.socket, (uint8_t *)&ctrlMsg, sizeof(enum ctrlMsgType)) != 0) {
		return 1;
	}

	if (tcp_message_ssend(WH_einsConn.socket, (void *)&holeId, sizeof(uint16_t)) != 0) {
		return 1;
	}

	if (tcp_message_srecv(WH_einsConn.socket, (uint8_t *)&ctrlMsg, sizeof(enum ctrlMsgType), 1) !=
	    sizeof(enum ctrlMsgType)) {
		return 1;
	}

	if (ctrlMsg != CTRL_OK) {
		return 1;
	}

	if (tcp_message_srecv(WH_einsConn.socket, (uint8_t *)ws, sizeof(HoleSetup), 1) != sizeof(HoleSetup)) {
		return 1;
	}

	return 0;
}

/*Utils*/

/** WH_get_id
 * @return the WORM-ID.
 */
uint16_t WH_get_id(void)
{
	return WH_myId;
}

/** WH_typesize
 * @return the size of the tipe provided
 */
size_t WH_typesize(const ConnectionDataType *const type)
{
	// TODO optimize converting to static array
	switch (type->type) {
		case INT8:
		case UINT8:
			return 1;

		case INT16:
		case UINT16:
			return 2;

		case INT32:
		case UINT32:
			return 4;

		case INT64:
		case UINT64:
			return 8;

		case STRING:
			return 1;

		case ARRAY:
			switch (type->ext.arrayType) {
				case INT8:
				case UINT8:
					return 1;

				case INT16:
				case UINT16:
					return 2;

				case INT32:
				case UINT32:
					return 4;

				case INT64:
				case UINT64:
					return 8;

				case STRING:
					return 1;

				default:
					fprintf(stderr, "NOT YET IMPLEMENTED (%d)\n", __LINE__);
					exit(1);
			}

		default:
			fprintf(stderr, "NOT YET IMPLEMENTED (%d)\n", __LINE__);
			exit(1);
	}
}

/** WH_connectionDataTypecmp
 * Compares 2 datatypes
 * @return 0 if are equal, something else if not.
 */
uint8_t WH_connectionDataTypecmp(const ConnectionDataType *const a, const ConnectionDataType *const b)
{
	if (a->type == b->type) {
		if (a->type == ARRAY) {
			if (a->ext.arrayType == b->ext.arrayType) {
				return 0;

			} else {
				return 1;
			}

		} else {
			return 0;
		}

	} else {
		return 1;
	}
}

/************************************************************
    Bulking Routines
*************************************************************/

/** WH_send_blk
 * Receives multiples messages.
 * @return the number of bytes readed, 0 if ERROR or none.
 */
uint8_t WH_send_blk(const BulkList *const bl)
{
	size_t i;
	uint8_t ret = 0;

	for (i = 0; i < WH_myDstWorms.numberOfWorms; i++) {
		size_t j;

		for (j = 0; j < WH_myDstWorms.worms[i].numberOfTypes; j++) {
			if (WH_myDstWorms.worms[i].conns[j] != NULL) {
				tcp_async_startTransaction(&(WH_myDstWorms.worms[i].conns[j]->socket));
			}
		}
	}

	if (bl->cpyalign) {
		for (i = 0; i < bl->len; i += ((MessageInfo *)(bl->list.rawdata + i))->size + sizeof(MessageInfo)) {
			ret += WH_send(bl->list.rawdata + i + sizeof(MessageInfo), bl->list.rawdata + i);
		}

	} else {
		for (i = 0; i < bl->len; i++) {
			ret += WH_send(bl->list.msgs[i].data, bl->list.msgs[i].info);
		}
	}

	for (i = 0; i < WH_myDstWorms.numberOfWorms; i++) {
		size_t j;

		for (j = 0; j < WH_myDstWorms.worms[i].numberOfTypes; j++) {
			if (WH_myDstWorms.worms[i].conns[j] != NULL) {
				tcp_async_stopTransaction(&(WH_myDstWorms.worms[i].conns[j]->socket));
			}
		}
	}

	return ret;
}

/** WH_BL_create
 * @param cpyalign if set to other than 0, then each data added to the list would be copied and
 * memory aligned
 * @return a new BulkList. Null if error.
 */
BulkList *WH_BL_create(int_fast8_t cpyalign)
{
	BulkList *ret = calloc(sizeof(BulkList), 1);

	if (ret) {  // if not null
		ret->cpyalign = cpyalign;
	}

	return ret;
}

/** WH_BL_add
 * Adds a message info and msg to the list.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_BL_add(BulkList *bl, const void *const msg, const MessageInfo *const mi)
{
	uint8_t ret = 0;

	if (bl->cpyalign) {
		size_t oldlen = bl->len;
		bl->len += mi->size + sizeof(MessageInfo);

		bl->list.rawdata = realloc(bl->list.rawdata, bl->len);

		if (bl->list.rawdata == NULL) {
			fprintf(stderr, "[WH]: Can't realloc to %lu bytes", bl->len);
			return 1;
		}

		memcpy(bl->list.rawdata + oldlen, mi, sizeof(MessageInfo));
		memcpy(bl->list.rawdata + oldlen + sizeof(MessageInfo), msg, mi->size);

	} else {
		bl->list.msgs = realloc(bl->list.msgs, bl->len * sizeof(BulkMsg *));

		if (bl->list.msgs) {
			return 1;
		}

		BulkMsg *smsg = &(bl->list.msgs[bl->len]);
		smsg->data    = (void *)msg;
		smsg->info    = (MessageInfo *)mi;

		bl->len++;
	}

	return ret;
}

/** WH_BL_free
 * Frees the list
 */
void WH_BL_free(BulkList *bl)
{
	if (bl) {
		if (bl->len > 0 && (bl->list.rawdata || bl->list.msgs)) {
			if (bl->cpyalign) {
				free(bl->list.rawdata);

			} else {
				free(bl->list.msgs);
			}
		}

		free(bl);
	}
}

/************************************************************
    Dynamic Routing Library
*************************************************************/
/***************************************/
extern uint8_t _binary_structures_h_start;
extern uint8_t _binary_structures_h_end;

const char *_WH_DymRoute_CC_includes =
    "\n"
    "#include <stdint.h>\n"
    "#include <stdio.h>\n"
    "#include <pthread.h>\n"
    "#include <arpa/inet.h>\n"     // TODO remove someday
    "#include <openssl/ssl.h>\n";  // TODO remove include
const char *_WH_DymRoute_CC_FuncStart =
    "\n\n"
    "uint8_t WH_DymRoute_precompiled_route (const void *restrict const data,"
    " const MessageInfo *restrict const mi,"
    " const DestinationHoles *restrict const cns)\n{\n"
    "int ret = 0;\n"
    "DestinationHole *dw;\n";
const char *_WH_DymRoute_CC_FuncEnd =
    "\n"
    "return ret;"
    "}\n";
const char *_WH_DymRoute_CC_send  = "ret += WH_DymRoute_send(data, mi, dw);\n";
const char *_WH_DymRoute_CC_setDw = "dw = cns->worms+%d;\n";

// RR special
// const char *_WH_DymRoute_CC_RRstatic    =    "static uint16_t rr%d=0;\n";
const char *_WH_DymRoute_CC_RRswitch =
    "static uint16_t rr%d=0;\n"
    "switch(rr%d){\n";
const char *_WH_DymRoute_CC_RRcase  = "case %d :\n{ ";
const char *_WH_DymRoute_CC_RRbreak = "break;\n} ";
const char *_WH_DymRoute_CC_RRend =
    "default:\n"
    "ret++;"
    "}\n"
    "rr%d=(rr%d+1) %% %d;\n";

// Cat special
const char *_WH_DymRoute_CC_Catswitch = "switch(mi->category){\n";
const char *_WH_DymRoute_CC_Catcase   = "case %d :\n{ ";
const char *_WH_DymRoute_CC_Catbreak  = "break;\n} ";
const char *_WH_DymRoute_CC_Catend =
    "default:\n"
    "ret++;"
    "}\n";

// Hash special
const char *_WH_DymRoute_CC_Hashswitch = "switch(((mi->hash)\%(%u))){\n";
const char *_WH_DymRoute_CC_Hashcase   = "case %d :\n{ ";
const char *_WH_DymRoute_CC_Hashbreak  = "break;\n} ";
const char *_WH_DymRoute_CC_Hashend =
    "default:\n"
    "ret++;"
    "}\n";

// Ignore special
const char *_WH_DymRoute_CC_Ignoreunused = "(void)dw;\n";

void *_WH_DymRoute_libHandle = NULL;
/***************************************/

/* Name WH_DymRoute_precompiled_route
 * Enrute a message
 * Return the number of msgs sent
 */
uint8_t (*WH_DymRoute_precompiled_route)(const void *restrict const data,
                                         const MessageInfo *restrict const mi,
                                         const DestinationHoles *restrict const cns) = 0;

/* Name WH_send
 * TODO
 * Params:
 * Return 0 if OK, something else if error.
 */
uint8_t WH_send(const void *restrict const data, const MessageInfo *restrict const mi)
{
	return WH_DymRoute_route(data, mi);
}

/** WH_recv
 * TODO
 * Params:
 * @return the number of bytes readed, 0 if ERROR or none.
 */
uint32_t WH_recv(void *restrict data, MessageInfo *restrict mi)
{
// int pollCnt = 0;
#ifdef LIBWORM_ROUTE_DEBUG
	fprintf(stderr, "ROUTEDEBUG: Polling...\n");
#endif
	Connection *c = NULL;

	do {
		c = WH_connectionPoll(&WH_myRcvWorms, mi);
	} while ((!c && (WH_errno == WH_ERRNO_CLEAR || WH_errno == WH_ERRNO_EMPTY)) ||
	         ((c && mi->type) ? WH_connectionDataTypecmp(&c->type, mi->type) : 0));

	if (c == NULL) {  // no msg found
		WH_errno = WH_ERRNO_CLEAR;
		return 0;
	}

#ifdef LIBWORM_ROUTE_DEBUG
	fprintf(stderr, "ROUTEDEBUG: Msg found!...\n");
#endif

	if (!mi->type) {
		mi->type = &(c->type);
	}

	uint64_t tmp;
	uint32_t ret = 0;

	switch (c->type.type) {
		case INT8:
		case UINT8:
			if (!tcp_message_recv_async(&(c->socket), data, 1 * mi->size)) {
				ret = 1 * mi->size;
			}

			break;

		case INT16:
		case UINT16:
			if (!tcp_message_recv_async(&(c->socket), data, 2 * mi->size)) {
				ret = 2 * mi->size;
			}

			break;

		case INT32:
		case UINT32:
			if (!tcp_message_recv_async(&(c->socket), data, 4 * mi->size)) {
				ret = 4 * mi->size;
			}

			break;

		case INT64:
		case UINT64:
			if (!tcp_message_recv_async(&(c->socket), data, 8 * mi->size)) {
				ret = 8 * mi->size;
			}

			break;

		case STRING:
		case ARRAY: {
			if (mi->size < tcp_async_peakInt64(&(c->socket))) {  // TODO: it wont work everytime
				errno = EMSGSIZE;
				return 0;
			}

			if (!tcp_message_recv_async(&(c->socket), &tmp, sizeof(tmp))) {
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Array of %lu elements\n", tmp);
#endif
				switch (c->type.ext.arrayType) {
					case INT8:
					case UINT8:
						if (!tcp_message_recv_async(&(c->socket), data, 1 * tmp)) {
							ret = 1 * tmp;
						}

						break;

					case INT16:
					case UINT16:
						if (!tcp_message_recv_async(&(c->socket), data, 2 * tmp)) {
							ret = 2 * tmp;
						}

						break;

					case INT32:
					case UINT32:
						if (!tcp_message_recv_async(&(c->socket), data, 4 * tmp)) {
							ret = 4 * tmp;
						}

						break;

					case INT64:
					case UINT64:
						if (!tcp_message_recv_async(&(c->socket), data, 8 * tmp)) {
							ret = 8 * tmp;
						}

						break;

					default:
						fprintf(stderr, "ARRAYTYPE: NOT YET IMPLEMENTED [%d]\n", c->type.ext.arrayType);  // TODO implement
						break;
				}

			} else {
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Error recvArray\n");
#endif
			}

			break;
		}

		default:
			fprintf(stderr, "NOT YET IMPLEMENTED [%d]\n", c->type.type);  // TODO implement
			break;
	}

#ifdef LIBWORM_ROUTE_DEBUG
	fprintf(stderr, "ROUTEDEBUG: Recv %dB\n", ret);
#endif
#ifdef WH_STATISTICS
	if ((c->stats.lastCheck + (60 * 1000000000ul)) < hptl_fget()) {
		c->stats.lastCheck     = hptl_fget();
		c->stats.lastMinIO     = c->stats.lastMinIO_tmp;
		c->stats.lastMinIO_tmp = ret;
	} else {
		c->stats.lastMinIO_tmp += ret;
	}
	c->stats.totalIO += ret;
#endif
	return ret;
}

/* Name WH_DymRoute_route
 * Enrute a message
 * Return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route(const void *restrict const data, const MessageInfo *restrict const mi)
{
	if (WH_DymRoute_precompiled_route == 0) {
		return 1;

	} else {
		return WH_DymRoute_precompiled_route(data, mi, &WH_myDstWorms);
	}
}

/* Name WH_DymRoute_init
 * Starts the Dynamic Routing Library, and setups connection configuration.
 * Also makes connections
 * Return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_init(const uint8_t *const routeDescription, DestinationHoles *wms)
{
	pid_t myPid     = getpid();
	char *cString   = malloc(1024);
	char *soString  = malloc(1024);
	char *gccString = malloc(1024);
	char *errorString;

	if (!cString || !soString || !gccString) {
#ifdef LIBWORM_ROUTE_DEBUG
		perror("ROUTEDEBUG: Error in Malloc");
#endif
		return 1;
	}

	sprintf(cString, "/tmp/%dT%lu.c", myPid, hptl_get());
	sprintf(soString, "/tmp/%dT%lu.so", myPid, hptl_get());
	FILE *f = fopen(cString, "w+");

	if (!f) {
		free(cString);
		free(soString);
		free(gccString);
#ifdef LIBWORM_ROUTE_DEBUG
		perror("ROUTEDEBUG: Error creating tmp.c");
#endif
		return 2;
	}

	/*Write headers to the file*/
	fwrite(_WH_DymRoute_CC_includes, strlen(_WH_DymRoute_CC_includes), 1, f);
	fwrite(&_binary_structures_h_start, &_binary_structures_h_end - &_binary_structures_h_start, 1, f);
	fwrite(_WH_DymRoute_CC_FuncStart, strlen(_WH_DymRoute_CC_FuncStart), 1, f);

	int ret = WH_DymRoute_route_create(f, routeDescription, wms);
	fwrite(_WH_DymRoute_CC_FuncEnd, strlen(_WH_DymRoute_CC_FuncEnd), 1, f);
	fclose(f);

	/*Compile the .c*/
	if (!ret) {
		sprintf(gccString, "gcc -O3 -Wall %s -o %s -shared -Llib -lworm -lpthread -fPIC -pipe ", cString, soString);

#ifdef LIBWORM_ROUTE_DEBUG
		fprintf(stderr, "ROUTEDEBUG: Calling %s\n", gccString);
#endif

		ret = system(gccString);

#ifdef LIBWORM_ROUTE_DEBUG

		if (ret) {
			fprintf(stderr, "ROUTEDEBUG: Error calling gcc\n");
		}

#endif
	}

	/*Link the .SO*/
	if (!ret) {
		void *tmplibHandle = dlopen(soString, RTLD_NOW);

		if (!tmplibHandle) {
#ifdef LIBWORM_ROUTE_DEBUG
			fprintf(stderr, "ROUTEDEBUG: %s\n", dlerror());
#endif
			free(cString);
			free(soString);
			free(gccString);
			return 5;
		}

		WH_DymRoute_precompiled_route = dlsym(tmplibHandle, "WH_DymRoute_precompiled_route");

		if ((errorString = dlerror()) != NULL || WH_DymRoute_precompiled_route == NULL) {
			fputs(errorString, stderr);
			ret = 6;
		}

		if (_WH_DymRoute_libHandle) {
			void *libtofree = _WH_DymRoute_libHandle;

			_WH_DymRoute_libHandle = tmplibHandle;

			hptl_waitns(50 * 1000 * 1000L);
			dlclose(libtofree);

		} else {
			_WH_DymRoute_libHandle = tmplibHandle;
		}
	}

	free(cString);
	free(soString);
	free(gccString);
	return ret;
}

/* Name WH_DymRoute_send
 * Sends a message to the network
 * Return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_send(const void *const data, const MessageInfo *const mi, DestinationHole *const dw)
{
	// TODO search info
	// WH_setupConnectionType
	for (uint32_t i = 0; i < dw->numberOfTypes; i++) {
		if (!WH_connectionDataTypecmp(dw->supportedTypes + i, mi->type)) {
			// tcp_message_send_async(AsyncSocket *sock, const void *message, size_t len)
			if (!dw->conns[i]) {
				if (WH_setupConnectionType((DestinationHole *)dw, mi->type)) {
#ifdef LIBWORM_ROUTE_DEBUG
					fprintf(stderr, "ROUTEDEBUG: sending data to worm: %d [FAIL-SETUP]\n", dw->id);
#endif
					return 1;
				}

			} else if (unlikely(dw->conns[i]->socket.closed)) {  // check if the socket has been closed
				destroy_asyncSocket(&(dw->conns[i]->socket));
				free(dw->conns[i]);
				dw->conns[i] = NULL;
				int j;
				for (j = i + 1; i < dw->numberOfTypes; j++) {
					dw->conns[j - 1]          = dw->conns[j];
					dw->supportedTypes[j - 1] = dw->supportedTypes[j];
				}

				dw->numberOfTypes--;
				continue;
			}
#ifdef WH_STATISTICS
			if ((dw->conns[i]->stats.lastCheck + (60 * 1000000000ul)) < hptl_fget()) {
				dw->conns[i]->stats.lastCheck     = hptl_fget();
				dw->conns[i]->stats.lastMinIO     = dw->conns[i]->stats.lastMinIO_tmp;
				dw->conns[i]->stats.lastMinIO_tmp = 0;
			}
#endif

			if (mi->type->type == ARRAY) {
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "[ARRAY %luB] ", mi->size);
#endif
				tcp_message_send_async(&(dw->conns[i]->socket), &(mi->size), sizeof(mi->size));
			}

#ifdef LIBWORM_ROUTE_DEBUG
			fprintf(stderr, "ROUTEDEBUG: sending data (%luB) to worm: %d\n", mi->size, dw->id);
#endif
#ifdef WH_STATISTICS
			dw->conns[i]->stats.lastMinIO_tmp += mi->size * WH_typesize(mi->type);
			dw->conns[i]->stats.totalIO += mi->size * WH_typesize(mi->type);
#endif
			return tcp_message_send_async(&(dw->conns[i]->socket), data, mi->size * WH_typesize(mi->type));
		}
	}

#ifdef LIBWORM_ROUTE_DEBUG
	fprintf(stderr, "ROUTEDEBUG: sending data to worm: %d [FAIL]\n", dw->id);
#endif

	if (!WH_setupConnectionType((DestinationHole *)dw, mi->type)) {  // Refresh worm data
		return WH_DymRoute_send(data, mi, dw);                       // Try again if it is possible
	}

	// fprintf(stderr, "Sending %d bytes to %d\n", mi->size, cn->ip);
	return 1;
}

/* Name WH_DymRoute_route_createFunc
 * Searchs for a Function, and calls the correct one.
 * Return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_create(FILE *f, const uint8_t *const routeDescription, DestinationHoles *wms)
{
	uint16_t i = 0;

	while (routeDescription[i] != '\0') {
		switch (routeDescription[i]) {  // default function is DUP
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case 'd':
			case 'D':
// DUP Function
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Calling DUP...\n");
#endif
				return WH_DymRoute_route_createFuncDUP(f, routeDescription + i, wms);

			case 'r':
			case 'R':
// Round Robin Function
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Calling RR...\n");
#endif
				return WH_DymRoute_route_createFuncRR(f, routeDescription + i, wms);

			case 'c':
			case 'C':
// Category Function
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Calling CAT...\n");
#endif
				return WH_DymRoute_route_createFuncCat(f, routeDescription + i, wms);

			case 'h':
			case 'H':
// Hash Function
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Calling HASH...\n");
#endif
				return WH_DymRoute_route_createFuncHash(f, routeDescription + i, wms);

			case 'i':
			case 'I':
// Ignore Function
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Calling IGNORE...\n");
#endif
				return WH_DymRoute_route_createFuncIgnore(f, routeDescription + i, wms);

			case ' ':
			case '\n':
			case '\t':
			case '\r':
			case '(':
			case ')':
			case '"':
			case '\'':
				i++;
				break;

			default:
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Unexpected routing function.");
#endif
				return -1;
		}
	}

	return 91;
}

/* Name WH_DymRoute_route_createFuncDUP
 * Adds a "c code" to duplicate messages.
 * Return 0 if OK, something else if error.
 * Used Constants:
    //TODO
 */
uint8_t WH_DymRoute_route_createFuncDUP(FILE *f, const uint8_t *const routeDescription, DestinationHoles *wms)
{
	uint8_t ret        = 0;
	int16_t parentesys = 0;
	uint16_t i         = 0;
	uint16_t nextNode  = 0;

	while (routeDescription[i] != '\0') {
		if (routeDescription[i] == ')') {
			parentesys--;

			if (parentesys < 0) {
				break;
			}

		} else if (routeDescription[i] == '(') {
			parentesys++;

			if (parentesys == 1) {
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Function Evaluation: |%s|\n", routeDescription + i);
#endif
				ret += WH_DymRoute_route_create(f, routeDescription + i, wms);
			}

		} else if (parentesys == 0 && (routeDescription[i] >= '0' && routeDescription[i] <= '9')) {
			nextNode = atoi((char *)(routeDescription + i));

			while (routeDescription[i] >= '0' && routeDescription[i] <= '9') {
				i++;
			}

#ifdef LIBWORM_ROUTE_DEBUG
			fprintf(stderr, "ROUTEDEBUG: Sending to %d\n", nextNode);
#endif

			DestinationHole *worm = WH_addWorm(wms, nextNode, 1);

			if (worm) {
				fprintf(f, _WH_DymRoute_CC_setDw, WH_findWormIndex(wms, nextNode));
				fprintf(f, "%s", _WH_DymRoute_CC_send);

			} else {
				return 77;  // some random error code
			}

			i--;
		}

		i++;
	}

	return ret;
}

/* Name WH_DymRoute_route_createFuncRR
 * Adds a "c code" for round robin.
 * Return 0 if OK, something else if error.
 * Used Constants:
    _WH_DymRoute_CC_RRswitch %rrid %rrid
    _WH_DymRoute_CC_RRcase %idcase
    _WH_DymRoute_CC_RRbreak
    _WH_DymRoute_CC_RRend %rrid %rrid %ntotal
 */
uint8_t WH_DymRoute_route_createFuncRR(FILE *f, const uint8_t *const routeDescription, DestinationHoles *wms)
{
	uint8_t ret        = 0;
	int16_t parentesys = 0;
	uint16_t i         = 0;
	uint16_t nextNode  = 0;

	static uint16_t rrCallId = 0;
	uint16_t myrrCallId      = rrCallId++;
	uint16_t rrCaseId        = 0;

	//_WH_DymRoute_CC_RRstatic

	fprintf(f, _WH_DymRoute_CC_RRswitch, myrrCallId, myrrCallId);

	while (routeDescription[i] != '\0') {
		if (routeDescription[i] == ')') {
			parentesys--;

			if (parentesys < 0) {
				break;
			}

		} else if (routeDescription[i] == '(') {
			if (parentesys == 0) {
				fprintf(f, _WH_DymRoute_CC_RRcase, rrCaseId);

#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Function Evaluation: |%s|\n", routeDescription + i);
#endif
				ret += WH_DymRoute_route_create(f, routeDescription + i, wms);

				rrCaseId++;
				fprintf(f, "%s", _WH_DymRoute_CC_RRbreak);
			}

			parentesys++;

		} else if (parentesys == 0 && (routeDescription[i] >= '0' && routeDescription[i] <= '9')) {
			nextNode = atoi((char *)(routeDescription + i));

			while (routeDescription[i] >= '0' && routeDescription[i] <= '9') {
				i++;
			}

			DestinationHole *worm = WH_addWorm(wms, nextNode, 1);

			if (worm) {
				fprintf(f, _WH_DymRoute_CC_RRcase, rrCaseId);
				// RR
				fprintf(f, _WH_DymRoute_CC_setDw, WH_findWormIndex(wms, nextNode));
				fprintf(f, "%s", _WH_DymRoute_CC_send);

#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: %d in RR list\n", nextNode);
#endif

				rrCaseId++;
				fprintf(f, "%s", _WH_DymRoute_CC_RRbreak);

			} else {
				return 77;  // some random error code
			}

			i--;
		}

		i++;
	}

	fprintf(f, _WH_DymRoute_CC_RRend, myrrCallId, myrrCallId, rrCaseId);

	return ret;
}

/* Name WH_DymRoute_route_createFuncCat
 * Adds a "c code" for round robin.
 * Return 0 if OK, something else if error.
 * Used Constants:
        _WH_DymRoute_CC_Catswitch
        _WH_DymRoute_CC_Catcase     %caseId
        _WH_DymRoute_CC_Catbreak
        _WH_DymRoute_CC_Catend
 */
uint8_t WH_DymRoute_route_createFuncCat(FILE *f, const uint8_t *const routeDescription, DestinationHoles *wms)
{
	uint8_t ret        = 0;
	int16_t parentesys = 0;
	uint16_t i         = 0;

	uint16_t myCat = 0;

	fprintf(f, "%s", _WH_DymRoute_CC_Catswitch);

	while (routeDescription[i] != '\0') {
		if (routeDescription[i] == ')') {
			parentesys--;

			if (parentesys < 0) {
				break;
			}

		} else if (routeDescription[i] == '(') {
			if (parentesys == 0) {
				if (!sscanf((char *)routeDescription + i, "%*[^0-9]%hu%*[^0-9].%*[^0-9]", &myCat)) {
					fprintf(stderr, "ERROR: La definición de categoria es incorrecta\n");
					return -1;
				}

#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Category switching: %d\n", myCat);
#endif
				fprintf(f, _WH_DymRoute_CC_Catcase, myCat);

			} else if (parentesys == 1) {
#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Function Evaluation: |%s|\n", routeDescription + i);
#endif
				ret += WH_DymRoute_route_create(f, routeDescription + i, wms);

				fprintf(f, "%s", _WH_DymRoute_CC_Catbreak);
			}

			parentesys++;
		}

		i++;
	}

	fprintf(f, "%s", _WH_DymRoute_CC_Catend);
	return ret;
}

/* Name WH_DymRoute_route_createFuncHash
 * Adds a "c code"  for hashing splitting.
 * Return 0 if OK, something else if error.
 * Used Constants:
        TODO
 */
uint8_t WH_DymRoute_route_createFuncHash(FILE *f, const uint8_t *const routeDescription, DestinationHoles *wms)
{
	uint8_t ret        = 0;
	int16_t parentesys = 0;
	uint16_t i         = 0;
	uint16_t nextNode  = 0;

	uint16_t myHash = 0;

	unsigned numOptions = WH_DymRoute_route_countElems(routeDescription);

	fprintf(f, _WH_DymRoute_CC_Hashswitch, numOptions);

	while (routeDescription[i] != '\0') {
		if (routeDescription[i] == ')') {
			parentesys--;

			if (parentesys < 0) {
				break;
			}

		} else if (routeDescription[i] == '(') {
			if (parentesys == 0) {
				fprintf(f, _WH_DymRoute_CC_Hashcase, myHash);

#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: Hash switching: %d\n", myHash);
				fprintf(stderr, "ROUTEDEBUG: Function Evaluation: |%s|\n", routeDescription + i);
#endif
				ret += WH_DymRoute_route_create(f, routeDescription + i, wms);

				myHash++;
				fprintf(f, _WH_DymRoute_CC_Hashbreak, myHash);
			}

			parentesys++;

		} else if (parentesys == 0 && (routeDescription[i] >= '0' && routeDescription[i] <= '9')) {
			nextNode = atoi((char *)(routeDescription + i));

			while (routeDescription[i] >= '0' && routeDescription[i] <= '9') {
				i++;
			}

			DestinationHole *worm = WH_addWorm(wms, nextNode, 1);

			if (worm) {
				fprintf(f, _WH_DymRoute_CC_Hashcase, myHash);
				// RR
				fprintf(f, _WH_DymRoute_CC_setDw, WH_findWormIndex(wms, nextNode));
				fprintf(f, "%s", _WH_DymRoute_CC_send);

#ifdef LIBWORM_ROUTE_DEBUG
				fprintf(stderr, "ROUTEDEBUG: %d in Hash switch %d\n", nextNode, myHash);
#endif
				fprintf(f, "%s", _WH_DymRoute_CC_Hashbreak);
				myHash++;

			} else {
				return 76;  // some random error code
			}

			i--;
		}

		i++;
	}

	fprintf(f, "%s", _WH_DymRoute_CC_Hashend);
	return ret;
}

/* Name WH_DymRoute_route_createFuncIgnore
 * Adds a "c code" for ignoring packets
 * Return 0 if OK, something else if error.
 * Used Constants:
        _WH_DymRoute_CC_Ignoreunused
 */
uint8_t WH_DymRoute_route_createFuncIgnore(FILE *f, const uint8_t *const routeDescription, DestinationHoles *wms)
{
	uint8_t ret        = 0;
	int16_t parentesys = 0;
	uint16_t i         = 0;

	UNUSED(wms);

#ifdef LIBWORM_ROUTE_DEBUG
	fprintf(stderr, "ROUTEDEBUG: Ignoring messages until the end of this context\n");
#endif
	fprintf(f, "%s", _WH_DymRoute_CC_Ignoreunused);

	while (routeDescription[i] != '\0') {
		if (routeDescription[i] == ')') {
			parentesys--;

			if (parentesys < 0) {
				break;
			}

		} else if (routeDescription[i] == '(') {
			parentesys++;
		}

		i++;
	}

	return ret;
}

/** WH_DymRoute_route_countElems
 * Counts the following elements, for example (1 2), returns 2, but (1 (2 3)) also returns 2.
 * @return the number of elements.
 */
uint32_t WH_DymRoute_route_countElems(const uint8_t *const routeDescription)
{
	uint32_t numElems   = 0;
	uint32_t parentesys = 1;
	uint8_t *pointer    = (uint8_t *)routeDescription;

	if (pointer[0] == '(') {
		pointer++;
	}

	while (parentesys) {
		switch (pointer[0]) {
			case '(':
				if (parentesys == 1) {
					numElems++;
				}

				parentesys++;
				break;

			case ')':
				parentesys--;
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':

				if (parentesys == 1) {
					numElems++;

					while (pointer[0] >= '0' && pointer[0] <= '9') {
						pointer++;
					}

					pointer--;
				}

				break;

			case '\0':
				fprintf(stderr, "ERROR routing counting %d\n", numElems);
				return numElems;  // ERROR!

			default:
				break;
		}

		pointer++;
	}

	return numElems;
}

/** WH_DymRoute_invalidate
 * Invalidate the current routing system, and frees the necesary data
 */
void WH_DymRoute_invalidate()
{
	void *tmppointer;

	if (WH_DymRoute_precompiled_route) {
		tmppointer                    = WH_DymRoute_precompiled_route;
		WH_DymRoute_precompiled_route = NULL;
	}

	if (_WH_DymRoute_libHandle) {
		tmppointer             = _WH_DymRoute_libHandle;
		_WH_DymRoute_libHandle = NULL;

		hptl_waitns(10 * 1000 * 1000L);
		dlclose(tmppointer);
	}
}

/* Name WH_findWorm
 * Return the worm mached (if no exists)
 */
DestinationHole *WH_findWorm(DestinationHoles *wms, const uint16_t holeId)
{
	for (size_t i = 0; i < wms->numberOfWorms; i++) {
		if (wms->worms[i].id == holeId) {
			return wms->worms + i;
		}
	}

	return NULL;
}

/* Name WH_findWormIndex
 * Return the worm index in DestinationHoles
 */
size_t WH_findWormIndex(DestinationHoles *wms, const uint16_t holeId)
{
	for (size_t i = 0; i < wms->numberOfWorms; i++) {
		if (wms->worms[i].id == holeId) {
			return i;
		}
	}

	return -1;
}

/** WH_addWorm
 * @param init Determines if the worm should be initialized or not
 * @return the created connection
 */
DestinationHole *WH_addWorm(DestinationHoles *wms, const uint16_t holeId, const uint8_t init)
{
	DestinationHole *worm = NULL;
	worm                  = WH_findWorm(wms, holeId);

	if (worm) {
		return worm;
	}

	if (holeId == WH_myId) {  // It is a check to remove posible loopbacks; //TODO allow loopbacks
		WH_abort("Loopback not allowed");
	}

	wms->worms = realloc(wms->worms, sizeof(DestinationHole) * (wms->numberOfWorms + 1));

	worm     = calloc(sizeof(DestinationHole), 1);  // TODO REVISAR
	worm->id = holeId;

	if (init) {
		HoleSetup wSetup;

		if (WH_getWormData(&wSetup, holeId)) {
#ifdef LIBWORM_ROUTE_DEBUG
			fprintf(stderr, "ROUTEaddworm: Worm %d no data retrived\n", holeId);
#endif
			return NULL;
		}

		worm->port = wSetup.listenPort;

		if (wSetup.isIPv6) {
			strdup(inet_ntop(AF_INET6, &wSetup.IP, worm->ip, INET6_ADDRSTRLEN));

		} else {
			strdup(inet_ntop(AF_INET, &wSetup.IP, worm->ip, INET_ADDRSTRLEN));
		}
	}

	// TODO: Obtener información de los tipos disponibles.
	worm->numberOfTypes  = 0;
	worm->supportedTypes = NULL;
	worm->conns          = NULL;

	if (init) {
		WH_connectWorm(worm);
	}

	wms->worms[wms->numberOfWorms] = *worm;

	wms->numberOfWorms++;
	return wms->worms + (wms->numberOfWorms - 1);
}

/** WH_disconnectWorm
 * Disconnect from a worm_id and flushes its contents (or it should...)
 * @Param flow: if Set to 1, recv routes are disconnected
 *              if Set to 2, send routes are disconnected
 *              if Set to 3, both routes are disconnected
 */
void WH_disconnectWorm(const uint16_t holeId, int flow)
{
	if (flow & 1)
		WH_removeWorm(&WH_myRcvWorms, holeId);
	if (flow & 2)
		WH_removeWorm(&WH_myDstWorms, holeId);
}

/** WH_removeWorm
 * Removes, close connections and frees all the data related to that worm.
 */
void WH_removeWorm(DestinationHoles *wms, const uint16_t holeId)
{
#ifdef LIBWORM_DEBUG
	fprintf(stderr, "[WH]: Removing worm id=%d...\n", holeId);
#endif

	/*if wms is destworm, route table must be disabled*/
	if (wms == &WH_myDstWorms) {
#ifdef LIBWORM_DEBUG
		fprintf(stderr, "[WH]: Invalidating route table...\n");
#endif
		WH_DymRoute_invalidate();
	}

	DestinationHole *worm = WH_findWorm(wms, holeId);
	ssize_t index         = WH_findWormIndex(wms, holeId);

	wms->numberOfWorms--;

	if (index == -1) {
#ifdef LIBWORM_DEBUG
		fprintf(stderr, "[WH]: worm not found! %d (%ld)\n", holeId, index);
#endif
		return;
	}

	if ((wms->numberOfWorms - index) > 0) {
#ifdef LIBWORM_DEBUG
		fprintf(stderr, "[WH]: memmoving...(index=%ld, nworms=%lu)\n", index, wms->numberOfWorms);
#endif
		memmove(wms->worms + index, wms->worms + index + 1, (wms->numberOfWorms - index) * sizeof(DestinationHole *));
	}

	for (size_t i = 0; i < worm->numberOfTypes; i++) {
		if (worm->conns[i]) {
			destroy_asyncSocket(&(worm->conns[i]->socket));
			// fprintf(stderr, "[WH]: free conns[%lu]\n",i);
			free(worm->conns[i]);
			worm->conns[i] = NULL;
		}
	}

	// fprintf(stderr, "[WH]: free conns\n");
	free(worm->conns);
	// fprintf(stderr, "[WH]: free supportedTypes\n");
	free(worm->supportedTypes);
	// fprintf(stderr, "[WH]: free worm\n");
	// free(worm); //TODO this causes free corruption... why!?
}

/************************************************************/