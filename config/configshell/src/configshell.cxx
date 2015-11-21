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
/*  configshell.C

  When this starts, stdin is actually a pipe from paraset, called the 
      command pipe internally, and stdout is actually a pipe to paraset,
      called the status pipe.

  First dup these two handles and remember their handles in static variables
     command_handle,  status_handle


  Now use searchpty() to find an available pty device
  open the corresponding tty
Now spawn xterm in slave mode, telling it which pty
  
     assign the tty handle to stdin, stdout, and stderr


 */

#include <termios.h>
#include <machdep.h>
#ifndef hp10
#include <sysent.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#ifdef sun4
#include <sys/signal.h>
#else
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef hp700
#define TCGETS TCGETATTR
#define TCSETS TCSETATTR
#endif

#endif
#include <string.h>

#ifdef USG
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

// global variables
static int command_handle;
static FILE* command_stream;
static int status_handle;
static FILE* status_stream;
static int window_pid=0;	   // pid of xterm
static unsigned long window_id=0;  // window_id of xterm
static int processing=0;	//boolean flag, we're processing a command
static int is_batch =0;

// function prototypes for functions defined in this module
static int searchpty (char*,char*,int,int&);
static int spawn (char **);
int pain(int argc, char* argv[]);
void mainloop();
void sendstatus();
int init_xterm();
int setup();
void sighandler(int);
void sigpipe(int);

#include <sys/types.h>

// prototype for function in   totop.C

int bring_to_top(unsigned long window_id);
int iconify(unsigned long window_id);
static int put_is_processing = 0;

int main(int argc, char* argv[])
// The main routine for configshell
{
    for (int i=0;i<argc;i++) {
        if (strcmp(argv[i],"-cfg_put") == 0) {
            put_is_processing = 1;
	    is_batch = 1;
	} else if (strcmp(argv[i],"-batch")==0)
	    is_batch = 1;
    }
    argc = argc; argv = argv;
    setup();

    signal(SIGINT, SIG_IGN);        /* ^C from keyboard*/
    signal(SIGHUP, SIG_IGN);	    /* Hangup */
    signal(SIGTERM, SIG_IGN);	    /* software termination signal */
/*  note, the SIGKILL signal cannot be intercepted */
    signal(SIGIO, (SIG_TYP) sighandler);	    /* i/o is possible on a descriptor */
    signal(SIGCHLD, (SIG_TYP) sighandler); /* child's status has changed */
    signal(SIGWINCH, (SIG_TYP) sighandler);  /* window changed */
    signal(SIGPIPE, (SIG_TYP) sigpipe);  /* write on a broken pipe */
    mainloop();

    if (!is_batch) {
	fprintf(status_stream,"Killing process %d\n",window_pid);
        write (status_handle,"Killing the xterm window\n",27);
        kill (window_pid,SIGKILL);  /* see kill(2) and sigvec(2) */
    }
    return 0;
}



