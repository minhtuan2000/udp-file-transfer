#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "requester.h"
#include "../common/url_utils.h"
#include "../common/cli_utils.h"
#include "../common/logger.h"

int send_request(url_info *url)
{
    return send_request_and_get_file_size(url, NULL);
}

int send_request_and_get_file_size(url_info *url, int *file_size)
{
    info("Initiating client...");
    http_reply reply;
    int ret = _send_http_request(url, &reply);
    if (ret)
    {
        error("Failed to send request.");
        return SEND_REQUEST_FAILED;
    }
    // Now parse the responses
    char *response = _read_http_reply(&reply);
    if (response == NULL)
    {
        error("Failed to receive response.");
        return SEND_REQUEST_FAILED;
    }
    if (strncmp(response, "OK", 2) == 0)
    {
        info(response);
        if (file_size != NULL)
        {
            sscanf(response + 2, "%d", file_size);
        }
    }
    else
    {
        error(response);
        return SEND_REQUEST_FAILED;
    }
    // Free buffer
    free(reply.reply_buffer);
    return SEND_REQUEST_OK;
}

int _send_http_request(url_info *url, http_reply *reply)
{
    info("Sending HTTP request...");
    int tmp;
    int sockfd = -1;

    tmp = _establish_connection(url, &sockfd);
    if (tmp)
    {
        error("Failed to establish connection.");
        return tmp;
    }
    tmp = _write_request_to_socket(url, sockfd);
    if (tmp)
    {
        error("Failed to write request to socket.");
        return tmp;
    }
    tmp = _read_response_from_socket(reply, sockfd);
    if (tmp)
    {
        error("Failed to receive response from socket.");
        return tmp;
    }
    return SEND_REQUEST_OK;
}

int _establish_connection(url_info *url, int *sockfd)
{
    char message[CHUNK_SIZE];

    addrinfo hints, *res, *ires;
    memset(&hints, 0, sizeof(hints)); // Set all variable to null
    hints.ai_family = AF_UNSPEC;      // Allow both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;  // Allow TCP only

    char strport[5];
    sprintf(strport, "%d", url->port);

    int tmp = getaddrinfo(url->host, strport, &hints, &res);
    if (tmp != 0)
    {
        error("Could not obtain the IP address.");
        return SEND_REQUEST_FAILED;
    }
    else
    {
        info("Successfully obtained the available IP addresses.");
    }
    for (ires = res; ires != NULL; ires = ires->ai_next)
    {
        info("Connecting...");
        *sockfd = socket(ires->ai_family, ires->ai_socktype, ires->ai_protocol);
        if (*sockfd == -1)
        {
            info("Failed to establish connection, retrying...");
            continue;
        }
        tmp = connect(*sockfd, ires->ai_addr, ires->ai_addrlen);
        if (tmp == 0)
        {
            info("Established connection successfully.");
            break;
        }
    }
    if (tmp != 0)
    {
        sprintf(message, "Failed to connect to host %s at port %s.", url->host, strport);
        error(message);
        return SEND_REQUEST_FAILED;
    }
    else
    {
        sprintf(message, "Connected to host %s at port %s.", url->host, strport);
        info(message);
    }
    freeaddrinfo(res);
    return SEND_REQUEST_OK;
}

int _write_request_to_socket(url_info *url, int sockfd)
{
    info("Writing request to socket...");
    char message[CHUNK_SIZE];
    char *request = _generate_http_get_request(url);              // Obtain the request string
    ssize_t byte_count = write(sockfd, request, strlen(request)); // Write request to socket
    shutdown(sockfd, SHUT_WR);                                    // Shutdown
    free(request);                                                // Free buffer
    if (byte_count == -1)
    {
        error("Failed to write request to socket.");
        return SEND_REQUEST_FAILED;
    }
    else
    {
        sprintf(message, "Writen %li bytes to socket", byte_count);
        info(message);
    }
    return SEND_REQUEST_OK;
}

char *_generate_http_get_request(url_info *info)
{
    char *request_buffer = (char *)malloc(100 + strlen(info->path) + strlen(info->host));
    // HTTP Protocol
    snprintf(request_buffer, 1024,
             "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
             info->path, info->host);
    return request_buffer;
}

