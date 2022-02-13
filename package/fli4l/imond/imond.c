/*----------------------------------------------------------------------------
 *  imond.c - monitor/control isdn connections
 *
 *  should be started as daemon by fli4l-boot script /etc/rc
 *
 *  usage: imond [-port port] [-log-to-syslog] [-beep] [-led /dev/comX] \
 *		 [imond-log-dir [telmond-log-dir [mgetty-log-dir]]]
 *
 *  If logfile is set, imond writes each successful connection after hangup
 *  in following format:
 *
 *  Circuit  YYYY/MM/DD  HH:MM:SS - YYYY/MM/DD  HH:MM:SS  SEC   SEC     ...
 *  ^^^^^^^  ^^^^^^^^^^  ^^^^^^^^   ^^^^^^^^^^  ^^^^^^^^  ^^^   ^^^
 *  Circuit  Start-Date  St-Time     End-Date   End-Time  Time  ChTime  ...
 *
 *  CHARGE   IBYTES  OBYTES  DEVICE  CHGINTVAL  CHGPERMIN
 *  ^^^^^^   ^^^^^^  ^^^^^^  ^^^^^^  ^^^^^^^^^  ^^^^^^^^^
 *  Charge   input   output  device  charge     charge per
 *           bytes   bytes           interval   minute
 *
 *  Copyright (c) 2000-2016 - Frank Meyer, fli4l-Team <team@fli4l.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:	    04.04.2000  fm
 *  Last Update:    $Id: imond.c 44011 2016-01-14 08:34:43Z sklein $
 *----------------------------------------------------------------------------
 * convert_week_day_to_day ()		- convert week day to index 0 - 6
 * add_days ()				- add n days
 * is_holiday ()			- check if day is holiday
 * find_pppoe_conf_data ()		- find conf_idx for pppoe device
 *					  and current time
 * find_circuit ()			- find circuit index of current
 *					  default route circuit
 * date_time ()				- return date and time
 * open_stat_pipe ()			- open status pipe
 * set_status ()			- read status /var/run/imond.stat
 * map_channel ()			- map info index to channel index
 * get_circuit_name_by_conf_idx ()	- get name of circuit
 * get_timetable ()			- get timetable of confiuration X
 * my_system ()				- spawn a subprocess
 * check_routing ()			- check current routing
 * set_dialmode ()			- enable/disable all circuits
 *					  (autodial/off)
 * do_dial ()				- dial/hangup
 * get_telmond_log_file ()		- get name of telmond log file
 * cmd_quantity ()			- print quantity of transferred data
 * cmd_rate ()				- print current rate
 * password_valid ()			- check password
 * receive_file ()			- receive a file
 * send_file ()				- send a file
 * print_status ()			- print current status
 * fill_time_table ()			- fill time tables
 * read_config ()			- read configuration imond.conf
 * measure_pppoe_rates ()		- measure pppoe rates
 * read_line ()				- read line from config file
 * catch_sig ()				- catch signals
 * main ()				- main function
 *----------------------------------------------------------------------------
 * TODO: zusätzlich im Log: IP, EINHEITSlänge, KOSTEN/EINHEIT
 *----------------------------------------------------------------------------
 */

#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <shadow.h>

#include <crypt.h>

#include <syslog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <linux/kd.h>
#include <signal.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>				/* decl of inet_addr()	    */
#include <sys/socket.h>

#include <linux/if.h>
#include <linux/ppp_defs.h>

#ifndef aligned_u64
#  define aligned_u64 unsigned long long __attribute__((aligned(8)))
#endif

#include <linux/types.h>
#include <linux/if_ppp.h>
//#include <linux/isdn.h>

#ifdef __STDC__
#  include <stdarg.h>
#  define VARARG	    ...
#  define VA_START(a,b)	    va_start (a, b)
#  define VA_END(a)	    va_end (a)
#else /* not __STDC__ */
#  include <varargs.h>
#  define VARARG	    va_alist
#  define VA_START(a,b)	    va_start (a)
#  define VA_END(a)	    va_end (a)
#endif /* __STDC__ */

#include "md5.h"

#define PROTOCOL_VERSION	11		/* protocol version	    */
#define IMOND_VERSION		"2.1.4"		/* imond version	    */

#define RAWUP                   1               /* add isdn raw-up support  */

#define	PASS_REQUIRED		1		/* flag if normal pass req. */
#define ADMIN_PASS_REQUIRED	2		/* flag if admin pass req.  */
#define ADMIN_MODE              4		/* flag if in admin mode    */

int password_mode;				/* password mode	    */

#define TRUE			1
#define FALSE			0
#define OK			0
#define ERR			(-1)

#define ACK_STRING		"\006"		/* ACK = 0x06		    */
#define NAK_STRING		"\025"		/* NAK = 0x15		    */
#define ACK_CHAR		'\006'		/* ACK = 0x06		    */
#define NAK_CHAR		'\025'		/* NAK = 0x15		    */

#define POLLING_INTERVAL	10		/* polling interval	    */
#define MIN_MEASURE_INTERVAL	0.5		/* min. measure interval    */
#define CPU_MEASURE_TIME	5		/* cpu load measure time    */
#define UPTIME_MEASURE_TIME	60		/* uptime measure time 	    */

#define TRANSFER_BUFFER_SIZE	1024		/* file transfer buf size   */

/* possible commands for check_routing */
#define SET_IMMEDIATELY		1
#define SET_AGAIN		2
#define SET_IF_CHANGED		3

#define MAX_CLIENTS		64

#define PPPOE_PSEUDO_IDX	9999

static int			client_fd[MAX_CLIENTS];
static char			client_ip[MAX_CLIENTS][16];
static int 			alarm_triggered;

#define PASSWORD_STATE_NONE	0
#define PASSWORD_STATE_NORMAL	1
#define PASSWORD_STATE_ADMIN	2

static unsigned char		password_state[MAX_CLIENTS];

/* this is the salt for each client */
static char			client_salt[MAX_CLIENTS][33];

static struct timeval		current_time;
static struct timeval		last_time_isdn;
static struct timeval		last_time_pppoe;
static float			seconds_passed_isdn;
static float			seconds_passed_pppoe;

static char *			telmond_log_dir;
static char *			imond_log_dir;
static char *			mgetty_log_dir;
static char			imond_log_file[256];
static char			mgetty_log_file[256];
static int			log_to_syslog = FALSE;
static int			do_beep = FALSE;
static int			beep_pid = 0;

static int			isdninfo_fd;

static char		default_device[16]; /* deflt routing device: ipppX  */
static char		default_circuit[32];

static int		pppoe_used;	    /* flag if pppoe used	    */
static int		pppoe_socket = -1;  /* socket of /dev/ppp0	    */
static int		pppoe_is_active;    /* true if ppp0 is available    */
static int		n_active_channels;  /* number of active channels    */
static char		pppoe_interface[] = "ppp0";

typedef struct
{
  unsigned long		ibytes;
  unsigned long		obytes;
} Siobytes;

struct
{
    int		    is_active;			/* flag: TRUE if active	    */
    int		    conf_idx;			/* idx in array conf_data   */
    int		    is_outgoing_direction;	/* flag: TRUE if outgoing   */
    unsigned long long  ibytes;			/* # input bytes	    */
    unsigned long long  obytes;			/* # output bytes	    */
    unsigned long   last_ibytes;		/* # last read ibytes */
    unsigned long   last_obytes;		/* # last read obytes */
    int		    time_bandwidth_higher;	/* time since rate higher   */
    int		    time_bandwidth_lower;	/* time since rate lower    */
    int		    ibytes_per_second;		/* input bytes per second   */
    int		    obytes_per_second;		/* output bytes per second  */
    time_t	    online_time;		/* online time in seconds   */
    time_t	    charge_time;		/* charge time in seconds   */
    time_t	    online_start;		/* time start we are online */
    float	    charge;			/* charge		    */
    char	    ip_address[16];		/* only filled if wanted    */
    char	    drvid[32];
    char	    status[32];
    char	    phone[32];
    char	    usage[32];
    char	    inout[32];
} infos[64];

struct
{
    int		    conf_idx;			/* idx in array conf_data   */
    time_t	    online_time;		/* online time in seconds   */
    time_t	    charge_time;		/* charge time in seconds   */
    time_t	    online_start;		/* time start we are online */
    unsigned long long   ibytes;		/* # input bytes	    */
    unsigned long long   obytes;		/* # output bytes	    */
    unsigned long   last_ibytes;                /* # input bytes            */
    unsigned long   last_obytes;                /* # output bytes           */
    int		    ibytes_per_second;		/* input bytes per second   */
    int		    obytes_per_second;		/* output bytes per second  */
    float	    charge;			/* charge		    */
    char	    ip_address[16];		/* only filled if wanted    */
    char	    status[32];
} pppoe_infos;

#define CONF_FILE		"/etc/imond.conf"

#define MAX_CIRCUITS		32
#ifndef MAX_CONFIG_LINES
#define MAX_CONFIG_LINES	(MAX_CIRCUITS * 4)
#endif

typedef struct
{
    char    time_table[168];	    /* timetable			    */
    char    is_lcr;		    /* TRUE or FALSE			    */
    char    is_default_route;	    /* TRUE or FALSE			    */
    char    outgoing_phone[128];    /* outgoing phone number		    */
    char    incoming_phone[128];    /* incoming phone numbers (sep by comma)*/
    int	    circuit_idx;	    /* index of circuit			    */
    float   charge_per_minute;	    /* charge per min			    */
    int	    charge_int;		    /* charge interval in seconds, e.g. 60  */
    int	    is_pppoe;		    /* flag: configuration of pppoe circuit */
} CONF_DATA;


typedef struct
{
    char    name[32];		    /* name of circuit			    */
    char    device_1[16];	    /* "ipppX" or "isdnX"		    */
    char    device_2[16];	    /* "ipppX" or "isdnX" (ch-bundling)	    */
    int     online;                 /* flag if circuit online               */
    int	    hup_timeout;	    /* hangup timeout in seconds	    */
    int	    bandwidth_time;	    /* time for bandwidth channel bundling  */
    int	    bandwidth_rate;	    /* rate for bandwidth channel bundling  */
    int	    time_2;		    /* time down			    */
    int	    rate_2;		    /* rate down			    */
    int	    link_added;		    /* flag if link added (for ch-bundling) */
} CIRCUIT;


#define SUNDAY		     0	    /* first day of a unix week		    */
#define HOURS_PER_DAY	    24	    /* hours per day			    */
#define DAYS_PER_WEEK	     7	    /* days per week			    */


#define IS_LEAP_YEAR(y)	((((y) % 4) == 0) &&    \
			     ((((y) % 100) != 0) || (((y) % 400) == 0)))

struct holidays
{
    int	    day;
    int	    month;
};

#include <time.h>
int stime(const time_t *t)
{
   struct timespec ts = {};
   ts.tv_sec = *t;
   return clock_settime(CLOCK_REALTIME, &ts);
}

static int  g_days_per_month[]    =
{
    31,					/* January			    */
    28,					/* February (may be changed to 29)  */
    31,					/* March			    */
    30,					/* April			    */
    31,					/* May				    */
    30,					/* June				    */
    31,					/* July				    */
    31,					/* August			    */
    30,					/* September			    */
    31,					/* October			    */
    30,					/* November			    */
    31					/* December			    */
};

#define N_HOLIDAYS  9

struct holidays holiday_table[N_HOLIDAYS] =
{
    {  1,  1 },				    /* Neujahr			    */
    {  1,  5 },				    /* Maifeiertag		    */
    {  3, 10 },				    /* Tag der deutschen Einheit    */
    { 25, 12 },				    /* 1. Weihnachtsfeiertag	    */
    { 26, 12 },				    /* 2. Weihnachtsfeiertag	    */

    { -2,  0 },				    /* Karfreitag		    */
    {  1,  0 },				    /* Ostermontag		    */
    { 39,  0 },				    /* Christi Himmelfahrt	    */
    { 50,  0 },				    /* Pfingstmontag		    */
#if 0
    { 60,  0 },				    /* Fronleichnam		    */
#endif
};

struct holidays holidays[N_HOLIDAYS];

#define INT(x)	    (x)			    /* integer function		    */

static char *	en_wday_strings[DAYS_PER_WEEK] =
{
    "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"
};

static char *	ge_wday_strings[DAYS_PER_WEEK] =
{
    "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"
};

#define EXCLUSIVE_STATUS_STR	"Exclusive"
#define OFFLINE_STATUS_STR	"Offline"
#define CALLING_STATUS_STR	"Calling"
#define ONLINE_STATUS_STR	"Online"

#define ISDN_IS_ACTIVE		infos[channel_idx].is_active
#define ISDN_CONF_IDX		(channel_idx >= 0 ?			\
				    infos[channel_idx].conf_idx : -1)
#define ISDN_IS_OUTGOING_DIR	infos[channel_idx].is_outgoing_direction
#define ISDN_IBYTES		infos[channel_idx].ibytes
#define ISDN_OBYTES		infos[channel_idx].obytes
#define ISDN_LAST_IBYTES	infos[channel_idx].last_ibytes
#define ISDN_LAST_OBYTES	infos[channel_idx].last_obytes
#define ISDN_TIME_BW_HIGHER	infos[channel_idx].time_bandwidth_higher
#define ISDN_TIME_BW_LOWER	infos[channel_idx].time_bandwidth_lower
#define ISDN_ONLINE_START	infos[channel_idx].online_start
#define ISDN_ONLINE_TIME	infos[channel_idx].online_time
#define ISDN_CHARGE_TIME	infos[channel_idx].charge_time
#define ISDN_CHARGE		infos[channel_idx].charge
#define ISDN_IBYTES_PER_SECOND	infos[channel_idx].ibytes_per_second
#define ISDN_OBYTES_PER_SECOND	infos[channel_idx].obytes_per_second
#define ISDN_DRVID		infos[channel_idx].drvid
#define ISDN_PHONE		infos[channel_idx].phone
#define ISDN_USAGE		infos[channel_idx].usage
#define ISDN_INOUT		infos[channel_idx].inout
#define ISDN_STATUS		infos[channel_idx].status
#define ISDN_IP_ADDRESS		infos[channel_idx].ip_address

#define PPPOE_CONF_IDX		pppoe_infos.conf_idx
#define PPPOE_ONLINE_TIME	pppoe_infos.online_time
#define PPPOE_CHARGE_TIME	pppoe_infos.charge_time
#define PPPOE_ONLINE_START	pppoe_infos.online_start
#define PPPOE_CHARGE		pppoe_infos.charge
#define PPPOE_IP_ADDRESS	pppoe_infos.ip_address
#define PPPOE_STATUS		pppoe_infos.status
#define PPPOE_IBYTES		pppoe_infos.ibytes
#define PPPOE_OBYTES		pppoe_infos.obytes
#define PPPOE_LAST_IBYTES       pppoe_infos.last_ibytes
#define PPPOE_LAST_OBYTES       pppoe_infos.last_obytes
#define PPPOE_IBYTES_PER_SECOND	pppoe_infos.ibytes_per_second
#define PPPOE_OBYTES_PER_SECOND	pppoe_infos.obytes_per_second

#define CONF_CIRCUIT_IDX	(conf_idx >= 0 ?			    \
				 conf_data[conf_idx].circuit_idx : -1)
#define CONF_CHARGE_INTERVAL	(conf_idx >= 0 ?			    \
				    conf_data[conf_idx].charge_int : 1)
#define CONF_CHARGE_PER_MINUTE	(conf_idx >= 0 ?			    \
				    conf_data[conf_idx].charge_per_minute : 0)


#define DIALMODE_OFF		0
#define DIALMODE_AUTO		1
#define DIALMODE_MANUAL		2

static char *			imond_pass;
static char *			imond_admin_pass;
static char			time_table[HOURS_PER_DAY * DAYS_PER_WEEK];
static int			dialmode = DIALMODE_AUTO;
static CONF_DATA		conf_data[MAX_CONFIG_LINES];
static int			n_conf_lines_used;

static CIRCUIT			circuits[MAX_CIRCUITS];
static int			n_circuits;

static int			route_idx = -1;	    /* -1: automatic	    */

static int			online_counter;	    /* #channels online	    */
static char *			led_device;	    /* led: /dev/comX	    */
static int			led_fd = -1;	    /* fd to /dev/comX	    */

