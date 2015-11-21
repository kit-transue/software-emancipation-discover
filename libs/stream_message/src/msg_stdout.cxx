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
#ifdef _WIN32
#pragma warning(disable:4786)
#endif /* _WIN32 */

#include "msg.h"

#include "msg_stdout.h"
#include "msg_format.h"

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif /* _WIN32 */

#define ARG_KEY '$'
#define BACK_SLASH '\\'


ostream& msg(const char* msg_id)
{
  return msg(msg_id, unknown_sev);
}

extern void (*popup_msg_callback)(msg_sev, char const *);
ostream& msg(const char* msg_id, msg_sev severity)
{
  //instantiate msg_ostream object here
  msg_class *mc;
  mc = new msg_ostream(msg_id, cerr, severity);
  ostream* out = mc->received_msg();

  return *out;
}

bool msg_viewer_is_running()
{
  return false;
}

// ===========================================================================
void init_lp_service(const char *log_presentation_service_name,
		     const char *message_group)
{
}
// ===========================================================================


AutoMessageGroup::AutoMessageGroup(char const *name)
{
}

AutoMessageGroup::~AutoMessageGroup()
{
}

void activate_message_group(const char* grpid_str)
{
}

void deactivate_message_group(void)
{
}
