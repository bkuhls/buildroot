/*!
 * @file    graphic.h
 * @brief   header file for graphic generation
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

#ifndef __GRAPHIC_H__
#define __GRAPHIC_H__

#define ALIGN_CENTER    0           /*!< align center (horizontal&vertical) */
#define ALIGN_LEFT     -1           /*!< align left (horizontal)            */
#define ALIGN_RIGHT     1           /*!< align right (horizontal)           */
#define ALIGN_UP       -1           /*!< align up (vertical)                */
#define ALIGN_DOWN      1           /*!< align down (vertical)              */
#define ROTATE_NONE     0           /*!< don't rotate                       */
#define ROTATE_LEFT    -1           /*!< rotate left by 90°                 */
#define ROTATE_RIGHT    1           /*!< rotate right by 90°                */

int graphic_create(double width, double height);
void graphic_delete();
int graphic_save(const char* file);

int graphic_line(double x0, double y0, double x1, double y1, double width,
        int dash, unsigned long color);
int graphic_rect(double x0, double y0, double x1, double y1,
        unsigned long color);
int graphic_block(double x, double y, unsigned long color,
        unsigned long color_text);
int graphic_text(double x, double y, const char* text, int halign, int valign,
        int rotate, unsigned long color);

#endif /* __GRAPHIC_H__ */
/* _oOo_ */
