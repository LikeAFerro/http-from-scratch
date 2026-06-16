#include "assets.h"
#include <stdio.h>

// TODO:
// - Default page (index.html)
// - Content-Type lookup table
// - Browser tests (e.g., curl, Firefox, Chrome)
// - Break-it challenge

int main(int argc, char *argv[])
{
    http_server_t server;
    http_status_t status = http_server_config(argc, argv, &server);
    if (status != OK) {
        switch (status) {
        case HELP:
            printf("Usage: %s [-p port] [-r root_dir]\n", argv[0]);
            printf("Options:\n");
            printf("  -p port      Specify the port to listen on (default: %d)\n", DEFAULT_PORT);
            printf(
                "  -r root_dir  Specify the root directory to serve files from (default: ./www)\n");
            printf("  -h           Show this help message\n");
            printf("Example: %s -p 8080 -r ./www\n", argv[0]);
            return 0;
        default:
            fprintf(stderr, "Error parsing command-line arguments.\n");
            return 1;
        }
    }

    status = http_server(&server);
    if (status != OK) {
        fprintf(stderr, "Error: %d\n", status);
        return 1;
    }
    return 0;
}
