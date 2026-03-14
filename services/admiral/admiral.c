/*  admiral.c - The LIONS distributed system message broker
    Copyright (C) 2026 splatte.dev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. */


#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <pthread.h>

#include "../../lib/c/lt_arena.h"
#include "../../lib/c/lt_base.h"
#include "../../lib/c/lmp.h"
#include "../../lib/c/liblmp.h"

void* network_loop(void* args) {
    lmp_admiral_network_args* a = (lmp_admiral_network_args*)args;

    mem_arena* networkArena = arena_create(KiB(8));


    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Failed to create socket", LMP_PRINT_TYPE_ERROR);
        return NULL;
    }

    int opt = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Failed to set socket option", LMP_PRINT_TYPE_ERROR);
        close(listenFd);
        return NULL;
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(ADMIRAL_PORT_ADMIRAL);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int b = bind(listenFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (b == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Failed to bind to socket", LMP_PRINT_TYPE_ERROR);
        close(listenFd);
        return NULL;
    }

    int l = listen(listenFd, SOMAXCONN);
    if (l == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Failed to bind to listen", LMP_PRINT_TYPE_ERROR);
        close(listenFd);
       return NULL;
    }

    char logBuffer[255];
    snprintf(logBuffer, sizeof(logBuffer), "Listening on %d", ADMIRAL_PORT_ADMIRAL);
    lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, logBuffer, LMP_PRINT_TYPE_INFO);

    struct pollfd fds[ADMIRAL_BACKLOG + 1];
    u8 fdsLength = 0;

    fds[0].fd = listenFd;
    fds[0].events = POLLIN;
    fdsLength++;

    for (;;) {
        u8 ready = poll(fds, fdsLength, -1);
        if (ready < 0) {
            lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Failed to start poll", LMP_PRINT_TYPE_INFO);
        }

        for (u8 i = 0; i < fdsLength; i++) {
            // NOTE(laith): no events in a fd
            if (fds[i].revents == 0) continue;


            // NOTE(laith): if admiral's fd is recieving an event (new connection, etc)
            if (fds[i].fd == listenFd) {

                struct sockaddr_in clientAddr;
                socklen_t clientLength = sizeof(clientAddr);

                int connectionFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientLength);
                if (connectionFd == -1) {
                    lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Failed to accept connection", LMP_PRINT_TYPE_ERROR);
                    continue;
                }

                char* client = lmp_net_get_client(connectionFd, networkArena);
                if (client == NULL) {
                    lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Could not parse client information", LMP_PRINT_TYPE_ERROR);
                    close(connectionFd);
                    continue;
                }

                lmp_admiral_service service = lmp_admiral_map_client_to_service(client);
                if (service == LMP_ADMIRAL_SERVICE_NONE) {
                    lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Bad client connected", LMP_PRINT_TYPE_ERROR);
                    close(connectionFd);
                    continue;
                }

                lmp_log_print(service, LMP_ADMIRAL_SERVICE_ADMIRAL, "Successfully connected", LMP_PRINT_TYPE_INFO);

                if (fdsLength >= ADMIRAL_BACKLOG + 1) {
                    lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Too many clients connected. Closing connection.", LMP_PRINT_TYPE_ERROR);
                    close(connectionFd);
                    continue;
                }

                fds[fdsLength].fd = connectionFd;
                fds[fdsLength].events = POLLIN;
                fdsLength++;

            // NOTE(laith): if any other fd has activity (recieving data from a fd, etc)
            } else {
                u8 connectionFd = fds[i].fd;

                char* client = lmp_net_get_client(connectionFd, networkArena);
                if (client == NULL) {
                    lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Could not parse client information", LMP_PRINT_TYPE_ERROR);
                    close(connectionFd);
                    continue;
                }

                lmp_admiral_service service = lmp_admiral_map_client_to_service(client);
                if (service == LMP_ADMIRAL_SERVICE_NONE) {
                    lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Bad client connected", LMP_PRINT_TYPE_ERROR);
                    close(connectionFd);
                    continue;
                }

                if (fds[i].revents & (POLLERR | POLLHUP)) {
                    lmp_log_print(service, LMP_ADMIRAL_SERVICE_ADMIRAL, "Closed connection", LMP_PRINT_TYPE_ERROR);
                    fds[i] = fds[fdsLength - 1];
                    fds[i].revents = 0;
                    fdsLength--;
                    close(connectionFd);
                    continue;
                }

                lmp_packet* readPacket = arena_push(networkArena, sizeof(lmp_packet));
                lmp_packet sendPacket;
                lmp_result result;

                lmp_packet_init(readPacket);
                lmp_packet_init(&sendPacket);
                lmp_result_init(&result);
                u8 buffer[LMP_PACKET_MAX_SIZE];

                lmp_error error = lmp_net_recv_packet(connectionFd, buffer, sizeof(buffer), readPacket, &result);
                if (error != LMP_ERR_NONE) {
                    close(connectionFd);
                    lmp_log_print(service, LMP_ADMIRAL_SERVICE_ADMIRAL, "Bad packet. Closing connection", LMP_PRINT_TYPE_ERROR);
                    arena_clear(networkArena);
                    continue;
                }

                s8 p = lmp_admiral_add_packet_to_queue(a->queue, readPacket);
                if (p == -1) {
                    lmp_admiral_invalidate_packet(&sendPacket);
                    lmp_error send_error = lmp_net_send_packet(connectionFd, &sendPacket, &result);

                    if (send_error != LMP_ERR_NONE) {
                        lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, service, "Could not send invalid response packet", LMP_PRINT_TYPE_WARN);
                    }

                    close(connectionFd);
                    lmp_log_print(service, LMP_ADMIRAL_SERVICE_ADMIRAL, "Recieved invalid admiral packet. Closing connection", LMP_PRINT_TYPE_ERROR);
                    arena_clear(networkArena);
                    continue;
                }

                arena_clear(networkArena);
            }
        }
    }

    arena_destroy(networkArena);
    close(listenFd);
    return 0;
}

