#define _GNU_SOURCE //TODO find a better and standar way of set affinity
#include <worm_private.h>

#include "async_inline.c"
//#define _WORMLIB_DEBUG_
//#define _WORMLIB_DEBUG_FLUSH_
/*
*Global variables
*/
Worm2EinsConn WH_einsConn;
uint16_t WH_myId;
WormSetup WH_mySetup;

DestinationWorms WH_myDstWorms = {0};
DestinationWorms WH_myRcvWorms = {0};

WormConfig WH_myConfig = {0};

pthread_t WH_wormThread;


volatile uint8_t WH_bussy = 0;
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

	memcpy(sTypes, types, sizeof(ConnectionDataType)*nTypes);

	if (WH_myConfig.inputTypes != NULL) {
		ConnectionDataType *tmp;
		tmp = WH_myConfig.inputTypes;

		if (WH_myConfig.numInputTypes > nTypes) {
			WH_myConfig.numInputTypes = nTypes;
		}

		WH_myConfig.inputTypes = sTypes;
		free(tmp);
	}

	WH_myConfig.inputTypes = sTypes;
	WH_myConfig.numInputTypes = nTypes;

	return 0;
}

#include <sched.h>
//#undef _GNU_SOURCE

/* Name WH_init
* Starts the WormHole Library
* Return 0 if OK, something else if error.
*/
uint8_t WH_init(void)
{
	WH_myDstWorms.numberOfWorms = 0;
	WH_myDstWorms.worms = malloc(sizeof(DestinationWorm));

	WH_myRcvWorms.numberOfWorms = 0;
	WH_myRcvWorms.worms = malloc(sizeof(DestinationWorm));

	WH_myId = atoi(getenv("WORM_ID"));

	WH_einsConn.Port = atoi(getenv("EINSTEIN_PORT"));
	WH_einsConn.IP = inet_addr(getenv("EINSTEIN_IP"));

	WH_einsConn.socket = tcp_connect_to(getenv("EINSTEIN_IP"), WH_einsConn.Port);

	if (WH_einsConn.socket == -1) {
#ifdef _WORMLIB_DEBUG_
		perror("[WH]: Error connecting to Einstein");
#endif
		return 1;
	}

	// Fill hello message with worm id
	size_t hellomsgSize = sizeof(enum ctrlMsgType) + sizeof(uint16_t);
	uint8_t hellomsg[hellomsgSize];
	enum ctrlMsgType *msgType = (enum ctrlMsgType *)hellomsg;
	*msgType = HELLOEINSTEIN;
	* ((uint16_t *)(hellomsg + sizeof(enum ctrlMsgType))) = htons(WH_myId);

	// Send hello message
	if (tcp_message_send(WH_einsConn.socket, hellomsg, hellomsgSize) != 0) {
#ifdef _WORMLIB_DEBUG_
		perror("[WH]: Error sending HELLO to Einstein");
#endif
		return 1;
	}

	// Receive WormSetup
	uint8_t *wormSetupMsg = (uint8_t *) &WH_mySetup;

	if (tcp_message_recv(WH_einsConn.socket, wormSetupMsg, sizeof(WormSetup), 1) != sizeof(WormSetup)) {
#ifdef _WORMLIB_DEBUG_
		perror("[WH]: Error recv myconfig from Einstein");
#endif
		return 1;
	}

	// Receive connectionDescription
	WH_mySetup.connectionDescription = calloc(WH_mySetup.connectionDescriptionLength + 1, sizeof(char));

	if (tcp_message_recv(WH_einsConn.socket, WH_mySetup.connectionDescription, WH_mySetup.connectionDescriptionLength, 1) != WH_mySetup.connectionDescriptionLength) {
#ifdef _WORMLIB_DEBUG_
		perror("[WH]: Error recv connectionDescription from Einstein");
#endif
		return 1;
	}

	//TypeSetup
	if (WH_myConfig.inputTypes == NULL) {
		ConnectionDataType tmpType;
		tmpType.type = ARRAY;
		tmpType.ext.arrayType = UINT8;
		WH_setup_types(1, &tmpType);
	}

	// Establecer afinidad
	if (WH_mySetup.core != 0) {
		if (sched_setaffinity(0, sizeof(WH_mySetup.core), (cpu_set_t *) &WH_mySetup.core)) {
			perror("[WH]: Error setting up process affinity");
		}
	}

	// Lanzar hilo de recepción de conexiones
	WH_bussy = 1;

	if (pthread_create(&WH_wormThread, NULL, WH_thread, NULL)) {
#ifdef _WORMLIB_DEBUG_
		perror("[WH]: Error creating thread");
#endif
		return 1;
	}

	//Creamos el enrutado dinámico
	if (WH_DymRoute_init(WH_mySetup.connectionDescription, &WH_myDstWorms)) {
#ifdef _WORMLIB_DEBUG_
		fprintf(stderr, "[WH]: Error creating DymRoute\n");
#endif
		return 1;
	}

	WH_bussy = 0;

	struct timeval ts;
	ts.tv_sec  =   0;
	ts.tv_usec = 50000;

	if (setsockopt(WH_einsConn.socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&ts,
				   sizeof(struct timeval)) < 0) {
		fputs("[TH] setsockopt failed [1]\n", stderr);
	}

	if (setsockopt(WH_einsConn.socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&ts,
				   sizeof(struct timeval)) < 0) {
		fputs("[TH] setsockopt failed [2]\n", stderr);
	}

	return 0;
}