void mainloop()
// imports status_handle - the status pipe's handle #
// The main loop waits for the config-command sent from aset
// and sent back the results.
{
//    char configurator[200];
//    strcpy (configurator,"/usr/davea/as/sun4-g/bin/rcsconfigurator");

    char commandbuf[8192];
    for (int loopcount=0;1;loopcount++) {
	char* done;
	fflush (status_stream);
	strcpy(commandbuf,"empty");
	do {
	    if (!window_pid) init_xterm();
	    processing=0;
	    done=fgets(commandbuf,sizeof(commandbuf),command_stream);
	    processing=0;
	    if (!window_pid) init_xterm();
	    if (!done && feof(command_stream)) {
		// pipe broke, so we'll quit, taking xterm window with us
		done=commandbuf;
		fputs ("$quitting because of broken command pipe\n", status_stream);
		strcpy(commandbuf, "exit");
	    }
	} while (!done) ;
	if (commandbuf[strlen(commandbuf)-1]=='\n')
	    commandbuf[strlen(commandbuf)-1]='\0';
//	fprintf(stderr, "Cmd%d: *%s*\n", loopcount, commandbuf);

	char *ptr = commandbuf+4;
	ptr += strspn(ptr," ");

	if (!strncmp(commandbuf, "icon", 4)) {
	    if (!is_batch) iconify(window_id);
	    continue;
	}

	if (!strncmp(commandbuf, "exit", 4)) {
	    if(!is_batch)system ("setwait");
	    break;
	}
	
        if (!put_is_processing && !is_batch)
            bring_to_top(window_id);
	if (!strncmp(commandbuf, "exec", 4)) {
	    char *cmdtail;
	    // display command on xterm in reverse video
	    cmdtail = strchr(ptr,' ');
	    if (!cmdtail) cmdtail = ptr;
            if (!is_batch) {
	        printf ("\033[7mCmd%d: %s\033[0m\n",loopcount,cmdtail);
	        fflush(stdout);
            }
	    // append the handle switch  (eg.  -handle9)
	    sprintf(ptr+strlen(ptr), " -handle%d", status_handle);

	    int err=system(ptr);
	    if (err) {
                if (!is_batch) {
		    printf("Error %d\n",err);
		    fflush(stdout);
                }
	    	fprintf (status_stream, "$error %d\n", err);
                if (!is_batch) putenv("PSET_FORCE_NOWAIT=0");
	    }
	    else fprintf (status_stream, "$ok\n");
	    continue;
	}
	    
	fputs ("$illegal command\n", status_stream);
    }
    fputs ("$exiting\n", status_stream);
    fflush (status_stream);
}

int setup()
{
    command_handle=dup(0);
    command_stream=fdopen(command_handle,"r");
    status_handle=dup(1);
    status_stream=fdopen(status_handle,"w");
    /* note:  probably should issue an fcntl(xx, F_SETFD, xx) on each */
    return 0;
}




/* ***************************************************************************
   init_xterm:  start up a slavemode xterminal, pointing stdin/stdout/stderr
      there.
   return:  nonzero for error
   ***************************************************************************/

