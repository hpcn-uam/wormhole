#include <worm.h>
#include <worm_private.h>
#include <common.h>

#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

extern Worm2EinsConn WH_einsConn;
extern uint16_t WH_myId;
extern WormSetup WH_mySetup;
extern DestinationWorms WH_myDstWorms;

#define TESTLIST "1"
#define TESTDATA "1234567890"

int main(int argc, char **argv)
{
	int st = WH_init();
	assert(st == 0);

	assert(!memcmp(WH_mySetup.connectionDescription, TESTLIST, strlen(TESTLIST)));
	fprintf(stderr, "Éxito setup\n");

	st = WH_DymRoute_init(WH_mySetup.connectionDescription, &WH_myDstWorms);
	assert(st == 0);
	fprintf(stderr, "Éxito Configurando enrutado\n");

	MessageInfo mi;
	ConnectionDataType type;
	type.type = ARRAY;
	type.ext.arrayType = INT8;

	mi.size = strlen(TESTDATA) + 1;
	mi.type = &type;

	st = WH_send(TESTDATA, &mi);
	assert(st == 0);
	fprintf(stderr, "Mensajes enrutados!\n");

}
