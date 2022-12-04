/*----------------------------------------------------------------------------
 *  bytes2read.c - calculate kiB, MiB, GiB, etc. from bytes
 *
 *  usage: bytes2read <overflows> <bytes>
 *
 *  Copyright (c) 2002 Tobias Gruetzmacher <fli4l@portfolio16.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:     12.09.2002 tobig
 *  Last Update:  $Id$
 *----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdio.h>

const int factor = 1024;
const char * units = "KMGTPE";

int main (int argc, char *argv[]) {
	unsigned long overflows, bytes;
	double output;
	int unit = 0;
	
	if (argc != 3) {
		printf("usage: %s <overflows> <bytes>\n", argv[0]);
		exit(1);
	}

	/* Errorchecking is futile! ;) */
	overflows = strtoul(argv[1], (char **) NULL, 10);
	bytes = strtoul(argv[2], (char **) NULL, 10);
	output = ((unsigned long long)overflows << 32) + bytes;
 
	while (output > factor)
	{
		unit++;
		output /= factor;
	}

	if (unit > 0)
		printf("%.2f%ci\n", output, units[unit-1]);
	else
		printf("%.0f\n", output);

	exit(0);
}
	
