#ifndef URL_UTILS_H
#define URL_UTILS_H

// parse_url error codes
#define PARSE_URL_OK 0
#define PARSE_URL_NO_SLASH 1
#define PARSE_URL_PROTOCOL_UNKNOWN 2
#define PARSE_URL_INVALID_HOST 3
#define PARSE_URL_INVALID_PORT 4

#define PROTOCOL_SIZE 10
#define HOST_SIZE 100
#define PATH_SIZE 512
#define FILE_NAME_SIZE 256

typedef struct url_info
{
    char protocol[PROTOCOL_SIZE];
    char host[HOST_SIZE];
    int port;
    char path[PATH_SIZE];
} url_info;

/**
 * Parse the URL
 */
int parse_url(char *input, url_info *url);

/**
 * Print information about the URL
 */
void print_url_info(url_info *url);

int _parse_protocol(char **input, url_info *url);
int _parse_host(char **input, url_info *url);
int _parse_port(char **input, url_info *url);
int _parse_path(char **input, url_info *url);

#endif
