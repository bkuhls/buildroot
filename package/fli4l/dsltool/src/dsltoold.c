/*!
 * @file    dsltoold.c
 * @brief   implemenation file for dsltool deamon
 * @details collect data from dsltool and send data to collectd server
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
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include "telnet.h"
#include "parser.h"
#include "collect.h"
#include "log.h"

/*!
 * @struct  configd_t
 * @brief   configuration data
 * @details set by command line
 */
typedef struct
{
    int daemonize;                  /*!< flag to daemonize                  */
    int loglevel;                   /*!< log level                          */
    char* logfile;                  /*!< log file name                      */
    char* modem;                    /*!< modem hostname                     */
    char* user;                     /*!< modem login username               */
    char* pass;                     /*!< modem login password               */
    char* host;                     /*!< host name for rrd tool             */
    char* socket;                   /*!< collectd socket name               */
} configd_t;

/*!
 * @var         static configd_t config
 * @brief       configuration data
 */
static configd_t config = {
        .daemonize = 0,
        .loglevel = LOGLEVEL_INFO,
        .logfile = NULL,
        .modem = NULL,
        .user = NULL,
        .pass = NULL,
        .host = NULL,
        .socket = NULL
};

/*!
 * @var         static const char* defaultlogfile
 * @brief       default log file name
 */
static const char* defaultlogfile = "/var/log/dsltoold.log";
/*!
 * @var         static int exec
 * @brief       execution flag
 */
static int loop = 1;
static int exec = 0;

/*
 * forward declarations
 */
static void execute(void);
static void sig_handler(int signr);

/*!
 * @fn          int main(int argc, char** argv)
 * @brief       program entry function
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

    setvbuf(stdout, NULL, _IONBF, 0); /* make stdout unbuffered */

    while (-1 != (opt = getopt(argc, argv, "hdl::vm:u:p:r:s:")))
    {
        switch (opt)
        {
        case 'h':
            help = 1;
            break;
        case 'd':
            config.daemonize = 1;
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
        case 'm':
            config.modem = optarg;
            break;
        case 'u':
            config.user = optarg;
            break;
        case 'p':
            config.pass = optarg;
            break;
        case 'r':
            config.host = optarg;
            break;
        case 's':
            config.socket = optarg;
            break;
        }
    }

    if (!help)
    {
        if ((optind) != argc)
        {
            help = 1;
            fprintf(stderr, "unknown arguments\n");
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
        if ((0 == config.host) || (0 == strlen(config.host)))
        {
            help = 1;
            fprintf(stderr, "hostname missing\n");
        }
        if ((0 == config.host) || (0 == strlen(config.host)))
        {
            help = 1;
            fprintf(stderr, "socketname missing\n");
        }
    }
    if (help)
    {
        fprintf(stdout, "Usage:\n");
        fprintf(stdout,
                "%s [-h] [-d] [-l[<logfile>]] [-v] -m<hostname> -u<username> -p<password> -r<hostname -s<socket>>\n", basename (argv[0]));
        fprintf(stdout, "    -h        print this help\n");
        fprintf(stdout, "    -d        daemonize\n");
        fprintf(stdout, "    -l        log to file");
        fprintf(stdout, "              (default: %s)\n", defaultlogfile);
        fprintf(stdout, "    -v        be verbose\n");
        fprintf(stdout, "    -m        modem hostname or IP\n");
        fprintf(stdout, "    -u        modem username\n");
        fprintf(stdout, "    -p        modem password\n");
        fprintf(stdout, "    -r        rrd hostname\n");
        fprintf(stdout, "    -s        collectd socket name\n");
        log_trace ("%s:%d exiting\n", __FILE__, __LINE__);
        return EXIT_FAILURE;
    }

    log_open(config.loglevel, config.logfile);
    collect_init(config.host, config.modem, config.socket);

    if (config.daemonize)
    {
        struct itimerval repeat = {
                .it_interval = {
                        .tv_sec = COLLECT_INTERVAL,
                        .tv_usec = 0},
                .it_value = {
                        .tv_sec = 0,
                        .tv_usec = 1000}
        };
        signal(SIGALRM, sig_handler);
        signal(SIGINT, sig_handler);
        setitimer(ITIMER_REAL, &repeat, NULL);

        while (loop)
        {
            usleep(100000);
            if (exec) {
            	exec = 0;
            	execute ();
            }
        };
    }
    else
    {
    }

    log_close();

    log_trace ("%s:%d exiting\n", __FILE__, __LINE__);
    return EXIT_SUCCESS;
}

/*!
 * @fn          static void execute(void)
 * @brief       main execution function
 * @details     opens telnet connection calls parser and passes data to collectd
 */
static void execute (void)
{
    parser_init(PARSER_STATUS, config.user, config.pass);
#if defined (CFG_DEMO)
    if (1)
    {
#else /* defined (CFG_DEMO) */
    int sock = telnet_open(config.modem);
    if (sock > 0)
    {
        telnet_receive_loop(sock, parser_receive);
        telnet_close(sock);
#endif /* defined (CFG_DEMO) */

        collect_output(parser_get_data());
        collect_copy(parser_get_data());
    }
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
    	case SIGALRM:
    		log_trace ("sig_alarm\n");
    		exec = 1;
    		break;
    	case SIGINT:
    		log_trace ("sig_int\n");
    		telnet_stop_loop();
    		loop = 0;
    		break;
    }
}

/* _oOo_ */
