#pragma once

/**
 * Logging system for the game.
 */

#include <stddef.h>

#if !defined(LOG_LEVEL_DEBUG) && !defined(LOG_LEVEL_ERROR)
#ifdef PROD
#define LOG_LEVEL_ERROR
#else
#define LOG_LEVEL_DEBUG
#endif
#endif

/**
 * Initialize the logging system.
 *
 * \param program_name The name of the program being executed.
 */
[[gnu::nonnull]]
void logger_init(const char *const program_name);

/**
 * Free the resources used by the logging system.
 */
void log_quit(void);

#ifndef PROD
#define PRINTF_LIKE gnu::format(printf, 4, 5)
#define location_param                                  \
    const char *const restrict file, const size_t line, \
        const char *const restrict function_name,
#define LOG_ERROR_NONNULL gnu::nonnull(1, 3, 4)

#define log_errorf(format, ...)                   \
    _log_errorf(__FILE__, __LINE__, __FUNCTION__, \
                (format)__VA_OPT__(, ) __VA_ARGS__)

#define log_errorf_errno(format, ...)                   \
    _log_errorf_errno(__FILE__, __LINE__, __FUNCTION__, \
                      (format)__VA_OPT__(, ) __VA_ARGS__)

#ifdef LOG_LEVEL_DEBUG
#define log_debugf(format, ...)                   \
    _log_debugf(__FILE__, __LINE__, __FUNCTION__, \
                (format)__VA_OPT__(, ) __VA_ARGS__)
#else
#define log_debugf(format, ...)
#endif

/**
 * Log a debug message with the given format prefixed by the file and the line
 * from where the message was printed.
 *
 * Use the log_debugf() marcro so the file and line parameters are filled
 * automatically.
 * In production build, the debug call will be removed.
 *
 * \param file The path of the file from were the message is printed.
 * \param line The line number from were the message is printed.
 * \param function_name The function name from were the message is printed.
 * \param format A printf like format for the log message.
 */
[[gnu::nonnull(1, 3)]] [[PRINTF_LIKE]]
void _log_debugf(const char *const restrict file, const size_t line,
                 const char *const restrict function_name,
                 const char *const restrict format, ...);
#else
#define PRINTF_LIKE gnu::format(printf, 1, 2)
#define location_param
#define LOG_ERROR_NONNULL gnu::nonnull(1)

#define log_errorf(format, ...) _log_errorf((format)__VA_OPT__(, ) __VA_ARGS__)
#define log_errorf_errno(format, ...) \
    _log_errorf_errno((format)__VA_OPT__(, ) __VA_ARGS__)
#define log_debugf(format, ...)
#endif

/**
 * Log an error message with the given format.
 *
 * In non-production build, this function also take the file and the line from
 * where the message is printed for debug purpose.
 * Use the log_errorf() so the file and line parameters are filled automatically
 * and in production, the location of the call will be removed.
 *
 * \param file Only in debug! The path of the file from were the message is
 *             printed.
 * \param line Only in debug! The line number from were the message is printed.
 * \param function_name Only in debug! The function name from were the message
 *                      is printed.
 * \param format A printf like format for the log message.
 */
[[LOG_ERROR_NONNULL]] [[PRINTF_LIKE]]
void _log_errorf(location_param const char *const restrict format, ...);

/**
 * Log an error message with the given format and add the error message
 * corresponding to errno at the end.
 *
 * In non-production build, this function also take the file and the line from
 * where the message is printed for debug purpose.
 * Use the log_errorf_errno() so the file and line parameters are filled
 * automatically and in production, the location of the call will be removed.
 *
 * \param file Only in debug! The path of the file from were the message is
 *             printed.
 * \param line Only in debug! The line number from were the message is printed.
 * \param function_name Only in debug! The function name from were the message
 *                      is printed.
 * \param format A printf like format for the log message.
 */
[[LOG_ERROR_NONNULL]] [[PRINTF_LIKE]]
void _log_errorf_errno(location_param const char *const restrict format, ...);
