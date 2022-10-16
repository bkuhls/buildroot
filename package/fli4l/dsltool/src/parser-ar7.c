/*!
 * @file    parser-ar7.c
 * @brief   implementation file for AR7 parser
 * @details parser for TI AR7 based modems
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
 * @struct      num_str_t
 * @brief       map number to string
 */
typedef struct
{
  int num;                          /*!< number                             */
  const char* str;                  /*!< string                             */
} num_str_t;

/*!
 * @var         channel_modes
 * @brief       map number to channel mode strings
 */
static num_str_t channel_modes[] = {
        {2, "G.dmt / G.992.1"},
        {4, "G.lite / G.992.2"},
        {8, "ADSL2 / G.dmt.bis / G.992.3"},
        {9, "RE-ADSL2 / G.dmt.bis DELT / G.992.3 Annex L"},
        {16, "ADSL2+ G.992.5"},
        {17, "ADSL2+ DELT / G.992.5 Annex-L"},
        {32, "RE-ADSL"},
        {33, "RE-ADSL DELT"},
        {128, "T1.413 (ADSL ANSI T1.413)"}
};

/*!
 * @var         path_types
 * @brief       map number to path type strings
 */
static num_str_t path_types[] = {
        {0, "Fast Path"},
        {1, "Interleaved Path"}
};

static int parse_avsar_modem_stats(int state, char* stream, size_t size);
#if defined (CFG_COMMAND)
static int parse_avsar_ver(int state, char* stream, size_t size);
static int parse_avsar_bit_allocation_table (int state, char* stream, size_t size);
static int parse_avsar_rxsnr (int state, char* stream, size_t size);
#endif /* defined (CFG_COMMAND) */

/*!
 * @defgroup    command_ar7 AR7 telnet shell commands
 * @{
 */
static const char* str_cmd_avsar_modem_stats =  "cat /proc/avalanche/avsar_modem_stats\r\n";
/*!< get modem statistics */
#if defined (CFG_COMMAND)
static const char* str_cmd_avsar_ver = "cat /proc/avalanche/avsar_ver\r\n";
/*!< get modem version */
static const char* str_cmd_avsar_bit_allocation_table = "cat /proc/avalanche/avsar_bit_allocation_table\r\n";
/*!< get bit allocation */
static const char* str_cmd_avsar_rxsnr = "cat /proc/avalanche/avsar_rxsnr0\r\n";
/*!< get snr data */
static const char* str_cmd_reboot = "reboot\r\n";
/*!< reboot modem */
#endif /* defined (CFG_COMMAND) */
static const char* str_cmd_exit = "exit\r\n";
/*!< exit telnet */
/*!@}*/

void parser_init(int command, const char* username, const char* password)
{
    int index = 0;

    _parser_init(command);

    switch (command)
    {
#if defined (CFG_COMMAND)
    case PARSER_INFO:
        parser_list[index].parser = parse_avsar_ver;
        parser_list[index].cmd_str = str_cmd_avsar_ver;
        index++;
        parser_list[index].parser = parse_avsar_modem_stats;
        parser_list[index].cmd_str = str_cmd_avsar_modem_stats;
        index++;
        parser_list[index].parser = parse_avsar_bit_allocation_table;
        parser_list[index].cmd_str = str_cmd_avsar_bit_allocation_table;
        index++;
        parser_list[index].parser = parse_avsar_rxsnr;
        parser_list[index].cmd_str = str_cmd_avsar_rxsnr;
        index++;
        parser_list[index].cmd_str = str_cmd_exit;
        index++;
        break;
#endif /* defined (CFG_COMMAND) */
    case PARSER_STATUS:
        parser_list[index].parser = parse_avsar_modem_stats;
        parser_list[index].cmd_str = str_cmd_avsar_modem_stats;
        index++;
        parser_list[index].cmd_str = str_cmd_exit;
        index++;
        break;
#if defined (CFG_COMMAND)
    case PARSER_RESYNC:
        parser_list[index].cmd_str = str_cmd_exit;
        index++;
        break;
    case PARSER_REBOOT:
        parser_list[index].cmd_str = str_cmd_reboot;
        index++;
        break;
#endif /* defined (CFG_COMMAND) */
    }

    parser_cfg.user = (char*) username;
    parser_cfg.pass = (char*) password;
    parser_cfg.enter = "\r\n";
    snprintf(parser_cfg.prompt, sizeof(parser_cfg.prompt), "# ");
}


