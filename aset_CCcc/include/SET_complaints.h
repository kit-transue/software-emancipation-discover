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
#ifndef SET_complaints_h
#define SET_complaints_h

#include "host_envir.h"

/* Reporting internal errors and warnings in the IF generation process. */

/* The "worry" functions for warnings are similar to the "complain" functions
 * for errors, but are mainly for debugging purposes.
 * They generally indicate that an optimization of the
 * IF generation is not working out as well as expected.
 * These are enabled by the -d-worry command line option.
 */

typedef enum {
	normal_csev,
	warning_csev,
	error_csev,
	catastrophe_csev,
	unknown_csev
} msg_csev;

extern an_error_severity SET_complaint_issued;

extern void complain(msg_csev, const char* txt);
extern void complain_int(msg_csev, const char* txt, int arg);
extern void complain_str(msg_csev, const char* txt, const char* str);
extern void complain_ulong(msg_csev, const char* txt, unsigned long arg);
extern void complain_ulong_ulong(msg_csev, const char* txt, unsigned long arg1,
				 unsigned long arg2);
extern void complain_ulong_ulong_ulong(msg_csev, const char* txt, unsigned long arg1,
				 unsigned long arg2, unsigned long arg3);
extern void complain_pos(msg_csev, const char* txt, a_source_position* pos);

extern void worry(const char* txt);
extern void worry_ulong(const char* txt, unsigned long arg);
extern void worry_ulong_ulong(const char* txt, unsigned long arg1,
			      unsigned long arg2);
extern void worry_str(const char* txt, const char* str);

#endif /* SET_complaints_h */
