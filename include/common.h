#ifndef __WORM_COMMON_H__
#define __WORM_COMMON_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

	enum ctrlMsgType {
		HELLOEINSTEIN, SETUP, QUERYID, RESPONSEID, PING, PONG, DOWNLINK, OVERLOAD, UNDERLOAD
	};

	typedef struct {
		uint16_t id;
		uint16_t listenPort;
		uint16_t core;
		uint32_t IP; //TODO fix para ipv6
		uint32_t connectionDescriptionLength;
		uint8_t *connectionDescription; // (LISP connection description)
	} WormSetup;

	typedef struct {
		uint16_t id;
	} PongStats;
	
	/* Name tcp_connect_to
	 * Connects to a host using TCP over IPv4
	 * Return -1 if ERROR, else the socket file descriptor.
	 */
	int tcp_connect_to(char *ip, uint16_t port);
	
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

#ifdef __cplusplus
}
#endif
#endif
