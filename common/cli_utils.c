#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_utils.h"
#include "logger.h"

int parse_logger(int argc, char *argv[])
{
    switch (argc)
    {
    case 0:;
        info("No logging option found. Using default.");
        return PARSE_LOGGER_OK;

    case 1:;
        char *option_key = strtok(argv[0], "=");
        char *option_value = strtok(NULL, "=");
        if (option_key == NULL || option_value == NULL)
        {
            error("Invalid argument.");
            return PARSE_LOGGER_INVALID;
        }
        return _update_logger(option_key, option_value);

    case 2:;
        return _update_logger(argv[0], argv[1]);

    default:;
        error("Too many logging options.");
        return PARSE_LOGGER_INVALID;
    }
    return PARSE_LOGGER_OK;
}

int _update_logger(char *option_key, char *option_value)
{
    if (strcmp(option_key, OPTION_LOG) && strcmp(option_key, OPTION_LOG_ABBR))
    {
        error("Invalid logger option key. Should be either '--log' or '-l'.");
        return PARSE_LOGGER_INVALID;
    }
    if (strcmp(option_value, LOGGER_TAG_NONE) == 0)
    {
        LOG_LEVEL = LOGGER_LEVEL_NONE;
        return PARSE_LOGGER_OK;
    }
    if (strcmp(option_value, LOGGER_TAG_ERROR) == 0)
    {
        LOG_LEVEL = LOGGER_LEVEL_ERROR;
        return PARSE_LOGGER_OK;
    }
    if (strcmp(option_value, LOGGER_TAG_INFO) == 0)
    {
        LOG_LEVEL = LOGGER_LEVEL_INFO;
        return PARSE_LOGGER_OK;
    }
    if (strcmp(option_value, LOGGER_TAG_DEBUG) == 0)
    {
        LOG_LEVEL = LOGGER_LEVEL_DEBUG;
        return PARSE_LOGGER_OK;
    }
    error("Invalid logger option value. Should be one of 'NONE', 'ERROR', 'INFO', 'DEBUG'.");
    return PARSE_LOGGER_INVALID;
}
