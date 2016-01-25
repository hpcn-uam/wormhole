#include <common.h>

#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>

int tcp_connect_to (char *ip, uint16_t port)
{
	int sockfd = socket (AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		return -1;
	}

	struct sockaddr_in cli_addr;

	bzero (&cli_addr, sizeof (cli_addr));

	cli_addr.sin_family = AF_INET;

	cli_addr.sin_port = htons (port);

	cli_addr.sin_addr.s_addr = inet_addr (ip);

	if (connect (sockfd, (struct sockaddr *) &cli_addr, sizeof (struct sockaddr_in)) < 0) {
		return -1;
	}

	return sockfd;
}

int tcp_listen_on_port (uint16_t port)
{
	int sockfd;
	struct sockaddr_in serv_addr;

	sockfd = socket (AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		perror ("socket");
		return -1;
	}

	int yes = 1;

	if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes)) == -1) {
		perror ("setsockopt");
		return -1;
	}

	bzero ( (char *) &serv_addr, sizeof (serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons (port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind (sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) != 0) {
		perror ("bind");
		return -1;
	}

	if (listen (sockfd, 5) != 0) {
		perror ("listen");
		return -1;
	}

	return sockfd;
}

int tcp_accept (int listen_socket)
{
	// TODO: Timeout
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof (cli_addr);
	int newsockfd = accept (listen_socket, (struct sockaddr *) &cli_addr, &clilen);

	return newsockfd;
}

int tcp_message_send (int socket, const void *message, size_t len)
{
	ssize_t sent = 0;
	ssize_t sent_now;

	do {
		sent_now = send (socket, message + sent, len - sent, MSG_NOSIGNAL);
		sent += sent_now;
	} while (sent != len && sent_now != -1);

	if (sent_now == -1) {
		return 1;
	}

	return 0;
}

int tcp_message_recv (int socket, void *message, size_t len)
{
	ssize_t received = 0;
	ssize_t received_now;

	do {
		received_now = recv (socket, message + received, len - received, MSG_NOSIGNAL);
		received += received_now;
	} while (received != len && received_now != -1);

	if (received_now == -1) {
		return 1;
	}

	return 0;
}