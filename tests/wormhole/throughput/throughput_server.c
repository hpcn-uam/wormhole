#include <common.h>
#include <messages.h>
#include <worm.h>
#include <worm_low.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int message_count;
	int message_size;
	int rc;
	int i, j;

	if (WH_init()) {
		perror("Error WH_init");
		return -1;  // Error initializing libworm
	}
	ConnectionDataType type = {.type = ARRAY, .ext.arrayType = UINT8};
	WH_setup_types(1, &type);

	if (argc != 3) {
		WH_abortf("usage: local_thr <message-size> <message-count>\n");
		return 1;
	}
	message_size  = atoi(argv[1]);
	message_count = atoi(argv[2]);

	void *buffer = calloc(message_size, 1);
	hptl_t begin, end;
	uint64_t elapsed;

	for (i = 0; i != message_count; i++) {
		MessageInfo mi;
		mi.size = message_size;
		mi.type = &type;

		rc = WH_recv(buffer, &mi);
		if (rc != message_size) {
			WH_abortf("error in WH_recv / Message size mismatch (%d/%d)\n", rc, message_size);
		}
		if (i == 0)
			begin = hptl_get();
	}

	end     = hptl_get();
	elapsed = hptl_ntimestamp(end - begin) / 1000;  // to us

	double throughput = ((double)message_count / (double)elapsed * 1000000);
	double megabits   = ((double)throughput * message_size * 8) / 1000000;

	WH_printf("message size: %d [B]\n", (int)message_size);
	WH_printf("message count: %d\n", (int)message_count);
	WH_printf("mean throughput: %d [msg/s]\n", (int)throughput);
	WH_printf("mean throughput: %.3f [Mb/s]\n", (double)megabits);

	WH_halt();

	return 0;
}