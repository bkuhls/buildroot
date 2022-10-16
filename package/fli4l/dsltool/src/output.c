/*!
 * @file    output.c
 * @brief   implementation file for data output
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

#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "parser.h"
#include "output.h"
#include "graphic.h"
#include "log.h"

static unsigned long color_bgnd = 0xf3f3f3;         /*!< background color   */
static unsigned long color_topleft = 0xcfcfcf;      /*!< border color       */
static unsigned long color_bottomright = 0x9e9e9e;  /*!< border color       */
static unsigned long color_graph = 0xe0e0e0;        /*!< graph background   */
static unsigned long color_grid = 0xdfabab;         /*!< grid color         */
static unsigned long color_coord = 0x666666;        /*!< coordsystem color  */
static unsigned long color_up = 0x00ff00;           /*!< upstream color     */
static unsigned long color_down = 0x0000ff;         /*!< downstream color   */
static unsigned long color_gainQ2 = 0xff00ff;       /*!< gain Q2 color      */
static unsigned long color_gap = 0xff0000;          /*!< gap color          */
static unsigned long color_snr = 0x999999;          /*!< SNR color          */
static unsigned long color_noisemargin = 0xffff00;  /*!< noisemargin color  */
static unsigned long color_chancharlog = 0x999999;  /*!< channel color      */
static unsigned long color_text = 0x000000;         /*!< text color         */

#define SCALE_FACTOR    1                           /*!< graph scale factor */

#define GRAPH_X0        50                          /*!< graph origin (x)   */
#define GRAPH_Y0        40                          /*!< graph origin (y)   */
#define GRAPH_HSCALE    32                          /*!< horiz. scale width */
#define GRAPH_WIDTH     calc_tone_width(data)       /*!< graph width        */
#define GRAPH_HEIGHT    150                         /*!< graph height       */
#define GRAPH_VSCALE1   10                          /*!< vert. scale width  */
#define GRAPH_VSCALE2   15                          /*!< vert. scale width  */
#define GRAPH_XMAX      (GRAPH_X0 + GRAPH_WIDTH)    /*!< graph max postion X*/
#define GRAPH_YMAX      (GRAPH_Y0 + GRAPH_HEIGHT)   /*!< graph max postion Y*/

/*!
 * @fn          static void output_tone(FILE* stream, parser_data_t* data, int* tone)
 * @brief       output tone data to file
 * @param[in]   stream
 *              file stream
 * @param[in]   data
 *              parser data
 * @param[in]   tone
 *              tone data array
 */
static void output_tone(FILE* stream, parser_data_t* data, int* tone)
{
    int i, j;

    for (i = 0; i < data->bandplan->num_tone; i += 32)
    {
        fprintf(stream, "tone %3d :", i);
        for (j = 0; j < 32; j++)
        {
            fprintf(stream, "%4d", tone[i+j]);
        }
        fprintf(stream, "\n");
    }
}

/*!
 * @fn          void output_data_std(parser_data_t* data)
 * @brief       print parser data to stdout
 * @param[in]   data
 *              parser data
 */
