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
/**********
*
* stubs.C - Stub modules for dis_prefs executable.
*
**********/

#include <vport.h>
#include vstartupHEADER
#include vstdlibHEADER

#include "galaxy_undefs.h"
#include "cLibraryFunctions.h"
#include "genString.h"
#include "messages.h"



//int gtDisplay_is_open(void) { return(0); }
int logger_end_transaction(void) { return(0); }
void logger_start_transaction(void) { }

void call_time_log(const char*nm, bool flag) {}

// referenced in psetmem.C
void system_message_force_logger(int) {}

// throw_error is externed in genError.h.
void throw_error(int) { }

class Obj;
extern "C" void obj_prt_obj(void*) {}
extern "C" char* shared_array_provide(char*, int, int&, int) { return(NULL); }



int popup_Question(const char*, const char*, const char*, const char*)
{ return(2); }


// stub for dis_message
void dis_message(helpContext, systemMessageType, msgKey fmtStr, ...)
{
    va_list ap;
    va_start(ap, fmtStr);
    genString msg;
    msg.vsprintf(fmtStr, ap);
    OSapi_fprintf(stderr, "dis_prefs Message:   %s", (char*) msg);
    va_end(ap);
}





extern "C" {

void aset_driver_exit (int) { exit (EXIT_FAILURE); }

void node_prt(const Obj*){};

void msg_error (const char* text, ...) { fprintf (stdout, "dis_prefs ERROR: %s\n", text); }
void msg_diag (const char* text, ...) { fprintf (stdout, "dis_prefs DIAGNOSTIC: %s\n", text); }


#define FIX_FILE(ch,out) (ch==1?stdout:(ch==2?stderr:out))
 
FILE* tee_file = NULL;
 
int gala_fprintf (int channel, FILE * out,  const char * fmt, va_list ap)
{
    FILE * f = FIX_FILE(channel,out);
    if (tee_file && (f==stderr||f==stdout)) {
        vfprintf(tee_file, fmt, ap);
        fflush(tee_file);
    }
    int retval = vfprintf (f, fmt, ap);
    fflush(f);
    return retval;
}
 
extern "C" int gala_fflush (int channel, FILE * out)
{
    FILE * f = FIX_FILE(channel,out);
    if (tee_file && (f==stderr||f==stdout))
        fflush(tee_file);
    return fflush(f);
}
 
extern "C" int gala_fwrite (const void * p, int size, int nitems, int channel, FILE * out)
{
    FILE * f = FIX_FILE(channel,out);
    const char * ptr = (const char *)p;
    int retval = -1;
    if (f==stderr||f==stdout){
        if (tee_file) {
            fwrite(ptr,size,nitems,tee_file);
            fflush(tee_file);
        }
        char * p = (char*)ptr;
        int num = size*nitems;
        for(int ii=0; ii<num; ++ii)
            putc(p[ii],f);
        retval = num;
    } else
        retval = fwrite(ptr, size, nitems, f);
    fflush(f);
    return retval;
}

}
