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

    struct sockaddr_in laittAddr = {0};
    laittAddr.sin_family = AF_INET;
    laittAddr.sin_port = htons(ADMIRAL_PORT_LAITT);
    laittAddr.sin_addr.s_addr = inet_addr(ADMIRAL_HOST_LAITT);

    int c = connect(fd, (struct sockaddr*)&laittAddr, sizeof(laittAddr));
    if (c == -1) {
        fprintf(stderr, "Failed to connect to laitt\n");
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
        string8 payload = str8_lit("{\"state\":\"ON\",\"brightness\":254}");
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "brighter") == 0) {
        string8 payload = str8_lit("{\"state\":\"ON\",\"brightness\":191.25}");
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "darker") == 0) {
        string8 payload = str8_lit("{\"state\":\"ON\",\"brightness\":127.50}");
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "darkest") == 0) {
        string8 payload = str8_lit("{\"state\":\"ON\",\"brightness\":63.75}");
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }


    if (strcmp(argv[1], "warmer") == 0) {
        string8 payload = str8_lit("{\"color\":{\"x\":0.5056,\"y\":0.4152},\"color_temp\":454,\"state\":\"ON\"}");
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "warmest") == 0) {
        string8 payload = str8_lit("{\"color\":{\"x\":0.5267,\"y\":0.4133},\"color_temp\":500,\"state\":\"ON\"}");
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "coolest") == 0) {
        string8 payload = str8_lit("{\"color\":{\"x\":0.3131,\"y\":0.3232},\"color_temp\":153,\"state\":\"ON\"}");
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "cooler") == 0) {
        string8 payload = str8_lit("{\"color\":{\"x\":0.3804,\"y\":0.3767},\"color_temp\":250,\"state\":\"ON\"}");
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "standard") == 0) {
        string8 payload = str8_lit("{\"color\":{\"x\":0.4599,\"y\":0.4106},\"color_temp\":370,\"state\":\"ON\"}");
        packet.payload = payload.str;
        packet.payload_length = payload.length;
        lmp_net_send_packet(fd, &packet, &result);
        if (result.error != LMP_ERR_NONE) {
            fprintf(stderr, "Error: Could not send packet\n");
        }
        return 0;
    }

    if (strcmp(argv[1], "blue") == 0) {
        printf("FUCK YEAH\n");
        return 0;
    }

    fprintf(stderr, "Error: Bad command\n");
    close(fd);

    return 1;
}
