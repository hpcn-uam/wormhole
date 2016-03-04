#include <worm.h>
#include <worm_private.h>
#include <common.h>

#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <malloc.h>

#define NUM_BIG_MESSAGES 500000000
#define SIZE_BUFFER 1024*4

#define PEAK_FRACTION_DOWN ((NUM_BIG_MESSAGES  )/3)
#define PEAK_FRACTION_UP   (PEAK_FRACTION_DOWN*2)

#define NUMNODES 4
#define RRNODES 1 //Min value=1

//#define CHECKMSG

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
	mi[0].category = 1;

	mi[1].size = SIZE_BUFFER;
	mi[1].type = type + 1;
	mi[1].category = 1;

	uint8_t *buffer = calloc(SIZE_BUFFER, 1);

	for (int k = 0; k < 10; k++) {

		if (WH_get_id() == 1) {
			//ENVIA
			gettimeofday(&start, 0);

			for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
#ifdef CHECKMSG
				*(int *)buffer = i;
#endif
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
					NUMNODES * (((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
					NUMNODES * (((double)(PEAK_FRACTION_UP - PEAK_FRACTION_DOWN) * SIZE_BUFFER * 8) / 1000) / (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));



		} else { //RECV
			gettimeofday(&start, 0);

			for (int i = 0; i < NUM_BIG_MESSAGES / RRNODES; i++) {
				WH_recv((void *)buffer, mi);

#ifdef CHECKMSG

				if (*(int *)buffer != i) {
					fprintf(stderr, "Recv %d esperado %d\n", *(int *)buffer, i);
				}

#endif

				if (i == PEAK_FRACTION_DOWN / RRNODES) {
					gettimeofday(&startPeak, 0);
				}

				if (i == PEAK_FRACTION_UP / RRNODES) {
					gettimeofday(&endPeak, 0);
				}

			}

			gettimeofday(&end, 0);
			fprintf(stderr, "[ARRAY] %lf gbps. Pico: %lf gpbs\n",
					((((double)NUM_BIG_MESSAGES / RRNODES) * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
					(((double)(PEAK_FRACTION_UP / RRNODES - PEAK_FRACTION_DOWN / RRNODES) * SIZE_BUFFER * 8) / 1000) / (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));

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
					NUMNODES * (((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
					NUMNODES * (((double)(PEAK_FRACTION_UP - PEAK_FRACTION_DOWN) * SIZE_BUFFER * 8) / 1000) / (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));


		} else { //RECV
			gettimeofday(&start, 0);

			for (int i = 0; i < NUM_BIG_MESSAGES / RRNODES; i++) {
				if (WH_recv((void *)buffer, mi + 1) != mi[1].size) {
					fprintf(stderr, "Recv Error!\n");
				}

				if (i == PEAK_FRACTION_DOWN / RRNODES) {
					gettimeofday(&startPeak, 0);
				}

				if (i == PEAK_FRACTION_UP / RRNODES) {
					gettimeofday(&endPeak, 0);
				}
			}

			gettimeofday(&end, 0);
			fprintf(stderr, "[BYTES] %lf gbps. Pico: %lf gpbs\n",
					((((double)NUM_BIG_MESSAGES / RRNODES) * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
					(((double)(PEAK_FRACTION_UP / RRNODES - PEAK_FRACTION_DOWN / RRNODES) * SIZE_BUFFER * 8) / 1000) / (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));

		}


	}

	return WH_halt();

}
