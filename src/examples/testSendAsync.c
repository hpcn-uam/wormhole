#include <stdio.h>
#include <common.h>
#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <strings.h>

#define NUM_SMALL_MESSAGES 1000000000
#define NUM_BIG_MESSAGES 1000
#define SIZE_BUFFER 1024*1024*50

int main(int argc, char **argv) {
	void *buffer = calloc(SIZE_BUFFER, sizeof(uint8_t));
	
	AsyncSocket sock;
	int st = tcp_connect_to_async("127.0.0.1", 5000, &sock, 1024*1024*100);
	
	assert(st == 0);
	
	bzero(sock.buff[0], 1024*1024*100);
	bzero(sock.buff[1], 1024*1024*100);

	struct timeval start, end;	
	
	fprintf(stderr, "Comenzando pruebas de enviar valores pequeños\n");
	
	uint64_t value = 0;
	gettimeofday(&start, 0);
	for (int i = 0; i < NUM_SMALL_MESSAGES; i++) {
		tcp_message_send_async(&sock, (void *)&value, sizeof(uint64_t));
	}
	gettimeofday(&end, 0);
	
	fprintf(stderr, "Terminadas pruebas. %f gbps, %f segundos\n",
	((double)NUM_SMALL_MESSAGES*sizeof(uint64_t)*8/1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
	(((double)end.tv_sec - start.tv_sec) + ((double)end.tv_usec - start.tv_usec)/1000000));

	
	
	
	fprintf(stderr, "Comenzando pruebas de enviar valores grandes\n");
	

	gettimeofday(&start, 0);
	for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
		tcp_message_send_async(&sock, (void *)buffer, SIZE_BUFFER);
	}
	gettimeofday(&end, 0);
	
	fprintf(stderr, "Terminadas pruebas. %f gbps\n",
	((double)NUM_BIG_MESSAGES*SIZE_BUFFER*8/1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)));

}
