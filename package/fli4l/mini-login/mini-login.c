/*----------------------------------------------------------------------------
 *  mini-login.c   - mini version of login
 *
 *  Copyright (c) 2000-2001 - Frank Meyer <frank@fli4l.de>
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:       06.07.2000  fm
 *  Last Update:    $Id$
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <pwd.h>
#include <shadow.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <termio.h>
#include <fcntl.h>

#include <crypt.h>

#define LINE_LEN 80

static struct termio    oldmode;
static struct termio    newmode;
static char linebuf[LINE_LEN + 1];
static int centred = 0;

static void centred_output(int len)
{
    int i;
    int start = (LINE_LEN - len) / 2;
    memmove(linebuf + start, linebuf, len);
    for (i = 0; i < start; ++i) {
	linebuf[i] = ' ';
    }
    linebuf[start + len] = '\0';
    fputs(linebuf, stdout);
}

static void char_output(char ch, char **p)
{
    if (centred) {
	*(*p)++ = ch;
    }
    else {
	putchar(ch);
    }
}

static void string_output(char const *s, char **p)
{
    if (centred) {
	while (*s) {
	    *(*p)++ = *s++;
	}
    }
    else {
	fputs(s, stdout);
    }
}

int
main (void)
{
    char    buf[128];
    char *  hostname = NULL;
    char *  pass;
    char *  p;
    struct  spwd * sp;
    int	    login_correct = 0;
    FILE *  fp;
    FILE *  fpv;
    FILE *  fpm;

    putchar ('\n');

    fp = fopen ("/etc/issue", "r");

    if (fp)
    {
	int ch;

	p = linebuf;
	while ((ch = getc (fp)) != EOF)
	{
	    if (ch == '%')
	    {
		ch = getc (fp);

		switch (ch)
		{
		    case 'v':
			fpv = fopen ("/etc/version", "r");

			if (fpv)
			{
			    while ((ch = getc (fpv)) != EOF &&
				   ch != '\r' && ch != '\n')
			    {
				char_output(ch, &p);
			    }
			    fclose (fpv);
			}
			else
			{
			    string_output("<unknown version>", &p);
			}
			break;
			
		    case 'm':
                        fpm = popen ("/bin/uname -m", "r");
                        if (fpm)
			{
			    while ((ch = getc (fpm)) != EOF &&
				   ch != '\r' && ch != '\n')
			    {
				char_output(ch, &p);
			    }
			    fclose (fpm);
			}
			else
			{
			    string_output("<unknown architecture>", &p);
			}
			break;
			
		    case 'c':
			if (p == linebuf) {
			    centred = 1;
			    break;
			}
			/* fall through */

		    case 'h':
			if (!hostname) {
			    struct utsname uts;
			    uname(&uts);
			    if (uts.nodename[0]) {
				hostname = strndup(uts.nodename, sizeof uts.nodename);
			    }
			    else {
				hostname = strdup("(unknown)");
			    }
			}
			string_output(hostname, &p);
			break;

		    default:
			char_output('%', &p);
			break;
		}
	    }
	    else if (ch != '\r')
	    {
		if (ch == '\n') {
		    if (centred) {
			centred_output(p - linebuf);
			centred = 0;
			p = linebuf;
		    }
		}
		char_output(ch, &p);
	    }
	}

	fclose (fp);

	if (ch != '\n')		/* Windows editors don't add nl at eof :-(  */
	{
	    putchar ('\n');
	}

	putchar ('\n');
    }

    sp = getspnam ("root");

    if (! sp)
    {
	fputs ("cannot get passwd entry for root\n", stderr);
	exit (1);
    }

    pass = sp->sp_pwdp;

    if (pass)
    {
	int	len = strlen (pass) + 1;
	char *	p   = malloc (len);
	strcpy (p, pass);
	pass = p;
    }

    endspent ();

    if (pass && *pass)
    {
	(void) ioctl (0, TCGETA, &oldmode);
	(void) ioctl (0, TCGETA, &newmode);

	newmode.c_iflag |= ICRNL;
	newmode.c_iflag &= ~INLCR;

	newmode.c_oflag |= ONLCR;
	newmode.c_oflag &= ~ONLRET;

	newmode.c_lflag &= ~ICANON;
	newmode.c_lflag &= ~ISIG;
	newmode.c_lflag &= ~ECHO;

	(void) ioctl (0, TCSETAW, &newmode);

	fputs ("Password: ", stdout);

	p = fgets (buf, sizeof(buf) - 1, stdin);
	putchar ('\n');

	if (p)
	{
	    p = strchr (buf, '\r');

	    if (p)
	    {
		*p = '\0';
	    }

	    p = strchr (buf, '\n');

	    if (p)
	    {
		*p = '\0';
	    }

	    if (! strcmp (pass, crypt (buf, pass)))
	    {
		login_correct = 1;
	    }
	}

	(void) ioctl (0, TCSETAW, &oldmode);
	fflush (stdout);
    }
    else
    {
	login_correct = 1;
    }

    if (login_correct)
    {
	char *	argv[2];

	argv[0] = "-sh";
	argv[1] = (char *) NULL;

	execvp ("/bin/sh", argv);
	fputs ("exec error: cannot execute /bin/sh", stderr);
	sleep (2);
	exit (1);
    }

    if (*buf)
    {
	fputs ("login incorrect\n", stderr);
	sleep (2);
    }

    return (1);
} /* main (argc, argv) */
