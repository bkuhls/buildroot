/*!
 * @file    collect.c
 * @brief   implementation file for collect output
 * @details function for sending data to collectd server
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
#include <string.h>

#include "parser.h"
#include "collect.h"
#include "log.h"

#include <collectd/client.h>

/*!
 * @var         static parser_data_t last;
 * @brief       last parser data
 */
static parser_data_t last;
/*!
 * @var         static const char* collect_host;
 * @brief       host name
 */
static const char* collect_host;
/*!
 * @var         static const char* collect_modem;
 * @brief       modem hostname or IP
 */
static const char* collect_modem;
/*!
 * @var         static const char* collect_socket;
 * @brief       collectd socket name
 */
static const char* collect_socket;

/*!
 * @fn          void collect_init(const char* host, const char* modem,
 *                  const char* socket)
 * @brief       initialize collect client
 * @param[in]   host
 *              host name
 * @param[in]   modem
 *              modem hostname or IP
 * @param[in]   socket
 *              collectd socket name
 */
void collect_init(const char* host, const char* modem, const char* socket)
{
    collect_host = host;
    collect_modem = modem;
    collect_socket = socket;
}

/*!
 * @fn          static void collectd_putval_gauge(lcc_connection_t* connection,
 *                  time_t time, const char* type, const char* instance,
 *                  double gauge)
 *
 * @brief       put gauge (double) value to collectd
 * @param[in]   connection
 * @param[in]   time
 *              time stamp
 * @param[in]   type
 *              collectd identifier type
 * @param[in]   instance
 *              collectd identifier instance
 * @param[in]   gauge
 *              value
 */
static void collectd_putval_gauge(lcc_connection_t* connection, time_t time,
        const char* type, const char* instance, double gauge)
{
    value_t values[1];
    int value_types[1];
    lcc_value_list_t vlist = {
            .values = values,
            .values_types = value_types,
            .values_len = 1,
            .time = time,
            .interval = 10
    };

    values[0].gauge = gauge;
    value_types[0] = LCC_TYPE_GAUGE;

    strcpy(vlist.identifier.host, collect_host);
    strcpy(vlist.identifier.plugin, "dsltool");
    strcpy(vlist.identifier.plugin_instance, collect_modem);
    strcpy(vlist.identifier.type, type);
    strcpy(vlist.identifier.type_instance, instance);

    lcc_putval(connection, &vlist);
}

/*!
 * @fn          void collect_output(parser_data_t* data)
 * @brief       send data to collectd
 * @param[in]   data
 *              parser data
 */
void collect_output(parser_data_t* data)
{
    int iBand;
    lcc_connection_t* connection;

    log_debug("collect_output");

    if (0 == lcc_connect(collect_socket, &connection))
    {
        if (2 == data->modemstate.valid)
        {
            collectd_putval_gauge(connection, data->time, "modemstate", "",
                    data->modemstate.state);
        }
        if (2 == data->attenuation[0].valid)
        {
            collectd_putval_gauge(connection, data->time, "attenuation", "up",
                    data->attenuation[0].up);
            collectd_putval_gauge(connection, data->time, "attenuation", "down",
                    data->attenuation[0].down);
        }
        if (2 == data->noisemargin[0].valid)
        {
            collectd_putval_gauge(connection, data->time, "noisemargin", "up",
                    data->noisemargin[0].up);
            collectd_putval_gauge(connection, data->time, "noisemargin", "down",
                    data->noisemargin[0].down);
        }
        for(iBand=1; iBand<data->bandplan->num_band; iBand++)
        {
            char type[20];
            if (2 == data->attenuation[iBand].valid)
            {
                snprintf(type, sizeof(type), "attenuation%d", iBand);
                collectd_putval_gauge(connection, data->time, type, "up",
                        data->attenuation[iBand].up);
                collectd_putval_gauge(connection, data->time, type, "down",
                        data->attenuation[iBand].down);
            }
            if (2 == data->noisemargin[iBand].valid)
            {
                snprintf(type, sizeof(type), "noisemargin%d", iBand);
                collectd_putval_gauge(connection, data->time, type, "up",
                        data->noisemargin[iBand].up);
                collectd_putval_gauge(connection, data->time, type, "down",
                        data->noisemargin[iBand].down);
            }
        }

        if (2 == data->txpower.valid)
        {
            collectd_putval_gauge(connection, data->time, "txpower", "up",
                    data->txpower.up);
            collectd_putval_gauge(connection, data->time, "txpower", "down",
                    data->txpower.down);
        }
        if (2 == data->bandwidth.kbit.valid)
        {
            collectd_putval_gauge(connection, data->time, "bandwidth",
                    "kbit-up", data->bandwidth.kbit.up);
            collectd_putval_gauge(connection, data->time, "bandwidth",
                    "kbit-down", data->bandwidth.kbit.down);
        }
        if ((2 == data->statistics.error.FEC.valid) &&
            (2 == last.statistics.error.FEC.valid))
        {
            collectd_putval_gauge(connection, data->time, "statistics",
                    "error-FEC-Rx",
                    (double) (data->statistics.error.FEC.Rx
                            - last.statistics.error.FEC.Rx)
                            / (double) COLLECT_INTERVAL);
            collectd_putval_gauge(connection, data->time, "statistics",
                    "error-FEC-Tx",
                    (double) (data->statistics.error.FEC.Tx
                            - last.statistics.error.FEC.Tx)
                            / (double) COLLECT_INTERVAL);
        }
        if ((2 == data->statistics.error.CRC.valid) &&
            (2 == last.statistics.error.CRC.valid))
        {
            collectd_putval_gauge(connection, data->time, "statistics",
                    "error-CRC-Rx",
                    (double) (data->statistics.error.CRC.Rx
                            - last.statistics.error.CRC.Rx)
                            / (double) COLLECT_INTERVAL);
            collectd_putval_gauge(connection, data->time, "statistics",
                    "error-CRC-Tx",
                    (double) (data->statistics.error.CRC.Tx
                            - last.statistics.error.CRC.Tx)
                            / (double) COLLECT_INTERVAL);
        }
        if ((2 == data->statistics.error.HEC.valid) &&
            (2 == last.statistics.error.HEC.valid))
        {
            collectd_putval_gauge(connection, data->time, "statistics",
                    "error-HEC-Rx",
                    (double) (data->statistics.error.HEC.Rx
                            - last.statistics.error.HEC.Rx)
                            / (double) COLLECT_INTERVAL);
            collectd_putval_gauge(connection, data->time, "statistics",
                    "error-HEC-Tx",
                    (double) (data->statistics.error.HEC.Tx
                            - last.statistics.error.HEC.Tx)
                            / (double) COLLECT_INTERVAL);
        }
        if (2 == data->statistics.failure.valid)
        {
            collectd_putval_gauge(connection, data->time, "statistics",
                    "failure-errsec-15min",
                    data->statistics.failure.error_secs_15min);
            collectd_putval_gauge(connection, data->time, "statistics",
                    "failure-errsec-day",
                    data->statistics.failure.error_secs_day);
        }
        lcc_disconnect(connection);
    }

}

/*!
 * @fn          void collect_copy(parser_data_t* data)
 * @brief       copy data
 * @details     copy parser data for difference calculation
 * @param[in]   data
 *              parser data
 */
void collect_copy(parser_data_t* data)
{
    memcpy(&last, data, sizeof(last));
}

/* _oOo_ */
