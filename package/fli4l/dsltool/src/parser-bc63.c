/*!
 * @file    parser-bc63.c
 * @brief   implementation file for bc63 parser
 * @details parser for Broadcom bc63xx based modems
 *
 * @author  Carsten Spie&szlig; fli4l@carsten-spiess.de
 * @date    2013.02.20
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

typedef struct
{
  int num;
  const char* str;
} num_str_t;

/*!
 * @struct      speedtouch_data_t
 * @brief       speedtouch private temporary data
 */
typedef struct {
    enum {
        eDefault,
        eVMG1312,
    } type;
} bc63_data_t;

/*!
 * @var         static bc63_data_t local_data;
 * @brief       bc63 private temporary data
 */
static bc63_data_t local_data;

static int parse_adsl_info_stats (int state, char* stream, size_t size);
#if defined (CFG_COMMAND)
static int parse_adsl_info_vendor (int state, char* stream, size_t size);
static int parse_atm_vcc_show (int state, char* stream, size_t size);
static int parse_adsl_info_bits (int state, char* stream, size_t size);
static void finish_adsl_info_bits (void);
static int parse_adsl_info_snr (int state, char* stream, size_t size);
#endif /* defined (CFG_COMMAND) */

/*!
 * @defgroup    command_bc63 bc63xx adslctl commands
 * @{
 */
static const char* str_cmd_adsl_info_stats = "adsl info --stats\n";
/*!< adsl statistics */
#if defined (CFG_COMMAND)
static const char* str_cmd_adsl_info_vendor = "adsl info --vendor\n";
/*!< adsl vendor */
static const char* str_cmd_atm_vcc_show = "atm vcc show\n";
/*!< atm vcc data */
static const char* str_cmd_adsl_info_bits = "adsl info --Bits\n";
/*!< adsl bitallocation table*/
static const char* str_cmd_adsl_info_snr= "adsl info --SNR\n";
/*!< adsl SNR table*/
static const char* str_cmd_reboot = "reboot\n";
/*!< reboot modem */
#endif /* defined (CFG_COMMAND) */
static const char* str_cmd_logout = "logout\n";
/*!< exit telnet */
/*!@}*/

void parser_init(int command, const char* username, const char* password)
{
    _parser_init(command);

    local_data.type = eDefault;

    switch (parser_cfg.command)
    {
#if defined (CFG_COMMAND)
    case PARSER_INFO:
        parser_list[0].parser = parse_adsl_info_stats;
        parser_list[0].cmd_str = str_cmd_adsl_info_stats;
        parser_list[1].parser = parse_adsl_info_vendor;
        parser_list[1].cmd_str = str_cmd_adsl_info_vendor;
        parser_list[2].parser = parse_atm_vcc_show;
        parser_list[2].cmd_str = str_cmd_atm_vcc_show;
        parser_list[3].parser = parse_adsl_info_bits;
        parser_list[3].cmd_str = str_cmd_adsl_info_bits;
        parser_list[3].finish = finish_adsl_info_bits;
        parser_list[4].parser = parse_adsl_info_snr;
        parser_list[4].cmd_str = str_cmd_adsl_info_snr;
        parser_list[5].cmd_str = str_cmd_logout;
        break;
#endif /* defined (CFG_COMMAND) */
    case PARSER_STATUS:
        parser_list[0].parser = parse_adsl_info_stats;
        parser_list[0].cmd_str = str_cmd_adsl_info_stats;
        parser_list[1].cmd_str = str_cmd_logout;
        break;
#if defined (CFG_COMMAND)
    case PARSER_RESYNC:
        parser_list[0].cmd_str = str_cmd_logout;
        break;
    case PARSER_REBOOT:
        parser_list[0].cmd_str = str_cmd_reboot;
        break;
#endif /* defined (CFG_COMMAND) */
    }

    parser_cfg.user = (char*) username;
    parser_cfg.pass = (char*) password;
    parser_cfg.enter = "\r";
    parser_cfg.null = 1;
    snprintf(parser_cfg.prompt, sizeof(parser_cfg.prompt), "> ");
}

