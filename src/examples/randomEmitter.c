#include <common.h>
#include <worm.h>
#include <worm_private.h>

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include <malloc.h>

#define NUM_BIG_MESSAGES 2000000

#define PEAK_FRACTION_DOWN ((NUM_BIG_MESSAGES) / 3)
#define PEAK_FRACTION_UP (PEAK_FRACTION_DOWN * 2)

#define NUMNODES 1
#define RRNODES 1  // Min value=1

//#define CHECKMSG

int main(int argc, char **argv)
{
	int c;
	unsigned msgSize        = 1024 * 4;
	ConnectionDataType type = {.type = UINT8, .ext.arrayType = UINT8};

	int st = WH_init();
	assert(st == 0);

	while ((c = getopt(argc, argv, "t:s:h")) != -1) {
		switch (c) {
			case 't':  // type
				if (optarg[0] == 'a') {
					type.type          = ARRAY;
					type.ext.arrayType = UINT8;
				}

				break;

			case 's': {  // size
				int intreaded = atoi(optarg);

				if (intreaded < 1) {
					return WH_abort("Error! size is too small\n");

				} else {
					msgSize = (unsigned)intreaded;
				}

				break;
			}

			case 'h': {
				char tmpmsg[4096];
				sprintf(tmpmsg, "Use: ./%s [-s <msg size>] [-t <type (a=array, else uint8)>]\n", argv[0]);
				return WH_abort(tmpmsg);
			}

			case '?': {
				char tmpmsg[4096];

				if (optopt == 's' || optopt == 't') {
					sprintf(tmpmsg, "Option -%c requires an argument.\n", optopt);

				} else {
					sprintf(tmpmsg, "Unknown option `-%c'.\n", optopt);
				}

				return WH_abort(tmpmsg);
			}

			default:
				return WH_abort(NULL);
		}
	}

	WH_setup_types(1, &type);

	struct timeval start, end;
	struct timeval startPeak, endPeak;

	MessageInfo mi;

	mi.size     = msgSize;
	mi.type     = &type;
	mi.category = 1;

	uint8_t *buffer = calloc(msgSize, 1);

	for (;;) {
		gettimeofday(&start, 0);

		for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
#ifdef CHECKMSG
			*(int *)buffer = i;
#endif
			WH_send((void *)buffer, &mi);

			if (i == PEAK_FRACTION_DOWN) {
				gettimeofday(&startPeak, 0);
			}

			if (i == PEAK_FRACTION_UP) {
				gettimeofday(&endPeak, 0);
			}
		}

		// WH_flushIO();

		gettimeofday(&end, 0);
		fprintf(stderr,
		        "%lf gbps. Pico: %lf gpbs\n",
		        NUMNODES * (((double)NUM_BIG_MESSAGES * msgSize * 8) / 1000) /
		            (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
		        NUMNODES * (((double)(PEAK_FRACTION_UP - PEAK_FRACTION_DOWN) * msgSize * 8) / 1000) /
		            (((double)endPeak.tv_sec - startPeak.tv_sec) * 1000000 + (endPeak.tv_usec - startPeak.tv_usec)));
	}

	return WH_halt();
}