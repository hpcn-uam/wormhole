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

int main(int argc, char **argv)
{
	int st = WH_init();

	assert(!memcmp(WH_mySetup.connectionDescription, "1", 29));
	fprintf(stderr, "Éxito setup\n");

	WormSetup otherWorm;



	for (int i = 0; i < 100; i++) {
		WH_getWormData(&otherWorm, 1);
	}

	fprintf(stderr, "Éxito getWormData\n");
}
