/*!
 * @file    dsltool.c
 * @brief   implemenation file for dsltool
 * @details collect data from dsltool and send output to text and graphics
 *          files or stdout
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
#include <limits.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>

#include "telnet.h"
#include "parser.h"
#include "output.h"
#include "log.h"

/*!
 * @struct      config_t
 * @brief       configuration data
 * @details     set by command line
 */
typedef struct
{
    int loglevel;                   /*!< log level                          */
    char* logfile;                  /*!< log file name                      */
    char* path;                     /*!< output path                        */
    char* modem;                    /*!< modem hostname                     */
    char* user;                     /*!< modem login username               */
    char* pass;                     /*!< modem login password               */
    int cmd;                        /*!< command number                     */
} config_t;

static void sig_handler(int signr);
/*!
 * @var         config
 * @brief       configuration data
 */
static config_t config = {
        .loglevel = LOGLEVEL_INFO,
        .logfile = NULL,
        .path = NULL,
        .modem = NULL,
        .user = NULL,
        .pass = NULL,
        .cmd = -1
};

/*!
 * @var         static const char* defaultlogfile
 * @brief       default log file name
 */
static const char* defaultlogfile = "/var/log/dsltoold.log";
/*!
 * @var         static const char* defaultpath
 * @brief       default output path
 */
static const char* defaultpath = "/var/run/";

/*!
 * @fn          int main(int argc, char** argv)
 * @brief       programm entry function
 * @param[in]   argc
 *              argument count
 * @param[in]   argv
 *              argument list
 * @return      error code
 * @retval      EXIT_SUCCESS
 *              success
 * @retval      EXIT_FAILURE
 *              failure
 */
int main(int argc, char** argv)
{
    int opt;
    int help = 0;
#if !defined (CFG_DEMO)
    int sock;
#endif /* !defined (CFG_DEMO) */

    while (-1 != (opt = getopt(argc, argv, "hl::vf::m:u:p:")))
    {
        switch (opt)
        {
        case 'h':
            help = 1;
            break;
        case 'l':
            if (optarg)
            {
                config.logfile = optarg;
            }
            else
            {
                config.logfile = (char*) defaultlogfile;
            }
            break;
        case 'v':
            config.loglevel++;
            break;
        case 'f':
            if (optarg)
            {
                config.path = optarg;
            }
            else
            {
                config.path = (char*) defaultpath;
            }
            break;
        case 'm':
            config.modem = optarg;
            break;
        case 'u':
            config.user = optarg;
            break;
        case 'p':
            config.pass = optarg;
            break;
        }
    }

    if (!help)
    {
        if ((optind + 1) > argc)
        {
            help = 1;
            fprintf(stderr, "missing command argument\n");
        }
        else if ((optind + 1) < argc)
        {
            help = 1;
            fprintf(stderr, "too many command arguments\n");
        }
        else
        {
            if (0 == strcmp(argv[optind], "info"))
            {
                config.cmd = PARSER_INFO;
            }
            else if (0 == strcmp(argv[optind], "status"))
            {
                config.cmd = PARSER_STATUS;
            }
            else if (0 == strcmp(argv[optind], "resync"))
            {
                config.cmd = PARSER_RESYNC;
            }
            else if (0 == strcmp(argv[optind], "reboot"))
            {
                config.cmd = PARSER_REBOOT;
            }
            else
            {
                help = 1;
                fprintf(stderr, "unknown command %s\n", argv[optind]);
            }
        }

        if ((0 == config.modem) || (0 == strlen(config.modem)))
        {
            help = 1;
            fprintf(stderr, "hostname missing\n");
        }
        if ((0 == config.user) || (0 == strlen(config.user)))
        {
            help = 1;
            fprintf(stderr, "user missing\n");
        }
        if ((0 == config.pass) || (0 == strlen(config.pass)))
        {
            help = 1;
            fprintf(stderr, "password missing\n");
        }
    }
    if (help)
    {
        fprintf(stdout, "Usage:\n");
        fprintf(stdout, "%s [-h] [-l[<logfile>]] [-v] [-f[<path>]] -m<hostname>"
                " -u<username> -p<password> info|status|resync|reboot\n",
                basename (argv[0]));
        fprintf(stdout, "    -h        print this help\n");
        fprintf(stdout, "    -l        log to file");
        fprintf(stdout, "              (default: %s)\n", defaultlogfile);
        fprintf(stdout, "    -v        be verbose\n");
        fprintf(stdout, "    -f        generate text & graphics files in <path>\n");
        fprintf(stdout, "              (default: %s)", defaultpath);
        fprintf(stdout, "              (only for info command)\n");
        fprintf(stdout, "    -m        modem hostname\n");
        fprintf(stdout, "    -u        modem username\n");
        fprintf(stdout, "    -p        modem password\n");
        fprintf(stdout, "    info      get modem info\n");
        fprintf(stdout, "    status    print modem status\n");
        fprintf(stdout, "    resync    resync ADSL line\n");
        fprintf(stdout, "    reboot    reboot ADSL modem\n");
        return EXIT_FAILURE;
    }

    log_open(config.loglevel, config.logfile);

    parser_init(config.cmd, config.user, config.pass);

    signal(SIGINT, sig_handler);

#if defined (CFG_DEMO)
    if (1)
    {
#else
    sock = telnet_open(config.modem);
    if (sock > 0)
    {
        telnet_receive_loop(sock, parser_receive);
        telnet_close(sock);
#endif /* !defined (CFG_DEMO) */

        switch (config.cmd)
        {
        case PARSER_INFO:
            if (config.path)
            {
                char file[PATH_MAX];

                snprintf(file, sizeof(file), "%s/%s", config.path, "dsltool.dat");
                output_data_txt(parser_get_data(), file);
                snprintf(file, sizeof(file), "%s/%s", config.path, "dsltool-bits.png");
                output_tone_bits_png(parser_get_data(), file);
                snprintf(file, sizeof(file), "%s/%s", config.path, "dsltool-snr.png");
                output_tone_snr_png(parser_get_data(), file);
                snprintf(file, sizeof(file), "%s/%s", config.path, "dsltool-char.png");
                output_tone_char_png(parser_get_data(), file);
            }
            else
            {
                output_data_std(parser_get_data());
            }
            break;
        case PARSER_STATUS:
            output_data_std(parser_get_data());
            output_tone_std(parser_get_data());
            break;
        }
        log_close();
    }

    return EXIT_SUCCESS;
}

/*!
 * @fn          static void sig_handler(int signr)
 * @brief       signal handler
 * @param[in]   signr
 *              signal number
 */
static void sig_handler(int signr)
{
    switch (signr)
    {
    	case SIGINT:
    		log_trace ("sig_int\n");
    		telnet_stop_loop();
    		break;
    }
}

/* _oOo_ */
