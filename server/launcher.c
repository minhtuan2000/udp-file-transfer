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
#include "launcher.h"
#include "../common/url_utils.h"
#include "../common/cli_utils.h"
#include "../common/logger.h"

int launch(int port)
{
    char message[CHUNK_SIZE];
    info("Launching server...");
    int tmp = launch_handler(port);
    if (tmp)
    {
        error("Failed to launch server.");
        return LAUNCH_FAILED;
    }
    return LAUNCH_SUCEEDED;
}
