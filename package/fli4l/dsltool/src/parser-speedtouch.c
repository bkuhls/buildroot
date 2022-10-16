/*!
 * @file    parser-speedtouch.c
 * @brief   implementation file for speedtouch parser
 * @details parser for Thomson Speedtouch based modems
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
#include <stdlib.h>
#include <string.h>

#include "telnet.h"
#include "parser.h"
#include "log.h"

/*!
 * @struct      speedtouch_data_t
 * @brief       speedtouch private temporary data
 */
typedef struct {
    struct {
        int major;                  /*!< modem FW major version             */
        int minor;                  /*!< modem FW minor version             */
        int sub;                    /*!< modem FW sub version               */
        int subsub;                 /*!< modem FW subsub version            */
    } version;                      /*!< modem FW version                   */
    char xDSL_type[20];             /*!< xDSL type string                   */
} speedtouch_data_t;

/*!
 * @var         static speedtouch_data_t local_data;
 * @brief       speedtouch private temporary data
 */
static speedtouch_data_t local_data;

static int parse_env(int state, char* stream, size_t size);
static int parse_adsl_info(int state, char* stream, size_t size);
#if defined (CFG_COMMAND)
static int parse_tdsl_data (int state, char* stream, size_t size);
static int parse_adsl_bits(int state, char* stream, size_t size);
#endif /* defined (CFG_COMMAND) */

/*!
 * @defgroup    command_speedtouch speedtouch telnet shell commands
 * @{
 */
static const char* str_cmd_dummy = "?\r\n";
/*!< placeholder */
static const char* str_cmd_env_list = "env list\r\n";
/*!< get firmware version */
static const char* str_cmd_5x_adsl_info = "adsl info\r\n";
/*!< get adsl info */
static const char* str_cmd_6x_adsl_info = "adsl info expand=1\r\n";
/*!< get adsl info */
#if defined (CFG_COMMAND)
static const char* str_cmd_tdsl_data = "debug exec cmd='tdsl getData all'\r\n";
/*!< get tone data*/
static const char* str_cmd_adsl_bits = "adsl debug bitloadinginfo\r\n";
/*!< get bitloading data*/
static const char* str_cmd_adsl_down =  "adsl config status down\r\n";
/*!< set adsl connection down */
static const char* str_cmd_adsl_up = "adsl config status up\r\n";
/*!< set adsl connection up */
static const char* str_cmd_reboot = "system reboot\r\n";
/*!< reboot modem*/
#endif /* defined (CFG_COMMAND) */
static const char* str_cmd_exit = "exit\r\n";
/*!< exit telnet */
/*!@}*/

void parser_init(int command, const char* username, const char* password)
{
    _parser_init(command);
    memset(&local_data, 0, sizeof(local_data));

    switch (parser_cfg.command)
    {
#if defined (CFG_COMMAND)
    case PARSER_INFO:
        parser_list[0].parser = parse_env;
        parser_list[0].cmd_str = str_cmd_env_list;
        parser_list[1].cmd_str = str_cmd_dummy;
        parser_list[2].cmd_str = str_cmd_dummy;
        parser_list[3].cmd_str = str_cmd_exit;
        break;
#endif /* defined (CFG_COMMAND) */
    case PARSER_STATUS:
        parser_list[0].parser = parse_env;
        parser_list[0].cmd_str = str_cmd_env_list;
        parser_list[1].cmd_str = str_cmd_dummy;
        parser_list[2].cmd_str = str_cmd_exit;
        break;
#if defined (CFG_COMMAND)
    case PARSER_RESYNC:
        parser_list[0].cmd_str = str_cmd_adsl_down;
        parser_list[1].cmd_str = str_cmd_adsl_up;
        parser_list[2].cmd_str = str_cmd_exit;
        break;
    case PARSER_REBOOT:
        parser_list[0].cmd_str = str_cmd_reboot;
        break;
#endif /* defined (CFG_COMMAND) */
    }

    parser_cfg.user = (char*) username;
    parser_cfg.pass = (char*) password;
    parser_cfg.enter = "\r\n";
    snprintf(parser_cfg.prompt, sizeof(parser_cfg.prompt), "{%s}=>", username);
}

