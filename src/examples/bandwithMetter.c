#include <worm.h>
#include <worm_private.h>
#include <common.h>

#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <malloc.h>

#define NUM_BIG_MESSAGES 2000000

#define PEAK_FRACTION_DOWN ((NUM_BIG_MESSAGES  )/3)
#define PEAK_FRACTION_UP   (PEAK_FRACTION_DOWN*2)

#define NUMNODES 1
#define RRNODES 1 //Min value=1

//#define CHECKMSG

int main(int argc, char **argv)
{
	int c;
	unsigned msgSize = 1024 * 4;
	ConnectionDataType type = {.type = UINT8, .ext.arrayType = UINT8};

	while ((c = getopt(argc, argv, "st:h")) != -1)
		switch (c) {
		case 't': //type
			if (optarg[0] == 'a') {
				type.type = ARRAY;
				type.ext.arrayType = UINT8;
			}

			break;

		case 's': //size
			msgSize = atoi(optarg);

			if (msgSize < 1) {
				fprintf(stderr, "Error! size is too small\n");
				return 1;
			}

			break;

		case 'h':
			fprintf(stderr, "Use: ./%s [-s <msg size>] [-t <type (a=array, else uint8)>]\n", argv[0]);
			return 0;

		case '?':
			if (optopt == 's' || optopt == 't') {
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);

			} else {
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			}

			return 1;

		default:
			abort();
		}

	WH_setup_types(1, &type);

	int st = WH_init();
	assert(st == 0);

	struct timeval start, end;
	struct timeval startPeak, endPeak;

	MessageInfo mi;

	mi.size = msgSize;
	mi.type = &type;
	mi.category = 1;

	uint8_t *buffer = calloc(msgSize, 1);

	for (;;) {
		gettimeofday(&start, 0);

		for (int i = 0; i < NUM_BIG_MESSAGES / RRNODES; i++) {
			WH_recv((void *)buffer, &mi);

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
		fprintf(stderr, "%lf gbps. Pico: %lf gpbs\n",
				((((double)NUM_BIG_MESSAGES / RRNODES) * msgSize * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
				(((double)(PEAK_FRACTION_UP / RRNODES - PEAK_FRACTION_DOWN / RRNODES) * msgSize * 8) / 1000) / (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));
	}

	return WH_halt();
}
