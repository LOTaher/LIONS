#include "lt_arena.h"
#include "libstmp.h"
#include <stdio.h>

int main(void) {
    stmp_admiral_queue queue;

    stmp_admiral_queue_init(&queue, 50);

    printf("Queue Size: %d\n", queue.size);
    printf("Queue Capacity: %d\n", queue.capacity);
    printf("Arena Capacity: %llu\n", queue.arena->capacity);
    printf("Arena Position: %llu\n", queue.arena->pos);

    stmp_packet packet;
    stmp_packet_init(&packet);
    packet.payload = (u8*)"hello!";

    stmp_admiral_message msg = {1, 1, 2, 0, &packet};

    stmp_admiral_queue_enqueue(&queue, &msg);

    printf("Queue Size: %d\n", queue.size);
    printf("Queue Message: %s\n", queue.messages[0]->packet->payload);

    stmp_admiral_message* msg2 = stmp_admiral_queue_dequeue(&queue);

    printf("Queue Size: %d\n", queue.size);
    printf("Dequeued Message: %d\n", msg2->id);

    return 0;
}