/** WH_halt
 * Stops and free the WormHole Library
 * @return 0 if OK, something else if error.
 */
uint8_t WH_halt(void)
{
	//TODO fix this function, in order to orderly shutdown
	enum ctrlMsgType type = HALT;

	if (tcp_message_send(WH_einsConn.socket, &type, sizeof(type))) {
		return 1;
	}

	WH_halting = 1;

	struct timespec ts;
	ts.tv_sec = 1;
	ts.tv_nsec = 0;

	WH_flushIO();

	while (WH_halting) {
		nanosleep(&ts, 0);
	}

	return 0;
}

/** WH_flushIO
 * Flushes all the IO queues.
 * @return 0 if OK, something else if error.
 */
uint8_t WH_flushIO(void)
{
	uint8_t ret = 0;

	// for each dstWorm
	for (uint32_t i = 0; i < WH_myDstWorms.numberOfWorms; i++) {
		if (WH_myDstWorms.worms[i].conns) {
			// for each Connection/type
			for (uint32_t j = 0; j < WH_myDstWorms.worms[i].numberOfTypes; j++) {
				if (WH_myDstWorms.worms[i].conns[j] != NULL) {
#ifdef _WORMLIB_DEBUG_
#ifdef _WORMLIB_DEBUG_FLUSH_
					fprintf(stderr, "[WORM:debug] Flushing OUT Connection: %d [%d:%d]\n",
							WH_myDstWorms.worms[i].id,
							WH_myDstWorms.worms[i].conns[j]->type.type,
							WH_myDstWorms.worms[i].conns[j]->type.ext.arrayType);
#endif
#endif
					flush_send(&(WH_myDstWorms.worms[i].conns[j]->socket));
				}
			}
		}
	}

	// for each recvWorm
	/*for (int i = 0; i < WH_myRcvWorms.numberOfWorms; i++) {
		if (WH_myRcvWorms.worms[i].conns) {
			// for each Connection/type
			for (int j = 0; j < WH_myRcvWorms.worms[i].numberOfTypes; j++) {
				if (WH_myRcvWorms.worms[i].conns[j] != NULL) {
	#ifdef _WORMLIB_DEBUG_
					fprintf(stderr, "[WORM:debug] Flushing IN Connection: %d [%d:%d]\n",
							WH_myRcvWorms.worms[i].id,
							WH_myRcvWorms.worms[i].conns[j]->type.type,
							WH_myRcvWorms.worms[i].conns[j]->type.ext.arrayType);
	#endif
					flush_recv(&(WH_myRcvWorms.worms[i].conns[j]->socket));
				}
			}
		}
	}*/

	return ret;
}

/* Name WH_thread
 * A worm Thread listening for info/petitions.
 */
void *WH_thread(void *arg)
{
	UNUSED(arg);

	int listeningSocket = tcp_listen_on_port(WH_mySetup.listenPort);
	struct timeval ts;
	ts.tv_sec  =   0;
	//ts.tv_usec = 250000;
	ts.tv_usec = 250000;

	if (listeningSocket == -1) {
		perror("Error opening socket");
		exit(-1);
	}

	while (1) {
		//poll for incomming connections/requests.
		int tmpsock = tcp_accept(listeningSocket, &ts); //TODO Optimizar para no reconfigurar constantemente el socket
		//int socket = tcp_accept(listeningSocket, NULL);

		if (tmpsock < 0) {
			enum ctrlMsgType type;

			//Lectura desde Einstein
			if (!WH_bussy) {
				if (tcp_message_recv(WH_einsConn.socket, &type, sizeof(type), 0) != sizeof(type)) {
					continue;

				} else {
					switch (type) {
					case HALT:
						fputs("Halting Worm by EINSTEIN...\n", stderr);

						if (WH_halting) {
							WH_halting = 0;

						} else {
							exit(0);
						}

					default:
						continue;
					}
				}
			}

		} else {

			SyncSocket *socket = tcp_upgrade2syncSocket(tmpsock, NOSSL, NULL);
			enum wormMsgType type;

			while (tcp_message_srecv(socket, &type, sizeof(type), 1) == sizeof(type)) {
				if (WH_TH_checkMsgType(type, socket)) {
					break;
				}
			}

			tcp_sclose(socket);
		}
	}

	return NULL;
}


/** WH_TH_checkMsgType
 * check the message type
 * @return 0 if ok, 1 if some error
 */
int WH_TH_checkMsgType(enum wormMsgType type, SyncSocket *socket)
{
	int ret = 0;

	switch (type) {
	case HELLO: //Contestamos a Hello
	default:
		WH_TH_hellow(socket); //TODO check if ret should be 1
		break;

	case SETUPWORMCONN:  //Establecemos una conexión completa con otro worm
		WH_TH_setupworm(socket);
		break;

	case SSLSTART:
		ret = syncSocketStartSSL(socket, SRVSSL, NULL);
		break;
	}

	return  ret;
}

