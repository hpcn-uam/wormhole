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
		UNUSED(argc);
		UNUSED(argv);
		
	int st = WH_init();
	assert(st == 0);

	//assert(WH_myId == 1);

	assert(WH_einsConn.Port == 5000);
	//assert(WH_einsConn.IP == inet_addr("127.0.0.1"));

	//assert(WH_mySetup.id == 1);
	assert(WH_mySetup.listenPort == 10000 + WH_myId);
	assert(WH_mySetup.core == -1);
	assert(WH_mySetup.IP == inet_addr("127.0.0.1"));
	assert(WH_mySetup.connectionDescriptionLength == strlen(TESTLIST));
	assert(!memcmp(WH_mySetup.connectionDescription, TESTLIST, strlen(TESTLIST)));
	fprintf(stderr, "Éxito setup\n");

	WormSetup otherWorm;

	for (int i = 0; i < 1; i++) {
		int id = (i % 3) + 1;
		WH_getWormData(&otherWorm, id);
		assert(otherWorm.id == id);
		assert(otherWorm.listenPort == 10000 + id);
		assert(otherWorm.core == -1);
		assert(otherWorm.IP == inet_addr("127.0.0.1"));
	}

	fprintf(stderr, "Éxito getWormData\n");
}
