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
#include "lt_arena.h"
#include "lt_string.h"

// ===============================================================
// Net
// ===============================================================

lmp_error   lmp_net_send_packet(u32 fd, const lmp_packet* packet, lmp_result* result);
lmp_error   lmp_net_recv_packet(u32 fd, u8* buffer, size_t size, lmp_packet* packet, lmp_result* result);

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
#define LMP_LOG_SERVICE_COLOR_LAITT     "\x1b[38;5;226m"
#define LMP_LOG_SERVICE_COLOR_LIGHTS  "\x1b[38;5;213m"

#define LMP_LOG_COLOR_RESET             "\x1b[0m"

#define LMP_LOG_LIONS_LOGO_COLORED "\x1b[38;5;220m[LIONS //]\x1b[0m"

typedef enum {
    LMP_PRINT_TYPE_INFO,
    LMP_PRINT_TYPE_WARN,
    LMP_PRINT_TYPE_ERROR
} lmp_log_print_type;

void    lmp_log(string8 message);
string8 lmp_log_build_service_string(arena *arena, string8 color, string8 hostname, string8 serviceName);

// ===============================================================
// Admiral
// ===============================================================

#define ADMIRAL_PORT_ADMIRAL                5321
#define ADMIRAL_HOST_ADMIRAL                "100.113.240.39" // mirage
#define ADMIRAL_ENDPOINT_ADMIRAL            "100.113.240.39:5321"

#define ADMIRAL_PORT_LAITT                  1818
#define ADMIRAL_HOST_LAITT                  "100.109.120.90" // inferno
#define ADMIRAL_ENDPOINT_LAITT              "100.109.120.90:1818"

#define ADMIRAL_PORT_LIGHTS               3456
#define ADMIRAL_HOST_LIGHTS               "100.113.240.39" // mirage
#define ADMIRAL_ENDPOINT_LIGHTS           "100.113.240.39:3456"

#define ADMIRAL_PORT_RECEPTION              4200
#define ADMIRAL_HOST_RECEPTION              "100.103.121.7" // nuke
#define ADMIRAL_ENDPOINT_RECEPTION          "100.103.121.7:4200"

#define ADMIRAL_PORT_S2                     6767
#define ADMIRAL_HOST_S2                     "100.113.240.39" // mirage
#define ADMIRAL_ENDPOINT_S2                 "100.113.240.39:6767"

#define ADMIRAL_PORT_GIBSON                 8800
#define ADMIRAL_HOST_GIBSON                 "100.113.240.39" // mirage
#define ADMIRAL_ENDPOINT_GIBSON             "100.113.240.39:8800"

typedef enum {
    LMP_ADMIRAL_SERVICE_ADMIRAL,
    LMP_ADMIRAL_SERVICE_RECEPTION,
    LMP_ADMIRAL_SERVICE_S2,
    LMP_ADMIRAL_SERVICE_GIBSON,
    LMP_ADMIRAL_SERVICE_LAITT,
    LMP_ADMIRAL_SERVICE_LIGHTS,
    LMP_ADMIRAL_SERVICE_COUNT,
    LMP_ADMIRAL_SERVICE_NONE
} lmp_admiral_service;

extern char* lmp_admiral_services[];

b8  lmp_admiral_service_handshake(lmp_admiral_service service, u32 fd);

#endif // LIBLMP_H
