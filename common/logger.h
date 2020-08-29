#ifndef LOGGER_H
#define LOGGER_H

#define LOGGER_LEVEL_NONE 3
#define LOGGER_LEVEL_ERROR 2
#define LOGGER_LEVEL_INFO 1
#define LOGGER_LEVEL_DEBUG 0

#define LOGGER_TAG_NONE "NONE"
#define LOGGER_TAG_ERROR "ERROR"
#define LOGGER_TAG_INFO "INFO"
#define LOGGER_TAG_DEBUG "DEBUG"

#define LOGGER_COLOR_RED "\033[1;31m"
#define LOGGER_COLOR_GREEN "\033[0;32m"
#define LOGGER_COLOR_WHITE ""

#define CHUNK_SIZE 2048

/**
 * Error loggers
 */
void error(char *message);

/**
 * Info loggers
 */
void info(char *message);

/**
 * Debug loggers
 */
void debug(char *message);

void _log(const char *color, const char *tag, char *message);

#endif
