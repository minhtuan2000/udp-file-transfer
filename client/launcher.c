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

#include "requester.h"
#include "receiver.h"
#include "launcher.h"
#include "../common/url_utils.h"
#include "../common/cli_utils.h"
#include "../common/logger.h"

int launch(url_info *url)
{
    info("Launching...");
    char message[CHUNK_SIZE];
    char file_name[FILE_NAME_SIZE];
    char host[HOST_SIZE];

    int tmp = _parse_query(url->path, file_name, host, &receiver_port);
    if (tmp)
    {
        error("Failed to parse query");
        debug(url->path);
        return LAUNCH_FAILED;
    }
    sprintf(message, "Query: file name %s, receiver host %s, recever port %d", file_name, host, receiver_port);
    info(message);

    launch_receiver();
    while (file_size == -1 || _is_not_finished())
    {
        if (file_size == -1)
        {
            info("Sending first time request...");
            int tmp = send_request_and_get_file_size(url, &file_size);
            if (tmp)
            {
                return LAUNCH_FAILED;
            }
            sprintf(message, "File size: %d", file_size);
            debug(message);
        }
        else
        {
            int next_chunk, next_chunk_size;
            _get_next_chunk(&next_chunk, &next_chunk_size);
            info("Sending request for missing chunk...");
            sprintf(message, "Missing chunk starts from %d with size %d", next_chunk, next_chunk_size);
            debug(message);
            _update_request(url, file_name, host, next_chunk, next_chunk_size);
        }
        sleep(DELAY_SECOND);
    }

    info("All parts received.");
    if (pthread_cancel(receiver_process))
    {
        error("Failed to stop receiver thread.");
        return LAUNCH_FAILED;
    }
    else
    {
        info("Stopped receiver thread.");
    }
    return _merge_file(file_name);
}

int _parse_query(char *path, char *file_name, char *host, int *port)
{
    char message[CHUNK_SIZE];
    char query[PATH_SIZE];
    strcpy(query, path);
    char *_ = strtok(query, "/");

    strcpy(file_name, strtok(NULL, "/"));
    if (file_name == NULL)
    {
        error("Failed to parse file name");
        return PARSE_QUERY_FAILED;
    }
    sprintf(message, "Parsed file name: %s", file_name);
    debug(message);

    strcpy(host, strtok(NULL, "/"));
    if (host == NULL)
    {
        error("Failed to parse receiver host");
        return PARSE_QUERY_FAILED;
    }
    sprintf(message, "Parsed receiver host: %s", host);
    debug(message);

    char *portstr = strtok(NULL, "/");
    if (portstr == NULL)
    {
        error("Failed to parse recevier port");
        return PARSE_QUERY_FAILED;
    }
    sscanf(portstr, "%d", port);
    sprintf(message, "Parsed receiver port: %d", *port);
    debug(message);

    return PARSE_QUERY_SUCEEDED;
}

int _update_request(url_info *url, char *file_name, char *host, int next_chunk, int next_chunk_size)
{
    info("Updating URL path...");
    sprintf(url->path, "sendfile/%s/%s/%d/%d/%d", file_name, host, receiver_port, next_chunk, next_chunk_size);
    debug(url->path);
    send_request(url);
    return UPDATE_REQUEST_SUCEEDED;
}
