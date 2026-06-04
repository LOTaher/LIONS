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
#include "../../lib/c/lt_string.h"

static string8 SERVICE_NAME      = str8("laitt");
static string8 SERVICE_HOSTNAME  = str8("inferno");
static string8 SERVICE_COLOR     = str8("\x1b[38;5;226m");

#define LAITT_MOSQUITTO_PORT    1883
#define LAITT_MOSQUITTO_HOST    "100.109.120.90"
#define LAITT_LIGHTS_TOPIC      "zigbee2mqtt/bedroom_lights"
#define LAITT_LIGHTS_TOPIC_SET  "zigbee2mqtt/bedroom_lights/set"

int main(void) {
    lmp_log(str8("Starting Laitt - MQTT Bridge - Version 1"));

    arena* networkArena = arena_create(KiB(1));

    string8 serviceString = lmp_log_build_service_string(networkArena, SERVICE_COLOR, SERVICE_HOSTNAME, SERVICE_NAME);

    struct mosquitto* mosq = mosquitto_new(NULL, 1, NULL);
    int c = mosquitto_connect(mosq, LAITT_MOSQUITTO_HOST, LAITT_MOSQUITTO_PORT, 60);
    if (c != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "%.*s Error connecting to Mosquitto\n", str8_fmt(serviceString));
        return 1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        fprintf(stderr, "%.*s Failed to create socket\n", str8_fmt(serviceString));
        return 1;
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        fprintf(stderr, "%.*s Failed to set socket option\n", str8_fmt(serviceString));
        close(fd);
        return 1;
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(ADMIRAL_PORT_LAITT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int b = bind(fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (b == -1) {
        fprintf(stderr, "%.*s Failed to bind to address\n", str8_fmt(serviceString));
        close(fd);
        return 1;
    }

    int l = listen(fd, SOMAXCONN);
    if (l == -1) {
        fprintf(stderr, "%.*s Failed to listen\n", str8_fmt(serviceString));
        close(fd);
       return 1;
    }

    lmp_log(str8("Listening on 1818"));

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
            fprintf(stderr, "%.*s Failed to accept connection\n", str8_fmt(serviceString));
            continue;
        }

        lmp_error error = lmp_net_recv_packet(connectionFd, buffer, sizeof(buffer), &readPacket, &result);
        if (error != LMP_ERR_NONE) {
            close(connectionFd);
            fprintf(stderr, "%.*s Recieved bad packet\n", str8_fmt(serviceString));
            arena_clear(networkArena);
            continue;
        }

        // TODO(laith): using a specific tailored payload, figure out if its subscribe or public, which topic string, and then the payload

        mosquitto_publish(mosq, NULL, LAITT_LIGHTS_TOPIC_SET, readPacket.payload_length, readPacket.payload, 0, NULL);
        printf("%.*s Sent payload to topic: %s\n", str8_fmt(serviceString), LAITT_LIGHTS_TOPIC_SET);

        close(connectionFd);
        arena_clear(networkArena);
    }

    return 0;
}
