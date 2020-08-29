#ifndef LAUNCHER_H
#define LAUNCHER_H

#define PARSE_QUERY_SUCEEDED 0
#define PARSE_QUERY_FAILED 1

#define UPDATE_REQUEST_SUCEEDED 0
#define UPDATE_REQUEST_FAILED 1

#define DELAY_SECOND 1

typedef struct url_info url_info;

extern int file_size;
extern int *file;
extern char **file_buf;
extern int receiver_port;
extern pthread_t receiver_process;

/**
 * Launch client 
 */
int launch(url_info *url);

int _parse_query(char *path, char *file_name, char *host, int *port);
int _update_request(url_info *url, char *file_name, char *host, int next_chunk, int next_chunk_size);

#endif
