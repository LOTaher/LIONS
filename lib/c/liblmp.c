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

#include "liblmp.h"
#include "lt_arena.h"
#include "lt_base.h"
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

char* lmp_net_get_client(u32 fd, mem_arena* arena) {
    struct sockaddr clientAddr = {0};
    socklen_t clientAddrLen = sizeof(clientAddr);
    int g = getpeername(fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (g == -1) {
        return NULL;
    }

    char host[255] = {0};
    char port[255] = {0};

    int n = getnameinfo(&clientAddr, clientAddrLen, host, sizeof(host), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV);
    if (n != 0) {
        return NULL;
    }

    char name[255] = {0};
    snprintf(name, sizeof(name), "%s:%s", host, port);

    char* allocatedName = arena_push(arena, sizeof(name));
    memcpy(allocatedName, name, sizeof(name));

    return allocatedName;
}

u8 lmp_net_is_connection_alive(u32 fd) {
    u8 buf[1];
    size_t n = recv(fd, buf, sizeof(buf), MSG_PEEK);

    return n == 0;
}

// ===============================================================
// Admiral
// ===============================================================

char* lmp_admiral_services[] = {
    "admiral",
    "reception",
    "s2",
    "gibson",
};

void lmp_admiral_queue_init(lmp_admiral_queue* queue, u8 capacity) {
    mem_arena* arena = arena_create(MiB(10));
    queue->arena = arena;
    queue->size = 0;
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    // queue->messeges

    pthread_mutex_init(&queue->mutex, NULL);
}

s8 lmp_admiral_queue_enqueue(lmp_admiral_queue* queue, lmp_admiral_message* message) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->size >= queue->capacity) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    queue->messages[queue->tail++] = message;
    queue->size++;

    pthread_mutex_unlock(&queue->mutex);

    return 1;
}

lmp_admiral_message* lmp_admiral_queue_dequeue(lmp_admiral_queue* queue) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->size == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    lmp_admiral_message* msg = queue->messages[queue->head++];

    queue->size--;

    if (queue->size == 0) {
        arena_clear(queue->arena);
        queue->head = 0;
        queue->tail = 0;
    }

    pthread_mutex_unlock(&queue->mutex);

    return msg;
}

lmp_admiral_message* lmp_admiral_message_create(mem_arena* arena, lmp_admiral_service destination, lmp_admiral_service sender, lmp_packet* packet) {
    u64 messageSize = sizeof(lmp_admiral_message) + LMP_PACKET_HEADER_SIZE + packet->payload_length + 1;
    lmp_admiral_message* message = arena_push(arena, messageSize);

    message->id = rand();
    message->destination = destination;
    message->sender = sender;

    memcpy((void*)&message->packet, packet, LMP_PACKET_HEADER_SIZE + packet->payload_length);

    return message;
}

// NOTE(laith): this function changes how ownership of the packet is handled. This paacket lives
// in the network loop arena and now it is getting copied over to the queue arena. with that, after
// this function ends, we can safely pop the packet memory of the network arena and start again
//
// Do NOT share memory across threads!
s8 lmp_admiral_packet_queue(lmp_admiral_queue* queue, lmp_packet* packet) {
    // NOTE(laith): this should be [dest][sender][EMPTY PAYLOAD BYTE] at the minimum
    if (packet->payload_length < 3) {
        return -1;
    }

    // NOTE(laith): this converts and ascii string to its byte form
    lmp_admiral_service destination = packet->payload[0] - '0';
    lmp_admiral_service sender = packet->payload[1] - '0';

    if (destination >= LMP_ADMIRAL_SERVICE_COUNT || sender >= LMP_ADMIRAL_SERVICE_COUNT) {
        return -1;
    }

    lmp_admiral_packet_sanitize(packet);

    lmp_admiral_message* message = lmp_admiral_message_create(queue->arena, destination, sender, packet);

    s8 e = lmp_admiral_queue_enqueue(queue, message);
    if (e == -1) {
        lmp_log_print(sender, destination, "Could not enqueue packet", LMP_PRINT_TYPE_ERROR);
        return -1;
    }

    lmp_log_print(sender, destination, "Added packet to queue", LMP_PRINT_TYPE_INFO);
    return 1;
}

void lmp_admiral_packet_sanitize(lmp_packet* packet) {
    u64 sanitizedPayloadSize = packet->payload_length - 2;
    u8 sanitizedPayload[sanitizedPayloadSize];

    u64 sanitizedPayloadInput = 0;
    for (u64 i = 2; i < packet->payload_length; i++) {
        sanitizedPayload[sanitizedPayloadInput++] = packet->payload[i];
    }

    memcpy((void*)packet->payload, sanitizedPayload, sanitizedPayloadSize);
    packet->payload_length = sanitizedPayloadSize;
}

void lmp_admiral_packet_invalidate(lmp_packet* packet) {
    packet->type = LMP_TYPE_INVALID;
    packet->arg = LMP_ARG_INVALID_PAYLOAD;
    packet->flags = LMP_FLAGS_NONE;
    packet->payload = LMP_PAYLOAD_EMPTY;
    packet->payload_length = 1;
}

lmp_admiral_service lmp_admiral_service_map_from_client(char* client) {
    if (strcmp(client, ADMIRAL_ENDPOINT_ADMIRAL) == 0) {
        return LMP_ADMIRAL_SERVICE_ADMIRAL;
    }

    if (strcmp(client, ADMIRAL_ENDPOINT_RECEPTION) == 0) {
        return LMP_ADMIRAL_SERVICE_RECEPTION;
    }

    if (strcmp(client, ADMIRAL_ENDPOINT_S2) == 0) {
        return LMP_ADMIRAL_SERVICE_S2;
    }

    if (strcmp(client, ADMIRAL_ENDPOINT_GIBSON) == 0) {
        return LMP_ADMIRAL_SERVICE_GIBSON;
    }

    return LMP_ADMIRAL_SERVICE_NONE;
}

// ===============================================================
// Log
// ===============================================================

const char* lmp_log_lions =         "LIONS //";
const char* lmp_log_lions_bracket = "[LIONS //]";

const char* lmp_log_print_type_colors[] = {
    LMP_LOG_TYPE_COLOR_INFO,
    LMP_LOG_TYPE_COLOR_WARN,
    LMP_LOG_TYPE_COLOR_ERROR
};

const char* lmp_log_print_service_colors[] = {
    LMP_LOG_SERVICE_COLOR_ADMIRAL,
    LMP_LOG_SERVICE_COLOR_RECEPTION,
    LMP_LOG_SERVICE_COLOR_S2,
    LMP_LOG_SERVICE_COLOR_GIBSON,
};

void lmp_log_print(lmp_admiral_service sender, lmp_admiral_service destination, const char* message, lmp_log_print_type type) {
    time_t timestamp = time(NULL);
    struct tm* time_info = localtime(&timestamp);

    // [LIONS]  (sender -> destination) hour:minute:second: message
    printf("%s[%s]%s (%s%s%s -> %s%s%s) %02d:%02d:%02d: %s\n", lmp_log_print_type_colors[type], lmp_log_lions, LMP_LOG_COLOR_RESET, lmp_log_print_service_colors[sender], lmp_admiral_services[sender], LMP_LOG_COLOR_RESET,  lmp_log_print_service_colors[destination], lmp_admiral_services[destination], LMP_LOG_COLOR_RESET, time_info->tm_hour, time_info->tm_min, time_info->tm_sec, message);
    return;
}