enum
{
    state_aistats_start,
    state_aistats_status,
    state_aistats_adslrate,
    state_aistats_vdslrate,
    state_aistats_operation_mode,
    state_aistats_channel_mode,
    state_aistats_vdsl_profile,
    state_aistats_noisemargin,
    state_aistats_attenuation,
    state_aistats_power,
    state_aistats_rate,
    state_aistats_bearer0,
    state_aistats_error_HEC,
    state_aistats_total,
    state_aistats_total_FEC,
    state_aistats_total_CRC,
    state_aistats_lastday,
    state_aistats_lastday_ES,
    state_aistats_last15m,
    state_aistats_last15m_ES,
    state_aistats_exit  = -1
};

/*!
 * @fn          static int parse_adsl_info_stats(int state, char* stream, size_t size)
 * @brief       parse statistics data
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_adsl_info_stats(int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_aistats_start:
        if (parser_find(stream, size, "adsl info --stats"))
        {
            state = state_aistats_status;
            log_debug("adsl info --stats\n");
        }
        break;
    case state_aistats_status:
        if (parser_find(stream, size, "Status"))
        {
            char str[SCANF_BUFFER];
            if (1 == sscanf(stream, " Status: "STR(SCANF_BUFFER,s), str))
            {
                strcpy(parser_data.modemstate.str, str);

                if(0==strcmp(parser_data.modemstate.str,"Showtime"))
                {
                    parser_data.modemstate.state = 1;
                }
                else
                {
                    parser_data.modemstate.state = 0;
                }
                parser_data.modemstate.valid = 2;

                log_debug("modemstate: %d %s\n", parser_data.modemstate.state, parser_data.modemstate.str);
            }
            state = state_aistats_adslrate;
            log_debug("adsl info --stats\n");
        }
        break;
    case state_aistats_adslrate:
        if (parser_find(stream, size, "Channel"))
        {
            char str[SCANF_BUFFER];
            int down, up;
            if (3 == sscanf(stream, " Channel: "STR(SCANF_BUFFER,s)" Upstream rate = %d Kbps, Downstream rate = %d Kbps", str, &up, &down))
            {
                parser_data.bandwidth.kbit.down = down;
                parser_data.bandwidth.kbit.up = up;
                parser_data.bandwidth.kbit.valid = 2;

                log_debug("bandwidth down: %d up: %d\n", parser_data.bandwidth.kbit.down, parser_data.bandwidth.kbit.up);
            }
            state = state_aistats_operation_mode;
        }
        else if (parser_find(stream, size, "Max"))
        {
            int down, up;
            if (2 == sscanf(stream, " Max: Upstream rate = %d Kbps, Downstream rate = %d Kbps", &up, &down))
            {
                parser_data.maxbandwidth.kbit.down = down;
                parser_data.maxbandwidth.kbit.up = up;
                parser_data.maxbandwidth.kbit.valid = 2;

                log_debug("maxbandwidth down: %d up: %d\n", parser_data.maxbandwidth.kbit.down, parser_data.maxbandwidth.kbit.up);
            }
            state = state_aistats_vdslrate;
        }
        break;
    case state_aistats_vdslrate:
        if (parser_find(stream, size, "Bearer"))
        {
            int down, up;
            if (2 == sscanf(stream, " Bearer: 0, Upstream rate = %d Kbps, Downstream rate = %d Kbps", &up, &down))
            {
                parser_data.bandwidth.kbit.down = down;
                parser_data.bandwidth.kbit.up = up;
                parser_data.bandwidth.kbit.valid = 2;

                log_debug("bandwidth down: %d up: %d\n", parser_data.bandwidth.kbit.down, parser_data.bandwidth.kbit.up);
            }
            state = state_aistats_operation_mode;
        }
        break;
    case state_aistats_operation_mode:
        if (parser_find(stream, size, "Mode"))
        {
            char str[SCANF_BUFFER];
            if(1 == sscanf(stream, " Mode: "STR(SCANF_BUFFER,[^\n]), str))
            {
                strcpy(parser_data.operationmode.str, str);
                parser_data.operationmode.valid = 1;

                if (0 == strcmp(str, "ADSL2+"))
                {
                    parser_data.bandplan = &bandplan_ADSL2plus_AnnexB;
                    state = state_aistats_channel_mode;
                    log_debug("operationmode: %s\n", parser_data.operationmode.str);
                }
                else if (0 == strcmp(str, "ADSL2"))
                {
                    parser_data.bandplan = &bandplan_ADSL2_AnnexB;
                    state = state_aistats_channel_mode;
                    log_debug("operationmode: %s\n", parser_data.operationmode.str);
                }
                else if (0 ==strcmp(str, "VDSL2 Annex B"))
                {
                    state = state_aistats_vdsl_profile;
                    local_data.type = eVMG1312;
#if defined(CFG_COMMAND)
                    parser_list[2].parser = parser_list[3].parser;
                    parser_list[2].cmd_str = parser_list[3].cmd_str;
                    parser_list[2].finish = parser_list[3].finish;
                    parser_list[3].parser = parser_list[4].parser;
                    parser_list[3].cmd_str = parser_list[4].cmd_str;
                    parser_list[3].finish = parser_list[4].finish;
                    parser_list[4].parser = parser_list[5].parser;
                    parser_list[4].cmd_str = parser_list[5].cmd_str;
                    parser_list[4].finish = parser_list[5].finish;
                    parser_list[5].parser = NULL;
                    parser_list[5].cmd_str = NULL;
                    parser_list[5].finish = NULL;
#endif /* defined(CFG_COMMAND) */
                    log_debug("operationmode: VDSL2\n");
                }
                else
                {
                    state = state_aistats_channel_mode;
                    log_debug("operationmode: %s\n", parser_data.operationmode.str);
                }
            }
        }
        break;
    case state_aistats_channel_mode:
        if (parser_find(stream, size, "Channel"))
        {
            char str[SCANF_BUFFER];
            if (1 == sscanf(stream, " Channel: "STR(SCANF_BUFFER,s), str))
            {
                strcpy(parser_data.channelmode.str, str);
                parser_data.channelmode.valid = 1;

                log_debug("channelmode: %s\n", parser_data.channelmode.str);
            }
            state = state_aistats_noisemargin;
        }
        break;
    case state_aistats_vdsl_profile:
        if (parser_find(stream, size, "VDSL2 Profile"))
        {
            char str[SCANF_BUFFER];
            if (1 == sscanf(stream, " VDSL2 Profile: "STR(SCANF_BUFFER,s), str))
            {
                if (strcmp(str, "Profile 17a"))
                {
                    parser_data.bandplan = &bandplan_VDSL2_AnnexB_17a_998ADE17_M2x_B;
                    log_debug("operationmode: %s\n", parser_data.operationmode.str);
                }

            }
            state = state_aistats_noisemargin;
        }
        break;
    case state_aistats_noisemargin:
        if (parser_find(stream, size, "SNR"))
        {
            double down, up;
            if (2 == sscanf(stream, " SNR (dB): %lf %lf", &down, &up))
            {
                parser_data.noisemargin[0].down = down;
                parser_data.noisemargin[0].up = up;
                parser_data.noisemargin[0].valid = 2;

                log_debug("noisemargin: down %f up %f\n",
                        parser_data.noisemargin[0].down,
                        parser_data.noisemargin[0].up);
            }
            state = state_aistats_attenuation;
        }
        break;
    case state_aistats_attenuation:
        if (parser_find(stream, size, "Attn"))
        {
            double down, up;
            if (2 == sscanf(stream, " Attn(dB): %lf %lf", &down, &up))
            {
                parser_data.attenuation[0].down = down;
                parser_data.attenuation[0].up = up;
                parser_data.attenuation[0].valid = 2;

                log_debug("attenuation: down %f up %f\n",
                        parser_data.attenuation[0].down,
                        parser_data.attenuation[0].up);
            }
            state = state_aistats_power;
        }
        break;
    case state_aistats_power:
        if (parser_find(stream, size, "Pwr"))
        {
            double down, up;
            if (2 == sscanf(stream, " Pwr (dBm): %lf %lf", &down, &up))
            {
                parser_data.txpower.down = down;
                parser_data.txpower.up = up;
                parser_data.txpower.valid = 2;

                log_debug("txpower: down %f up %f\n",
                        parser_data.txpower.down,
                        parser_data.txpower.up);
            }
            if (eVMG1312 == local_data.type)
            {
                state = state_aistats_bearer0;
            }
            else
            {
                state = state_aistats_rate;
            }
        }
        break;
    case state_aistats_rate:
        if (parser_find(stream, size, "Rate"))
        {
            int down, up;
            if( 2 == sscanf(stream, " Rate (Kbps): %d %d", &down, &up))
            {
                parser_data.bandwidth.kbit.down = down;
                parser_data.bandwidth.kbit.up = up;
                parser_data.bandwidth.kbit.valid = 2;

                log_debug("bandwidth down: %d up: %d\n",
                        parser_data.bandwidth.kbit.down,
                        parser_data.bandwidth.kbit.up);
            }
            state = state_aistats_error_HEC;
        }
        break;
    case state_aistats_bearer0:
        if (parser_find(stream, size, "Bearer 0"))
        {
            state = state_aistats_error_HEC;
            log_debug("  Bearer 0\n");
        }
        break;
    case state_aistats_error_HEC:
        if (parser_find(stream, size, "HEC"))
        {
            long int down, up;
            if( 2 == sscanf(stream, " HEC: %ld %ld", &down, &up))
            {
                parser_data.statistics.error.HEC.Rx = down;
                parser_data.statistics.error.HEC.Tx = up;
                parser_data.statistics.error.HEC.valid = 2;

                log_debug("HEC rx: %ld tx:%ld\n",
                        parser_data.statistics.error.HEC.Rx,
                        parser_data.statistics.error.HEC.Tx);
            }
            state = state_aistats_total;
        }
        break;
    case state_aistats_total:
        if (parser_find(stream, size, "Total time"))
        {
            if (eVMG1312 == local_data.type)
            {
                state = state_aistats_total_FEC;
            }
            else
            {
                state = state_aistats_total_CRC;
            }
            log_debug("Total time\n");
        }
        break;
    case state_aistats_total_FEC:
        if (parser_find(stream, size, "FEC"))
        {
            long int down, up;
            if (2 == sscanf(stream, " FEC: %ld %ld", &down, &up))
            {
                parser_data.statistics.error.FEC.Rx = down;
                parser_data.statistics.error.FEC.Tx = up;
                parser_data.statistics.error.FEC.valid = 2;

                log_debug("FEC Rx: %ld Tx: %ld\n", parser_data.statistics.error.FEC.Rx, parser_data.statistics.error.FEC.Tx);
            }
            state = state_aistats_total_CRC;
        }
        break;
    case state_aistats_total_CRC:
        if (parser_find(stream, size, "CRC"))
        {
            long int down, up;
            if (eVMG1312 == local_data.type)
            {
                if (2 == sscanf(stream, " CRC: %ld %ld", &down, &up))
                {
                    parser_data.statistics.error.CRC.Rx = down;
                    parser_data.statistics.error.CRC.Tx = up;
                    parser_data.statistics.error.CRC.valid = 2;

                    log_debug("CRC Rx: %ld Tx: %ld\n", parser_data.statistics.error.CRC.Rx, parser_data.statistics.error.CRC.Tx);
                }
            }
            else
            {
                if (1 == sscanf(stream, " CRC = %ld", &down))
                {
                    parser_data.statistics.error.CRC.Rx = down;
                    parser_data.statistics.error.CRC.Tx = 0;
                    parser_data.statistics.error.CRC.valid = 2;

                    log_debug("CRC Rx: %ld\n", parser_data.statistics.error.CRC.Rx);
                }
            }
            if (eVMG1312 == local_data.type)
            {
                state = state_aistats_last15m;
            }
            else
            {
                state = state_aistats_lastday;
            }
        }
        break;
    case state_aistats_lastday:
        if (parser_find(stream, size, "Latest 1 day time"))
        {
            state = state_aistats_lastday_ES;
            log_debug("Latest day\n");
        }
        break;
    case state_aistats_lastday_ES:
        if (parser_find(stream, size, "ES"))
        {
            if (eVMG1312 == local_data.type)
            {
                long int down, up;
                if (2 == sscanf(stream, " ES: %ld %ld", &down, &up))
                {
                    parser_data.statistics.failure.error_secs_day = down + up;
                    parser_data.statistics.failure.valid++;

                    log_debug("last day error seconds %ld\n", parser_data.statistics.failure.error_secs_day);
                }
            }
            else
            {
                long int ES;
                if (1 == sscanf(stream, " ES = %ld", &ES))
                {
                    parser_data.statistics.failure.error_secs_day = ES;
                    parser_data.statistics.failure.valid++;

                    log_debug("last day error seconds %ld\n", parser_data.statistics.failure.error_secs_day);
                }
            }
            if (eVMG1312 == local_data.type)
            {
                state = state_aistats_exit;
            }
            else
            {
                state = state_aistats_last15m;
            }
        }
        break;
    case state_aistats_last15m:
        if (parser_find(stream, size, "Latest 15 minutes time"))
        {
            state = state_aistats_last15m_ES;
            log_debug("Latest 15min\n");
        }
        break;
    case state_aistats_last15m_ES:
        if (parser_find(stream, size, "ES"))
        {
            if (eVMG1312 == local_data.type)
            {
                long int down, up;
                if (2 == sscanf(stream, " ES: %ld %ld", &down, &up))
                {
                    parser_data.statistics.failure.error_secs_15min = down + up;
                    parser_data.statistics.failure.valid++;

                    log_debug("last 15min error seconds %ld\n",
                            parser_data.statistics.failure.error_secs_15min);
                }
            }
            else
            {
                long int ES;
                if (1 == sscanf(stream, " ES = %ld", &ES))
                {
                    parser_data.statistics.failure.error_secs_15min = ES;
                    parser_data.statistics.failure.valid++;

                    log_debug("last 15min error seconds %ld\n", parser_data.statistics.failure.error_secs_15min);
                }
            }
            if (eVMG1312 == local_data.type)
            {
                state = state_aistats_lastday;
            }
            else
            {
                state = state_aistats_exit;
            }
        }
        break;
    }

    return state;
}

