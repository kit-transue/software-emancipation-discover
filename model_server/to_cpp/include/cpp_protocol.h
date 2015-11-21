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
#ifndef _cpp_protocol_h
#define _cpp_protocol_h

enum which_preprocessor {
    CPP,
    ESQL,
// must be last:
    NUM_PREPROCESSORS
};

#define COMM_PP_REPLY			1000
#define COMM_PP_GO			1001
#define COMM_PP_QUIT			1002
#define COMM_PP_OPEN_FILE		1003

#define REPLY_PP(pp) \
       rarray[0] = (void *)COMM_PP_REPLY; \
       send ((void*)&rarray[0], sizeof (void*) * 3, pp)

#define COPY(olen,nstr,ostr)                            \
    int olen;                                           \
    char *nstr;                                         \
    olen = strlen(ostr);                                \
    nstr = (char*)builtin_mmgr->malloc (olen+1);        \
    strcpy (nstr, ostr)

#define PROLOG_PP(fn)                                   \
    void *array[4];                                     \
    comm_buffer* buffer;                                \
    array[0] = (void*)fn

#define EPILOG_PP(nargs,pp)                             \
    buffer = 0;                                         \
    int error_send = !send ((void*)&array[0], sizeof(void*) * nargs, pp);     \
    int error = 1;					\
    while (error) {					\
       receive (&buffer, pp);				\
       if (! buffer) break;                             \
       if ((int)buffer->mdata[0] == COMM_PP_REPLY)      \
           error = 0;                              	\
       else						\
           process_command (buffer, pp);		\
    }

#endif
