/*----------------------------------------------------------------------------
 *  ovpnctrl.c - controls OpenVPN
 *
 *  Copyright (c) 2005 ???
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:	    27.04.2005  babel
 *  Last Update:    28.05.2005  hh
 *----------------------------------------------------------------------------
 */

#include <arpa/inet.h>				/* decl of inet_addr()	    */
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MYBUF 512

#define ERR_USAGE                  10
#define ERR_SERVICE_CONNECT_FAILED 11
#define ERR_READLINE_FAILED        12
#define ERR_SEND_CMD_FAILED        13
#define ERR_REGEX_IS_BAD           14

/*
 * usage: ovpnctrl "cmd" "host" "port" 
 */

static int service_connect(const char *host_name, int port);
static void service_disconnect(int fd);
static int send_command(int fd, const char *str);
static int read_line(int fd, char *buff, int maxlen);

int main(int argc, char *argv[])
{
  char      buff[MYBUF];
  int	    port;
  int	    fd;
  int	    exitcode = 0;
  regex_t   preg;
  int       regex_error;
  regmatch_t matches[2];

  if(5 == argc)
    {
#if DEBUG
      printf("Using regex \"%s\" to match end of command\n", argv[2]);
#endif
      regex_error = regcomp(&preg, argv[2], REG_EXTENDED|REG_NOSUB);
      if(0 != regex_error)
	{
	  regerror(regex_error, &preg, buff, sizeof(buff)-1);
	  fprintf(stderr, "%s\n", buff);
	  exitcode = ERR_REGEX_IS_BAD;
	}

      port = atoi(argv[4]);
#if DEBUG
      fprintf(stderr, "management port is: %d\n", port);
#endif
      fd = service_connect(argv[3], port);
#if DEBUG
      fprintf(stderr, "Connected to service with fd(%d)\n", fd);
#endif
      if(fd > 0)
	{
	  /* ignore help line */
	  if(read_line(fd, buff, sizeof(buff)) > 0)
	  {
	    if(!send_command(fd, argv[1]))
	      {
		for(;;)
		  {
		    if(read_line(fd, buff, sizeof(buff)) > 0)
		      {
#if DEBUG
			printf("Trying to match: \"%s\"\n", buff);
#endif
			regex_error = regexec(&preg, buff, 10, matches, 0 );
			puts(buff);
			if(REG_NOMATCH != regex_error)
			  break;
		      }
		    else
		      {
			exitcode = ERR_READLINE_FAILED;
			break;
		      }
		  }
	      }
	    else
	      {
		exitcode = ERR_SEND_CMD_FAILED;
	      }
	  }
	  else
	  {
	    exitcode = ERR_READLINE_FAILED;
	  }
	  service_disconnect (fd);
	}
      else
	{
	  exitcode = ERR_SERVICE_CONNECT_FAILED;
	}
    }
  else
    {
      fprintf(stderr, "Wrong parameter count, usage is:\n%s: <OpenVPN Management command> <eoc regex> <host> <port>\n", argv[0]);
      exitcode = ERR_USAGE;
    }

    return (exitcode);
}

/*----------------------------------------------------------------------------
 *  service_connect (host_name, port)	    - connect to tcp-service
 *----------------------------------------------------------------------------
 */
static int service_connect(const char *host_name, int port)
{
  struct sockaddr_in	addr;
  struct hostent *	host_p;
  int			fd;
  int			opt = 1;
  
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
    {						/* open socket		    */
      perror ("socket");
      return (-1);
    }
  
  (void) setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, (char *) &opt, sizeof (opt));
  
  if (connect (fd, (struct sockaddr *) &addr, sizeof (addr)) != 0)
    {
      (void) close (fd);
      perror (host_name);
      return (-1);
    }
  
  return (fd);
}


/*----------------------------------------------------------------------------
 *  service_disconnect (fd)		    - disconnect from service
 *----------------------------------------------------------------------------
 */
static void service_disconnect(int fd)
{
    (void) close (fd);
}

static int send_command(int fd, const char *str)
{
  int len = strlen(str);
  int sntb;

#if DEBUG
  fprintf(stderr, "send_command: sending cmd <%s> to service\n", str);
#endif
  sntb = write(fd, str, len);
#if DEBUG
  fprintf(stderr, "send_command: %d bytes written to service, expected %d\n", sntb, len);
#endif
  if(sntb != len)
    return (-1);
  sntb = write(fd, "\n", sizeof("\n")-1);
  if(sntb != sizeof("\n")-1)
    return (-1);

  return (0);
}

static int read_line(int fd, char *buff, int maxlen)
{
  static char oldbuff[MYBUF*2] = { 0 };
  char *ptr;
  int rdb;
  int offset;
  int bufflen;

  offset = strlen(oldbuff);

  for(;;)
    {
      /* search cr/lf in oldbuff */
      ptr = strstr(oldbuff, "\r\n");
      if(NULL != ptr)
	{
	  *ptr = '\0';
	  bufflen = strlen(oldbuff);
	  if(bufflen > maxlen)
	    {
	      fprintf(stderr, "FATAL: Line \"%s\" with %d characters is to long to fit to %d character buffer!\n", oldbuff, bufflen, maxlen);
	      return (-1);
	    }
	  strcpy(buff, oldbuff);
	  memmove(oldbuff, ptr+2, offset - bufflen + 1);
	  break;
	}

      rdb = read(fd, oldbuff + offset, sizeof(oldbuff) - offset - 1);
      if(rdb <= 0)
	{
	  perror("FATAL: Read from managment console failed:");
	  return (-1);
	}
      offset += rdb;
      oldbuff[offset] = '\0';
#if DEBUG
      fprintf(stderr, "read_line: $%s$\n", oldbuff);
#endif
    }

#if DEBUG
  fprintf(stderr, "read_line_exit: !%s!\n", buff);
  fprintf(stderr, "read_line_exit(oldbuff): &%s&\n", oldbuff);
#endif
  return (bufflen);
}