/** WH_TH_hellow
 * Process a HELLOW message
 */
inline void WH_TH_hellow(SyncSocket *socket)
{
	enum wormMsgType type;
	type = WORMINFO; //Con Worm Info

	//fputs("Worm Info Pedida\n", stderr);

	if (tcp_message_ssend(socket, &type, sizeof(type))) {
		perror("Error contestando socket [1]\n");
		tcp_sclose(socket); //cerramos el socket
		return;
	}

	if (tcp_message_ssend(socket, &WH_mySetup, sizeof(WH_mySetup))) { //Con wormSetup
		perror("Error contestando socket [2]\n");
		tcp_sclose(socket); //cerramos el socket
		return;
	}

	if (tcp_message_ssend(socket, &WH_myConfig, sizeof(WH_myConfig))) { //Con wormConfig
		perror("Error contestando socket [3]\n");
		tcp_sclose(socket); //cerramos el socket
		return;
	}

	//fprintf(stderr, "Enviando config con size=%zu\n", WH_myConfig.numInputTypes);

	if (tcp_message_ssend(socket, WH_myConfig.inputTypes, sizeof(ConnectionDataType)*WH_myConfig.numInputTypes)) { //Con wormConfig
		perror("Error contestando socket [4]");
		tcp_sclose(socket); //cerramos el socket
		return;
	}

	//fputs("Worm Info Respondida\n", stderr);
}

/** SETUPWORMCONN
 * Process a HELLOW message
 */
inline void WH_TH_setupworm(SyncSocket *socket)
{
	DestinationWorm tmpDestWorm;

	//Recibimos un destinationWorm
	if (tcp_message_srecv(socket, &tmpDestWorm, sizeof(DestinationWorm), 1) != sizeof(DestinationWorm)) {
		fputs("Error configurando socket", stderr);
		tcp_sclose(socket); //cerramos el socket
		return;
	}

	DestinationWorm *tmpDestWormPtr
		= WH_addWorm(&WH_myRcvWorms, tmpDestWorm.id, 0);

	tmpDestWormPtr->ip = tmpDestWorm.ip;
	tmpDestWormPtr->port = tmpDestWorm.port;
	tmpDestWormPtr->supportedTypes = realloc(tmpDestWormPtr->supportedTypes,
									 (tmpDestWormPtr->numberOfTypes + 1) * sizeof(ConnectionDataType));

	//TODO fix para en caso de que se reduzca/reordene la lista, no se quede memoria perdida...
	if (tmpDestWormPtr->conns)
		tmpDestWormPtr->conns = realloc(tmpDestWormPtr->conns,
										(tmpDestWormPtr->numberOfTypes + 1) * sizeof(Connection *)); //TODO test ==NULL

	else {
		tmpDestWormPtr->conns = calloc(sizeof(Connection *), tmpDestWormPtr->numberOfTypes + 1);
	}

	tmpDestWormPtr->conns[tmpDestWormPtr->numberOfTypes] = calloc(sizeof(Connection), 1); //TODO test ==NULL

	if (tcp_message_srecv(socket, tmpDestWormPtr->conns[tmpDestWormPtr->numberOfTypes], sizeof(Connection), 1) != sizeof(Connection)) {
		fputs("Error configurando socket", stderr);
		tcp_sclose(socket); //cerramos el socket
		return;
	}

	socket_sync_to_async_recv(&(tmpDestWormPtr->conns[tmpDestWormPtr->numberOfTypes]->socket), socket);

	tmpDestWormPtr->numberOfTypes++;
#ifdef _WORMLIB_DEBUG_
	fprintf(stderr, "[WORM] Input Connection: %d\n", tmpDestWormPtr->id);
#endif
}

/* Name WH_connectionPoll
	* Poll data from some socket
	* Return some connection with data, NULL if error/timeout.
	*/
Connection *WH_connectionPoll(DestinationWorms *wms)
{
	static uint32_t wormIndex = 0;
	static uint32_t connIndex = 0;

	connIndex++;

	// Search for an existent worm + connection
	if (wormIndex >= wms->numberOfWorms) {
		wormIndex = 0;
		connIndex = 0;

		if (wms->numberOfWorms == 0) {
			return NULL;    // Check por si aun no se ha conectado a ningún nodo de entrada.
		}
	}

	while (connIndex >= wms->worms[wormIndex].numberOfTypes) {
		wormIndex = (wormIndex + 1) % wms->numberOfWorms;
		connIndex = 0;
	}

	uint32_t startingWormIndex = wormIndex;
	uint32_t startingConnIndex = connIndex;

	do {
		if (can_be_read(&(wms->worms[wormIndex].conns[connIndex]->socket))) {
			return wms->worms[wormIndex].conns[connIndex];
		}

		connIndex++;

		while (connIndex >= wms->worms[wormIndex].numberOfTypes) {
			wormIndex = (wormIndex + 1) % wms->numberOfWorms;
			connIndex = 0;
		}
	} while (!(wormIndex == startingWormIndex && connIndex == startingConnIndex)); //arreglado bug

	return NULL;

}