int _read_response_from_socket(http_reply *reply, int sockfd)
{
    info("Reading response from socket...");
    char message[CHUNK_SIZE];
    reply->reply_buffer_length = 0;
    reply->reply_buffer = (char *)malloc(0);
    while (1)
    {
        char *response = (char *)malloc(CHUNK_SIZE); // Read one CHUNK_SIZE of 2048 bytes at a time
        memset(response, 0, CHUNK_SIZE);
        ssize_t byte_count = recv(sockfd, response, CHUNK_SIZE, 0);
        if (byte_count == -1)
        {
            error("Failed to receive response.");
            return SEND_REQUEST_FAILED;
        }
        else if (byte_count == 0)
        {
            debug(reply->reply_buffer);
            info("Done.");
            break;
        }
        else
        {
            sprintf(message, "Received %li bytes.", byte_count);
            info(message);
            debug("Reallocating reply buffer...");
            // Reallocate reply buffer
            reply->reply_buffer = (char *)realloc(reply->reply_buffer, reply->reply_buffer_length + byte_count);
            // Append response to reply buffer
            memcpy(reply->reply_buffer + reply->reply_buffer_length, response, byte_count);
            reply->reply_buffer_length += byte_count;
        }
        free(response); // Free response
    }
    return SEND_REQUEST_OK;
}

char *_next_line(char *buff, int len)
{
    if (len == 0)
    {
        return NULL;
    }
    char *last = buff + len - 1;
    while (buff != last)
    {
        if (*buff == '\r' && *(buff + 1) == '\n')
        {
            return buff;
        }
        buff++;
    }
    return NULL;
}

char *_read_http_reply(http_reply *reply)
{
    char message[CHUNK_SIZE];
    // Find the first line starts with "HTTP"
    char *start_line = strstr(reply->reply_buffer, "HTTP");
    debug(start_line);
    // Then isolate the first line of the reply
    char *status_line = _next_line(start_line, strlen(start_line));
    if (status_line == NULL)
    {
        error("Could not find status.");
        return NULL;
    }
    *status_line = '\0'; // Make the first line is a null-terminated string
    // Now let's read the status (parsing the first line)
    int status;
    double http_version;
    int rv = sscanf(start_line, "HTTP/%lf %d", &http_version, &status);
    if (rv != 2)
    {
        sprintf(message, "Could not parse http response first line (rv=%d, %s).", rv, reply->reply_buffer);
        error(message);
        return NULL;
    }
    if (status == 302 || status == 301)
    { // Redirecting
        // Parse location
        char *location = strstr(status_line + 2, "Location: ");
        if (location == NULL)
        {
            error("Redirected location not found.");
            return NULL;
        }
        location += strlen("Location: ");
        char *end_location = _next_line(location, strlen(location));
        end_location[0] = '\0';
        sprintf(message, "Server returned status %d, redirecting to %s.", status, location);
        info(message);
        // Parse the URL
        url_info url;
        int ret = parse_url(location, &url);
        if (ret)
        {
            sprintf(message, "Could not parse URL '%s.", location);
            error(message);
            return NULL;
        }
        return _redirect_client(&url);
    }
    if (status != 200)
    {
        sprintf(message, "Server returned status %d (should be 200)\n", status);
        error(message);
        return NULL;
    }
    char *buf = status_line + 2;
    char *line = _next_line(buf, strlen(buf));
    while (line[2] != '\r')
    {
        buf = line + 2;
        line = _next_line(buf, strlen(buf));
    }
    buf = line + 4;
    debug(buf);
    return buf;
}

char *_redirect_client(url_info *url)
{
    info("Redirecting client...");
    http_reply reply;
    int ret = _send_http_request(url, &reply);
    if (ret)
    {
        error("Failed to send request.");
        return NULL;
    }
    // Now parse the responses
    char *response = _read_http_reply(&reply);
    if (response == NULL)
    {
        error("Failed to receive response.");
        return NULL;
    }
    return response;
}
