/*!
 * @file    parser-vinax.c
 * @brief   implementation file for vinax parser
 * @details parser for modems based on vinax chipsets
 *
 * @author  Florian Wolters; florian@florian-wolters.de
 * @date    28.02.2014
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

static int parse_dsl_info(int state, char* stream, size_t size);
#if defined (CFG_COMMAND)
static int parse_vdsl_bits_upstream(int state, char* stream, size_t size);
static int parse_vdsl_bits_downstream(int state, char* stream, size_t size);
static int parse_vdsl_snr_downstream(int state, char* stream, size_t size);
#endif /*defined (CFG_COMMAND)*/

/*!
 * @defgroup    command_vinax vinax telnet shell commands
 * @{
 */
static const char* str_cmd_dsl_info = "dsl-info\r\n";
/*!< get dsl info */
#if defined (CFG_COMMAND)
static const char* str_cmd_vdsl_bits_upstream = "dsl_pipe g997bansg 0\r\n";
/*!< get upstream bitloading data*/
static const char* str_cmd_vdsl_bits_downstream = "dsl_pipe g997bansg 1\r\n";
/*!< get downstream bitloading data*/
static const char* str_cmd_vdsl_snr_downstream = "dsl_pipe g997sansg 1\r\n";
/*!< get downstream snr data*/
static const char* str_cmd_vdsl_resync =  "dsl_cpe_control -i 00_00_00_00_00_00_00_07 -f /firmware/vcpe_hw.bin -F /firmware/acpe_hw.bin -A /firmware/vdsl-annex-b.scr\r\n";
/*!< resync vdsl connection */
static const char* str_cmd_reboot = "reboot\r\n";
/*!< reboot modem*/
#endif /* defined (CFG_COMMAND) */
static const char* str_cmd_exit = "exit\r\n";
/*!< exit telnet */

/*!@}*/

void parser_init(int command, const char* username, const char* password)
{
    _parser_init(command);
    parser_data.bandplan = &bandplan_VDSL2_AnnexB_17a_998ADE17_M2x_B;

    switch (parser_cfg.command)
    {
#if defined (CFG_COMMAND)
    case PARSER_INFO:
        parser_list[0].cmd_str = str_cmd_dsl_info;
        parser_list[0].parser = parse_dsl_info;
        parser_list[1].cmd_str = str_cmd_vdsl_bits_upstream;
        parser_list[1].parser = parse_vdsl_bits_upstream;
        parser_list[2].cmd_str = str_cmd_vdsl_bits_downstream;
        parser_list[2].parser = parse_vdsl_bits_downstream;
        parser_list[3].cmd_str = str_cmd_vdsl_snr_downstream;
        parser_list[3].parser = parse_vdsl_snr_downstream;
        parser_list[4].cmd_str = str_cmd_exit;
        break;
#endif /* defined (CFG_COMMAND) */
    case PARSER_STATUS:
        parser_list[0].cmd_str = str_cmd_dsl_info;
        parser_list[0].parser = parse_dsl_info;
        parser_list[1].cmd_str = str_cmd_exit;
        break;
#if defined (CFG_COMMAND)
    case PARSER_RESYNC:
        parser_list[0].cmd_str = str_cmd_vdsl_resync;
        parser_list[1].cmd_str = str_cmd_exit;
        break;
    case PARSER_REBOOT:
        parser_list[0].cmd_str = str_cmd_reboot;
        parser_list[1].cmd_str = str_cmd_exit;
        break;
#endif /* defined (CFG_COMMAND) */
    }

    parser_cfg.user = (char*) username;
    parser_cfg.pass = (char*) password;
    parser_cfg.enter = "\r\n";
    snprintf(parser_cfg.prompt, sizeof(parser_cfg.prompt), "# ");
}