static int			sock_fd;

static void			check_routing (int);
static int                      my_system (char * cmd);

#define PPPOE_STATS_IBYTES pppoe_stats.p.ppp_ibytes
#define PPPOE_STATS_OBYTES pppoe_stats.p.ppp_obytes

static char *			date_time (time_t seconds);
static void measure_pppoe_rates (void);

/*----------------------------------------------------------------------------
 * imond_syslog (int log_level, const char *fmt, VARARG)
 * 
 * log message depending on log_to_syslog
 */
#define LOG_BUF_SIZE 1024

#ifdef __STDC__
void imond_syslog (int log_level, const char *fmt, VARARG);
void imond_syslog (int log_level, const char *fmt, VARARG)
#else
void imond_syslog (log_level, fmt, VARARG)
     int 		log_level;
     FILE *		file;
     const char *	fmt;
     va_dcl
#endif
{
    va_list ap;
    char	msg_buf[LOG_BUF_SIZE];
    
    VA_START (ap, fmt);
    if (log_to_syslog)
    {
	vsprintf (msg_buf, fmt, ap);
	syslog (log_level, "%s", msg_buf);
    }
    else
    {
	vfprintf (stderr, fmt, ap);
    }
    VA_END (ap);
}

/*----------------------------------------------------------------------------
 * imond_do_open_log (int append)
 * 	- open imond logfile, either in append or write mode
 * imond_close_log (char * log_file)
 * 	- close and sync imond logfile
 * imond_reset_log (char * log_file)
 * 	- reset imond logfile
 * imond_write_record (...)
 * 	 - write an accounting record to imond log file
 */
static FILE * imond_log_fp = NULL;
int imond_do_open_log (int append);
void imond_close_log (void);
void imond_reset_log (void);
void imond_write_record (char * circuit_name,
			 time_t online_start,
			 /* time_t now, */
			 /* long diff, */ 
			 long charge_time,
			 double charge_val,
			 unsigned long long ibytes,
			 unsigned long long obytes,
			 char * device,
			 int charge_int,
			 double charge_per_minute);

#define imond_open_log imond_do_open_log (1)

int imond_do_open_log (int append)
{
    if (* imond_log_file)
    {
	imond_log_fp = fopen (imond_log_file, append ? "a" : "w");
	if (imond_log_fp)
	{
	    return 1;
	}
	else
	{
	    imond_syslog (LOG_ERR, "Error opening imond logfile '%s': %s\n",
			  imond_log_file, strerror (errno));
	}
    }
    return 0;
}

void imond_close_log ()
{
    if (imond_log_fp)
    {
	fclose (imond_log_fp);
	imond_log_fp = NULL;
	
	/* Execute sync call as child process to prevent lockups */
	if (fork () == 0)
	{
	    sync ();
	    exit (0);
	}
    }
}

void imond_reset_log ()
{
    if (*imond_log_file && access (imond_log_file, 0) == 0)
    {
	imond_do_open_log (0);
	imond_close_log ();
    }
}

void imond_write_record (char * circuit_name,
			 time_t online_start,
			 /* time_t now, */
			 /* long diff, */ 
			 long charge_time,
			 double charge_val,
			 unsigned long long ibytes,
			 unsigned long long obytes,
			 char * device,
			 int charge_int,
			 double charge_per_minute)
{
    time_t  now		= time ((time_t *) 0);
    time_t  diff	= now - online_start;

    if (imond_do_open_log (1))
    {
	fprintf (imond_log_fp, "%-20s %s",
		 circuit_name, date_time (online_start));
	fprintf (imond_log_fp, " %s %5ld %5ld %6.2f",
		 date_time (now), (long) diff,
		 charge_time, charge_val);
	fprintf (imond_log_fp, " %lu %lu %lu %lu",
		 (unsigned long)(ibytes >> 32), (unsigned long)ibytes,
		 (unsigned long)(obytes >> 32), (unsigned long)obytes);
	fprintf (imond_log_fp, " %s %d %6.4f\n",
		 device, charge_int, charge_per_minute);
	imond_close_log ();
    }
}

/*---------------------------------------------------------------------------
 *
 * play_beep(freq,duration)
 * 
 * beep loudly
 *---------------------------------------------------------------------------
 */
void 
play_beep(int freq, int duration)
{
  int fd = open("/dev/tty1",0);
  if (fd)
    {
      ioctl(fd,KDMKTONE, (duration << 16) + 1193180/ freq);
      close(fd);
    }
}
  


/*----------------------------------------------------------------------------
 * beep (is_going_up)			    - beep
 *
 * we fork here to return immediately and continue our work
 *----------------------------------------------------------------------------
 */
int
beep (int is_going_up)
{
    int	pid;

    if ((pid = fork ()) != 0)				/* it's the parent  */
    {
	return (pid);
    }
    
    if (is_going_up)
      {
        play_beep(660,200);
        usleep (300000);
        play_beep(800,200);
      }
      else
      {
        play_beep(880,200);
        usleep (300000);
        play_beep(660,200);
      }
    exit (0);
    
} /* beep (int is_going_up) */

/*----------------------------------------------------------------------------
 * convert_week_day_to_day ()		    - convert week day to index 0 - 6
 *----------------------------------------------------------------------------
 */
static int
convert_week_day_to_day (char * week_day)
{
    int	i;

    for (i = 0; i < DAYS_PER_WEEK; i++)
    {
	if (! strcmp (week_day, en_wday_strings[i]) ||
	    ! strcmp (week_day, ge_wday_strings[i]))
	{
	    return (i);
	}
    }

    return (-1);
} /* convert_week_day_to_day (week_day) */


/*----------------------------------------------------------------------------
 * add_days ()				    - add n days
 *----------------------------------------------------------------------------
 */
static void
add_days (int start_day, int start_month, int start_year, int n_days,
	  int * new_day_p, int * new_month_p)
{
    int	days_this_month;

    while (n_days > 0)
    {
	start_day += n_days;

	days_this_month = g_days_per_month[start_month - 1];

	if (start_month == 1 && IS_LEAP_YEAR(start_year))
	{
	    days_this_month++;
	}

	if (start_day > days_this_month)
	{
	    n_days	= start_day - days_this_month - 1;
	    start_day	= 1;
	    start_month++;

	    if (start_month > 12)
	    {
		start_month = 1;
		start_year++;
	    }
	}
	else
	{
	    n_days = 0;
	}
    }

    while (n_days < 0)
    {
	start_day += n_days;

	if (start_day <= 0)
	{
	    n_days = start_day;

	    start_month--;

	    if (start_month == 0)
	    {
		start_month = 12;
		start_year--;
	    }

	    days_this_month = g_days_per_month[start_month - 1];

	    if (start_month == 1 && IS_LEAP_YEAR(start_year))
	    {
		days_this_month++;
	    }

	    start_day = days_this_month;
	}
	else
	{
	    n_days = 0;
	}
    }

    *new_day_p	    = start_day;
    *new_month_p    = start_month;

    return;
} /* add_days (start_day, start_month, start_year, n_days, new_day_p, new_month_p) */


/*----------------------------------------------------------------------------
 * is_holiday ()			    - check if day is holiday
 *----------------------------------------------------------------------------
 */
static int
is_holiday (int day, int month, int year)
{
    static int	last_year;
    static int	ostern_day, ostern_month;
    int		a, b, c, d, e;
    int		m, s, M, N, D;
    int		i;
    int		offset;

    if (last_year != year)
    {
	last_year = year;

	a = year % 19;
	b = year % 4;
	c = year % 7;

	m = INT ((8 * INT (year/100) + 13) / 25) - 2;
	s = INT (year/100) - INT (year/400) - 2;

	M = (15 + s - m) % 30;
	N = (6 + s) % 7;

	d = (M + 19 * a) % 30;

	if (d == 29)
	{
	    D = 28;
	}
	else if (d == 28 && a >= 11)
	{
	    D = 27;
	}
	else
	{
	    D = d;
	}

	e = (2 * b + 4 * c + 6 * D + N) % 7;

	offset = D + e + 1;

	ostern_day = 21 + offset;
	ostern_month = 3;

	while (ostern_day > 31)
	{
	    ostern_day -= 31;
	    ostern_month++;
	}

	for (i = 0; i < N_HOLIDAYS; i++)
	{
	    if (holiday_table[i].month == 0)
	    {
		add_days (ostern_day, ostern_month, year, holiday_table[i].day,
			  &(holidays[i].day), &(holidays[i].month));
	    }
	    else
	    {
		holidays[i].day	    = holiday_table[i].day;
		holidays[i].month   = holiday_table[i].month;
	    }
	}

#ifdef TEST
	printf ("Ostern: %02d.%02d.%04d\n", ostern_day, ostern_month, year);
#endif
    }

    for (i = 0; i < N_HOLIDAYS; i++)
    {
	if (holidays[i].day == day && holidays[i].month == month)
	{
	    return (TRUE);
	}
    }
    return (FALSE);
} /* is_holiday (day, month, year) */


int
find_phone (char * phone_str, char * phone)
{
    char *  pp;

    do
    {
	pp = strchr (phone_str, ',');	    /* numbers separated by comma   */

	if (pp)
	{
	    *pp = '\0';
	}

	if (! strcmp (phone_str, phone))    /* phone is pattern to find	    */
	{
	    if (pp)
	    {
		*pp = ',';
	    }
	    return (TRUE);
	}

	if (pp)
	{
	    *pp = ',';
	    phone_str = pp + 1;
	}
    } while (pp);

    return (FALSE);
} /* find_phone (phone_str, phone) */

/*
 * device_online () - check whether a device is actually online
 */
int 
circuit_device_online (char * name)
{
    return 0;
}

/*----------------------------------------------------------------------------
 * find_pppoe_conf_data ()		    - find conf_idx for pppoe device
 *					      and current time
 *----------------------------------------------------------------------------
 */
static int
find_pppoe_conf_data (void)
{
    struct tm *	tm;
    time_t	seconds;
    int		wday;
    int		hour;
    int		i;

    seconds = time ((time_t *) 0);
    tm	    = localtime (&seconds);

    if (is_holiday (tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900))
    {
	wday = SUNDAY;				/* holiday: use sunday  */
    }
    else
    {
	wday = tm->tm_wday;
    }

    hour = wday * HOURS_PER_DAY + tm->tm_hour;

    for (i = 0; i < n_conf_lines_used; i++)
    {
	if (conf_data[i].is_pppoe && conf_data[i].time_table[hour] == 1)
	{
	    return (i);
	}
    }

    return (-1);
} /* find_pppoe_conf_data () */


/*----------------------------------------------------------------------------
 * find_circuit ()			    - find circuit index of current
 *					      default route circuit
 *----------------------------------------------------------------------------
 */
static int
find_circuit (void)
{
    struct tm *	tm;
    time_t	seconds;
    int		wday;
    int		hour;
    int		i;

    if (route_idx >= 0)
    {
	return (route_idx);
    }

    seconds = time ((time_t *) 0);
    tm	    = localtime (&seconds);

    if (is_holiday (tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900))
    {
	wday = SUNDAY;				    /* holiday: use sunday  */
    }
    else
    {
	wday = tm->tm_wday;
    }

    hour = wday * HOURS_PER_DAY + tm->tm_hour;

    for (i = 0; i < n_conf_lines_used; i++)
    {
	if (conf_data[i].time_table[hour] == 1 && conf_data[i].is_lcr)
	{
	    return (conf_data[i].circuit_idx);
	}
    }

    return (-1);
} /* find_circuit () */


/*----------------------------------------------------------------------------
 * date_time ()				    - return date and time
 *----------------------------------------------------------------------------
 */
static char *
date_time (time_t seconds)
{
    static char buf[64];
    struct tm *	tm;

    tm = localtime (&seconds);

    sprintf (buf, "%04d/%02d/%02d %02d:%02d:%02d",
	     1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
	     tm->tm_hour, tm->tm_min, tm->tm_sec);
    return (buf);
} /* date_time (seconds) */

/*----------------------------------------------------------------------------
 * set_status ()			    - read status /var/run/imond.stat
 *----------------------------------------------------------------------------
 */
static int
set_status (char * arg)
{
    char *  p = arg;
    char * arg1;
    int	    rtc = ERR;

    /* e.g. "pppoe: up" or "pppoe up" or "pppoe up ip" */
    while (*p && *p != ':' && !isspace(*p))
    {
	p++;
    }

    if (!*p)					/* unknown status info	    */
    {						/* ignore it		    */
	return (ERR);
    }

    *p++ = '\0';

    while (isspace(*p))
    {
	p++;
    }
    arg1 = p;

    while (*p && !isspace(*p))
    {
	p++;
    }
    *p = '\0';

    if (! strcmp (arg, "pppoe") || ! strcmp (arg, "ppp0"))
    {
	if (! strcmp (arg1, "up"))
	{
	    if (PPPOE_ONLINE_START == 0)		/* new connection?  */
	    {
		online_counter++;

		if (led_fd < 0 && led_device)
		{
		    led_fd = open (led_device, 0);
		}

		if (do_beep == TRUE && beep_pid <= 0)
		{
		    beep_pid = beep (TRUE);
		}

		strcpy (PPPOE_STATUS, ONLINE_STATUS_STR);

		gettimeofday (&(last_time_pppoe), NULL);
		PPPOE_ONLINE_START = time ((time_t *) NULL);

		PPPOE_CONF_IDX = find_pppoe_conf_data ();

		rtc = OK;
	    }
	}
	else if (!strcmp (arg1, "down") || !strcmp (arg1, "restart"))
	{
	    strcpy (PPPOE_STATUS, OFFLINE_STATUS_STR);

	    if (PPPOE_ONLINE_START != 0)		/* known connection?*/
	    {
		int	conf_idx    = PPPOE_CONF_IDX;
		int	circuit_idx = CONF_CIRCUIT_IDX;
		int	charge_int  = CONF_CHARGE_INTERVAL;
		time_t	now	    = time ((time_t *) 0);
		time_t	diff	    = now - PPPOE_ONLINE_START;
		time_t  charge_diff;
		float   charge;
		
		int restart = ! strcmp (arg1, "restart");

		if (! restart)
		{
		    measure_pppoe_rates ();
		}

		if (online_counter > 0)
		{
		    online_counter--;

		    if (online_counter == 0 && led_fd >= 0)
		    {
			close (led_fd);
			led_fd = -1;
		    }

		    if (do_beep == TRUE && beep_pid <= 0)
		    {
			beep_pid = beep (FALSE);
		    }
		}

		charge = (diff * CONF_CHARGE_PER_MINUTE) / 60;
		charge_diff = diff;
		if (charge_int > 1)
		{
		    charge_diff = (charge_diff / charge_int) * charge_int 
			+ charge_int;
		}
 
		PPPOE_CHARGE_TIME += charge_diff;
		PPPOE_CHARGE += charge;

		imond_write_record (circuit_idx >= 0 ?
				    circuits[circuit_idx].name : "<unknown>",
				    PPPOE_ONLINE_START,
				    charge_diff, charge,
				    PPPOE_IBYTES,
				    PPPOE_OBYTES,
				    circuit_idx >= 0 ?
				    circuits[circuit_idx].device_1 : "<unknown>",
				    charge_int, CONF_CHARGE_PER_MINUTE);

		/* pppoe doesn't reset byte counters if hangup */
		PPPOE_IBYTES		    = 0;
		PPPOE_OBYTES		    = 0;
		PPPOE_IBYTES_PER_SECOND	    = 0;
		PPPOE_OBYTES_PER_SECOND	    = 0;
		PPPOE_ONLINE_TIME	    += diff;
		PPPOE_ONLINE_START	    = 0;
		PPPOE_IP_ADDRESS[0]	    = '\0';

		if (restart)
		{
		    PPPOE_LAST_IBYTES       = 0;
		    PPPOE_LAST_OBYTES       = 0;
		    imond_syslog (LOG_INFO, "pppd restartet ...");
		}

		rtc = OK;
	    }
	    check_routing (SET_AGAIN); /* must set route again */
	}
    }

    return (rtc);
} /* set_status (arg) */


/*----------------------------------------------------------------------------
 * map_channel ()			    - map info index to channel index
 *
 *  info_idx begins with 1, not 0 !
 *----------------------------------------------------------------------------
 */
