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
extern "C" {
#include "basic_hdrs.h"
#include "fe_common.h"
#include "SET_additions.h"
#include "SET_complaints.h"
}

#include <string>
#include "msg.h"
namespace std {};
using namespace std;

extern "C" an_error_severity SET_complaint_issued = es_none;

// This translation exists because we can't #include msg.h in the C code
// which uses SET_complaints.h.
static msg_sev csev_to_sev(msg_csev cs) {
    switch (cs) {
	case normal_csev:
	    return normal_sev;
	case warning_csev:
	    return warning_sev;
	case error_csev:
	    return error_sev;
	case catastrophe_csev:
	    return catastrophe_sev;
	default:
	    return unknown_sev;
    }
}

static a_boolean init_complaint(msg_csev csev, msg_sev &sev, string &prefix) {
    if (IF_file_name || generate_multiple_IF_files) {
	sev = csev_to_sev(csev);
	if (SET_complaint_issued < es_catastrophe && catastrophe_sev == sev) {
	    SET_complaint_issued = es_catastrophe;
	}
	else if (SET_complaint_issued < es_error && error_sev <= sev) {
	    SET_complaint_issued = es_error;
	}
	else if (SET_complaint_issued < es_warning && warning_sev <= sev) {
	    SET_complaint_issued = es_warning;
	}
	else if (SET_complaint_issued < es_remark && normal_sev <= sev) {
	    SET_complaint_issued = es_remark;
	}
	prefix = "In IF generation: ";
	return TRUE;
    }
    else return FALSE;
}

extern "C" void complain(msg_csev csev, const char* txt) {
    string msg_id;
    msg_sev sev;
    if (init_complaint(csev, sev, msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), sev) << eom;
    }
}

extern "C" void complain_int(msg_csev csev, const char* txt, int arg) {
    string msg_id;
    msg_sev sev;
    if (init_complaint(csev, sev, msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), sev) << arg << eoarg << eom;
    }
}

extern "C" void complain_str(msg_csev csev, const char* txt, const char* str) {
    string msg_id;
    msg_sev sev;
    if (init_complaint(csev, sev, msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), sev) << str << eoarg << eom;
    }
}

extern "C" void complain_ulong(msg_csev csev, const char* txt, unsigned long arg) {
    string msg_id;
    msg_sev sev;
    if (init_complaint(csev, sev, msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), sev) << arg << eoarg << eom;
    }
}

extern "C" void complain_ulong_ulong(msg_csev csev, const char* txt, unsigned long arg1,
				 unsigned long arg2) {
    string msg_id;
    msg_sev sev;
    if (init_complaint(csev, sev, msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), sev) << arg1 << arg2;
    }
}

extern "C" void complain_ulong_ulong_ulong(msg_csev csev, const char* txt, unsigned long arg1,
				 unsigned long arg2, unsigned long arg3) {
    string msg_id;
    msg_sev sev;
    if (init_complaint(csev, sev, msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), sev) << arg1 << eoarg << arg2 << eoarg << arg3 << eoarg << eom;
    }
}

extern "C" void complain_pos(msg_csev csev, const char* txt, a_source_position* pos) {
    char* filename;
    char* full_name;
    a_line_number lineno;
    a_boolean at_eof;
    string msg_id;
    msg_sev sev;
    if (init_complaint(csev, sev, msg_id)) {
	conv_seq_to_file_and_line(pos->seq, &filename, &full_name, &lineno,
				  &at_eof);
	msg_id += txt;
	msg(msg_id.c_str(), sev) << full_name << eoarg << lineno << eoarg << pos->column << eoarg << eom;
    }
}

static a_boolean init_worry(string &prefix) {
#if DEBUG
    if (db_worry && (IF_file_name != NULL || generate_multiple_IF_files)) {
	prefix = "In IF generation: ";
	return TRUE;
    }
    else return FALSE;
#else /* !DEBUG */
    return FALSE;
#endif /* DEBUG */
}

extern "C" void worry(const char* txt) {
    string msg_id;
    if (init_worry(msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), warning_sev);
    }
}

extern "C" void worry_ulong(const char* txt, unsigned long arg) {
    string msg_id;
    if (init_worry(msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), warning_sev) << arg << eoarg << eom;
    }
}

extern "C" void worry_ulong_ulong(const char* txt, unsigned long arg1,
			      unsigned long arg2) {
    string msg_id;
    if (init_worry(msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), warning_sev) << arg1 << eoarg << arg2 << eoarg << eom;
    }
}

extern "C" void worry_str(const char* txt, const char* str) {
    string msg_id;
    if (init_worry(msg_id)) {
	msg_id += txt;
	msg(msg_id.c_str(), warning_sev) << str << eoarg << eom;
    }
}

