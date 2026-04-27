#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// #include "../../lib/c/lt_arena.h"
// #include "../../lib/c/lt_base.h"
// #include "../../lib/c/lmp.h"
#include "../../lib/c/liblmp.h"
#define LT_STRING_IMPLEMENTATION
#include "../../lib/c/lt_strings.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./lightctl [command]\n");
        return 1;
    }

    mem_arena* arena = arena_create(KiB(1));
    string8 admiralPayload = str8_lit("45");

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        fprintf(stderr, "Failed to create socket\n");
        return 1;
    }

    int opt = 1;
    int s = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (s == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_S2, LMP_ADMIRAL_SERVICE_S2, "Failed to set socket option", LMP_PRINT_TYPE_ERROR);
        close(fd);
        return 1;
    }

    struct sockaddr_in localAddr = {0};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(ADMIRAL_PORT_LIGHTCTL);
    localAddr.sin_addr.s_addr = INADDR_ANY;

    int b = bind(fd, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (b == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_LIGHTCTL, LMP_ADMIRAL_SERVICE_LIGHTCTL, "Failed to bind to port", LMP_PRINT_TYPE_ERROR);
        close(fd);
        return 1;
    }

    struct sockaddr_in admiralAddr = {0};
    admiralAddr.sin_family = AF_INET;
    admiralAddr.sin_port = htons(ADMIRAL_PORT_ADMIRAL);
    admiralAddr.sin_addr.s_addr = inet_addr(ADMIRAL_HOST_ADMIRAL);

    int c = connect(fd, (struct sockaddr*)&admiralAddr, sizeof(admiralAddr));
    if (c == -1) {
        lmp_log_print(LMP_ADMIRAL_SERVICE_LIGHTCTL, LMP_ADMIRAL_SERVICE_ADMIRAL, "Failed to connect to admiral", LMP_PRINT_TYPE_ERROR);
        return 1;
    }

    lmp_packet packet;
    lmp_result result;
    lmp_packet_init(&packet);
    lmp_result_init(&result);

    packet.version = lmp_versions[1];
    packet.type = LMP_TYPE_SEND;
    packet.arg = LMP_ARG_SEND;
    packet.flags = LMP_FLAGS_NONE;

    if (strcmp(argv[1], "brightest") == 0) {
        string8 message = str8_lit("{\"state\":\"ON\",\"brightness\":254,\"transition\":0.5}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "brighter") == 0) {
        string8 message = str8_lit("{\"state\":\"ON\",\"brightness\":191,\"transition\":0.5}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "darker") == 0) {
        string8 message = str8_lit("{\"state\":\"ON\",\"brightness\":127,\"transition\":0.5}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "darkest") == 0) {
        string8 message = str8_lit("{\"state\":\"ON\",\"brightness\":63,\"transition\":0.5}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "warmer") == 0) {
        string8 message = str8_lit("{\"color\":{\"x\":0.5056,\"y\":0.4152},\"color_temp\":454,\"brightness\":254,\"transition\":0.5,\"state\":\"ON\"}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "warmest") == 0) {
        string8 message = str8_lit("{\"color\":{\"x\":0.5267,\"y\":0.4133},\"color_temp\":500,\"brightness\":254,\"transition\":0.5,\"state\":\"ON\"}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "coolest") == 0) {
        string8 message = str8_lit("{\"color\":{\"x\":0.3131,\"y\":0.3232},\"color_temp\":153,\"brightness\":254,\"transition\":0.5,\"state\":\"ON\"}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "cooler") == 0) {
        string8 message = str8_lit("{\"color\":{\"x\":0.3804,\"y\":0.3767},\"color_temp\":250,\"brightness\":254,\"transition\":0.5,\"state\":\"ON\"}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "standard") == 0) {
        string8 message = str8_lit("{\"color\":{\"x\":0.4599,\"y\":0.4106},\"color_temp\":370,\"brightness\":254,\"transition\":0.5,\"state\":\"ON\"}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "bed") == 0) {
        string8 message = str8_lit("{\"brightness\":31,\"color\":{\"hue\":25,\"saturation\":95,\"x\":0.5267,\"y\":0.4133},\"color_temp\":498,\"transition\":0.5,\"state\":\"ON\"}");
        string8 payload = str8_concat(admiralPayload, message, arena);
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    // TODO(laith): custom hex colors!

    fprintf(stderr, "Error: Bad command\n");
    close(fd);

    return 1;
}
