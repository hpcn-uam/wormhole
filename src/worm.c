#include <common.h>
#include <worm.h>
#include <worm_private.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>


/*
*Global variables
*/
Worm2EinsConn WH_einsConn;
uint16_t WH_myId;
WormSetup WH_mySetup;

DestinationWorms WH_myDstWorms;
/*
*===============
*/


uint8_t WH_init(void)
{

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
	* ((enum ctrlMsgType *) &hellomsg) = HELLOEINSTEIN;
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
	WH_mySetup.connectionDescription = malloc(WH_mySetup.connectionDescriptionLength);

	if (tcp_message_recv(WH_einsConn.socket, WH_mySetup.connectionDescription, WH_mySetup.connectionDescriptionLength) != 0) {
		return 1;
	}

	// TODO: Lanzar hilo de recepción de conexiones

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

/***************************************/
extern uint8_t _binary_obj_structures_h_start;
extern uint8_t _binary_obj_structures_h_end;
const char *_WH_DymRoute_CC_includes = "\n"
									   "#include<stdint.h>\n"
									   "#include<stdio.h>\n";
const char *_WH_DymRoute_CC_FuncStart = "\n\n"
										"uint8_t WH_DymRoute_precompiled_route (const MessageInfo *const mi, DestinationWorms *const cns)\n{\n"
										"int ret = 1;\n";
const char *_WH_DymRoute_CC_FuncEnd = "\n"
									  "return ret;"
									  "}\n";
void *_WH_DymRoute_libHandle;
/***************************************/

/* Name WH_DymRoute_precompiled_route
 * Enrute a message
 * Return the number of msgs sent
 */
uint8_t (*WH_DymRoute_precompiled_route)(const MessageInfo *const mi, DestinationWorms *const cns) = 0;

/* Name WH_send
 * TODO
 * Params:
 * Return 0 if OK, something else if error.
 */
uint8_t WH_send(const void *const data, const MessageInfo *const mi)
{
	return !WH_DymRoute_route(data, mi, &WH_myDstWorms);
}
/* Name WH_DymRoute_route
* Enrute a message
* Return the number of msgs sent
*/
uint8_t WH_DymRoute_route(const void *const data, const MessageInfo *const mi, DestinationWorms *wms)
{
	if (WH_DymRoute_precompiled_route == 0) {
		return 0;

	} else {
		return WH_DymRoute_precompiled_route(mi, &WH_myDstWorms);
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
		sprintf(tmpString, "gcc /tmp/%d.c -o /tmp/%d.so -shared -fPIC ", myPid, myPid);
		ret = system(tmpString);
	}

	if (!ret) {
		ret = WH_DymRoute_route_create(f, routeDescription, wms);
	}

	/*Link the .SO*/
	if (!ret) {
		sprintf(tmpString, "/tmp/%d.so", myPid);
		_WH_DymRoute_libHandle = dlopen(tmpString, RTLD_NOW);

		if (!_WH_DymRoute_libHandle) {
			free(tmpString);
			return -5;
		}

		WH_DymRoute_precompiled_route = dlsym(_WH_DymRoute_libHandle, "WH_DymRoute_precompiled_route");

		if ((errorString = dlerror()) != NULL)  {
			fputs(errorString, stderr);
			ret = -6;
		}
	}

	free(tmpString);
	return ret;
}

/* Name WH_DymRoute_send
 * Sends a message to the network
 * Return 0 if OK, something else if error.
  */
uint8_t WH_DymRoute_send(const void *const data, const MessageInfo *const mi, const Connection *const cn)
{
	return -1;
}

/* Name WH_DymRoute_route_create
 * Starts the Dynamic Routing Library, and setups connection configuration.
 * Return 0 if OK, something else if error.
 */
uint8_t WH_DymRoute_route_create(FILE *f, const uint8_t *const routeDescription, DestinationWorms *wms)
{
	uint8_t ret = 0;
	uint16_t parentesys = 0;
	uint16_t i = 0;
	uint16_t nextNode = 0;

	while (routeDescription[i] != '\0') {
		if (routeDescription[i] == ')') {
			parentesys--;

		} else if (routeDescription[i] == '(') {
			parentesys++;
			ret += WH_DymRoute_route_createFunc(f, routeDescription + i, wms);

		} else if (parentesys == 0 && (routeDescription[i] >= '0' && routeDescription[i] <= '9')) {
			nextNode = atoi((char *)(routeDescription + i));

			while (atoi((char *)(routeDescription + i)) != 0) {
				i++;
			}

			DestinationWorm *worm = WH_findWorm(wms, nextNode);

			if (worm == NULL) {
				puts("TODO");
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
	return 0;
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

/************************************************************/