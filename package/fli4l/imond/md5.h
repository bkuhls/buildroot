/* md5.h - Compute MD5 checksum of files or strings according to the
 *         definition of MD5 in RFC 1321 from April 1992.
 * Copyright (C) 1995-1999 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Written by Ulrich Drepper <drepper@gnu.ai.mit.edu> */
/* Hacked to work with BusyBox by Alfred M. Szmidt <ams@trillian.itslinux.org> */
/* Stripped down for fli4l by Tobias Greutzmacher <fli4l@portfolio16.de> */

/*
 * June 29, 2001        Manuel Novoa III
 *
 * Added MD5SUM_SIZE_VS_SPEED configuration option.
 *
 * Current valid values, with data from my system for comparison, are:
 *   (using uClibc and running on linux-2.4.4.tar.bz2)
 *                     user times (sec)  text size (386)
 *     0 (fastest)         1.1                6144
 *     1                   1.4                5392
 *     2                   3.0                5088
 *     3 (smallest)        5.1                4912
 */

#define MD5SUM_SIZE_VS_SPEED 2

/**********************************************************************/

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <sys/types.h>
#if defined HAVE_LIMITS_H
# include <limits.h>
#endif

//----------------------------------------------------------------------------
//--------md5.h
//----------------------------------------------------------------------------

/* md5.h - Declaration of functions and data types used for MD5 sum
   computing library functions. */

typedef u_int32_t md5_uint32;

/* Structure to save state of computation between the single steps.  */
struct md5_ctx {
	md5_uint32 A;
	md5_uint32 B;
	md5_uint32 C;
	md5_uint32 D;

	md5_uint32 total[2];
	md5_uint32 buflen;
	char buffer[128];
};

/*
 * The following three functions are build up the low level used in
 * the functions `md5_stream' and `md5_buffer'.
 */

/* Initialize structure containing state of computation.
   (RFC 1321, 3.3: Step 3)  */
void md5_init_ctx __P((struct md5_ctx * ctx));

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is necessary that LEN is a multiple of 64!!! */
void md5_process_block __P((const void *buffer, size_t len,
								   struct md5_ctx * ctx));

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is NOT required that LEN is a multiple of 64.  */
void md5_process_bytes __P((const void *buffer, size_t len,
								   struct md5_ctx * ctx));

/* Process the remaining bytes in the buffer and put result from CTX
   in first 16 bytes following RESBUF.  The result is always in little
   endian byte order, so that a byte-wise output yields to the wanted
   ASCII representation of the message digest.

   IMPORTANT: On some systems it is required that RESBUF is correctly
   aligned for a 32 bits value.  */
void *md5_finish_ctx __P((struct md5_ctx * ctx, void *resbuf));




/* Compute MD5 message digest for bytes read from STREAM.  The
   resulting message digest number will be written into the 16 bytes
   beginning at RESBLOCK.  */
int md5_stream __P((FILE * stream, void *resblock));

/* Compute MD5 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
void *md5_buffer __P((const char *buffer, size_t len, void *resblock));

//----------------------------------------------------------------------------
//--------end of md5.h
//----------------------------------------------------------------------------