static int
map_channel (int info_idx)
{
    return (-1);
} /* map_channel (info_idx) */


/*----------------------------------------------------------------------------
 * get_circuit_name_by_conf_idx ()	    - get name of circuit
 *----------------------------------------------------------------------------
 */
char *
get_circuit_name_by_conf_idx (int conf_idx)
{
    char *  rtc = "unknown";
    int	    circuit_idx;

    if (conf_idx >= 0 && conf_idx < n_conf_lines_used)
    {
	circuit_idx = CONF_CIRCUIT_IDX;

	if (circuit_idx >= 0)
	{
	    rtc = circuits[circuit_idx].name;
	}
    }

    return (rtc);
} /* get_circuit_name_by_conf_idx (conf_idx) */


/*----------------------------------------------------------------------------
 * get_timetable ()			    - get timetable of confiuration X
 *----------------------------------------------------------------------------
 */
char *
get_timetable (int conf_idx)
{
    static char	timetable_str[4096];
    char *	tp;
    char *	p;
    int		circuit_idx;
    int		i;
    int		j;

    if (conf_idx < 0 || conf_idx >= MAX_CONFIG_LINES)
    {
	conf_idx = -1;
	tp = time_table;
    }
    else
    {
	tp = conf_data[conf_idx].time_table;
    }

    strcpy (timetable_str, "   ");

    for (j = 0; j < HOURS_PER_DAY; j++)
    {
	sprintf (timetable_str + strlen (timetable_str), " %2d", j);
    }

    p = timetable_str + strlen (timetable_str);

    *p++ = '\r';
    *p++ = '\n';
    *p++ = ' ';

    for (j = 0; j < 74; j++)
    {
	*p++ = '-';
    }

    *p++ = '\r';
    *p++ = '\n';
    *p++ = '\0';

    for (i = 0; i < DAYS_PER_WEEK; i++)
    {
	sprintf (timetable_str + strlen (timetable_str), " %s",
		 en_wday_strings[i]);

	if (conf_idx < 0)
	{
	    for (j = 0; j < HOURS_PER_DAY; j++)
	    {
		sprintf (timetable_str + strlen (timetable_str), " %2d",
			 tp[i * HOURS_PER_DAY + j] + 1);
	    }
	}
	else
	{
	    for (j = 0; j < HOURS_PER_DAY; j++)
	    {
		sprintf (timetable_str + strlen (timetable_str), " %2d",
			 tp[i * HOURS_PER_DAY + j] > 0 ? conf_idx + 1 : 0);
	    }
	}

	strcat (timetable_str, "\r\n");
    }

    strcat (timetable_str,
	    " \r\n No.  Name                     LCR     DRoute  Device  Ch/Min   ChInt\r\n");

    if (conf_idx < 0)
    {
	for (i = 0; i < n_conf_lines_used; i++)
	{
	    circuit_idx = conf_data[i].circuit_idx;

	    sprintf (timetable_str + strlen (timetable_str),
		     " %2d   %-20s     %-7s %-7s %-7s %6.4f    %3d\n",
		     i + 1, get_circuit_name_by_conf_idx (i),
		     conf_data[i].is_lcr ? "yes" : "no",
		     conf_data[i].is_default_route ? "yes" : "no",
		     circuits[circuit_idx].device_1,
		     conf_data[i].charge_per_minute,
		     conf_data[i].charge_int);
	}
    }
    else
    {
	circuit_idx = CONF_CIRCUIT_IDX;

	if (circuit_idx >= 0)
	{
	    sprintf (timetable_str + strlen (timetable_str),
		     " %2d   %-20s     %-7s %-7s %-7s %6.4f    %3d\n",
		     conf_idx + 1, get_circuit_name_by_conf_idx (conf_idx),
		     conf_data[conf_idx].is_lcr ? "yes" : "no",
		     conf_data[conf_idx].is_default_route ? "yes" : "no",
		     circuits[circuit_idx].device_1,
		     CONF_CHARGE_PER_MINUTE,
		     conf_data[conf_idx].charge_int);
	}
    }

    return (timetable_str);
} /* get_timetable (conf_idx) */


/*----------------------------------------------------------------------------
 * my_system ()				    - spawn a subprocess
 *----------------------------------------------------------------------------
 */
static int
my_system (char * cmd)
{
    int	rtc;

#ifdef TEST

    printf ("imond: %s\n", cmd);
    rtc = 0;

#else

#ifdef DEBUG_ROUTES
    imond_syslog (LOG_INFO, "my_system (\"%s\")\n", cmd);
#endif
    rtc = (int) ((unsigned int) system (cmd) >> 8);

#endif

    return (rtc);
} /* my_system (cmd) */


/*----------------------------------------------------------------------------
 * check_routing ()			    - check current routing
 *----------------------------------------------------------------------------
 */
static void
check_routing (int cmd)
{
    static int	default_route_deleted = TRUE;
    char	buf[256];
    FILE *	fp;
    char *	device;
    char *	circuit;
    int		route_changed;
    int		circuit_idx;

    buf[0] = '\0';

    if (route_idx >= 0)				    /* manual routing ...   */
    {
	circuit_idx = route_idx;
    }
    else					    /* automatic LC routing */
    {
	circuit_idx = find_circuit ();

#ifdef TEST
	printf ("imond: check_routing(): circuit_idx = %d\n", circuit_idx);
#endif
    }

    if (circuit_idx >= 0)
    {
	device	= circuits[circuit_idx].device_1;
	circuit	= circuits[circuit_idx].name;

#ifdef TEST
	printf ("imond: check_routing(): device = %s, last default device = %s\n",
		device, default_device);
#endif

	if (strcmp (default_device, device) != 0)
	{
	    route_changed = TRUE;
	}
	else
	{
	    route_changed = FALSE;
	}

	if (cmd == SET_IMMEDIATELY || route_changed == TRUE)
	{
	    unlink ("/etc/last-default-route-interface");
	    fp = fopen ("/etc/default-route-interface", "w");

	    if (fp)
	    {
		fprintf (fp, "%s\n", device);
		fclose (fp);
	    }
	    else
	    {
		perror ("/etc/default-route-interface");
		exit (1);
	    }

	    if (*default_device)
	    {
		imond_syslog (LOG_INFO,
			      "removing default route to circuit '%s'\n",
			      default_circuit);

		sprintf (buf,
		    "/usr/local/bin/fli4lctrl hangup %s; /usr/local/bin/delete-all-routes %s; ",
		    default_device, default_device);
	    }

	    strcpy (default_device, device);
	    strcpy (default_circuit, circuit);
	}

	if (cmd == SET_IMMEDIATELY)
	{
	    if (dialmode == DIALMODE_AUTO || strcmp (default_device, "pppoe"))
	    {
		imond_syslog (LOG_INFO, 
			      "setting default route to circuit '%s'\n",
			      default_circuit);

		sprintf (buf + strlen (buf),
			 "/usr/local/bin/add-default-route %s 2>/dev/null",
			 default_device);
		default_route_deleted = FALSE;
	    }
	}
	else if (cmd == SET_AGAIN)		/* hangup of pppoe device   */
	{
	    if (dialmode == DIALMODE_AUTO || strcmp (default_device, "pppoe"))
	    {					/* auto or new isdn default */
		imond_syslog (LOG_INFO, 
			      "setting default route again to circuit '%s'\n",
			      default_circuit);

		sprintf (buf + strlen (buf),
			 "sleep 3; /usr/local/bin/add-default-route %s 2>/dev/null",
			 default_device);
		default_route_deleted = FALSE;
	    }
	    else if (! strcmp (default_device, "pppoe"))
	    {			/* non-auto and default circuit is pppoe    */
		sprintf (buf + strlen (buf),
			 "/usr/local/bin/delete-all-routes %s",
			 default_device);
		default_route_deleted = TRUE;
		*default_device = '\0';
		rename ("/etc/default-route-interface", 
			"/etc/last-default-route-interface");
	    }
	}
	else if (route_changed == TRUE)
	{
	    if (dialmode == DIALMODE_AUTO || strcmp (default_device, "pppoe"))
	    {
		imond_syslog (LOG_INFO, 
			      "changing default route to circuit '%s'\n",
			      default_circuit);

		sprintf (buf + strlen (buf),
			 "(sleep 3; /usr/local/bin/add-default-route %s 2>/dev/null) &",
			 default_device);
		default_route_deleted = FALSE;
	    }
	}
    }
    else
    {
	if (! default_route_deleted)
	{
	    imond_syslog (LOG_INFO, "deleting default route\n");

	    sprintf (buf,
		 "/usr/local/bin/fli4lctrl hangup %s deldefault",
		 default_device);
	    default_route_deleted = TRUE;
	    *default_device = '\0';
	    rename ("/etc/default-route-interface", 
		    "/etc/last-default-route-interface");

	}
    }

    if (* buf)
    {
	(void) my_system (buf);
    }

    return;
} /* check_routing (cmd) */


/*----------------------------------------------------------------------------
 * set_dialmode ()			    - enable/disable all circuits
 *					      (autodial/off)
 *----------------------------------------------------------------------------
 */
static int
set_dialmode (int new_dialmode)
{
    char    buf[128];
    char *  dialmode_string;
    int	    circuit_idx;
    int	    srtc;
    int	    rtc = 0;

    switch (new_dialmode)
    {
	case DIALMODE_OFF:	dialmode_string = "off";    break;
	case DIALMODE_AUTO:	dialmode_string = "auto";   break;
	default:		dialmode_string = "manual"; break;
    }

    for (circuit_idx = 0; circuit_idx < n_circuits; circuit_idx++)
    {
	sprintf (buf, "/usr/local/bin/fli4lctrl dialmode %s %s",
		 circuits[circuit_idx].device_1, dialmode_string);

	srtc = my_system (buf);

	if (srtc != 0)
	{
	    rtc = -1;
	}
    }

    dialmode = new_dialmode;

    return (rtc);
} /* set_dialmode (new_dialmode) */


/*----------------------------------------------------------------------------
 * do_dial ()				    - dial/hangup
 *----------------------------------------------------------------------------
 */
static int
do_dial (char * channel_id, int dial_flag)
{
    char    buf[128];
    int	    circuit_idx;
    int	    srtc;
    int	    rtc = 0;

    if (channel_id && dial_flag == FALSE)	/* hangup specific channel  */
    {
	if (! strcmp (channel_id, "pppoe"))
	{
	    strcpy (buf, "/usr/local/bin/fli4lctrl hangup pppoe");

	    srtc = my_system (buf);

	    if (srtc != 0)
	    {
		rtc = -1;
	    }

	    return (rtc);			/* return here !	    */
	}
	else
	{
	    int	    channel_idx = map_channel (atoi (channel_id));
	    int	    conf_idx    = ISDN_CONF_IDX;

	    circuit_idx = CONF_CIRCUIT_IDX;
	}
    }
    else
    {
	circuit_idx = find_circuit ();
    }

    if (circuit_idx >= 0)
    {
	sprintf (buf, "/usr/local/bin/fli4lctrl %s %s",
		     dial_flag == TRUE ? "dial" : "hangup",
		 circuits[circuit_idx].device_1);

	srtc = my_system (buf);

	if (srtc != 0)
	{
	    rtc = -1;
	}
    }

    return (rtc);
} /* do_dial (channel_id, dial_flag) */


/*----------------------------------------------------------------------------
 * get_telmond_log_file ()		    - get name of telmond log file
 *----------------------------------------------------------------------------
 */
static char *
get_telmond_log_file (int client_idx)
{
    static char	log_file[128];

    if (telmond_log_dir)
    {
	sprintf (log_file, "%s/telmond-%s.log", telmond_log_dir,
		 client_ip[client_idx]);

	if (access (log_file, 0) != 0)
	{
	    sprintf (log_file, "%s/telmond.log", telmond_log_dir);
	}

	return (log_file);
    }
    return ((char *) NULL);
} /* get_telmond_log_file () */


/*----------------------------------------------------------------------------
 * hexmd5 (string)	- A small wrapper to the md5 algorithm to use it
 *			  on a string and return some hex digits 
 *----------------------------------------------------------------------------
 */
static char *
hexmd5(const char *string) {
	static unsigned char md5buffer[16];
	static char tmp[33];
	size_t i;
	
	md5_buffer(string, strlen(string), md5buffer);
	
	for (i = 0; i < 16; i++)
		sprintf (tmp + (2 * i), "%02x", md5buffer[i]);
	      
	return tmp;
}

