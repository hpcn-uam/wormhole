#include <common.h>
#include <messages.h>
#include <worm.h>
#include <worm_low.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int batch_size, batches, tsleep;
ConnectionDataType type = {.type = ARRAY, .ext.arrayType = UINT8};
size_t message_size;
hptl_t *measurements;

const char *recv_ip;
void *ctx;

int trashycontents = 1;

static void *recvthr(void *np)
{
	(void)np;
	int i, j, rc;

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

			if (j == 0) {
				measurements[i] = hptl_get() - *((hptl_t *)(buffer));
			}
		}
	}

	trashycontents = 0;
}

int main(int argc, char *argv[])
{
	const char *send_ip;
	int rc;
	int i, j;
	void *s;

	if (WH_init()) {
		perror("Error WH_init");
		return -1;  // Error initializing libworm
	}
	WH_setup_types(1, &type);

	if (argc != 5) {
		WH_abortf("Params: %s <message-size> <batch size> <batches> <sleep>\n", argv[0]);
		return 1;
	}
	message_size = atoi(argv[1]);
	batch_size   = atoi(argv[2]);
	batches      = atoi(argv[3]);
	tsleep       = atoi(argv[4]);

	measurements = calloc(batches, sizeof(hptl_t));

	void *buffer = calloc(message_size, 1);

	pthread_t recvr;
	if (pthread_create(&recvr, NULL, recvthr, NULL)) {
		WH_abortf("Error creating thread\n");
	}

	for (i = 0; i != batches; i++) {
		for (j = 0; j != batch_size; j++) {
			MessageInfo mi;
			mi.size = message_size;
			mi.type = &type;

			*((hptl_t *)buffer) = hptl_get();

			rc = WH_send(buffer, &mi);
			if (rc) {
				WH_abort("error in WH_send\n");
			}
		}
		hptl_waitns(tsleep);
	}

	while (trashycontents) {
		MessageInfo mi;
		mi.size = message_size;
		mi.type = &type;

		*((hptl_t *)buffer) = hptl_get();  // 0;
		WH_send(buffer, &mi);
	}

	// WH_disconnectWorm(1, 3);

	if (pthread_join(recvr, NULL)) {
		WH_abortf("Error joining thread\n");
	}

	for (i = 0; i < batches; i++) {
		hptl_t m = measurements[i];
		printf("%lu ns\n", hptl_ntimestamp(m));
		WH_printf("%lu ns", hptl_ntimestamp(m));
	}

	WH_halt();
	return 0;
}