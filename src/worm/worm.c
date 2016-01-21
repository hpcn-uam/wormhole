#include <common.h>
#include <worm.h>
#include <worm_private.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <strings.h>

/*
*Global variables
*/
Worm2EinsConn WH_einsConn;
uint16_t WH_myId;
WormSetup WH_mySetup;


/*
*===============
*/


uint8_t WH_init (void) {
    
    // TODO: Deshardcodear y leer de variables de entorno
    WH_myId = 1;
    WH_einsConn.Port = 5000;
    WH_einsConn.IP = inet_addr("127.0.0.1");
    
    
    WH_einsConn.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (WH_einsConn.socket == -1) {
        return 1;
    }
       
    struct sockaddr_in cli_addr;
    bzero(&cli_addr, sizeof(cli_addr));

    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(WH_einsConn.Port);
    cli_addr.sin_addr.s_addr = WH_einsConn.IP;
    if ((connect(WH_einsConn.socket, (struct sockaddr *) &cli_addr, sizeof(struct sockaddr_in))) < 0) {
        return 1;
    }
    
    // Fill hello message with worm id
    size_t hellomsgSize = sizeof(enum ctrlMsgType) + sizeof(uint16_t);
    uint8_t hellomsg[hellomsgSize];
    *((enum ctrlMsgType *) &hellomsg) = HELLOEINSTEIN;
    *((uint16_t *) (&hellomsg + sizeof(enum ctrlMsgType))) = WH_myId;
    
    // Send hello message
    if (tcp_message_send(WH_einsConn.socket, hellomsg, hellomsgSize) != 0) {
        return 1;
    }
    
    // Receive WormSetup
    uint8_t *wormSetupMsg = (uint8_t *)&WH_mySetup;
	if (tcp_message_recv(WH_einsConn.socket, wormSetupMsg, sizeof(WormSetup)) != 0) {
        return 1;
    }
    
    // Receive connectionDescription
    WH_mySetup.connectionDescription = malloc(WH_mySetup.connectionDescriptionLength);
	if (tcp_message_recv(WH_einsConn.socket, WH_mySetup.connectionDescription, WH_mySetup.connectionDescriptionLength) != 0) {
        return 1;
    }
    
	return 0;
}