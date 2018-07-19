#include <common.h>
#include <messages.h>
#include <worm.h>
#include <worm_low.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ConnectionDataType type = {.type = ARRAY, .ext.arrayType = UINT8};

int main(int argc, char *argv[])
{
	int message_count;
	int message_size;
	int rc;
	int i, j;
	void *s;

	if (WH_init()) {
		perror("Error WH_init");
		return -1;  // Error initializing libworm
	}
	WH_setup_types(1, &type);

	if (argc != 3) {
		WH_abortf("Params: %s <message-size> <message-count>\n", argv[0]);
		return 1;
	}
	message_size  = atoi(argv[1]);
	message_count = atoi(argv[2]);

	void *buffer = calloc(message_size, 1);

	for (i = 0; i != message_count; i++) {
		MessageInfo mi;
		mi.size = message_size;
		mi.type = &type;

		rc = WH_send(buffer, &mi);
		if (rc) {
			WH_abort("error in WH_send\n");
		}
	}

	WH_halt();
	return 0;
}