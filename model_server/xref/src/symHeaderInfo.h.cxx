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
#include <string.h>
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#include <iostream>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <genError.h>
#include <symHeaderInfo.h>
#include <xrefSymbol.h>

class xrefSymbol;
void headerInfo::print()
{Initialize( headerInfo::print );
  msg("table_max = $1\n") << table_max << eom;
  msg("entry_cnt = $1\n") << entry_cnt << eom;
  msg("fragment_cnt = $1\n") << fragment_cnt << eom;
  msg("sym_file_size = $1\n") << sym_file_size << eom;
  msg("current size: next_offset_sym = $1\n") << next_offset_sym << eom;
  msg("ind_file_size = $1\n") << ind_file_size << eom;
  msg("current size: next_offset_ind = $1\n") << next_offset_sym << eom;

 if (next_offset_ind != (entry_cnt+fragment_cnt)*SIZEOF_XREFSYM) 
     msg("%%%error - inconsistent header information for pmod.ind\n") << eom;

}

void symHeaderInfo::insert_time_t(time_t *t)
{
  int n = sizeof(time_t);
  memcpy((char*)dummy, (char *)t, n);
}

time_t *symHeaderInfo::extract_time_t(time_t *t)
{
  if (t) {
    int n = sizeof(time_t);
    memcpy((char*)t, (char *)dummy, n);
  }
  return t;
}
