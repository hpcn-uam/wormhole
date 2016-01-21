#include <common.h>

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