int init_xterm()
// create the xterm window if it is not created yet.
{
    if (is_batch) {
	window_pid = -1;
	return 0;
    }

    char cmd[64];
    char *argv[20];
    struct termios term;

#ifdef USG
   int fd;
   char *slavename;
   int tty;

   fd = open ("/dev/ptmx", O_RDWR);
   grantpt(fd);
   unlockpt(fd);
   slavename = ptsname(fd);

   tty = open(slavename, O_RDWR);
   if (tty < 0)
	exit(100);
   ioctl (tty, I_PUSH, "ptem");
   ioctl (tty, I_PUSH, "ldterm");
	
   sprintf (cmd, "\-S%s%d", &slavename[strlen(slavename)-2], fd);
   printf ("%s <%s>\n", slavename, cmd);
#else
#if !defined(irix5) && !defined(irix6)    // For SGI, we'll use _getpty (see below)

#   define TERMLEN 11
    static char pterm[TERMLEN] = "/dev/ptyxx";
    static char tterm[TERMLEN] = "/dev/ttyxx";
    int    fd;    /* fd of available pty   */
    int tty ;      /* fd of available tty   */

/*  search pty table for an available pseudo-terminal  */

    if (!(fd = searchpty(pterm, tterm, TERMLEN-1, tty))) {
	printf("no available pty's");
	exit(100);
    }
#ifdef hp10
    sprintf (cmd, "-S%c%c%d", tterm[TERMLEN-3], tterm[TERMLEN-2], fd);
#else
    sprintf (cmd, "\-S%c%c%d", tterm[TERMLEN-3], tterm[TERMLEN-2], fd);
#endif
#endif /* !irix5 && !irix6 */
#endif

#if defined(irix5) || defined(irix6)
    int filedes[2];
    char *slavename;
    slavename = _getpty(&filedes[0], O_RDWR, 0600, 0);
    if (0 == slavename) {
	printf("getpty failed");
	exit(100);
    }
    if (0 > (filedes[1] = open(slavename, O_RDWR))) {
	(void)close(filedes[0]);
        printf("getpty failed");
	exit(100);
    }
    int fd = filedes[0];
    int tty = filedes[1];
    sprintf(cmd, "\-S%s%d", &slavename[strlen(slavename) - 2], fd);
#endif /* irix5 || irix6 */

    argv[0]="xterm";
    argv[1]= cmd;           // tell it to be in slave mode, and on which pty
    argv[2] = "-T";
    argv[3] = "CM Monitor";  // window title
    argv[4] = "-n";
    argv[5] = "Config";      // icon title
    argv[6] = "-name";
    argv[7] = "pset_config";
    if (put_is_processing)
    {
        argv[8] = "-iconic";
        argv[9] = 0;
    }
    else
        argv[8] = 0;

    /*  spawn the xterm in slave mode, telling it which pty to use */
    window_pid = spawn (argv);


    /*  attach our stdin, stdout and stderr to the corresponding tty */
    if (tty != 0) { close (0); dup (tty); }
    if (tty != 1) { close (1); dup (tty); }
    if (tty != 2) { close (2); dup (tty); }

    /* change attributes of tty  */
    ioctl (tty, TCGETS, &term);

//  	term.c_lflag |= ISI | ICANON | ECHOE | PENDIN | ECHOCTL |
//  	    ECHOKE | IEXTEN;
#ifndef hp700
  	term.c_lflag |= ISIG | ECHOE | PENDIN | ECHOCTL |
  	    ECHOKE | IEXTEN;
#else
  	term.c_lflag |= ISIG | ICANON | ECHO | ECHOE | IEXTEN;
#endif

    term.c_iflag |= BRKINT | ICRNL;    // signal interrupt on break 

    term.c_oflag |= OPOST | ONLCR ;

    ioctl (tty, TCSETS, &term);

    /* Read window id of xterm */
//    char buf[100];
//    gets (buf);
    scanf ("%xl\n", &window_id);  //  convert it as hex

    if (tty > 2)  { close(tty); }
    if (fd > 2) { close (fd); }

    if (!is_batch && getenv ("PSET_DEBUG_CONFIGURATOR"))
    	printf ("((dbg info:Window's PID is %d))\n",window_pid);
    return 0;
}

#define PTYCHAR1        "pqrstuvwxyzPQRSTUVWXYZ"    
#define PTYCHAR2        "0123456789abcdef" 

