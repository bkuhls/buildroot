/*
 * Simple wrapper programm which calls its own command-name with a attached postfix
 * but serialized. Its main purpose is the serialization of iptables commands.
 * As it rewrites its own called name it should be suitable for all other tools, too
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 *
 * I don't accept liability nor provide warranty for this Software.
 * This material is provided "AS-IS" and at no charge.
 *
 * Version History
 *
 * 0.1     Initial Release
 *
 * rresch  (c) 2009 Robert Resch <fli4l@robert.reschpara.de>
 * 
 * code-review and rewriting (c) 2009 Yves Schumann <yves@eisfair.org>
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>

#define BUFFER_SIZE 1024
static char postfix[] = ".real";

int main(int argc, char **argv, char **envp) {
        char real_name[BUFFER_SIZE];
        ssize_t len;
        int fd;

        /* get real name */
        if ( (len = readlink("/proc/self/exe", real_name, sizeof(real_name))) < 0) {
                fprintf(stderr, 
                        "Unable to read programm name for '%s'\n", argv[0]);
                return 1;
        }
        if (sizeof(real_name) < len + sizeof(postfix)) {
                fprintf(stderr, 
                        "Not enough room in buffer for real name of '%s'\n",
                        argv[0]);
                return 2;
        }
        strcpy(real_name + len, postfix);

	/* open file to get descriptor used for locking */
        if ( (fd = open(real_name, O_RDONLY)) < 0) {
                fprintf(stderr, "Unable to open '%s', %s\n", 
                        real_name, strerror(errno));
                return 3;
        }

	/* actually lock - its a blocking call - waiting till lock succeeds */
	if (flock(fd, LOCK_EX) == -1) {
                fprintf(stderr, "Unable to lock '%s', %s\n", 
                        real_name, strerror(errno));
		return 4;
	}

        execve(real_name, argv, envp);
        fprintf(stderr, "Unable to execute '%s', %s\n", 
                real_name, strerror(errno));
        return 127;
}

