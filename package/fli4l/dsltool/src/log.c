/*!
 * @file    log.c
 * @brief   implementation file for logging
 * @details send log output to file or stderr
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    20.02.2013
 *
 * @note
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * @n
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * @n
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "log.h"

/*!
 * @var         static int log_level
 * @brief       log severity level
 */
static int log_level = LOGLEVEL_INFO;
/*!
 * @var         static FILE* log_stream
 * @brief       log file stream
 */
static FILE* log_stream = NULL;

/*!
 * @fn          void log_open(int level, const char* file)
 * @brief       open log file
 * @param[in]   level
 *              log severity level
 * @param[in]   file
 *              log file name
 */
void log_open(int level, const char* file)
{
    log_level = level;
    if (file)
    {
        struct stat statbuf;

        if (stat(file, &statbuf) >= 0)
        {
            if (S_ISREG(statbuf.st_mode) == 0)
            {
                fprintf(stderr, "\"%s\" is not a file\n", file);
                return;
            }
        }
        log_stream = fopen(file, "a");
        if (!log_stream)
        {
            fprintf(stderr, "error opening \"%s\"\n", file);
        }
    }
}

/*!
 * @fn          void log_close()
 * @brief       close log file
 */
void log_close()
{
    if (log_stream)
    {
        fclose(log_stream);
        log_stream = NULL;
    }
}

/*!
 * @fn          static void log_message(int level, const char* format,
 *                  va_list args)
 * @brief       write message to log file
 * @param[in]   level
 *              message severity level
 * @param[in]   format
 *              printf format string
 * @param[in]   args
 *              arguments to be filled in format string
 */
static void log_message(int level, const char* format, va_list args)
{
    if (level <= log_level)
    {
        if (log_stream)
        {
            vfprintf(log_stream, format, args);
        }
        else
        {
            vfprintf(stderr, format, args);
        }
    }
}

/*!
 * @fn          void log_error(const char* format, ...)
 * @brief       log error message
 * @param[in]   format
 *              printf format string
 * @param[in]   ...
 *              arguments to be filled in format string
 */
void log_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOGLEVEL_ERROR, format, args);
    va_end(args);
}

/*!
 * @fn          void log_warning(const char* format, ...)
 * @brief       log warning message
 * @param[in]   format
 *              printf format string
 * @param[in]   ...
 *              arguments to be filled in format string
 */
void log_warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOGLEVEL_WARNING, format, args);
    va_end(args);
}

/*!
 * @fn          void log_info(const char* format, ...)
 * @brief       log info message
 * @param[in]   format
 *              printf format string
 * @param[in]   ...
 *              arguments to be filled in format string
 */
void log_info(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOGLEVEL_INFO, format, args);
    va_end(args);
}

/*!
 * @fn          void log_debug(const char* format, ...)
 * @brief       log debug message
 * @param[in]   format
 *              printf format string
 * @param[in]   ...
 *              arguments to be filled in format string
 */
void log_debug(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log_message(LOGLEVEL_DEBUG, format, args);
    va_end(args);
}

#if defined(DEBUG)
/*!
 * @fn          void log_trace(const char* format, ...)
 * @brief       log trace message
 * @param[in]   format
 *              printf format string
 * @param[in]   ...
 *              arguments to be filled in format string
 */
void log_trace (const char* format, ...)
{
    va_list args;
    va_start (args, format);
    log_message (LOGLEVEL_TRACE, format, args);
    va_end (args);
}
#endif /* defined(DEBUG) */

/* _oOo_ */
