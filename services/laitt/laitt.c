/*  laitt.c - The LIONS distributed system MQTT bridge
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

#include <stdio.h>
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
#include <mosquitto.h>

#include "../../lib/c/lt_arena.h"
#include "../../lib/c/lt_base.h"
#include "../../lib/c/lmp.h"
#include "../../lib/c/liblmp.h"

int main(void) {
    mem_arena* networkArena = arena_create(KiB(1));

    struct mosquitto* mosq = mosquitto_new(NULL, 1, NULL);
    int c = mosquitto_connect(mosq, LAITT_MOSQUITTO_HOST, LAITT_MOSQUITTO_PORT, 60);
    if (c != MOSQ_ERR_SUCCESS) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_LAITT, LMP_ADMIRAL_SERVICE_LAITT, "Error connecting to Mosquitto", LMP_PRINT_TYPE_ERROR);
        return 1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Failed to create socket", LMP_PRINT_TYPE_ERROR);
        return 1;
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Failed to set socket option", LMP_PRINT_TYPE_ERROR);
        close(fd);
        return 1;
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(ADMIRAL_PORT_LAITT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int b = bind(fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (b == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_LAITT, LMP_ADMIRAL_SERVICE_LAITT, "Failed to bind to socket", LMP_PRINT_TYPE_ERROR);
        close(fd);
        return 1;
    }

    int l = listen(fd, SOMAXCONN);
    if (l == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_LAITT, LMP_ADMIRAL_SERVICE_LAITT, "Failed to bind to listen", LMP_PRINT_TYPE_ERROR);
        close(fd);
       return 1;
    }

    char logBuffer[255];
    snprintf(logBuffer, sizeof(logBuffer), "Listening on %d", ADMIRAL_PORT_LAITT);
    lmp_log_print(LMP_ADMIRAL_SERVICE_LAITT, LMP_ADMIRAL_SERVICE_LAITT, logBuffer, LMP_PRINT_TYPE_INFO);

    lmp_packet readPacket;
    lmp_result result;
    u8 buffer[LMP_PACKET_MAX_SIZE];

    mosquitto_loop_start(mosq);

    for (;;) {
        lmp_packet_init(&readPacket);
        lmp_result_init(&result);

        struct sockaddr_in clientAddr;
        socklen_t clientLength = sizeof(clientAddr);

        int connectionFd = accept(fd, (struct sockaddr *)&clientAddr, &clientLength);
        if (connectionFd == -1) {
            lmp_log_print(LMP_ADMIRAL_SERVICE_LAITT, LMP_ADMIRAL_SERVICE_LAITT, "Failed to accept connection", LMP_PRINT_TYPE_ERROR);
            continue;
        }

        char* client = lmp_net_get_client(connectionFd, networkArena);
        if (client == NULL) {
            lmp_log_print(LMP_ADMIRAL_SERVICE_LAITT, LMP_ADMIRAL_SERVICE_LAITT, "Could not parse client information", LMP_PRINT_TYPE_ERROR);
            close(connectionFd);
            continue;
        }

        lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_LAITT, "Successfully connected", LMP_PRINT_TYPE_INFO);

        lmp_error error = lmp_net_recv_packet(connectionFd, buffer, sizeof(buffer), &readPacket, &result);
        if (error != LMP_ERR_NONE) {
            close(connectionFd);
            lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_LAITT, "Bad packet. Closing connection", LMP_PRINT_TYPE_ERROR);
            arena_clear(networkArena);
            continue;
        }

        mosquitto_publish(mosq, NULL, LAITT_LIGHTS_TOPIC_SET, readPacket.payload_length, readPacket.payload, 0, NULL);
        lmp_log_print(LMP_ADMIRAL_SERVICE_ADMIRAL, LMP_ADMIRAL_SERVICE_LAITT, "Sent payload. Closing connection", LMP_PRINT_TYPE_INFO);

        close(connectionFd);
        arena_clear(networkArena);
    }

    return 0;
}