void output_data_std(parser_data_t* data)
{
    int iBand;

    fprintf(stdout,
            "DSL Type                : %s\n", data->bandplan->dsl_type);
    fprintf(stdout,
            "DSL Bands               : %d\n", data->bandplan->num_band);

    if (2 == data->modemstate.valid)
    {
        fprintf(stdout,
                "Modemstate              : (%d) \"%s\"\n", data->modemstate.state, data->modemstate.str);
    }
    if (1 == data->operationmode.valid)
    {
        fprintf(stdout,
                "Operation Mode          : \"%s\"\n", data->operationmode.str);
    }
    if (1 == data->channelmode.valid)
    {
        fprintf(stdout,
                "Channel Mode            : \"%s\"\n", data->channelmode.str);
    }
    if (2 == data->noisemargin[0].valid)
    {
        fprintf(stdout,
                "Noise Margin       [dB] : Up   %6.1f  Down %6.1f\n",
                data->noisemargin[0].up, data->noisemargin[0].down);
    }
    if (2 == data->attenuation[0].valid)
    {
            fprintf(stdout,
                "Attenuation        [dB] : Up   %6.1f  Down %6.1f\n",
                data->attenuation[0].up, data->attenuation[0].down);
    }
    for(iBand=1; iBand<data->bandplan->num_band; iBand++)
    {
        if (2 == data->noisemargin[iBand].valid)
        {
            fprintf(stdout,
                    "Noise Margin %d     [dB] : Up   %6.1f  Down %6.1f\n", iBand,
                    data->noisemargin[iBand].up, data->noisemargin[iBand].down);
        }
        if (2 == data->attenuation[iBand].valid)
        {
                fprintf(stdout,
                    "Attenuation %d      [dB] : Up   %6.1f  Down %6.1f\n", iBand,
                    data->attenuation[iBand].up, data->attenuation[iBand].down);
        }
    }
    if (2 == data->txpower.valid)
    {
        fprintf(stdout,
                "Tx Power          [dBm] : Up   %6.1f  Down %6.1f\n",
                data->txpower.up, data->txpower.down);
    }
    if (2 == data->bandwidth.kbit.valid)
    {
        fprintf(stdout,
                "Bandwidth      [kBit/s] : Up   %6d  Down %6d\n",
                data->bandwidth.kbit.up, data->bandwidth.kbit.down);
    }
    if (2 == data->bandwidth.cells.valid)
        {
        fprintf(stdout,
                "Bandwidth     [Cells/s] : Up   %6d  Down %6d\n",
                data->bandwidth.cells.up, data->bandwidth.cells.down);
    }
    if (2 == data->maxbandwidth.kbit.valid)
    {
        fprintf(stdout,
                "Max.Bandwidth  [kBit/s] : Up   %6d  Down %6d\n",
                data->maxbandwidth.kbit.up, data->maxbandwidth.kbit.down);
    }
    if (2 == data->statistics.error.FEC.valid)
    {
        fprintf(stdout,
                "Error FEC               : Rx  %12ld     Tx   %12ld\n",
                data->statistics.error.FEC.Rx,
                data->statistics.error.FEC.Tx);
    }
    if (2 == data->statistics.error.CRC.valid)
    {
        fprintf(stdout,
                "Error CRC               : Rx  %12ld     Tx   %12ld\n",
                data->statistics.error.CRC.Rx,
                data->statistics.error.CRC.Tx);
    }
    if (2 == data->statistics.error.HEC.valid)
    {
        fprintf(stdout,
                "Error HEC               : Rx  %12ld     Tx   %12ld\n",
                data->statistics.error.HEC.Rx,
                data->statistics.error.HEC.Tx);
    }
    if (2 == data->statistics.failure.valid)
    {
        fprintf(stdout,
                "Failure   [error s]     : %6d/15min  %6d/day\n", data->statistics.failure.error_secs_15min, data->statistics.failure.error_secs_day);
    }
    if (2 == data->atm.pvc.valid)
    {
        fprintf(stdout,
                "VPI : %3d      VCI      : %3d\n", data->atm.pvc.vpi, data->atm.pvc.vci);
    }
    if(3 == data->atm.atu_c.valid)
    {
        fprintf(stdout,
                "ATU-C Vendor: %c%c%c%c  V-Spec: %3d.%-3d  Revision: %3d\n",
                data->atm.atu_c.vendor[0], data->atm.atu_c.vendor[1],
                data->atm.atu_c.vendor[2], data->atm.atu_c.vendor[3],
                data->atm.atu_c.vendspec[0],data->atm.atu_c.vendspec[1],
                data->atm.atu_c.revision);

    }
    if(3 == data->atm.atu_r.valid)
    {
        fprintf(stdout,
                "ATU-R Vendor: %c%c%c%c  V-Spec: %3d.%-3d  Revision: %3d\n",
                data->atm.atu_r.vendor[0], data->atm.atu_r.vendor[1],
                data->atm.atu_r.vendor[2], data->atm.atu_r.vendor[3],
                data->atm.atu_r.vendspec[0],data->atm.atu_r.vendspec[1],
                data->atm.atu_r.revision);

    }
}

/*!
 * @fn          void output_tone_std(parser_data_t* data)
 * @brief       print tone data to stdout
 * @param[in]   data
 *              parser data
 */
