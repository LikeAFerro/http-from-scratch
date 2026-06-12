#ifndef ASSETS_H
#define ASSETS_H

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 4096
#define N 1

typedef enum {
    SOCKET_ERROR,
    REQUEST_ERROR,
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404
} http_status_t;

http_status_t http_server();

#endif // ASSETS_H
