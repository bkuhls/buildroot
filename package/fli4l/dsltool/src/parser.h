/*!
 * @file    parser.h
 * @brief   header file for parser
 * @details definitions & function declarations
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

#ifndef __PARSER_H__
#define __PARSER_H__

#include <time.h>
#include "bandplan.h"

#define PARSER_INFO     0           /*!< info command                       */
#define PARSER_STATUS   1           /*!< status command                     */
#define PARSER_RESYNC   2           /*!< resync command                     */
#define PARSER_REBOOT   3           /*!< reboot command                     */

#define DSL_TONE_MAX    4096        /*!< maximum tone number for VDSL2       */

#if defined(CFG_DEMO_ADSL) || defined(CFG_DEMO_VDSL)
#define CFG_DEMO
#endif /* defined(CFG_DEMO_ADSL) || defined(CFG_DEMO_VDSL) */
#if defined(CFG_TEST)
typedef struct
{
    char* data;
    size_t size;
} test_data2_t;
#define DAT(name) {name,sizeof(name)}
#define SEP() {"",1}
#endif /* definded(CFG_TEST) */

;
/*!
 * @struct  parser_data_t
 * @brief   parser data structure
 */
typedef struct
{
    time_t time;                    /*!< time stamp                         */
    bandplan_t* bandplan;           /*!< bandplan pointer                   */
    /*!
     * @var     modemstate
     * @brief   modem state
     */
    struct
    {
        char str[80];               /*!< modem state string                 */
        int state;                  /*!< modem state                        */
        int valid;                  /*!< valid flag                         */
    } modemstate;
    /*!
     * @var     operationmode
     * @brief   operation mode
     * @var     channelmode
     * @brief   channel mode (fast or interleaved path)
     */
    struct
    {
        char str[80];               /*! mode string                         */
        int valid;                  /*!< valid flag                         */
    } operationmode, channelmode;
    /*!
     * @var     noisemargin
     * @brief   noisemargin values
     * @var     attenuation
     * @brief   attenuation values
     * @var     txpower
     * @brief   transmit power values
     */
    struct
    {
        double up;                  /* upstream value                       */
        double down;                /* downstream value                     */
        int valid;                  /*!< valid flag                         */
    } noisemargin[DSL_BAND_MAX], attenuation[DSL_BAND_MAX], txpower;
    /*!
     * @var     bandwidth
     * @brief   bandwidth values
     */
    struct
    {
        /*!
         * @var     cells
         * @brief   cells per second values
         * @var     kbit
         * @brief   kbit per second values
         */
        struct
        {
            int up;                 /* upstream value                       */
            int down;               /* downstream value                     */
            int valid;              /*!< valid flag */
        } cells, kbit;
    } bandwidth;

    /*!
     * @var     maxbandwidth
     * @brief   maxbandwidth values
     */
    struct
    {
        /*!
         * @var     kbit
         * @brief   kbit per second values
         */
        struct
        {
            int up;                 /* upstream value                       */
            int down;               /* downstream value                     */
            int valid;              /*!< valid flag */
        } kbit;
    } maxbandwidth;
    /*!
     * @var     atm
     * @brief   ATM data
     */
    struct
    {
        /*!
         * @var     pvc
         * @brief   virtual path/channel
         */
        struct
        {
            int vpi;                /*!< virtual path identifier            */
            int vci;                /*!< virtual channel identifier         */
            int valid;              /*!< valid flag                         */
        } pvc;
        /*!
         * @var     atu_c
         * @brief   ATU-C (DSLAM) data
         * @var     atu_r
         * @brief   ATU-R (DSL-Modem) data
         */
        struct
        {
            char vendor[4];         /*!< vendor code                        */
            int vendspec[2];        /*!< vendor specific bytes              */
            int revision;           /*!< revision                           */
            int valid;              /*!< valid flag */
        } atu_c, atu_r;
    } atm;
    /*!
     * @var     statistics
     * @brief   statistic values
     */
    struct
    {
        /*!
         * @var     error
         * @brief   error counters
         */
        struct
        {
            /*!
             * @var     FEC
             * @brief   frame error counter
             * @var     CRC
             * @brief   CRC error counter
             * @var     HEC
             * @brief   header error counter
             */
            struct
            {
                long Rx;            /*!< receive errors                     */
                long Tx;            /*!< transmit errors                   */
                int valid;          /*!< valid flag */
            } FEC, CRC, HEC;
        } error;
        /*!
         * @var     failure
         * @brief   failure values
         */
        struct
        {
            int error_secs_15min;   /*!< error seconds per 15 min           */
            int error_secs_day;     /*!< error seconds per day              */
            int valid;              /*!< valid flag */
        } failure;
    } statistics;
#if defined (CFG_COMMAND)
    /*!
     * @var     tone
     * @brief   tone data
     */
    struct
    {
        /*!
         * @var     bitalloc_up
         * @brief   upstream bit allocation
         * @var     bitalloc_down
         * @brief   downstream bit allocation
         * @var     snr
         * @brief   signal noise ration data
         * @var     gainQ2
         * @brief   downstream bit allocation
         * @var     noisemargin
         * @brief   noisemargin data
         * @var     chancharlog
         * @brief   channel characteristics
         */
        struct
        {
            int data[DSL_TONE_MAX];/*!< tone data  */
            int valid;              /*!< valid flag */
        }bitalloc_up, bitalloc_down, snr, gainQ2, noisemargin, chancharlog;
    }tone;
#endif /* defined (CFG_COMMAND) */
} parser_data_t;