char *
cmd_help (char * answer, char * arg, int client_idx)
{
    if (! arg)
    {
	if ((password_mode & ADMIN_PASS_REQUIRED) == 0 ||
	    (password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
	{
	    sprintf (answer,
		"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
		" Administrative Commands:",
		"   addlink            adjust-time       hup-timeout         removelink",
		"   reset-imond-log-file                 reset-telmond-log-file",
		"   receive            send              delete              support sync",
		" ");
	}
	else
	{
	    *answer = '\0';
	}

	sprintf (answer + strlen (answer),
	    "%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
	    " User or Administrative Commands:",
	    "   dial               dialmode           disable            enable",
	    "   halt               hangup             hup-timeout        poweroff",
	    "   reboot             route",
	    " ");

	sprintf (answer + strlen (answer),
	    "%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s",
	    " User Commands:",
	    "   channels           charge             chargetime         circuit",
	    "   circuits           cpu                client-ip          date",
	    "   device             dialmode           driverid           fli4l-id",
	    "   highscore          hostname           inout              imond-log-file",
	    "   ip                 is-allowed         is-enabled         links",
	    "   log-dir            md5pass            mgetty-log-file    pass",
	    "   phone              pppoe              quantity           quit",
	    "   rate               route              salt               telmond-log-file",
	    "   time               timetable          uptime             usage",
	    "   version",
	    " ",
	    " Type \"help command\" to get more help.");

	if ((password_mode & ADMIN_PASS_REQUIRED) != 0 &&
	    (password_state[client_idx] & PASSWORD_STATE_ADMIN) == 0)
	{
	    strcpy (answer + strlen (answer),
		    "\r\n Enter Admin password to get info about administrative commands.");
	}

    }
    else if (! strcmp (arg, "version"))
    {
	strcpy (answer, "    version                  show protocol- and imond-version");
    }
    else if (! strcmp (arg, "quit"))
    {
	strcpy (answer, "    quit                     quit");
    }
    else if (! strcmp (arg, "pass"))
    {
	strcpy (answer, "    pass [password]          ask, if password required or set password");
    }
    else if (! strcmp (arg, "channels"))
    {
	strcpy (answer, "    channels                 show number of channels");
    }
    else if (! strcmp (arg, "is-enabled"))
    {
	strcpy (answer, "    is-enabled               return 1 if enabled, else 0");
    }
    else if (! strcmp (arg, "dialmode"))
    {
	strcpy (answer, "    dialmode [auto|manual|off]   get/set dialmode");
    }
    else if (! strcmp (arg, "fli4l-id"))
    {
	strcpy (answer, "    fli4l-id                 get fli4l-id");
    }
    else if (! strcmp (arg, "highscore"))
    {
	strcpy (answer, "    highscore                get highscore");
    }
    else if (! strcmp (arg, "hostname"))
    {
	strcpy (answer, "    hostname                 get hostname");
    }
    else if (! strcmp (arg, "is-allowed"))
    {
	strcpy (answer, "    is-allowed dial|dialmode|route|reboot|imond-log|telmond-log|mgetty-log\r\n                             get permission");
    }
    else if (! strcmp (arg, "log-dir"))
    {
	strcpy (answer, "    log-dir imond|telmond|mgetty   get logdirectory");
    }
    else if (! strcmp (arg, "enable"))
    {
	strcpy (answer, "    enable                   set dialmode to 'auto'");
    }
    else if (! strcmp (arg, "disable"))
    {
	strcpy (answer, "    disable                  hangup, set dialmode off");
    }
    else if (! strcmp (arg, "dial"))
    {
	strcpy (answer, "    dial                     dial default route connection");
    }
    else if (! strcmp (arg, "hangup"))
    {
	strcpy (answer, "    hangup [#channel-id]     hangup connections");
    }
    else if (! strcmp (arg, "reboot"))
    {
	strcpy (answer, "    reboot                   reboot");
    }
    else if (! strcmp (arg, "halt"))
    {
	strcpy (answer, "    halt                     shutdown (halt)");
    }
    else if (! strcmp (arg, "poweroff"))
    {
	strcpy (answer, "    poweroff                 shutdown (poweroff)");
    }
    else if (! strcmp (arg, "date"))
    {
	strcpy (answer, "    date                     show date");
    }
    else if (! strcmp (arg, "circuits"))
    {
	strcpy (answer, "    circuits                 show number of default route circuits");
    }
    else if (! strcmp (arg, "circuit"))
    {
	strcpy (answer, "    circuit ci-index         show name of circuit");
    }
    else if (! strcmp (arg, "device"))
    {
	strcpy (answer, "    device ci-index          show device of circuit");
    }
    else if (! strcmp (arg, "hup-timeout"))
    {
	strcpy (answer, "    hup-timeout ci-index [value]  show/set hup-timeout of circuit");
    }
    else if (! strcmp (arg, "timetable"))
    {
	strcpy (answer, "    timetable [ci-index]     show timetable");
    }
    else if (! strcmp (arg, "route"))
    {
	strcpy (answer, "    route [ci-index]         get/set default route circuit");
    }
    else if (! strcmp (arg, "addlink"))
    {
	strcpy (answer, "    addlink ci-index         add link to channel");
    }
    else if (! strcmp (arg, "removelink"))
    {
	strcpy (answer, "    removelink ci-index      remove link from channel");
    }
    else if (! strcmp (arg, "links"))
    {
	strcpy (answer, "    links ci-index           show number of links: 0, 1, or 2");
    }
    else if (! strcmp (arg, "driverid"))
    {
	strcpy (answer, "    driverid #channel-id     show driver id of channel");
    }
    else if (! strcmp (arg, "status"))
    {
	strcpy (answer, "    status #channel-id       show status of channel");
    }
    else if (! strcmp (arg, "online-time"))
    {
	strcpy (answer, "    online-time #channel-id  show current online time of channel");
    }
    else if (! strcmp (arg, "time"))
    {
	strcpy (answer, "    time #channel-id         show sum of online times of channel");
    }
    else if (! strcmp (arg, "chargetime"))
    {
	strcpy (answer, "    chargetime #channel-id   show sum of charge times");
    }
    else if (! strcmp (arg, "charge"))
    {
	strcpy (answer, "    charge #channel-id       show charge");
    }
    else if (! strcmp (arg, "phone"))
    {
	strcpy (answer, "    phone #channel-id        show phone number/circuit name");
    }
    else if (! strcmp (arg, "usage"))
    {
	strcpy (answer, "    usage #channel-id        show usage of channel");
    }
    else if (! strcmp (arg, "inout"))
    {
	strcpy (answer, "    inout #channel-id        show direction of channel");
    }
    else if (! strcmp (arg, "ip"))
    {
	strcpy (answer, "    ip #channel-id           show ip address of interface using channel");
    }
    else if (! strcmp (arg, "quantity"))
    {
	strcpy (answer, "    quantity #channel-id     show transferred data quantity of channel");
    }
    else if (! strcmp (arg, "rate"))
    {
	strcpy (answer, "    rate #channel-id         show isdn rate of channel");
    }
    else if (! strcmp (arg, "sync"))
    {
	strcpy (answer, "    sync                     synchronize cache with mounted devices");
    }
    else if (! strcmp (arg, "pppoe"))
    {
	strcpy (answer, "    pppoe                    return 1 if pppoe used, otherwise 0");
    }
    else if (! strcmp (arg, "cpu"))
    {
	strcpy (answer, "    cpu                      show cpu usage");
    }
    else if (! strcmp (arg, "uptime"))
    {
	strcpy (answer, "    uptime                   router uptime (in seconds)");	
    }
    else if (! strcmp (arg, "telmond-log-file"))
    {
	strcpy (answer, "    telmond-log-file         show logfile of telmond");
    }
    else if (! strcmp (arg, "reset-telmond-log-file"))
    {
	strcpy (answer, "    reset-telmond-log-file   reset logfile of telmond");
    }
    else if (! strcmp (arg, "imond-log-file"))
    {
	strcpy (answer, "    imond-log-file           show logfile of imond");
    }
    else if (! strcmp (arg, "mgetty-log-file"))
    {
	strcpy (answer, "    mgetty-log-file          show logfile of mgetty");
    }
    else if (! strcmp (arg, "client-ip"))
    {
	strcpy (answer, "    client-ip [ip-address]   show/set ip address of client");
    }
    else if (! strcmp (arg, "reset-imond-log-file"))
    {
	strcpy (answer, "    reset-imond-log-file     reset logfile of imond");
    }
    else if (! strcmp (arg, "adjust-time"))
    {
	strcpy (answer, "    adjust-time #seconds     adjust time by n seconds");
    }
    else if (! strcmp (arg, "receive"))
    {
	strcpy (answer, "    receive filename #bytes pw  receive a file");
    }
    else if (! strcmp (arg, "send"))
    {
	strcpy (answer, "    send filename pw         send a file");
    }
    else if (! strcmp (arg, "delete"))
    {
	strcpy (answer, "    delete filename pw       delete a file");
    }
    else if (! strcmp (arg, "support"))
    {
	strcpy (answer, "    support password         show config/status of fli4l");
    }
    else if (! strcmp (arg, "salt"))
    {
	strcpy (answer, "    support                  send a random salt to the client");
    }
    else if (! strcmp (arg, "md5pass"))
    {
	strcpy (answer, "    md5pass [password]       set password in a secure way");
    }
    else
    {
	sprintf (answer, "    unknown command: %s", arg);
    }

    strcat (answer, "\r\nOK 0\r\n");
    return (answer);
} /* cmd_help (answer, arg, client_idx) */

/*----------------------------------------------------------------------------
 * cmd_passwd ()				    - get password from client
 *----------------------------------------------------------------------------
 */
char *
cmd_pass (char * answer, char * arg, int client_idx)
{
    int	error_occured = FALSE;

    if (arg)
    {
	error_occured = TRUE;

	if (imond_admin_pass && ! strcmp (arg, imond_admin_pass))
	{
	    password_state[client_idx] |=   PASSWORD_STATE_ADMIN |
					    PASSWORD_STATE_NORMAL;
	    error_occured = FALSE;
	}

	if (imond_pass && ! strcmp (arg, imond_pass))	    /* NOT else if! */
	{
	    password_state[client_idx] |= PASSWORD_STATE_NORMAL;

	    if (! imond_admin_pass)		/* no admin pass necessary  */
	    {
		password_state[client_idx] |= PASSWORD_STATE_ADMIN;
	    }
	    error_occured = FALSE;
	}
    }

    if (error_occured)
    {
	strcpy (answer, "ERR\r\n");
    }
    else if (password_state[client_idx] & PASSWORD_STATE_ADMIN)
    {
	sprintf (answer, "OK %d\r\n", password_mode | ADMIN_MODE);
    }
    else
    {
	sprintf (answer, "OK %d\r\n", password_mode);
    }
    return (answer);
} /* cmd_pass (answer, arg, client_idx) */

/*----------------------------------------------------------------------------
 * cmd_salt ()				    - send a random salt to the client
 *----------------------------------------------------------------------------
 */

char *
cmd_salt (char * answer, int client_idx)
{
    FILE * random;
    char buf[1024];
    char salt[33];
    static unsigned char md5buffer[16];
    size_t i;

    random = fopen("/dev/urandom", "r");
    fread (buf, 1024, 1, random);
    fclose (random);
   
    md5_buffer(buf, 1024, md5buffer);

    for (i = 0; i < 16; i++)
	sprintf (salt + (2 * i), "%02x", md5buffer[i]);
    
    strcpy (client_salt[client_idx], salt);
    sprintf (answer, "OK %s\r\n", salt);
    return (answer);
}

/*----------------------------------------------------------------------------
 * cmd_md5passwd ()				- get password in a secure way
 *----------------------------------------------------------------------------
 */
char *
cmd_md5pass (char * answer, char * arg, int client_idx)
{
    int	error_occured = FALSE;
    char tmp[255];
    char * md5passwd;
    
    if (arg)
    {
	error_occured = TRUE;

	if (strlen(client_salt[client_idx]) == 0)
	{
	    strcpy (answer, "ERR there was no salt!\r\n");
	    return (answer);
	}

	if (imond_admin_pass)
	{
	    sprintf (tmp, "%s%s", imond_admin_pass, client_salt[client_idx]);
	    md5passwd = hexmd5(tmp);
	    if (! strcmp (arg, md5passwd))
	    {
		password_state[client_idx] |=   PASSWORD_STATE_ADMIN |
						PASSWORD_STATE_NORMAL;
		error_occured = FALSE;
	    }
	}

	if (imond_pass)
	{
	    sprintf (tmp, "%s%s", imond_pass, client_salt[client_idx]);
	    md5passwd = hexmd5(tmp);
	    if (! strcmp (arg, md5passwd))
	    {
		password_state[client_idx] |= PASSWORD_STATE_NORMAL;

		if (! imond_admin_pass)		/* no admin pass necessary  */
		{
		    password_state[client_idx] |= PASSWORD_STATE_ADMIN;
		}
		error_occured = FALSE;
	    }
	}
    }

    if (error_occured)
    {
	strcpy (answer, "ERR\r\n");
    }
    else if (password_state[client_idx] & PASSWORD_STATE_ADMIN)
    {
	sprintf (answer, "OK %d\r\n", password_mode | ADMIN_MODE);
    }
    else
    {
	sprintf (answer, "OK %d\r\n", password_mode);
    }
    return (answer);
} /* cmd_md5pass (answer, arg, client_idx) */

/*----------------------------------------------------------------------------
 * cmd_ip ()				    - print ip address of channel
 *----------------------------------------------------------------------------
 */
char *
cmd_ip (char * answer, int idx)
{
    int	channel_idx;

    if (idx == PPPOE_PSEUDO_IDX)
    {
	if (* PPPOE_IP_ADDRESS)
	{
	    sprintf (answer, "OK %s\r\n", PPPOE_IP_ADDRESS);
	}

	else if (PPPOE_ONLINE_START != 0)
	{
	    char    ip_file[64];
	    char    ip_buf[32];
	    FILE *  ip_fp;
	    int	    conf_idx;

	    ip_buf[0] = '\0';

	    conf_idx = PPPOE_CONF_IDX;

	    if (conf_idx >= 0)
	    {
		int circuit_idx = CONF_CIRCUIT_IDX;

		if (circuit_idx >= 0)
		{
		    sprintf (ip_file, "/var/run/%s.ip",
			     circuits[circuit_idx].device_1);

		    ip_fp = fopen (ip_file, "r");

		    if (ip_fp)
		    {
			if (fgets (ip_buf, 32, ip_fp))
			{
			    char *  p = strchr (ip_buf, '\r');

			    if (! p)
			    {
				p = strchr (ip_buf, '\n');
			    }

			    if (p)	    /* copy only if file complete   */
			    {
				*p = '\0';

				strncpy (PPPOE_IP_ADDRESS, ip_buf, 16);
			    }
			}

			fclose (ip_fp);
		    }
		}
	    }

	    sprintf (answer, "OK %s\r\n", ip_buf);
	}
    }
    else
    {
	channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    if (* ISDN_IP_ADDRESS)
	    {
		sprintf (answer, "OK %s\r\n", ISDN_IP_ADDRESS);
	    }

	    else if (ISDN_ONLINE_START != 0)
	    {
		char    ip_file[64];
		char    ip_buf[32];
		FILE *  ip_fp;
		int	conf_idx;

		ip_buf[0] = '\0';

		conf_idx = ISDN_CONF_IDX;

		if (conf_idx >= 0)
		{
		    int circuit_idx = CONF_CIRCUIT_IDX;

		    if (circuit_idx >= 0)
		    {
			sprintf (ip_file, "/var/run/%s.ip",
				 circuits[circuit_idx].device_1);

			ip_fp = fopen (ip_file, "r");

			if (ip_fp)
			{
			    if (fgets (ip_buf, 32, ip_fp))
			    {
				char *  p = strchr (ip_buf, '\r');

				if (! p)
				{
				    p = strchr (ip_buf, '\n');
				}

				if (p)		/* copy only if file complete   */
				{
				    *p = '\0';

				    strncpy (ISDN_IP_ADDRESS, ip_buf, 16);
				}
			    }

			    fclose (ip_fp);
			}
		    }
		}

		sprintf (answer, "OK %s\r\n", ip_buf);
	    }
	}
	else
	{
	    strcpy (answer, "ERR\r\n");
	}
    }

    return (answer);
} /* cmd_ip (answer, idx) */

int
addlink (int circuit_idx, int channel_idx)
{
    int	rtc;

    if (circuits[circuit_idx].link_added == 0)
    {
	char buf[256];

	sprintf (buf, "/usr/local/bin/fli4lctrl addlink %s >/dev/console 2>&1",
		 circuits[circuit_idx].device_1);

	rtc = my_system (buf);

	if (rtc == 0)
	{
	    circuits[circuit_idx].link_added++;
	}
    }
    else
    {
	rtc = -1;
    }
    return (rtc);
} /* addlink (int circuit_idx, int channel_idx) */


int
removelink (int circuit_idx, int channel_idx)
{
    int	rtc;

    if (circuits[circuit_idx].link_added > 0)
    {
	char buf[256];

	sprintf (buf,
		 "/usr/local/bin/fli4lctrl removelink %s >/dev/console 2>&1",
		 circuits[circuit_idx].device_1);

	rtc = my_system (buf);

	if (rtc == 0)
	{
	    circuits[circuit_idx].link_added--;
	}
    }
    else
    {
	rtc = -1;
    }
    return (rtc);
} /* removelink (int circuit_idx, int channel_idx) */

/*----------------------------------------------------------------------------
 * cmd_quantity ()			- print quantity of transferred data
 *----------------------------------------------------------------------------
 */
char *
cmd_quantity (char * answer, int idx)
{
    unsigned long ibytes = 0, obytes = 0;
    unsigned long ibytes_overflows = 0, obytes_overflows = 0;

    if (idx < 0)						/* PPPoE    */
    {
	if (PPPOE_ONLINE_START)
	{
	    ibytes = (unsigned long)(PPPOE_IBYTES);
	    ibytes_overflows = (unsigned long)(PPPOE_IBYTES >> 32);

	    obytes = (unsigned long)(PPPOE_OBYTES);
	    obytes_overflows = (unsigned long)(PPPOE_OBYTES >> 32);
	}
    }
    else
    {
	int	channel_idx;

	channel_idx = map_channel (idx);

	if (channel_idx >= 0)
	{
	    ibytes = (unsigned long)(ISDN_IBYTES);
	    ibytes_overflows = (unsigned long)(ISDN_IBYTES >> 32);

	    obytes = (unsigned long)(ISDN_OBYTES);
	    obytes_overflows = (unsigned long)(ISDN_OBYTES >> 32);
	}
    }
    sprintf (answer, "OK %lu %lu %lu %lu\r\n",
	     ibytes_overflows, ibytes, obytes_overflows, obytes);

    return (answer);
} /* cmd_quantity (answer, idx) */


/*----------------------------------------------------------------------------
 * cmd_rate ()				    - print current rate
 *----------------------------------------------------------------------------
 */
char *
cmd_rate (char * answer, int idx)
{
    if (idx < 0)						/* PPPoE    */
    {
	sprintf (answer, "OK %d %d\r\n",
		 PPPOE_IBYTES_PER_SECOND, PPPOE_OBYTES_PER_SECOND);
    }
    else
    {
	int	channel_idx;

	channel_idx = map_channel (idx);

	if (channel_idx < 0)
	{
	    strcpy (answer, "OK 0 0\r\n");
	}
	else
	{
	    sprintf (answer, "OK %d %d\r\n",
		     ISDN_IBYTES_PER_SECOND, ISDN_OBYTES_PER_SECOND);
	}
    }
    return (answer);
} /* cmd_rate (answer, idx) */


/*----------------------------------------------------------------------------
 * password_valid ()			- check password
 *----------------------------------------------------------------------------
 */
static int
password_valid (char * arg)
{
    char *	    pass;
    struct  spwd *  sp;

    sp = getspnam ("root");

    if (! sp)
    {
	return (-1);
    }

    pass = sp->sp_pwdp;

    if (pass && *pass)
    {
	if (! strcmp (pass, crypt (arg, pass)))
	{
	    return (TRUE);
	}
    }
    return (FALSE);
} /* password_valid (arg) */

/*----------------------------------------------------------------------------
 * receive_file ()			- receive a file
 *
 * file transfer:
 *
 *  command: receive filename nbytes password
 *  send ACK or NAK
 *  if ACK:
 *	repeat:
 *	    receive raw data in 1024 byte blocks (or rest)
 *	    send ACK or NAK
 *	end-repeat
 *  end-if
 *  send OK\r\n or ERR\r\n
 *----------------------------------------------------------------------------
 */
static int
receive_file (int client_idx, char * filename, long nbytes)
{
    char    buffer[TRANSFER_BUFFER_SIZE];
    FILE *  fp;
    size_t  bytes_to_receive;
    size_t  bytes_read;
    size_t  bytes_sum;

    fp = fopen (filename, "w");

    if (! fp)
    {
	write (client_fd[client_idx], NAK_STRING, 1);
	return (ERR);
    }

    while (nbytes > 0)
    {
	write (client_fd[client_idx], ACK_STRING, 1);

	bytes_sum = 0;

	if (nbytes < TRANSFER_BUFFER_SIZE)
	{
	    bytes_to_receive = nbytes;
	    nbytes = 0;
	}
	else
	{
	    bytes_to_receive = TRANSFER_BUFFER_SIZE;
	    nbytes -= TRANSFER_BUFFER_SIZE;
	}

	do
	{
	    bytes_read = read (client_fd[client_idx], buffer,
			       bytes_to_receive - bytes_sum);

	    if (bytes_read <= 0 ||
		fwrite (buffer, bytes_read, 1, fp) != 1)
	    {
		fclose (fp);
		write (client_fd[client_idx], NAK_STRING, 1);
		return (ERR);
	    }

	    bytes_sum += bytes_read;
	} while (bytes_sum < bytes_to_receive);
    }

    (void) fclose (fp);
    (void) write (client_fd[client_idx], ACK_STRING, 1);
    return (OK);
} /* receive_file (client_idx, filename, nbytes) */


/*----------------------------------------------------------------------------
 * send_file ()			    - send a file
 *
 * file transfer:
 *
 *  command: send filename password
 *
 *  if filename ok:
 *	send "OK len-in-bytes\r\n"
 *	repeat:
 *	    send raw data in 1024 byte blocks (or rest)
 *	    receive ACK or NAK
 *	end-repeat
 *  end-if
 *  send OK\r\n or ERR\r\n
 *----------------------------------------------------------------------------
 */
static int
send_file (int client_idx, char * filename)
{
    char	buffer[TRANSFER_BUFFER_SIZE];
    long	nbytes;
    long	bytes_to_send;
    struct stat	st;
    FILE *	fp;
    int		rtc = OK;

    if (stat (filename, &st) != 0)
    {
	return (ERR);
    }

    fp = fopen (filename, "r");

    if (! fp)
    {
	return (ERR);
    }

    nbytes = st.st_size;

    sprintf (buffer, "OK %ld\r\n", nbytes);
    write (client_fd[client_idx], buffer, strlen (buffer));

    while (nbytes > 0)
    {
	if (nbytes >= TRANSFER_BUFFER_SIZE)
	{
	    bytes_to_send = TRANSFER_BUFFER_SIZE;
	}
	else
	{
	    bytes_to_send = nbytes;
	}

	/* error handling missing here, later */
	(void) fread (buffer, bytes_to_send, 1, fp);
	(void) write (client_fd[client_idx], buffer, bytes_to_send);
	(void) read (client_fd[client_idx], buffer, 1);

	if (*buffer == NAK_CHAR)
	{
	    rtc = ERR;
	    break;
	}

	nbytes -= bytes_to_send;
    }

    (void) fclose (fp);
    return (rtc);
} /* send_file (client_idx, filename, nbytes) */

#ifdef DEBUG_ROUTES
void show_routing (int wait, char * info)
{
    char buf [1024];
    int i;
    strcpy (buf, "/usr/local/bin/route_check.sh ");
    if (wait)
    {
	strcat (buf, "-wait");
    }
    sprintf (buf+strlen (buf), " '%s' ", info);
    for (i=0; i<n_circuits; i++)
    {
	sprintf (buf+strlen (buf), " %s ", circuits[i].device_1);
    }
    if (wait)
    {
	strcat (buf, " &");
    }
    my_system (buf);
}
#endif


/*----------------------------------------------------------------------------
 * print_status ()			    - print current status
 *----------------------------------------------------------------------------
 */
static char *
print_status (int client_idx, char * buf)
{
    static char	answer[4096];
    char *  log_file;
    char *  arg;
    char *  p;
    int	    len;
    int	    idx;				    /* first argument	    */
    int	    value;				    /* optional second arg  */
    int	    channel_idx;
    int	    circuit_idx;
    int	    rtc;

    strcpy (answer, "ERR\r\n");			    /* assume error	    */

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

    len = strlen (buf);

    if (len > 0)
    {
	p = buf + len - 1;

	while (*p == ' ')
	{
	    *p = '\0';
	    p--;
	}
    }

    arg = strchr (buf, ' ');

    if (arg)
    {
	*arg = '\0';
	arg++;

	while (*arg == ' ')
	{
	    arg++;
	}

	idx = atoi (arg);

	p = strchr (arg, ' ');

	if (p)
	{
	    while (*p == ' ')
	    {
		p++;
	    }

	    value = atoi (p);
	}
	else
	{
	    value   = -1;
	}
    }
    else
    {
	idx	= -1;
	value	= -1;
    }

    /* 1st: commands that need a non-numerical value and no password: */

    if (! strcmp (buf, "help") && arg)
    {
	cmd_help (answer, arg, client_idx);
	return (answer);
    }

    if (! strcmp (buf, "pass"))
    {
	cmd_pass (answer, arg, client_idx);
	return (answer);
    }

    if (! strcmp (buf, "md5pass"))
    {
	cmd_md5pass (answer, arg, client_idx);
	return (answer);
    }

    if (! arg)
    {
	/* here the commands which need no password: */

	if (! strcmp (buf, "version"))
	{
	    sprintf (answer, "OK %d %s\r\n", PROTOCOL_VERSION, IMOND_VERSION);
	    return (answer);
	}

	if (! strcmp (buf, "quit"))
	{
	    return ((char *) NULL);
	}

	if (! strcmp (buf, "salt"))
	{
	    cmd_salt(answer, client_idx);
	    return (answer);
	}

	/* block following commands if password required: */
	if ((password_mode & PASS_REQUIRED) != 0 &&
	    (password_state[client_idx] & PASSWORD_STATE_NORMAL) == 0)
	{
	    strcpy (answer, "ERR password required\r\n");
	    return (answer);
	}

	if (! strcmp (buf, "is-enabled"))
	{
	    sprintf (answer, "OK %d\r\n", dialmode ? 1 : 0);
	    return (answer);
	}

	if (! strcmp (buf, "fli4l-id"))
	{
	    FILE *  fp;
	    int	    fli4l_id = 0;

	    fp = fopen ("/var/run/fli4l-id", "r");

	    if (fp)
	    {
		(void) fscanf (fp, "%d", &fli4l_id);
		fclose (fp);
	    }

	    sprintf (answer, "OK %d\r\n", fli4l_id);
	    return (answer);
	}

	if (! strcmp (buf, "highscore"))
	{
	    FILE *  fp;
	    int	    highscore = 0;

	    fp = fopen ("/var/run/highscore", "r");

	    if (fp)
	    {
		(void) fscanf (fp, "%d", &highscore);
		fclose (fp);
	    }

	    sprintf (answer, "OK %d\r\n", highscore);
	    return (answer);
	}

	if (! strcmp (buf, "hostname"))
	{
	    struct utsname  utsbuf;

	    if (uname (&utsbuf) == 0)
	    {
		sprintf (answer, "OK %s\r\n", utsbuf.nodename);
	    }
	    return (answer);
	}

	if (! strcmp (buf, "dialmode"))
	{
	    char * dialmode_string;

	    switch (dialmode)
	    {
		case DIALMODE_OFF:	dialmode_string = "off";    break;
		case DIALMODE_AUTO:	dialmode_string = "auto";   break;
		default:		dialmode_string = "manual"; break;
	    }

	    sprintf (answer, "OK %s\r\n", dialmode_string);
	    return (answer);
	}

	if (! strcmp (buf, "channels"))
	{
#ifdef TEST
	    strcpy (answer, "OK 2\r\n");
#else
	    sprintf (answer, "OK %d\r\n", n_active_channels);
#endif
	    return (answer);
	}

	if (! strcmp (buf, "timetable"))
	{
	    strcpy (answer, get_timetable (-1));
	    strcat (answer, "OK\r\n");
	    return (answer);
	}

	if (! strcmp (buf, "circuits"))
	{
	    sprintf (answer, "OK %d\r\n", n_circuits);
	    return (answer);
	}

	if (! strcmp (buf, "date"))
	{
	    struct tm *	tm;
	    time_t	seconds;
	    seconds = time ((time_t *) 0);

	    tm = localtime (&seconds);

	    sprintf (answer, "OK %s %02d/%02d/%4d %02d:%02d:%02d\r\n",
		     en_wday_strings[tm->tm_wday], tm->tm_mday, tm->tm_mon + 1,
		     1900 + tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
	    return (answer);
	}

	if (! strcmp (buf, "route"))
	{					    /* -1 -> 0 : automatic  */
	    sprintf (answer, "OK %d\r\n", route_idx + 1);
	    return (answer);
	}

	if (! strcmp (buf, "pppoe"))
	{
	    sprintf (answer, "OK %d\r\n", pppoe_used ? 1 : 0);
	    return (answer);
	}

	if (! strcmp (buf, "uptime"))
	{
	    FILE *	    uptime_fp;
	    static long     last_uptime_tv_sec;
	    static long     last_uptime = 0;
	    int 	    TimeDiff;

	    TimeDiff=current_time.tv_sec - last_uptime_tv_sec;

	    if ( TimeDiff > UPTIME_MEASURE_TIME)
	    {
		uptime_fp = fopen ("/proc/uptime", "r");

		if (uptime_fp)
		{
		    if (fscanf (uptime_fp, "%ld", &last_uptime) == 1)
		    {
			sprintf(answer, "OK %ld\r\n", last_uptime);
		    }
		    fclose(uptime_fp);
		}

		last_uptime_tv_sec = current_time.tv_sec;
	    }
	    else				/* not enough time passed   */
	    {					/* return last value	    */
		if (current_time.tv_sec < last_uptime_tv_sec)
		{				/* sometimes???		    */
		    last_uptime_tv_sec = current_time.tv_sec;
		}

		sprintf (answer, "OK %ld\r\n", last_uptime + TimeDiff);
	    }
	    return (answer);
	}

	if (! strcmp (buf, "cpu"))
	{
	    FILE *	    cpu_fp;
	    static long	    percentage;
	    static long long int last_busy = -1;
	    static long long int last_total = -1;
	    static long long int last_cpu_tv_sec;
	    long long int        user;
	    long long int        nice;
	    long long int        system;
	    long long int        idle;
	    long long int        iowait;
	    long long int        irq;
	    long long int        softirq;
	    long long int        steal;
	    long long int        busy;
	    long long int        total;

	    if (current_time.tv_sec - last_cpu_tv_sec > CPU_MEASURE_TIME)
	    {
		cpu_fp = fopen ("/proc/stat", "r");

		if (cpu_fp)
		{
		    if (fscanf (cpu_fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice,
				&system, &idle, &iowait, &irq, &softirq, &steal) == 8)
		    {
			total = user + nice + system + idle + iowait + irq + softirq + steal;
			busy = total - idle - iowait;

			if (last_total > 0 && (total - last_total) > 0)
			{
			    percentage = (100L * (busy - last_busy)) /
					 (total - last_total);
			    sprintf (answer, "OK %ld\r\n", percentage);
			}
			else
			{
			    strcpy (answer, "OK 0\r\n");
			}

			last_busy = busy;
			last_total = total;
		    }
		    fclose (cpu_fp);
		}

		last_cpu_tv_sec = current_time.tv_sec;
	    }
	    else				/* not enough time passed   */
	    {					/* return last value	    */
		if (current_time.tv_sec < last_cpu_tv_sec)  /* sometimes??? */
		{
		    last_cpu_tv_sec = current_time.tv_sec;
		}

		sprintf (answer, "OK %ld\r\n", percentage);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "telmond-log-file"))
	{
	    log_file = get_telmond_log_file (client_idx);

	    if (log_file)
	    {
		FILE *	tmp_fp = fopen (log_file, "r");
		char	tmp_buf[512];

		if (tmp_fp)
		{
		    tmp_buf[0] = ' ';

		    while (fgets (tmp_buf + 1, 511, tmp_fp))
		    {
			len = strlen (tmp_buf);
			*(tmp_buf + len - 1) = '\r';
			*(tmp_buf + len)     = '\n';
			*(tmp_buf + len + 1) = '\0';

			len++;

			if (write (client_fd[client_idx], tmp_buf, len) != len)
			{
			    close (client_fd[client_idx]);
			    client_fd[client_idx] = -1;
			    break;
			    fclose (tmp_fp);
			    return ((char *) NULL);
			}
		    }

		    fclose (tmp_fp);
		}
	    }

	    strcpy (answer, "OK 0\r\n");	    /* always return OK	    */
	    return (answer);
	}

	if (! strcmp (buf, "imond-log-file"))
	{
	    if (*imond_log_file)
	    {
		FILE *	tmp_fp = fopen (imond_log_file, "r");
		char	tmp_buf[512];

		if (tmp_fp)
		{
		    tmp_buf[0] = ' ';

		    while (fgets (tmp_buf + 1, 511, tmp_fp))
		    {
			len = strlen (tmp_buf);
			*(tmp_buf + len - 1) = '\r';
			*(tmp_buf + len)     = '\n';
			*(tmp_buf + len + 1) = '\0';

			len++;

			if (write (client_fd[client_idx], tmp_buf, len) != len)
			{
			    close (client_fd[client_idx]);
			    client_fd[client_idx] = -1;
			    break;
			    fclose (tmp_fp);
			    return ((char *) NULL);
			}
		    }
		    fclose (tmp_fp);
		}
	    }

	    strcpy (answer, "OK 0\r\n");	    /* always return OK	    */
	    return (answer);
	}

	if (! strcmp (buf, "mgetty-log-file"))
	{
	    if (*mgetty_log_file)
	    {
		FILE *	tmp_fp = fopen (mgetty_log_file, "r");
		char	tmp_buf[512];

		if (tmp_fp)
		{
		    tmp_buf[0] = ' ';

		    while (fgets (tmp_buf + 1, 511, tmp_fp))
		    {
			len = strlen (tmp_buf);

			if (*(tmp_buf + len - 1) == '\n')
			{
			    *(tmp_buf + len - 1) = '\r';
			    *(tmp_buf + len)     = '\n';
			    *(tmp_buf + len + 1) = '\0';
			}
			else 
			{
			    *(tmp_buf + len)     = '\r';
			    *(tmp_buf + len + 1) = '\n';
			    *(tmp_buf + len + 2) = '\0';
			    len++;
			}

			len++;

			if (write (client_fd[client_idx], tmp_buf, len) != len)
			{
			    close (client_fd[client_idx]);
			    client_fd[client_idx] = -1;
			    break;
			    fclose (tmp_fp);
			    return ((char *) NULL);
			}
		    }
		    fclose (tmp_fp);
		}
	    }

	    strcpy (answer, "OK 0\r\n");	    /* always return OK	    */
	    return (answer);
	}

	if (! strcmp (buf, "client-ip"))
	{
	    sprintf (answer, "OK %s\r\n", client_ip[client_idx]);
	    return (answer);
	}

	if (! strcmp (buf, "enable"))
	{
	    char * may_enable;

	    if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
	    {
		may_enable = "yes";
	    }
	    else
	    {
		may_enable = getenv ("IMOND_ENABLE");
	    }

	    if (may_enable && ! strcmp (may_enable, "yes"))
	    {
		rtc = set_dialmode (DIALMODE_AUTO);
		sprintf (answer, "OK %d\r\n", rtc);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "disable"))
	{
	    char * may_disable;

	    if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
	    {
		may_disable = "yes";
	    }
	    else
	    {
		may_disable = getenv ("IMOND_ENABLE");
	    }

	    if (may_disable && ! strcmp (may_disable, "yes"))
	    {
		rtc = set_dialmode (DIALMODE_OFF);
		sprintf (answer, "OK %d\r\n", rtc);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "reboot") || ! strcmp (buf, "halt") || ! strcmp (buf, "poweroff"))
	{
	    char * may_reboot;

	    if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
	    {
		may_reboot = "yes";
	    }
	    else
	    {
		may_reboot = getenv ("IMOND_REBOOT");
	    }

	    if (may_reboot && ! strcmp (may_reboot, "yes"))
	    {
		(void) set_dialmode (DIALMODE_OFF);	/* dialmode off	    */
		(void) do_dial ((char *) NULL, FALSE);	/* hangup conections*/

		if (! strcmp (buf, "reboot"))
		{
		    rtc = my_system ("/sbin/reboot -d 3 &");
		}
		else if (! strcmp (buf, "poweroff"))
		{
		    rtc = my_system ("(sleep 3; /sbin/poweroff) &");
		}
		else
		{
		    rtc = my_system ("(sleep 3; /sbin/halt) &");
		}

		sprintf (answer, "OK %d\r\n", rtc);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "dial"))
	{
	    char * may_dial;

	    if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
	    {
		may_dial = "yes";
	    }
	    else
	    {
		may_dial = getenv ("IMOND_DIAL");
	    }

	    if (may_dial && ! strcmp (may_dial, "yes"))
	    {
		rtc = do_dial ((char *) NULL, TRUE);
		sprintf (answer, "OK %d\r\n", rtc);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "hangup"))
	{
	    char * may_hangup;

	    if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
	    {
		may_hangup = "yes";
	    }
	    else
	    {
		may_hangup = getenv ("IMOND_DIAL");
	    }

	    if (may_hangup && ! strcmp (may_hangup, "yes"))
	    {
		rtc = do_dial ((char *) NULL, FALSE);
		sprintf (answer, "OK %d\r\n", rtc);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "help"))
	{
	    cmd_help (answer, (char *) NULL, client_idx);
	    return (answer);
	}

	/* block following commands if admin password required: */
	if ((password_mode & ADMIN_PASS_REQUIRED) != 0 &&
	    (password_state[client_idx] & PASSWORD_STATE_ADMIN) == 0)
	{
	    strcpy (answer, "ERR administrator password required\r\n");
	    return (answer);
	}

	if (! strcmp (buf, "sync"))
	{
	    if (fork () == 0)
	    {
		/* Execute sync call as child process to prevent lockups */
		sync ();
		exit (0);
	    }
	    strcpy (answer, "OK\r\n");
	    return (answer);
	}

	if (! strcmp (buf, "reset-telmond-log-file"))
	{
	    log_file = get_telmond_log_file (client_idx);

	    if (log_file && access (log_file, 0) == 0)
	    {
		FILE * tmp_fp = fopen (log_file, "w");

		if (tmp_fp)
		{
		    fclose (tmp_fp);
		}
	    }
	    strcpy (answer, "OK 0\r\n");	    /* always return OK	    */
	    return (answer);
	}

	if (! strcmp (buf, "reset-imond-log-file"))
	{
	    imond_reset_log ();
	    strcpy (answer, "OK 0\r\n");	    /* always return OK	    */
	    return (answer);
	}
    }
    else
    {
	/* here the commands with arguments which need no password: */

	/* ... yet no commands ... */

	/* block following commands if password required: */
	if ((password_mode & PASS_REQUIRED) != 0 &&
	    (password_state[client_idx] & PASSWORD_STATE_NORMAL) == 0)
	{
	    strcpy (answer, "ERR password required\r\n");
	    return (answer);
	}

	if (! strcmp (buf, "driverid"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    strcpy (answer, "OK PPPoE\r\n");
		}
	    }
	    else
	    {
		channel_idx = map_channel (idx);

		if (channel_idx >= 0)
		{
		    sprintf (answer, "OK %s\r\n", ISDN_DRVID);
		}
	    }
	    return (answer);
	}

	if (! strcmp (buf, "hangup"))
	{
	    char * may_hangup;

	    if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
	    {
		may_hangup = "yes";
	    }
	    else
	    {
		may_hangup = getenv ("IMOND_DIAL");
	    }

	    if (may_hangup && ! strcmp (may_hangup, "yes"))
	    {
		rtc = do_dial (arg, FALSE);
		sprintf (answer, "OK %d\r\n", rtc);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "client-ip"))
	{
	    if (! strcmp (client_ip[client_idx], "127.0.0.1"))
	    {
		strncpy (client_ip[client_idx], arg, 15);
		client_ip[client_idx][15] = '\0';
		sprintf (answer, "OK %s\r\n", client_ip[client_idx]);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "status"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    sprintf (answer, "OK %s\r\n", PPPOE_STATUS);
		}
	    }
	    else
	    {
		channel_idx = map_channel (idx);

		if (channel_idx >= 0)
		{
		    sprintf (answer, "OK %s\r\n", ISDN_STATUS);
		}
	    }

	    return (answer);
	}

	if (! strcmp (buf, "phone"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    int	    conf_idx    = PPPOE_CONF_IDX;
		    int	    circuit_idx = CONF_CIRCUIT_IDX;

		    if (circuit_idx >= 0)
		    {
			sprintf (answer, "OK %s\r\n",
				 circuits[circuit_idx].name);
		    }
		    else
		    {
			strcpy (answer, "OK <unknown>\r\n");
		    }
		}
	    }
	    else
	    {
		channel_idx = map_channel (idx);

		if (channel_idx >= 0)
		{
		    char *  phone_info = ISDN_PHONE;

		    if (ISDN_ONLINE_START != 0 && ISDN_CONF_IDX >= 0)
		    {		/* we are online and found a conf_data line */
			int conf_idx = ISDN_CONF_IDX;

			strncpy (phone_info,
				 get_circuit_name_by_conf_idx (conf_idx), 31);
			*(phone_info + 31) = '\0';
		    }

		    sprintf (answer, "OK %s\r\n", phone_info);
		}
	    }
	    return (answer);
	}

	if (! strcmp (buf, "online-time"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    int	    seconds;
		    int	    minutes;
		    int	    hours;

		    if (PPPOE_ONLINE_START > 0)
		    {
			seconds = time ((time_t *) NULL) - PPPOE_ONLINE_START;
		    }
		    else
		    {
			seconds = 0;
		    }

		    hours = seconds / 3600;
		    seconds -= (hours * 3600);

		    minutes = seconds / 60;
		    seconds -= (minutes * 60);

		    sprintf (answer, "OK %02d:%02d:%02d\r\n",
			     hours, minutes, seconds);
		}
	    }
	    else
	    {
		channel_idx = map_channel (idx);

		if (channel_idx >= 0)
		{
		    int	    seconds;
		    int	    minutes;
		    int	    hours;

		    if (ISDN_ONLINE_START > 0)
		    {
			seconds = time ((time_t *) NULL) - ISDN_ONLINE_START;
		    }
		    else
		    {
			seconds = 0;
		    }

		    hours = seconds / 3600;
		    seconds -= (hours * 3600);

		    minutes = seconds / 60;
		    seconds -= (minutes * 60);

		    sprintf (answer, "OK %02d:%02d:%02d\r\n",
			     hours, minutes, seconds);
		}
	    }
	    return (answer);
	}

	if (! strcmp (buf, "time"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    int	    seconds = (int) PPPOE_ONLINE_TIME;
		    int	    minutes;
		    int	    hours;

		    if (PPPOE_ONLINE_START > 0)
		    {
			seconds += time ((time_t *) NULL) - PPPOE_ONLINE_START;
		    }

		    hours = seconds / 3600;
		    seconds -= (hours * 3600);

		    minutes = seconds / 60;
		    seconds -= (minutes * 60);

		    sprintf (answer, "OK %02d:%02d:%02d\r\n",
			     hours, minutes, seconds);
		}
	    }
	    else
	    {
		channel_idx = map_channel (idx);

		if (channel_idx >= 0)
		{
		    int	    seconds = (int) ISDN_ONLINE_TIME;
		    int	    minutes;
		    int	    hours;

		    if (ISDN_ONLINE_START > 0)
		    {
			seconds += time ((time_t *) NULL) - ISDN_ONLINE_START;
		    }

		    hours = seconds / 3600;
		    seconds -= (hours * 3600);

		    minutes = seconds / 60;
		    seconds -= (minutes * 60);

		    sprintf (answer, "OK %02d:%02d:%02d\r\n",
			     hours, minutes, seconds);
		}
	    }
	    return (answer);
	}

	if (! strcmp (buf, "chargetime"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    int	    seconds = (int) PPPOE_CHARGE_TIME;
		    int	    minutes;
		    int	    hours;

		    if (PPPOE_ONLINE_START > 0)
		    {
			int conf_idx	= PPPOE_CONF_IDX;
			int charge_int	= CONF_CHARGE_INTERVAL;
			int diff	= time ((time_t *) NULL) -
					    PPPOE_ONLINE_START;

			if (charge_int > 1)
			{
			    diff = (diff / charge_int) * charge_int +
				    charge_int;
			}

			seconds += diff;
		    }

		    hours = seconds / 3600;
		    seconds -= (hours * 3600);

		    minutes = seconds / 60;
		    seconds -= (minutes * 60);

		    sprintf (answer, "OK %02d:%02d:%02d\r\n",
			     hours, minutes, seconds);
		}
	    }
	    else
	    {
		channel_idx = map_channel (idx);

		if (channel_idx >= 0)
		{
		    int	    seconds = (int) ISDN_CHARGE_TIME;
		    int	    minutes;
		    int	    hours;

		    if (ISDN_IS_OUTGOING_DIR)
		    {
			if (ISDN_ONLINE_START > 0)
			{
			    int conf_idx    = ISDN_CONF_IDX;
			    int charge_int  = CONF_CHARGE_INTERVAL;
			    int diff	    = time ((time_t *) NULL) - ISDN_ONLINE_START;

			    if (charge_int > 1)
			    {
				diff = (diff / charge_int) * charge_int +
					charge_int;
			    }

			    seconds += diff;
			}
		    }

		    hours = seconds / 3600;
		    seconds -= (hours * 3600);

		    minutes = seconds / 60;
		    seconds -= (minutes * 60);

		    sprintf (answer, "OK %02d:%02d:%02d\r\n",
			     hours, minutes, seconds);
		    return (answer);
		}
	    }
	    return (answer);
	}

	if (! strcmp (buf, "charge"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    float   charge  = PPPOE_CHARGE;

		    if (PPPOE_ONLINE_START > 0)
		    {
			int conf_idx	= PPPOE_CONF_IDX;
			int charge_int	= CONF_CHARGE_INTERVAL;
			int diff	= time ((time_t *) NULL) -
					    PPPOE_ONLINE_START;

			if (charge_int > 1)
			{
			    diff = (diff / charge_int) * charge_int +
				    charge_int;
			}

			if (conf_idx >= 0)
			{
			    charge += (diff * CONF_CHARGE_PER_MINUTE) / 60;
			}
		    }

		    sprintf (answer, "OK %0.2f\r\n", charge);
		}
	    }
	    else
	    {
		channel_idx = map_channel (idx);

		if (channel_idx >= 0)
		{
		    float   charge  = ISDN_CHARGE;

		    if (ISDN_IS_OUTGOING_DIR)
		    {
			if (ISDN_ONLINE_START > 0)
			{
			    int conf_idx    = ISDN_CONF_IDX;
			    int charge_int  = CONF_CHARGE_INTERVAL;
			    int diff	    = time ((time_t *) NULL) - ISDN_ONLINE_START;

			    if (charge_int > 1)
			    {
				diff = (diff / charge_int) * charge_int +
					charge_int;
			    }

			    if (conf_idx >= 0)
			    {
				charge += (diff * CONF_CHARGE_PER_MINUTE) / 60;
			    }
			}
		    }

		    sprintf (answer, "OK %0.2f\r\n", charge);
		}
	    }
	    return (answer);
	}

	if (! strcmp (buf, "usage"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    strcpy (answer, "OK Net\r\n");
		}
	    }
	    else
	    {
		channel_idx = map_channel (idx);

		if (channel_idx >= 0)
		{
		    sprintf (answer, "OK %s\r\n", ISDN_USAGE);
		    return (answer);
		}
	    }
	    return (answer);
	}

	if (! strcmp (buf, "inout"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    strcpy (answer, "OK Outgoing\r\n");
		}
	    }
	    else
	    {
		channel_idx = map_channel (idx);

		if (channel_idx >= 0)
		{
		    sprintf (answer, "OK %s\r\n", ISDN_INOUT);
		}
	    }
	    return (answer);
	}

	if (! strcmp (buf, "ip"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    cmd_ip (answer, PPPOE_PSEUDO_IDX);
		}
	    }
	    else
	    {
		cmd_ip (answer, idx);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "quantity"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    cmd_quantity (answer, -1);
		}
	    }
	    else
	    {
		cmd_quantity (answer, idx);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "rate"))
	{
	    if (! strcmp (arg, "pppoe"))
	    {
		if (pppoe_used)
		{
		    cmd_rate (answer, -1);
		}
	    }
	    else
	    {
		cmd_rate (answer, idx);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "circuit"))
	{
	    circuit_idx = idx - 1;

	    if (circuit_idx >= 0 && circuit_idx < n_circuits)
	    {
		sprintf (answer, "OK %s\r\n", circuits[circuit_idx].name);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "device"))
	{
	    circuit_idx = idx - 1;

	    if (circuit_idx >= 0 && circuit_idx < n_circuits)
	    {
		sprintf (answer, "OK %s\r\n", circuits[circuit_idx].device_1);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "links"))
	{
	    circuit_idx = idx - 1;

	    if (circuit_idx < 0)		/* use automatic circuit    */
	    {
		circuit_idx = find_circuit ();		/* get circuit_idx  */
	    }

	    return (answer);
	}

	if (! strcmp (buf, "timetable"))
	{
	    int conf_idx = idx - 1;

	    if (conf_idx < n_conf_lines_used)
	    {
		strcpy (answer, get_timetable (conf_idx));
	    }
	    else
	    {
		*answer = '\0';
	    }

	    strcat (answer, "OK\r\n");
	    return (answer);
	}

	if (! strcmp (buf, "log-dir"))
	{
	    char * Directory= (char *) NULL;	
	    
	    if (! strcmp (arg, "imond"))
	    {
		Directory = imond_log_dir;
	    } 
	    else if (! strcmp (arg, "telmond"))
	    {
		Directory = telmond_log_dir;
	    }
	    else if (! strcmp (arg, "mgetty"))
	    {
		Directory = mgetty_log_dir;
	    }
	    
	    sprintf (answer, "OK %s\r\n", Directory);
	    return (answer);
	}    
		
	if (! strcmp (buf, "is-allowed"))
	{
	    char *  may_do = (char *) NULL;

	    if (! strcmp (arg, "imond-log"))
	    {
		may_do = (*imond_log_file) ? "yes" : "no";
	    }
	    else if (! strcmp (arg, "telmond-log"))
	    {
		may_do = telmond_log_dir ? "yes" : "no";
	    }
	    else if (! strcmp (arg, "mgetty-log"))
	    {
		may_do = (*mgetty_log_file) ? "yes" : "no";
	    }
	    else if (! strcmp (arg, "dial") || ! strcmp (arg, "hangup"))
	    {
		if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
		{
		    may_do = "yes";
		}
		else
		{
		    may_do = getenv ("IMOND_DIAL");
		}
	    }
	    else if (! strcmp (arg, "dialmode"))
	    {
		if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
		{
		    may_do = "yes";
		}
		else
		{
		    may_do = getenv ("IMOND_ENABLE");
		}
	    }
	    else if (! strcmp (arg, "route"))
	    {
		if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
		{
		    may_do = "yes";
		}
		else
		{
		    may_do = getenv ("IMOND_ROUTE");
		}
	    }
	    else if (! strcmp (arg, "reboot") || ! strcmp (arg, "halt") || ! strcmp (arg, "poweroff"))
	    {
		if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
		{
		    may_do = "yes";
		}
		else
		{
		    may_do = getenv ("IMOND_REBOOT");
		}
	    }

	    if (may_do && ! strcmp (may_do, "yes"))
	    {
		rtc = 1;
	    }
	    else
	    {
		rtc = 0;
	    }

	    sprintf (answer, "OK %d\r\n", rtc);
	    return (answer);
	}

	if (! strcmp (buf, "dialmode"))
	{
	    char * may_enable;

	    if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
	    {
		may_enable = "yes";
	    }
	    else
	    {
		may_enable = getenv ("IMOND_ENABLE");
	    }

	    if (may_enable && ! strcmp (may_enable, "yes"))
	    {
		if (! strcmp (arg, "off"))
		{
		    rtc = set_dialmode (DIALMODE_OFF);
		}
		else if (! strcmp (arg, "auto"))
		{
		    rtc = set_dialmode (DIALMODE_AUTO);
		}
		else if (! strcmp (arg, "manual"))
		{
		    rtc = set_dialmode (DIALMODE_MANUAL);
		}
		else
		{
		    return (answer);
		}

		sprintf (answer, "OK %d\r\n", rtc);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "route"))
	{
	    char * may_route;

	    if ((password_state[client_idx] & PASSWORD_STATE_ADMIN) != 0)
	    {
		may_route = "yes";
	    }
	    else
	    {
		may_route = getenv ("IMOND_ROUTE");
	    }

	    if (may_route && ! strcmp (may_route, "yes"))
	    {
		circuit_idx = idx - 1;

		if (circuit_idx >= -1 && circuit_idx < n_circuits)
		{				    /* -1: automatic rtg    */
		    if (circuit_idx < 0)
		    {
			route_idx = -1;
#ifdef DEBUG_ROUTES
			imond_syslog (LOG_INFO, "route lcr\n");
#endif
		    }
		    else
		    {
			route_idx = circuit_idx;
#ifdef DEBUG_ROUTES
			imond_syslog (LOG_INFO, "route circuit %d (\"%s\")\n", 
				      circuit_idx, 
				      circuits[circuit_idx].device_1);
#endif
		    }

#ifdef DEBUG_ROUTES
		    imond_syslog (LOG_INFO, "checking routes\n");
		    show_routing (0, "before check_routing (SET_IF_CHANGED)");
#endif
		    check_routing (SET_IF_CHANGED);
#ifdef DEBUG_ROUTES
		    show_routing (1, "after check_routing (SET_IF_CHANGED)");
#endif
		    strcpy (answer, "OK\r\n");
		}
	    }

	    return (answer);
	}

	/* block following commands if admin password required: */
	if ((password_mode & ADMIN_PASS_REQUIRED) != 0 &&
	    (password_state[client_idx] & PASSWORD_STATE_ADMIN) == 0)
	{
	    strcpy (answer, "ERR administrator password required\r\n");
	    return (answer);
	}

	if (! strcmp (buf, "support"))
	{
	    int	pw_valid;

	    pw_valid = password_valid (arg);

	    if (pw_valid == TRUE)
	    {
		char	tmp_buf[512];
		FILE *	fp;
		char *	tmp_p;

		system ("/usr/local/bin/support.sh");

		fp = fopen ("/tmp/support.txt", "r");

		if (! fp)
		{
		    strcpy (answer, "OK cannot get support file\r\n");
		    return (answer);
		}

		tmp_buf[0] = ' ';		    /* one leading space    */
		tmp_p = tmp_buf + 1;

		while (fgets (tmp_p, 500, fp))
		{
		    len = strlen (tmp_buf);
		    *(tmp_buf + len - 1) = '\r';
		    *(tmp_buf + len)     = '\n';
		    *(tmp_buf + len + 1) = '\0';

		    len++;

		    (void) write (client_fd[client_idx], tmp_buf, len);
		}

		fclose (fp);
		unlink ("/tmp/support.txt");
		strcpy (answer, "OK\r\n");
		return (answer);
	    }
	    else if (pw_valid == -1)
	    {
		strcpy (answer, "OK cannot get passwd entry for root\r\n");
	    }
	    else
	    {
		strcpy (answer, "ERR password incorrect\r\n");
	    }
	    return (answer);
	}

	if (! strcmp (buf, "receive"))
	{
	    char    filename[128];
	    long    nbytes;
	    char    pw[64];
	    int	    pw_valid;

	    if (sscanf (arg, "%s %ld %s", filename, &nbytes, pw) != 3)
	    {
		write (client_fd[client_idx], NAK_STRING, 1);
		strcpy (answer, "ERR\r\n");
		return (answer);
	    }

	    pw_valid = password_valid (pw);

	    if (pw_valid == TRUE)
	    {
		if (receive_file (client_idx, filename, nbytes) == OK)
		{
		    strcpy (answer, "OK\r\n");
		}
	    }
	    else if (pw_valid == -1)
	    {
		write (client_fd[client_idx], NAK_STRING, 1);
		strcpy (answer, "ERR cannot get passwd entry for root\r\n");
	    }
	    else
	    {
		write (client_fd[client_idx], NAK_STRING, 1);
		strcpy (answer, "ERR password invalid\r\n");
	    }
	    return (answer);
	}

	if (! strcmp (buf, "send"))
	{
	    char    filename[128];
	    char    pw[64];
	    int	    pw_valid;

	    if (sscanf (arg, "%s %s", filename, pw) != 2)
	    {
		strcpy (answer, "ERR\r\n");
		return (answer);
	    }

	    pw_valid = password_valid (pw);

	    if (pw_valid == TRUE)
	    {
		if (send_file (client_idx, filename) == OK)
		{
		    strcpy (answer, "OK\r\n");
		}
	    }
	    else if (pw_valid == -1)
	    {
		strcpy (answer, "ERR cannot get passwd entry for root\r\n");
	    }
	    else
	    {
		strcpy (answer, "ERR password invalid\r\n");
	    }
	    return (answer);
	}

	if (! strcmp (buf, "delete"))
	{
	    char    filename[128];
	    char    pw[64];
	    int	    pw_valid;

	    if (sscanf (arg, "%s %s", filename, pw) != 2)
	    {
		strcpy (answer, "ERR\r\n");
		return (answer);
	    }

	    pw_valid = password_valid (pw);

	    if (pw_valid == TRUE)
	    {
		if (unlink (filename) == OK)
		{
		    strcpy (answer, "OK\r\n");
		}
	    }
	    else if (pw_valid == -1)
	    {
		strcpy (answer, "ERR cannot get passwd entry for root\r\n");
	    }
	    else
	    {
		strcpy (answer, "ERR password invalid\r\n");
	    }
	    return (answer);
	}

	if (! strcmp (buf, "set-status"))
	{
	    if (set_status (arg) == OK)
	    {
		strcpy (answer, "OK\r\n");
	    }

	    return (answer);
	}

	if (! strcmp (buf, "hup-timeout"))
	{
	    circuit_idx = idx - 1;

	    if (circuit_idx >= 0 && circuit_idx < n_circuits)
	    {
		if (value >= 0)		    /* 2nd argument ...		    */
		{
		    if (strcmp (circuits[circuit_idx].device_1, "pppoe") != 0)
		    {			    /* no timeout setting on pppoe  */
			char tmp_buf[128];

			sprintf (tmp_buf, "/sbin/isdnctrl huptimeout %s %d",
				 circuits[circuit_idx].device_1, value);

			rtc = my_system (tmp_buf);

			if (rtc == 0 && *(circuits[circuit_idx].device_2))
			{
			    sprintf (tmp_buf, "/sbin/isdnctrl huptimeout %s %d",
				     circuits[circuit_idx].device_2, value);

			    rtc = my_system (tmp_buf);
			}

			if (rtc == 0)
			{
			    circuits[circuit_idx].hup_timeout = value;
			}

			sprintf (answer, "OK %d\r\n",
				 circuits[circuit_idx].hup_timeout);
		    }
		}
		else			    /* no 2nd arg, send current val */
		{
		    sprintf (answer, "OK %d\r\n",
			     circuits[circuit_idx].hup_timeout);
		}
	    }

	    return (answer);
	}

	if (! strcmp (buf, "addlink") ||
		 ! strcmp (buf, "removelink"))
	{
	    circuit_idx = idx - 1;

	    if (circuit_idx < 0)			/* use automatic circuit    */
	    {
		circuit_idx = find_circuit ();			/* get circuit_idx  */
#ifdef TEST
		printf ("found circuit %d (%s) for channel-bundling\n",
			circuit_idx, circuit_idx >= 0 ? circuits[circuit_idx].name : "NOTHING");
#endif
	    }

	    if (circuit_idx >= 0 && circuit_idx < n_circuits)
	    {
		{
		    rtc = 0;			/* 0: no channel bundling   */
		}
		sprintf (answer, "OK %d\r\n", rtc);
	    }

	    return (answer);
	}

	if (! strcmp (buf, "adjust-time"))
	{
	    time_t  seconds;

	    seconds = time ((time_t *) NULL) + idx;

           if (value ==1)
               imond_syslog(LOG_INFO,
                            "adjusting imond time without setting system"
                            " time: %d seconds\n", idx);
           if (value == 1 || stime (&seconds) == 0)
	    {
		if (PPPOE_ONLINE_START != 0)		/* pppoe online?    */
		{
		    PPPOE_ONLINE_START += idx;
		}

		if (last_time_pppoe.tv_sec > 0)
		{
		    last_time_pppoe.tv_sec += idx;
		}

		strcpy (answer, "OK\r\n");
	    }

	    return (answer);
	}
    }

    strcpy (answer, "ERR\r\n");
    return (answer);
} /* print_status (client_idx, buf) */


