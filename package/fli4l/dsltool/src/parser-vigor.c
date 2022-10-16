/*!
 * @file    parser-vigor.c
 * @brief   implementation file for vigor parser
 * @details parser for vigor modems
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    18.10.2014
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
#include <math.h>

#include "telnet.h"
#include "parser.h"
#include "log.h"

static int parse_show_adsl(int state, char* stream, size_t size);

/*!
 * @defgroup    command_vinax vinax telnet shell commands
 * @{
 */
static const char* str_cmd_show_adsl = "show adsl\r\n";
/*!< get dsl info */
static const char* str_cmd_quit = "quit\r\n";
/*!< exit telnet */

/*!@}*/

void parser_init(int command, const char* username, const char* password)
{
    _parser_init(command);
    parser_data.bandplan = &bandplan_NONE;

    switch (parser_cfg.command)
    {
#if defined (CFG_COMMAND)
    case PARSER_INFO:
        parser_list[0].cmd_str = str_cmd_show_adsl;
        parser_list[0].parser = parse_show_adsl;
        parser_list[1].cmd_str = str_cmd_quit;
        break;
#endif /* defined (CFG_COMMAND) */
    case PARSER_STATUS:
        parser_list[0].cmd_str = str_cmd_show_adsl;
        parser_list[0].parser = parse_show_adsl;
        parser_list[1].cmd_str = str_cmd_quit;
        break;
#if defined (CFG_COMMAND)
    case PARSER_RESYNC:
        parser_list[0].cmd_str = str_cmd_quit;
        break;
    case PARSER_REBOOT:
        parser_list[0].cmd_str = str_cmd_quit;
        break;
#endif /* defined (CFG_COMMAND) */
    }

    parser_cfg.user = (char*) username;
    parser_cfg.pass = (char*) password;
    parser_cfg.enter = "\r\n";
    snprintf(parser_cfg.prompt, sizeof(parser_cfg.prompt), "> ");
}

enum
{
    show_adsl_ATU_R,
    show_adsl_running_mode,
    show_adsl_actual_rate,
    show_adsl_max_rate,
    show_adsl_path_mode,
    show_adsl_up_attenuation,
    show_adsl_ATU_C,
    show_adsl_down_attenuation,
    show_adsl_exit = -1
};

