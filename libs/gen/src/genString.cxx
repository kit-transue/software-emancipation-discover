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
// genString.h.C
//------------------------------------------
// synopsis:
// 
// NULL-terminated string class with built-in
// storage management and various operators.
//------------------------------------------
#include <genString.h>

#include <cstdio>
using namespace std;
#include <cctype>

#include <cLibraryFunctions.h>
#include <genError.h>
#include <machdep.h>

int gen_strcmp(const char* p1, const char* p2)
{
  if(p1 && p2) return strcmp(p1, p2);
  else if(p1)	return 1;
  else if(p2)	return -1;
  else	return 0;
}
// FUNCTION DEFINITIONS

// DESTRUCTOR

// /dev/null FILE pointer; used in genString::vsprintf()
static FILE *dev_zero = NULL;

genString::~genString()
{ delete [] ptr; }

int genString::compare(const char* st) const
{

    if(ptr && st)
	return strcmp(ptr, st);
    else if(ptr)
	return 1;
    else if(st)
	return -1;
    else
	return 0;
}

genString& genString::operator =(const char *st)
{
    //Initialize(genString::operator =);

    put_value(st);

    return *this;
}
 
#define COPY(src,trg,len) memcpy(trg,src,len)
genString& genString::operator +=(const char *st)
{
//    Initialize(genString::operator +=);

    if(st)
    {
	const int olen = length();
	if(olen == 0)
	{
	    put_value(st);
	}
	else if(*st)
	{
	    const int nlen = strlen(st);
	    char* nptr = new char[olen + nlen + 1];
	    if(ptr)
		COPY(ptr, nptr, olen);
	        COPY(st, nptr + olen, nlen);
	    nptr[olen + nlen] = '\0';

	    if(ptr) delete [] ptr;
	    ptr = nptr;
	}
    }
    return *this;
}
 

genString& genString::operator +=(char c)
{
//    Initialize(genString::operator +=);

    if(c)
    {
	char cc[2];
	cc[0] = c;
	cc[1] = '\0';

	operator +=(cc);
    }
    return *this;
}
 

void genString::put_value(const char* st, int len)
{
//    Initialize(genString::put_value);

    char* temp = NULL;
    if(st)
    {
	if(len <= 0)
	    len = (*st ? strlen(st) : 0);
	temp = new char[len + 1];
	if(len > 0)
	    COPY(st, temp, len);
	temp[len] = '\0';
    }

    delete [] ptr;
    ptr = temp;
}

int genString::vsprintf(const char* fmt, va_list ap1, va_list ap2)
{
    int size = 0;

    if (fmt && fmt[0]) {
	// Let us open /dev/null only once
#ifndef _WIN32
	if (dev_zero || (dev_zero = fopen("/dev/null", "w"))) {
#else
        if (dev_zero || (dev_zero = fopen("NUL", "w"))) {
#endif
	    size = vfprintf(dev_zero, fmt, ap1);
	    if (size > 0) {
		char *buf = new char[size+13];  //boris: 13 is my lucky number
		if (buf) {
		    ::vsprintf (buf, fmt, ap2);
                    delete [] ptr;
		    ptr = buf;
		} else 
		    size = 0;
	    }
	}
    }
  
    if (size <= 0)
	put_value(0, 0);

    return size;
}


const char* genString::printf(const char* fmt ...)
{
//    Initialize(genString::printf);

    va_list ap1;
    va_list ap2;
    va_start(ap1, fmt);
    va_start(ap2, fmt);

    int sz = genString::vsprintf(fmt, ap1, ap2);

    va_end(ap2);
    va_end(ap1);

    return ptr;
}


void genString::l_trim()
{
//    Initialize(genString::l_trim);

    const int len = length();
    int i;
    for(i = 0; (i < len)  &&  isspace(ptr[i]); ++i) {}
    if(i > 0)
	COPY(ptr + i, ptr, len - i + 1);
}


void genString::r_trim()
{
//    Initialize(genString::r_trim);

    for(int i = length() - 1; (i >= 0)  &&  isspace(ptr[i]); --i)
	ptr[i] = '\0';
}


void genString::trim()
{
//    Initialize(genString::trim);

    l_trim();
    r_trim();
}


/*
   START-LOG-------------------------------------------

   $Log: genString.cxx  $
   Revision 1.15 2000/07/10 23:04:35EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.26  1994/06/23  18:45:12  builder
Port

Revision 1.2.1.25  1994/06/23  18:18:21  builder
Port (hp stuff)

Revision 1.2.1.24  1994/05/10  23:40:37  trung
Bug track: 0
misc. fixes for subsystem, reparse

Revision 1.2.1.23  1994/04/13  14:12:41  builder
Port - use aset_stdarg first

Revision 1.2.1.22  1994/01/24  21:37:41  azaparov
Bug track: 6117
Fixed bug 6117

Revision 1.2.1.21  1993/12/09  15:44:16  aharlap
bug # 5479

Revision 1.2.1.20  1993/12/08  01:36:58  aharlap
Bug track: # N/A
changed buffer size

Revision 1.2.1.19  1993/09/23  13:29:23  kws
Bug track: Port
OS_dependent changes for port

Revision 1.2.1.18  1993/07/07  16:25:34  mg
uninlined destructor for future optimization

Revision 1.2.1.17  1993/06/08  20:24:35  sergey
*** empty log message ***

Revision 1.2.1.16  1993/05/09  23:25:38  aharlap
*** empty log message ***

Revision 1.2.1.15  1993/05/09  22:56:38  aharlap
*** empty log message ***

Revision 1.2.1.14  1993/05/09  22:48:33  aharlap
*** empty log message ***

Revision 1.2.1.13  1993/05/09  22:45:00  aharlap
*** empty log message ***

Revision 1.2.1.12  1993/05/09  22:43:02  aharlap
*** empty log message ***

Revision 1.2.1.11  1993/05/09  22:40:26  aharlap
*** empty log message ***

Revision 1.2.1.10  1993/05/09  22:24:26  aharlap
*** empty log message ***

Revision 1.2.1.9  1993/04/29  19:05:40  mg
commented out Initialize

Revision 1.2.1.8  1993/04/26  17:23:32  mg
*** empty log message ***

Revision 1.2.1.7  1993/04/21  18:21:51  glenn
Use "delete []".
Rewrite l_trim to avoid need for re-allocation.
Simplify r_trim.  Part of bug #2613.

Revision 1.2.1.6  1993/03/02  01:21:36  boris
Fixed r_trim() l_trim() to use isspace()

Revision 1.2.1.5  1993/02/05  20:34:31  oak
Added r_trim, l_trim, and trim.

Revision 1.2.1.4  1993/01/16  02:14:57  glenn
Add compare.  Implement all comparison operators with compare.
Compare is safe to use with NULL pointers.

   END-LOG---------------------------------------------
*/
