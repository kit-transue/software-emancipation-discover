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
/* xopen.c
 *
 * Checks to see if the X display can be opened.
 *  
 * [abover, 05/17/97]
 *
 */

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>

#define TIMEOUT 20

char *exe, *ds;

void sigalrm_handler();

void sigalrm_handler()
{
  fprintf(stderr, "%s: ERROR: Timeout while opening display: \"%s\".\n", exe, ds);

  exit(1);
}

main(int argc, char **argv)
{
  int ret = 1;
  char newdisp[1024];

  Display *dv;

  exe = argv[0];

  if (argc > 2) {
    fprintf(stderr, "Usage: %s <display>\n", exe);
    return ret;
  }
  else if (argc == 2)
    ds = argv[1];
  else
    ds = getenv("DISPLAY");

  if (!ds)
    fprintf(stderr, "%s: ERROR: No display specified.\n", exe);
  else {
    if (*ds == ':') {
      gethostname(newdisp, 1024);
      strcat (newdisp, ds);
      ds = newdisp;
    }

    signal(SIGALRM, sigalrm_handler);
    alarm(TIMEOUT);

    if ((dv = XOpenDisplay(ds)) == NULL) {
      alarm(0);
      fprintf(stderr, "%s: ERROR: Cannot open display: \"%s\".\n", exe, ds);
    }
    else {
      alarm(0);
      fprintf(stderr, "%s: STATUS: Successfully opened display: \"%s\".\n", exe, ds);
      ret = 0;
    }
  }

  return ret;
}
