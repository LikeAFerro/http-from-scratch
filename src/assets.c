#include "assets.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

http_status_t http_server() {
    // Create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return SOCKET_ERROR;
    }

    // Bind and listen
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

    // Main loop to accept and handle requests
    while (1) {
        // Accept a new connection
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd == -1) {
            return SOCKET_ERROR;
        }

        // Read the request
        char request_buffer[BUFFER_SIZE];
        ssize_t bytes = recv(client_fd, request_buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) {
            close(client_fd);
            continue; // Ignore errors and continue accepting new connections
        }
        request_buffer[bytes] = '\0'; // Null-terminate the buffer

        // Parse the request
        http_request_t request;
        http_status_t status = parse_request(request_buffer, &request);
        if (status != OK) {
            send(client_fd, BAD_REQUEST_RESPONSE, strlen(BAD_REQUEST_RESPONSE), 0);
            close(client_fd);
            continue;
        }

        // Handle the request and prepare the response
        http_response_t response;
        response.body = NULL; // Initialize body to NULL
        status = handle_request(&request, &response);
        if (status != OK) {
            switch (status) {
            case BAD_REQUEST:
                send(client_fd, BAD_REQUEST_RESPONSE, strlen(BAD_REQUEST_RESPONSE), 0);
                break;
            case NOT_FOUND:
                send(client_fd, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
                break;
            case MEMORY_ERROR:
                // For memory errors, we can send a 500 Internal Server Error response
                send(client_fd, INTERNAL_ERROR_RESPONSE, strlen(INTERNAL_ERROR_RESPONSE), 0);
                break;
            default:
                send(client_fd, BAD_REQUEST_RESPONSE, strlen(BAD_REQUEST_RESPONSE), 0);
            }
            free(response.body);
            close(client_fd);
            continue;
        }

        // Send the response
        char headers[HEADER_SIZE];
        snprintf(headers,
                 sizeof(headers),
                 "%s %d OK\r\nContent-Length: %zu\r\n\r\n",
                 response.version,
                 response.status_code,
                 response.content_length);
        send(client_fd, headers, strlen(headers), 0);

        if (response.body) {
            send(client_fd, response.body, response.content_length, 0);
        }

        free(response.body);
        close(client_fd);
    }
}

http_status_t parse_request(const char *request_str, http_request_t *request) {
    if (!request_str || !request) {
        return BAD_REQUEST;
    }

    // Parse the request line (e.g., "GET /index.html HTTP/1.1")
    if (sscanf(request_str, "%7s %255s %15s", request->method, request->path, request->version) !=
            3 ||
        request->path[0] != '/') {
        return BAD_REQUEST;
    }

    if (strcmp(request->path, "/") == 0) {
        // Default to index.html if root is requested
        strcpy(request->path, "/index.html");
    }

    // Verify that the method is either GET or HEAD
    if (strcmp(request->method, "GET") != 0 && strcmp(request->method, "HEAD") != 0) {
        return BAD_REQUEST;
    }

    return OK;
}

http_status_t handle_request(const http_request_t *request, http_response_t *response) {
    if (!request || !response || strstr(request->version, "HTTP/") == NULL) {
        return BAD_REQUEST;
    }

    if (strcmp(request->method, "GET") == 0 || strcmp(request->method, "HEAD") == 0) {
        if (strstr(request->path, "..") != NULL) {
            return BAD_REQUEST; // Prevent directory traversal
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "./www%s", request->path);

        // Open the file first, then stat the fd to avoid TOCTOU race
        int file_fd = open(full_path, O_RDONLY | O_NOFOLLOW);
        if (file_fd == -1) {
            response->status_code = NOT_FOUND;
            return NOT_FOUND;
        }

        struct stat st;
        // Check that the opened fd refers to a regular file
        if (fstat(file_fd, &st) != 0 || !S_ISREG(st.st_mode)) {
            close(file_fd);
            response->status_code = NOT_FOUND;
            return NOT_FOUND;
        }

        strcpy(response->version, request->version); // Echo back the HTTP version
        response->content_length = st.st_size;

        // If the method is GET, read the file content into the response body
        if (strcmp(request->method, "GET") == 0) {
            response->body = malloc(response->content_length + 1);
            if (!response->body) {
                close(file_fd);
                return MEMORY_ERROR; // Memory allocation failed
            }

            FILE *file = fdopen(file_fd, "rb");
            if (!file) {
                close(file_fd);
                free(response->body);
                return MEMORY_ERROR; // fdopen failed (resource exhaustion)
            }
            if (fread(response->body, 1, response->content_length, file) !=
                response->content_length) {
                free(response->body);
                fclose(file);        // fclose also closes file_fd
                return MEMORY_ERROR; // Failed to read the entire file
            }
            response->body[response->content_length] = '\0'; // Null-terminate the body
            fclose(file);                                    // fclose also closes file_fd
        } else {
            close(file_fd); // HEAD: fd no longer needed
        }

        response->status_code = OK;
    } else {
        response->status_code = BAD_REQUEST;
    }
    return response->status_code;
}
