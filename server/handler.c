#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>

#include "handler.h"
#include "sender.h"
#include "../common/url_utils.h"
#include "../common/cli_utils.h"
#include "../common/logger.h"

int launch_handler(int port)
{
    int sockfd, tmp;
    tmp = _establish_listener(&sockfd, port);
    if (tmp)
    {
        error("Failed to establish listener");
        return HANDLER_FAILURE;
    }
    tmp = _start_listener(sockfd);
    if (tmp)
    {
        error("Exception occurs while listening.");
        return HANDLER_FAILURE;
    }
    return HANDLER_OK;
}

int _establish_listener(int *sockfd, int port)
{
    info("Establishing a listener...");
    addrinfo hints, *res, *ires;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // Accept both ipv4 and ipv6
    hints.ai_socktype = SOCK_STREAM; // TCP only
    hints.ai_flags = AI_PASSIVE;     // For server
    char strport[5];
    sprintf(strport, "%d", port);
    if (getaddrinfo(NULL, strport, &hints, &res))
    {
        error("Failed to get address information.");
        return HANDLER_FAILURE;
    }
    for (ires = res; ires != NULL; ires = ires->ai_next)
    {
        *sockfd = socket(ires->ai_family, ires->ai_socktype, 0);
        // Set timeout
        struct timeval tv;
        tv.tv_sec = TIMEOUT_SECOND;
        tv.tv_usec = 0;
        setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
        // Set reusable port
        int option = 1;
        setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (*sockfd == -1)
        {
            continue;
        }
        if (bind(*sockfd, ires->ai_addr, ires->ai_addrlen) == 0)
        {
            break;
        }
    }
    if (ires == NULL)
    {
        error("Failed to open or bind a socket.");
        return HANDLER_FAILURE;
    }
    freeaddrinfo(res);
    return HANDLER_OK;
}

int _start_listener(int sockfd)
{
    info("Start listening...");
    char message[CHUNK_SIZE];
    if (listen(sockfd, MAX_BACKLOG))
    {
        error("Failed to start listening.");
        return HANDLER_FAILURE;
    }
    sockaddr_in address;
    socklen_t address_length = sizeof(address);
    while (1)
    {
        int incomingfd = accept(sockfd, (sockaddr *)&address, &address_length);
        if (incomingfd < 0)
        {
            debug("Waiting for connection...");
            continue;
        }
        info("Accepted a connection.");
        http_request request;
        request.request_buffer_length = 0;
        request.request_buffer = (char *)malloc(0);
        while (1)
        {
            char *input = (char *)malloc(CHUNK_SIZE); // Read one CHUNK_SIZE of 2048 bytes at a time
            memset(input, 0, CHUNK_SIZE);
            ssize_t byte_count = recv(incomingfd, input, CHUNK_SIZE, 0);
            if (byte_count == -1)
            {
                if (request.request_buffer_length > 0)
                {
                    debug(request.request_buffer);
                    break;
                }
                error("Failed to receive request.");
                info("Ignoring...");
            }
            else if (byte_count == 0)
            {
                info("Done.");
                break;
            }
            else
            {
                sprintf(message, "Received %li bytes.", byte_count);
                info(message);
                debug("Reallocating request buffer...");
                // Reallocate request buffer
                request.request_buffer = (char *)realloc(request.request_buffer, request.request_buffer_length + byte_count);
                // Append request to request buffer
                memcpy(request.request_buffer + request.request_buffer_length, input, byte_count);
                request.request_buffer_length += byte_count;
            }
            free(input); // Free request
        }
        int tmp = _handle_request(&request, incomingfd);
        if (tmp)
        {
            error("Failed to handle request.");
            info("Ignoring...");
        }
        shutdown(incomingfd, SHUT_RDWR); // Close connection
        close(incomingfd);
    }
    return HANDLER_OK;
}