#if defined (CFG_COMMAND)
/*!
 * @fn          static int parse_avsar_ver(int state, char* stream, size_t size)
 * @brief       parse version
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_avsar_ver(int state, char* stream, size_t size)
{
    return -1;
}
#endif /* defined (CFG_COMMAND) */

enum
{
    state_avms_start,
    state_avms_dsl_stats,
    state_avms_dsl_rate,
    state_avms_dsl_ds_line,
    state_avms_dsl_us_line,
    state_avms_dsl_txpower,
    state_avms_dsl_path,
    state_avms_dsl_mode,
    state_avms_dsl_annex,
    state_avms_dsl_atuc,
    state_avms_dsl_atur,
    state_avms_dsl_tx_interleave,
    state_avms_dsl_tx_interleave_CRC_FEC,
    state_avms_dsl_tx_interleave_HEC,
    state_avms_dsl_rx_interleave,
    state_avms_dsl_rx_interleave_CRC_FEC,
    state_avms_dsl_rx_interleave_HEC,
    state_avms_dsl_tx_fastpath,
    state_avms_dsl_tx_fastpath_CRC_FEC,
    state_avms_dsl_tx_fastpath_HEC,
    state_avms_dsl_rx_fastpath,
    state_avms_dsl_rx_fastpath_CRC_FEC,
    state_avms_dsl_rx_fastpath_HEC,
    state_avms_exit = -1
};