enum
{
    state_env_start,
    state_env_build,
    state_env_atm_addr,
    state_env_intf1_pvc,
    state_env_exit = -1
};

/*!
 * @fn          int parse_env(int state, char* stream, size_t size)
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
int parse_env(int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_env_start:
        if (parser_find(stream, size, "env list"))
        {
            log_debug("env list\n");
            state = state_env_build;
        }
        break;
    case state_env_build:
        if (parser_find(stream, size, "_BUILD"))
        {
            int major, minor, sub, subsub;
            if(4 == sscanf(stream, " _BUILD=%d.%d.%d.%d", &major, &minor, &sub, &subsub))
            {
                local_data.version.major = major;
                local_data.version.minor = minor;
                local_data.version.sub = sub;
                local_data.version.subsub = subsub;

                log_debug("BUILD: %d.%d.%d.%d\n", major, minor, sub, subsub);
            }
            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                parser_list[1].parser = parse_adsl_info;
                parser_list[1].cmd_str = str_cmd_5x_adsl_info;

#if defined (CFG_COMMAND)
                if(PARSER_INFO == parser_cfg.command)
                {
                    parser_list[2].parser = parse_tdsl_data;
                    parser_list[2].cmd_str = str_cmd_tdsl_data;
                }
#endif /* defined (CFG_COMMAND) */

                state = state_env_atm_addr;
            }
            else
            {
                parser_list[1].parser = parse_adsl_info;
                parser_list[1].cmd_str = str_cmd_6x_adsl_info;

#if defined (CFG_COMMAND)
                if(PARSER_INFO == parser_cfg.command)
                {
                    parser_list[2].parser = parse_adsl_bits;
                    parser_list[2].cmd_str = str_cmd_adsl_bits;
                }
#endif /* defined (CFG_COMMAND) */

                state = state_env_intf1_pvc;
            }
        }
        break;
    case state_env_atm_addr:
        if (parser_find(stream, size, "ATM_addr"))
        {
            int vpi, vci;
            if(2 == sscanf(stream, " ATM_addr=%d.%d", &vpi, &vci))
            {
                parser_data.atm.pvc.vpi = vpi;
                parser_data.atm.pvc.vci = vci;
                parser_data.atm.pvc.valid = 2;

                log_debug("VPI/VCI:%d.%d\n", vpi, vci);
            }
            state = state_env_exit;
        }
        break;
    case state_env_intf1_pvc:
        if (parser_find(stream, size, "Intf1_PVC"))
        {
            int vpi, vci;
            if(2 == sscanf(stream, " Intf1_PVC=%d.%d", &vpi, &vci))
            {
                parser_data.atm.pvc.vpi = vpi;
                parser_data.atm.pvc.vci = vci;
                parser_data.atm.pvc.valid = 2;

                log_debug("VPI/VCI:%d.%d\n", vpi, vci);
            }
            state = state_env_exit;
        }
        break;
    }
    return state;
}

enum
{
    state_adsl_info_start,
    state_adsl_info_modemstatus,
    state_adsl_info_operationmode,
    state_adsl_info_xdsl_type,
    state_adsl_info_xdsl_standard,
    state_adsl_info_xdsl_annex,
    state_adsl_info_channelmode,
    state_adsl_info_vendor,
    state_adsl_info_vendor_vendor,
    state_adsl_info_vendor_spec,
    state_adsl_info_vendor_rev,
    state_adsl_info_noisemargin,
    state_adsl_info_attenuation,
    state_adsl_info_txpower,
    state_adsl_info_bandwith,
    state_adsl_info_bandwith_down,
    state_adsl_info_bandwith_up,
    state_adsl_info_statistics,
    state_adsl_info_statistics_errors,
    state_adsl_info_statistics_rx_FEC,
    state_adsl_info_statistics_rx_CRC,
    state_adsl_info_statistics_rx_HEC,
    state_adsl_info_statistics_tx_FEC,
    state_adsl_info_statistics_tx_CRC,
    state_adsl_info_statistics_tx_HEC,
    state_adsl_info_statistics_nef_15min,
    state_adsl_info_statistics_nef_15min_errsec,
    state_adsl_info_statistics_nef_day,
    state_adsl_info_statistics_nef_day_errsec,
    state_adsl_info_exit = -1
};