int _handle_request(http_request *request, int incomingfd)
{
    char message[CHUNK_SIZE];
    info("Handling request...");
    char *tmp;
    char method[10];
    tmp = strtok(request->request_buffer, " ");
    strcpy(method, tmp);
    if (strcmp(method, "GET"))
    {
        sprintf(message, "Invalid method %s.", method);
        error(message);
        _send_response(incomingfd, "Error Bad request");
        return HANDLER_OK;
    }
    sprintf(message, "Method: %s.", method);
    debug(message);

    char path[PATH_SIZE];
    tmp = strtok(NULL, " ");
    if (tmp == NULL)
    {
        sprintf(message, "Invalid path.");
        error(message);
        _send_response(incomingfd, "Error Bad request");
        return HANDLER_OK;
    }
    strcpy(path, tmp);
    sprintf(message, "Path: %s.", path);
    debug(message);

    char *_ = strtok(path, "/");
    char file_name[FILE_NAME_SIZE];
    tmp = strtok(NULL, "/");
    if (tmp == NULL)
    {
        sprintf(message, "Invalid file name.");
        error(message);
        _send_response(incomingfd, "Error Bad request");
        return HANDLER_OK;
    }
    strcpy(file_name, tmp);
    sprintf(message, "File name: %s.", file_name);
    debug(message);

    char file_buf[MAX_BACKLOG];
    char file_path[PATH_SIZE];
    long file_size = -1;
    sprintf(file_path, "files/%s", file_name);
    if (access(file_path, F_OK) != -1)
    {
        FILE *file = fopen(file_path, "r");
        if (file)
        {
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            if (file_size > MAX_BACKLOG - 2)
            {
                sprintf(message, "File %s of size %ld is too large.", file_name, file_size);
                error(message);
                _send_response(incomingfd, "Error Target file too large.");
                return HANDLER_OK;
            }
            sprintf(message, "File size: %ld.", file_size);
            debug(message);

            fseek(file, 0, SEEK_SET);
            fread(file_buf, 1, file_size, file);
            fclose(file);
        }
        else
        {
            error("Failed to open file");
            _send_response(incomingfd, "Error Internal error Can not open file.");
            return HANDLER_FAILURE;
        }
    }
    else
    {
        sprintf(message, "File %s does not exist or server does not have permission to read it.", file_name);
        error(message);
        _send_response(incomingfd, "Error No such file");
        return HANDLER_OK;
    }
    char host[HOST_SIZE];
    tmp = strtok(NULL, "/");
    if (tmp == NULL)
    {
        sprintf(message, "Invalid host name.");
        error(message);
        _send_response(incomingfd, "Error Bad request");
        return HANDLER_OK;
    }
    strcpy(host, tmp);
    sprintf(message, "Host name: %s.", host);
    debug(message);

    char portstr[5];
    tmp = strtok(NULL, "/");
    if (tmp == NULL)
    {
        sprintf(message, "Invalid port string.");
        error(message);
        _send_response(incomingfd, "Error Bad request");
        return HANDLER_OK;
    }
    strcpy(portstr, tmp);
    int port;
    if (sscanf(portstr, "%d", &port) == EOF)
    {
        sprintf(message, "Port %s is not valid.", portstr);
        error(message);
        _send_response(incomingfd, "Error Bad request");
        return HANDLER_OK;
    }
    sprintf(message, "Port: %d.", port);
    debug(message);

    int chunk = 0;
    int chunk_size = file_size;
    char chunkstr[5];
    tmp = strtok(NULL, "/");
    if (tmp != NULL)
    {
        strcpy(chunkstr, tmp);
        if (sscanf(chunkstr, "%d", &chunk) == EOF || chunk < 0 || chunk >= file_size)
        {
            sprintf(message, "Starting position %s is not valid.", chunkstr);
            error(message);
            _send_response(incomingfd, "Error Bad request");
            return HANDLER_OK;
        }
        char chunksizestr[5];
        tmp = strtok(NULL, "/");
        if (tmp == NULL)
        {
            sprintf(message, "Invalid chunk size string.");
            error(message);
            _send_response(incomingfd, "Error Bad request");
            return HANDLER_OK;
        }
        strcpy(chunksizestr, tmp);
        if (sscanf(chunksizestr, "%d", &chunk_size) == EOF || chunk_size <= 0 || chunk + chunk_size > file_size)
        {
            sprintf(message, "Chunk size %s is not valid.", chunksizestr);
            error(message);
            _send_response(incomingfd, "Error Bad request");
            return HANDLER_OK;
        }
        _send_response(incomingfd, "OK");
    }
    else
    {
        sprintf(message, "OK %ld", file_size);
        _send_response(incomingfd, message);
    }
    sprintf(message, "Chunk: %d.", chunk);
    debug(message);
    sprintf(message, "Chunk size: %d.", chunk_size);
    debug(message);
    launch_sender(host, port, file_buf, chunk, chunk_size);
    return HANDLER_OK;
}

void _send_response(int incomingfd, char *message)
{
    info("Sending back response...");
    char response[CHUNK_SIZE];
    sprintf(response, "HTTP/1.1 200 OK\r\nServer: Server\r\nConnection: close\r\n\r\n%s", message);
    debug(response);
    send(incomingfd, response, strlen(response), 0);
}
