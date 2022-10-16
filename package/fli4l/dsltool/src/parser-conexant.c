/*!
 * @file    parser-conexant.c
 * @brief   implementation file for conexant parser
 * @details parser for conexant based modems
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    2013.11.10
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
#include <stdlib.h>
#include <string.h>

#include "telnet.h"
#include "parser.h"
#include "log.h"

typedef struct
{
  int num;
  const char* str;
} num_str_t;

static int parse_main (int state, char* stream, size_t size);
static int parse_3_constatus (int state, char* stream, size_t size);
static int parse_3_1_general (int state, char* stream, size_t size);
static int parse_3_1_1_adsl (int state, char* stream, size_t size);
static int parse_3_2_adsl (int state, char* stream, size_t size);
static int parse_quit (int state, char* stream, size_t size);

/*!
 * @defgroup    command_conexant conexant menu commands
 * @{
 */
static const char* str_cmd_main = "m\r\n";
/*!< main menu */
static const char* str_cmd_constatus = "3\r\n";
/*!<  connection status */
static const char* str_cmd_3_general = "1\r\n";
/*!<  general connection status */
static const char* str_cmd_3_adsl = "2\r\n";
/*!<  adsl transceiver status */
static const char* str_cmd_3_1_adsl = "1\r\n";
/*!<  adsl status */
static const char* str_cmd_quit = "q\r\n";
/*!<  quit session */
static const char* str_cmd_quit_ok = "y\r\n";
/*!< exit telnet */
/*!@}*/

void parser_init(int command, const char* username, const char* password)
{
    int index = 0;

    _parser_init(command);

    switch (parser_cfg.command)
    {
#if defined (CFG_COMMAND)
    case PARSER_INFO:
#endif /* defined (CFG_COMMAND) */
    case PARSER_STATUS:
        /* initial main menu */
        parser_list[index].parser = parse_main;
        index++;

        parser_list[index].parser = parse_3_constatus;
        parser_list[index].cmd_str = str_cmd_constatus;
        index++;
        parser_list[index].parser = parse_3_1_general;
        parser_list[index].cmd_str = str_cmd_3_general;
        index++;
        parser_list[index].parser = parse_3_1_1_adsl;
        parser_list[index].cmd_str = str_cmd_3_1_adsl;
        index++;
        parser_list[index].parser = parse_main;
        parser_list[index].cmd_str = str_cmd_main;
        index++;

        parser_list[index].parser = parse_3_constatus;
        parser_list[index].cmd_str = str_cmd_constatus;
        index++;
        parser_list[index].parser = parse_3_2_adsl;
        parser_list[index].cmd_str = str_cmd_3_adsl;
        index++;
        parser_list[index].parser = parse_main;
        parser_list[index].cmd_str = str_cmd_main;
        index++;

        /* quit */
        parser_list[index].parser = parse_quit;
        parser_list[index].cmd_str = str_cmd_quit;
        index++;
        parser_list[index].cmd_str = str_cmd_quit_ok;
        index++;
        break;
#if defined (CFG_COMMAND)
    case PARSER_RESYNC:
    case PARSER_REBOOT:
        parser_list[index].parser = parse_quit;
        parser_list[index].cmd_str = str_cmd_quit;
        index++;
        parser_list[index].cmd_str = str_cmd_quit_ok;
        index++;
        break;
#endif /* defined (CFG_COMMAND) */
    }

    parser_cfg.user = (char*) username;
    parser_cfg.pass = (char*) password;
    parser_cfg.enter = "\r\n";
    snprintf(parser_cfg.prompt, sizeof(parser_cfg.prompt), "\x1b[24;04H");
    parser_cfg.no1prompt = 1;
}

enum
{
    state_main_start,
    state_main_exit = -1
};

