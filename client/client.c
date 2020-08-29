#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "client.h"
#include "launcher.h"
#include "../common/url_utils.h"
#include "../common/cli_utils.h"
#include "../common/logger.h"

int main(int argc, char *argv[])
{
    url_info url;
    if (argc < 2)
    {
        error("Missing argument. Please enter URL.");
        print_help();
        return CLIENT_INVALID_ARGUMENTS;
    }
    if (strcmp(argv[1], OPTION_HELP) == 0 || strcmp(argv[1], OPTION_HELP_ABBR) == 0)
    {
        print_help();
        return CLIENT_OK;
    }
    // Parse logging level
    if (argc > 2)
    {
        int ret = parse_logger(argc - 2, argv + 2);
        if (ret)
        {
            error("Invalid logging argument");
            print_help();
            return CLIENT_INVALID_ARGUMENTS;
        }
    }
    char *urlstr = argv[1];
    // Parse the URL
    int ret = parse_url(urlstr, &url);
    if (ret)
    {
        error("Failed to parse URL.");
        print_help();
        return CLIENT_INVALID_ARGUMENTS;
    }
    print_url_info(&url);
    // Run the client
    return launch(&url);
}

void print_help()
{
    printf("usage: client request_url [option]\n");
    printf("  request_url:\n");
    printf("     http://<server_address>:<server_port>/sendfile/<filename>/<dest_address>/<dest_port_number>\n");
    printf("       where:\n");
    printf("         <server_address> is the address of the server\n");
    printf("         <server_port> is the port number of the server\n");
    printf("         <filename> is the name of the file to send\n");
    printf("         <dest_address> is the (IPv4) address of the client, to which the file should be sent\n");
    printf("         <dest_port_number> is the UDP port number of the client, to which the file should be sent\n");
    printf("  option:\n");
    printf("    --log, -l   Set log level (DEBUG, INFO, ERROR, NONE). Default: INFO\n");
    printf("usage: client --help/-h\n");
}
