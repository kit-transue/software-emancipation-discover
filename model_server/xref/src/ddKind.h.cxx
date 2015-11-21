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

#include <general.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */

static const char * wrong_type(const char * type, int n)
{
 static char buf[100];
 sprintf(buf, "WRONG-TYPE %s(%d)",type,  n);
 return buf;
}

#define ddEnum(x) static const char * paste(x,_names)[] = 
#define tok(x) quote(x)

#define generate_prt_fun(x)\
 static int paste(x,_size) = sizeof(paste(x,_names))/sizeof(char*); \
 const char* paste(x,_name)(int n){  \
    if (n < 0 || n >= paste(x,_size)) return wrong_type(quote(x),n);  \
    else return paste(x,_names)[n]; }\
 int paste(x,_item)(const char * name){  \
    int n; for(n=0; n< paste(x,_size); ++n)\
      if(!strcmp(name,paste(x,_names)[n])) break;\
    return n;}

#include <ddKind.h>

/*
   START-LOG-------------------------------------------

   $Log: ddKind.h.cxx  $
   Revision 1.4 2000/07/10 23:13:10EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
// Revision 1.5  1992/12/24  17:09:56  mg
// wrong type
//
// Revision 1.4  1992/12/20  16:06:42  mg
// *** empty log message ***
//
// Revision 1.3  1992/12/20  16:03:57  mg
// polished
//
// Revision 1.2  1992/12/18  19:07:22  trung
// add macro for linkType
//
// Revision 1.1  1992/12/18  18:55:39  mg
// Initial revision
//

   END-LOG---------------------------------------------
*/