/* Name WH_connectWorm
 * Connect and fill the socket data.
 * Return 0 if OK, something else if error.
 */
uint8_t WH_connectWorm(DestinationWorm *c)
{
	struct in_addr ip_addr;
	ip_addr.s_addr = c->ip;

	int socket = 0;

	//fprintf(stderr, "[DEBUG:%d ; %s:%d]\n", __LINE__, inet_ntoa(ip_addr), c->port);

	do {
		// Force keep-trying
		socket = tcp_connect_to(inet_ntoa(ip_addr), c->port);

		if (socket < 0) {
			fprintf(stderr, "%s:%d\t", inet_ntoa(ip_addr), c->port);
			perror("Error estableciendo conexion volatil con WORM");
			fflush(stderr);
		}
	} while (socket < 0);

	//Solicitamos datos...
	enum wormMsgType type = HELLO; //Con Worm Info

	WormSetup wormSetup;
	WormConfig wormConfig = {0};

	if (tcp_message_send(socket, &type, sizeof(type))) {
		perror("Error solicitando información del Worm [1]");
		close(socket);
		return 1;
	}

	if (tcp_message_recv(socket, &type, sizeof(type), 1) != sizeof(type)) {
		perror("Error solicitando información del Worm [1]");
		close(socket);
		return 1;
	}

	if (type != WORMINFO) {
		fprintf(stderr, "Respuesta de worm no esperada...\n");
		close(socket);
		return 1;
	}

	if (tcp_message_recv(socket, &wormSetup, sizeof(wormSetup), 1) != sizeof(wormSetup)) { //Con wormSetup
		perror("Error solicitando información del Worm [2]");
		close(socket);
		return 1;
	}

	if (tcp_message_recv(socket, &wormConfig, sizeof(wormConfig), 1) != sizeof(wormConfig)) { //Con wormConfig
		perror("Error solicitando información del Worm [3]");
		close(socket);
		return 1;
	}

	wormConfig.inputTypes = realloc(c->supportedTypes, sizeof(ConnectionDataType) * wormConfig.numInputTypes);

	if (tcp_message_recv(socket, wormConfig.inputTypes, sizeof(ConnectionDataType)*wormConfig.numInputTypes, 1) != (ssize_t)(sizeof(ConnectionDataType)*wormConfig.numInputTypes)) {  //Con wormConfig
		perror("Error solicitando información del Worm [4]");
		close(socket);
		return 1;
	}

	close(socket);

	//Rellenamos el worm entrante
	c->id = wormSetup.id;
	c->numberOfTypes = wormConfig.numInputTypes;
	c->supportedTypes = wormConfig.inputTypes;

	//TODO en caso de que se reduzca el numero de conexiones problemas
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
uint8_t WH_setupConnectionType(DestinationWorm *dw, const ConnectionDataType *const type)
{
	struct in_addr ip_addr;
	ip_addr.s_addr = dw->ip;

	WH_connectWorm(dw);

	int tmpsock = tcp_connect_to(inet_ntoa(ip_addr), dw->port);

	if (tmpsock < 0) {
		return 1;
	}

	SyncSocket *socket = NULL;

	//setup SSL
	if (WH_mySetup.isSSLNode) {
		enum wormMsgType msgtype = SSLSTART; //Con Worm config

		if (tcp_message_ssend(socket, &msgtype, sizeof(msgtype))) {
			fputs("Error configurando Worm externo [SSL]", stderr);
			return 1;
		}

		if (syncSocketStartSSL(socket, CLISSL, NULL)) {
			fputs("Error configurando Worm externo [SSL 2]", stderr);
			return 1;
		}

	} else {
		socket = tcp_upgrade2syncSocket(tmpsock, NOSSL, NULL);
	}


	int8_t flag = 0;

	for (uint32_t i = 0; i < dw->numberOfTypes; i++) {
		if (!WH_connectionDataTypecmp(dw->supportedTypes + i, type)) {
			flag = 1;
			break;
		}
	}

	if (!flag) {
#ifdef _WORMLIB_DEBUG_
		fprintf(stderr, "No se puede abrir un socket con el datatype solicitado!\n");
#endif
		tcp_sclose(socket);
		return 1;
	}

	//Informamos del tipo de conexion...
	enum wormMsgType msgtype = SETUPWORMCONN; //Con Worm config

	if (tcp_message_ssend(socket, &msgtype, sizeof(msgtype))) {
		fputs("Error configurando Worm externo", stderr);
		return 1;
	}

	DestinationWorm dwtmp;
	dwtmp.id = WH_mySetup.id;
	dwtmp.ip = WH_mySetup.IP;
	dwtmp.port = WH_mySetup.listenPort;
	dwtmp.numberOfTypes = 1;
	dwtmp.supportedTypes = NULL;
	dwtmp.conns = NULL;

	if (tcp_message_ssend(socket, &dwtmp, sizeof(dwtmp))) { //DstWorm
		fprintf(stderr, "Error configurando Worm externo %d\n", dw->id);
		return 1;
	}

	Connection conntmp;
	conntmp.type = *type;
	conntmp.socket = (AsyncSocket) {
		0
	};

	if (tcp_message_ssend(socket, &conntmp, sizeof(conntmp))) { //DstWorm
		fprintf(stderr, "Error configurando Worm externo %d\n", dw->id);
		return 1;
	}

	flag = 0;

	for (uint32_t i = 0; i < dw->numberOfTypes; i++) {
		if (!WH_connectionDataTypecmp(dw->supportedTypes + i, type)) {

			dw->conns[i] = calloc(sizeof(Connection), 1);
			socket_sync_to_async_send(&(dw->conns[i]->socket), socket);
			dw->conns[i]->type = *type;
			flag = 1;
			break;
		}
	}

	if (!flag) {
		tcp_sclose(socket);
		return 1;
	}

#ifdef _WORMLIB_DEBUG_
	fprintf(stderr, "[WORM] Output Connection: %d\n", dw->id);
#endif

	return 0;
}


uint8_t WH_getWormData(WormSetup *ws, const uint16_t wormId)
{
	enum ctrlMsgType ctrlMsg = QUERYID;

	if (tcp_message_send(WH_einsConn.socket, (uint8_t *) &ctrlMsg, sizeof(enum ctrlMsgType)) != 0) {
		return 1;
	}

	if (tcp_message_send(WH_einsConn.socket, (void *) &wormId, sizeof(uint16_t)) != 0) {
		return 1;
	}

	if (tcp_message_recv(WH_einsConn.socket, (uint8_t *) &ctrlMsg, sizeof(enum ctrlMsgType), 1) != sizeof(enum ctrlMsgType)) {
		return 1;
	}

	if (ctrlMsg != CTRL_OK) {
		return 1;
	}

	if (tcp_message_recv(WH_einsConn.socket, (uint8_t *) ws, sizeof(WormSetup), 1) != sizeof(WormSetup)) {
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
	//TODO optimize converting to static array
	switch (type->type) {
	case   INT8:
	case  UINT8:
		return 1;

	case  INT16:
	case UINT16:
		return 2;

	case  INT32:
	case UINT32:
		return 4;

	case  INT64:
	case UINT64:
		return 8;

	case  STRING:
		return 1;

	case ARRAY:
		switch (type->ext.arrayType) {
		case   INT8:
		case  UINT8:
			return 1;

		case  INT16:
		case UINT16:
			return 2;

		case  INT32:
		case UINT32:
			return 4;

		case  INT64:
		case UINT64:
			return 8;

		case  STRING:
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
	Dynamic Routing Library
*************************************************************/
//#define _DYM_ROUTE_DEBUG_
/***************************************/
extern uint8_t _binary_obj_structures_h_start;
extern uint8_t _binary_obj_structures_h_end;

const char *_WH_DymRoute_CC_includes = "\n"
									   "#include<stdint.h>\n"
									   "#include<stdio.h>\n"
									   "#include <pthread.h>\n";
const char *_WH_DymRoute_CC_FuncStart = "\n\n"
										"uint8_t WH_DymRoute_precompiled_route (const void *const data, const MessageInfo *const mi, DestinationWorms *const cns)\n{\n"
										"int ret = 0;\n"
										"DestinationWorm *dw;\n";
const char *_WH_DymRoute_CC_FuncEnd = "\n"
									  "return ret;"
									  "}\n";
const char *_WH_DymRoute_CC_send =    "ret += WH_DymRoute_send(data, mi, dw);\n";
const char *_WH_DymRoute_CC_setDw =   "dw = cns->worms+%d;\n";

//RR special
//const char *_WH_DymRoute_CC_RRstatic    =    "static uint16_t rr%d=0;\n";
const char *_WH_DymRoute_CC_RRswitch    =    "static uint16_t rr%d=0;\n"
		"switch(rr%d){\n";
const char *_WH_DymRoute_CC_RRcase      =    "case %d :\n{ ";
const char *_WH_DymRoute_CC_RRbreak     =    "break;\n} ";
const char *_WH_DymRoute_CC_RRend       =    "default:\n"
		"ret++;"
		"}\n"
		"rr%d=(rr%d+1) %% %d;\n";

//Cat special
const char *_WH_DymRoute_CC_Catswitch   =    "switch(mi->category){\n";
const char *_WH_DymRoute_CC_Catcase     =    "case %d :\n{ ";
const char *_WH_DymRoute_CC_Catbreak    =    "break;\n} ";
const char *_WH_DymRoute_CC_Catend      =    "default:\n"
		"ret++;"
		"}\n";

void *_WH_DymRoute_libHandle;
/***************************************/

/* Name WH_DymRoute_precompiled_route
 * Enrute a message
 * Return the number of msgs sent
 */
uint8_t (*WH_DymRoute_precompiled_route)(const void *const data, const MessageInfo *const mi, DestinationWorms *const cns) = 0;

/* Name WH_send
 * TODO
 * Params:
 * Return 0 if OK, something else if error.
 */
uint8_t WH_send(const void *const data, const MessageInfo *const mi)
{
	return WH_DymRoute_route(data, mi);
}

/** WH_recv
 * TODO
 * Params:
 * @return the number of bytes readed, 0 if ERROR or none.
 */
uint32_t WH_recv(void *data, MessageInfo *mi)
{
	//int pollCnt = 0;
#ifdef _DYM_ROUTE_DEBUG_
	fprintf(stderr, "ROUTEDEBUG: Polling...\n");
#endif
	Connection *c = NULL;

	do {
		c = WH_connectionPoll(&WH_myRcvWorms);

		/*if (!c) {
			pollCnt++;

			if (pollCnt > 100000) { //TODO poner un valor menos "aleatorio"
				pollCnt = 0;

				//struct timespec ts;
				//ts.tv_sec = 0;
				//ts.tv_nsec = 1000;
				//nanosleep(&ts, 0);
				//WH_flushIO();
			}
		}*/
	} while ((!c) || ((mi->type) ? (c->type.type != mi->type->type) : 1)); //TODO, tener en cuenta tipos internos en arrays, etc.

#ifdef _DYM_ROUTE_DEBUG_
	fprintf(stderr, "ROUTEDEBUG: Msg found!...\n");
#endif

	if (!mi->type) {
		mi->type = &(c->type);
	}

	uint32_t tmp;
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
		if (!tcp_message_recv_async(&(c->socket), &tmp, sizeof(tmp)))
			if (!tcp_message_recv_async(&(c->socket), &data, 1 * tmp)) {
				ret = 1 * tmp;
			}

		break;

	case ARRAY: {
			if (!tcp_message_recv_async(&(c->socket), &tmp, sizeof(tmp))) {
#ifdef _DYM_ROUTE_DEBUG_
				fprintf(stderr, "ROUTEDEBUG: Array of %d elements\n", tmp);
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
					if (! tcp_message_recv_async(&(c->socket), data, 4 * tmp)) {
						ret = 4 * tmp;
					}

					break;

				case INT64:
				case UINT64:
					if (! tcp_message_recv_async(&(c->socket), data, 8 * tmp)) {
						ret = 8 * tmp;
					}

					break;

				default:
					fprintf(stderr, "ARRAYTYPE: NOT YET IMPLEMENTED [%d]\n", c->type.ext.arrayType); //TODO implement
					break;
				}

			} else {
#ifdef _DYM_ROUTE_DEBUG_
				fprintf(stderr, "ROUTEDEBUG: Error recvArray\n");
#endif
			}

			break;
		}

	default:
		fprintf(stderr, "NOT YET IMPLEMENTED [%d]\n", c->type.type); //TODO implement
		break;
	}

#ifdef _DYM_ROUTE_DEBUG_
	fprintf(stderr, "ROUTEDEBUG: Recv %dB\n", ret);
#endif

	return ret;
}

/* Name WH_DymRoute_route
* Enrute a message
* Return 0 if OK, something else if error.
*/
uint8_t WH_DymRoute_route(const void *const data, const MessageInfo *const mi)
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
uint8_t WH_DymRoute_init(const uint8_t *const routeDescription, DestinationWorms *wms)
{
	pid_t myPid = getpid();
	char *tmpString = malloc(1024);
	char *errorString;

	if (!tmpString) {
#ifdef _DYM_ROUTE_DEBUG_
		perror("ROUTEDEBUG: Error in Malloc");
#endif
		return 1;
	}

	sprintf(tmpString, "/tmp/%d.c", myPid);
	FILE *f = fopen(tmpString, "w+");

	if (!f) {
		free(tmpString);
#ifdef _DYM_ROUTE_DEBUG_
		perror("ROUTEDEBUG: Error creating tmp.c");
#endif
		return 2;
	}

	/*Write headers to the file*/
	fwrite(_WH_DymRoute_CC_includes, strlen(_WH_DymRoute_CC_includes), 1, f);
	fwrite(&_binary_obj_structures_h_start, &_binary_obj_structures_h_end - &_binary_obj_structures_h_start, 1, f);
	fwrite(_WH_DymRoute_CC_FuncStart, strlen(_WH_DymRoute_CC_FuncStart), 1, f);

	int ret = WH_DymRoute_route_create(f, routeDescription, wms);
	fwrite(_WH_DymRoute_CC_FuncEnd, strlen(_WH_DymRoute_CC_FuncEnd), 1, f);
	fclose(f);

	/*Compile the .c*/
	if (!ret) {
		sprintf(tmpString, "gcc -O3 -Wall /tmp/%d.c -o /tmp/%d.so -shared -Llib -lworm -lpthread -fPIC ", myPid, myPid);

#ifdef _DYM_ROUTE_DEBUG_
		fprintf(stderr, "ROUTEDEBUG: Calling %s\n", tmpString);
#endif

		ret = system(tmpString);

#ifdef _DYM_ROUTE_DEBUG_

		if (ret) {
			fprintf(stderr, "ROUTEDEBUG: Error calling gcc\n");
		}

#endif
	}

	/*Link the .SO*/
	if (!ret) {
		sprintf(tmpString, "/tmp/%d.so", myPid);
		_WH_DymRoute_libHandle = dlopen(tmpString, RTLD_NOW);

		if (!_WH_DymRoute_libHandle) {
#ifdef _DYM_ROUTE_DEBUG_
			fprintf(stderr, "ROUTEDEBUG: %s\n", dlerror());
#endif
			free(tmpString);
			return 5;
		}

		WH_DymRoute_precompiled_route = dlsym(_WH_DymRoute_libHandle, "WH_DymRoute_precompiled_route");

		if ((errorString = dlerror()) != NULL)  {
			fputs(errorString, stderr);
			ret = 6;
		}
	}

	free(tmpString);
	return ret;
}

/* Name WH_DymRoute_send
 * Sends a message to the network
 * Return 0 if OK, something else if error.
  */
uint8_t WH_DymRoute_send(const void *const data, const MessageInfo *const mi, const DestinationWorm *const dw)
{
	//TODO search info
	//WH_setupConnectionType
	for (uint32_t i = 0; i < dw->numberOfTypes; i++) {
		if (!WH_connectionDataTypecmp(dw->supportedTypes + i, mi->type)) {
			//tcp_message_send_async(AsyncSocket *sock, const void *message, size_t len)
			if (!dw->conns[i]) {
				if (WH_setupConnectionType((DestinationWorm *)dw, mi->type)) {
#ifdef _DYM_ROUTE_DEBUG_
					fprintf(stderr, "ROUTEDEBUG: sending data to worm: %d [FAIL-SETUP]\n", dw->id);
#endif
					return 1;
				}
			}

			if (mi->type->type == ARRAY) {
#ifdef _DYM_ROUTE_DEBUG_
				fprintf(stderr, "[ARRAY %dB] ", mi->size);
#endif
				tcp_message_send_async(&(dw->conns[i]->socket), &(mi->size), sizeof(mi->size));
			}

#ifdef _DYM_ROUTE_DEBUG_
			fprintf(stderr, "ROUTEDEBUG: sending data (%dB) to worm: %d\n", mi->size, dw->id);
#endif
			return tcp_message_send_async(&(dw->conns[i]->socket),
										  data,
										  mi->size * WH_typesize(mi->type));
		}
	}

#ifdef _DYM_ROUTE_DEBUG_
	fprintf(stderr, "ROUTEDEBUG: sending data to worm: %d [FAIL]\n", dw->id);
#endif
	//fprintf(stderr, "Sending %d bytes to %d\n", mi->size, cn->ip);
	return 1;
}

/* Name WH_DymRoute_route_create
 * Starts the Dynamic Routing Library, and setups connection configuration.
 * Return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_create(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms)
{
	uint8_t ret = 0;
	int16_t parentesys = 0;
	uint16_t i = 0;
	uint16_t nextNode = 0;

	while (routeDescription[i] != '\0') {
		if (routeDescription[i] == ')') {
			parentesys--;

			if (parentesys < 0) {
				break;
			}

		} else if (routeDescription[i] == '(') {
			parentesys++;

			if (parentesys == 1) {
#ifdef _DYM_ROUTE_DEBUG_
				fprintf(stderr, "ROUTEDEBUG: Function Evaluation: |%s|\n", routeDescription + i);
#endif
				ret += WH_DymRoute_route_createFunc(f, routeDescription + i, wms);
			}

		} else if (parentesys == 0 && (routeDescription[i] >= '0' && routeDescription[i] <= '9')) {
			nextNode = atoi((char *)(routeDescription + i));

			while (routeDescription[i] >= '0' && routeDescription[i] <= '9') {
				i++;
			}

#ifdef _DYM_ROUTE_DEBUG_
			fprintf(stderr, "ROUTEDEBUG: Sending to %d\n", nextNode);
#endif

			DestinationWorm *worm = WH_addWorm(wms, nextNode, 1);

			if (worm) {
				fprintf(f, _WH_DymRoute_CC_setDw, WH_findWormIndex(wms, nextNode));
				fprintf(f, "%s", _WH_DymRoute_CC_send);

			} else {
				return 77;    //some random error code
			}

			i--;
		}

		i++;
	}


	return ret;
}

/* Name WH_DymRoute_route_createFunc
 * Searchs for a Function, and calls the correct function.
 * Return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_createFunc(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms)
{
	uint16_t i = 1;

	if (routeDescription[0] != '(') {
#ifdef _DYM_ROUTE_DEBUG_
		fprintf(stderr, "ROUTEDEBUG: Route CreateFunc, unexpected call");
#endif
		return WH_DymRoute_route_create(f, routeDescription + i, wms);
	}

	switch (routeDescription[i]) {
	case 'd':
	case 'D':
		// DUP Function
#ifdef _DYM_ROUTE_DEBUG_
		fprintf(stderr, "ROUTEDEBUG: Calling DUP...\n");
#endif
		return WH_DymRoute_route_create(f, routeDescription + i, wms);

	case 'r':
	case 'R':
		// Round Robin Function
#ifdef _DYM_ROUTE_DEBUG_
		fprintf(stderr, "ROUTEDEBUG: Calling RR...\n");
#endif
		return WH_DymRoute_route_createFuncRR(f, routeDescription + i, wms);

	case 'c':
	case 'C':
		// Category Function
#ifdef _DYM_ROUTE_DEBUG_
		fprintf(stderr, "ROUTEDEBUG: Calling CAT...\n");
#endif
		return WH_DymRoute_route_createFuncCat(f, routeDescription + i, wms);

	default:
#ifdef _DYM_ROUTE_DEBUG_
		fprintf(stderr, "ROUTEDEBUG: Unexpected routing function.");
#endif
		return -1;
	}
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
uint8_t WH_DymRoute_route_createFuncRR(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms)
{
	uint8_t ret = 0;
	int16_t parentesys = 0;
	uint16_t i = 0;
	uint16_t nextNode = 0;

	static uint16_t rrCallId = 0;
	uint16_t myrrCallId = rrCallId++;
	uint16_t rrCaseId = 0;

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

#ifdef _DYM_ROUTE_DEBUG_
				fprintf(stderr, "ROUTEDEBUG: Function Evaluation: |%s|\n", routeDescription + i);
#endif
				ret += WH_DymRoute_route_createFunc(f, routeDescription + i, wms);

				rrCaseId++;
				fprintf(f, "%s", _WH_DymRoute_CC_RRbreak);
			}

			parentesys++;

		} else if (parentesys == 0 && (routeDescription[i] >= '0' && routeDescription[i] <= '9')) {
			nextNode = atoi((char *)(routeDescription + i));

			while (routeDescription[i] >= '0' && routeDescription[i] <= '9') {
				i++;
			}

			DestinationWorm *worm = WH_addWorm(wms, nextNode, 1);

			if (worm) {

				fprintf(f, _WH_DymRoute_CC_RRcase, rrCaseId);
				//RR
				fprintf(f, _WH_DymRoute_CC_setDw, WH_findWormIndex(wms, nextNode));
				fprintf(f, "%s", _WH_DymRoute_CC_send);

#ifdef _DYM_ROUTE_DEBUG_
				fprintf(stderr, "ROUTEDEBUG: %d in RR list\n", nextNode);
#endif

				rrCaseId++;
				fprintf(f, "%s", _WH_DymRoute_CC_RRbreak);

			} else {
				return 77;    //some random error code
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
uint8_t WH_DymRoute_route_createFuncCat(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms)
{
	uint8_t ret = 0;
	int16_t parentesys = 0;
	uint16_t i = 0;

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

#ifdef _DYM_ROUTE_DEBUG_
				fprintf(stderr, "ROUTEDEBUG: Category switching: %d\n", myCat);
#endif
				fprintf(f, _WH_DymRoute_CC_Catcase, myCat);

			} else if (parentesys == 1) {
#ifdef _DYM_ROUTE_DEBUG_
				fprintf(stderr, "ROUTEDEBUG: Function Evaluation: |%s|\n", routeDescription + i);
#endif
				ret += WH_DymRoute_route_createFunc(f, routeDescription + i, wms);

				fprintf(f, "%s", _WH_DymRoute_CC_Catbreak);
			}

			parentesys++;
		}

		i++;
	}

	fprintf(f, "%s", _WH_DymRoute_CC_Catend);
	return ret;
}


/* Name WH_findWorm
 * Return the worm mached (if no exists)
 */
DestinationWorm *WH_findWorm(DestinationWorms *wms, const uint16_t wormId)
{
	for (size_t i = 0; i < wms->numberOfWorms; i++) {
		if (wms->worms[i].id == wormId) {
			return wms->worms + i;
		}
	}

	return NULL;
}


/* Name WH_findWormIndex
	* Return the worm index in DestinationWorms
	*/
size_t WH_findWormIndex(DestinationWorms *wms, const uint16_t wormId)
{
	for (size_t i = 0; i < wms->numberOfWorms; i++) {
		if (wms->worms[i].id == wormId) {
			return i;
		}
	}

	return 0;
}

/** WH_addWorm
 * @param init Determines if the worm should be initialized or not
 * @return the created connection
 */
DestinationWorm *WH_addWorm(DestinationWorms *wms, const uint16_t wormId, const uint8_t init)
{
	DestinationWorm *worm = NULL;
	worm = WH_findWorm(wms, wormId);

	if (worm) {
		return worm;
	}

	wms->worms = realloc(wms->worms, sizeof(DestinationWorm) * (wms->numberOfWorms + 1));

	worm = calloc(sizeof(DestinationWorm), 1); //TODO REVISAR
	worm->id = wormId;

	if (init) {
		WormSetup wSetup;

		if (WH_getWormData(&wSetup, wormId)) {
#ifdef _DYM_ROUTE_DEBUG_
			fprintf(stderr, "ROUTEaddworm: Worm %d no data retrived\n", wormId);
#endif
			return NULL;
		}

		worm->port = wSetup.listenPort;
		worm->ip = wSetup.IP;
	}

	//TODO: Obtener información de los tipos disponibles.
	worm->numberOfTypes = 0;
	worm->supportedTypes = NULL;
	worm->conns = NULL;

	if (init) {
		WH_connectWorm(worm);
	}

	wms->worms[wms->numberOfWorms] = *worm;

	wms->numberOfWorms++;
	return wms->worms + (wms->numberOfWorms - 1);
}

/************************************************************/