void output_tone_std(parser_data_t* data)
{
    if (data->tone.bitalloc_up.valid)
    {
        fprintf(stdout, "bitalloc_up\n");
        output_tone(stdout, data, data->tone.bitalloc_up.data);
    }
    if (data->tone.bitalloc_down.valid)
    {
        fprintf(stdout, "bitalloc_down\n");
        output_tone(stdout, data, data->tone.bitalloc_down.data);
    }
    if (data->tone.snr.valid)
    {
        fprintf(stdout, "snr\n");
        output_tone(stdout, data, data->tone.snr.data);
    }
    if (data->tone.gainQ2.valid)
    {
        fprintf(stdout, "gainQ2\n");
        output_tone(stdout, data, data->tone.gainQ2.data);
    }
    if (data->tone.noisemargin.valid)
    {
        fprintf(stdout, "noisemargin\n");
        output_tone(stdout, data, data->tone.noisemargin.data);
    }
    if (data->tone.chancharlog.valid)
    {
        fprintf(stdout, "chancharlog\n");
        output_tone(stdout, data, data->tone.chancharlog.data);
    }
}

/*!
 * @fn          void output_data_txt(parser_data_t* data, const char* file)
 * @brief       generate variables file
 * @param[in]   data
 *              parser data
 * @param[in]   file
 *              output file name
 */