/*----------------------------------------------------------------------------
 * fill_time_table ()			    - fill time tables
 *----------------------------------------------------------------------------
 */
static void
fill_time_table (int do_fill_global_table, int start_time, int end_time)
{
    int	i;

    if (do_fill_global_table)
    {
	for (i = start_time; i < end_time; i++)
	{
	    time_table[i] = n_conf_lines_used;
	    conf_data[n_conf_lines_used].time_table[i] = 1;
	}
    }
    else
    {
	for (i = start_time; i < end_time; i++)
	{
	    conf_data[n_conf_lines_used].time_table[i] = 1;
	}
    }

    return;
}


/*----------------------------------------------------------------------------
 * read_config ()			    - read configuration imond.conf
 *----------------------------------------------------------------------------
 */
static void
read_config (void)
{
    FILE *  fp;
    char    buf[256];
    int	    line;
    char    phone_buf[64];
    char    start_week_day[16];
    char    end_week_day[16];
    char    lcr[16];
    char    device[16];
    char *  p;
    char *  device_1;
    char *  device_2;
    char    default_route[16];
    char    circuit[32];
    int	    bandwidth_time;
    int	    bandwidth_rate;
    int	    is_lcr;
    int	    is_default_route;
    int	    day;
    int	    start_day;
    int	    end_day;
    int	    start_hour;
    int	    end_hour;
    int	    start_time;
    int	    end_time;
    int	    hup_timeout;
    int	    i;
    int	    j;

    for (i = 0; i < DAYS_PER_WEEK; i++)
    {
	for (j = 0; j < HOURS_PER_DAY; j++)
	{
	    time_table[i * HOURS_PER_DAY + j] = -1;
	}
    }

    fp = fopen (CONF_FILE, "r");

    if (! fp)
    {
	imond_syslog (LOG_WARNING, "%s: %s\n", CONF_FILE, strerror (errno));
	return;
    }

    for (line = 0, n_conf_lines_used = 0;
	 fgets (buf, 128, fp);
	 line++)
    {
	if (*buf == '#')
	{
	    continue;
	}

	if (sscanf (buf, "%2s-%2s %2d-%2d %s %d:%d %d %s %s %s %s %f %d",
		    start_week_day,
		    end_week_day,
		    &start_hour,
		    &end_hour,
		    device,
		    &bandwidth_rate,
		    &bandwidth_time,
		    &hup_timeout,
		    lcr,
		    default_route,
		    phone_buf,
		    circuit,
		    &(conf_data[n_conf_lines_used].charge_per_minute),
		    &(conf_data[n_conf_lines_used].charge_int)) != 14)
	{
	    imond_syslog (LOG_ERR, "%s: format error in line %d\n",
			  CONF_FILE, line + 1);
	    fprintf (stderr, "imond: %s: format error in line %d\n",
		     CONF_FILE, line + 1);
	    fclose (fp);
	    exit (1);
	}

	if (! strcmp (device, "pppoe"))
	{
	    strcpy (PPPOE_STATUS, OFFLINE_STATUS_STR);
	    conf_data[n_conf_lines_used].is_pppoe = TRUE;
	    pppoe_used = TRUE;
	}
	else
	{
	    conf_data[n_conf_lines_used].is_pppoe = FALSE;
	}

	p = strchr (phone_buf, ':');		    /* outgoings:incomings  */

	if (p)
	{
	    *p++ = '\0';
	    strncpy (conf_data[n_conf_lines_used].incoming_phone, p, 127);
	}

	strncpy (conf_data[n_conf_lines_used].outgoing_phone, phone_buf, 127);

	device_1 = device;
	device_2 = strchr (device, '/');

	if (device_2)
	{
	    *device_2++ = '\0';
	}

	for (i = 0; i < n_circuits; i++)
	{
	    if (! strcmp (circuits[i].device_1, device_1))
	    {
		break;
	    }
	}

	conf_data[n_conf_lines_used].circuit_idx = i;

	if (i == n_circuits)
	{
	    strcpy (circuits[i].name, circuit);
	    circuits[i].online = 0;
	    n_circuits++;
	}
	else
	{
	    if (strcmp (circuits[i].name, circuit) != 0)
	    {
		imond_syslog (LOG_WARNING,
			      "device %s has different circuit names: "
			      "%s and %s\n",
			      device, circuits[i].name, circuit);
	    }
	}

	strcpy (circuits[i].device_1, device_1);

	if (device_2)
	{
	    strcpy (circuits[i].device_2, device_2);
	}
	else
	{
	    *(circuits[i].device_2) = '\0';
	}

	circuits[i].bandwidth_time  = bandwidth_time;
	circuits[i].bandwidth_rate  = bandwidth_rate;
	circuits[i].hup_timeout	    = hup_timeout;

	if (! strcmp (lcr, "yes"))
	{
	    is_lcr = TRUE;
	}
	else
	{
	    is_lcr = FALSE;
	}

	if (! strcmp (default_route, "yes"))
	{
	    is_default_route = TRUE;
	}
	else
	{
	    is_default_route = FALSE;
	}

	conf_data[n_conf_lines_used].is_lcr		= is_lcr;
	conf_data[n_conf_lines_used].is_default_route	= is_default_route;

	start_day   = convert_week_day_to_day (start_week_day);
	end_day	    = convert_week_day_to_day (end_week_day);

	day = start_day;

	for (;;)
	{
	    if (start_hour > end_hour)
	    {
		start_time  = day * HOURS_PER_DAY + start_hour;
		end_time    = day * HOURS_PER_DAY + HOURS_PER_DAY;

		fill_time_table (is_lcr, start_time, end_time);

		start_time  = day * HOURS_PER_DAY + 0;
		end_time    = day * HOURS_PER_DAY + end_hour;

		fill_time_table (is_lcr, start_time, end_time);
	    }
	    else
	    {
		start_time  = day * HOURS_PER_DAY + start_hour;
		end_time    = day * HOURS_PER_DAY + end_hour;

		fill_time_table (is_lcr, start_time, end_time);
	    }

	    if (day == end_day)
	    {
		break;
	    }

	    day++;

	    if (day == DAYS_PER_WEEK)
	    {
		day = 0;
	    }
	}

	n_conf_lines_used++;
	if (n_conf_lines_used == MAX_CONFIG_LINES)
	{
	    imond_syslog (LOG_ERR, "maximal number of config lines reached"
			  " (%d), ignoring remaining entires\n", 
			  MAX_CONFIG_LINES);
	    break;
	}
    }

    fputs (get_timetable (-1), stdout);

    fclose (fp);
    return;
} /* read_config () */