/*!
 * @fn          static int parse_avsar_modem_stats(int state, char* stream, size_t size)
 * @brief       parse modem statistics
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_avsar_modem_stats(int state, char* stream, size_t size)
{
    static int channel_mode = 0;
    switch (state)
    {
    case state_avms_start:
        if (parser_find(stream, size, "AR7 DSL Modem Statistics"))
        {
            log_debug("AR7 DSL Modem Statistics\n");
            state = state_avms_dsl_stats;
        }
        break;
    case state_avms_dsl_stats:
        if (parser_find(stream, size, "[DSL Modem Stats]"))
        {
            log_debug("[DSL Modem Stats]\n");
            state = state_avms_dsl_rate;
        }
        break;
    case state_avms_dsl_rate:
        if (parser_find(stream, size, "US Connection Rate"))
        {
            int up, down;
            if( 2 == sscanf(stream, " US Connection Rate: %d DS Connection Rate: %d", &up, &down))
            {
                parser_data.bandwidth.kbit.up = up;
                parser_data.bandwidth.kbit.down = down;
                parser_data.bandwidth.kbit.valid = 2;
                if((0 == up) || (0 == down))
                {
                    strcpy(parser_data.modemstate.str, "down");
                    parser_data.modemstate.state = 0;
                }
                else
                {
                    strcpy(parser_data.modemstate.str, "up");
                    parser_data.modemstate.state = 1;
                }
                parser_data.modemstate.valid = 2;

                log_debug("bandwidth up: %d down: %d\n",
                        parser_data.bandwidth.kbit.up,
                        parser_data.bandwidth.kbit.down);

                log_debug("modemstate: %s\n", parser_data.modemstate.str);
            }
            state = state_avms_dsl_ds_line;
        }
        break;
    case state_avms_dsl_ds_line:
        if(parser_find(stream, size, "DS Line Attenuation"))
        {
            double attenuation, noisemargin;
            if( 2 == sscanf(stream, " DS Line Attenuation: %lf DS Margin: %lf", &attenuation, &noisemargin))
            {
                parser_data.attenuation[0].down = attenuation;
                parser_data.attenuation[0].valid++;
                parser_data.noisemargin[0].down = noisemargin;
                parser_data.noisemargin[0].valid++;

                log_debug("down attenuation: %f margin %f\n", attenuation, noisemargin);
            }
            state = state_avms_dsl_us_line;
        }
        break;
    case state_avms_dsl_us_line:
        if(parser_find(stream, size, "US Line Attenuation"))
        {
            double attenuation, noisemargin;
            if( 2 == sscanf(stream, " US Line Attenuation: %lf US Margin: %lf", &attenuation, &noisemargin))
            {
                parser_data.attenuation[0].up = attenuation;
                parser_data.attenuation[0].valid++;
                parser_data.noisemargin[0].up = noisemargin;
                parser_data.noisemargin[0].valid++;

                log_debug("up attenuation: %f margin %f\n", attenuation, noisemargin);
            }
            state = state_avms_dsl_txpower;
        }
        break;
    case state_avms_dsl_txpower:
        if(parser_find(stream, size, "US Transmit Power"))
        {
            double up, down;
            if( 2 == sscanf(stream, " US Transmit Power : %lf DS Transmit Power: %lf", &up, &down))
            {
                parser_data.txpower.up = up;
                parser_data.txpower.down = down;
                parser_data.txpower.valid = 2;

                log_debug("txpower: down %f up %f\n", down, up);
            }
            state = state_avms_dsl_path;
        }
        break;
    case state_avms_dsl_path:
        if(parser_find(stream, size, "Trained Path"))
        {
            int path;
            if( 1 == sscanf(stream, " Trained Path: %d US Peak Cell Rate: %*d", &path))
            {
                int i;
                for(i = 0; i< (sizeof(path_types)/sizeof(path_types[0])); i++)
                {
                    if(path_types[i].num == path)
                    {
                        strcpy(parser_data.channelmode.str, path_types[i].str);
                        parser_data.channelmode.valid = 1;

                        log_debug("channelmode: %s\n", parser_data.channelmode.str);
                        break;
                    }
                }

            }
            state = state_avms_dsl_mode;
        }
        break;
    case state_avms_dsl_mode:
        if(parser_find(stream, size, "Trained Mode"))
        {
            int mode;
            if( 1 == sscanf(stream, " Trained Mode: %d Selected Mode: %*d", &mode))
            {
                int i;
                for(i = 0; i< (sizeof(channel_modes)/sizeof(channel_modes[0])); i++)
                {
                    if(channel_modes[i].num == mode)
                    {
                        channel_mode = channel_modes[i].num;
                        strcpy(parser_data.operationmode.str, channel_modes[i].str);
                        parser_data.operationmode.valid = 1;

                        log_debug("operationmode: %s\n", parser_data.operationmode.str);
                        break;
                    }
                }

            }
            state = state_avms_dsl_annex;
        }
        break;
    case state_avms_dsl_annex:
        if(parser_find(stream, size, "Annex"))
        {
            char str[SCANF_BUFFER];
            int psd;
            if( 2 == sscanf(stream, " Annex: "STR(SCANF_BUFFER,s)" psd_mask_qualifier: 0x%04x", str, &psd))
            {
                if(1 == parser_data.operationmode.valid)
                {
                    if(0 == strcmp(str, "AnxA"))
                    {
                        strcat(parser_data.operationmode.str, " Annex A");

                        if(16 == channel_mode)
                        {
                            parser_data.bandplan = &bandplan_ADSL2plus_AnnexA;
                        }
                        else
                        {
                            parser_data.bandplan = &bandplan_ADSL2_AnnexA;
                        }
                    }
                    else if(0 == strcmp(str, "AnxB"))
                    {
                        strcat(parser_data.operationmode.str, " Annex B");

                        if(16 == channel_mode)
                        {
                            parser_data.bandplan = &bandplan_ADSL2plus_AnnexB;
                        }
                        else
                        {
                            parser_data.bandplan = &bandplan_ADSL2_AnnexB;
                        }
                    }
                    log_debug("operationmode: %s\n", parser_data.operationmode.str);
                }
            }
            state = state_avms_dsl_atuc;
        }
        break;
    case state_avms_dsl_atuc:
        if(parser_find(stream, size, "ATUC ghsVid"))
        {
            int dat[8];
            if( 8 == sscanf(stream, " ATUC ghsVid: %02x %02x %02x %02x %02x %02x %02x %02x",
                    &dat[0], &dat[1], &dat[2], &dat[3],
                    &dat[4], &dat[5], &dat[6], &dat[7]))
            {
                parser_data.atm.atu_c.vendor[0] = dat[2];
                parser_data.atm.atu_c.vendor[1] = dat[3];
                parser_data.atm.atu_c.vendor[2] = dat[4];
                parser_data.atm.atu_c.vendor[3] = dat[5];
                parser_data.atm.atu_c.vendspec[0] = dat[6];
                parser_data.atm.atu_c.vendspec[1] = dat[7];
                parser_data.atm.atu_c.revision = 0;
                parser_data.atm.atu_c.valid += 3;

                log_debug("ATU-C vendor %4s spec %d.%d\n",
                        parser_data.atm.atu_c.vendor,
                        parser_data.atm.atu_c.vendspec[0],
                        parser_data.atm.atu_c.vendspec[1]);
            }
            state = state_avms_dsl_atur;
        }
        break;
    case state_avms_dsl_atur:
        if(parser_find(stream, size, "ATUR ghsVid"))
        {
            int dat[8];
            if( 8 == sscanf(stream, " ATUR ghsVid: %02x %02x %02x %02x %02x %02x %02x %02x",
                    &dat[0], &dat[1], &dat[2], &dat[3],
                    &dat[4], &dat[5], &dat[6], &dat[7]))
            {
                parser_data.atm.atu_r.vendor[0] = dat[2];
                parser_data.atm.atu_r.vendor[1] = dat[3];
                parser_data.atm.atu_r.vendor[2] = dat[4];
                parser_data.atm.atu_r.vendor[3] = dat[5];
                parser_data.atm.atu_r.vendspec[0] = dat[6];
                parser_data.atm.atu_r.vendspec[1] = dat[7];
                parser_data.atm.atu_r.revision = 0;
                parser_data.atm.atu_r.valid += 3;

                log_debug("ATU-R vendor %4s spec %d.%d\n",
                        parser_data.atm.atu_r.vendor,
                        parser_data.atm.atu_r.vendspec[0],
                        parser_data.atm.atu_r.vendspec[1]);
            }
            state = state_avms_dsl_tx_interleave;
        }
        break;
    case state_avms_dsl_tx_interleave:
        if (parser_find(stream, size, "[Upstream (TX) Interleave path]"))
        {
            log_debug("[Upstream (TX) Interleave path]\n");
            state = state_avms_dsl_tx_interleave_CRC_FEC;
        }
        break;
    case state_avms_dsl_tx_interleave_CRC_FEC:
        if(parser_find(stream, size, "CRC"))
        {
            long int CRC, FEC, NCD;
            if( 3 == sscanf(stream, " CRC: %ld FEC: %ld NCD: %ld", &CRC, &FEC, &NCD))
            {
                parser_data.statistics.error.CRC.Tx = CRC;
                parser_data.statistics.error.CRC.valid ++;
                parser_data.statistics.error.FEC.Tx = FEC;
                parser_data.statistics.error.FEC.valid ++;

                log_debug("CRC tx: %ld FEC tx: %ld\n", CRC, FEC);
            }
            state = state_avms_dsl_tx_interleave_HEC;
        }
        break;
    case state_avms_dsl_tx_interleave_HEC:
        if(parser_find(stream, size, "LCD"))
        {
            long int LCD, HEC;
            if( 2 == sscanf(stream, " LCD: %ld HEC: %ld", &LCD, &HEC))
            {
                parser_data.statistics.error.HEC.Tx = HEC;
                parser_data.statistics.error.HEC.valid++;

                log_debug("HEC tx: %ld\n", HEC);
            }
            state = state_avms_dsl_rx_interleave;
        }
        break;
    case state_avms_dsl_rx_interleave:
        if (parser_find(stream, size, "[Downstream (RX) Interleave path]"))
        {
            log_debug("[Downstream (RX) Interleave path]\n");
            state = state_avms_dsl_rx_interleave_CRC_FEC;
        }
        break;
    case state_avms_dsl_rx_interleave_CRC_FEC:
        if(parser_find(stream, size, "CRC"))
        {
            long int CRC, FEC, NCD;
            if( 3 == sscanf(stream, " CRC: %ld FEC: %ld NCD: %ld", &CRC, &FEC, &NCD))
            {
                parser_data.statistics.error.CRC.Rx = CRC;
                parser_data.statistics.error.CRC.valid ++;
                parser_data.statistics.error.FEC.Rx = FEC;
                parser_data.statistics.error.FEC.valid ++;

                log_debug("CRC rx: %ld FEC rx: %ld\n", CRC, FEC);
            }
            state = state_avms_dsl_rx_interleave_HEC;
        }
        break;
    case state_avms_dsl_rx_interleave_HEC:
        if(parser_find(stream, size, "LCD"))
        {
            long int LCD, HEC;
            if( 2 == sscanf(stream, " LCD: %ld HEC: %ld", &LCD, &HEC))
            {
                parser_data.statistics.error.HEC.Rx = HEC;
                parser_data.statistics.error.HEC.valid++;

                log_debug("HEC rx: %ld\n", HEC);
            }
            state = state_avms_dsl_tx_fastpath;
        }
        break;
    case state_avms_dsl_tx_fastpath:
        if(parser_find(stream, size, "[Upstream (TX) Fast path]"))
        {
            log_debug("[Upstream (TX) Fast path]\n");
            state = state_avms_dsl_tx_fastpath_CRC_FEC;
        }
        break;
    case state_avms_dsl_tx_fastpath_CRC_FEC:
        if(parser_find(stream, size, "CRC"))
        {
            long int CRC, FEC, NCD;
            if( 3 == sscanf(stream, " CRC: %ld FEC: %ld NCD: %ld", &CRC, &FEC, &NCD))
            {
                parser_data.statistics.error.CRC.Tx += CRC;
                parser_data.statistics.error.FEC.Tx += FEC;

                log_debug("CRC tx: %ld FEC tx: %ld\n", CRC, FEC);
            }
            state = state_avms_dsl_tx_fastpath_HEC;
        }
        break;
    case state_avms_dsl_tx_fastpath_HEC:
        if(parser_find(stream, size, "LCD"))
        {
            long int LCD, HEC;
            if( 2 == sscanf(stream, " LCD: %ld HEC: %ld", &LCD, &HEC))
            {
                parser_data.statistics.error.HEC.Tx += HEC;

                log_debug("HEC tx: %ld\n", HEC);
            }
            state = state_avms_dsl_rx_fastpath;
        }
        break;
    case state_avms_dsl_rx_fastpath:
        if(parser_find(stream, size, "[Downstream (RX) Fast path]"))
        {
            log_debug("[Downstream (RX) Fast path]\n");
            state = state_avms_dsl_rx_fastpath_CRC_FEC;
        }
        break;
    case state_avms_dsl_rx_fastpath_CRC_FEC:
        if(parser_find(stream, size, "CRC"))
        {
            long int CRC, FEC, NCD;
            if( 3 == sscanf(stream, " CRC: %ld FEC: %ld NCD: %ld", &CRC, &FEC, &NCD))
            {
                parser_data.statistics.error.CRC.Rx += CRC;
                parser_data.statistics.error.FEC.Rx += FEC;

                log_debug("CRC rx: %ld FEC rx: %ld\n", CRC, FEC);
            }
            state = state_avms_dsl_rx_fastpath_HEC;
        }
        break;
    case state_avms_dsl_rx_fastpath_HEC:
        if(parser_find(stream, size, "LCD"))
        {
            long int LCD, HEC;
            if( 2 == sscanf(stream, " LCD: %ld HEC: %ld", &LCD, &HEC))
            {
                parser_data.statistics.error.HEC.Rx += HEC;

                log_debug("HEC rx: %ld\n", HEC);
            }
            state = state_avms_exit;
        }
        break;
    }

    return state;
}

#if defined (CFG_COMMAND)

#define TONES_PER_LINE 16           /*!< tones per line                     */

