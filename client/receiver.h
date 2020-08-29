#ifndef RECEIVER_H
#define RECEIVER_H

#define LAUNCH_SUCEEDED 0
#define LAUNCH_FAILED 1

#define RECEIVE_SUCEEDED 0
#define RECEIVE_FAILED 1

#define HANDLE_SUCEEDED 0
#define HANDLE_FAILED 1

#define MAX_FILE_SIZE 65536

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

/**
 * Launch a file receiver that runs concurrently with the client
 */
int launch_receiver();

void *_receive_file();
int _is_not_finished();
void _get_next_chunk(int *next_chunk, int *next_chunk_size);
int _handle(char *input);
int _merge_file(char *file_name);

#endif
