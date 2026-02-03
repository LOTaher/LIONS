/*  stmp.h - SplaT Middleware Protocol C library definition file
    Copyright (C) 2025 splatte.dev

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

#ifndef STMP_H
#define STMP_H

#include <stddef.h>
#include <stdint.h>

#include "lt_base.h"

typedef u8 stmp_version;
typedef u8 stmp_type;
typedef u8 stmp_arg;
typedef u8 stmp_flag;

/* [0] Version */
static const stmp_version stmp_versions[] = {1, 2};

/* [1] Type */
#define STMP_TYPE_INIT 0x01
#define STMP_TYPE_PING 0x02
#define STMP_TYPE_SEND 0x03
#define STMP_TYPE_TERM 0x04
#define STMP_TYPE_INVALID 0x05

/* [2] Argument */
#define STMP_ARG_INIT_INIT 0x01
#define STMP_ARG_INIT_ACCEPT 0x02
#define STMP_ARG_PING 0x00
#define STMP_ARG_SEND 0x00
#define STMP_ARG_TERM_CLEAN 0x01
#define STMP_ARG_TERM_BUSY 0x02
#define STMP_ARG_INVALID_VERSION 0x01
#define STMP_ARG_INVALID_TYPE 0x02
#define STMP_ARG_INVALID_MESSAGE 0x03
#define STMP_ARG_INVALID_ARGUMENT 0x04
#define STMP_ARG_INVALID_FLAGS 0x05
#define STMP_ARG_INVALID_PAYLOAD 0x06

/* [3] Flags */
#define STMP_FLAGS_NONE 0
#define STMP_FLAGS_LOG (1 << 0)
#define STMP_FLAGS_INCOGNITO (1 << 1)

/* [4] Payload */
#define STMP_PAYLOAD_EMPTY 0x00

/* Packet */
#define STMP_PACKET_HEADER_SIZE 0x04 // 4
#define STMP_PACKET_MAX_SIZE 0x5DC // 1500
#define STMP_PACKET_MIN_SIZE 0x05 // 5
#define STMP_PACKET_TERMINATE 0x7F

typedef enum {
    STMP_ERR_NONE,
    STMP_ERR_BAD_SIZE,
    STMP_ERR_BAD_VERSION,
    STMP_ERR_BAD_TYPE,
    STMP_ERR_BAD_ARG,
    STMP_ERR_BAD_PAYLOAD,
    STMP_ERR_BAD_TERMINATE,
    STMP_ERR_BAD_INPUT
} stmp_error;

typedef struct {
    stmp_version version;
    stmp_type type;
    stmp_arg arg;
    stmp_flag flags;

    const u8* payload;
    size_t payload_length;
} stmp_packet;

typedef struct {
    size_t size;
    stmp_error error;
} stmp_result;

void stmp_packet_init(stmp_packet* packet);
void stmp_result_init(stmp_result* result);
void stmp_packet_serialize(u8* buffer, size_t size, const stmp_packet* packet, stmp_result* result);
void stmp_packet_deserialize(const u8* buffer, size_t size, stmp_packet* packet, stmp_result* result);

#endif // STMP_H
