/*-----------------------------------------------------------------------------
 *  imond-stat.c   - send a status message to imond
 *
 *  Multicall:
 *  usage:      imond-stat message
 *  or:         imond-send message
 *  example:    imond-stat "pppoe: up"
 *  or:         imond-send "set-status pppoe: up"
 *  or:         imond-send "dialmode"
 *
 *  Copyright (c) 2000-2016 - Frank Meyer, fli4l-Team <team@fli4l.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:       26.11.2000  fm
 *  Last Update:    $Id: imond-stat.c 44011 2016-01-14 08:34:43Z sklein $
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/errno.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>              /* decl of inet_addr()      */
#include <sys/socket.h>

#include <signal.h>
#define TRUE        1
#define FALSE       0

/*----------------------------------------------------------------------------
 *  service_connect (host_name, port)	    - connect to tcp-service
 *----------------------------------------------------------------------------
 */
static int
service_connect (char * host_name, int port)
{
    struct sockaddr_in	addr;
    struct hostent *    host_p;
    int         fd;
    int         opt = 1;

    (void) memset ((char *) &addr, 0, sizeof (addr));

    if ((addr.sin_addr.s_addr = inet_addr ((char *) host_name)) == INADDR_NONE)
    {
        host_p = gethostbyname (host_name);

    if (! host_p)
    {
        fprintf (stderr, "%s: host not found\n", host_name);
        return (-1);
    }

    (void) memcpy ((char *) (&addr.sin_addr), host_p->h_addr,
            host_p->h_length);
    }

    addr.sin_family  = AF_INET;
    addr.sin_port    = htons ((unsigned short) port);

    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {                       /* open socket          */
        perror ("socket");
        return (-1);
    }

    (void) setsockopt (fd, IPPROTO_TCP, TCP_NODELAY,
       (char *) &opt, sizeof (opt));

    if (connect (fd, (struct sockaddr *) &addr, sizeof (addr)) != 0)
    {
        (void) close (fd);
        perror (host_name);
        return (-1);
    }

    return (fd);
} /* service_connect (char * host_name, int port) */


/*----------------------------------------------------------------------------
 *  main (argc, argv)               - main function
 *----------------------------------------------------------------------------
 */
int timeout = 0;
void alarm_handler(int dummy)
{
    timeout=1;
}
int
main (int argc, char * argv[])
{
    FILE *  fp;
    char    buf[1024];
    int     port;
    int     fd;
    int     len;
    int     i;
    char    * prefix;
    char    * portf;
    char    * p;

    if (argc == 1)
    {
        fprintf (stderr, "usage: %s message\n", argv[0]);
        exit (1);
    }

    if ( strstr(argv[0], "telmond-send") != 0 )
    {
        prefix = "";
        port = 5001;
        portf = "/var/run/telmond.port";
    }
    else
    {
        prefix = strstr(argv[0], "imond-stat") ? "set-status " : "";
        port = 5000;
        portf = "/var/run/imond.port";
    }
    
    fp = fopen (portf, "r");

    if (fp)
    {
        (void) fscanf (fp, "%d", &port);
        fclose (fp);
    }

    fd = service_connect ("localhost", port);

    if (fd < 0)
    {
        return (1);
    }

    p = buf;
    *p = 0;
    for (i = 1; i < argc ; i++ )
    {
        sprintf (p, "%s%s\n", prefix, argv[i]);
        p += strlen(p);
    }
    sprintf (p, "%s\n", "quit");

    (void) write (fd, buf, strlen (buf));       /* send command */
    len = read (fd, buf, sizeof(buf));          /* read answer  */

    if ( ! *prefix) 
    {
        struct sigaction act;
        act.sa_handler = alarm_handler;
        sigemptyset (&act.sa_mask);
        act.sa_flags = 0;

        sigaction (SIGALRM, &act, NULL);

        while (len > 0)
        {
            write (1, buf, len);
            alarm(5);
            len = read (fd, buf, sizeof(buf));
        }
        if (len < 0)
        {
            fprintf(stderr, "%s while waiting for reply\n", timeout ? "timeout" : "unknown error");
            return 1;
        }
    }   
    return (0);
} /* main (int argc, char * argv[]) */