/*!
 * @fn          static int parse_adsl_info(int state, char* stream, size_t size)
 * @brief       parse adsl info
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_adsl_info(int state, char* stream, size_t size)
{
    int count = 0;

    switch (state)
    {
    case state_adsl_info_start:
        if (parser_find(stream, size, "adsl info"))
        {
            log_debug("adsl info\n");
            state = state_adsl_info_modemstatus;
        }
        break;
    case state_adsl_info_modemstatus:
        if (parser_find(stream, size, "Modemstate"))
        {
            char str[SCANF_BUFFER];
            if (1 == sscanf(stream, " Modemstate : "STR(SCANF_BUFFER,s), str))
            {
                strcpy(parser_data.modemstate.str, str);
                parser_data.modemstate.state =
                        (0 == strncmp(str, "up", 2)) ? 1 : 0;
                parser_data.modemstate.valid = 2;

                log_debug("modemstate: %s\n", parser_data.modemstate.str);
            }
            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                state = state_adsl_info_operationmode;
            }
            else
            {
                state = state_adsl_info_xdsl_type;
            }
        }
        break;
    case state_adsl_info_xdsl_type:
        if (parser_find(stream, size, "xDSL Type"))
        {
            char str[SCANF_BUFFER];
            if(1 == sscanf(stream, " xDSL Type: "STR(SCANF_BUFFER,s), str))
            {
                strcpy(local_data.xDSL_type, str);
                log_debug("xDSL Type %s\n", local_data.xDSL_type);
            }
            state = state_adsl_info_xdsl_standard;
        }
        break;
    case state_adsl_info_xdsl_standard:
        if (parser_find(stream, size, "xDSL Standard"))
        {
            char str[SCANF_BUFFER];
            if (1 == sscanf(stream, " xDSL Standard : ITU-T "STR(SCANF_BUFFER,s), str))
            {
                strcpy(parser_data.operationmode.str, str);
                parser_data.operationmode.valid = 1;

                log_debug("operationmode: %s\n", parser_data.operationmode.str);
            }
            state = state_adsl_info_xdsl_annex;
        }
        break;
    case state_adsl_info_xdsl_annex:
        if (parser_find(stream, size, "xDSL Annex"))
        {
            char str[SCANF_BUFFER];
            if (1 == sscanf(stream, " xDSL Annex : annex "STR(SCANF_BUFFER,s), str))
            {
                if (0 == strcmp(parser_data.operationmode.str, "G.992.5"))
                {
                    if (0 == strcmp(str, "A"))
                    {
                        parser_data.bandplan = &bandplan_ADSL2plus_AnnexA;
                    }
                    else if (0 == strcmp(str, "B"))
                    {
                        parser_data.bandplan = &bandplan_ADSL2plus_AnnexB;
                    }
                }
                else if(0 == strcmp(parser_data.operationmode.str, "G.992.3"))
                {
                    if (0 == strcmp(str, "A"))
                    {
                        parser_data.bandplan = &bandplan_ADSL2_AnnexA;
                    }
                    else if (0 == strcmp(str, "B"))
                    {
                        parser_data.bandplan = &bandplan_ADSL2_AnnexB;
                    }
                }
                else if(0 == strcmp(parser_data.operationmode.str, "G.992.1"))
                {
                    if (0 == strcmp(str, "A"))
                    {
                        parser_data.bandplan = &bandplan_ADSL_AnnexA;
                    }
                    else if (0 == strcmp(str, "B"))
                    {
                        parser_data.bandplan = &bandplan_ADSL_AnnexB;
                    }
                }

                strcat(parser_data.operationmode.str, " Annex ");
                strcat(parser_data.operationmode.str, str);
                parser_data.operationmode.valid = 1;

                log_debug("operationmode: %s\n", parser_data.operationmode.str);
            }
            if(0 == strcmp(local_data.xDSL_type, "ADSL"))
            {
                state = state_adsl_info_channelmode;
            }
            else
            {

                state = state_adsl_info_bandwith;
            }
        }
        break;
    case state_adsl_info_operationmode:
        if (parser_find(stream, size, "Operation Mode"))
        {
            char str1[SCANF_BUFFER];
            char str2[SCANF_BUFFER];
            if (2 == sscanf(stream, " Operation Mode : "STR(SCANF_BUFFER,s)" Annex "STR(SCANF_BUFFER,s), str1, str2))
            {
                strcpy(parser_data.operationmode.str, str1);
                strcat(parser_data.operationmode.str, " Annex ");
                strcat(parser_data.operationmode.str, str2);
                parser_data.operationmode.valid = 1;

                if (0 == strcmp(str1, "G.992.5"))
                {
                    if (0 == strcmp(str2, "A"))
                    {
                        parser_data.bandplan = &bandplan_ADSL2plus_AnnexA;
                    }
                    else if (0 == strcmp(str2, "B"))
                    {
                        parser_data.bandplan = &bandplan_ADSL2plus_AnnexB;
                    }
                }
                else if(0 == strcmp(str1, "G.992.3"))
                {
                    if (0 == strcmp(str2, "A"))
                    {
                        parser_data.bandplan = &bandplan_ADSL2_AnnexA;
                    }
                    else if (0 == strcmp(str2, "B"))
                    {
                        parser_data.bandplan = &bandplan_ADSL2_AnnexB;
                    }
                }
                else if(0 == strcmp(str1, "G.992.1"))
                {
                    if (0 == strcmp(str2, "A"))
                    {
                        parser_data.bandplan = &bandplan_ADSL_AnnexA;
                    }
                    else if (0 == strcmp(str2, "B"))
                    {
                        parser_data.bandplan = &bandplan_ADSL_AnnexB;
                    }
                }

                log_debug("operationmode: %s\n", parser_data.operationmode.str);
            }
            state = state_adsl_info_channelmode;
        }
        break;
    case state_adsl_info_channelmode:
        if (parser_find(stream, size, "Channel Mode"))
        {
            char str[SCANF_BUFFER];
            if (1 == sscanf(stream, " Channel Mode : "STR(SCANF_BUFFER,s), str))
            {
                strcpy(parser_data.channelmode.str, str);
                parser_data.channelmode.valid = 1;

                log_debug("channelmode: %s\n", parser_data.channelmode.str);
            }
            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                state = state_adsl_info_vendor;
            }
            else
            {
                state = state_adsl_info_bandwith;
            }
        }
        break;
    case state_adsl_info_vendor:
        if (parser_find(stream, size, "Vendor"))
        {
            state = state_adsl_info_vendor_vendor;
        }
        else if (parser_find(stream, size, "Transfer statistics"))
        {
            log_debug("Transfer statistics\n");
            state = state_adsl_info_statistics_errors;
        }
        break;
    case state_adsl_info_vendor_vendor:
        if (parser_find(stream, size, "Vendor"))
        {
            char local[SCANF_BUFFER], remote[SCANF_BUFFER];
            if (2 == sscanf(stream, " Vendor : "STR(SCANF_BUFFER,s)" "STR(SCANF_BUFFER,s), local, remote))
            {
                memcpy(parser_data.atm.atu_r.vendor, local, sizeof(parser_data.atm.atu_r.vendor));
                parser_data.atm.atu_r.valid++;

                memcpy(parser_data.atm.atu_c.vendor, remote, sizeof(parser_data.atm.atu_c.vendor));
                parser_data.atm.atu_c.valid++;

                log_debug("vendor: local %4s remote %4s\n", parser_data.atm.atu_r.vendor, parser_data.atm.atu_c.vendor);
            }
            state = state_adsl_info_vendor_spec;
        }
        break;
    case state_adsl_info_vendor_spec:
        if (parser_find(stream, size, "VendorSpecific"))
        {
            int local[2], remote[2];
            if (4 == sscanf(stream, " VendorSpecific : %02x%02x %02x%02x", &local[0], &local[1], &remote[0], &remote[1]))
            {
                parser_data.atm.atu_r.vendspec[0] = local[0];
                parser_data.atm.atu_r.vendspec[1] = local[1];
                parser_data.atm.atu_r.valid++;

                parser_data.atm.atu_c.vendspec[0] = remote[0];
                parser_data.atm.atu_c.vendspec[1] = remote[1];
                parser_data.atm.atu_c.valid++;

                log_debug("vendorspec: local %d.%d remote %d.%d\n",
                        parser_data.atm.atu_r.vendspec[0], parser_data.atm.atu_r.vendspec[1],
                parser_data.atm.atu_c.vendspec[0], parser_data.atm.atu_c.vendspec[1]);
            }

            state = state_adsl_info_vendor_rev;
        }
        break;
    case state_adsl_info_vendor_rev:
        if (parser_find(stream, size, "StandardRevisionNr"))
        {
            int local, remote;
            if (2 == sscanf(stream, " StandardRevisionNr : %02x %02x", &local, &remote))
            {
                parser_data.atm.atu_r.revision = local;
                parser_data.atm.atu_r.valid++;

                parser_data.atm.atu_c.revision = remote;
                parser_data.atm.atu_c.valid++;

                log_debug("revision: local %d remote %d\n",
                        parser_data.atm.atu_r.revision, parser_data.atm.atu_c.revision);
            }

            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                state = state_adsl_info_noisemargin;
            }
            else
            {
                state = state_adsl_info_statistics;
            }
        }
        break;
    case state_adsl_info_noisemargin:
        if (parser_find(stream, size, "Margin"))
        {
            double down, up;

            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                count = sscanf(stream, " Margin [dB] : %lf %lf", &down, &up);
            }
            else
            {
                count = sscanf(stream, " Margin (dB) : %lf %lf", &down, &up);
            }

            if (2 == count)
            {
                parser_data.noisemargin[0].down = down;
                parser_data.noisemargin[0].up = up;
                parser_data.noisemargin[0].valid = 2;

                log_debug("noisemargin: down %f up %f\n", down, up);
            }
            state = state_adsl_info_attenuation;
        }
        break;
    case state_adsl_info_attenuation:
        if (parser_find(stream, size, "Attenuation"))
        {
            double down, up;

            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                count = sscanf(stream, " Attenuation [dB] : %lf %lf", &down, &up);
            }
            else
            {
                count = sscanf(stream, " Attenuation (dB) : %lf %lf", &down, &up);
            }

            if (2 == count)
            {
                parser_data.attenuation[0].down = down;
                parser_data.attenuation[0].up = up;
                parser_data.attenuation[0].valid = 2;

                log_debug("attenuation: down %f up %f\n", down, up);
            }
            state = state_adsl_info_txpower;
        }
        break;
    case state_adsl_info_txpower:
        if (parser_find(stream, size, "OutputPower"))
        {
            double down, up;

            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                count = sscanf(stream, " OutputPower [dBm] : %lf %lf", &down, &up);
            }
            else
            {
                count = sscanf(stream, " OutputPower (dBm) : %lf %lf", &down, &up);
            }

            if (2 == count)
            {
                parser_data.txpower.down = down;
                parser_data.txpower.up = up;
                parser_data.txpower.valid = 2;

                log_debug("txpower: down %f up %f\n", down, up);
            }

            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                state = state_adsl_info_bandwith;
            }
            else
            {
                state = state_adsl_info_vendor;
            }
        }
        break;
    case state_adsl_info_bandwith:
        if (parser_find(stream, size, "Available Bandwidth"))
        {
            log_debug("Available Bandwidth\n");
            state = state_adsl_info_bandwith_down;
        }
        break;
    case state_adsl_info_bandwith_down:
        if (parser_find(stream, size, "Downstream"))
        {
            int cells, kbit;
            if (2 == sscanf(stream, " Downstream : %d %d", &cells, &kbit))
            {
                parser_data.bandwidth.cells.down = cells;
                parser_data.bandwidth.kbit.down = kbit;
                parser_data.bandwidth.cells.valid++;
                parser_data.bandwidth.kbit.valid++;

                log_debug("bandwidth down cells %d kbit %d\n", cells, kbit);
            }
            state = state_adsl_info_bandwith_up;
        }
        break;
    case state_adsl_info_bandwith_up:
        if (parser_find(stream, size, "Upstream"))
        {
            int cells, kbit;
            if (2 == sscanf(stream, " Upstream : %d %d", &cells, &kbit))
            {
                parser_data.bandwidth.cells.up = cells;
                parser_data.bandwidth.kbit.up = kbit;
                parser_data.bandwidth.cells.valid++;
                parser_data.bandwidth.kbit.valid++;

                log_debug("bandwidth up cells %d kbit %d\n", cells, kbit);
            }
            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                state = state_adsl_info_statistics;
            }
            else
            {
                state = state_adsl_info_noisemargin;
            }
        }
        break;
    case state_adsl_info_statistics:
        if (parser_find(stream, size, "Transfer statistics"))
        {
            log_debug("Transfer statistics\n");
            state = state_adsl_info_statistics_errors;
        }
        break;
    case state_adsl_info_statistics_errors:
        if (parser_find(stream, size, "Errors"))
        {
            log_debug("Errors\n");
            state = state_adsl_info_statistics_rx_FEC;
        }
        break;
    case state_adsl_info_statistics_rx_FEC:
        if (parser_find(stream, size, "Received FEC"))
        {
            long error;
            if (1 == sscanf(stream, " Received FEC : %ld", &error))
            {
                parser_data.statistics.error.FEC.Rx = error;
                parser_data.statistics.error.FEC.valid++;

                log_debug("FEC rx %ld\n", error);
            }
            state = state_adsl_info_statistics_rx_CRC;
        }
        break;
    case state_adsl_info_statistics_rx_CRC:
        if (parser_find(stream, size, "Received CRC"))
        {
            long error;
            if (1 == sscanf(stream, " Received CRC : %ld", &error))
            {
                parser_data.statistics.error.CRC.Rx = error;
                parser_data.statistics.error.CRC.valid++;

                log_debug("CRC rx %ld\n", error);
            }
            state = state_adsl_info_statistics_rx_HEC;
        }
        break;
    case state_adsl_info_statistics_rx_HEC:
        if (parser_find(stream, size, "Received HEC"))
        {
            long error;
            if (1 == sscanf(stream, " Received HEC : %ld", &error))
            {
                parser_data.statistics.error.HEC.Rx = error;
                parser_data.statistics.error.HEC.valid++;

                log_debug("HEC rx %ld\n", error);
            }
            state = state_adsl_info_statistics_tx_FEC;
        }
        break;
    case state_adsl_info_statistics_tx_FEC:
        if (parser_find(stream, size, "Transmitted FEC"))
        {
            long error;
            if (1 == sscanf(stream, " Transmitted FEC : %ld", &error))
            {
                parser_data.statistics.error.FEC.Tx = error;
                parser_data.statistics.error.FEC.valid++;

                log_debug("FEC tx %ld\n", error);
            }
            state = state_adsl_info_statistics_tx_CRC;
        }
        break;
    case state_adsl_info_statistics_tx_CRC:
        if (parser_find(stream, size, "Transmitted CRC"))
        {
            long error;
            if (1 == sscanf(stream, " Transmitted CRC : %ld", &error))
            {
                parser_data.statistics.error.CRC.Tx = error;
                parser_data.statistics.error.CRC.valid++;

                log_debug("CRC tx %ld\n", error);
            }
            state = state_adsl_info_statistics_tx_HEC;
        }
        break;
    case state_adsl_info_statistics_tx_HEC:
        if (parser_find(stream, size, "Transmitted HEC") ||
            parser_find(stream, size, "Tranmsitted HEC"))
        {
            long error;

            if ((5 == local_data.version.major) ||
               ((6 == local_data.version.major) && (2 > local_data.version.minor)))
            {
                count = sscanf(stream, " Transmitted HEC : %ld", &error);
            }
            else
            {
                count = sscanf(stream, " Tranmsitted HEC : %ld", &error);
            }
            if (1 == count)
            {
                parser_data.statistics.error.HEC.Tx = error;
                parser_data.statistics.error.HEC.valid++;

                log_debug("HEC tx %ld\n", error);
            }
            state = state_adsl_info_statistics_nef_15min;
        }
        break;
    case state_adsl_info_statistics_nef_15min:
        if (parser_find(stream, size, "Near end failures last 15 minutes"))
        {
            log_debug("Near end failures last 15 minutes\n");
            state = state_adsl_info_statistics_nef_15min_errsec;
        }
        break;
    case state_adsl_info_statistics_nef_15min_errsec:
        if (parser_find(stream, size, "Errored seconds"))
        {
            int error;
            if (1 == sscanf(stream, " Errored seconds : %d seconds", &error))
            {
                parser_data.statistics.failure.error_secs_15min = error;
                parser_data.statistics.failure.valid++;

                log_debug("errorsec 15min: %ld\n", error);
            }
            state = state_adsl_info_statistics_nef_day;
        }
        break;
    case state_adsl_info_statistics_nef_day:
        if (parser_find(stream, size, "Near end failures current day"))
        {
            log_debug("Near end failures current day\n");
            state = state_adsl_info_statistics_nef_day_errsec;
        }
        break;
    case state_adsl_info_statistics_nef_day_errsec:
        if (parser_find(stream, size, "Errored seconds"))
        {
            int error;
            if (1 == sscanf(stream, " Errored seconds : %d seconds", &error))
            {
                parser_data.statistics.failure.error_secs_day = error;
                parser_data.statistics.failure.valid++;

                log_debug("errorsec day: %ld\n", error);
            }
            state = state_adsl_info_exit;
        }
        break;
    }

    return state;
}

#if defined (CFG_COMMAND)
enum
{
    state_tdsl_data_start,
    state_tdsl_data_bitalloc,
    state_tdsl_data_bitalloc_tone,
    state_tdsl_data_snr,
    state_tdsl_data_snr_tone,
    state_tdsl_data_gainQ2,
    state_tdsl_data_gainQ2_tone,
    state_tdsl_data_noisemargin,
    state_tdsl_data_noisemargin_tone,
    state_tdsl_data_chancharlog,
    state_tdsl_data_chancharlog_tone,
    state_tdsl_data_exit  = -1
};

/*!
 * @fn          static int parse_tone (const char* stream, const char* prefix,
 *                  int* tone_data)
 * @brief       parse tone data
 * @param[in]   stream
 *              data line to parse
 * @param[in]   prefix
 *              line prefix
 * @param[in,out]   tone_data
 *              tone data
 * @return      number of params parsed
 */
