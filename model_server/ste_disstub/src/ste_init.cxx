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
/////////////////////////   FILE ste_init.C  ///////////////////////////
//
// -- Contains initialization of STE.               

#include <ste.h>
#include "steBufferHeap.h"
#include "machdep.h"
 
const char *DEF_FG = "DEF_FG";
const char *DEF_BG = "DEF_BG";

//
// Instance necessary globals
//

objArr* ste_turned_on_array = 0;
ste* ste_main = 0; 

//boris: pset_uxid is used for pset_send_own_callback
static long pset_uxid = 0;

long
ste_get_pset_uxid ()
{
    return pset_uxid;
}

extern "C" void
ste_set_Xclient_id (long id)
{
    pset_uxid = id;
}

extern "C" void
steInitialize () 
{
    ste_turned_on_array = new objArr;
    ste_main = db_new(ste, ());
    char *heap_buf = new char [313];
    OSapi_bzero(heap_buf,313);
    steBufferHeapPtr com_heap = db_new(steBufferHeap,(heap_buf,313,313));
    put_relation(ste_common_heap, com_heap, ste_main);
}
