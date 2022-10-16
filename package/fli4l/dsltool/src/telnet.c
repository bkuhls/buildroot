/*!
 * @file    telnet.c
 * @brief   implementation file for telnet
 * @details telnet protocoll processing, pass data to parser
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
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "telnet.h"

#ifndef TCP_USER_TIMEOUT
#define TCP_USER_TIMEOUT 18 /* How long for loss retry before timeout */
#endif

static int telnet_loop;
/*!
 * @var         static char buffer[4096]
 * @brief       receive buffer
 */
static char buffer[4096];

/*!
 * @fn          int telnet_open(const char* hostname)
 * @brief       open telnet connection
 * @param[in]   hostname
 *              name or IP address
 * @return      telnet socket
 * @retval      -1
 *              error
 */
int telnet_open(const char* hostname)
{
    int sock;
    int rc;
    int timeout = 2000;
    int syncnt = 1;

    struct hostent* host;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return sock;
    }

    setsockopt (sock, SOL_TCP, TCP_SYNCNT, (char*) &syncnt, sizeof (syncnt));
    setsockopt (sock, SOL_TCP, TCP_USER_TIMEOUT, (char*) &timeout, sizeof (timeout));

    host = gethostbyname(hostname);
    if (0 == host)
    {
        return -1; // @todo
    }

    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, host->h_addr, host->h_length);
    server.sin_port = htons(23);

    rc = connect(sock, (struct sockaddr *) &server, sizeof(server));
    if (rc < 0)
    {
        close(sock);
        return rc;
    }
    return sock;
}

/*!
 * @fn          void telnet_close(int sock)
 * @brief       close telnet connection
 * @param[in]   sock
 *              telnet socket (opened with @ref telnet_open)
 */
void telnet_close(int sock)
{
    close(sock);
}

/*!
 * @fn          void telnet_receive_loop(int sock, int (*callback)(int, char*, size_t))
 * @brief       telnet receiver loop
 * @param[in]   sock
 *              telnet socket (opened with @ref telnet_open)
 * @param[in]   callback
 *              pointer to parser callback function (@ref parser_receive)
 */
void telnet_receive_loop(int sock, int (*callback)(int, char*, size_t))
{
    char* data = malloc(1);
    size_t size = 0;

    int rc;
    fd_set FD;

    telnet_loop = 1;

    while (telnet_loop)
    {
        struct timeval timeout =
        { .tv_sec = 2, .tv_usec = 0 };

        FD_ZERO(&FD);
        FD_SET(sock, &FD);

        rc = select(sock + 1, &FD, (fd_set*) NULL, (fd_set*) NULL, &timeout);

        if (0 == rc)
        {
            break; /* timeout */
        }
        else if (rc < 0)
        {
            continue;
        }
        else
        {
            if (FD_ISSET(sock, &FD))
            {
                int bsize = read(sock, buffer, sizeof(buffer) - 1);

                if (bsize > 0)
                {
                    char* temp;

                    temp = malloc(bsize + size );

                    memcpy (temp, data, size);
                    memcpy (&temp[size], buffer, bsize);
                    free(data);
                    data = temp;
                    size += bsize;

                    if (callback(sock, data, size))
                    {
                        free(data);
                        data = malloc(1);
                        size = 0;
                    }
                }
                else
                {
                    // connection closed
                    break;
                }
            }
        }
    }

    if (data)
    {
        free(data);
    }
}

/*!
 * @fn          void telnet_stop_loop(void)
 * @brief       stop receive loop;
 */
void telnet_stop_loop(void)
{
	telnet_loop = 0;
}

/*!
 * @fn          void telnet_send(int sock, const char* data, int length)
 * @brief       send data
 * @param[in]   sock
 *              telnet socket (opened with @ref telnet_open)
 * @param[in]   data
 *              data to be sent
 * @param[in]   length
 *              size of data to be sent
 */
void telnet_send(int sock, const char* data, int length)
{
    int rc;
    fd_set FD;

    do
    {
        struct timeval timeout =
        { .tv_sec = 5, .tv_usec = 0 };
        FD_ZERO(&FD);
        FD_SET(sock, &FD);

        rc = select(sock + 1, (fd_set*) NULL, &FD, (fd_set*) NULL, &timeout);
    } while (rc < 0);

    if (FD_ISSET(sock, &FD))
    {
        send(sock, data, length, 0);
    }
}

/* _oOo_ */
