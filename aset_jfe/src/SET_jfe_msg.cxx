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
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "msg.h"
#include "basics.h"
#include "host_envir.h"

extern "C"
{

#include "SET_jfe_msg.h"

/* Some messages are needed to be sent to log_presentation and also
   printed to stdout. msg_to_stdout flag is used to handle that.
*/
void ParserMsg( const char *message, int edg_severity, a_boolean msg_to_stdout )
{
    if( message == NULL )
        return;

    msg_sev sev;
    switch(edg_severity) {
	case es_default:
	case es_none:
        case es_remark:
	    sev = normal_sev;
	    break;
	case es_warning:
	case es_discretionary_error:
	    sev = warning_sev;
	    break;
	case es_error:
	    sev = error_sev;
	    break;
	case es_catastrophe:
	case es_command_line_error:
	case es_internal_error:
	    sev = catastrophe_sev;
	    break;
	default:
	    sev = normal_sev;
	    break;
    }
    msg("aset_jfe: $1", sev) << message << eoarg << eom;

    if (msg_to_stdout && msg_viewer_is_running()) {
        /* If msg_to_stdout is true and log_presentation is running then 
           print the message to stdout. If log_presentation is not 
           running then all messages are sent to stdout so there is no 
           need to print to stdout again. 
         */
        printf("aset_jfe: %s\n", message);
    }
}

void ProgressMsg( const char *message )
{
    if( message == NULL )
        return;
    report_progress(message);
}

void fwd_lp_service_names(const char *log_presentation_service_name,
                          const char *message_group)
{
    init_lp_service(log_presentation_service_name, message_group);
}

} /*extern "C"*/


/* COMPLAINTS */

extern "C" msg_csev SET_complaint_issued = normal_csev;

static void init_complaint(msg_csev sev, msg_sev &msev, string &prefix) {
    switch (sev) {
    case normal_csev:
	msev = normal_sev;
	break;
    case warning_csev:
	msev = warning_sev;
	break;
    case error_csev:
	msev = error_sev;
	break;
    case catastrophe_csev:
	msev = catastrophe_sev;
	break;
    default:
	// This should not happen, but if it does, we substitute something likely to work.
	sev = error_csev;
	msev = error_sev;
	break;
    }
    if (SET_complaint_issued < sev) {
        SET_complaint_issued = sev;
    }
    prefix = "In IF generation: ";
}

extern "C" void complain(msg_csev sev, const char *txt) {
    string msg_id;
    msg_sev msev;
    init_complaint(sev, msev, msg_id);
    msg_id += txt;
    msg(msg_id.c_str(), msev) << eom;
}

extern "C" void complain_int(msg_csev sev, const char *txt, int p) {
    string msg_id;
    msg_sev msev;
    init_complaint(sev, msev, msg_id);
    msg_id += txt;
    msg(msg_id.c_str(), msev) << p << eoarg << eom;
}

extern "C" void complain_str(msg_csev sev, const char *txt, const char *s) {
    string msg_id;
    msg_sev msev;
    init_complaint(sev, msev, msg_id);
    msg_id += txt;
    msg(msg_id.c_str(), msev) << s << eoarg << eom;
}

extern "C" void fwd_activate_message_group(const char * msgid_str) {
    activate_message_group(msgid_str);
    return;
}

extern "C" void fwd_deactivate_message_group() {
    deactivate_message_group();
    return;
}