static int parse_tone (const char* stream, const char* prefix, int* tone_data)
{
    int tonestart;
    int tone[10];
    char format[80];
    int count = 0;

    snprintf(format, sizeof(format), "%s %%d %%d %%d %%d %%d %%d %%d %%d %%d %%d", prefix);

    count = sscanf (stream, format,
            &tonestart, &tone[0], &tone[1], &tone[2], &tone[3], &tone[4],
            &tone[5], &tone[6], &tone[7], &tone[8], &tone[9]);

    if ((count > 1) && (count < 12))
    {
        int i;
        int max = count - 1;
        for (i=0; i<max; i++)
        {
            if ((tonestart + i) < parser_data.bandplan->num_tone)
            {
                tone_data[tonestart + i] = abs (tone[i]);
            }
            else
            {
                break;
            }
        }
    }

    return count;
}

/*!
 * @fn          static void finish_bitalloc (void)
 * @brief       finish bit allocation table
 */
static void finish_bitalloc (void)
{
    int i;
    int flag = 0;

    for (i = 0; i < 64; i++)
    {
        if(0 == parser_data.tone.bitalloc_down.data[i])
        {
            if (flag && ( 0 == parser_data.tone.bitalloc_down.data[i+1]))
            {
                break;
            }
        }
        else
        {
            parser_data.tone.bitalloc_up.data[i] = parser_data.tone.bitalloc_down.data[i];
            parser_data.tone.bitalloc_down.data[i] = 0;
            flag = 1;
        }
    }

    parser_data.tone.bitalloc_up.valid = 1;
    parser_data.tone.bitalloc_down.valid = 1;
}

