#include <worm.h>
#include <worm_private.h>
#include <common.h>

#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <malloc.h>

#define NUM_BIG_MESSAGES 50000000
#define SIZE_BUFFER 1024*4

int main(int argc, char **argv)
{
	int st = WH_init();
	assert(st == 0);

	struct timeval start, end;

	MessageInfo mi;
	ConnectionDataType type;
	type.type = ARRAY;
	type.ext.arrayType = UINT8;

	mi.size = SIZE_BUFFER;
	mi.type = &type;
	mi.category = 1;

	uint8_t *buffer = calloc(SIZE_BUFFER, 1);

	if (WH_get_id() == 1) {
		//ENVIA
		fprintf(stderr, "Comenzando pruebas de enviar valores grandes\n");

		gettimeofday(&start, 0);

		for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
			WH_send((void *)buffer, &mi);
		}

		gettimeofday(&end, 0);
		//destroy_asyncSocket(&sock);
		fprintf(stderr, "Terminadas pruebas. %f gbps\n",
				(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)));


	} else { //RECV
		fprintf(stderr, "Comenzando pruebas de recibir valores grandes\n");

		gettimeofday(&start, 0);

		for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
			WH_recv((void *)buffer, &mi);

			if (i > NUM_BIG_MESSAGES - 1000) {
				WH_flushIO();
			}

		}

		gettimeofday(&end, 0);
		//destroy_asyncSocket(&sock);
		fprintf(stderr, "Terminadas pruebas. %f gbps\n",
				(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)));

	}


	return WH_halt();

}
