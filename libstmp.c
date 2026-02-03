#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include "libstmp.h"

const char* stmp_log_print_type_colors[] = {
    STMP_LOG_COLOR_INFO,
    STMP_LOG_COLOR_WARN,
    STMP_LOG_COLOR_ERROR
};

stmp_error stmp_net_send_packet(u32 fd, const stmp_packet* packet, stmp_result* result) {
    u8 buffer[STMP_PACKET_MAX_SIZE];
    stmp_packet_serialize(buffer, sizeof(buffer), packet, result);
    if (result->error != STMP_ERR_NONE) {
        return result->error;
    }

    ssize_t sent = send(fd, buffer, result->size, 0);
    if (sent < 0) {
        return STMP_ERR_BAD_INPUT;
    }

    return result->error;
}

stmp_error stmp_net_recv_packet(u32 fd, u8* buffer, size_t size, stmp_packet* packet, stmp_result* result) {
    u8 terminated = 0;
    u8 invalid = 0;
    u64 packet_size = 0;

    for (;;) {
        u8 scratch[STMP_PACKET_MAX_SIZE];

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

            if (scratch[i] == STMP_PACKET_TERMINATE) {
                terminated = 1;
                break;
            }
        }

        if (terminated || invalid) break;
    }

    if (terminated) {
        stmp_packet_deserialize(buffer, packet_size, packet, result);
        return result->error;
    }

    if (invalid) {
        return STMP_ERR_BAD_SIZE;
    }

    return STMP_ERR_BAD_INPUT;
}

void stmp_log_print(const char* service, const char* message, stmp_log_print_type type) {
	time_t timestamp = time(NULL);
	struct tm* time_info = localtime(&timestamp);

    switch (type) {
        case INFO:
            fprintf(stderr, "%s[%s] %d:%d:%d [INFO]: %s%s\n", stmp_log_print_type_colors[type], service, time_info->tm_hour, time_info->tm_min, time_info->tm_sec, message, STMP_LOG_COLOR_RESET);
            return;
        case WARN:
            fprintf(stderr, "%s[%s] %d:%d:%d [WARN]: %s%s\n", stmp_log_print_type_colors[type], service, time_info->tm_hour, time_info->tm_min, time_info->tm_sec, message, STMP_LOG_COLOR_RESET);
            return;
        case ERROR:
            fprintf(stderr, "%s[%s] %d:%d:%d [ERROR]: %s%s\n", stmp_log_print_type_colors[type], service, time_info->tm_hour, time_info->tm_min, time_info->tm_sec, message, STMP_LOG_COLOR_RESET);
            return;
    }
}

void stmp_admiral_queue_init(stmp_admiral_queue* queue, u8 capacity) {
    mem_arena* arena = arena_create(MiB(10));
    queue->arena = arena;
    queue->size = 0;
    queue->capacity = capacity;

    queue->messages = arena_push(queue->arena, sizeof(stmp_admiral_message*) * capacity);
}

void stmp_admiral_queue_enqueue(stmp_admiral_queue* queue, const stmp_admiral_message* message) {
    if (queue->size >= queue->capacity) {
        return;
    }

    stmp_admiral_message* allocated = arena_push(queue->arena, sizeof(stmp_admiral_message));
    memcpy(allocated, message, sizeof(*message));

    queue->messages[queue->size++] = allocated;
}

stmp_admiral_message* stmp_admiral_queue_dequeue(stmp_admiral_queue* queue) {
    if (queue->size == 0) {
        return NULL;
    }

    stmp_admiral_message* msg = queue->messages[queue->size - 1];

    queue->size--;

    return msg;
}

stmp_error stmp_admiral_parse_and_queue_packet(stmp_admiral_queue* queue, stmp_packet* packet) {
    static u8 id = 1;

    u8 destination = packet->payload[0];
    u8 sender = packet->payload[1];

    if ((destination < HOTEL || destination > SCHEDULER) ||
        (sender < HOTEL || destination > SCHEDULER)) {
        return STMP_ERR_BAD_PAYLOAD;
    }

    stmp_admiral_priority priority = packet->payload[2];

    if (priority < LOW || priority > HIGH) return STMP_ERR_BAD_PAYLOAD;

    stmp_admiral_message message = {id, destination, sender, priority, packet};
    // NOTE(laith): the lifetime of this scope does not end until enqueue is called.
    // enqueue memcpy's the value of the stack allocated message, so there is no need to allocate
    // it here
    stmp_admiral_queue_enqueue(queue, &message);
    stmp_log_print("admiral", "Recieved and added message to queue", INFO);
    id++;

    return STMP_ERR_NONE;
}

void stmp_admiral_invalidate_packet(stmp_packet* packet) {
    packet->type = STMP_TYPE_INVALID;
    packet->arg = STMP_ARG_INVALID_PAYLOAD;
    packet->flags = STMP_FLAGS_NONE;
    packet->payload = STMP_PAYLOAD_EMPTY;
    packet->payload_length = 1;
}
