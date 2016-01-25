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

#define TESTLIST "1"

int main(int argc, char **argv)
{
	int st = WH_init();
	assert(st == 0);

	assert(!memcmp(WH_mySetup.connectionDescription, TESTLIST, strlen(TESTLIST)));
	fprintf(stderr, "Éxito setup\n");

	WormSetup otherWorm;



	for (int i = 0; i < 100; i++) {
		WH_getWormData(&otherWorm, 1);
	}

	fprintf(stderr, "Éxito getWormData\n");
}
