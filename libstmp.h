// ===============================================================
// This library contains all and everything STMP
// ===============================================================

#ifndef LIBSTMP_H
#define LIBSTMP_H

#include "lt_base.h"
#define LT_ARENA_IMPLEMENTATION
#include "lt_arena.h"
#include "stmp.h"
#include <stddef.h>

// ===============================================================
// Net
// ===============================================================
stmp_error stmp_net_send_packet(u32 fd, const stmp_packet* packet, stmp_result* result);
stmp_error stmp_net_recv_packet(u32 fd, u8* buffer, size_t size, stmp_packet* packet, stmp_result* result);

// ===============================================================
// Log
// ===============================================================
#define STMP_LOG_COLOR_INFO "\x1b[34m"  // Blue
#define STMP_LOG_COLOR_WARN "\x1b[33m"  // Yellow
#define STMP_LOG_COLOR_ERROR "\x1b[31m"  // Red
#define STMP_LOG_COLOR_RESET "\x1b[0m"

typedef enum {
    INFO,
    WARN,
    ERROR
} stmp_log_print_type;

void stmp_log_print(const char* service, const char* message, stmp_log_print_type type);

// ===============================================================
// Admiral
// ===============================================================
typedef enum {
    LOW,
    MEDIUM,
    HIGH
} stmp_admiral_priority;

typedef struct {
    u8 id;
    u8 destination;
    u8 sender;
    // bool ack;
    stmp_admiral_priority priority;
    stmp_packet* packet;
} stmp_admiral_message;

typedef struct {
    mem_arena* arena;
    stmp_admiral_message** messages;
    u8 size;
    u8 capacity;
} stmp_admiral_queue;

// NOTE(laith): this will be updated to add all the services that admiral will support
// put new endpoints in between hotel and scheduler
typedef enum {
    ADMIRAL,
    HOTEL,
    SCHEDULER
} stmp_admiral_endpoint;

// typedef enum {
//     // send an ack recieved back to the client
//     ACK,
//     // send a disconnect to the client
//     DISCONNECT,
//     // send a message to the client
//     MESSAGE,
//     // send a ping back to the client
//     PING,
// } stmp_admiral_action;

void stmp_admiral_queue_init(stmp_admiral_queue* queue, u8 capacity);
void stmp_admiral_queue_enqueue(stmp_admiral_queue* queue, const stmp_admiral_message* message);
stmp_admiral_message* stmp_admiral_queue_dequeue(stmp_admiral_queue* queue);
stmp_error stmp_admiral_parse_and_queue_packet(stmp_admiral_queue* queue, stmp_packet* packet);
void stmp_admiral_invalidate_packet(stmp_packet* packet);

// service sends admiral an init init
// admiral sends an init accept back

// admiral sends an init init
// service sends init accept back

#endif // LIBSTMP_H