/*!
 * @struct  parser_cfg_t
 * @brief   parser configuration data
 */
typedef struct {
    int state;                      /*!< parser state                       */
    char* user;                     /*!< user name                          */
    char* pass;                     /*!< password                           */
    char* enter;                    /*!< CR and or LF string                */
    int null;                       /*!< flag for send null byte            */
    char prompt[40];                /*!< prompt                             */
    int no1prompt;                  /*!< flag for missing first prompt      */
    int index;                      /*!< command index                      */
    int command;                    /*!< command number                     */
    int cont;                       /*!< flag for continue parsing          */
} parser_cfg_t;

/*!
 * @struct  parser_entry_t
 * @brief   parser command sequence entry
 */
typedef struct
{
    int (*parser)(int, char*, size_t);      /*!< parser callback function           */
    void (*finish)(void);           /*!< finalization callback function     */
    const char* cmd_str;            /*!< command string                     */
} parser_entry_t;

void _parser_init(int command);
/*!
 * @fn          void parser_init(int command, const char* username, const char* password);
 * @brief       initialize parser
 * @details     specific parser initialization, must call @ref _parser_init first
 * @param[in]   command
 *              command code
 * @param[in]   username
 *              modem login user name
 * @param[in]   password
 *              modem login user password
 */
void parser_init(int command, const char* username, const char* password);

parser_data_t* parser_get_data(void);
int parser_find(char* str, size_t str_size, const char* find);
void parser_trim(char *p);

#if !defined(CFG_DEMO)
int parser_receive(int sock, char* stream, size_t size);
#endif /* !defined(CFG_DEMO) */

extern parser_data_t parser_data;
extern parser_cfg_t parser_cfg;

#define MAX_PARSER_LIST         20          /*!< max.size of parser list    */
extern parser_entry_t parser_list[MAX_PARSER_LIST];

/*!
 * @def         SCANF_BUFFER
 * @brief       default scanf string buffer size
 * @def         STR_
 * @brief       string expansion
 * @def         STR
 * @brief       scanf safe string format
 */
#define SCANF_BUFFER    80
#define STR_(x) # x
#define STR(s,f) "%" STR_(s) STR_(f)
#undef DSL_TONE_MAX
#endif /* __PARSER_H__ */
/* _oOo_ */
