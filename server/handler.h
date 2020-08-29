#ifndef HANDLER_H
#define HANDLER_H

#define HANDLER_OK 0
#define HANDLER_FAILURE 1

#define MAX_BACKLOG 65536
#define TIMEOUT_SECOND 1

typedef struct addrinfo addrinfo;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

typedef struct http_request
{
    char *request_buffer;
    int request_buffer_length;
} http_request;

/**
 * Launch a request handler
 */
int launch_handler(int port);

int _establish_listener(int *sockfd, int port);
int _start_listener(int sockfd);
int _handle_request(http_request *request, int incomingfd);
void _send_response(int incomingfd, char *message);

#endif
