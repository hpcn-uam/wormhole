#include <common.h>

#include "async_inline.c"

size_t current_send_buf = 0;

uint32_t sslStarted = 0;

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
		close(sockfd);
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

	if (listen(sockfd, 50) != 0) {
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
	} while (sent != (ssize_t)len && sent_now != -1 && sent_now != 0);

	if (sent_now == -1 || sent_now == 0) {
		return sent;
	}

	return 0;
}

size_t tcp_message_recv(int socket, void *message, size_t len, uint8_t sync)
{
	ssize_t received = 0;
	ssize_t received_now;

	do {
		received_now = recv(socket, message + received, len - received, MSG_NOSIGNAL);

		if (received_now > 0) {
			received += received_now;
		}
	} while (received != (ssize_t)len && (
		sync ?
		((received_now == -1 || received_now == 0) && (errno == EAGAIN || errno == EWOULDBLOCK))
		: (received_now != -1 && received_now != 0)));

	return received;
}

//const char *ciphers = "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:RC4-SHA";
const char *ciphers = "RC4-SHA";

/** tcp_upgrade2syncSocket
	 * upgrades a simple socket to SyncSocket
	 * @param socket : the older socket
	 * @param mode : the SSL mode
	 * @param sslConfig : Can be NULL (even with ssl=1). Sets the SSL config. connection.
	 * @return a new SyncSocket
	 */
SyncSocket *tcp_upgrade2syncSocket(int socket, enum syncSocketType mode, SSL_CTX *config)
{
	SyncSocket *ret = malloc(sizeof(SyncSocket));

	if (!ret) {
		return ret;
	}

	if (mode != NOSSL) {
		if (!sslStarted) {
			SSL_load_error_strings();
			SSL_library_init();
			sslStarted = 1;
			fprintf(stderr, "Initializated SSL library\n");
		}

		if (!config) {

			if (mode == SRVSSL) {
				ret->config = SSL_CTX_new(TLSv1_2_server_method());

			} else {
				ret->config = SSL_CTX_new(TLSv1_2_client_method());
			}

			SSL_CTX_set_options(ret->config, SSL_OP_SINGLE_DH_USE);

			if (ret->config == NULL) {
				ERR_print_errors_fp(stderr);
				exit(1);
			}

		} else {
			ret->config = config;
		}

		if (!config) {
			fprintf(stderr, "Loading certificates...");

			if (!SSL_CTX_load_verify_locations(ret->config, "../certs/ca.pem", NULL)) {
				ERR_print_errors_fp(stderr);
				return NULL;
			}

			if (!SSL_CTX_use_certificate_file(ret->config, "../certs/worm.pem", SSL_FILETYPE_PEM)) {
				ERR_print_errors_fp(stderr);
				return NULL;
			}

			if (!SSL_CTX_use_PrivateKey_file(ret->config, "../certs/prv/worm.key.pem", SSL_FILETYPE_PEM)) {
				ERR_print_errors_fp(stderr);
				return NULL;
			}

			/* verify private key */
			if (!SSL_CTX_check_private_key(ret->config)) {
				fprintf(stderr, "Private key does not match the public certificate\n");
				return NULL;
			}

			if (!SSL_CTX_set_cipher_list(ret->config, ciphers)) {
				fprintf(stderr, "No cipher could be selected\n");
				return NULL;
			}

			SSL_CTX_set_verify(ret->config, SSL_VERIFY_PEER, NULL);
			SSL_CTX_set_verify_depth(ret->config, 1);
			fprintf(stderr, "Done!\n");
		}

		ret->tls = SSL_new(ret->config);
		SSL_set_fd(ret->tls, socket);

		if (mode == SRVSSL) {
			fprintf(stderr, "SERVER MODE\n");
			int ssl_err = SSL_accept(ret->tls);

			if (ssl_err <= 0) {
				perror("Error de ssl");
				ERR_print_errors_fp(stderr);
				fprintf(stderr, "SSL_accept error...(%d)\n", ssl_err);
				SSL_shutdown(ret->tls);
				SSL_free(ret->tls);
				return NULL;
			}

		} else {
			fprintf(stderr, "CLIENT MODE\n");
			int ssl_err = SSL_connect(ret->tls);

			if (ssl_err <= 0) {
				perror("Error de ssl");
				ERR_print_errors_fp(stderr);
				fprintf(stderr, "SSL_connect error...(%d)\n", ssl_err);
				SSL_shutdown(ret->tls);
				SSL_free(ret->tls);
				return NULL;
			}
		}

		fprintf(stderr, "Connection established with algorithm: %s\n", SSL_get_cipher_name(ret->tls));

	} else {
		ret->tls = NULL;
		ret->config = NULL;
	}

	ret->sockfd = socket;

	return ret;
}

