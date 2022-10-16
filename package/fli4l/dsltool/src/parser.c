/*!
 * @file    parser.c
 * @brief   implementation file for parser
 * @details general parser functions
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
 * @var         parser_data_t parser_data
 * @brief       global parser data variable
 */
parser_data_t parser_data;
/*!
 * @var         parser_cfg_t parser_cfg
 * @brief       global parser config variable
 */
parser_cfg_t parser_cfg;
/*!
 * @var         parser_entry_t parser_list[MAX_PARSER_LIST];
 * @brief       global parser command list
 */
parser_entry_t parser_list[MAX_PARSER_LIST];

/*!
 * @fn          void _parser_init(int command)
 * @brief       initialize parser
 * @details     common parser initialization, must be called by @ref parser_init
 * @param[in]   command
 *              command code
 */
void _parser_init(int command)
{
    int i;
    for(i = 0; i < MAX_PARSER_LIST; i++)
    {
        parser_list[i].parser = NULL;
        parser_list[i].finish = NULL;
        parser_list[i].cmd_str = NULL;
    }

    parser_cfg.state = 0;
    parser_cfg.command = command;
    parser_cfg.cont = 0;
    parser_cfg.index = 0;
    parser_cfg.null = 0;
    parser_cfg.no1prompt = 0;
    memset(&parser_data, 0, sizeof(parser_data));
    parser_data.time = time(NULL);
    parser_data.bandplan = &bandplan_NONE;
}

/*!
 * @fn          parser_data_t* parser_get_data()
 * @brief       get parser data pointer
 * @return      pointer to parser data
 */
parser_data_t* parser_get_data(void)
{
    return &parser_data;
}

/*!
 * @fn          int parser_find(char* str, size_t str_size, const char* find)
 * @brief       find text in parse string
 * @param[in]   str
 *              string to parse
 * @param[in]   str_size
 *              length of string to parse
 * @param[in]   find
 *              string to find
 * @return      string found
 * @retval      0
 *              not found
 * @retval      1
 *              string found
 */
int parser_find(char* str, size_t str_size, const char* find)
{
    size_t find_size = strlen(find);
    int i;

#if 0
    log_debug("%s(\"%s\", %d, \"%s\" %d)\r\n", __func__, str, str_size, find, find_size);
#endif

    if(str_size < find_size)
    {
        return 0;
    }
    for (i = 0; i<= (str_size - find_size);i++)
    {
        if(0 == strncmp(&(str[i]), find, find_size))
        {
            return 1;
        }
    }

    return 0;
}

/*!
 * @fn          void parser_trim(char *p)
 * @brief       remove trailing spaces
 * @param[in]   p
 *              string to be trimmerd
*/
void parser_trim(char *p)
{
    char *end;
    int len = strlen( p);

    while (*p && len)
    {
        end = p + len-1;
        if(' ' == *end)
            *end = 0;
        else
            break;
        len = strlen( p);
    }
}

#if !defined(CFG_DEMO)
static int parse(char* stream, size_t size, int (*parser)(int, char*, size_t));

/*!
 * @fn          int parser_receive(int sock, char* stream, size_t size)
 * @brief       main parser entry
 * @details     parser callback
 * @param[in]   sock
 *              telnet socket
 * @param[in]   stream
 *              data to be parsed
 * @param[in]   size
 *              length of data to parse
 * @return      completion code
 * @retval      1
 *              parse complete
 * @retval      0
 *              continue parsing
 */
int parser_receive(int sock, char* stream, size_t size)
{
    char buffer[256];

#if 0
    log_debug("%s(\"%s\", %d)\r\n", __func__, stream, size);
#endif

    if(0 == parser_cfg.state)
    {
        if (parser_find(stream, size, "Username") ||
            parser_find(stream, size, "Login") ||
            parser_find(stream, size, "login") ||
            parser_find(stream, size, "Account"))
        {
            log_debug("*** login: %s\r\n", parser_cfg.user);
            snprintf(buffer, sizeof(buffer), "%s%s", parser_cfg.user, parser_cfg.enter);
            telnet_send(sock, buffer, strlen(buffer));
            return 1;
        }
        else if (parser_find(stream, size, "Password") ||
                 parser_find(stream, size, "password") ||
                 parser_find(stream, size, "PASSWORD"))
        {
            log_debug("*** password: %s\r\n", parser_cfg.pass);
            snprintf(buffer, sizeof(buffer), "%s", parser_cfg.pass);
            telnet_send(sock, buffer, strlen(buffer));
            snprintf(buffer, sizeof(buffer), "%s", parser_cfg.enter);
            telnet_send(sock, buffer, strlen(buffer) + parser_cfg.null);
            parser_cfg.state = 1;
            return 1;
        }
    }
    else
    {
        if ((1 == parser_cfg.cont) ||
            ((0 == parser_cfg.index) && (1 == parser_cfg.no1prompt)) ||
            (parser_find(stream, size, parser_cfg.prompt)))
        {
#if 0
            log_debug("*** prompt %d\n",parser_cfg.index);
#endif
            if (parser_cfg.index > 0)
            {
                // log_debug(data);
                if (parser_list[parser_cfg.index - 1].parser)
                {
                    parser_cfg.cont = 1;
                    if (-1 == parse(stream, size, parser_list[parser_cfg.index - 1].parser))
                    {
                        parser_cfg.cont = 0;
#if 0
                        log_debug ("*** parse finished");
#endif
                    }
                    else
                    {
#if 0
                        log_debug ("*** parse cont\n");
#endif
                    }
                }
            }
            if((0 == parser_cfg.cont) &&
               (parser_cfg.index < MAX_PARSER_LIST))
            {
                if(parser_list[parser_cfg.index - 1].finish)
                {
                    parser_list[parser_cfg.index - 1].finish();
                }
                if (parser_list[parser_cfg.index].cmd_str)
                {
                    log_debug("*** command: \"%s\"\n", parser_list[parser_cfg.index].cmd_str);
                    telnet_send(sock, parser_list[parser_cfg.index].cmd_str, strlen(parser_list[parser_cfg.index].cmd_str));
                    parser_cfg.index++;
                    return 1;
                }
                else
                {
                    log_debug("*** no command\n", parser_list[parser_cfg.index].cmd_str);
                    parser_cfg.index++;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*!
 * @fn          static int parse(char* stream, size_t size, int (*parser)(int, char*, size_t))
 * @brief       parse data
 * @details      split into lines and pass to callback function
 * @param[in]   stream
 *              data to be parsed
 * @param[in]   size
 *              length of data to parse
 * @param[in]   parser
 *              parser callback function
 * @return      next state
 * @retval      -1
 *              parse finished
 */
static int parse(char* stream, size_t size, int (*parser)(int, char*, size_t))
{
    int state = 0;
    char* parse = stream;

    while(parse)
    {
        char* next = parse;

        if ('\x1b' == *parse)
        {
            next++;
        }

        while(next < (stream + size))
        {
            if (('\r' == *next) || ('\n' == *next) || ('\x1b' == *next))
            {
                break;
            }
            next++;
        }

        if((next > parse))
        {
            char* temp = malloc(next - parse + 1);
            if(NULL != temp)
            {
                memcpy (temp, parse, next - parse);
                temp[next - parse] = '\0';
                state = parser(state, temp, next - parse);
                free(temp);
            }
            else
            {
                state = -1;
                break;
            }
        }

        if (('\r' == *next) || ('\n' == *next))
        {
            next++;
        }

        parse = next;
        if(next >= (stream + size))
        {
            parse = 0;
        }
    }

    return state;
}
#endif /* !defined(CFG_DEMO) */

/* _oOo_ */
