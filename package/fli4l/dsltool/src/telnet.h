/*!
 * @file    telnet.h
 * @brief   header file for telnet
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

#ifndef __TELNET_H__
#define __TELNET_H__

int telnet_open(const char* hostname);
void telnet_close(int sock);
void telnet_receive_loop(int sock, int (*callback)(int, char*, size_t));
void telnet_stop_loop(void);
void telnet_send(int sock, const char* data, int length);

#endif /* __TELNET_H__ */
/* _oOo_ */
