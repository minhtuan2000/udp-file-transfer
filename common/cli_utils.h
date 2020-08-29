#ifndef CLI_UTILS_H
#define CLI_UTILS_H

#define PARSE_LOGGER_OK 0
#define PARSE_LOGGER_INVALID 1

#define OPTION_LOG "--log"
#define OPTION_LOG_ABBR "-l"
#define OPTION_HELP "--help"
#define OPTION_HELP_ABBR "-h"

extern int LOG_LEVEL;

/**
 * Parse logger option
 */
int parse_logger(int argc, char *argv[]);

/**
 * Print help
 */
void print_help();

int _update_logger(char *option_key, char *option_value);

#endif