enum
{
    dsl_info_start,
    dsl_info_suc_link_retrains,
    dsl_info_failed_link_retrains,
    dsl_info_link_init_errors,
    dsl_info_link_init_timeouts,
    dsl_info_seconds_since_showtime,
    dsl_info_modem_status,
    dsl_info_selected_mode,
    dsl_info_trellis_coding,
    dsl_info_interleaver_mode,
    dsl_info_current_rate,
    dsl_info_attainable_rate,
    dsl_info_interleave_delay,
    dsl_info_latn_0,
    dsl_info_latn_1,
    dsl_info_latn_2,
    dsl_info_satn_0,
    dsl_info_satn_1,
    dsl_info_satn_2,
    dsl_info_snr_0,
    dsl_info_snr_1,
    dsl_info_snr_2,
    dsl_info_acatp,
    dsl_info_exit = -1
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
int parse_dsl_info(int state, char* stream, size_t size)
{
    switch (state)
    {
    case dsl_info_start:
        if (parser_find(stream, size, "dsl-info"))
        {
            log_debug("dsl-info\n");
        }
        state = dsl_info_suc_link_retrains;
        break;
    case dsl_info_suc_link_retrains:
        if (parser_find(stream, size, "Succeeded Link Retrains:"))
        {
            int suc_link_retrains;
            if( 1 == sscanf(stream, "Succeeded Link Retrains: %d", &suc_link_retrains))
            {
                log_debug("Succeeded Link Retrains: %d\n", suc_link_retrains);
            }
            state = dsl_info_failed_link_retrains;
        }
        break;
    case dsl_info_failed_link_retrains:
        if (parser_find(stream, size, "Failed Link Retrains:"))
        {
            int fail_link_retrains;
            if( 1 == sscanf(stream, "Failed Link Retrains: %d", &fail_link_retrains))
            {
                log_debug("Failed Link Retrains: %d\n", fail_link_retrains);
            }
            state = dsl_info_link_init_errors;
        }
        break;
    case dsl_info_link_init_errors:
        if (parser_find(stream, size, "Link Init Errors:"))
        {
            int link_init_errors;
            if( 1 == sscanf(stream, "Link Init Errors: %d", &link_init_errors))
            {
                log_debug("Link Init Errors: %d\n", link_init_errors);
            }
            state = dsl_info_link_init_timeouts;
        }
        break;
    case dsl_info_link_init_timeouts:
        if (parser_find(stream, size, "Link Init Timeouts:"))
        {
            int link_init_timeouts;
            if( 1 == sscanf(stream, "Link Init Timeouts: %d", &link_init_timeouts))
            {
                log_debug("Link Init Timeouts: %d\n", link_init_timeouts);
            }
            state = dsl_info_seconds_since_showtime;
        }
        break;
    case dsl_info_seconds_since_showtime:
        if (parser_find(stream, size, "Seconds since Showtime:"))
        {
            int secs_since_showtime;
            if( 1 == sscanf(stream, "Seconds since Showtime: %d", &secs_since_showtime))
            {
                log_debug("Seconds since Showtime: %d\n", secs_since_showtime);
            }
            state = dsl_info_modem_status;
        }
        break;
    case dsl_info_modem_status:
        if (parser_find(stream, size, "Modem status:"))
        {
            char modem_status[80];
            if( 1 == sscanf(stream, "Modem status: %[^\n]", modem_status))
            {
                log_debug("Modem status: %s\n", modem_status);
                strcpy(parser_data.modemstate.str, modem_status);
                if(0==strcmp(parser_data.modemstate.str,"Showtime tc sync"))
                {
                    parser_data.modemstate.state = 1;
                }
                else
                {
                    parser_data.modemstate.state = 0;
                }
                parser_data.modemstate.valid = 2;
                log_debug("linestate: %s\n", modem_status);
            }
            state = dsl_info_selected_mode;
        }
        break;   
    case dsl_info_selected_mode:
        if (parser_find(stream, size, "Selected DSL Mode:"))
        {
            char dsl_mode[80];
            if( 1 == sscanf(stream, "Selected DSL Mode: %[^\n]", dsl_mode))
            {
                log_debug("Selected DSL Mode: %s\n", dsl_mode);
                strcpy(parser_data.operationmode.str, dsl_mode);
                parser_data.operationmode.valid = 1;
            }
            state = dsl_info_trellis_coding;
        }
        break;   
    case dsl_info_trellis_coding:
        if (parser_find(stream, size, "Trellis Coding:"))
        {
            char trellis_coding[80];
            if( 1 == sscanf(stream, "Trellis Coding: %[^\n]", trellis_coding))
            {
                log_debug("Trellis Coding: %s\n", trellis_coding);
            }
            state = dsl_info_interleaver_mode;
        }
        break;   
    case dsl_info_interleaver_mode:
        if (parser_find(stream, size, "Interleaver Mode:"))
        {
            char interleaver_mode[80];
            if( 1 == sscanf(stream, "Interleaver Mode: %[^\n]", interleaver_mode))
            {
                log_debug("Interleaver Mode: %s\n", interleaver_mode);
            }
            state = dsl_info_current_rate;
        }
        break;   
   case dsl_info_current_rate:
        if (parser_find(stream, size, "Current Rate (kbps):"))
        {
            double cur_up, cur_down;
            if( 2 == sscanf(stream, "Current Rate (kbps): %lf %lf", &cur_down, &cur_up))
            {
                log_debug("Current Rate (kbps): %d / %d\n", (int)floor(cur_down+0.5), (int)floor(cur_up+0.5));
                parser_data.bandwidth.kbit.down = (int)floor(cur_down+0.5);
                parser_data.bandwidth.kbit.up = (int)floor(cur_up+0.5);
                parser_data.bandwidth.kbit.valid = 2;
            }
            state = dsl_info_attainable_rate;
        }
        break;
    case dsl_info_attainable_rate:
        if (parser_find(stream, size, "Attainable Rate (kbps):"))
        {
            double max_up, max_down;
            if( 2 == sscanf(stream, "Attainable Rate (kbps): %lf %lf", &max_down, &max_up))
            {
                log_debug("Attainable Rate (kbps): %d / %d\n", (int)floor(max_down+0.5), (int)floor(max_up+0.5));
                parser_data.maxbandwidth.kbit.down = (int)floor(max_down+0.5);
                parser_data.maxbandwidth.kbit.up = (int)floor(max_up+0.5);
                parser_data.maxbandwidth.kbit.valid = 2;
            }
            state = dsl_info_interleave_delay;
        }
        break;
    case dsl_info_interleave_delay:
        if (parser_find(stream, size, "Interleave Delay (ms):"))
        {
            int delay_up, delay_down;
            if( 2 == sscanf(stream, "Interleave Delay (ms): %d %d", &delay_down, &delay_up))
            {
                log_debug("Interleave Delay (ms): %d / %d\n", delay_down, delay_up);
            }
            state = dsl_info_latn_0;
        }
        break;
    case dsl_info_latn_0:
        if (parser_find(stream, size, "LATN[0] (dB):"))
        {
            double latn_0_up, latn_0_down;
            if( 2 == sscanf(stream, "LATN[0] (dB): %lf %lf", &latn_0_down, &latn_0_up))
            {
                log_debug("LATN[0] (dB): %2.1lf / %2.1lf\n", latn_0_down, latn_0_up);
                parser_data.attenuation[0].down = latn_0_down;
                parser_data.attenuation[0].up = latn_0_up;
                parser_data.attenuation[0].valid = 2;
            }
            state = dsl_info_latn_1;
        }
        break;
    case dsl_info_latn_1:
        if (parser_find(stream, size, "LATN[1] (dB):"))
        {
            double latn_1_up, latn_1_down;
            if( 2 == sscanf(stream, "LATN[1] (dB): %lf %lf", &latn_1_down, &latn_1_up))
            {
                log_debug("LATN[1] (dB): %2.1lf / %2.1lf\n", latn_1_down, latn_1_up);
                parser_data.attenuation[1].down = latn_1_down;
                parser_data.attenuation[1].up = latn_1_up;
                parser_data.attenuation[1].valid = 2;
            }
            state = dsl_info_latn_2;
        }
        break;
    case dsl_info_latn_2:
        if (parser_find(stream, size, "LATN[2] (dB):"))
        {
            double latn_2_up, latn_2_down;
            if( 2 == sscanf(stream, "LATN[2] (dB): %lf %lf", &latn_2_down, &latn_2_up))
            {
                log_debug("LATN[2] (dB): %2.1lf / %2.1lf\n", latn_2_down, latn_2_up);
                parser_data.attenuation[2].down = latn_2_down;
                parser_data.attenuation[2].up = latn_2_up;
                parser_data.attenuation[2].valid = 2;
            }
            state = dsl_info_satn_0;
        }
        break;
    case dsl_info_satn_0:
        if (parser_find(stream, size, "SATN[0] (dB):"))
        {
            double satn_0_up, satn_0_down;
            if( 2 == sscanf(stream, "SATN[0] (dB): %lf %lf", &satn_0_down, &satn_0_up))
            {
                log_debug("SATN[0] (dB): %2.1f / %2.1f\n", satn_0_down, satn_0_up);
            }
            state = dsl_info_satn_1;
        }
        break;
    case dsl_info_satn_1:
        if (parser_find(stream, size, "SATN[1] (dB):"))
        {
            double satn_1_up, satn_1_down;
            if( 2 == sscanf(stream, "SATN[1] (dB): %lf %lf", &satn_1_down, &satn_1_up))
            {
                log_debug("SATN[1] (dB): %2.1lf / %2.1lf\n", satn_1_down, satn_1_up);
            }
            state = dsl_info_satn_2;
        }
        break;
    case dsl_info_satn_2:
        if (parser_find(stream, size, "SATN[2] (dB):"))
        {
            double satn_2_up, satn_2_down;
            if( 2 == sscanf(stream, "SATN[2] (dB): %lf %lf", &satn_2_down, &satn_2_up))
            {
                log_debug("SATN[2] (dB): %2.1lf / %2.1lf\n", satn_2_down, satn_2_up);
            }
            state = dsl_info_snr_0;
        }
        break;
    case dsl_info_snr_0:
        if (parser_find(stream, size, "SNR Margin[0] (dB):"))
        {
            double snr_0_up, snr_0_down;
            if( 2 == sscanf(stream, "SNR Margin[0] (dB): %lf %lf", &snr_0_down, &snr_0_up))
            {
                log_debug("SNR Margin[0] (dB): %2.1lf / %2.1lf\n", snr_0_down, snr_0_up);
                parser_data.noisemargin[0].down = snr_0_down;
                parser_data.noisemargin[0].up = snr_0_up;
                parser_data.noisemargin[0].valid = 2;
            }
            state = dsl_info_snr_1;
        }
    case dsl_info_snr_1:
        if (parser_find(stream, size, "SNR Margin[1] (dB):"))
        {
            double snr_1_up, snr_1_down;
            if( 2 == sscanf(stream, "SNR Margin[1] (dB): %lf %lf", &snr_1_down, &snr_1_up))
            {
                log_debug("SNR Margin[1] (dB): %2.1lf / %2.1lf\n", snr_1_down, snr_1_up);
                parser_data.noisemargin[1].down = snr_1_down;
                parser_data.noisemargin[1].up = snr_1_up;
                parser_data.noisemargin[1].valid = 2;
            }
            state = dsl_info_snr_2;
        }
    case dsl_info_snr_2:
        if (parser_find(stream, size, "SNR Margin[2] (dB):"))
        {
            double snr_2_up, snr_2_down;
            if( 2 == sscanf(stream, "SNR Margin[2] (dB): %lf %lf", &snr_2_down, &snr_2_up))
            {
                log_debug("SNR Margin[2] (dB): %2.1lf / %2.1lf\n", snr_2_down, snr_2_up);
                parser_data.noisemargin[2].down = snr_2_down;
                parser_data.noisemargin[2].up = snr_2_up;
                parser_data.noisemargin[2].valid = 2;
            }
            state = dsl_info_acatp;
        }
        break;
    case dsl_info_acatp:
        if (parser_find(stream, size, "ACATP (dB):"))
        {
            double acatp_up, acatp_down;
            if( 2 == sscanf(stream, "ACATP (dB): %lf %lf", &acatp_down, &acatp_up))
            {
                log_debug("ACATP (dB): %2.1lf / %2.1lf\n", acatp_down, acatp_up);
                parser_data.txpower.down = acatp_down;
                parser_data.txpower.up = acatp_up;
                parser_data.txpower.valid = 2;
            }
            state = dsl_info_exit;
        }
        break;
    default:
        state = dsl_info_exit;
    break;
    }

    return state;
}

#if defined (CFG_COMMAND)
enum
{
    parse_vdsl_bits_upstream_start,
    parse_vdsl_bits_upstream_numdata,
    parse_vdsl_bits_upstream_nformat,
    parse_vdsl_bits_upstream_data,
    parse_vdsl_bits_upstream_exit = -1
};

/*!
 * @fn          int parse_vdsl_bits_upstream(int state, char* stream, size_t size)
 * @brief       parse upstream bitloading info
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
int parse_vdsl_bits_upstream(int state, char* stream, size_t size)
{
    int i;
    switch(state)
    {
    case parse_vdsl_bits_upstream_start:
        if (parser_find(stream, size, "dsl_pipe g997bansg 0"))
        {
            log_debug("dsl_pipe g997bansg 0\n");
            state = parse_vdsl_bits_upstream_numdata;
        }
    break;
    case parse_vdsl_bits_upstream_numdata:
        if (parser_find(stream, size, "nReturn=0 nDirection=0 nNumData="))
        {
            int num;
            if ( 1 == sscanf(stream, "nReturn=0 nDirection=0 nNumData=%d", &num))
            {
                log_debug("nNumData=%d\n", num);
            }
            state = parse_vdsl_bits_upstream_nformat;
        }
        break;
    case parse_vdsl_bits_upstream_nformat:
        if (parser_find(stream, size, "nFormat=bits(hex) nData="))
        {
            log_debug("nFormat found\n");
            state = parse_vdsl_bits_upstream_data;
        }
        break;
    case parse_vdsl_bits_upstream_data:
        //log_debug("Data = %d \"%s\"\n", size, stream);
        for(i=0;i<parser_data.bandplan->num_tone;i++)
        {
            int data;
            if(1==sscanf(&stream[3*i], "%x", &data))
            {
                parser_data.tone.bitalloc_up.data[i]=data;
            }
        }
        parser_data.tone.bitalloc_up.valid = 1;
        state = parse_vdsl_bits_upstream_exit;
        break;
    default:
        state = parse_vdsl_bits_upstream_exit;
    break;
    }
    return state;
}

enum
{
    parse_vdsl_bits_downstream_start,
    parse_vdsl_bits_downstream_numdata,
    parse_vdsl_bits_downstream_nformat,
    parse_vdsl_bits_downstream_data,
    parse_vdsl_bits_downstream_exit = -1
};

/*!
 * @fn          int parse_vdsl_bits_downstream(int state, char* stream, size_t size)
 * @brief       parse downstream bitloading info
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
int parse_vdsl_bits_downstream(int state, char* stream, size_t size)
{
    int i;
    switch(state)
    {
    case parse_vdsl_bits_downstream_start:
        if (parser_find(stream, size, "dsl_pipe g997bansg 1"))
        {
            log_debug("dsl_pipe g997bansg 1\n");
            state = parse_vdsl_bits_downstream_numdata;
        }
    break;
    case parse_vdsl_bits_downstream_numdata:
        if (parser_find(stream, size, "nReturn=0 nDirection=1 nNumData="))
        {
            int num;
            if ( 1 == sscanf(stream, "nReturn=0 nDirection=1 nNumData=%d", &num))
            {
                log_debug("nNumData=%d\n", num);
            }
            state = parse_vdsl_bits_downstream_nformat;
        }
        break;
    case parse_vdsl_bits_downstream_nformat:
        if (parser_find(stream, size, "nFormat=bits(hex) nData="))
        {
            log_debug("nFormat found\n");
            state = parse_vdsl_bits_downstream_data;
        }
        break;
    case parse_vdsl_bits_downstream_data:
        //log_debug("Data = %d \"%s\"\n", size, stream);
        for(i=0;i<parser_data.bandplan->num_tone;i++)
        {
            int data;
            if(1==sscanf(&stream[3*i], "%x", &data))
            {
                parser_data.tone.bitalloc_down.data[i]=data;
            }
        }
        parser_data.tone.bitalloc_down.valid = 1;
        state = parse_vdsl_bits_downstream_exit;
        break;
    default:
        state = parse_vdsl_bits_downstream_exit;
    break;
    }
    return state;
}

enum
{
    parse_vdsl_snr_downstream_start,
    parse_vdsl_snr_downstream_numdata,
    parse_vdsl_snr_downstream_data,
    parse_vdsl_snr_downstream_exit = -1
};

/*!
 * @fn          int parse_vdsl_snr_downstream(int state, char* stream, size_t size)
 * @brief       parse downstream bitloading info
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
int parse_vdsl_snr_downstream(int state, char* stream, size_t size)
{
    int i;
    int offset;
    switch(state)
    {
    case parse_vdsl_snr_downstream_start:
        if (parser_find(stream, size, "dsl_pipe g997sansg 1"))
        {
            log_debug("dsl_pipe g997sansg 1\n");
            state = parse_vdsl_snr_downstream_numdata;
        }
    break;
    case parse_vdsl_snr_downstream_numdata:
        if (parser_find(stream, size, "nReturn=0 nDirection=1 nNumData="))
        {
            int num;
            if ( 1 == sscanf(stream, "nReturn=0 nDirection=1 nNumData=%d", &num))
            {
                log_debug("nNumData=%d\n", num);
            }
            state = parse_vdsl_snr_downstream_data;
        }
        break;
    case parse_vdsl_snr_downstream_data:
        if (parser_find(stream, size, "nFormat=snr(hex) nData=\""))
        {
            offset = 24;
            log_debug("nFormat found\n");
            for(i=0;i<parser_data.bandplan->num_tone;i++)
            {
                int data;
                if(1==sscanf(&stream[3*i+offset], "%x", &data))
                {
                    if(255==data)
                    {
                        data = 0;
                    } else {
                        data = data / 2 - 32;
                    }
                    parser_data.tone.snr.data[i]=data;
                }
            }
            parser_data.tone.snr.valid = 1;
            state = parse_vdsl_snr_downstream_exit;
        }
        break;
    default:
        state = parse_vdsl_snr_downstream_exit;
    break;
    }
    return state;
}
#endif /*defined (CFG_COMMAND)*/


// _oOo_
