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
#ifndef _genString_h
#define _genString_h

#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <stdarg.h>

class genString
{
  public:
    genString();
    genString(const genString& that);
    genString(const char* st);

    ~genString();

    void put_value(const char*, int len = 0);
    const char* printf(const char* fmt...);

    int compare(const char*) const;

    int operator==(const genString& that) const;
    int operator==(const char* st) const;
    int operator==(char* st) const;

    int operator!=(const genString& that) const;
    int operator!=(const char* st) const;
    int operator!=(char* st) const;

    int operator<(const genString& that) const;
    int operator<(const char* st) const;

    int operator>(const genString& that) const;
    int operator>(const char* st) const;

    int operator<=(const genString& that) const;
    int operator<=(const char* st) const;

    int operator>=(const genString& that) const;
    int operator>=(const char* st) const;

    genString& operator=(const genString& that);
    genString& operator=(const char*);

    genString& operator+=(const char*);
    genString& operator+=(char);

    int length() const;
    int is_null() const;
    int not_null() const;

    char operator[](unsigned int) const;
    operator char*() const;
    operator const char*() const;
    const char* str() const;

    void l_trim();
    void r_trim();
    void trim();
    
    int vsprintf(const char *fmt, va_list ap1, va_list ap2);

  private:
    char* ptr;
};


// DEFAULT CONSTRUCTOR 

inline genString::genString() : ptr(0) {}


// COPY CONSTRUCTORS

inline genString::genString(const genString& that) : ptr(0)
{ put_value(that.ptr); }

inline genString::genString(const char* st) : ptr(0)
{ put_value(st); }


// OVERLOADED OPERATOR FUNCTIONS

inline genString& genString::operator=(const genString& that)
{ return *this = that.ptr; }

inline int genString::operator==(const genString& that) const
{ return compare(that.ptr) == 0; }

inline int genString::operator==(const char* st) const
{ return compare(st) == 0; }

inline int genString::operator==(char* st) const
{ return compare(st) == 0; }

inline int genString::operator!=(const genString& that) const
{ return compare(that.ptr) != 0; }

inline int genString::operator!=(const char* st) const
{ return compare(st) != 0; }

inline int genString::operator!=(char* st) const
{ return compare(st) != 0; }

inline int genString::operator<(const genString& that) const
{ return compare(that.ptr) < 0; }

inline int genString::operator<(const char* st) const
{ return compare(st) < 0; }

inline int genString::operator>(const genString& that) const
{ return compare(that.ptr) > 0; }

inline int genString::operator>(const char* st) const
{ return compare(st) > 0; }

inline int genString::operator<=(const genString& that) const
{ return compare(that.ptr) <= 0; }

inline int genString::operator<=(const char* st) const
{ return compare(st) <= 0; }

inline int genString::operator>=(const genString& that) const
{ return compare(that.ptr) >= 0; }

inline int genString::operator>=(const char* st) const
{ return compare(st) >= 0; }


// return the length of the string, or zero for null string.
inline int genString::length() const
{ return ptr ? strlen(ptr) : 0; }

// returns true if this genString is a null string
inline int genString::is_null() const
{ return ptr == 0; }

// return true if this genString is not a null string
inline int genString::not_null() const
{ return ptr != 0; }

inline char genString::operator[](unsigned int i) const
{ return (ptr == 0 || i >= strlen(ptr)) ? '\0' : ptr[i]; }

// conversion routines convert a genString to a char*.
inline genString::operator char*() const
{ return ptr; }

inline genString::operator const char*() const
{ return ptr; }

inline const char* genString::str() const
{ return ptr; }


#endif // _genString_h









