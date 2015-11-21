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
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <setjmp.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <csetjmp>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <systemMessages.h>
#include <messages.h>
#include <genError.h>

#define BUFF_SIZE 20
static jmp_buf buff_arr[BUFF_SIZE];

static int     depth = 0;

void pop_buf() 
{
  if(depth == 0){
     msg("Attempted long-jump to nowhere", catastrophe_sev) << eom;
     return;
  }
  depth --;
}

int max_buf() {
  return (depth >= BUFF_SIZE);
}

PUSH_BUF_RET push_buf() {
  if(depth >= BUFF_SIZE){
     msg("Exceeded the buffer size", catastrophe_sev) << eom;
     return 0;
  }
  depth ++;
  return (PUSH_BUF_RET) buff_arr[depth-1];
}

void throw_error(int err_code)
{
  if(depth == 0){
     msg("Attempted long-jump to nowhere", catastrophe_sev) << eom;
     gen_trace_set_level(-1);
     return;
  }  
  depth --;

  longjmp(buff_arr[depth], err_code);
}

