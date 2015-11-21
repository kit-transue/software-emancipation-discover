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
//-----------------------------------------------------------------------------
// gString.C
// 
// Generic string class implementation.
//-----------------------------------------------------------------------------

#include "gString.h"
#include <cstdarg>
namespace std {};
using namespace std;

//-----------------------------------------------------------------------------

vchar gString::blank_[] = { 0 };

//-----------------------------------------------------------------------------

gString::~gString (void)
{
    if(ptr_) delete [] ptr_;
}

int gString::compare (const vchar* st) const
{
    if (ptr_ && st)
	return vcharCompare (ptr_, st);
    else if (ptr_)
	return 1;
    else if (st)
	return -1;
    else return 0;
}

const gString& gString::operator = (const vchar *st)
{
    put_value (st);
    return *this;
}

const gString& gString::operator += (const vchar *st)
{
    if (st) {
	const int olen = length ();
	if (olen == 0)
	    put_value (st);
	else if (*st) {
	    const int nlen = vcharLength (st);
	    vchar* nptr = new vchar [olen + nlen + 1];
	    if (ptr_)
		vcharCopyBounded (ptr_, nptr, olen);
	    vcharCopyBounded (st, nptr + olen, nlen);
	    nptr[olen + nlen] = '\0';
	    if(ptr_) delete [] ptr_;
	    ptr_ = nptr;
	}
    }
    return *this;
}

const gString& gString::operator += (vchar c)
{
    if (c) {
	vchar cc[2];
	cc[0] = c;
	cc[1] = '\0';
	operator += (cc);
    }
    return *this;
}

int gString::put_value (const vchar* st, int len)
{
    vchar* temp = NULL;
    if (st) {
	if (len <= 0)
	    len = (*st ? vcharLength (st) : 0);
	temp = new vchar [len + 1];
	if (len > 0)
	    vcharCopyBounded (st, temp, len);
	temp[len] = '\0';
    }
    else len = 0;
    if(ptr_) delete [] ptr_;
    ptr_ = temp;
    return len;
}

int gString::put_value_scribed (vscribe* scr, int len)
{
    vchar* temp = NULL;
    if (scr) {
	if (len <= 0)
	    len = vcharLengthScribed (scr);
        if (len) {
            temp = new vchar [len + 1];
            vcharCopyScribedBounded (scr, temp, len);
	    temp[len] = '\0';
        } else 
            delete scr;
    } else
        len = 0;
    if(ptr_) delete [] ptr_;
    ptr_ = temp;
    return len;
}

int gString::vsprintf (const vchar* fmt, va_list ap)
{
    return put_value_scribed (vcharScribeFormatVarargs ((vchar*) fmt, ap));
}


int gString::sprintf (const vchar* fmt, ... )
{
    va_list ap;
    va_start (ap, fmt);
    int len = vsprintf (fmt, ap);
    va_end (ap);

    return len;
}

int gString::vsprintf_scribed (vscribe* fmt, va_list ap)
{
    return put_value_scribed (vcharScribeFormatScribedVarargs (fmt, ap));
}

int gString::sprintf_scribed (vscribe* fmt, ... )
{
    va_list ap;
    va_start (ap, fmt);
    int len = vsprintf_scribed (fmt, ap);
    va_end (ap);

    return len;
}

void gString::l_trim (void)
{
    const int len = length();
    for (int i = 0; (i < len) && vcharIsWhiteSpace (ptr_[i]); ++i) {}
    if (i > 0)
	vcharCopyBounded (ptr_ + i, ptr_, len - i + 1);
}

void gString::r_trim (void)
{
    for (int i = length() - 1; (i >= 0) && vcharIsWhiteSpace (ptr_[i]); --i)
	ptr_[i] = '\0';
}

void gString::trim (void)
{
    l_trim ();
    r_trim ();
}
