#include <common.h>
#include <worm.h>
#include <worm_private.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#include "async_inline.c"
#define _WORMLIB_DEBUG_
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
		return 1;
	}

	// Receive WormSetup
	uint8_t *wormSetupMsg = (uint8_t *) &WH_mySetup;

	if (tcp_message_recv(WH_einsConn.socket, wormSetupMsg, sizeof(WormSetup)) != 0) {
		return 1;
	}

	// Receive connectionDescription
	WH_mySetup.connectionDescription = calloc(WH_mySetup.connectionDescriptionLength + 1, sizeof(char));

	if (tcp_message_recv(WH_einsConn.socket, WH_mySetup.connectionDescription, WH_mySetup.connectionDescriptionLength) != 0) {
		return 1;
	}

	//TypeSetup
	if (WH_myConfig.inputTypes == NULL) {
		fputs("me ejecuté!\n", stderr);
		ConnectionDataType tmpType;
		tmpType.type = ARRAY;
		tmpType.ext.arrayType = UINT8;
		WH_setup_types(1, &tmpType);
	}

	// Lanzar hilo de recepción de conexiones
	if (pthread_create(&WH_wormThread, NULL, WH_thread, NULL)) {
		return 1;
	}

	return 0;
}

/* Name WH_thread
 * A worm Thread listening for info/petitions.
 */
void *WH_thread(void *arg)
{
	int listeningSocket = tcp_listen_on_port(WH_mySetup.listenPort);
	//struct timeval ts;
	//ts.tv_sec  =   0;
	//ts.tv_usec = 250000;

	if (listeningSocket == -1) {
		perror("Error opening socket");
		exit(-1);
	}

	while (1) {
		//poll for incomming connections/requests.
		//int socket = tcp_accept(listeningSocket,&ts);
		int socket = tcp_accept(listeningSocket, NULL);

		if (socket < 0) {
			fputs("Error abriendo socket", stderr);

		} else {
			enum wormMsgType type;

			if (tcp_message_recv(socket, &type, sizeof(type))) {
				fputs("Error abriendo socket", stderr);
				continue;
			}

			switch (type) {
			case HELLO: //Contestamos a Hello
			default:
				type = WORMINFO; //Con Worm Info

				//fputs("Worm Info Pedida\n", stderr);

				if (tcp_message_send(socket, &type, sizeof(type))) {
					perror("Error contestando socket [1]\n");
					close(socket); //cerramos el socket
					continue;
				}

				if (tcp_message_send(socket, &WH_mySetup, sizeof(WH_mySetup))) { //Con wormSetup
					perror("Error contestando socket [2]\n");
					close(socket); //cerramos el socket
					continue;
				}

				if (tcp_message_send(socket, &WH_myConfig, sizeof(WH_myConfig))) { //Con wormConfig
					perror("Error contestando socket [3]\n");
					close(socket); //cerramos el socket
					continue;
				}

				//fprintf(stderr, "Enviando config con size=%zu\n", WH_myConfig.numInputTypes);

				if (tcp_message_send(socket, WH_myConfig.inputTypes, sizeof(ConnectionDataType)*WH_myConfig.numInputTypes)) { //Con wormConfig
					perror("Error contestando socket [4]");
					close(socket); //cerramos el socket
					continue;
				}

				//fputs("Worm Info Respondida\n", stderr);

				close(socket); //cerramos el socket
				break;

			case SETUPWORMCONN: { //Establecemos una conexión completa con otro worm
				DestinationWorm tmpDestWorm;

				fputs("Worm setup Pedido\n", stderr);

				//Recibimos un destinationWorm
				if (tcp_message_recv(socket, &tmpDestWorm, sizeof(DestinationWorm))) {
					fputs("Error configurando socket", stderr);
					close(socket); //cerramos el socket
					continue;
				}

				DestinationWorm *tmpDestWormPtr
					= WH_addWorm(&WH_myRcvWorms, tmpDestWorm.id);

				tmpDestWormPtr->ip = tmpDestWorm.ip;
				tmpDestWormPtr->port = tmpDestWorm.port;
				tmpDestWormPtr->numberOfTypes++;
				tmpDestWormPtr->supportedTypes = realloc(tmpDestWormPtr->supportedTypes,
												 tmpDestWormPtr->numberOfTypes * sizeof(ConnectionDataType));
				tmpDestWormPtr->conns = realloc(tmpDestWormPtr->conns,
												tmpDestWormPtr->numberOfTypes * sizeof(Connection));

				if (tcp_message_recv(socket, &(tmpDestWormPtr->conns[tmpDestWormPtr->numberOfTypes - 1]), sizeof(Connection))) {
					fputs("Error configurando socket", stderr);
					close(socket); //cerramos el socket
					continue;
				}

				socket_upgrade_to_async(&(tmpDestWormPtr->conns[tmpDestWormPtr->numberOfTypes - 1].socket), socket);

#ifdef _WORMLIB_DEBUG_
				fprintf(stderr, "Conexión entrante en worm! Id nodo conectante: %d\n",
						WH_myRcvWorms.worms[WH_myRcvWorms.numberOfWorms].id);
#endif

				fputs("Worm setup finalizado\n", stderr);

				break;
			}
			}

		}
	}

	return NULL;
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
	}

	while (connIndex >= wms->worms[wormIndex].numberOfTypes) {
		wormIndex = (wormIndex + 1) % wms->numberOfWorms;
		connIndex = 0;
	}

	uint32_t startingWormIndex = wormIndex;
	uint32_t startingConnIndex = connIndex;

	while (wormIndex != startingWormIndex && connIndex != startingConnIndex) {
		if (can_be_read(&(wms->worms[wormIndex].conns[connIndex].socket))) {
			return &wms->worms[wormIndex].conns[connIndex];
		}

		connIndex++;

		while (connIndex >= wms->worms[wormIndex].numberOfTypes) {
			wormIndex = (wormIndex + 1) % wms->numberOfWorms;
			connIndex = 0;
		}
	}

	return 0;

}

