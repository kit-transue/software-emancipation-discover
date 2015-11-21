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
/* xscan.c
 *
 * Scanning utility to find an available port for
 * an X display (inspired by xforward.c).
 *
 * [abover, 05/17/97]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <X11/Xproto.h>
#include <string.h>

void usage(char *);

void usage(char *exe)
{
  fprintf(stderr, "Usage: %s [ <xnum> ] [ -h ]\n", exe);
  fprintf(stderr, "\n");
  fprintf(stderr, "       xnum       - X server number between 0 and 100 inclusive\n");
  fprintf(stderr, "       -h         - show usage\n");
}

main(int argc, char **argv)
{
  int ret = 1;

  int sockfd;
  struct sockaddr_in addr;
  int xpval = -1;
  int xpmin = 0;
  int xplim = 100;
  char *exe = argv[0];
  int go = 1;
  int i;

  /* parse arguments */

  if (argc == 1)
    xpval = xpmin;
  else
    for (i = 1, go = 1; (i < argc) && (go); i++) {
      if (!strcmp(argv[i], "-h")) {
        usage(exe);
        ret = 2;
        go = 0;
      }
      else
        if (xpval == -1) {
          xpval = abs(atoi(argv[i]));
          if ((xpval == 0) && (strcmp(argv[i], "0"))) {
            usage(exe);
            ret = 1;
            go = 0;
	  }
        }
        else {
          usage(exe);
          ret = 1;
          go = 0;
        }
    }

  if (!go)
    return ret;

  if ((xpval > xplim) || (xpval < xpmin)) {
    fprintf(stderr, "%s: ERROR: Server number out of range (%d -> %d): %d.\n", exe, xpmin, xplim, xpval);
    return ret;
  }

  xpval += X_TCP_PORT;
  xplim += X_TCP_PORT;
  xpmin += X_TCP_PORT;

  /* cycle through the ports */

  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    fprintf(stderr, "%s: ERROR: Cannot create socket: %s\n", exe, strerror(errno));
  else {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    for (go = 1; go; xpval++) {
      addr.sin_port = htons(xpval);
      if (bind(sockfd, (struct sockaddr *)(&addr), sizeof(addr)) == 0)
        break;
      if (xpval > xplim) {
        fprintf(stderr, "%s: ERROR: All ports in use.\n", exe);
        go = 0;
      }
      else if (errno != EADDRINUSE) {
        fprintf(stderr, "%s: ERROR: Cannot bind: %s.\n", exe, strerror(errno));
        go = 0;
      }
    }
    (void)close(sockfd);
    if (go) {
      fprintf(stdout, "%d\n", xpval - X_TCP_PORT);
      ret = 0;
    }
  }

  return ret;
}
