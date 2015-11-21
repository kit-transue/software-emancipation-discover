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
#ifndef _systemMessages_h
#define _systemMessages_h

/*
 * systemMessages.h
 *
 * Restrictions:
 * This file must be C/C++ compatible.
 *
 */

#ifndef _prototypes_h
#include <prototypes.h>
#endif

#ifndef _general_h
#include <general.h>
#endif

struct systemMessageOptions
{
    boolean logFile	: 1;

    boolean error	: 1;
    boolean warning	: 1;
    boolean informative	: 1;
    boolean diagnostic	: 1;

    boolean import	: 1;
    boolean compile	: 1;
    boolean propagate	: 1;
};

enum systemMessageType
{
    MSG_UNKNOWN = -1,
    MSG_DIAG = 0,
    MSG_INFORM,
    MSG_WARN,
    MSG_ERROR,
    MSG_STATUS,
    MSG_WORKING,
    MSG_STDERR
};

#ifdef __cplusplus
systemMessageOptions& msg_options();
void msg_options(systemMessageOptions&, boolean notify = true);
#endif

#define TXT(x)	x
#define TXTT(x)	x

void popup_SystemMessages PROTO((
    struct systemMessageOptions*, void (*)(struct systemMessageOptions*)));

BEGIN_C_DECL
int  msg_size PROTO((unsigned int));
void msg_type PROTO((enum systemMessageType, const char*, ...));
void msg_diag PROTO((const char*, ...));
void msg_inform PROTO((const char*, ...));
void msg_warn PROTO((const char*, ...));
void msg_error PROTO((const char*, ...));
void msg_status PROTO((const char*, ...));
void msg_working PROTO((const char*, ...));

int  msg_question PROTO((const char*, ...));

void SystemMessages_update PROTO((struct systemMessageOptions*));

#if defined(sun5)
  char* gettext PROTO((const char*));  
#elif defined(__MC_LINUX__)
#include <libintl.h>
#else
  psetCONST char* gettext PROTO((const char*));  
#endif /* sun5 */

END_C_DECL


/*
    START-LOG-------------------------------

    $Log: systemMessages.h  $
    Revision 1.9 2000/07/07 08:18:10EDT sschmidt 
    Port to SUNpro 5 compiler
 * Revision 1.2.1.6  1994/07/27  00:09:25  jethran
 * removed quickhelp stuff as part of overall removal of quickhelp
 *
 * Revision 1.2.1.5  1994/01/04  23:53:54  boris
 * Bug track: Macro CONST
 * Renamed macro CONST into psetCONST
 *
 * Revision 1.2.1.4  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.3  1993/01/26  19:48:52  sharris
 * add MSG_WORKING message type
 *
 * Revision 1.2.1.2  1992/10/09  19:57:20  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif /* _systemMessages_h */
