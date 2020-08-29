#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "server.h"
#include "launcher.h"
#include "../common/url_utils.h"
#include "../common/cli_utils.h"
#include "../common/logger.h"

int main(int argc, char *argv[])
{
    url_info url;
    if (argc < 2)
    {
        error("Missing argument. Please enter port number.");
        print_help();
        return SERVER_INVALID_ARGUMENT;
    }
    if (strcmp(argv[1], OPTION_HELP) == 0 || strcmp(argv[1], OPTION_HELP_ABBR) == 0)
    {
        print_help();
        return SERVER_OK;
    }
    // Parse logging level
    if (argc > 2)
    {
        int ret = parse_logger(argc - 2, argv + 2);
        if (ret)
        {
            error("Invalid logging argument");
            print_help();
            return SERVER_INVALID_ARGUMENT;
        }
    }
    char *urlstr = argv[1];
    // Parse the port number
    int port;
    int ret = sscanf(argv[1], "%d", &port);
    if (ret == EOF)
    {
        error("Failed to parse port number.");
        print_help();
        return SERVER_INVALID_ARGUMENT;
    }
    // Run the server
    return launch(port);
}

void print_help()
{
    printf("usage: server port [option]\n");
    printf("  port: The port number where the server will be listening\n");
    printf("  option:\n");
    printf("    --log, -l   Set log level (DEBUG, INFO, ERROR, NONE). Default: INFO\n");
    printf("usage: server --help/-h\n");
}