/*!
 * @fn          static int parse_main (int state, char* stream, size_t size)
 * @brief       parse main menu
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_main (int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_main_start:
        if (parser_find(stream, size, "\x1b[05;36HMAIN MENU"))
        {
            log_debug("MAIN MENU\n");
            state = state_main_exit;
        }
        break;
    }

    return state;
}

enum
{
    state_3cs_start,
    state_3cs_exit = -1
};

/*!
 * @fn          static int parse_3_constatus (int state, char* stream, size_t size)
 * @brief       parse connection status menu
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_3_constatus (int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_3cs_start:
        if (parser_find(stream, size, "\x1b[04;31HConnection Status"))
        {
            log_debug("Connection Status\n");
            state = state_3cs_exit;
        }
        break;
    }

    return state;
}

enum
{
    state_31g_start,
    state_31g_exit = -1
};

/*!
 * @fn          static int parse_3_1_general (int state, char* stream, size_t size)
 * @brief       parse general connection status menu
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_3_1_general (int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_31g_start:
        if (parser_find(stream, size, "\x1b[04;31HGeneral Status"))
        {
            log_debug("General Status\n");
            state = state_31g_exit;
        }
        break;
    }

    return state;
}

enum
{
    state_311adsl_start,
    state_311adsl_up_snr,
    state_311adsl_up_snr_dat,
    state_311adsl_down_snr,
    state_311adsl_down_snr_dat,
    state_311adsl_up_attn,
    state_311adsl_up_attn_dat,
    state_311adsl_down_attn,
    state_311adsl_down_attn_dat,
    state_311adsl_tx_pwr,
    state_311adsl_tx_pwr_dat,
    state_311adsl_up_crc,
    state_311adsl_up_crc_dat,
    state_311adsl_down_crc,
    state_311adsl_down_crc_dat,
    state_311adsl_up_fec,
    state_311adsl_up_fec_dat,
    state_311adsl_down_fec,
    state_311adsl_down_fec_dat,
    state_311adsl_up_hec,
    state_311adsl_up_hec_dat,
    state_311adsl_down_hec,
    state_311adsl_down_hec_dat,
    state_311adsl_exit = -1
};

/*!
 * @fn          static int parse_3_1_1_adsl (int state, char* stream, size_t size)
 * @brief       parse adsl transceiver status
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_3_1_1_adsl (int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_311adsl_start:
        if (parser_find(stream, size, "\x1b[04;31HADSL Transceiver Status"))
        {
            log_debug("ADSL Transceiver Status\n");
            state = state_311adsl_up_snr;
        }
        break;
    case state_311adsl_up_snr:
        if (parser_find(stream, size, "\x1b[07;45H"))
        {
            state = state_311adsl_up_snr_dat;
        }
        break;
    case state_311adsl_up_snr_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            double data;
            if(1 == sscanf(stream, "\x1b[0K%lf dB", &data))
            {
                parser_data.noisemargin[0].up = data;
                parser_data.noisemargin[0].valid++;

                log_debug("noisemargin: up %f\n", data);
            }
            state = state_311adsl_down_snr;
        }
        break;
    case state_311adsl_down_snr:
        if (parser_find(stream, size, "\x1b[08;45H"))
        {
            state = state_311adsl_down_snr_dat;
        }
        break;
    case state_311adsl_down_snr_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            double data;
            if(1 == sscanf(stream, "\x1b[0K%lf dB", &data))
            {
                parser_data.noisemargin[0].down = data;
                parser_data.noisemargin[0].valid++;

                log_debug("noisemargin: down %f\n", data);
            }
            state = state_311adsl_up_attn;
        }
        break;
    case state_311adsl_up_attn:
        if (parser_find(stream, size, "\x1b[09;45H"))
        {
            state = state_311adsl_up_attn_dat;
        }
        break;
    case state_311adsl_up_attn_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            double data;
            if(1 == sscanf(stream, "\x1b[0K%lf dB", &data))
            {
                parser_data.attenuation[0].up = data;
                parser_data.attenuation[0].valid++;

                log_debug("attenuation: up %f\n", data);
            }
            state = state_311adsl_down_attn;
        }
        break;
    case state_311adsl_down_attn:
        if (parser_find(stream, size, "\x1b[10;45H"))
        {
            state = state_311adsl_down_attn_dat;
        }
        break;
    case state_311adsl_down_attn_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            double data;
            if(1 == sscanf(stream, "\x1b[0K%lf dB", &data))
            {
                parser_data.attenuation[0].down = data;
                parser_data.attenuation[0].valid++;

                log_debug("attenuation: down %f\n", data);
            }
            state = state_311adsl_tx_pwr;
        }
        break;
    case state_311adsl_tx_pwr:
        if (parser_find(stream, size, "\x1b[11;45H"))
        {
            state = state_311adsl_tx_pwr_dat;
        }
        break;
    case state_311adsl_tx_pwr_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            double data;
            if(1 == sscanf(stream, "\x1b[0K%lf dB", &data))
            {
                parser_data.txpower.up = -data;
                parser_data.txpower.valid += 2;

                log_debug("Tx Power: %f\n", data);
            }
            state = state_311adsl_up_crc;
        }
        break;
    case state_311adsl_up_crc:
        if (parser_find(stream, size, "\x1b[15;45H"))
        {
            state = state_311adsl_up_crc_dat;
        }
        break;
    case state_311adsl_up_crc_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            long data;
            if(1 == sscanf(stream, "\x1b[0K%ld", &data))
            {
                parser_data.statistics.error.CRC.Tx = data;
                parser_data.statistics.error.CRC.valid++;

                log_debug("CRC: Tx %ld\n", data);
            }
            state = state_311adsl_down_crc;
        }
        break;
    case state_311adsl_down_crc:
        if (parser_find(stream, size, "\x1b[16;45H"))
        {
            state = state_311adsl_down_crc_dat;
        }
        break;
    case state_311adsl_down_crc_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            long data;
            if(1 == sscanf(stream, "\x1b[0K%ld", &data))
            {
                parser_data.statistics.error.CRC.Rx = data;
                parser_data.statistics.error.CRC.valid++;

                log_debug("CRC: Rx %ld\n", data);
            }
            state = state_311adsl_up_fec;
        }
        break;
    case state_311adsl_up_fec:
        if (parser_find(stream, size, "\x1b[17;45H"))
        {
            state = state_311adsl_up_fec_dat;
        }
        break;
    case state_311adsl_up_fec_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            long data;
            if(1 == sscanf(stream, "\x1b[0K%ld", &data))
            {
                parser_data.statistics.error.FEC.Tx = data;
                parser_data.statistics.error.FEC.valid++;

                log_debug("FEC: Tx %ld\n", data);
            }
            state = state_311adsl_down_fec;
        }
        break;
    case state_311adsl_down_fec:
        if (parser_find(stream, size, "\x1b[18;45H"))
        {
            state = state_311adsl_down_fec_dat;
        }
        break;
    case state_311adsl_down_fec_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            long data;
            if(1 == sscanf(stream, "\x1b[0K%ld", &data))
            {
                parser_data.statistics.error.FEC.Rx = data;
                parser_data.statistics.error.FEC.valid++;

                log_debug("FEC: Rx %ld\n", data);
            }
            state = state_311adsl_up_hec;
        }
        break;
    case state_311adsl_up_hec:
        if (parser_find(stream, size, "\x1b[19;45H"))
        {
            state = state_311adsl_up_hec_dat;
        }
        break;
    case state_311adsl_up_hec_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            long data;
            if(1 == sscanf(stream, "\x1b[0K%ld", &data))
            {
                parser_data.statistics.error.HEC.Tx = data;
                parser_data.statistics.error.HEC.valid++;

                log_debug("HEC: Tx %ld\n", data);
            }
            state = state_311adsl_down_hec;
        }
        break;
    case state_311adsl_down_hec:
        if (parser_find(stream, size, "\x1b[20;45H"))
        {
            state = state_311adsl_down_hec_dat;
        }
        break;
    case state_311adsl_down_hec_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            long data;
            if(1 == sscanf(stream, "\x1b[0K%ld", &data))
            {
                parser_data.statistics.error.HEC.Rx = data;
                parser_data.statistics.error.HEC.valid++;

                log_debug("HEC: Rx %ld\n", data);
            }
            state = state_311adsl_exit;
        }
        break;
    }

    return state;
}

enum
{
    state_32adsl_start,
    state_32adsl_modulation,
    state_32adsl_modulation_dat,
    state_32adsl_annex,
    state_32adsl_annex_dat,
    state_32adsl_linestate,
    state_32adsl_linestate_dat,
    state_32adsl_up_rate,
    state_32adsl_up_rate_dat,
    state_32adsl_down_rate,
    state_32adsl_down_rate_dat,
    state_32adsl_exit = -1
};

/*!
 * @fn          static int parse_3_2_adsl (int state, char* stream, size_t size)
 * @brief       parse adsl status
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_3_2_adsl (int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_32adsl_start:
        if (parser_find(stream, size, "\x1b[04;31HADSL Status"))
        {
            log_debug("ADSL Status\n");
            state = state_32adsl_modulation;
        }
        break;
    case state_32adsl_modulation:
        if (parser_find(stream, size, "\x1b[05;50H"))
        {
            state = state_32adsl_modulation_dat;
        }
        break;
    case state_32adsl_modulation_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            char str[SCANF_BUFFER];

            log_debug("\"%s\"%d\n",stream, size);
            if(1 == sscanf(stream, "\x1b[0K"STR(SCANF_BUFFER,s), str))
            {
                strcpy(parser_data.operationmode.str, str);
                parser_data.operationmode.valid = 1;

                log_debug("modulation: %s\n", str);
            }
            state = state_32adsl_annex;
        }
        break;
    case state_32adsl_annex:
        if (parser_find(stream, size, "\x1b[06;50H"))
        {
            state = state_32adsl_annex_dat;
        }
        break;
    case state_32adsl_annex_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            char str[SCANF_BUFFER];
            log_debug("\"%s\"%d\n",stream, size);
            if(1 == sscanf(stream, "\x1b[0K"STR(SCANF_BUFFER,s), str))
            {
                if(parser_data.operationmode.str[0] != '\0')
                {
                    strcat(parser_data.operationmode.str, " ");
                }
                strcat(parser_data.operationmode.str, str);
                parser_data.operationmode.valid = 1;

                log_debug("annex: %s\n", str);
            }
            state = state_32adsl_linestate;
        }
        break;
    case state_32adsl_linestate:
        if (parser_find(stream, size, "\x1b[07;50H"))
        {
            state = state_32adsl_linestate_dat;
        }
        break;
    case state_32adsl_linestate_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            char str[SCANF_BUFFER];
            log_debug("\"%s\"%d\n",stream, size);
            if(1 == sscanf(stream, "\x1b[0K"STR(SCANF_BUFFER,s), str))
            {
                strcpy(parser_data.modemstate.str, str);
                if(0==strcmp(parser_data.modemstate.str,"SHOWTIME"))
                {
                    parser_data.modemstate.state = 1;
                }
                else
                {
                    parser_data.modemstate.state = 0;
                }
                parser_data.modemstate.valid = 2;

                log_debug("linestate: %s\n", str);
            }
            state = state_32adsl_up_rate;
        }
        break;
    case state_32adsl_up_rate:
        if (parser_find(stream, size, "\x1b[17;50H"))
        {
            state = state_32adsl_up_rate_dat;
        }
        break;
    case state_32adsl_up_rate_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            int data;
            if(1 == sscanf(stream, "\x1b[0K%d", &data))
            {
                parser_data.bandwidth.kbit.up = data;
                parser_data.bandwidth.kbit.valid++;

                log_debug("bandwidth: Up %d\n", data);
            }
            state = state_32adsl_down_rate;
        }
        break;
    case state_32adsl_down_rate:
        if (parser_find(stream, size, "\x1b[18;50H"))
        {
            state = state_32adsl_down_rate_dat;
        }
        break;
    case state_32adsl_down_rate_dat:
        if (parser_find(stream, size, "\x1b[0K"))
        {
            int data;
            if(1 == sscanf(stream, "\x1b[0K%d", &data))
            {
                parser_data.bandwidth.kbit.down = data;
                parser_data.bandwidth.kbit.valid++;

                log_debug("bandwidth: Down %d\n", data);
            }
            state = state_32adsl_exit;
        }
        break;
    }

    return state;
}

enum
{
    state_quit_start,
    state_quit_exit = -1
};

/*!
 * @fn          static int parse_quit (int state, char* stream, size_t size)
 * @brief       parse quit session
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_quit (int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_quit_start:
        if (parser_find(stream, size, "\x1b[04;38HQuit Session"))
        {
            log_debug("Quit Session\n");
            state = state_quit_exit;
        }
        break;
    }

    return state;
}

/* _oOo_ */