static int searchpty (char *dev, int devlen)
{
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

static int searchpty (char *dev, char* tdev, int devlen, int& tty)
{
    int ptychar1 = 0;
    int ptychar2;
    int fd;
    tty = -1;

    while (PTYCHAR1[ptychar1]) {
	dev[devlen-2] = PTYCHAR1[ptychar1];
	ptychar2 = 0;
	while (PTYCHAR2[ptychar2]) {
	    dev[devlen-1] = PTYCHAR2[ptychar2];
	    if ( (fd = open (dev, O_RDWR)) >= 0) {
		tdev[devlen-2] = PTYCHAR1[ptychar1];
		tdev[devlen-1] = PTYCHAR2[ptychar2];
		tty = open (tdev, O_RDWR);
		if (tty >= 0) 
		    return fd;
		else 
		    close (fd);
	    }
	    ptychar2++;
	}
	ptychar1++;
    }

    return 0;
}



static int spawn (char **argv)
{
    int pid;

    pid = OSapi_vfork();

    if (pid < 0)
	exit(100);
    else if (pid == 0) /* child */
	execvp (argv[0], argv);

    return pid;
}
void sighandler(int sig)
{
//  fprintf(status_stream,"stat:  signal %d received\n",sig);
//  fprintf(stderr, "stderr: signal %d received\n",sig);
    if (sig==SIGCHLD) {
	pid_t whichpid;
	int status;
	const int options= WNOHANG;
	if (!is_batch) {
	whichpid=waitpid(window_pid, &status, options);  // which pid has stopped or terminated?
		// also flushes any zombie task <defunct>
	if (whichpid==window_pid && (status&0377)!=0177) {
	    // if the xterm has terminated, then
	    //    start it back up again, providing we're in the middle of
            //    processing a command.  If we're waiting for a command, it'll
            //    be taken care of then
	        window_pid=0;   // flag window is dead
		if (processing) init_xterm();
	}
        }
    }
    // in case it's a system V, re-enable the signal handler
    signal(sig, (SIG_TYP) sighandler);
}
void sigpipe(int)
{
    // we got the SIGPIPE signal, indicating that ParaSet died
    //     so we kill the XTERM window, and exit ourselves
    if (!is_batch) kill (window_pid,SIGKILL);  // see kill(2) and sigvec(2)
    exit(0);
}


/* *************************************************************************
References:
   see    man termio  and search for "Terminal Parameters" to see how we
    can customize the xterm interface with the ioctl() call.
          also search on 

   see cbreak mode???
   wait or no-wait     tiocwait
   man 3c stty
   man ioctl

   we have sources to tcsh




Note, I'll get SIGCHLD when the xterm dies.  Presumably the way I handle that
   is to create a new xterm.  see man signal


    **********************************************************************/


/* ***************************************************************************
For bug 2369 (Peter's bug about getting repeated messages on stderr saying
   that the configurator is not found -- when the path is set up wrong)

see files  ui/setup/customize

New function (using new private flag variable) customize::haveconfigurator()
   the variable gets set or reset to indicate the presence of the configurator,
   every time the customize:configurator(char *) function is called to set
   the path.  haveconfigurator() simply returns the value of the variable.

In operate_on_modules (all overloads),  test for
     customize::haveconfigurator()  before looping.  Give a single dialog
     box, rather than calling the configurator() function at all.  This will
     take care of get,put,lock, and unlock.

status is done some other way. Make sure it doesn't display any message at all
     if haveconfigurator() is invalid.

This logic may be deferred now that status is no longer being called automatically
     anywhere.  For version 1 we might say "the user shouldn't use get/put/lock
     and unlock menu picks, unless he's picked a valid configurator".

    ***************************************************************************/




/*
   START-LOG-------------------------------------------

   $Log: configshell.C  $
   Revision 1.13 2000/04/04 10:03:24EDT sschmidt 
   Port to HP 10.  Merging from /main/hp10port to mainline.
// Revision 1.13  1994/07/21  15:49:37  so
// for put, don't pop up the xterm
//
// Revision 1.12  1994/05/09  13:20:54  builder
// Port
//
// Revision 1.11  1994/04/20  18:05:09  builder
// Port
//
// Revision 1.10  1994/02/28  22:32:05  builder
// Port
//
// Revision 1.9  1994/01/04  14:48:06  so
// setenv PSET_FORCE_NOWAIT 0 if some script generates errors.
//
// Revision 1.8  1993/12/21  17:49:18  kws
// Make window come up non-iconic
//
// Revision 1.7  1993/12/19  21:00:42  kws
// Use env var for debugging info
//
// Revision 1.6  93/11/05  15:57:44  15:57:44  builder ()
// Port
// 
// Revision 1.5  93/09/22  20:03:06  20:03:06  kws
// *** empty log message ***
// 
// Revision 1.4  1993/09/06  22:46:43  aharlap
// added command "icon"
// bug # 4680
//
// Revision 1.3  1993/04/15  21:53:53  davea
// bugs 3332, 3182 - bring configurator window to top.
// Also, when it's killed, defer recreating it till it's needed again.
//
// Revision 1.2  1993/03/22  21:33:16  davea
// add ^C logic (signals), missing xterm logic, and broken commandpipe logic
//
// Revision 1.1  1993/02/16  19:42:35  davea
// Initial revision
//

   END-LOG---------------------------------------------
*/