/*----------------------------------------------------------------------------
 * read_line ()				    - read line from config file
 *----------------------------------------------------------------------------
 */
static int
read_line (int fd, char * buf, int maxlen)
{
    char *	p = buf;
    int		i;
    int		rtc;
    int		ret = 0;

    /* set alarm clock to be able to handle crashing or malicious clients */
    alarm_triggered = 0;
    alarm (5);
    for (p = buf, i = 1; i < maxlen; i++, p++)
    {
	rtc = read (fd, p, 1);

	if (rtc != 1)
	{
	    if (errno == EINTR && alarm_triggered)
	    {
		imond_syslog (LOG_INFO, "timeout while trying to read request "
			      "from client\n");
	    }
	    ret = -1;
	    *p = '\0';
	    break;
	}

	if (*p == '\n')
	{
	    break;
	}
    }
    /* stop alarm clock */
    alarm (0);

    p++;
    *p = '\0';
    return ret;
} /* read_line (fd, buf, maxlen) */


static struct ifreq pppoe_req;
static struct ppp_stats pppoe_stats;

/*----------------------------------------------------------------------------
 * measure_pppoe_rates ()		    - measure pppoe rates
 *----------------------------------------------------------------------------
 */
static void
measure_pppoe_rates (void)
{
    if (PPPOE_ONLINE_START > 0)
    {
	if (ioctl (pppoe_socket, SIOCGPPPSTATS, &pppoe_req) == 0)
	{
	    unsigned long ibytes, obytes;

	    ibytes = PPPOE_STATS_IBYTES - PPPOE_LAST_IBYTES;
	    obytes = PPPOE_STATS_OBYTES - PPPOE_LAST_OBYTES;
	    if (ibytes & 0x80000000)
	    {
		imond_syslog(LOG_INFO, "measure_pppoe_rates: ignoring %lx ibytes (%lx - %lx)", ibytes, 
			PPPOE_STATS_IBYTES, PPPOE_LAST_IBYTES);
	    }
	    else
	    {
		PPPOE_IBYTES += ibytes;
		PPPOE_IBYTES_PER_SECOND = (int)
		  ((float) ibytes / seconds_passed_pppoe);
	    }
	    if (obytes & 0x80000000)
	    {
		imond_syslog(LOG_INFO, "measure_pppoe_rates: ignoring %lx obytes (%lx - %lx)", obytes, 
			PPPOE_STATS_OBYTES, PPPOE_LAST_OBYTES);
	    }
	    else
	    {
		PPPOE_OBYTES += obytes;
		PPPOE_OBYTES_PER_SECOND = (int)
		  ((float) obytes / seconds_passed_pppoe);
	    }
	    PPPOE_LAST_IBYTES = PPPOE_STATS_IBYTES;
	    PPPOE_LAST_OBYTES = PPPOE_STATS_OBYTES;

	    last_time_pppoe.tv_sec = current_time.tv_sec;
	    last_time_pppoe.tv_usec = current_time.tv_usec;
	}
	else
	{
	    /* we are offline, but don't know about it yet */	    
	    imond_syslog(LOG_INFO, "measure_pppoe_rates: device went down without telling us about it yet");
	    PPPOE_LAST_IBYTES = 0;
	    PPPOE_LAST_OBYTES = 0;
	    PPPOE_IBYTES_PER_SECOND = 0;
	    PPPOE_OBYTES_PER_SECOND = 0;
	}
    }
    else
    {
	/* we are offline   */
	PPPOE_IBYTES_PER_SECOND = 0;
	PPPOE_OBYTES_PER_SECOND = 0;
    }

    return;
} /* measure_pppoe_rates (void) */


