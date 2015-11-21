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
#ifndef _ParaCancel_h
#define _ParaCancel_h

#ifndef ISO_CPP_HEADERS
#include <signal.h>
#else /* ISO_CPP_HEADERS */
#include <csignal>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <stdio.h>

#ifndef _WIN32

//#include <gtTopShell.h>
//#include <gtForm.h>

class ParaCancel
{
  public:
    //    ParaCancel(gtApplicationShell*, unsigned int argc, char** argv); // slave
    ParaCancel(const char* message, int signal, int pid=0, int win=0); // master
    ParaCancel(const char *group);
    ~ParaCancel ();
    static void set(void);
    static void reset(void);
    static int is_cancelled() {return canceller ? canceller->cancelled : 0;}

    static void kill(void*) {if (canceller) delete canceller;}

  public: 
    static ParaCancel* canceller;

  private:
    //    gtTopLevelShell* shell;
    //    gtForm* shell_form;
    int pid;
    int sig;  
    int cancelled;
    FILE* fd;

  private:
    static void cancel(int);

    static int wm_close(void*);
    //    static void cancel_CB (gtPushButton*, gtEventPtr, void*, gtReason);
};

#else // _WIN32

class gtApplicationShell;
class gtTopLevelShell;
class gtForm;

class ParaCancel
{
  public:
    ParaCancel(const char *group);
    ~ParaCancel ();
    static void set(void);
    static void reset(void);

    static int is_cancelled() {return canceller ? canceller->cancelled : 0;}

    static void kill(void*) {if (canceller) delete canceller;}

  public: 
    static ParaCancel* canceller;

  private:
    // gtTopLevelShell* shell;
    // gtForm* shell_form;
    int pid;
    int sig;  
    int cancelled;
    FILE* fd;

  private:

    static unsigned long __stdcall watch_cancel_event(void *);
    static void cancel(int);

    static int wm_close(void*);
    // static void cancel_CB (gtPushButton*, gtEventPtr, void*, gtReason);
};

#endif // _WIN32

#endif // _ParaCancel_h
