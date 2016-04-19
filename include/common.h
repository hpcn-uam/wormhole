#ifndef __WORM_COMMON_H__
#define __WORM_COMMON_H__

#include <arpa/inet.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <strings.h>
#include <errno.h>

#include <tls.h>

#define WH_COMMON_LIMITW8_RECV (1) //(1<<2) //Wait time in cycles if no more data retrived
#define WH_COMMON_LIMITW8_SEND (1<<15) //Wait time in cycles if no more data retrived

#define UNUSED(x) (void)(x)

#ifdef __cplusplus
extern "C" {
#endif

#define likely(x)    __builtin_expect (!!(x), 1)
#define unlikely(x)  __builtin_expect (!!(x), 0)

#define OPTIMAL_BUFFER_SIZE (512*1024)

	enum syncSocketType {
		NOSSL, SRVSSL, CLIENTSSL
	};
	enum asyncSocketType {
		SEND_SOCKET, RECV_SOCKET
	};
	enum ctrlMsgType {
		HELLOEINSTEIN, SETUP, QUERYID, RESPONSEID, PING, PONG, DOWNLINK, OVERLOAD, UNDERLOAD, CTRL_OK, CTRL_ERROR, HALT
	};

	typedef struct {
		uint16_t id;
		uint16_t listenPort;
		uint32_t IP; //TODO fix para ipv6
		uint32_t connectionDescriptionLength;
		uint8_t *connectionDescription; // (LISP connection description)
		int64_t core;
	} WormSetup;

	typedef struct {
		uint16_t id;
	} PongStats;

	typedef struct {
		int sockfd;
		size_t buf_len;
		size_t read_pos[2];
		size_t write_pos[2];
		int to_access[2];
		uint8_t *buff[2];
		size_t current_send_buf;
		size_t current_recv_buf;
		int can_read;

		enum asyncSocketType socket_type;
		int finish;
		int flush;
		pthread_spinlock_t lock;
		pthread_t thread;
	} AsyncSocket;

	typedef struct {
		int sockfd;
		struct tls *tls;
		struct tls *tlsIO;
		struct tls_config *config;
	} SyncSocket;

	/** tcp_connect_to
	 * Connects to a host using TCP over IPv4
	 * @return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_connect_to(char *ip, uint16_t port);

	/** tcp_listen_on_port
	 * Starts listening in a socket using TCP over IPv4
	 * @return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_listen_on_port(uint16_t port);

	/** tcp_accept
	 * Accepts a new connection from a listen socket
	 * @return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_accept(int listen_socket, struct timeval *timeout);

	/** tcp_message_send
	 * Sends a full message to a socket
	 * @return 0 if OK, something else if error.
	 */
	int tcp_message_send(int socket, const void *message, size_t len);

	/** tcp_message_recv
	 * Receives a full message from a socket
	 * @return number of bytes read.
	 */
	size_t tcp_message_recv(int socket, void *message, size_t len, uint8_t sync);

	/** tcp_upgrade2syncSocket
	 * upgrades a simple socket to SyncSocket
	 * @param socket : the older socket
	 * @param mode : the SSL mode
	 * @param sslConfig : Can be NULL (even with ssl=1). Sets the SSL config. connection.
	 * @return a new SyncSocket
	 */
	SyncSocket *tcp_upgrade2syncSocket(int socket, enum syncSocketType mode, struct tls_config *sslConfig);

	/** tcp_message_ssend
	 * Sends a full message to a socket
	 * @return 0 if OK, something else if error.
	 */
	int tcp_message_ssend(SyncSocket *socket, const void *message, size_t len);

	/** tcp_message_srecv
	 * Receives a full message from a socket
	 * @return number of bytes read.
	 */
	size_t tcp_message_srecv(SyncSocket *socket, void *message, size_t len, uint8_t sync);

	/** tcp_sclose
	 * Receives a full message from a socket
	 * @return number of bytes read.
	 */
	void tcp_sclose(SyncSocket *socket);



	/** tcp_connect_to_async
	 * Connects to a host using TCP over IPv4
	 * @return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_connect_to_async(char *ip, uint16_t port, AsyncSocket *sock);

	/** tcp_accept_async
	 * Accepts a new connection from a listen socket
	 * @return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_accept_async(int listen_socket, AsyncSocket *sock, struct timeval *timeout);


	int socket_upgrade_to_async_send(AsyncSocket *async_sock, int sockfd);
	int socket_upgrade_to_async_recv(AsyncSocket *async_sock, int sockfd);

	void destroy_asyncSocket(AsyncSocket *sock);

	void flush_recv(AsyncSocket *sock);

#ifdef __cplusplus
}
#endif
#endif
