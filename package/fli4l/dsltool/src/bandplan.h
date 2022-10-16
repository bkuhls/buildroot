/*!
 * @file    bandplan.h
 * @brief   header file for bandplans
 * @details definitions & function declarations
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    27.02.2014
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

#ifndef __BANDPLAN_H__
#define __BANDPLAN_H__

#define DSL_BAND_MAX    4           /*!< maximum number of bandplans        */

/*!
 * @struct  bandplan_t
 * @brief   bandplan data structure
 */
typedef struct
{
    const char* dsl_type;           /*!< string for DSL type                */
    int num_tone;                   /*!< number of tones                    */
    double spacing;                 /*!< spacing between tones [kHz]        */
    int num_band;                   /*!< number of bands                    */
    struct {
        int up_min;                 /*!< min tone number for upstream       */
        int up_max;                 /*!< max tone number for upstream       */
        int down_min;               /*!< min tone number for downstream     */
        int down_max;               /*!< max tone number for downstream     */
    } band[DSL_BAND_MAX];           /*!< band data                          */
} bandplan_t;

extern bandplan_t bandplan_NONE;
extern bandplan_t bandplan_ADSL_AnnexA;
extern bandplan_t bandplan_ADSL_AnnexB;
extern bandplan_t bandplan_ADSL2_AnnexA;
extern bandplan_t bandplan_ADSL2_AnnexB;
extern bandplan_t bandplan_ADSL2plus_AnnexA;
extern bandplan_t bandplan_ADSL2plus_AnnexB;
extern bandplan_t bandplan_ADSL2plus_AnnexJ;
extern bandplan_t bandplan_VDSL2_AnnexB_8b_998_M2x_B;
extern bandplan_t bandplan_VDSL2_AnnexB_17a_998ADE17_M2x_B;

#endif /* __BANDPLAN_H__ */
/* _oOo_ */
