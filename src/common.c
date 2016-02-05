#include <common.h>

#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

size_t current_send_buf = 0;

int tcp_connect_to(char *ip, uint16_t port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		return -1;
	}

	struct sockaddr_in cli_addr;

	bzero(&cli_addr, sizeof(cli_addr));

	cli_addr.sin_family = AF_INET;

	cli_addr.sin_port = htons(port);

	cli_addr.sin_addr.s_addr = inet_addr(ip);

	if (connect(sockfd, (struct sockaddr *) &cli_addr, sizeof(struct sockaddr_in)) < 0) {
		return -1;
	}

	return sockfd;
}

int tcp_listen_on_port(uint16_t port)
{
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		perror("socket");
		return -1;
	}

	int yes = 1;

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		perror("setsockopt");
		return -1;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		perror("bind");
		return -1;
	}

	if (listen(sockfd, 5) != 0) {
		perror("listen");
		return -1;
	}

	return sockfd;
}

int tcp_accept(int listen_socket, struct timeval *timeout)
{
	//TIMEOUT
	if (timeout) {
		if (setsockopt(listen_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)timeout,
					   sizeof(struct timeval)) < 0) {
			fputs("setsockopt failed\n", stderr);
		}

		if (setsockopt(listen_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)timeout,
					   sizeof(struct timeval)) < 0) {
			fputs("setsockopt failed\n", stderr);
		}
	}

	// TODO: Timeout
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = accept(listen_socket, (struct sockaddr *) &cli_addr, &clilen);

	return newsockfd;
}

int tcp_message_send(int socket, const void *message, size_t len)
{
	ssize_t sent = 0;
	ssize_t sent_now;

	do {
		sent_now = send(socket, message + sent, len - sent, MSG_NOSIGNAL);
		sent += sent_now;
	} while (sent != len && sent_now != -1 && sent_now != 0);

	if (sent_now == -1 || sent_now == 0) {
		return 1;
	}

	return 0;
}

int tcp_message_recv(int socket, void *message, size_t len)
{
	ssize_t received = 0;
	ssize_t received_now;

	do {
		received_now = recv(socket, message + received, len - received, MSG_NOSIGNAL);
		received += received_now;
	} while (received != len && received_now != -1 && received_now != 0);

	if (received_now == -1 || received_now == 0) {
		return 1;
	}

	return 0;
}

typedef void *(*async_fun_p)(void *);


void *send_fun(void *args)
{
	AsyncSocket *sock = (AsyncSocket *) args;

	size_t current_buf = 0;

	for (;; current_buf = (current_buf + 1) % 2) {
		int writing = 0;
		
		// Wait until the buffer can be sent
		do {
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 100;
			nanosleep(&ts, 0);
			pthread_spin_lock(&(sock->lock));
			if (sock->to_access[current_buf]) {
				writing = 1;
			}
			pthread_spin_unlock(&(sock->lock));
		} while (!writing);
		
		tcp_message_send(sock->sockfd, sock->buff[current_buf], sock->write_pos[current_buf]);
		
		pthread_spin_lock(&(sock->lock));
		sock->to_access[current_buf] = 0;
		pthread_spin_unlock(&(sock->lock));

	}
}


void *recv_fun(void *args)
{
	AsyncSocket *sock = (AsyncSocket *) args;

	size_t current_buf = 0;

	for (;;) {
		tcp_message_recv(sock->sockfd, sock->buff[current_buf] + sock->write_pos[current_buf], sock->buf_len - sock->write_pos[current_buf]);

		pthread_spin_lock(&(sock->lock));
		sock->to_access[current_buf] = 1;
		sock->write_pos[current_buf] = sock->buf_len;

		current_buf = (current_buf + 1) % 2;

		// Wait until the buffer has been sent
		while (sock->to_access[current_buf]) {
			pthread_spin_unlock(&(sock->lock));	
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 100;
			nanosleep(&ts, 0);
			pthread_spin_lock(&(sock->lock));
		}

		pthread_spin_unlock(&(sock->lock));

		sock->write_pos[current_buf] = 0;

	}
}


int init_asyncSocket(AsyncSocket *sock, size_t buf_len, async_fun_p async_fun)
{
	sock->buf_len = buf_len;

	sock->read_pos[0] = 0;
	sock->read_pos[1] = 0;

	sock->write_pos[0] = 0;
	sock->write_pos[1] = 0;

	sock->to_access[0] = 0;
	sock->to_access[1] = 0;
	
	sock->can_read = 0;


	sock->buff[0] = malloc(sizeof(uint8_t) * buf_len);

	if (!sock->buff[0]) {
		return 1;
	}

	sock->buff[1] = malloc(sizeof(uint8_t) * buf_len);

	if (!sock->buff[1]) {
		free(sock->buff[0]);
		return 1;
	}
	
	sock->current_send_buf = 0;
	sock->current_recv_buf = 0;

	if (pthread_spin_init(&(sock->lock), 0) != 0) {
		free(sock->buff[0]);
		free(sock->buff[1]);
		return 1;
	}

	pthread_spin_init(&(sock->to_access[0]), 0);
	pthread_spin_init(&(sock->to_access[1]), 0);
	pthread_spin_lock(&(sock->to_access[0]));
	pthread_spin_lock(&(sock->to_access[1]));

	pthread_create(&(sock->thread), 0, async_fun, sock);

	return 0;
}

void destroy_asyncSocket(AsyncSocket *sock)
{
	free(sock->buff[0]);
	free(sock->buff[1]);
	close(sock->sockfd);

	pthread_spin_destroy(&(sock->lock));
}

int tcp_connect_to_async(char *ip, uint16_t port, AsyncSocket *sock, size_t buf_len)
{
	sock->sockfd = tcp_connect_to(ip, port);

	if (sock->sockfd == -1) {
		return 1;
	}

	if (init_asyncSocket(sock, buf_len, send_fun) != 0) {
		close(sock->sockfd);
		return 1;
	}

	return 0;
}

int tcp_accept_async(int listen_socket, AsyncSocket *sock, size_t buf_len)
{
	sock->sockfd = tcp_accept(listen_socket, NULL);

	if (sock->sockfd == -1) {
		return 1;
	}

	if (init_asyncSocket(sock, buf_len, recv_fun) != 0) {
		close(sock->sockfd);
		return 1;
	}

	return 0;
}
