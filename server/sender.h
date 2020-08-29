#ifndef SENDER_H
#define SENDER_H

#define SENDER_OK 0
#define SENDER_FAILURE 1

#define MAX_DATA_SIZE 2048

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;

typedef struct file_to_send
{
    char *host;
    int port;
    char *file_buf;
    int chunk;
    int chunk_size;
} file_to_send;

/**
 * Launch a file sender
 */
void launch_sender(char *host, int port, char *file_buf, int chunk, int chunk_size);

/**
 * Send file 
 */
void *send_file();

in_addr_t _establish_connection(char *host, int port, int *sockfd);

#endif
