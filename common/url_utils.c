#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "url_utils.h"
#include "cli_utils.h"
#include "logger.h"

int parse_url(char *input, url_info *url)
{
    info("Parsing url...");
    debug(input);

    int protocol_status = _parse_protocol(&input, url);
    if (protocol_status != PARSE_URL_OK)
    {
        return protocol_status;
    }

    int host_status = _parse_host(&input, url);
    if (host_status != PARSE_URL_OK)
    {
        return host_status;
    }

    int port_status = _parse_port(&input, url);
    if (port_status != PARSE_URL_OK)
    {
        return port_status;
    }

    int path_status = _parse_path(&input, url);
    if (path_status != PARSE_URL_OK)
    {
        return path_status;
    }

    info("URL parsed successfully");
    return PARSE_URL_OK;
}

int _parse_protocol(char **input, url_info *url)
{
    info("Parsing protocol...");
    size_t cnt = strcspn(*input, ":");
    if ((*input)[cnt + 1] != '/' || (*input)[cnt + 2] != '/')
    {
        error("Protocol should be followed by '://'");
        return PARSE_URL_PROTOCOL_UNKNOWN;
    }
    if (cnt >= (int)strlen(*input))
    {
        error("Unexpected end of URL");
        return PARSE_URL_NO_SLASH;
    }

    strncpy(url->protocol, *input, cnt);
    url->protocol[cnt] = '\0';
    debug("Found protocol");
    debug(url->protocol);

    if (strcmp(url->protocol, "http") != 0)
    {
        error("Only 'http' protocol is accepted");
        return PARSE_URL_PROTOCOL_UNKNOWN;
    }

    *input += cnt + 3;
    return PARSE_URL_OK;
}

int _parse_host(char **input, url_info *url)
{
    info("Parsing host...");
    size_t cnt = strcspn(*input, ":/");
    if (cnt >= (int)strlen(*input))
    {
        error("Unexpected end of URL");
        return PARSE_URL_NO_SLASH;
    }
    if (cnt == 0)
    {
        error("Host can not be empty");
        return PARSE_URL_INVALID_HOST;
    }
    strncpy(url->host, *input, cnt);
    url->host[cnt] = '\0';
    debug("Found host");
    debug(url->host);

    *input += cnt;
    return PARSE_URL_OK;
}

int _parse_port(char **input, url_info *url)
{
    info("Parsing port...");
    // Port 80 by default
    url->port = 80;
    if ((*input)[0] == ':')
    {
        // Found ':', parse port
        (*input)++;
        size_t cnt = strcspn((*input), "/");
        if (cnt >= (int)strlen(*input))
        {
            error("Unexpected end of URL");
            return PARSE_URL_NO_SLASH;
        }
        char tmp[10];
        strncpy(tmp, *input, cnt);
        tmp[cnt] = '\0';
        url->port = atoi(tmp);
        debug("Found port");
        debug(tmp);
        if (url->port == 0)
        {
            error("Port is not a number or a zero");
            return PARSE_URL_INVALID_PORT;
        }
        *input += cnt;
    }
    return PARSE_URL_OK;
}

int _parse_path(char **input, url_info *url)
{
    info("Parsing path...");
    (*input)++;
    strcpy(url->path, *input);
    debug("Found path");
    debug(url->path);
    return PARSE_URL_OK;
}

void print_url_info(url_info *url)
{
    printf("Protocol: \"%s\"\n", url->protocol);
    printf("Hostname: \"%s\"\n", url->host);
    printf("Port: %d\n", url->port);
    printf("Path: \"%s\"\n", url->path);
}