/*!
 * @fn          static int parse_tone_bat(const char* stream, int* tone_data, int* tonestart)
 * @brief       parse tone bit allocation
 * @param[in]   stream
 *              data line to parse
 * @param[in,out] tone_data
 *              pointer to tonedata
 * @param[in,out] tonestart
 *              pointer to first index, updated to next index after parsing
 * @return      parse finished
 * @retval      0
 *              continue parsing
 * @retval      1
 *              finished
 */
static int parse_tone_bat(const char* stream, int* tone_data, int* tonestart)
{
    int tone[TONES_PER_LINE];
    int count = sscanf (stream,"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            &tone[0], &tone[1], &tone[2], &tone[3], &tone[4], &tone[5], &tone[6], &tone[7],
            &tone[8], &tone[9], &tone[10], &tone[11], &tone[12], &tone[13], &tone[14], &tone[15]);

    if (count == TONES_PER_LINE)
    {
        int i;
        for (i=0; i<TONES_PER_LINE; i++)
        {
            if (((*tonestart) + i) < parser_data.bandplan->num_tone)
            {
                tone_data[(*tonestart) + i] = tone[i];
            }
            else
            {
                break;
            }
        }
        *tonestart += TONES_PER_LINE;
    }
    else if (tonestart > 0)
    {
        return 1;
    }
    return 0;
}

