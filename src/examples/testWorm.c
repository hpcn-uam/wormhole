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

int main (int argc, char **argv) {
	int st = WH_init();
	assert(st == 0);
	
	assert(WH_myId == 1);
	
	assert(WH_einsConn.Port == 5000);
	assert(WH_einsConn.IP == inet_addr("127.0.0.1"));
	
	assert(WH_mySetup.id == 1);
	assert(WH_mySetup.listenPort == 10000);
	assert(WH_mySetup.core == 0);
	assert(WH_mySetup.IP == inet_addr("127.0.0.1"));
	assert(WH_mySetup.connectionDescriptionLength == 29);
	assert( !memcmp(WH_mySetup.connectionDescription, "(LISP connection description)", 29) );
	fprintf(stderr, "Éxito\n");
}