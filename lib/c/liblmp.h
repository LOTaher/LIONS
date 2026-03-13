/*  liblmp.h - Definitions for the LIONS Distributed System
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

#ifndef LIBLMP_H
#define LIBLMP_H
#include <stddef.h>
#include <pthread.h>
#include "lt_base.h"
#include "lmp.h"
#define LT_ARENA_IMPLEMENTATION
#include "lt_arena.h"

// ===============================================================
// Net
// ===============================================================

lmp_error   lmp_net_send_packet(u32 fd, const lmp_packet* packet, lmp_result* result);
lmp_error   lmp_net_recv_packet(u32 fd, u8* buffer, size_t size, lmp_packet* packet, lmp_result* result);
char*       lmp_net_get_client(u32 fd, mem_arena* arena);
u8          lmp_net_is_connection_alive(u32 fd);

// ===============================================================
// Admiral
// ===============================================================

#define ADMIRAL_BACKLOG                     15
#define ADMIRAL_QUEUE_CAPACITY              50
#define ADMIRAL_QUEUE_READ_RETRY_SECONDS    30

#define ADMIRAL_PORT_ADMIRAL                5321
#define ADMIRAL_HOST_ADMIRAL                "100.113.240.39" // mirage
#define ADMIRAL_ENDPOINT_ADMIRAL            "100.113.240.39:5321"
// #define ADMIRAL_HOST_ADMIRAL             "100.109.120.90" // inferno
// #define ADMIRAL_ENDPOINT_ADMIRAL         "100.109.120.90:5321"

#define ADMIRAL_PORT_RECEPTION              4200
#define ADMIRAL_HOST_RECEPTION              "100.103.121.7" // nuke
#define ADMIRAL_ENDPOINT_RECEPTION          "100.103.121.7:4200"

#define ADMIRAL_PORT_S2                     6767
#define ADMIRAL_HOST_S2                     "100.113.240.39" // mirage
#define ADMIRAL_ENDPOINT_S2                 "100.113.240.39:6767"
// #define ADMIRAL_HOST_S2                  "100.103.121.7" // nuke
// #define ADMIRAL_ENDPOINT_S2              "100.103.121.7:6767"

#define ADMIRAL_PORT_GIBSON                 8800
#define ADMIRAL_HOST_GIBSON                 "100.113.240.39" // mirage
#define ADMIRAL_ENDPOINT_GIBSON             "100.113.240.39:8800"

typedef enum {
    LMP_ADMIRAL_SERVICE_ADMIRAL,
    LMP_ADMIRAL_SERVICE_RECEPTION,
    LMP_ADMIRAL_SERVICE_S2,
    LMP_ADMIRAL_SERVICE_GIBSON,
    LMP_ADMIRAL_SERVICE_COUNT,
    LMP_ADMIRAL_SERVICE_NONE
} lmp_admiral_service;

extern char* lmp_admiral_services[];

typedef struct {
    u64                          id;
    lmp_admiral_service          destination;
    lmp_admiral_service          sender;
    lmp_packet                   packet;
} lmp_admiral_message;

typedef struct {
    mem_arena*           arena;
    lmp_admiral_message* messages[255];
    u8                   size;
    u8                   capacity;
    u8                   head;
    u8                   tail;
    pthread_mutex_t      mutex;
} lmp_admiral_queue;


typedef struct {
    lmp_admiral_queue* queue;
} lmp_admiral_network_args;

typedef struct {
    lmp_admiral_queue* queue;
} lmp_admiral_admiral_args;

void                 lmp_admiral_queue_init(lmp_admiral_queue* queue, u8 capacity);
s8                   lmp_admiral_queue_enqueue(lmp_admiral_queue* queue, const lmp_admiral_message* message);
lmp_admiral_message* lmp_admiral_queue_dequeue(lmp_admiral_queue* queue);

s8                   lmp_admiral_add_packet_to_queue(lmp_admiral_queue* queue, lmp_packet* packet);
void                 lmp_admiral_invalidate_packet(lmp_packet* packet);
void                 lmp_admiral_sanitize_message(lmp_admiral_message* message);

lmp_admiral_service  lmp_admiral_map_client_to_service(char* client);

// ===============================================================
// Reception
// ===============================================================

#define RECEPTION_BACKLOG 15

// ===============================================================
// Log
// ===============================================================

#define LMP_LOG_LIONS_COLOR             "\x1b[38;5;220m"

#define LMP_LOG_TYPE_COLOR_INFO         "\x1b[38;5;27m"
#define LMP_LOG_TYPE_COLOR_WARN         "\x1b[38;5;208m"
#define LMP_LOG_TYPE_COLOR_ERROR        "\x1b[38;5;124m"

#define LMP_LOG_SERVICE_COLOR_ADMIRAL   "\x1b[38;5;48m"
#define LMP_LOG_SERVICE_COLOR_RECEPTION "\x1b[38;5;93m"
#define LMP_LOG_SERVICE_COLOR_S2        "\x1b[38;5;201m"
#define LMP_LOG_SERVICE_COLOR_GIBSON    "\x1b[38;5;34m"

#define LMP_LOG_COLOR_RESET             "\x1b[0m"

typedef enum {
    LMP_PRINT_TYPE_INFO,
    LMP_PRINT_TYPE_WARN,
    LMP_PRINT_TYPE_ERROR
} lmp_log_print_type;

void lmp_log_print(lmp_admiral_service sender, lmp_admiral_service destination, const char* message, lmp_log_print_type type);

#endif // LIBLMP_H