/** tcp_message_ssend
 * Sends a full message to a socket
 * @return 0 if OK, something else if error.
 */
int tcp_message_ssend(SyncSocket *socket, const void *message, size_t len)
{
	if (socket->config == NOSSL) {
		return tcp_message_send(socket->sockfd, message, len);

	} else {
		ssize_t sent = 0;
		ssize_t sent_now;

		do {
			sent_now = SSL_write(socket->tls, message + sent, len - sent);

			if (sent_now > 0) {
				sent += sent_now;
			}
		} while (sent != (ssize_t)len && sent_now != -1 && sent_now != 0);

		return sent;
	}
}

/** tcp_message_srecv
 * Receives a full message from a socket
 * @return number of bytes read.
 */
size_t tcp_message_srecv(SyncSocket *socket, void *message, size_t len, uint8_t sync)
{
	if (socket->config == NOSSL) {
		return tcp_message_recv(socket->sockfd, message, len, sync);

	} else {
		ssize_t received = 0;
		ssize_t received_now;

		do {
			received_now = SSL_read(socket->tls, message + received, len - received);

			if (received_now > 0) {
				received += received_now;
			}
		} while (received != (ssize_t)len && (
			sync ?
			((received_now == -1 || received_now == 0) && (errno == EAGAIN || errno == EWOULDBLOCK))
			: (received_now != -1 && received_now != 0)));

		return received;
	}
}

/** tcp_sclose
 * Receives a full message from a socket
 * @return number of bytes read.
 */
void tcp_sclose(SyncSocket *socket)
{
	if (socket) {
		if (socket->tls) {
//			tls_close(socket->tls);
//			tls_free(socket->tls);
//			tls_config_free(socket->config);
			SSL_shutdown(socket->tls);
			SSL_free(socket->tls);
		}

		close(socket->sockfd); //TODO check if necesary
		free(socket);
	}
}

/** syncSocketStartSSL
 * Changes the syncsocketMode in order to start a SSL session.
 * @return 0 if ssl has successfully started or 1 if not.
 */
int syncSocketStartSSL(SyncSocket *socket, enum syncSocketType mode, SSL_CTX *sslConfig)
{
	if (!socket->tls) { //If there is no SSL connection
		SyncSocket *newSocket = tcp_upgrade2syncSocket(socket->sockfd, mode, sslConfig);

		if (newSocket) {
			socket->config	= newSocket->config;
			socket->tls		= newSocket->tls;
			//socket->tlsIO	= newSocket->tlsIO;
			free(newSocket);
			return 0;

		} else {
			fprintf(stderr, "Error creating SSL session...\n");
			return 1;
		}

	} else {
		return 1;
	}
}

/************
 * ASYNC LIB *
 ************/

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

			} else if (sock->finish) {
				return 0;
			}

			pthread_spin_unlock(&(sock->lock));
		} while (!writing);

		if (sock->write_pos[current_buf] > 0) {
			tcp_message_ssend(sock->ssock, sock->buff[current_buf], sock->write_pos[current_buf]);
		}

		pthread_spin_lock(&(sock->lock));
		sock->to_access[current_buf] = 0;
		pthread_spin_unlock(&(sock->lock));

	}
}


