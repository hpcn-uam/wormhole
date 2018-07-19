#include <hptl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

int batch_size, batches, tsleep;
size_t message_size;
std::vector<hptl_t> measurements;

std::mutex send_mutex;

const char *recv_ip;
void *ctx;

void recvthr()
{
	int i, j, rc;
	zmq_msg_t msg;

	void *r = zmq_socket(ctx, ZMQ_PULL);
	if (!r) {
		printf("[%d] error in zmq_socket: %s\n", __LINE__, zmq_strerror(errno));
		exit(-1);
	}

	rc = zmq_connect(r, recv_ip);
	if (rc != 0) {
		printf("[%d] error in zmq_connect (%s): %s\n", __LINE__, recv_ip, zmq_strerror(errno));
		exit(-1);
	}

	rc = zmq_msg_init_size(&msg, message_size);
	if (rc != 0) {
		printf("[%d] error in zmq_msg_init_size: %s\n", __LINE__, zmq_strerror(errno));
		exit(-1);
	}

	send_mutex.unlock();
	for (i = 0; i != batches; i++) {
		for (j = 0; j != batch_size; j++) {
			rc = zmq_recvmsg(r, &msg, 0);
			if (rc < 0) {
				printf("[%d] error in zmq_recvmsg: %s\n", __LINE__, zmq_strerror(errno));
				exit(-1);
			}
			if (zmq_msg_size(&msg) != message_size) {
				printf("[%d] message of incorrect size received (%lu)\n", __LINE__, zmq_msg_size(&msg));
				exit(-1);
			}
			if (j == 0) {
				measurements[i] = hptl_get() - *((hptl_t *)zmq_msg_data(&msg));
			}
		}
	}

	rc = zmq_msg_close(&msg);
	if (rc != 0) {
		printf("[%d] error in zmq_msg_close: %s\n", __LINE__, zmq_strerror(errno));
		exit(-1);
	}

	rc = zmq_close(r);
	if (rc != 0) {
		printf("[%d] error in zmq_close: %s\n", __LINE__, zmq_strerror(errno));
		exit(-1);
	}
}

int main(int argc, char *argv[])
{
	const char *send_ip;
	int rc;
	int i, j;
	void *s;
	zmq_msg_t msg;

	if (argc != 6) {
		printf(
		    "usage: %s <bind-to> <message-size> "
		    "<batch size> <batches> <sleep>\n",
		    argv[0]);
		return 1;
	}
	send_ip      = strdup((std::string(argv[1]) + ":5555").c_str());
	recv_ip      = strdup((std::string(argv[1]) + ":5556").c_str());
	message_size = atoi(argv[2]);
	batch_size   = atoi(argv[3]);
	batches      = atoi(argv[4]);
	tsleep       = atoi(argv[5]);

	send_mutex.lock();
	measurements = std::vector<hptl_t>(batches);

	ctx = zmq_ctx_new();
	// ctx = zmq_init(1);
	if (!ctx) {
		printf("[%d] error in zmq_init: %s\n", __LINE__, zmq_strerror(errno));
		return -1;
	}

	s = zmq_socket(ctx, ZMQ_PUSH);
	if (!s) {
		printf("[%d] error in zmq_socket: %s\n", __LINE__, zmq_strerror(errno));
		return -1;
	}

	rc = zmq_connect(s, send_ip);
	if (rc != 0) {
		printf("[%d] error in zmq_connect (%s): %s\n", __LINE__, send_ip, zmq_strerror(errno));
		return -1;
	}

	std::thread recvr(recvthr);
	send_mutex.lock();
	hptl_init(NULL);

	for (i = 0; i != batches; i++) {
		for (j = 0; j != batch_size; j++) {
			rc = zmq_msg_init_size(&msg, message_size);
			if (rc != 0) {
				printf("[%d] error in zmq_msg_init_size: %s\n", __LINE__, zmq_strerror(errno));
				return -1;
			}
			*((hptl_t *)zmq_msg_data(&msg)) = hptl_get();

			rc = zmq_sendmsg(s, &msg, 0);
			if (rc < 0) {
				printf("[%d] error in zmq_sendmsg: %s\n", __LINE__, zmq_strerror(errno));
				return -1;
			}

			rc = zmq_msg_close(&msg);
			if (rc != 0) {
				printf("[%d] error in zmq_msg_close: %s\n", __LINE__, zmq_strerror(errno));
				return -1;
			}
		}
		hptl_waitns(tsleep);
	}

	recvr.join();

	for (auto m : measurements) {
		printf("%lu ns\n", hptl_ntimestamp(m));
	}

	rc = zmq_close(s);
	if (rc != 0) {
		printf("[%d] error in zmq_close: %s\n", __LINE__, zmq_strerror(errno));
		return -1;
	}

	rc = zmq_ctx_term(ctx);
	if (rc != 0) {
		printf("[%d] error in zmq_ctx_term: %s\n", __LINE__, zmq_strerror(errno));
		return -1;
	}

	return 0;
}