/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
#include <fcntl.h>
#ifndef _WIN32
#include <sys/termios.h>
#endif
#include <c_machdep.h>
#include <stdio.h>

#ifdef USG
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

#ifdef irix6
#define vfork fork
#endif

#include <sys/ioctl.h>

static int searchpty (dev, devlen)
char *dev;
int   devlen;
{
#define PTYCHAR1        "pqrstuvwxyzPQRSTUVWXYZ"    
#define PTYCHAR2        "0123456789abcdef"

    int ptychar1 = 0;
    int ptychar2;
    int fd;

    while (PTYCHAR1[ptychar1]) {
	dev[devlen-2] = PTYCHAR1[ptychar1];
	ptychar2 = 0;
	while (PTYCHAR2[ptychar2]) {
	    dev[devlen-1] = PTYCHAR2[ptychar2];
	    if ( (fd = open (dev, O_RDWR)) >= 0)
		return fd;
	    ptychar2++;
	}
	ptychar1++;
    }

    return 0;
	    
}

static int spawn (argv)
char *argv[];
{
    int pid;

    pid = vfork();

    if (pid < 0)
	exit(100);
    else if (pid == 0) /* child */
	execvp (argv[0], argv);

    return pid;
}
	
    
void aset_process_init_tty(tickler)
    int tickler;
{
#ifdef _WIN32
    fprintf(stderr, "aset_process_init_tty(tickler) not implemented\n");
    fflush(stderr);
#else
    char cmd[64];
    int    pid;   /* pid of xterm */
    int tty;
    char *argv[10];
    int window;
    struct termios term;
   int fd;
#   define TERMLEN 11
    static char pterm[TERMLEN] = "/dev/ptyxx";
    static char tterm[TERMLEN] = "/dev/ttyxx";

    if(! (tickler == 3 || tickler == -1) )
    {
	dup2(tickler, 3);
	close(tickler);
    }

    if (fd = searchpty (pterm, TERMLEN-1)) {

	tterm[TERMLEN-3] = pterm[TERMLEN-3];
	tterm[TERMLEN-2] = pterm[TERMLEN-2];

	sprintf (cmd, "-S%c%c%d", tterm[TERMLEN-3],
		 tterm[TERMLEN-2], fd);

	tty = open (tterm, O_RDWR, 0);
	if (tty < 0)
	    exit(100);
    }

    if (fd) {
	argv[0]="xterm";
	argv[1]= cmd;
	argv[2] = "-T";
	argv[3] = (char *) gettext(/*T*/"ParaSET I/O Window");
	argv[4] = "-iconic";
	argv[5] = 0;


	pid = spawn (argv);

	if (tty != 0) { close (0); dup (tty); }
	if (tty != 1) { close (1); dup (tty); }

	ioctl (tty, TCGETS, &term);
	term.c_lflag |= ISIG | ICANON | ECHOE | PENDIN | ECHOCTL |
	    ECHOKE | IEXTEN;
	term.c_iflag |= BRKINT;
	ioctl (tty, TCSETS, &term);

	/* Read window id of xterm */
	scanf ("%x", &window);

	if (tty > 2)  { close(tty); }
	if (fd > 2) { close (fd); }
    }
#endif
} 

/*
// aset_process_init_tty
//------------------------------------------
// synopsis:
// Setup a slave pty for stdio
// description:
// ...
//------------------------------------------
// $Log: tty.c  $
// Revision 1.8 2000/07/07 08:12:50EDT sschmidt 
// Port to SUNpro 5 compiler
 * Revision 1.2.1.8  1994/03/11  12:14:33  jp
 * Fixed bug #6676.
 *
 * Revision 1.2.1.7  1994/03/10  23:21:50  builder
 * Port
 *
 * Revision 1.2.1.5  1993/09/30  18:04:41  kws
 * Port
 *
 * Revision 1.2.1.4  1992/12/09  20:13:37  smit
 * Fix bug #2061
 *
 * Revision 1.2.1.3  1992/10/23  18:13:47  smit
 * make xterm startup as iconic window.
 *
 * Revision 1.2.1.2  92/10/09  17:31:44  builder
 * fixed rcs header
 * 
 * Revision 1.2.1.1  92/10/07  21:26:01  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  21:25:59  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:24:41  smit
 * Initial revision
 * 
 * Revision 1.6  92/09/29  11:05:38  smit
 * Fixes bug #1388
 * 
 * Revision 1.5  92/09/17  17:04:41  smit
 * handle control sequences.
 * 
 * Revision 1.4  92/09/16  18:42:11  smit
 * fixes bug #66
 * 
 * Revision 1.3  92/06/18  19:21:37  smit
 * don't duplicate tickler if there isn't one.
 * 
 * Revision 1.2  92/02/13  09:48:55  glenn
 * dupe the pipe output file descriptor (tickler) onto 3.
 * 
 * Revision 1.1  92/01/30  12:14:37  kws
 * Initial revision
 * 
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/