/*!
 * @fn          int parse_dsl_info(int state, char* stream, size_t size)
 * @brief       parse environment
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
int parse_show_adsl(int state, char* stream, size_t size)
{
    switch (state)
    {
    case show_adsl_ATU_R:
        if (parser_find(stream, size, "ATU-R Info"))
        {
            log_debug("ATU-R Info\n");
            state = show_adsl_running_mode;
        }
        break;
    case show_adsl_running_mode:
        if (parser_find(stream, size, "Running Mode"))
        {
            char *temp;
            char strMode[SCANF_BUFFER];
            char strState[SCANF_BUFFER];
            temp = strstr(stream, "State");
            temp[-1] = '\0';

            if (1 == sscanf(stream, " Running Mode : "STR(SCANF_BUFFER,[a-zA-Z0-9 +]s), strMode))
            {
                parser_trim(strMode);
                log_debug("Running Mode: %s\n", strMode);

                if (0 == strcmp(strMode, "17A"))
                {
                    parser_data.bandplan = &bandplan_VDSL2_AnnexB_17a_998ADE17_M2x_B;
                }
                else if (0 == strcmp(strMode, "ADSL2+ Annex A"))
                {
                    parser_data.bandplan = &bandplan_ADSL2plus_AnnexA;
                }
                else if (0 == strcmp(strMode, "ADSL2+ Annex B"))
                {
                    parser_data.bandplan = &bandplan_ADSL2plus_AnnexB;
                }
                else if (0 == strcmp(strMode, "ADSL2+ Annex J"))
                {
                    parser_data.bandplan = &bandplan_ADSL2plus_AnnexJ;
                }
            }
            if (1 == sscanf(temp, " State : "STR(SCANF_BUFFER,s), strState))
            {
                log_debug("State: %s\n", strState);

                strcpy (parser_data.modemstate.str, strState);
                if(0 == strcmp(parser_data.modemstate.str, "SHOWTIME"))
                {
                    parser_data.modemstate.state = 1;
                }
                else
                {
                    parser_data.modemstate.state = 0;
                }
                parser_data.modemstate.valid = 2;
            }
            state = show_adsl_actual_rate;
        }
        break;
    case show_adsl_actual_rate:
        if (parser_find(stream, size, "DS Actual Rate"))
        {
            double up, down;
            if (2 == sscanf(stream, " DS Actual Rate : %lf bps US Actual Rate : %lf bps", &down, &up))
            {
                log_debug("Actual Rate DS: %lf US: %lf\n", down, up);
            }
            parser_data.bandwidth.kbit.down = down/1000;
            parser_data.bandwidth.kbit.up = up/1000;
            parser_data.bandwidth.kbit.valid = 2;

            state = show_adsl_max_rate;
        }
        break;
    case show_adsl_max_rate:
        if (parser_find(stream, size, "DS Attainable Rate"))
        {
            double up, down;
            if (2 == sscanf(stream, " DS Attainable Rate : %lf bps US Attainable Rate : %lf bps ", &down, &up))
            {
                log_debug("Attainable Rate DS: %lf US: %lf\n", down, up);
            }
            parser_data.maxbandwidth.kbit.down = down/1000;
            parser_data.maxbandwidth.kbit.up = up/1000;
            parser_data.maxbandwidth.kbit.valid = 2;

            state = show_adsl_path_mode;
        }
        break;
    case show_adsl_path_mode:
        if (parser_find(stream, size, "DS Path Mode"))
        {
            char strMode[SCANF_BUFFER];
            if (1 == sscanf(stream, " DS Path Mode : "STR(SCANF_BUFFER,s), strMode))
            {
                log_debug("Path Mode: %s\n", strMode);
                strcpy (parser_data.channelmode.str, strMode);
                parser_data.channelmode.valid = 1;
            }
            state = show_adsl_up_attenuation;
        }
        break;
    case show_adsl_up_attenuation:
        if (parser_find(stream, size, "NE Current Attenuation"))
        {
            double attenuation, noisemargin;
            int i;
            if (2 == sscanf(stream, " NE Current Attenuation : %lf dB Cur SNR Margin : %lf dB", &attenuation, &noisemargin))
            {
                log_debug("Attenuation: %lf SNR: %lf\n", attenuation, noisemargin);
            }
            for (i=0; i <parser_data.bandplan->num_band; i++)
            {
                parser_data.attenuation[i].up = attenuation;
                parser_data.attenuation[i].valid++;
                parser_data.noisemargin[i].up = noisemargin;
                parser_data.noisemargin[i].valid++;
            }

            state = show_adsl_ATU_C;
        }
        break;
    case show_adsl_ATU_C:
        if (parser_find(stream, size, "ATU-C Info"))
        {
            log_debug("ATU-C Info\n");
            state = show_adsl_down_attenuation;
        }
        break;
    case show_adsl_down_attenuation:
        if (parser_find(stream, size, "Far Current Attenuation"))
        {
            double attenuation, noisemargin;
            int i;
            if (2 == sscanf(stream, " Far Current Attenuation : %lf dB Far SNR Margin : %lf dB", &attenuation, &noisemargin))
            {
                log_debug("Attenuation: %lf SNR: %lf\n", attenuation, noisemargin);
            }
            for (i=0; i <parser_data.bandplan->num_band; i++)
            {
                parser_data.attenuation[i].down = attenuation;
                parser_data.attenuation[i].valid++;
                parser_data.noisemargin[i].down = noisemargin;
                parser_data.noisemargin[i].valid++;
            }

            state = show_adsl_exit;
        }
        break;
    default:
        state = show_adsl_exit;
    break;
    }

    return state;
}

// _oOo_