void output_data_txt(parser_data_t* data, const char* file)
{
    FILE* stream = NULL;
    int iBand;

    if (file)
    {
        struct stat statbuf;

        if (stat(file, &statbuf) >= 0)
        {
            if (S_ISREG(statbuf.st_mode) == 0)
            {
                log_error("\"%s\" is not a file\n", file);
                return;
            }
        }
        stream = fopen(file, "w");
    }

    if (!stream)
    {
        log_error("error opening \"%s\"\n", file);
        return;
    }

    fprintf(stream, "dsltype=\"%s\"\n", data->bandplan->dsl_type);
    fprintf(stream, "dslband=\"%d\"\n", data->bandplan->num_band);

    if (2 == data->modemstate.valid)
    {
        fprintf(stream, "modemstate=\"%d\"\n", data->modemstate.state);
        fprintf(stream, "modemstate_str=\"%s\"\n", data->modemstate.str);
    }

    if (1 == data->operationmode.valid)
    {
        fprintf(stream, "operationmode=\"%s\"\n", data->operationmode.str);
    }
    if (1 == data->channelmode.valid)
    {
        fprintf(stream, "channelmode=\"%s\"\n", data->channelmode.str);
    }
    if (2 == data->noisemargin[0].valid)
    {
        fprintf(stream, "noisemargin_up=\"%.1f\"\n",
                data->noisemargin[0].up);
        fprintf(stream, "noisemargin_down=\"%.1f\"\n",
                data->noisemargin[0].down);
    }
    if (2 == data->attenuation[0].valid)
    {
        fprintf(stream, "attenuation_up=\"%.1f\"\n",
                data->attenuation[0].up);
        fprintf(stream, "attenuation_down=\"%.1f\"\n",
                data->attenuation[0].down);
    }
    for(iBand=1; iBand<data->bandplan->num_band; iBand++)
    {
        if (2 == data->noisemargin[iBand].valid)
        {
            fprintf(stream, "noisemargin%d_up=\"%.1f\"\n", iBand,
                    data->noisemargin[iBand].up);
            fprintf(stream, "noisemargin%d_down=\"%.1f\"\n", iBand,
                    data->noisemargin[iBand].down);
        }
        if (2 == data->attenuation[iBand].valid)
        {
            fprintf(stream, "attenuation%d_up=\"%.1f\"\n", iBand,
                    data->attenuation[iBand].up);
            fprintf(stream, "attenuation%d_down=\"%.1f\"\n", iBand,
                    data->attenuation[iBand].down);
        }
    }
    if (2 == data->txpower.valid)
    {
        fprintf(stream, "txpower_up=\"%.1f\"\n", data->txpower.up);
        fprintf(stream, "txpower_down=\"%.1f\"\n", data->txpower.down);
    }
    if (2 == data->bandwidth.kbit.valid)
    {
        fprintf(stream,
                "bandwidth_kbit_up=\"%d\"\n", data->bandwidth.kbit.up);
        fprintf(stream,
                "bandwidth_kbit_down=\"%d\"\n", data->bandwidth.kbit.down);
    }
    if (2 == data->bandwidth.cells.valid)
    {
        fprintf(stream,
                "bandwidth_cells_up=\"%d\"\n", data->bandwidth.cells.up);
        fprintf(stream,
                "bandwidth_cells_down=\"%d\"\n", data->bandwidth.cells.down);
    }
    if (2 == data->maxbandwidth.kbit.valid)
    {
        fprintf(stream,
                "maxbandwidth_kbit_up=\"%d\"\n", data->maxbandwidth.kbit.up);
        fprintf(stream,
                "maxbandwidth_kbit_down=\"%d\"\n", data->maxbandwidth.kbit.down);
    }
    if (2 == data->statistics.error.FEC.valid)
    {
        fprintf(stream,
                "error_rx_FEC=\"%ld\"\n", data->statistics.error.FEC.Rx);
        fprintf(stream,
                "error_tx_FEC=\"%ld\"\n", data->statistics.error.FEC.Tx);
    }
    if (2 == data->statistics.error.CRC.valid)
    {
        fprintf(stream,
                "error_rx_CRC=\"%ld\"\n", data->statistics.error.CRC.Rx);
        fprintf(stream,
                "error_tx_CRC=\"%ld\"\n", data->statistics.error.CRC.Tx);
    }
    if (2 == data->statistics.error.HEC.valid)
    {
        fprintf(stream,
                "error_rx_HEC=\"%ld\"\n", data->statistics.error.HEC.Rx);
        fprintf(stream,
                "error_tx_HEC=\"%ld\"\n", data->statistics.error.HEC.Tx);
    }
    if (2 == data->statistics.failure.valid)
    {
        fprintf(stream,
                "error_secs_15min=\"%d\"\n", data->statistics.failure.error_secs_15min);
        fprintf(stream,
                "error_secs_day=\"%d\"\n", data->statistics.failure.error_secs_day);
    }
    if (2 == data->atm.pvc.valid)
    {
        fprintf(stream, "ATM_VPI=\"%d\"\n", data->atm.pvc.vpi);
        fprintf(stream, "ATM_VCI=\"%d\"\n", data->atm.pvc.vci);
    }
    if(3 == data->atm.atu_c.valid)
    {
        fprintf(stream, "ATU_C_vendor=\"%c%c%c%c\"\n",
                data->atm.atu_c.vendor[0], data->atm.atu_c.vendor[1],
                data->atm.atu_c.vendor[2], data->atm.atu_c.vendor[3]);
        fprintf(stream, "ATU_C_vendspec=\"%d.%d\"\n",
                data->atm.atu_c.vendspec[0],data->atm.atu_c.vendspec[1]);
        fprintf(stream, "ATU_C_revision=\"%d\"\n",
                data->atm.atu_c.revision);
    }
    if(3 == data->atm.atu_r.valid)
    {
        fprintf(stream, "ATU_R_vendor=\"%c%c%c%c\"\n",
                data->atm.atu_r.vendor[0], data->atm.atu_r.vendor[1],
                data->atm.atu_r.vendor[2], data->atm.atu_r.vendor[3]);
        fprintf(stream, "ATU_R_vendspec=\"%d.%d\"\n",
                data->atm.atu_r.vendspec[0],data->atm.atu_r.vendspec[1]);
        fprintf(stream, "ATU_R_revision=\"%d\"\n",
                data->atm.atu_r.revision);
    }

    fclose(stream);
}

/*!
 * @fn          static int calc_tone_scale(parser_data_t* data)
 * @brief       calculate tone scaling
 * @param[in]   data
 *              parser data
 * @return      tone scale value
 */
static int calc_tone_scale(parser_data_t* data)
{
    if(data->bandplan->num_tone > 512)
    {
        return data->bandplan->num_tone/(512*SCALE_FACTOR);
    }

    return 1;
}

