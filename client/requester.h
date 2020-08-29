#ifndef REQUESTER_H
#define REQUESTER_H

#define SEND_REQUEST_OK 0
#define SEND_REQUEST_FAILED 1

/* Structure used to store the buffer and buffer length when receiving the reply from an http server. */
typedef struct http_reply
{
    char *reply_buffer;
    int reply_buffer_length;
} http_reply;

typedef struct url_info url_info;
typedef struct addrinfo addrinfo;

/**
 * Send follow-up requests for missing file chunk
 */
int send_request(url_info *url);

/**
 * Send first time request and read the file size
 */
int send_request_and_get_file_size(url_info *url, int *file_size);

int _send_http_request(url_info *url, http_reply *reply);
int _establish_connection(url_info *url, int *sockfd);
int _write_request_to_socket(url_info *url, int sockfd);
char *_generate_http_get_request(url_info *info);
int _read_response_from_socket(http_reply *reply, int sockfd);
char *_next_line(char *buff, int len);
char *_read_http_reply(http_reply *reply);
char *_redirect_client(url_info *url);

#endif