/*!
 * @fn          static int parse_tone_snr(const char* stream, int* tone_data, int* tonestart)
 * @brief       parse tone signal noise ratio
 * @param[in]   stream
 *              data line to parse
 * @param[in,out] tone_data
 *              pointer to tonedata
 * @param[in,out] tonestart
 *              pointer to first index, updated to next index after parsing
 * @return      parse finished
 * @retval      0
 *              continue parsing
 * @retval      1
 *              finished
 */
static int parse_tone_snr(const char* stream, int* tone_data, int* tonestart)
{
    int tone[TONES_PER_LINE];
    int count = sscanf (stream,"%04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x",
            &tone[0], &tone[1], &tone[2], &tone[3], &tone[4], &tone[5], &tone[6], &tone[7],
            &tone[8], &tone[9], &tone[10], &tone[11], &tone[12], &tone[13], &tone[14], &tone[15]);

    if (count == TONES_PER_LINE)
    {
        int i;
        for (i=0; i<TONES_PER_LINE; i++)
        {
            if (((*tonestart) + i) < parser_data.bandplan->num_tone)
            {
                if(tone[i] & 0x8000)
                {
                    tone[i] = abs(tone[i] - 0x10000);
                }
                tone_data[(*tonestart) + i] = tone[i] / 100;
            }
            else
            {
                break;
            }
        }
        *tonestart += TONES_PER_LINE;
    }
    else if (tonestart > 0)
    {
        return 1;
    }
    return 0;
}

