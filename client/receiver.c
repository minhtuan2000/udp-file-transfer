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

#include "receiver.h"
#include "../common/url_utils.h"
#include "../common/cli_utils.h"
#include "../common/logger.h"

int file_size = -1;
int file[MAX_FILE_SIZE];
char file_buf[MAX_FILE_SIZE];
int receiver_port;
pthread_t receiver_process;

int launch_receiver()
{
    if (pthread_create(&receiver_process, NULL, _receive_file, NULL))
    {
        error("Failed to create thread for a receiver.");
        return RECEIVE_FAILED;
    }
    return RECEIVE_SUCEEDED;
}

void *_receive_file()
{
    int port = receiver_port;
    char message[CHUNK_SIZE];
    info("Initiating a data receiver...");
    sockaddr_in socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons((uint16_t)port);
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t socket_address_len = sizeof(socket_address);
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        error("Can't open a socket.");
        return NULL;
    };
    debug("Opened a socket");
    // Bind the socket to the port
    int res = bind(sockfd, (sockaddr *)&socket_address, sizeof(socket_address));
    if (res < 0)
    {
        error("Can't bind the socket to the port.");
        return NULL;
    }
    sprintf(message, "Socket binded to the port %d.", port);
    debug(message);
    while (1)
    {
        char *input = malloc(MAX_FILE_SIZE);
        // Receive message
        int byte_count = (int)recvfrom(sockfd, input, MAX_FILE_SIZE, 0, (sockaddr *)&socket_address, &socket_address_len);
        if (byte_count < 0)
        {
            error("Failed to receive message.");
            return NULL;
        }
        else
        {
            sprintf(message, "Received %d bytes from client.", byte_count);
            info(message);
            _handle(input);
        }
        free(input);
    }
    // Close the socket at the end
    close(sockfd);
    return NULL;
}

int _is_not_finished()
{
    for (int i = 0; i < file_size; i++)
    {
        if (file[i] == 0)
        {
            return 1;
        }
    }
    return 0;
}

void _get_next_chunk(int *next_chunk, int *next_chunk_size)
{
    char message[CHUNK_SIZE];
    for (int i = 0; i < file_size; i++)
    {
        if (file[i] == 0)
        {
            *next_chunk = i;
            sprintf(message, "Found next chunk: %d", *next_chunk);
            debug(message);
            *next_chunk_size = -1;
            for (int j = i; j < file_size; j++)
            {
                if (file[j])
                {
                    *next_chunk_size = j - i;
                    break;
                }
            }
            if (*next_chunk_size == -1)
            {
                *next_chunk_size = file_size - i;
            }
            sprintf(message, "Found next chunk size: %d", *next_chunk_size);
            debug(message);
            break;
        }
    }
}

int _handle(char *input)
{
    char message[CHUNK_SIZE];
    int pos = ntohs(((uint8_t)input[0]) | (((uint16_t)input[1]) << 8));
    int size = ntohs(((uint8_t)input[2]) | (((uint16_t)input[3]) << 8));
    sprintf(message, "Handling data from %d with size %d...", pos, size);
    info(message);
    for (int i = pos; i < pos + size; i++)
    {
        if (file[i] == 0)
        {
            int j = i;
            while (j < pos + size && file[j] == 0)
                j++;
            int new_size = j - i;
            memcpy(file_buf + i, input + 4 + i - pos, (size_t)new_size);
            for (int k = i; k < j; k++)
                file[k] = 1;
            file[i] = new_size;
            sprintf(message, "Filled in data from %d to %d...", i, j);
            debug(message);
        }
    }
    return HANDLE_SUCEEDED;
}

int _merge_file(char *file_name)
{
    info("Merging data...");
    char file_path[100];
    sprintf(file_path, "download/%s", file_name);
    FILE *fout = fopen(file_path, "w");
    fwrite(file_buf, 1, file_size, fout);
    fclose(fout);
    info("Finished");
    return LAUNCH_SUCEEDED;
}