/*----------------------------------------------------------------------------
 * catch_sig ()				    - catch signals
 * catch_alarm ()			    - catch alarm signal
 *----------------------------------------------------------------------------
 */
static void
catch_sig (int sig)
{
    int	i;
    
    imond_syslog (LOG_INFO, "got signal %d, exiting now\n", sig);

    for (i = 0; i < MAX_CLIENTS; i++)
    {
	if (client_fd[i] >= 0)
	{
	    close (client_fd[i]);
	    client_fd[i] = -1;
	    password_state[i] = PASSWORD_STATE_NONE;
	}
    }

    close (sock_fd);

    exit (sig);
} /* catch_sig (sig) */
static void
catch_alarm (int sig)
{
    alarm_triggered = 1;
}

static void
check_ppp_device (void)
{
    if (pppoe_socket >= 0 && !pppoe_is_active)
    {
	if (ioctl (pppoe_socket, SIOCGPPPSTATS, &pppoe_req) == 0)
	{
	    PPPOE_LAST_IBYTES = PPPOE_STATS_IBYTES;
	    PPPOE_LAST_OBYTES = PPPOE_STATS_OBYTES;
	    pppoe_is_active = 1;
	}
    }
}

/*----------------------------------------------------------------------------
 * main ()				    - main function
 *----------------------------------------------------------------------------
 */
