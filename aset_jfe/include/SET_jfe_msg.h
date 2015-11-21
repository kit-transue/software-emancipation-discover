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
#ifndef SET_jfe_msg_H
#define SET_jfe_msg_H

extern void fwd_lp_service_names(const char *log_presentation_service_name,
				 const char *message_group);
void ParserMsg( const char *msg, int edg_severity, a_boolean msg_to_stdout);
void ProgressMsg( const char *msg);

/* COMPLAINTS */

typedef enum {
	normal_csev,
	warning_csev,
	error_csev,
	catastrophe_csev,
	unknown_csev
} msg_csev;

void complain(msg_csev, const char *msg);
void complain_int(msg_csev, const char *msg, int p);
void complain_str(msg_csev, const char *msg, const char *s);
void fwd_activate_message_group(const char * msgid_str);
void fwd_deactivate_message_group();

/* The following variable starts out as normal_sev, and tracks the
 * highest severity used when invoking any of the various forms
 * of "complain" ("In IF generation").
 */

extern msg_csev SET_complaint_issued;

#endif
