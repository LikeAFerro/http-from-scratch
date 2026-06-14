#ifndef ASSETS_H
#define ASSETS_H

#include <string.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 4096
#define METHOD_SIZE 8
#define PATH_SIZE 256
#define HEADER_SIZE 512
#define VERSION_SIZE 16
#define BAD_REQUEST_RESPONSE "HTTP/1.1 400 Bad Request\r\nContent-Length: 11\r\n\r\nBad Request"
#define NOT_FOUND_RESPONSE "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found"
#define N 1

typedef enum { SOCKET_ERROR, OK = 200, BAD_REQUEST = 400, NOT_FOUND = 404 } http_status_t;

typedef struct {
    char method[METHOD_SIZE];
    char path[PATH_SIZE];
    char version[VERSION_SIZE];
} http_request_t; // For simplicity, we won't parse the headers and only handle GET requests.

typedef struct {
    char version[VERSION_SIZE];
    http_status_t status_code;
    size_t content_length;
    char *body;
} http_response_t;

http_status_t http_server();
http_status_t parse_request(const char *request_str, http_request_t *request);
http_status_t handle_request(const http_request_t *request, http_response_t *response);
http_status_t build_status_line(const http_request_t *request, http_response_t *response);

#endif // ASSETS_H
