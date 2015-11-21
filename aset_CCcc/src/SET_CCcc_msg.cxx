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
#include "msg.h"
#include "fe_common.h"

extern "C"
{

#include "SET_CCcc_msg.h"


static msg_sev edg_to_msg_severity(int edg_severity)
{
    msg_sev sev;
    switch(edg_severity) {
	case es_default:
	case es_none:
	case es_remark:
	    sev = normal_sev;
	    break;
	case es_warning:
	    sev = warning_sev;
	    break;
	case es_discretionary_error:
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
    return sev;
}

void ParserMsg( const char *message, int edg_severity )
{
    if( message != NULL ) {
	msg("$1", edg_to_msg_severity(edg_severity)) << message << eoarg << eom;
    }
}

void ParserMsgId( const char *message_id, int edg_severity )
{
    if( message_id != NULL ) {
	msg(message_id, edg_to_msg_severity(edg_severity)) << eom;
    }
}

void fwd_lp_service_names(const char *log_presentation_service_name,
                          const char *message_group)
{
    init_lp_service(log_presentation_service_name, message_group);
}

} /*extern "C"*/
