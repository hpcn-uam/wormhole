#include <worm.h>
#include <worm_private.h>
#include <common.h>

#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <malloc.h>

#define NUM_BIG_MESSAGES 5000000
#define SIZE_BUFFER 1024*4

#define PEAK_FRACTION_DOWN ((NUM_BIG_MESSAGES  )/3)
#define PEAK_FRACTION_UP   (PEAK_FRACTION_DOWN*2)

int main(int argc, char **argv)
{
	ConnectionDataType type[2];
	type[0].type = ARRAY;
	type[0].ext.arrayType = UINT8;
	type[1].type = UINT8;

	WH_setup_types(2, type);

	int st = WH_init();
	assert(st == 0);

	struct timeval start, end;
	struct timeval startPeak, endPeak;

	MessageInfo mi[2];

	mi[0].size = SIZE_BUFFER;
	mi[0].type = type + 0;

	mi[1].size = SIZE_BUFFER;
	mi[1].type = type + 1;

	uint8_t *buffer = calloc(SIZE_BUFFER, 1);

	for (int k = 0; k < 10; k++) {

		if (WH_get_id() == 1) {
			//ENVIA
			gettimeofday(&start, 0);

			for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
				*(int *)buffer = i;
				WH_send((void *)buffer, mi);

				if (i == PEAK_FRACTION_DOWN) {
					gettimeofday(&startPeak, 0);
				}

				if (i == PEAK_FRACTION_UP) {
					gettimeofday(&endPeak, 0);
				}
			}

			WH_flushIO();

			gettimeofday(&end, 0);
			fprintf(stderr, "[ARRAY] %lf gbps. Pico: %lf gpbs\n",
					(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
					(((double)(PEAK_FRACTION_UP - PEAK_FRACTION_DOWN) * SIZE_BUFFER * 8) / 1000) / (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));



		} else { //RECV
			gettimeofday(&start, 0);

			for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
				WH_recv((void *)buffer, mi);

				if (*(int *)buffer != i) {
					fprintf(stderr, "Recv %d esperado %d\n", *(int *)buffer, i);
				}

				if (i > NUM_BIG_MESSAGES - 1000) {
					WH_flushIO();
				}

				if (i == PEAK_FRACTION_DOWN) {
					gettimeofday(&startPeak, 0);
				}

				if (i == PEAK_FRACTION_UP) {
					gettimeofday(&endPeak, 0);
				}

			}

			gettimeofday(&end, 0);
			fprintf(stderr, "[ARRAY] %lf gbps. Pico: %lf gpbs\n",
					(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
					(((double)(PEAK_FRACTION_UP - PEAK_FRACTION_DOWN) * SIZE_BUFFER * 8) / 1000) / (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));

		}

		/**************************************************************************/

		if (WH_get_id() == 1) {
			//ENVIA
			gettimeofday(&start, 0);

			for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
				if (WH_send((void *)buffer, mi + 1)) {
					fprintf(stderr, "Send Error!\n");
				}


				if (i == PEAK_FRACTION_DOWN) {
					gettimeofday(&startPeak, 0);
				}

				if (i == PEAK_FRACTION_UP) {
					gettimeofday(&endPeak, 0);
				}
			}

			WH_flushIO();

			gettimeofday(&end, 0);
			fprintf(stderr, "[BYTES] %lf gbps. Pico: %lf gpbs\n",
					(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
					(((double)(PEAK_FRACTION_UP - PEAK_FRACTION_DOWN) * SIZE_BUFFER * 8) / 1000) / (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));


		} else { //RECV
			gettimeofday(&start, 0);

			for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
				if (WH_recv((void *)buffer, mi + 1) != mi[1].size) {
					fprintf(stderr, "Recv Error!\n");
				}

				if (i > NUM_BIG_MESSAGES - 1000) {
					WH_flushIO();
				}

				if (i == PEAK_FRACTION_DOWN) {
					gettimeofday(&startPeak, 0);
				}

				if (i == PEAK_FRACTION_UP) {
					gettimeofday(&endPeak, 0);
				}
			}

			gettimeofday(&end, 0);
			fprintf(stderr, "[BYTES] %lf gbps. Pico: %lf gpbs\n",
					(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
					(((double)(PEAK_FRACTION_UP - PEAK_FRACTION_DOWN) * SIZE_BUFFER * 8) / 1000) / (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));

		}


	}

	return WH_halt();

}
