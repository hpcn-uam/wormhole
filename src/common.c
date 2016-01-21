#include <common.h>

#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int tcp_connect_to(char *ip, uint16_t port) {
	int sk = socket(AF_INET, SOCK_STREAM, 0);
	if (sk == -1) {
		return -1;
	}

	struct sockaddr_in cli_addr;
	bzero(&cli_addr, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(port);
	cli_addr.sin_addr.s_addr = inet_addr(ip);

	if ((connect(sk, (struct sockaddr *) &cli_addr, sizeof(struct sockaddr_in))) < 0) {
		return -1;
	}

	return sk;
}

int tcp_message_send(int socket, const void *message, size_t len) {
	ssize_t sent = 0;
	ssize_t sent_now;
	do {
		sent_now = send(socket, message + sent, len - sent, MSG_NOSIGNAL);
		sent += sent_now;
	} while (sent != len && sent_now != -1);
	if (sent_now == -1) {
		return 1;
	}
	return 0;
}

int tcp_message_recv(int socket, void *message, size_t len) {
	ssize_t received = 0;
	ssize_t received_now;
	do {
		received_now = recv(socket, message + received, len - received, MSG_NOSIGNAL);
		received += received_now;
	} while (received != len && received_now != -1);
	if (received_now == -1) {
		return 1;
	}
	return 0;
}