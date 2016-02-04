#ifndef __WORM_COMMON_H__
#define __WORM_COMMON_H__

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

	enum ctrlMsgType {
		HELLOEINSTEIN, SETUP, QUERYID, RESPONSEID, PING, PONG, DOWNLINK, OVERLOAD, UNDERLOAD, CTRL_OK, CTRL_ERROR, HALT
	};

	typedef struct {
		uint16_t id;
		uint16_t listenPort;
		int16_t core;
		uint32_t IP; //TODO fix para ipv6
		uint32_t connectionDescriptionLength;
		uint8_t *connectionDescription; // (LISP connection description)
	} WormSetup;

	typedef struct {
		uint16_t id;
	} PongStats;

	typedef struct {
		int sockfd;
		size_t buf_len;
		size_t write_pos[2];
		pthread_spinlock_t to_access[2];
		uint8_t *buff[2];

		pthread_spinlock_t lock;
		pthread_t thread;
	} AsyncSocket;

	/* Name tcp_connect_to
	 * Connects to a host using TCP over IPv4
	 * Return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_connect_to(char *ip, uint16_t port);

	/* Name tcp_listen_on_port
	 * Starts listening in a socket using TCP over IPv4
	 * Return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_listen_on_port(uint16_t port);

	/* Name tcp_accept
	 * Accepts a new connection from a listen socket
	 * Return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_accept(int listen_socket);

	/* Name tcp_message_send
	 * Sends a full message to a socket
	 * Return 0 if OK, something else if error.
	 */
	int tcp_message_send(int socket, const void *message, size_t len);

	/* Name tcp_message_recv
	 * Receives a full message from a socket
	 * Return 0 if OK, something else if error.
	 */
	int tcp_message_recv(int socket, void *message, size_t len);




	/* Name tcp_connect_to_async
	 * Connects to a host using TCP over IPv4
	 * Return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_connect_to_async(char *ip, uint16_t port, AsyncSocket *sock, size_t buf_len);

	/* Name tcp_accept_async
	 * Accepts a new connection from a listen socket
	 * Return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_accept_async(int listen_socket, AsyncSocket *sock, size_t buf_len);

	/* Name tcp_message_send_async
	 * Sends a full message to a socket
	 * Return 0 if OK, something else if error.
	 */
	int tcp_message_send_async(AsyncSocket *sock, const void *message, size_t len);

	/* Name tcp_message_recv_async
	 * Receives a full message from a socket
	 * Return 0 if OK, something else if error.
	 */
	int tcp_message_recv_async(AsyncSocket *sock, void *message, size_t len);


#ifdef __cplusplus
}
#endif
#endif
