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
#include <cLibraryFunctions.h>
#include <msg.h>
#include <messages.h>
#include <genString.h>
#include <msgDict.h>
#include <customize.h>

#include <systemMessages.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <gtPushButton.h>

extern "C" int OSapi_fprintf(FILE*, const char*, ...);
static genString vmsg;

void vmessage(const char* fmt, va_list ap) 
{
    vmsg.vsprintf(fmt, ap);
}

void dis_message(helpContext hcontext, systemMessageType mtype, msgKey mkey, ...)
{
  static msgDict *msg_dict = NULL;

  // Make sure the message dictionary is initialized.
  if (msg_dict == NULL) {
    msg_dict = new msgDict(msgDictLOAD);
  }


  // Try to get the msg details from the database.
  if (msg_dict) {
    char *msg;
    if (!(msg = (char *)msg_dict->find(mkey))) {
      msg = (char *)mkey;
    }

    va_list ap;
    va_start(ap, mkey);
    vmessage(msg, ap);
    va_end(ap);
  } else 
    vmsg.printf("DISCOVER Error: Message Dictionary is not loaded.");

  // See if the msg database was loaded,
  if (msg_dict && !msg_dict->dbLoaded() ) {
    if (customize::is_init() ) {
      // We don't know why db wasn't loaded.
      vmsg += " ** No Msg DB Found";
    } else {
      // Cannot load until customize/preferences are initialized.
      // Tell user and set up to try again next time.
      vmsg += " ** During Customize Init.";
      delete msg_dict;
      msg_dict = NULL;
    }
  }

  if (vmsg.length() <= 0)
    vmsg = "(nil)";

  if (hcontext) gtPushButton::next_help_context(hcontext);

// The following msg_XXX calls use the complete message as the format argument
// to a vsprintf call, so literal "%" characters are dangerous (in worst case,
// may cause segfaults or memory scribbles).  To avoid the problem, we need to
// double each "%" we find.  -- wmm, 970625

  if (strchr(vmsg, '%')) {
      genString tmp = "";
      for (const char* p = vmsg; *p; p++) {
	  if (*p == '%') {
	      tmp += '%';
	  }
	  tmp += *p;
      }
      vmsg = tmp;
  }
  
  switch (mtype) {
    case MSG_ERROR: msg_error(vmsg.str()); break;
    case MSG_WARN: msg_warn(vmsg.str()); break;
    case MSG_STATUS: msg_inform(vmsg.str()); break;     // Using msg_inform(), \not msg_status(). Why?
    case MSG_INFORM: msg_inform(vmsg.str()); break;
    case MSG_DIAG: msg_diag(vmsg.str()); break;
    case MSG_STDERR:
    default: OSapi_fprintf(stderr, "%s", vmsg.str()); break;
  }
}