void* admiral_loop(void* args) {
    lmp_admiral_admiral_args* a = (lmp_admiral_admiral_args*)args;

    char logBuffer[255];

    for (;;) {
        memset(logBuffer, 0, sizeof(logBuffer));
        lmp_admiral_message* msg = lmp_admiral_queue_dequeue(a->queue);

        // NOTE(laith): can edit to enforce a retry cooldown
        if (msg == NULL) {
            // snprintf(logBuffer, sizeof(logBuffer),"No message in the queue. Retrying in %d seconds", ADMIRAL_QUEUE_READ_RETRY_SECONDS);
            // lmp_log_print("admiral", logBuffer, LMP_PRINT_TYPE_WARN);
            // sleep(ADMIRAL_QUEUE_READ_RETRY_SECONDS);
            continue;
        }

        lmp_admiral_sanitize_message(msg);

        lmp_log_print(msg->sender, msg->destination, "Forwarding message", LMP_PRINT_TYPE_INFO);
    }

    // TODO(laith): send the net packet, for now lets log to test
    // use the new endpoint macros to connect to the IP and port to send

    return 0;
}

int main(void) {
    lmp_admiral_queue queue;
    lmp_admiral_queue_init(&queue, ADMIRAL_QUEUE_CAPACITY);

    lmp_admiral_network_args networkArgs = {
        .queue = &queue,
    };

    pthread_t networkThread;

    pthread_create(&networkThread, NULL, network_loop, (void*)&networkArgs);

    lmp_admiral_admiral_args admiralArgs = {
        .queue = &queue,
    };

    pthread_t admiralThread;

    pthread_create(&admiralThread, NULL, admiral_loop, (void*)&admiralArgs);

    pthread_join(networkThread, NULL);
    pthread_join(admiralThread, NULL);

    return 0;
}