void *recv_fun(void *args)
{
	AsyncSocket *sock = (AsyncSocket *) args;

	size_t current_buf = 0;
	uint32_t myLoad = 0;
	int32_t received = 0;

	for (;;) {
		do {
			int received_now = tcp_message_srecv(sock->ssock,
												 sock->buff[current_buf] + received,
												 sock->buf_len - received, 0);

			if (received_now > 0) {
				received += received_now;
			}

			pthread_spin_lock(&(sock->lock));

			if (sock->finish) {
				sock->to_access[current_buf] = 1;
				sock->write_pos[current_buf] = received;
				pthread_spin_unlock(&(sock->lock));
				return 0;
			}

			if (received == (int32_t)sock->buf_len || sock->flush) {
				sock->to_access[current_buf] = 1;
				sock->write_pos[current_buf] = received;
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
				received = 0;
				myLoad = 0;

			} else {
				pthread_spin_unlock(&(sock->lock));
				myLoad++;

				if (myLoad > WH_COMMON_LIMITW8_RECV && received > 0) {
					sock->flush = 2;
				}
			}
		} while (!sock->flush);

		myLoad = 0;

		if (sock->flush) {
			sock->flush--;
		}
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

	sock->flush = 0;
	sock->finish = 0;

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

	pthread_create(&(sock->thread), 0, async_fun, sock);

	return 0;
}

void flush_recv(AsyncSocket *sock)
{
	pthread_spin_lock(&(sock->lock));
	sock->flush = 1;
	pthread_spin_unlock(&(sock->lock));
}

void destroy_asyncSocket(AsyncSocket *sock)
{
	if (sock->socket_type == SEND_SOCKET) {
		flush_send(sock);
		flush_send(sock);

	} else {
		flush_recv(sock);
	}

	pthread_spin_lock(&(sock->lock));
	sock->finish = 1;
	pthread_spin_unlock(&(sock->lock));
	pthread_join(sock->thread, 0);
	pthread_spin_destroy(&(sock->lock));
	free(sock->buff[0]);
	free(sock->buff[1]);
	tcp_sclose(sock->ssock);
}

int tcp_connect_to_async(char *ip, uint16_t port, AsyncSocket *sock)
{
	size_t buf_len = OPTIMAL_BUFFER_SIZE;
	int sockfd = tcp_connect_to(ip, port);
	sock->ssock = tcp_upgrade2syncSocket(sockfd, NOSSL, NULL);


	if (sock->ssock == NULL) {
		return 1;
	}

	if (init_asyncSocket(sock, buf_len, send_fun) != 0) {
		tcp_sclose(sock->ssock);
		return 1;
	}

	sock->socket_type = SEND_SOCKET;

	return 0;
}

int tcp_accept_async(int listen_socket, AsyncSocket *sock, struct timeval *timeout)
{
	size_t buf_len = OPTIMAL_BUFFER_SIZE;
	int sockfd = tcp_accept(listen_socket, timeout);
	sock->ssock = tcp_upgrade2syncSocket(sockfd, NOSSL, NULL);

	if (sock->ssock == NULL) {
		return 1;
	}

	if (init_asyncSocket(sock, buf_len, recv_fun) != 0) {
		tcp_sclose(sock->ssock);
		return 1;
	}

	sock->socket_type = RECV_SOCKET;
	return 0;
}

int socket_upgrade_to_async_send(AsyncSocket *async_sock, int sockfd)
{
	size_t buf_len = OPTIMAL_BUFFER_SIZE;
	async_sock->ssock = tcp_upgrade2syncSocket(sockfd, NOSSL, NULL);

	if (init_asyncSocket(async_sock, buf_len, send_fun) != 0) {
		return 1;
	}

	async_sock->socket_type = SEND_SOCKET;
	return 0;
}

int socket_upgrade_to_async_recv(AsyncSocket *async_sock, int sockfd)
{
	size_t buf_len = OPTIMAL_BUFFER_SIZE;
	async_sock->ssock = tcp_upgrade2syncSocket(sockfd, NOSSL, NULL);

	if (init_asyncSocket(async_sock, buf_len, recv_fun) != 0) {
		return 1;
	}

	async_sock->socket_type = RECV_SOCKET;
	return 0;
}


int socket_sync_to_async_send(AsyncSocket *async_sock, SyncSocket *ssock)
{
	size_t buf_len = OPTIMAL_BUFFER_SIZE;
	async_sock->ssock = ssock;

	if (init_asyncSocket(async_sock, buf_len, send_fun) != 0) {
		return 1;
	}

	async_sock->socket_type = SEND_SOCKET;
	return 0;
}

int socket_sync_to_async_recv(AsyncSocket *async_sock, SyncSocket *ssock)
{
	size_t buf_len = OPTIMAL_BUFFER_SIZE;
	async_sock->ssock = ssock;

	if (init_asyncSocket(async_sock, buf_len, recv_fun) != 0) {
		return 1;
	}

	async_sock->socket_type = RECV_SOCKET;
	return 0;
}

/** asyncSocketStartSSL
 * Changes the syncsocketMode in order to start a SSL session.
 * IMPORTANT NOTE: All data must be flushed before call this function, or data-loss can happen.
 * @return 0 if ssl has successfully started or 1 if not.
 */
int asyncSocketStartSSL(AsyncSocket *socket, enum syncSocketType mode, SSL_CTX *sslConfig)
{
	int ret;

	struct timeval lasttimeout = {.tv_sec = 0, .tv_usec = 10};
	struct timeval timeout = {.tv_sec = 0, .tv_usec = 10};
	struct timeval ssltimeout = {.tv_sec = 3, .tv_usec = 0};

	fprintf(stderr, "Stopping thread...\n");
	pthread_spin_lock(&(socket->lock));
	socket->finish = 1;

	if (getsockopt(socket->ssock->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&lasttimeout,
				   NULL) < 0) {
		lasttimeout = timeout;
		lasttimeout.tv_usec = 250000;
	}

	if (setsockopt(socket->ssock->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
				   sizeof(struct timeval)) < 0) {
		fputs("setsockopt failed\n", stderr);
	}

	if (setsockopt(socket->ssock->sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
				   sizeof(struct timeval)) < 0) {
		fputs("setsockopt failed\n", stderr);
	}

	pthread_spin_unlock(&(socket->lock));
	pthread_join(socket->thread, NULL);
	fprintf(stderr, "Stopped...\n");

	if (setsockopt(socket->ssock->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&ssltimeout,
				   sizeof(struct timeval)) < 0) {
		fputs("setsockopt failed\n", stderr);
	}

	if (setsockopt(socket->ssock->sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&ssltimeout,
				   sizeof(struct timeval)) < 0) {
		fputs("setsockopt failed\n", stderr);
	}

	ret = syncSocketStartSSL(socket->ssock, mode, sslConfig);

	socket->finish = 0;

	if (setsockopt(socket->ssock->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&lasttimeout,
				   sizeof(struct timeval)) < 0) {
		fputs("setsockopt failed\n", stderr);
	}

	if (setsockopt(socket->ssock->sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&lasttimeout,
				   sizeof(struct timeval)) < 0) {
		fputs("setsockopt failed\n", stderr);
	}

	if (socket->socket_type == RECV_SOCKET) {
		pthread_create(&(socket->thread), 0, recv_fun, socket);

	} else {
		pthread_create(&(socket->thread), 0, send_fun, socket);
	}

	fprintf(stderr, "Relaunched\n");

	return ret;
}