#if defined (CFG_COMMAND)
enum
{
    state_aivendor_start,
    state_aivendor_chipset_id,
    state_aivendor_exit = -1
};

/*!
 * @fn          static int parse_adsl_info_vendor (int state, char* stream, size_t size)
 * @brief       parse vendor information
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_adsl_info_vendor (int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_aivendor_start:
        if (parser_find(stream, size, "adsl info --vendor"))
        {
            log_debug("adsl info --vendor\n");
            state = state_aivendor_chipset_id;
        }
        break;
    case state_aivendor_chipset_id:
        if (parser_find(stream, size, "ChipSet Vendor Id"))
        {
            char str[SCANF_BUFFER];
            if(1 == sscanf(stream, "ChipSet Vendor Id: "STR(SCANF_BUFFER,s)":", str))
            {
                memcpy(parser_data.atm.atu_c.vendor, str, sizeof(parser_data.atm.atu_c.vendor));
                parser_data.atm.atu_c.vendspec[0] = 0;
                parser_data.atm.atu_c.vendspec[1] = 0;
                parser_data.atm.atu_c.revision = 0;
                parser_data.atm.atu_c.valid = 3;
                log_debug("ATU-C vendor: %4s\n", parser_data.atm.atu_c.vendor);
            }
            state = state_aivendor_exit;
        }
        break;
    }
    return state;
}

enum
{
    state_avshow_start,
    state_avshow_vcp_vci,
    state_avshow_exit = -1
};

/*!
 * @fn          static int parse_atm_vcc_show (int state, char* stream, size_t size)
 * @brief       parse atm vcc data
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_atm_vcc_show (int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_aivendor_start:
        if (parser_find(stream, size, "atm vcc --show"))
        {
            log_debug("atm vcc --show\n");
            state = state_avshow_vcp_vci;
        }
        break;
    case state_avshow_vcp_vci:
        {
            int port, vpi, vci;
            if (3 == sscanf(stream, "%d.%d.%d", &port, &vpi, &vci))
            {
                parser_data.atm.pvc.vpi = vpi;
                parser_data.atm.pvc.vci = vci;
                parser_data.atm.pvc.valid = 2;

                log_debug("VPI %d VCI %d\n", parser_data.atm.pvc.vpi, parser_data.atm.pvc.vci);
                state = state_avshow_exit;
            }
            break;
        }
    }

    return state;
}

enum
{
    state_aibits_start,
    state_aibits_tone,
    state_aibits_value,
    state_aibits_exit = -1
};

/*!
 * @fn          static int parse_adsl_info_bits (int state, char* stream, size_t size)
 * @brief       parse bit allocation data
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_adsl_info_bits (int state, char* stream, size_t size)
{
    int tone;
    int value;

    switch (state)
    {
    case state_aibits_start:
        if (parser_find(stream, size, "adsl info --Bits"))
        {
            log_debug("adsl info --Bits\n");
            state = state_aibits_tone;
        }
        break;
    case state_aibits_tone:
        if (parser_find(stream, size, "Tone number"))
        {
            parser_data.tone.bitalloc_up.valid = 1;
            parser_data.tone.bitalloc_down.valid = 1;

            log_debug("Tone number\n");
            state = state_aibits_value;
        }
        break;
    case state_aibits_value:
        if(2 == sscanf(stream, " %d %d", &tone, &value))
        {
            parser_data.tone.bitalloc_down.data[tone] = value;
        }
        else
        {
            state = state_aibits_exit;
        }
        break;
    }

    return state;
}

/*!
 * @fn          static void finish_adsl_info_bits (void)
 * @brief       finish bit allocation table
 */