/*!
 * @fn          static int calc_tone_width(parser_data_t* data)
 * @brief       calculate graph width from tone scaling
 * @param[in]   data
 *              parser data
 * @return      graph width
 */
static int calc_tone_width(parser_data_t* data)
{
    if(data->bandplan->num_tone > 512)
    {
        return 512*SCALE_FACTOR;
    }

    return 512;
}

/*!
 * @fn          static int output_png_init(parser_data_t* data)
 * @brief       initialize png graphic
 * @details     draw background and borders
 * @param[in]   data
 *              parser data
 * @return      error code
 * @retval      0
 *              success
 * @retval      -1
 *              failure
 */
static int output_png_init(parser_data_t* data)
{
    double width = calc_tone_width(data) + 100;
    double height = GRAPH_HEIGHT + 100;

    if (0 != graphic_create(width, height))
    {
        return -1;
    }
    graphic_rect(0, 0, width, height, color_bgnd);
    graphic_line(0.5, 0.5, width - 0.5, 0.5, 1.0, 0, color_topleft);
    graphic_line(1.5, 1.5, width - 1.5, 1.5, 1.0, 0, color_topleft);
    graphic_line(0.5, 0.5, 0.5, height - 0.5, 1.0, 0, color_topleft);
    graphic_line(1.5, 1.5, 1.5, height - 1.5, 1.0, 0, color_topleft);
    graphic_line(0.5, height - 0.5, width - 0.5, height - 0.5, 1.0, 0,
            color_bottomright);
    graphic_line(1.5, height - 1.5, width - 1.5, height - 1.5, 1.0, 0,
            color_bottomright);
    graphic_line(width - 0.5, 0.5, width - 0.5, height - 0.5, 1.0, 0,
            color_bottomright);
    graphic_line(width - 1.5, 1.5, width - 1.5, height - 1.5, 1.0, 0,
            color_bottomright);

    graphic_rect(GRAPH_X0, GRAPH_Y0, GRAPH_XMAX, GRAPH_YMAX, color_graph);

    return 0;
}

/*!
 * @fn          static void output_png_hscale(parser_data_t* data)
 * @brief       draw horizontal scale and grid
 * @param[in]   data
 *              parser data
 */
static void output_png_hscale(parser_data_t* data)
{
    int i;
    int tone_scale = calc_tone_scale(data);

    for (i = GRAPH_HSCALE; i <= calc_tone_width(data); i+= 32)
    {
        int x0 = GRAPH_X0 + i;
        int y0 = GRAPH_Y0 - 3;
        int y1 = GRAPH_YMAX + 3;
        graphic_line(x0, y0, x0, y1, 1.0, 1, color_grid);
    }

    for (i = 0; i <= calc_tone_width(data); i+= GRAPH_HSCALE * 2)
    {
        char text[16];
        int x0 = GRAPH_X0 + i;
        int y0 = GRAPH_Y0 - 4;
        int y1 = GRAPH_YMAX + 6;

        snprintf(text, sizeof(text), "%d", i * tone_scale);
        graphic_text(x0, y0, text, (calc_tone_width(data) == i) ? ALIGN_RIGHT : ALIGN_CENTER
                , ALIGN_DOWN, ROTATE_NONE, color_text);

        snprintf(text, sizeof(text), "%.0lf", (double)(i * tone_scale * data->bandplan->spacing));
        graphic_text(x0, y1, text, (calc_tone_width(data) == i) ? ALIGN_RIGHT : ALIGN_CENTER
                ,ALIGN_UP, ROTATE_NONE, color_text);
    }

    graphic_text(GRAPH_XMAX + 4, GRAPH_Y0 - 4, "tone", ALIGN_LEFT, ALIGN_DOWN,
            ROTATE_NONE, color_text);
    graphic_text(GRAPH_XMAX + 4, GRAPH_YMAX + 6, "kHz", ALIGN_LEFT, ALIGN_UP,
            ROTATE_NONE, color_text);
}

