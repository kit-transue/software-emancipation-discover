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
#if defined(hp700)
#include <aset_stdarg.h>
#endif

// Needed to systemMessageType enum.
#include <messages.h>
#include <gglobalFuncs.h>

typedef unsigned char * _vstrptr;
#include "../../DIS_main/interface.h"

static void pset_stub (char* msg, int silent=0) {
    static int squelch = -1;
    if (squelch == -1) squelch = (OSapi_getenv("PSET_SILENCE_STUBS") != NULL);
    if(!silent && !squelch) OSapi_fprintf (stderr, "%s called, but is not defined\n", msg);
}      

void remote_call_async (Application *app, _vstrptr server, _vstrptr str) {

    rcall_server_eval_async (app, server, str);

}


_vstrptr server_eval(_vstrptr, _vstrptr)
{
    pset_stub("server_eval (_vstrptr, _vstrptr)");
    return NULL;
}


// This looks like the real thing but help context and msgtype are ignored.
void dis_message(helpContext hlpContext, systemMessageType mtype,
                 msgKey msgFormat, ...)
{
    gInit(dis_message);

    va_list ap;
    va_start(ap, msgFormat);
    dis_message((const vchar*) msgFormat, ap);
}