static void finish_adsl_info_bits (void)
{
    int band;
    int i;

    for (band = 0; band < parser_data.bandplan->num_band; band++)
    {
        for (i = parser_data.bandplan->band[band].up_min;
                i < parser_data.bandplan->band[band].up_max; i++)
        {
            parser_data.tone.bitalloc_up.data[i] = parser_data.tone.bitalloc_down.data[i];
            parser_data.tone.bitalloc_down.data[i] = 0;
        }
    }
}

enum
{
    state_aisnr_start,
    state_aisnr_tone,
    state_aisnr_value,
    state_aisnr_exit = -1
};

/*!
 * @fn          static int parse_adsl_info_snr (int state, char* stream, size_t size)
 * @brief       parse snr data
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_adsl_info_snr (int state, char* stream, size_t size)
{
    int tone;
    double value;

    switch (state)
    {
    case state_aisnr_start:
        if (parser_find(stream, size, "adsl info --SNR"))
        {
            log_debug("adsl info --SNR\n");
            state = state_aisnr_tone;
        }
        break;
    case state_aisnr_tone:
        if (parser_find(stream, size, "Tone number"))
        {
            parser_data.tone.snr.valid = 1;

            log_debug("Tone number\n");
            state = state_aisnr_value;
        }
        break;
    case state_aisnr_value:
        if(2 == sscanf(stream, " %d %lf", &tone, &value))
        {
            parser_data.tone.snr.data[tone] = (int)floor(value + 0.5);
        }
        else
        {
            state = state_aisnr_exit;
        }
        break;
    }

    return state;
}

#endif /* defined (CFG_COMMAND) */

/* _oOo_ */