/*!
 * @fn          static void output_png_vscale_left(parser_data_t* data, int vscale, int inc, int max,
 *                  const char* text)
 * @brief       draw left vertical scale and grid
 * @param[in]   data
 *              parser data
 * @param[in]   vscale
 *              vertical scale space
 * @param[in]   inc
 *              vertical scale increment
 * @param[in]   max
 *              vertical scale maximum
 * @param[in]   text
 *              label
 */
static void output_png_vscale_left(parser_data_t* data, int vscale, int inc, int max, const char* text)
{
    int i;
    int imax = (GRAPH_HEIGHT / vscale) + 1;

    for (i = 1; i < imax; i++)
    {
        int x0 = GRAPH_X0 - 3;
        int x1 = GRAPH_XMAX + 3;
        int y0 = GRAPH_YMAX - (i * vscale);
        graphic_line(x0, y0, x1, y0, 1.0, 1, color_grid);
    }

    for (i = 0; i < imax; i += inc)
    {
        char text[16];
        int x0 = GRAPH_X0 - 6;
        int y0 = GRAPH_YMAX - (i * vscale);

        snprintf(text, sizeof(text), "%d", i * max);
        graphic_text(x0, y0, text, ALIGN_RIGHT, ALIGN_CENTER, ROTATE_NONE,
                color_text);
    }
    if (NULL != text)
    {
        graphic_text(GRAPH_X0 - 30, (GRAPH_Y0 + GRAPH_YMAX) / 2, text,
                ALIGN_CENTER, ALIGN_CENTER, ROTATE_LEFT, color_text);
    }
}

/*!
 * @fn          static void output_png_vscale_right(parser_data_t* data, int vscale, int inc, int max,
 *                  const char* text)
 * @brief       draw right vertical scale
 * @param[in]   data
 *              parser data
 * @param[in]   vscale
 *              vertical scale space
 * @param[in]   inc
 *              vertical scale increment
 * @param[in]   max
 *              vertical scale maximum
 * @param[in]   text
 *              label
 */
static void output_png_vscale_right(parser_data_t* data, int vscale, int inc, int max, const char* text)
{
    int i;
    int imax = (GRAPH_HEIGHT / vscale) + 1;

    for (i = 0; i < imax; i += inc)
    {
        char text[16];
        int x0 = GRAPH_XMAX + 6;
        int y0 = GRAPH_YMAX - (i * vscale);

        snprintf(text, sizeof(text), "%d", i * max);
        graphic_text(x0, y0, text, ALIGN_LEFT, ALIGN_CENTER, ROTATE_NONE,
                color_text);
    }
    if (NULL != text)
    {
        graphic_text(GRAPH_XMAX + 30, (GRAPH_Y0 + GRAPH_YMAX) / 2,
                text, ALIGN_CENTER, ALIGN_CENTER, ROTATE_RIGHT,
                color_text);
    }
}

/*!
 * @fn          static void output_png_caption(int index, const char* text, unsigned long color)
 * @brief       draw legend
 * @param[in]   index
 *              index of legend
 * @param[in]   text
 *              label
 * @param[in]   color
 *              color of box
 */
static void output_png_caption(int index, const char* text, unsigned long color)
{
    int x0 = GRAPH_X0 + (index * 128);
    int x1 = x0 + 15;
    int y0 = GRAPH_YMAX + 24;
    int y1 = y0;

    graphic_block(x0, y0, color, color_text);
    graphic_text(x1, y1, text, ALIGN_LEFT, ALIGN_UP, ROTATE_NONE, color_text);
}

/*!
 * @fn          static void output_png_finish(parser_data_t* data, const char* file)
 * @brief       finish png graphic
 * @details     save to file
 * @param[in]   data
 *              parser data
 * @param[in]   file
 *              .png file name
 */
static void output_png_finish(parser_data_t* data, const char* file)
{
    graphic_line(GRAPH_X0 - 5, GRAPH_YMAX, GRAPH_XMAX + 5, GRAPH_YMAX, 1.0, 1,
            color_coord);
    graphic_line(GRAPH_X0, GRAPH_Y0 - 5, GRAPH_X0, GRAPH_YMAX + 5, 1.0, 1,
            color_coord);

    graphic_save(file);
    graphic_delete();
}

