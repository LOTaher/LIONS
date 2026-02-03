#include "lt_arena.h"
#include "lt_base.h"
#include "stmp.h"
#include "libstmp.h"

#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#define ADMIRAL_PORT 5321
#define ADMIRAL_BACKLOG 15
#define ADMIRAL_QUEUE_CAPACITY 50

int main(void) {
    stmp_admiral_queue queue;
    stmp_admiral_queue_init(&queue, ADMIRAL_QUEUE_CAPACITY);
    mem_arena* arena = arena_create(MiB(100));

    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1) {
        stmp_log_print("admiral", "Failed to create socket", ERROR);
        return -1;
    }

    int yes = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        stmp_log_print("admiral", "Failed to set socket option", ERROR);
        close(socketFd);
        return -1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(ADMIRAL_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int b = bind(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (b == -1) {
        stmp_log_print("admiral", "Failed to bind to socket", ERROR);
        close(socketFd);
        return -1;
    }

    int l = listen(socketFd, ADMIRAL_BACKLOG);
    if (l == -1) {
        stmp_log_print("admiral", "Failed to bind to listen", ERROR);
        close(socketFd);
        return -1;
    }

    stmp_log_print("admiral", "Listening on specified port...", INFO);

    while (true) {
        stmp_log_print("admiral", "Waiting for connection...", INFO);

        struct sockaddr_in clientAddr;
        socklen_t clientLength = sizeof(clientAddr);

        int connectionFd = accept(socketFd, (struct sockaddr *)&clientAddr, &clientLength);
        if (connectionFd == -1) {
            stmp_log_print("admiral", "Failed to accept connection", ERROR);
            continue;
        }

        stmp_log_print("admiral", "Accepted connection", INFO);

        // NOTE(laith): marking arena here as the readPacket may be enqueued for the lifetime of
        // the program, until it is dequeued
        u64 mark = arena_mark(arena);
        stmp_packet* readPacket = arena_push(arena, sizeof(stmp_packet));
        stmp_packet sendPacket;
        stmp_result result;

        stmp_packet_init(readPacket);
        stmp_packet_init(&sendPacket);
        stmp_result_init(&result);
        u8 buffer[STMP_PACKET_MAX_SIZE];

        stmp_error error = stmp_net_recv_packet(connectionFd, buffer, sizeof(buffer), readPacket, &result);
        if (error != STMP_ERR_NONE) {
            arena_pop(arena, mark);
            close(connectionFd);
            stmp_log_print("admiral", "Recieved a bad packet. Closing connection.", ERROR);
            continue;
        }

        error = stmp_admiral_parse_and_queue_packet(&queue, readPacket);
        if (error != STMP_ERR_NONE) {
            arena_pop(arena, mark);
            stmp_admiral_invalidate_packet(&sendPacket);
            stmp_error send_error = stmp_net_send_packet(connectionFd, &sendPacket, &result);

            if (send_error != STMP_ERR_NONE) {
                stmp_log_print("admiral", "Could not send invalid response.", ERROR);
            }

            close(connectionFd);
            stmp_log_print("admiral", "Recieved non admiral STMP packet. Closing connection.", WARN);
            continue;
        }

        stmp_log_print("admiral", "Message queued. Closing connection", INFO);
        close(connectionFd);
    }

    close(socketFd);
    arena_destroy(arena);
    return 0;
}