/*!
 * @fn          static int parse_tdsl_data (int state, char* stream, size_t size)
 * @brief       parse tone data
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_tdsl_data (int state, char* stream, size_t size)
{
    switch (state)
    {
    case state_tdsl_data_start:
        if (parser_find (stream, size, "debug exec cmd='tdsl getData all'"))
        {
            log_debug("debug exec cmd='tdsl getData all'\n");
            state = state_tdsl_data_bitalloc;
        }
        break;
    case state_tdsl_data_bitalloc:
        if (parser_find (stream, size, "bitalloc start"))
        {
            log_debug("bitalloc start\n");
            state = state_tdsl_data_bitalloc_tone;
        }
        break;
    case state_tdsl_data_bitalloc_tone:
        if (parser_find (stream, size, "bitalloc end"))
        {
            finish_bitalloc();

            log_debug("bitalloc end\n");
            state = state_tdsl_data_snr;
        }
        else if (parser_find (stream, size, "tone"))
        {
            parse_tone (stream, "tone %d:", parser_data.tone.bitalloc_down.data);
        }
        break;
    case state_tdsl_data_snr:
        if (parser_find (stream, size, "snr start"))
        {
            log_debug("snr start\n");
            state = state_tdsl_data_snr_tone;
        }
        break;
    case state_tdsl_data_snr_tone:
        if (parser_find (stream, size, "snr end"))
        {
            parser_data.tone.snr.valid = 1;

            log_debug("snr end\n");
            state = state_tdsl_data_gainQ2;
        }
        else if (parser_find (stream, size, "tone"))
        {
            parse_tone (stream, "tone %d:", parser_data.tone.snr.data);
        }
        break;
    case state_tdsl_data_gainQ2:
        if (parser_find (stream, size, "gainQ2 start"))
        {
            log_debug("gainQ2 start\n");
            state = state_tdsl_data_gainQ2_tone;
        }
        break;
    case state_tdsl_data_gainQ2_tone:
        if (parser_find (stream, size, "gainQ2 end"))
        {
            parser_data.tone.gainQ2.valid = 1;

            log_debug("gainQ2 end\n");
            state = state_tdsl_data_noisemargin;
        }
        else if (parser_find (stream, size, "tone"))
        {
            parse_tone (stream, "tone %d:", parser_data.tone.gainQ2.data);
        }
        break;
    case state_tdsl_data_noisemargin:
        if (parser_find (stream, size, "noisemargin start"))
        {
            log_debug("noisemargin start\n");
            state = state_tdsl_data_noisemargin_tone;
        }
        break;
    case state_tdsl_data_noisemargin_tone:
        if (parser_find (stream, size, "noisemargin end"))
        {
            parser_data.tone.noisemargin.valid = 1;

            log_debug("noisemargin end\n");
            state = state_tdsl_data_chancharlog;
        }
        else if (parser_find (stream, size, "tone"))
        {
            parse_tone (stream, "tone %d:", parser_data.tone.noisemargin.data);
        }
        break;
    case state_tdsl_data_chancharlog:
        if (parser_find (stream, size, "chanCharLog start"))
        {
            log_debug("chanCharLog start\n");
            state = state_tdsl_data_chancharlog_tone;
        }
        break;
    case state_tdsl_data_chancharlog_tone:
        if (parser_find (stream, size, "chanCharLog end"))
        {
            parser_data.tone.chancharlog.valid = 1;

            log_debug("chanCharLog end\n");
            state = state_tdsl_data_exit;
        }
        else if (parser_find (stream, size, "tone"))
        {
            parse_tone (stream, "tone %d:", parser_data.tone.chancharlog.data);
        }
        break;
    }
    return state;
}

enum {
    state_adsl_bits_start,
    state_adsl_bits_tone,
    state_adsl_bits_bits,
    state_adsl_bits_exit  = -1
};

/*!
 * @fn          static int parse_adsl_bits(int state, char* stream, size_t size)
 * @brief       parse bit allocation table
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_adsl_bits(int state, char* stream, size_t size)
{
    int count;

    switch (state)
    {
    case state_adsl_info_start:
        if (parser_find(stream, size, "adsl debug bitloadinginfo"))
        {
            log_debug("adsl debug bitloadinginfo\n");
            state = state_adsl_bits_tone;
        }
        break;
    case state_adsl_bits_tone:
        if (parser_find(stream, size, "Tone"))
        {
            log_debug("Tone\n");
            state = state_adsl_bits_bits;
        }
        break;
    case state_adsl_bits_bits:
        if(1 == sscanf(stream, " %d:", &count))
        {
            count = parse_tone (stream, "%d :", parser_data.tone.bitalloc_down.data);
            if (count < 11)
            {
                finish_bitalloc();

                log_debug("bitalloc end\n");
                state = state_adsl_bits_exit;
            }
        }
        break;
    }
    return state;
}

#endif /* defined (CFG_COMMAND) */

// _oOo_
