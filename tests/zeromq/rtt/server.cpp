#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <string>

int main(int argc, char *argv[])
{
	const char *send_ip;
	const char *recv_ip;
	int batch_size, batches;
	size_t message_size;
	void *ctx;
	void *s, *r;
	int rc;
	int i, j;
	zmq_msg_t msg;

	if (argc != 5) {
		printf(
		    "usage: %s <bind-to> <message-size> "
		    "<batch size> <batches> <sleep>\n",
		    argv[0]);
		return 1;
	}
	send_ip      = strdup((std::string(argv[1]) + ":5556").c_str());
	recv_ip      = strdup((std::string(argv[1]) + ":5555").c_str());
	message_size = atoi(argv[2]);
	batch_size   = atoi(argv[3]);
	batches      = atoi(argv[4]);

	ctx = zmq_init(1);
	if (!ctx) {
		printf("error in zmq_init: %s\n", zmq_strerror(errno));
		return -1;
	}

	r = zmq_socket(ctx, ZMQ_PULL);
	if (!r) {
		printf("error in zmq_socket: %s\n", zmq_strerror(errno));
		return -1;
	}

	s = zmq_socket(ctx, ZMQ_PUSH);
	if (!s) {
		printf("error in zmq_socket: %s\n", zmq_strerror(errno));
		return -1;
	}

	rc = zmq_bind(r, recv_ip);
	if (rc != 0) {
		printf("error in zmq_bind: %s\n", zmq_strerror(errno));
		return -1;
	}

	rc = zmq_bind(s, send_ip);
	if (rc != 0) {
		printf("error in zmq_bind: %s\n", zmq_strerror(errno));
		return -1;
	}

	rc = zmq_msg_init(&msg);
	if (rc != 0) {
		printf("error in zmq_msg_init: %s\n", zmq_strerror(errno));
		return -1;
	}

	for (i = 0; i != batches; i++) {
		for (j = 0; j != batch_size; j++) {
			rc = zmq_recvmsg(r, &msg, 0);
			if (rc < 0) {
				printf("error in zmq_recvmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
			// if (zmq_msg_size(&msg) != message_size) {
			// printf("message of incorrect size received\n");
			// return -1;
			// }
			rc = zmq_sendmsg(s, &msg, 0);
			if (rc < 0) {
				printf("error in zmq_sendmsg: %s\n", zmq_strerror(errno));
				return -1;
			}
		}
	}

	rc = zmq_msg_close(&msg);
	if (rc != 0) {
		printf("error in zmq_msg_close: %s\n", zmq_strerror(errno));
		return -1;
	}

	zmq_sleep(1);

	rc = zmq_close(s);
	if (rc != 0) {
		printf("error in zmq_close: %s\n", zmq_strerror(errno));
		return -1;
	}

	rc = zmq_close(r);
	if (rc != 0) {
		printf("error in zmq_close: %s\n", zmq_strerror(errno));
		return -1;
	}

	rc = zmq_ctx_term(ctx);
	if (rc != 0) {
		printf("error in zmq_ctx_term: %s\n", zmq_strerror(errno));
		return -1;
	}

	return 0;
}