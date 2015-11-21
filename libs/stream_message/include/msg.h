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
#ifndef _MSG_H
#define _MSG_H

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */ 

//global eom and eoarg
static class endOfMessage { } eom;
static class endOfArg { } eoarg;

// Do not add values after unknown_sev;  do update the sev_str array
typedef enum {
	normal_sev,
	warning_sev,
	error_sev,
	catastrophe_sev,
	unknown_sev
} msg_sev;

void     operator<<(ostream& out, endOfMessage& eom);
ostream& operator<<(ostream& out, endOfArg& eoarg);
ostream& msg(const char* msg_id);
ostream& msg(const char* msg_id, msg_sev);
bool msg_viewer_is_running();

extern void init_lp_service(const char *log_presentation_service_name,
			    const char *message_group);

class AutoMessageGroup {
public:
    AutoMessageGroup(const char *name);
    virtual ~AutoMessageGroup();
    static char const *current();
    static unsigned long id_str_to_num(const char *id);
    AutoMessageGroup(unsigned long id);
private:
    AutoMessageGroup();
    AutoMessageGroup(const AutoMessageGroup &);
};

//to send progress information to the GUI
void report_progress(const char*);

// Specify that a task consists of a number of equal steps.
void define_progress_steps(const char *task, const char *step, unsigned long);

void register_ui_callbacks(void (*)(msg_sev, char const *), void (*)(msg_sev, char const *));

void activate_message_group(const char* grpid_str);
void deactivate_message_group(void);
#endif
