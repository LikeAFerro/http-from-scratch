#include "assets.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

http_status_t http_server() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return SOCKET_ERROR;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        return SOCKET_ERROR;
    }

    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        return SOCKET_ERROR;
    }

    if (listen(fd, N) != 0) {
        return SOCKET_ERROR;
    }

    printf("Server is listening on %s:%d\n", SERVER_IP, PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd == -1) {
            return SOCKET_ERROR;
        }
        char buffer[BUFFER_SIZE];
        ssize_t bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) {
            continue; // Ignore errors and continue accepting new connections
        }
        buffer[bytes] = '\0'; // Null-terminate the buffer
        printf("Received request:\n%s\n", buffer);

        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        send(client_fd, response, strlen(response), 0);

        close(client_fd);
    }
}
