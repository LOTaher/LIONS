/*  liblmp.c - Utilities for the LIONS Middleware Protocol
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#include "lt_arena.h"
#include "liblmp.h"
#include "lt_base.h"
#include "lt_string.h"
#include "lmp.h"

// ===============================================================
// Net
// ===============================================================

lmp_error lmp_net_send_packet(u32 fd, const lmp_packet* packet, lmp_result* result) {
    u8 buffer[LMP_PACKET_MAX_SIZE];
    lmp_packet_serialize(buffer, sizeof(buffer), packet, result);
    if (result->error != LMP_ERR_NONE) {
        return result->error;
    }

    ssize_t sent = send(fd, buffer, result->size, 0);
    if (sent < 0) {
        return LMP_ERR_BAD_INPUT;
    }

    return result->error;
}

lmp_error lmp_net_recv_packet(u32 fd, u8* buffer, size_t size, lmp_packet* packet, lmp_result* result) {
    u8 terminated = 0;
    u8 invalid = 0;
    u64 packet_size = 0;

    for (;;) {
        u8 scratch[LMP_PACKET_MAX_SIZE];

        int bytes = recv(fd, scratch, sizeof(scratch), 0);
        if (bytes <= 0) {
            break;
        }

        for (int i = 0; i < bytes; i++) {
            buffer[packet_size] = scratch[i];
            packet_size++;

            if (packet_size >= size) {
                invalid = 1;
                break;
            }

            if (scratch[i] == LMP_PACKET_TERMINATE) {
                terminated = 1;
                break;
            }
        }

        if (terminated || invalid) break;
    }

    if (terminated) {
        lmp_packet_deserialize(buffer, packet_size, packet, result);
        return result->error;
    }

    if (invalid) {
        return LMP_ERR_BAD_SIZE;
    }

    return LMP_ERR_BAD_INPUT;
}

// ===============================================================
// Admiral
// ===============================================================

char* lmp_admiral_services[] = {
    "admiral",
    "reception",
    "s2",
    "gibson",
    "laitt",
    "lightctl"
};

b8 lmp_admiral_service_handshake(lmp_admiral_service service, u32 fd) {
    lmp_packet sendInitPacket = {0};
    lmp_result result = {0};
    lmp_packet_init(&sendInitPacket);
    lmp_result_init(&result);

    sendInitPacket.version = 0x02;
    sendInitPacket.type = LMP_TYPE_INIT;
    sendInitPacket.arg = LMP_ARG_INIT_INIT;
    u8 emptyPayload[] = {LMP_PAYLOAD_EMPTY};
    sendInitPacket.payload = emptyPayload;
    sendInitPacket.payload_length = 1;

    lmp_net_send_packet(fd, &sendInitPacket, &result);

    if (result.error != LMP_ERR_NONE) {
        printf("unable to send packet (INIT)\n");
        return 0;
    }

    lmp_packet sendSendPacket = {0};
    lmp_packet_init(&sendSendPacket);

    sendSendPacket.version = 0x02;
    sendSendPacket.type = LMP_TYPE_SEND;
    sendSendPacket.arg = LMP_ARG_SEND;
    u8 payload[] = {LMP_ADMIRAL_SERVICE_ADMIRAL, service};
    sendSendPacket.payload = payload;
    sendSendPacket.payload_length = 2;

    lmp_net_send_packet(fd, &sendSendPacket, &result);

    if (result.error != LMP_ERR_NONE) {
        printf("unable to send packet (SEND)\n");
        return 0;
    }

    u8 buffer[LMP_PACKET_MAX_SIZE];
    lmp_packet recvPacket = {0};
    lmp_packet_init(&recvPacket);

    lmp_error error = lmp_net_recv_packet(fd, buffer, sizeof(buffer), &recvPacket, &result);

    if (error != LMP_ERR_NONE) {
        printf("unable to recv packet\n");
        return 0;
    }

    if (recvPacket.arg != LMP_ARG_INIT_ACCEPT) {
        printf("recv arg from admiral bad\n");
        return 0;
    }

    return 1;
}

// ===============================================================
// Log
// ===============================================================

void lmp_log(string8 message) {
    printf("%s %.*s\n", LMP_LOG_LIONS_LOGO_COLORED, str8_fmt(message));
}

string8 lmp_log_build_service_string(arena *arena, string8 color, string8 hostname, string8 serviceName) {
    u64 logoLen = sizeof(LMP_LOG_LIONS_LOGO_COLORED) - 1;
    u64 size = logoLen + 1 + color.length + hostname.length + serviceName.length + 10;
    u8* buf = (u8*)arena_push(arena, size);
    int written = snprintf((char *)buf, size, "%s %.*s(%.*s | %.*s)%s", LMP_LOG_LIONS_LOGO_COLORED, str8_fmt(color), str8_fmt(hostname), str8_fmt(serviceName), LMP_LOG_COLOR_RESET);
    return (string8){buf, (u64)written};
}

