#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "logger.h"

// The default log level is info
int LOG_LEVEL = LOGGER_LEVEL_INFO;

void error(char *message)
{
    if (LOG_LEVEL <= LOGGER_LEVEL_ERROR)
    {
        _log(LOGGER_COLOR_RED, LOGGER_TAG_ERROR, message);
    }
}

void info(char *message)
{
    if (LOG_LEVEL <= LOGGER_LEVEL_INFO)
    {
        _log(LOGGER_COLOR_GREEN, LOGGER_TAG_INFO, message);
    }
}

void debug(char *message)
{
    if (LOG_LEVEL <= LOGGER_LEVEL_DEBUG)
    {
        _log(LOGGER_COLOR_WHITE, LOGGER_TAG_DEBUG, message);
    }
}

void _log(const char *color, const char *tag, char *message)
{
    time_t now;
    time(&now);
    printf("%.19s [%s%s\033[0m] %s\n", ctime(&now), color, tag, message);
}
