/*!
 * @file    log.h
 * @brief   header file for logging
 * @details definitions & function declarations
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

#ifndef __LOG_H__
#define __LOG_H__

#define LOGLEVEL_ERROR     0        /*!< log severity error                 */
#define LOGLEVEL_WARNING   1        /*!< log severity warning               */
#define LOGLEVEL_INFO      2        /*!< log severity info                  */
#define LOGLEVEL_DEBUG     3        /*!< log severity debug                 */
#define LOGLEVEL_TRACE     4        /*!< log severity trace                 */

void log_open(int level, const char* file);
void log_close();
void log_error(const char* format, ...);
void log_warning(const char* format, ...);
void log_info(const char* format, ...);
void log_debug(const char* format, ...);
#if defined(DEBUG)
void log_trace (const char* format, ...);
#else /* defined(DEBUG) */
#define log_trace(...)
#endif /* defined(DEBUG) */

#endif /* __LOG_H__ */
/* _oOo_ */
