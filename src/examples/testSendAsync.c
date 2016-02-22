#include <stdio.h>
#include <common.h>
#include "../async_inline.c"
#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <strings.h>
#include <malloc.h>

#define NUM_SMALL_MESSAGES 500000000
#define NUM_BIG_MESSAGES 500000
#define SIZE_BUFFER 1024*4 

int main(int argc, char **argv)
{
	void *buffer;

	if (posix_memalign(&buffer, 4096, SIZE_BUFFER)) {
		fputs("error muy feo en reserva de memoria", stderr);
	}

	AsyncSocket sock;
	int st = tcp_connect_to_async("127.0.0.1", 5000, &sock);

	assert(st == 0);


	struct timeval start, end;

	fprintf(stderr, "Comenzando pruebas de enviar valores pequeños\n");

	uint64_t value = 0;
	gettimeofday(&start, 0);

	for (int i = 0; i < NUM_SMALL_MESSAGES; i++) {
		value = i;
		tcp_message_send_async(&sock, (void *)&value, sizeof(uint64_t));
	}

	gettimeofday(&end, 0);


	fprintf(stderr, "Terminadas pruebas. %f gbps, %f segundos\n",
			((double)NUM_SMALL_MESSAGES * sizeof(uint64_t) * 8 / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
			(((double)end.tv_sec - start.tv_sec) + ((double)end.tv_usec - start.tv_usec) / 1000000));





	fprintf(stderr, "Comenzando pruebas de enviar valores grandes\n");


	gettimeofday(&start, 0);

	for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
		*((int *) buffer) = i;
		tcp_message_send_async(&sock, (void *)buffer, SIZE_BUFFER);
		//tcp_message_send(sock.sockfd, (void *)buffer, SIZE_BUFFER);
	}

	gettimeofday(&end, 0);

	//destroy_asyncSocket(&sock);

	fprintf(stderr, "Terminadas pruebas. %f gbps\n",
			(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)));

}
