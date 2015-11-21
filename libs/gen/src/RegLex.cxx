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
//
// RegLex.C
// --------
//

#include <stdlib.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#include <cstdio>
#endif /* ISO_CPP_HEADERS */

#include <RegExp.h>
#include "RegLex.h"

#if defined(sun5)

#define STRTOD strtoul

extern "C" {
    unsigned long strtoul(char *, char **, int);
};

#else

#define STRTOD strtol

extern "C" {
    long int strtol(const char *, char **, int);
};

#endif


static
int subexpressions(char* exp, Regexp * rr, lexToken*subs, int sub_len)
{

 int no_sub = 1;
 subs[0].ptr = exp + rr->BeginningOfMatch(0);
 subs[0].len = rr->EndOfMatch(0) - rr->BeginningOfMatch(0);
 int b1 = 0;
 int e1 = -1;

 int ib, ie;
 int match_len = rr->NumOfMatched(); // have to know when to stop
 int match_bnd = subs[0].len;        // (pointer arithmetic in Regexp* can kill us)
 for(int ii=1; ii<match_len && no_sub<sub_len; ++ii){
   ib = rr->BeginningOfMatch(ii);
   ie = rr->EndOfMatch(ii);
   if(ib>=e1 && ib<=match_bnd) { // next top level
     b1 = ib;
     e1 = ie;
     
     subs[no_sub].ptr = exp + ib;
     subs[no_sub].len = ie - ib;
     ++no_sub;
   }  
 }
 return no_sub;
}


void lexToken::print()
{
   putchar('\'');
   for(int jj=0; jj<len; ++jj) 
                        putchar(ptr[jj]);
   putchar('\'');
}

int
lexToken::getInt(int base, int * status)
{
    int stat = -1;
    int ret_val = 0;

    if (len>0 && ptr) {
	char ch = ptr[len];

	ptr[len] = '\0';
	ret_val  = (int)STRTOD(ptr, 0, base);
	ptr[len] = ch;
	stat     = 0;
    }
    if (status)
	*status = stat;

    return ret_val;
}

char *
lexToken::getString(int * status)
{
    int stat = -1;
    char * ret_val = (char *)0;

    if (len>0 && ptr) {
	ret_val = (char *)malloc(len+1);
	if (ret_val) {
	    stat = 0;
	    strncpy(ret_val, ptr, len);
	    ret_val[len] = '\0';
	}
    }
    if (status)
	*status = stat;

    return ret_val;
}

int
lexToken::integer(lexToken * token, int base)
{
    int ret_val = 0;

    if (token)
	ret_val = token->getInt(base);

    return ret_val;
}

char * lexToken::string(lexToken * token)
{
    char * ret_val = (char *)0;

    if (token)
	ret_val = token->getString();

    return ret_val;
}

extern "C" 
int matchRegSubs(char* exp, int len, Regexp * rr, lexToken*subs, int sub_len)
{
  rr->Search(exp, len, 0, len);
  int no_subs =  subexpressions(exp, rr, subs, sub_len);
  return no_subs;
}
