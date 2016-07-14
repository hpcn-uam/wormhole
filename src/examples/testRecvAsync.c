#include <stdio.h>
#include <common.h>
#include "../netlib_inline.c"
#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <strings.h>
#include <malloc.h>

#define NUM_SMALL_MESSAGES 500000000
#define NUM_BIG_MESSAGES 5000000
#define NUM_UNALIGNED_MESSAGES 50000000
#define SIZE_BUFFER 1024*4

int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);

	void *buffer = calloc(SIZE_BUFFER, 1);

	int listen_socket = tcp_listen_on_port(5000);
	assert(listen_socket != -1);
	assert(buffer != NULL);


	AsyncSocket sock;
	int ret = 0;

	do {
		ret = tcp_accept_async(listen_socket, &sock, 0, NOSSL, NULL);
	} while (ret);

	struct timeval start, end;

	fprintf(stderr, "Comenzando pruebas de recibir valores pequeños\n");

	uint64_t value;
	gettimeofday(&start, 0);

	for (uint32_t i = 0; i < NUM_SMALL_MESSAGES; i++) {
		if (tcp_message_recv_async(&sock, (void *)&value, sizeof(uint64_t))) {
			return -1;
		}

		if (value != i) {
			fprintf(stderr, "Paquete perdido %d\n", i);
		}
	}

	gettimeofday(&end, 0);

	fprintf(stderr, "Terminadas pruebas. %f gbps\n",
			((double)NUM_SMALL_MESSAGES * sizeof(uint64_t) * 8 / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)));


	fprintf(stderr, "Comenzando pruebas de recibir valores grandes\n");

	gettimeofday(&start, 0);

	for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
		if (tcp_message_recv_async(&sock, (void *)buffer, SIZE_BUFFER)) {
			return -1;
		}

		if (*(int *)buffer != i) {
			fprintf(stderr, "Paquete perdido %d\n", i);
		}
	}

	gettimeofday(&end, 0);

	fprintf(stderr, "Terminadas pruebas. %f gbps\n",
			(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)));


	fprintf(stderr, "Comenzando pruebas de recibir valores sin alineamiento\n");

	gettimeofday(&start, 0);

	for (int i = 0; i < NUM_UNALIGNED_MESSAGES; i++) {
		if (tcp_message_recv_async(&sock, (void *)buffer, SIZE_BUFFER - 5)) {
			return -1;
		}

		if (*(int *)buffer != i) {
			fprintf(stderr, "Paquete perdido %d\n", i);
		}
	}

	gettimeofday(&end, 0);
	destroy_asyncSocket(&sock);
	fprintf(stderr, "Terminadas pruebas. %f gbps\n",
			(((double)NUM_UNALIGNED_MESSAGES * (SIZE_BUFFER - 1) * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)));

}
