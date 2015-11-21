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
#include "timer.h"

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <sys/types.h>
#ifndef ISO_CPP_HEADERS
#include <time.h>
#else /* ISO_CPP_HEADERS */
#include <ctime>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <genString.h>
#include <machdep.h>
#include <cLibraryFunctions.h>

double timer::start_time[NUM_OF_TIMERS] = {0, 0};
int timer::enabled = 0;
//static struct timeb tloc;

double my_time()
{
  long secs;
  long microsecs;
  if (OSapi_time(&microsecs, &secs)) {
    msg("Failed call to OSapi_time", catastrophe_sev) << eom;
    ;
  }
  return (microsecs/1000000.0 + secs);
}

void timer::init(uint which_timer, const char* label, const char* name)
{
    if (enabled==0) {
	genString hack = OSapi_getenv("PSET_ARENA");
	if ((char*)hack && !strncmp(hack, "showtime", 8))
	    enabled = 1;
	else
	    enabled = -1;
    }
    if (enabled<0) return;
    if (which_timer < NUM_OF_TIMERS) {
	start_time[which_timer] = my_time();
        time_t date_n = (time_t) start_time[which_timer];
        msg("INITTIME: $1 : $2 : $3 - $4", normal_sev) << which_timer << eoarg << label << eoarg << name << eoarg << ctime(&date_n) << eom;
	;
    }
    else {
	msg("INITTIME: NUM_OF_TIMERS exceeded.", warning_sev) << eom;
	;
    }
}

void  timer::showtime(uint which_timer, const char* label, const char* name)
{
    if (enabled<0) return;
    double now = my_time();
    double elapsed = now - start_time[which_timer];
    start_time[which_timer] = now;
    msg("TIME: $1   : $2 : $3", normal_sev) << elapsed << eoarg << label << eoarg << name << eom;
    
}

int
proc_size(int pid)
{
#if defined(sun5) || defined(irix6)
  struct stat st;
  char proc[1024];
  int  fsize;

  if (pid == 0)
    pid = OSapi_getpid();

  sprintf(proc, "/proc/%d", pid);
  stat(proc, &st);

  if (st.st_mode & S_IFDIR) {
    strcat(proc, "/as");
    stat(proc, &st);
  }

  fsize = (int) st.st_size;

  return
    fsize/(1024*1000);
#else
  return 0;
#endif
}
