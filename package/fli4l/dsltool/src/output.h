/*!
 * @file    output.h
 * @brief   header file for data output
 * @details function declarations
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

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

void output_data_std(parser_data_t* data);
void output_tone_std(parser_data_t* data);
void output_data_txt(parser_data_t* data, const char* file);
void output_tone_bits_png(parser_data_t* data, const char* file);
void output_tone_snr_png(parser_data_t* data, const char* file);
void output_tone_char_png(parser_data_t* data, const char* file);

#endif /* __OUTPUT_H__ */
/* _oOo_ */
