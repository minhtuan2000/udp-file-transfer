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

#include "sender.h"
#include "../common/url_utils.h"
#include "../common/cli_utils.h"
#include "../common/logger.h"

void launch_sender(char *host, int port, char *file_buf, int chunk, int chunk_size)
{
    file_to_send args;
    args.host = host;
    args.port = port;
    args.file_buf = file_buf;
    args.chunk = chunk;
    args.chunk_size = chunk_size;
    pthread_t sender;
    if (pthread_create(&sender, NULL, send_file, &args))
    {
        error("Failed to create a file sender.");
    }
};

void *send_file(file_to_send *args)
{
    char message[CHUNK_SIZE];
    info("Initiating file sender...");
    struct sockaddr_in socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons((uint16_t)args->port);
    // Create a socket
    int sockfd = -1;
    socket_address.sin_addr.s_addr = _establish_connection(args->host, args->port, &sockfd);
    if (sockfd < 0)
    {
        error("Can't open a socket.");
        return NULL;
    };
    if (socket_address.sin_addr.s_addr == INADDR_NONE)
    {
        error("No IP address found.");
        return NULL;
    }
    //Send data
    while (args->chunk_size > 0)
    {
        int next_size = MAX_DATA_SIZE;
        if (args->chunk_size + 4 < next_size)
        {
            next_size = args->chunk_size + 4;
        }
        sprintf(message, "Chunk: %d.", args->chunk);
        debug(message);
        sprintf(message, "Chunk size: %d.", next_size - 4);
        debug(message);
        char *data = malloc(next_size);
        uint16_t chunk_network = htons((uint16_t)args->chunk);
        data[0] = chunk_network & 0xFF;
        data[1] = chunk_network >> 8;
        uint16_t next_size_network = htons((uint16_t)next_size - 4);
        data[2] = next_size_network & 0xFF;
        data[3] = next_size_network >> 8;
        memcpy(data + 4, args->file_buf + args->chunk, next_size - 4);
        int byte_count = sendto(sockfd, data, next_size, 0, (struct sockaddr *)&socket_address, sizeof(socket_address));
        sprintf(message, "Sent %d bytes.", byte_count);
        info(message);
        if (byte_count != next_size)
        {
            error("Can't send data.");
            info("Retrying...");
        }
        else
        {
            args->chunk_size -= byte_count + 4;
            args->chunk += byte_count - 4;
        }
        free(data);
    }
    info("Finished sending file, shutdown sender.");
    // Close the socket at the end
    close(sockfd);
};

in_addr_t _establish_connection(char *host, int port, int *sockfd)
{
    char message[CHUNK_SIZE];

    addrinfo hints, *res, *ires;
    in_addr_t address;
    memset(&hints, 0, sizeof(hints)); // Set all variable to null
    hints.ai_family = AF_INET;        // Allow ipv4 only
    hints.ai_socktype = SOCK_DGRAM;   // Allow UDP only

    char strport[5];
    sprintf(strport, "%d", port);

    int tmp = getaddrinfo(host, strport, &hints, &res);
    if (tmp != 0)
    {
        error("Could not obtain the IP address.");
        return INADDR_NONE;
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
            address = ((sockaddr_in *)ires->ai_addr)->sin_addr.s_addr;
            info("Established connection successfully.");
            break;
        }
    }
    if (tmp != 0)
    {
        sprintf(message, "Failed to connect to host %s at port %s.", host, strport);
        error(message);
        return INADDR_NONE;
    }
    else
    {
        sprintf(message, "Connected to host %s at port %s.", host, strport);
        info(message);
    }
    freeaddrinfo(res);
    return address;
}