/*!
 * @fn          void output_tone_bits_png(parser_data_t* data, const char* file)
 * @brief       draw tone bits graph
 * @param[in]   data
 *              parser data
 * @param[in]   file
 *              .png file name
 */
void output_tone_bits_png(parser_data_t* data, const char* file)
{
    int iTone;
    int iBand;
    int tone_scale = calc_tone_scale(data);

    remove (file);

    if (data->tone.bitalloc_up.valid ||
        data->tone.bitalloc_down.valid ||
        data->tone.gainQ2.valid)
    {
        if (0 == output_png_init(data))
        {
            output_png_hscale(data);
            output_png_vscale_left(data, GRAPH_VSCALE1, 3, 1, "bits");
            output_png_caption(0, "Up-Stream", color_up);
            output_png_caption(1, "Down-Stream", color_down);
            output_png_caption(2, "GAP", color_gap);
            if (data->tone.gainQ2.valid)
            {
                output_png_vscale_right(data, GRAPH_VSCALE1, 2, 10, "dB");
                output_png_caption(3, "Gain Q2", color_gainQ2);
            }

            /* upstream bitallocs */
            if (data->tone.bitalloc_up.valid)
            {
                for (iTone = 0; iTone < data->bandplan->num_tone; iTone++)
                {
                    if (data->tone.bitalloc_up.data[iTone])
                    {
                        int x = GRAPH_X0 + (iTone/tone_scale);
                        int y = GRAPH_YMAX
                                - (data->tone.bitalloc_up.data[iTone] * GRAPH_VSCALE1);
                        if (y < GRAPH_Y0)
                        {
                            y = GRAPH_Y0;
                        }
                        graphic_line(x, GRAPH_YMAX, x, y, 1.0, 0, color_up);
                    }
                }
            }
            /* dowstream bitallocs */
            if (data->tone.bitalloc_down.valid)
            {
                for (iTone = 0; iTone < data->bandplan->num_tone; iTone++)
                {
                    if (data->tone.bitalloc_down.data[iTone])
                    {
                        int x = GRAPH_X0 + (iTone/tone_scale);
                        int y = GRAPH_YMAX
                                - (data->tone.bitalloc_down.data[iTone] * GRAPH_VSCALE1);
                        if (y < GRAPH_Y0)
                        {
                            y = GRAPH_Y0;
                        }
                        graphic_line(x, GRAPH_YMAX, x, y, 1.0, 0, color_down);
                    }
                }
            }
            /* upstream gaps */
            if (data->tone.bitalloc_up.valid)
            {
                for (iBand = 0;iBand < data->bandplan->num_band; iBand++)
                {
                    int minTone = 0;
                    int maxTone = 0;
                    for (iTone = data->bandplan->band[iBand].up_min;
                            iTone < data->bandplan->band[iBand].up_max; iTone++)
                    {
                        if (data->tone.bitalloc_up.data[iTone])
                        {
                            if (0 == minTone)
                            {
                                minTone = iTone;
                            }
                            maxTone = iTone;
                        }
                    }

                    for (iTone = minTone; iTone < maxTone; iTone++)
                    {
                        if (0 == data->tone.bitalloc_up.data[iTone])
                        {
                            int x = GRAPH_X0 + (iTone/tone_scale);
                            int y = GRAPH_Y0 + 4;
                            graphic_line(x, GRAPH_Y0, x, y, 3.0, 0, color_gap);
                        }
                    }
                }
            }
            /* downtream gaps */
            if (data->tone.bitalloc_down.valid)
            {
                for (iBand = 0;iBand < data->bandplan->num_band; iBand++)
                {
                    int minTone = 0;
                    int maxTone = 0;
                    for (iTone = data->bandplan->band[iBand].down_min;
                            iTone < data->bandplan->band[iBand].down_max; iTone++)
                    {
                        if (data->tone.bitalloc_down.data[iTone])
                        {
                            if (0 == minTone)
                            {
                                minTone = iTone;
                            }
                            maxTone = iTone;
                        }
                    }

                    for (iTone = minTone; iTone < maxTone; iTone++)
                    {
                        if (0 == data->tone.bitalloc_down.data[iTone])
                        {
                            int x = GRAPH_X0 + (iTone/tone_scale);
                            int y = GRAPH_Y0 + 4;
                            graphic_line(x, GRAPH_Y0, x, y, 3.0, 0, color_gap);
                        }
                    }
                }
            }
            /* gain Q2 */
            if (data->tone.gainQ2.valid)
            {
                for (iTone = 0; iTone < data->bandplan->num_tone; iTone++)
                {
                    if (data->tone.gainQ2.data[iTone])
                    {
                        int x = GRAPH_X0 + (iTone/tone_scale);
                        int y = GRAPH_YMAX - (data->tone.gainQ2.data[iTone]);
                        if (y < GRAPH_Y0)
                        {
                            y = GRAPH_Y0;
                        }
                        graphic_line(x, y - 1, x, y + 1, 1.0, 0, color_gainQ2);
                    }
                }
            }
            output_png_finish(data, file);
        }
    }
}