/* Name WH_connectWorm
 * Connect and fill the socket data.
 * Return 0 if OK, something else if error.
 */
uint8_t WH_connectWorm(DestinationWorm *c)
{
	struct in_addr ip_addr;
	ip_addr.s_addr = c->ip;

	int socket = tcp_connect_to(inet_ntoa(ip_addr), c->port);

	if (socket == -1) {
		perror("Error solicitando información del Worm [0]");
		return 1;
	}

	//Solicitamos datos...
	enum wormMsgType type = HELLO; //Con Worm Info

	WormSetup wormSetup;
	WormConfig wormConfig = {0};

	if (tcp_message_send(socket, &type, sizeof(type))) {
		perror("Error solicitando información del Worm [1]");
		close(socket);
		return 1;
	}

	if (tcp_message_recv(socket, &type, sizeof(type))) {
		perror("Error solicitando información del Worm [1]");
		close(socket);
		return 1;
	}

	if (type != WORMINFO) {
		fprintf(stderr, "Respuesta de worm no esperada...\n");
	}

	if (tcp_message_recv(socket, &wormSetup, sizeof(wormSetup))) { //Con wormSetup
		perror("Error solicitando información del Worm [2]");
		close(socket);
		return 1;
	}

	if (tcp_message_recv(socket, &wormConfig, sizeof(wormConfig))) { //Con wormConfig
		perror("Error solicitando información del Worm [3]");
		close(socket);
		return 1;
	}

	wormConfig.inputTypes = realloc(c->supportedTypes, sizeof(ConnectionDataType) * wormConfig.numInputTypes);

	if (tcp_message_recv(socket, wormConfig.inputTypes, sizeof(ConnectionDataType)*wormConfig.numInputTypes)) { //Con wormConfig
		perror("Error solicitando información del Worm [4]");
		close(socket);
		return 1;
	}

	close(socket);

	//Rellenamos el worm entrante
	c->id = wormSetup.id;
	c->conns = NULL;
	c->numberOfTypes = wormConfig.numInputTypes;
	c->supportedTypes = wormConfig.inputTypes;
	//FIXME en caso de que se reduzca el numero de conexiones
	c->conns = realloc(c->conns, sizeof(Connection) * wormConfig.numInputTypes);

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

	int socket = tcp_connect_to(inet_ntoa(ip_addr), dw->port);

	if (socket == -1) {
		return 1;
	}

	WH_connectWorm(dw);
	int8_t flag = 0;

	for (int i = 0; i < dw->numberOfTypes; i++) {
		if (!memcmp(dw->supportedTypes + i, type, sizeof(ConnectionDataType))) {
			flag = 1;
			break;
		}
	}

	if (!flag) {
#ifdef _WORMLIB_DEBUG_
		fprintf(stderr, "No se puede abrir un socket con el datatype solicitado!\n");
#endif
		return -1;
	}

	//Informamos del tipo de conexion...
	enum wormMsgType msgtype = SETUPWORMCONN; //Con Worm config

	if (tcp_message_send(socket, &msgtype, sizeof(type))) {
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

	if (tcp_message_send(socket, &dwtmp, sizeof(dwtmp))) { //DstWorm
		fprintf(stderr, "Error configurando Worm externo %d\n", dw->id);
		return 1;
	}

	Connection conntmp;
	conntmp.type = *type;

	if (tcp_message_send(socket, &conntmp, sizeof(conntmp))) { //DstWorm
		fprintf(stderr, "Error configurando Worm externo %d\n", dw->id);
		return 1;
	}

	for (int i = 0; i < dw->numberOfTypes; i++) {
		if (!memcmp(dw->supportedTypes + i, type, sizeof(ConnectionDataType))) {

			socket_upgrade_to_async(&(dw->conns[i].socket), socket);

			break;
		}
	}


#ifdef _WORMLIB_DEBUG_
	fprintf(stderr, "Conexión saliente a worm! Id nodo conectado: %d\n", dw->id);
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

	if (tcp_message_recv(WH_einsConn.socket, (uint8_t *) &ctrlMsg, sizeof(enum ctrlMsgType)) != 0) {
		return 1;
	}

	if (ctrlMsg != CTRL_OK) {
		return 1;
	}

	if (tcp_message_recv(WH_einsConn.socket, (uint8_t *) ws, sizeof(WormSetup)) != 0) {
		return 1;
	}

	return 0;
}

/************************************************************
	Dynamic Routing Library
*************************************************************/
#define _DYM_ROUTE_DEBUG_
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
	return WH_DymRoute_route(data, mi, &WH_myDstWorms);
}
/* Name WH_DymRoute_route
* Enrute a message
* Return 0 if OK, something else if error.
*/
uint8_t WH_DymRoute_route(const void *const data, const MessageInfo *const mi, DestinationWorms *wms)
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
		return -1;
	}

	sprintf(tmpString, "/tmp/%d.c", myPid);
	FILE *f = fopen(tmpString, "w+");

	if (!f) {
		free(tmpString);
		return -2;
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
		sprintf(tmpString, "gcc -O3 -Wall /tmp/%d.c -o /tmp/%d.so -shared -lpthread -fPIC ", myPid, myPid);
		ret = system(tmpString);
	}

	/*Link the .SO*/
	if (!ret) {
		sprintf(tmpString, "/tmp/%d.so", myPid);
		_WH_DymRoute_libHandle = dlopen(tmpString, RTLD_NOW);

		if (!_WH_DymRoute_libHandle) {
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
	for (int i = 0; i < dw->numberOfTypes; i++) {
		if (!memcmp(dw->supportedTypes + i, mi->type, sizeof(ConnectionDataType))) {
			//tcp_message_send_async(AsyncSocket *sock, const void *message, size_t len)
			if (&(dw->conns[i]) == NULL) {
				if (WH_setupConnectionType((DestinationWorm *)dw, mi->type)) {
#ifdef _DYM_ROUTE_DEBUG_
					fprintf(stderr, "ROUTEDEBUG: sending data to worm: %d [FAIL-SETUP]\n", dw->id);
#endif
					return -1;
				}
			}

#ifdef _DYM_ROUTE_DEBUG_
			fprintf(stderr, "ROUTEDEBUG: sending data to worm: %d\n", dw->id);
#endif
			return tcp_message_send_async(&(dw->conns[i].socket), data, mi->size);
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

			DestinationWorm *worm = WH_addWorm(wms, nextNode);

			if (worm) {
				fprintf(f, _WH_DymRoute_CC_setDw, WH_findWormIndex(wms, nextNode));
				fprintf(f, _WH_DymRoute_CC_send);

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
		fprintf(stderr, "ROUTEDEBUG: Calling DUP...\n");
		return WH_DymRoute_route_create(f, routeDescription + i, wms);

	case 'r':
	case 'R':
		// Round Robin Function
		fprintf(stderr, "ROUTEDEBUG: Calling RR...\n");
		return WH_DymRoute_route_createFuncRR(f, routeDescription + i, wms);

	case 'c':
	case 'C':
		// Category Function
		fprintf(stderr, "ROUTEDEBUG: Calling CAT...\n");
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
				fprintf(f, _WH_DymRoute_CC_RRbreak);
			}

			parentesys++;

		} else if (parentesys == 0 && (routeDescription[i] >= '0' && routeDescription[i] <= '9')) {
			nextNode = atoi((char *)(routeDescription + i));

			while (routeDescription[i] >= '0' && routeDescription[i] <= '9') {
				i++;
			}

			DestinationWorm *worm = WH_addWorm(wms, nextNode);

			if (worm) {

				fprintf(f, _WH_DymRoute_CC_RRcase, rrCaseId);
				//RR
				fprintf(f, _WH_DymRoute_CC_setDw, WH_findWormIndex(wms, nextNode));
				fprintf(f, _WH_DymRoute_CC_send);

#ifdef _DYM_ROUTE_DEBUG_
				fprintf(stderr, "ROUTEDEBUG: %d in RR list\n", nextNode);
#endif

				rrCaseId++;
				fprintf(f, _WH_DymRoute_CC_RRbreak);

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

	fprintf(f, _WH_DymRoute_CC_Catswitch);

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

				fprintf(f, _WH_DymRoute_CC_Catbreak);
			}

			parentesys++;
		}

		i++;
	}

	fprintf(f, _WH_DymRoute_CC_Catend);
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


/* Name WH_addWorm
	* Return the created connection
	*/
DestinationWorm *WH_addWorm(DestinationWorms *wms, const uint16_t wormId)
{
	DestinationWorm *worm = NULL;
	worm = WH_findWorm(wms, wormId);

	if (worm) {
		return worm;
	}

	wms->numberOfWorms++;
	wms->worms = realloc(wms->worms, sizeof(DestinationWorm) * wms->numberOfWorms);

	worm = calloc(sizeof(DestinationWorm), 1); //TODO REVISAR
	worm->id = wormId;

	WormSetup wSetup;

	if (WH_getWormData(&wSetup, wormId)) {
#ifdef _DYM_ROUTE_DEBUG_
		fprintf(stderr, "ROUTEaddworm: Worm %d no data retrived\n", wormId);
#endif
		return NULL;
	}

	worm->port = wSetup.listenPort;
	worm->ip = wSetup.IP;

	//TODO: Obtener información de los tipos disponibles.
	worm->numberOfTypes = 0;
	worm->supportedTypes = NULL;

	WH_connectWorm(worm);

	wms->worms[wms->numberOfWorms - 1] = *worm;
	return wms->worms + (wms->numberOfWorms - 1);
}

/************************************************************/