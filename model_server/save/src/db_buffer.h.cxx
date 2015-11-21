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
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <genError.h>
#include <db.h>
#include <psetmem.h>
#include <machdep.h>

void db_buffer::put (void* data, int len) 
{
//    Initialize (db_buffer::put);
    
    if (len > 0) {
	byte* pointer = grow (len);
	OS_dependent::bcopy (data, pointer, len);
    }
}

void db_buffer::put (char const *str) 
{
//    Initialize (db_buffer::put);

    int len = str ? strlen (str) : 0;
    byte* pointer = grow (len + 1);
    if(len)
        OS_dependent::bcopy (str, pointer, len);
    pointer[len] = '\0';
}

void db_buffer::put_byte (byte ch) 
{
//    Initialize (db_buffer::put_byte);

    byte* pointer = grow (1);
    *pointer = ch;
}

void db_buffer::put_short (short sh_src) 
{
//    Initialize (db_buffer::put_byte);
    short sh = htons(sh_src);
    byte* dest = grow (sizeof (short));
    byte* src = (byte*)&sh;
    for (int ii = 0; ii < sizeof (short); ++ii)
	*dest++ = *src++;
}

void db_buffer::put_int (int k_src) 
{
//    Initialize (db_buffer::put_int);

    int k = htonl(k_src);
    byte* dest = grow (sizeof (int));
    byte* src = (byte*)&k;
    for (int ii = 0; ii < sizeof (int); ++ii)
	*dest++ = *src++;
}

void db_buffer::put_double (double k) 
{
    byte* dest = grow (sizeof (double));
#ifndef _WIN32
    byte* src = (byte*)&k;
    for (int ii = 0; ii < sizeof (double); ++ii)
	*dest++ = *src++;
#else
    byte* src = (byte*)&k + sizeof(double) - 1;
    for (int ii = 0; ii < sizeof (double); ++ii)
	*dest++ = *src--;
#endif
}

bool db_buffer::get (char *& str)
{
//    Initialize (db_buffer::get);
    
    str = (char *)operator [] (pos);
    if (str) {
	int new_pos = pos + strlen (str) + 1;
	if (new_pos <= size ()) {
	    pos = new_pos;
	    return 1;
	}
    }
    return 0;
}

bool db_buffer::get (void* data, int len)
{
//    Initialize (db_buffer::get);
    
    if (len > 0) {
	byte* pointer = operator [] (pos);
	if (data) {
	    int new_pos = pos + len;
	    if (new_pos <= size ()) {
		pos = new_pos;
		OS_dependent::bcopy (pointer, data, len);
		return 1;
	    }
	}
    }
    return 0;
}

bool db_buffer::get_byte (byte& ch)
{
//    Initialize (db_buffer::get_byte);
    
    byte* pointer = operator [] (pos);
    if (pointer) {
	ch = *pointer;
	pos++;
	return 1;
    }
    return 0;
}

bool db_buffer::get_short (short& sh)
{
//    Initialize (db_buffer::get_byte);
    
    byte* src = operator [] (pos);
    if (src) {
	int new_pos = pos + sizeof (short);
	if (new_pos <= size ()) {
	    byte* dest = (byte*)&sh;
	    for (int ii = 0; ii < sizeof (short); ++ii)
	    	*dest++ = *src++;
	    sh = ntohs(sh);
	    pos = new_pos;
	    return 1;
	}
    }
    return 0;
}

bool db_buffer::get_double(double & dbl)
{
    byte* src = operator [] (pos);
    if (src) {
	int new_pos = pos + sizeof (double);
	if (new_pos <= size ()) {
	    byte* dest = (byte*)&dbl;
#ifndef _WIN32
            for (int ii = 0; ii < sizeof (double); ++ii)
		*dest++ = *src++;
#else
	    src += (sizeof(double) - 1);
            for (int ii = 0; ii < sizeof (double); ++ii)
		*dest++ = *src--;
#endif
	    pos = new_pos;
	    return 1;
	}
    }
    return 0;
}

bool db_buffer::get_int (int& k)
{
//    Initialize (db_buffer::get_byte);
    
    byte* src = operator [] (pos);
    if (src) {
	int new_pos = pos + sizeof (int);
	if (new_pos <= size ()) {
	    byte* dest = (byte*)&k;
            for (int ii = 0; ii < sizeof (int); ++ii)
		*dest++ = *src++;
	    k = ntohl(k);
	    pos = new_pos;
	    return 1;
	}
    }
    return 0;
}

bool db_buffer::skip (int nn)
{
//    Initialize (db_buffer::skip);

    
    if (nn > 0 && operator [] (pos)) {
	int new_pos = pos + nn;
	if (new_pos < size ()) {
	    pos = new_pos;
	    return 1;
	}
    }
    return 0;
}

void db_buffer::compress()
{
    Initialize(db_buffer::compress);
    if (length > 0 && length < capacity)
	ptr = (byte*)psetrealloc((char *)ptr,length);
    capacity = length;
}

/*
   START-LOG-------------------------------------------

   $Log: db_buffer.h.cxx  $
   Revision 1.5 2002/01/23 09:56:35EST ktrans 
   Merge from branch: mainly dormant code removal
// Revision 1.8  1993/11/04  20:43:02  aharlap
// Bug track: # ???
// commented Initialize
//
// Revision 1.7  1993/07/20  18:33:07  harry
// HP/Solaris Port
//
// Revision 1.6  1993/01/26  01:05:57  aharlap
// checking for zero length
//
// Revision 1.5  1993/01/08  07:17:43  aharlap
// optimized
//
// Revision 1.4  1993/01/07  18:55:55  aharlap
// changed get
//
// Revision 1.3  1993/01/07  04:11:01  mg
// new
//
// Revision 1.2  1993/01/06  14:38:57  aharlap
// use bcopy
//
// Revision 1.1  1993/01/05  21:23:45  aharlap
// Initial revision
//

   END-LOG---------------------------------------------

*/