enum
{
    state_avbat_start,
    state_avbat_up,
    state_avbat_up_data,
    state_avbat_down,
    state_avbat_down_data,
    state_avbat_exit = -1
};

/*!
 * @fn          static int parse_avsar_bit_allocation_table (int state, char* stream, size_t size)
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
static int parse_avsar_bit_allocation_table (int state, char* stream, size_t size)
{
    static int tonestart;
    switch (state)
    {
    case state_avbat_start:
        if (parser_find(stream, size,"cat /proc/avalanche/avsar_bit_allocation_table"))
        {
            state = state_avbat_up;
            log_debug("avsar_bit_allocation_table\n");
        }
        break;
    case state_avbat_up:
        if (parser_find(stream, size, "AR7 DSL Modem US Bit Allocation"))
        {
            tonestart = 0;
            parser_data.tone.bitalloc_up.valid = 1;

            state = state_avbat_up_data;
            log_debug("AR7 DSL Modem US Bit Allocation\n");
        }
        break;
    case state_avbat_up_data:
        if (parse_tone_bat(stream, parser_data.tone.bitalloc_up.data, &tonestart))
        {
            state = state_avbat_down;
            log_debug("AR7 DSL Modem US Bit Allocation done\n");
        }
        else
        {
            break;
        }
    case state_avbat_down:
        if (parser_find(stream, size, "AR7 DSL Modem DS Bit Allocation"))
        {
            tonestart = 0;
            parser_data.tone.bitalloc_down.valid = 1;

            state = state_avbat_down_data;
            log_debug("AR7 DSL Modem DS Bit Allocation\n");
        }
        break;
    case state_avbat_down_data:
        if (parse_tone_bat(stream, parser_data.tone.bitalloc_down.data, &tonestart))
        {
            state = state_avbat_exit;
            log_debug("AR7 DSL Modem DS Bit Allocation done\n");
        }
        break;
    }

    return state;
}

enum
{
    state_avrsnr_start,
    state_avrsnr_tone,
    state_avrsnr_tone_data,
    state_avrsnr_exit = -1
};

/*!
 * @fn          static int parse_avsar_rxsnr (int state, char* stream, size_t size)
 * @brief       parse receive signal noise ratio
 * @param[in]   state
 *              actual command to parse
 * @param[in]   stream
 *              data line to parse
 * @param[in]   size
 *              length of data line to parse
 * @return      state
 *              next command to parse
 */
static int parse_avsar_rxsnr (int state, char* stream, size_t size)
{
    static int tonestart;
    switch (state)
    {
    case state_avrsnr_start:
        if (parser_find(stream, size,"cat /proc/avalanche/avsar_rxsnr0"))
        {
            state = state_avrsnr_tone;
            log_debug("avsar_rxsnr0\n");
        }
        break;
    case state_avrsnr_tone:
        if (parser_find(stream, size, "AR7 DSL Modem Rx SNR"))
        {
            tonestart = 0;
            parser_data.tone.snr.valid = 1;

            state = state_avrsnr_tone_data;
            log_debug("AR7 DSL Modem Rx SNR\n");
        }
        break;
    case state_avrsnr_tone_data:
        if(parse_tone_snr(stream, parser_data.tone.snr.data, &tonestart))
        {
            state = state_avrsnr_exit;
            log_debug("SNR data done\n");
        }
        break;
    }

    return state;
}
#endif /* defined (CFG_COMMAND) */

/* _oOo_ */