int
main (int argc, char ** argv)
{
    static struct sockaddr_in   my_listen_addr;
    static size_t		my_listen_size;
    FILE *			pid_fp;
    int				channel_idx;
    time_t			cur_seconds;
    time_t			last_seconds = 0;
    char			buf[256];
    char *			dialmode_string;
    struct timeval		tv;
    fd_set			fdset;
    int				select_rtc;
    int				port = 5000;
    int				max_fd;
    int				i;
    int                         killpid;
    int                         so_reuseaddr;

    struct sigaction act;

#ifndef TEST
    int				rtc;
#endif

    (void) signal (SIGHUP, SIG_IGN);
    (void) signal (SIGPIPE, SIG_IGN);

#ifdef TEST

    isdninfo_fd = fileno (stdin);

#else

    rtc = fork ();

    if (rtc < 0)
    {
	perror ("fork");
	exit (1);
    }

    if (rtc > 0)					/* it's the parent  */
    {
	return (0);
    }

    isdninfo_fd = open ("/dev/isdninfo", O_RDONLY);

#endif

    signal (SIGINT, catch_sig);
    signal (SIGTERM, catch_sig);

    /* install alarm handler */
    act.sa_handler = catch_alarm;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;
    act.sa_restorer = NULL;
    sigaction (SIGALRM, &act, NULL);

    imond_pass = getenv ("IMOND_PASS");

    if (imond_pass && !*imond_pass)
    {
	imond_pass = (char *) NULL;
    }

    if (imond_pass)
    {
	password_mode |= PASS_REQUIRED;
    }

    imond_admin_pass = getenv ("IMOND_ADMIN_PASS");

    if (imond_admin_pass && !*imond_admin_pass)
    {
	imond_admin_pass = (char *) NULL;
    }

    if (imond_admin_pass)
    {
	if (imond_pass && ! strcmp (imond_pass, imond_admin_pass))
	{
	    imond_admin_pass = (char *) NULL;
	}
	else
	{
	    password_mode |= ADMIN_PASS_REQUIRED;
	}
    }

    dialmode_string = getenv ("DIALMODE");

    if (dialmode_string && *dialmode_string)
    {
	if (! strcmp (dialmode_string, "off"))
	{
	    dialmode = DIALMODE_OFF;
	}
	else if (! strcmp (dialmode_string, "auto"))
	{
	    dialmode = DIALMODE_AUTO;
	}
	else if (! strcmp (dialmode_string, "manual"))
	{
	    dialmode = DIALMODE_MANUAL;
	}
	else
	{
	    imond_syslog (LOG_WARNING,
			  "DIALMODE='%s' is unknown. Setting to 'off'...\n",
			  dialmode_string);
	    dialmode = DIALMODE_OFF;
	}
    }


    for (i = 0; i < MAX_CLIENTS; i++)
    {
	client_fd[i] = -1;
	password_state[i] = PASSWORD_STATE_NONE;
    }

    if ((sock_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)   /* open socket  */
    {
	perror ("socket");
	return (-1);
    }

    while (argc >= 3 && *(argv[1]) == '-')
    {
	if (! strcmp (argv[1], "-port"))
	{
	    port = atoi (argv[2]);

	    argc -= 2;
	    argv += 2;
	}
	else if (! strcmp (argv[1], "-log-to-syslog"))
	{
	    log_to_syslog = TRUE;
	    openlog ("imond", LOG_PID, LOG_USER);

	    argc --;
	    argv ++;
	}
	else if (! strcmp (argv[1], "-beep"))
	{
	    do_beep = TRUE;

	    argc --;
	    argv ++;
	}
	else if (! strcmp (argv[1], "-led"))
	{
	    led_device = argv[2];

	    argc -= 2;
	    argv += 2;
	}
	else
	{
	    break;
	}
    }

    if (argc >= 2 && *argv[1])		    /* empty argument is allowed    */
    {
	imond_log_dir = argv[1];
	sprintf (imond_log_file, "%s/imond.log", imond_log_dir);
    }

    if (argc >= 3 && *argv[2])		    /* empty argument is allowed    */
    {
	telmond_log_dir = argv[2];
    }

    if (argc == 4 && *argv[3])		    /* empty argument is allowed    */
    {
	mgetty_log_dir = argv[3];
	sprintf (mgetty_log_file, "%s/mfax.log", mgetty_log_dir);
    }

    my_listen_size = sizeof (my_listen_addr);

    (void) memset ((char *) &my_listen_addr, my_listen_size, 0);

    my_listen_addr.sin_family      = AF_INET;
    my_listen_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    my_listen_addr.sin_port        = htons (port);

    so_reuseaddr = 1;               /* re-use port immediately  */
    (void) setsockopt (sock_fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr,
                       sizeof so_reuseaddr);

    if (bind (sock_fd, (struct sockaddr *) &my_listen_addr, my_listen_size) < 0)
    {
	(void) close (sock_fd);
	perror ("bind");
	exit (1);
    }

    (void) listen (sock_fd, 5);

    read_config ();

	ISDN_LAST_IBYTES=0;
	ISDN_LAST_OBYTES=0;
    PPPOE_LAST_IBYTES=0;
    PPPOE_LAST_OBYTES=0;

    if (pppoe_used)
    {
	pppoe_socket = socket (AF_INET, SOCK_DGRAM, 0);

	if (pppoe_socket < 0)
	{
	    perror ("imond: couldn't create IP socket");
	}
	pppoe_req.ifr_data = (caddr_t) &pppoe_stats;
	strncpy (pppoe_req.ifr_name, pppoe_interface,
		 sizeof (pppoe_req.ifr_name));
	check_ppp_device ();
    }

    check_routing (SET_IMMEDIATELY);

    pid_fp = fopen ("/var/run/imond.pid", "w");

    if (pid_fp)
    {
	fprintf (pid_fp, "%d\n", getpid ());
	fclose (pid_fp);
    }

    for (;;)
    {
	FD_ZERO (&fdset);
	FD_SET (sock_fd, &fdset);

	if (isdninfo_fd >= 0)
	{
	    FD_SET (isdninfo_fd, &fdset);
	}

	max_fd = sock_fd;

	if (max_fd < isdninfo_fd)
	{
	    max_fd = isdninfo_fd;
	}

	for (i = 0; i < MAX_CLIENTS; i++)
	{
	    if (client_fd[i] >= 0)
	    {
		FD_SET (client_fd[i], &fdset);

		if (max_fd < client_fd[i])
		{
		    max_fd = client_fd[i];
		}
	    }
	}

	tv.tv_sec   = POLLING_INTERVAL;		    /* check every 10 sec   */
	tv.tv_usec  = 0;

	select_rtc = select (max_fd + 1, &fdset, (fd_set *) 0, (fd_set *) 0,
			     &tv);

	/* kill all zombies from beep & sync  */
	do
	{
	    killpid = waitpid (-1, (int *) NULL, WNOHANG);
	
	    /* we killed the beep-process, make next beep possible */
	    if (killpid > 0 && killpid == beep_pid)
	    {
		beep_pid = 0;
	    }
	} while (killpid > 0);

	if (pppoe_used) {
	    check_ppp_device();
	}

	if (gettimeofday (&current_time, NULL) == 0)
	{
	    if (pppoe_used && pppoe_is_active)
	    {
		seconds_passed_pppoe =
		    (float) (current_time.tv_sec  - last_time_pppoe.tv_sec) +
		    (float) (current_time.tv_usec - last_time_pppoe.tv_usec) /
			    1000000.;

		if (seconds_passed_pppoe >= MIN_MEASURE_INTERVAL &&
		    seconds_passed_pppoe < POLLING_INTERVAL + 5)
		{				/* sometimes 4294, but why? */
		    measure_pppoe_rates ();
		}
	    }

	    seconds_passed_isdn =
		(float) (current_time.tv_sec  - last_time_isdn.tv_sec) +
		(float) (current_time.tv_usec - last_time_isdn.tv_usec) /
			1000000.;

#ifdef TEST
	    printf ("seconds_passed_isdn=%f\n", (double) seconds_passed_isdn);
		last_time_isdn.tv_sec = current_time.tv_sec;
		last_time_isdn.tv_usec = current_time.tv_usec;
#endif

	}

	if (select_rtc > 0)
	{
	    if (isdninfo_fd >= 0 && FD_ISSET (isdninfo_fd, &fdset))
	    {
//		read_info ();
	    }
	    else
	    {
		/*------------------------------------------------------------
		 * imonc makes heavy I/O, so the select timeout will not
		 * be activated if imonc is connected. But we cannot
		 * call check_routing() too often, because it slows
		 * down the machine. Here we do it 10 seconds after the
		 * last call.
		 *------------------------------------------------------------
		 */

		cur_seconds = time ((time_t *) NULL);

		if (cur_seconds >= last_seconds + POLLING_INTERVAL)
		{
		    check_routing (SET_IF_CHANGED);
		    last_seconds = cur_seconds;
		}
	    }

	    if (FD_ISSET (sock_fd, &fdset))
	    {
		int new_fd;

		for (i = 0; i < MAX_CLIENTS; i++)
		{
		    if (client_fd[i] < 0)
		    {
			break;
		    }
		}

		new_fd = accept (sock_fd, (struct sockaddr *) &my_listen_addr,
				 (int *) &my_listen_size);

		if (new_fd >= 0)
		{
		    if (i < MAX_CLIENTS)
		    {
			unsigned char a[4];

			client_fd[i] = new_fd;
			(void) memcpy (a, &(my_listen_addr.sin_addr.s_addr), 4);
			(void) sprintf (client_ip[i], "%d.%d.%d.%d",
					a[0], a[1], a[2], a[3]);

			password_state[i] = PASSWORD_STATE_NONE;
			client_salt[i][0] = '\0';
			
			if (! strcmp (client_ip[i], "127.0.0.1"))
			{	    /* local connection: no password needed */
#ifndef TEST
			    password_state[i] |= PASSWORD_STATE_NORMAL |
						 PASSWORD_STATE_ADMIN;
#endif
			}
			else
			{
			    if (! (password_mode & PASS_REQUIRED))
			    {
				password_state[i] |= PASSWORD_STATE_NORMAL;

				if (! (password_mode & ADMIN_PASS_REQUIRED))
				{
				    password_state[i] |= PASSWORD_STATE_ADMIN;
				}
			    }
			}
		    }
		    else
		    {				/* sorry, too many clients */
			close (new_fd);
		    }
		}
	    }

	    for (i = 0; i < MAX_CLIENTS; i++)
	    {
		if (client_fd[i] >= 0 && FD_ISSET (client_fd[i], &fdset))
		{
		    if (read_line (client_fd[i], buf, 255) == 0)
		    {
			char * answer;

			answer = print_status (i, buf);

			if (answer)
			{
			    int	n = strlen (answer);

			    if (write (client_fd[i], answer, n) != n)
			    {
				close (client_fd[i]);
				client_fd[i] = -1;
			    }
			}
			else
			{
			    close (client_fd[i]);
			    client_fd[i] = -1;
			    password_state[i] = PASSWORD_STATE_NONE;
			}
		    }
		    else
		    {
			close (client_fd[i]);
			client_fd[i] = -1;
			password_state[i] = PASSWORD_STATE_NONE;
		    }
		}
	    }
	}
	else if (select_rtc == 0)				/* timeout  */
	{
	    check_routing (SET_IF_CHANGED);
	}
	else if (select_rtc < 0)
	{
	    if (errno != EINTR)
	    {
		perror ("select");
		sleep (5);
		exit (1);
	    }
	    else
	    {
		imond_syslog (LOG_ERR, "select rtc = %d, errno = %d\n",
			      select_rtc, errno);
	    }
	}
    }

    /* NOTREACHED */
    return (0);
} /* main (int argc, char **argv) */

// vim: softtabstop=4:shiftwidth=4