/*!
 * @fn          void output_tone_snr_png(parser_data_t* data, const char* file)
 * @brief       draw SNR graph
 * @param[in]   data
 *              parser data
 * @param[in]   file
 *              .png file name
 */
void output_tone_snr_png(parser_data_t* data, const char* file)
{
    int iTone;
    int tone_scale = calc_tone_scale(data);

    remove (file);

    if (data->tone.snr.valid ||
        data->tone.noisemargin.valid)
    {
        if (0 == output_png_init(data))
        {
            output_png_hscale(data);
            output_png_vscale_left(data, GRAPH_VSCALE1, 2, 5, "dB");
            output_png_caption(0, "SNR", color_snr);
            if (data->tone.noisemargin.valid)
            {
                output_png_caption(1, "Noisemargin", color_noisemargin);
            }
            if (data->tone.snr.valid)
            {
                for (iTone = 0; iTone <= data->bandplan->num_tone; iTone++)
                {
                    if (data->tone.snr.data[iTone])
                    {
                        int x = GRAPH_X0 + (iTone/tone_scale);
                        int y = GRAPH_YMAX - ((data->tone.snr.data[iTone] * 2));
                        if (y < GRAPH_Y0)
                        {
                            y = GRAPH_Y0;
                        }
                        graphic_line(x, GRAPH_YMAX, x, y, 1.0, 0, color_snr);
                    }
                }
            }
            if (data->tone.noisemargin.valid)
            {
                for (iTone = 0; iTone <= data->bandplan->num_tone; iTone++)
                {
                    if (data->tone.noisemargin.data[iTone])
                    {
                        int x = GRAPH_X0 + (iTone/tone_scale);
                        int y = GRAPH_YMAX - ((data->tone.noisemargin.data[iTone] * 2));
                        if (y < GRAPH_Y0)
                        {
                            y = GRAPH_Y0;
                        }
                        graphic_line(x, GRAPH_YMAX, x, y, 1.0, 0,
                                color_noisemargin);
                    }
                }
            }

            output_png_finish(data, file);
        }
    }
}

/*!
 * @fn          void output_tone_char_png(parser_data_t* data, const char* file)
 * @brief       draw channel characteristics graph
 * @param[in]   data
 *              parser data
 * @param[in]   file
 *              .png file name
 */
void output_tone_char_png(parser_data_t* data, const char* file)
{
    int iTone;

    remove (file);

    if (data->tone.chancharlog.valid)
    {
        if (0 == output_png_init(data))
        {
            output_png_hscale(data);
            output_png_vscale_left(data, GRAPH_VSCALE2, 2, 10, NULL);
            output_png_caption(0, "Channel characteristics", color_chancharlog);

            for (iTone = 0; iTone < data->bandplan->num_tone; iTone++)
            {
                if (data->tone.chancharlog.data[iTone])
                {
                    int x = GRAPH_X0 + iTone;
                    int y = GRAPH_YMAX
                            - ((data->tone.chancharlog.data[iTone] * 3) / 2);
                    if (y < GRAPH_Y0)
                    {
                        y = GRAPH_Y0;
                    }
                    graphic_line(x, GRAPH_YMAX, x, y, 1.0, 0,
                            color_chancharlog);
                }
            }

            output_png_finish(data, file);
        }
    }
}

/* _oOo_ */
