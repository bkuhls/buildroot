/*!
 * @file    collect.h
 * @brief   header file for collect output
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

#ifndef __COLLECT_H__
#define __COLLECT_H__

#define COLLECT_INTERVAL   10       /*!< collect interval in seconds        */

void collect_init(const char* host, const char* modem, const char* socket);
void collect_output(parser_data_t* data);
void collect_copy(parser_data_t* data);

#endif /* __COLLECT_H__ */
/* _oOo_ */
