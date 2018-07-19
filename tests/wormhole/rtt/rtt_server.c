#include <common.h>
#include <messages.h>
#include <worm.h>
#include <worm_low.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int batch_size, batches;
	size_t message_size;
	int rc;
	int i, j;

	if (WH_init()) {
		perror("Error WH_init");
		return -1;  // Error initializing libworm
	}
	ConnectionDataType type = {.type = ARRAY, .ext.arrayType = UINT8};
	WH_setup_types(1, &type);
	if (argc != 4) {
		WH_abortf("Params: %s <message-size> <batch size> <batches>\n", argv[0]);
		return 1;
	}
	message_size = atoi(argv[1]);
	batch_size   = atoi(argv[2]);
	batches      = atoi(argv[3]);

	void *buffer = calloc(message_size, 1);

	for (i = 0; i != batches; i++) {
		for (j = 0; j != batch_size; j++) {
			MessageInfo mi;
			mi.size = message_size;
			mi.type = &type;

			rc = WH_recv(buffer, &mi);
			if (rc != message_size) {
				WH_abortf("error in WH_recv / Message size mismatch (%d/%d)\n", rc, message_size);
			}

			rc = WH_send(buffer, &mi);
			if (rc) {
				WH_abort("error in WH_send\n");
			}
		}
	}

	while (1) {
		MessageInfo mi;
		mi.size = message_size;
		mi.type = &type;

		rc = WH_recv(buffer, &mi);
		if (errno == ENOTCONN) {
			break;
		}

		rc = WH_send(buffer, &mi);
		if (rc) {
			WH_abort("error in WH_send\n");
		}
	}

	WH_halt();

	return 0;
}