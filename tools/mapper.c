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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/utsname.h>

main()
{
    int exit_code = 1;

    char buf[MAXPATHLEN];
    struct utsname unamedata;
    int fd;
    caddr_t addr;
    uname(&unamedata);
    sprintf (buf,"/tmp/.%s_mmaplocker", unamedata.nodename);
    fd = open (buf, O_RDONLY);
    if (fd) {
	if (sizeof (addr) == read (fd, (char*)&addr, sizeof (addr)))
	    exit_code = 0;
	close (fd);
    }

    if (exit_code) {
	char * name = tempnam ("/tmp", "mapper");
	if (name) {
	    int unit = open(name, O_RDWR|O_CREAT, 0600);
	    if (unit > 0) {
		addr = mmap(0, 2*1024*1024, 
				    PROT_WRITE, MAP_SHARED, unit, 0);
		if (addr != (caddr_t)-1) {
		    addr = mmap(0, 160*1024*1024, 
				PROT_WRITE, MAP_SHARED, unit, 0);
		    if (addr != (caddr_t)-1) {
			addr = mmap(0, 160*1024*1024, 
				    PROT_WRITE, MAP_SHARED, unit, 0);
			if (addr != (caddr_t)-1) {
			    fd = open (buf, O_RDWR|O_CREAT, 0666);
			    if (fd) {
				if (sizeof (addr) == write (fd, (char*)&addr,
							    sizeof (addr)))
				    exit_code = 0;
				close (fd);
			    }
			}
		    }
		}
		close (unit);
	    }
	    unlink(name);
	}
	free (name);
    }

    exit (exit_code);
}

/*
  -----------begin  RCSLOG -------------
 $Log: mapper.c  $
 Revision 1.1 1995/04/12 12:54:20EDT aharlap 
 this file compiled under sun4 to mapper which should be copied to $PSETHOME/bin for both sun4 
 and sun5, but no mapper for hp700 installations
 * Revision 1.5  1994/01/21  20:54:08  davea
 * bug 6093
 * must build name string without using malloc
 *
  -----------end    RCSLOG ------------
 */
