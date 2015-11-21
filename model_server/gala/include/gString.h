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
// gString.h
//
// Generic string class.
//-----------------------------------------------------------------------------

#ifndef _gString_h
#define _gString_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef vcharINCLUDED
#include vcharHEADER
#endif

#ifndef vstdargINCLUDED
#include vstdargHEADER
#endif

//-----------------------------------------------------------------------------

class gString
{
  public:
    gString (void);
    gString (const gString&);
    gString (const vchar*);
    gString (const char*);

    ~gString (void);

    int put_value (const vchar*, int = 0);
    int put_value_scribed (vscribe*, int = 0);

    int compare (const vchar*) const;
    int compare (const char*) const;

    int sprintf (const vchar* fmt, ... );
    int vsprintf (const vchar *fmt, va_list ap);

    int sprintf_scribed (vscribe* fmt, ... );
    int vsprintf_scribed (vscribe* fmt, va_list ap);

    int operator == (const gString&) const;
    int operator == (const vchar*) const;
    int operator == (const char*) const;

    int operator != (const gString&) const;
    int operator != (const vchar*) const;
    int operator != (const char*) const;

    int operator < (const gString&) const;
    int operator < (const vchar*) const;
    int operator < (const char*) const;

    int operator > (const gString&) const;
    int operator > (const vchar*) const;
    int operator > (const char*) const;

    int operator <= (const gString&) const;
    int operator <= (const vchar*) const;
    int operator <= (const char*) const;

    int operator >= (const gString&) const;
    int operator >= (const vchar*) const;
    int operator >= (const char*) const;

    const gString& operator = (const gString&);
    const gString& operator = (const vchar*);
    const gString& operator = (const char*);

    const gString& operator += (const vchar*);
    const gString& operator += (const char*);
    const gString& operator += (gString);
    const gString& operator += (vchar);
    const gString& operator += (char);

    int length (void) const;
    int is_null (void) const;
    int not_null (void) const;

    vchar operator [] (int) const;
    operator vchar* (void) const;
    operator char* (void) const;
    operator const vchar* (void) const;
    operator const char* (void) const;
    const vchar* str (void) const;

    void l_trim (void);
    void r_trim (void);
    void trim (void);
    
  private:
    static vchar blank_[];
    vchar* ptr_;
};

//-----------------------------------------------------------------------------

inline gString::gString (void) : ptr_ (0) { }

inline gString::gString (const gString& that) : ptr_ (0)
				{ put_value (that.ptr_); }

inline gString::gString (const vchar* st) : ptr_ (0)
				{ put_value (st); }

inline gString::gString (const char* st) : ptr_ (0)
				{ put_value ((const vchar*)st); }

inline gString::compare (const char* st) const
				{ return compare ((const vchar*)st); }

inline const gString& gString::operator = (const gString& that)
				{ return *this = that.ptr_; }

inline const gString& gString::operator = (const char* st)
				{ return operator = ((const vchar*)st); }

inline int gString::operator == (const gString& that) const
				{ return compare (that.ptr_) == 0; }

inline int gString::operator == (const vchar* st) const
				{ return compare (st) == 0; }

inline int gString::operator == (const char* st) const
				{ return compare (st) == 0; }

inline int gString::operator != (const gString& that) const
				{ return compare (that.ptr_) != 0; }

inline int gString::operator != (const vchar* st) const
				{ return compare (st) != 0; }

inline int gString::operator != (const char* st) const
				{ return compare (st) != 0; }

inline int gString::operator < (const gString& that) const
				{ return compare (that.ptr_) < 0; }

inline int gString::operator < (const vchar* st) const
				{ return compare (st) < 0; }

inline int gString::operator < (const char* st) const
				{ return compare (st) < 0; }

inline int gString::operator > (const gString& that) const
				{ return compare (that.ptr_) > 0; }

inline int gString::operator > (const vchar* st) const
				{ return compare (st) > 0; }

inline int gString::operator > (const char* st) const
				{ return compare (st) > 0; }

inline int gString::operator <= (const gString& that) const
				{ return compare (that.ptr_) <= 0; }

inline int gString::operator <= (const vchar* st) const
				{ return compare (st) <= 0; }

inline int gString::operator <= (const char* st) const
				{ return compare (st) <= 0; }

inline int gString::operator >= (const gString& that) const
				{ return compare (that.ptr_) >= 0; }

inline int gString::operator >= (const vchar* st) const
				{ return compare (st) >= 0; }

inline int gString::operator >= (const char* st) const
				{ return compare (st) >= 0; }

inline const gString& gString::operator += (const char* st)
				{ return operator += ((const vchar*)st); }

inline const gString& gString::operator += (gString that)
				{ return operator += (that.ptr_); }

inline const gString& gString::operator += (char c)
				{ return operator += ((vchar)c); }

inline int gString::length (void) const
				{ return ptr_ ? vcharLength (ptr_) : 0; }

inline int gString::is_null (void) const
				{ return ptr_ == 0; }

inline int gString::not_null (void) const
				{ return ptr_ != 0; }

inline vchar gString::operator [] (int i) const
		{ return (i < 0 || !ptr_ || i >= (int)vcharLength (ptr_)) ? '0' : ptr_[i]; }

inline gString::operator vchar* (void) const
				{ return ptr_ ? ptr_ : blank_; }

inline gString::operator char* (void) const
				{ return ptr_ ? (char*)ptr_ : (char*)blank_; }

inline gString::operator const vchar* (void) const
				{ return ptr_ ? ptr_ : blank_; }

inline gString::operator const char* (void) const
				{ return ptr_ ? (const char*)ptr_ : (const char*)blank_; }

inline const vchar* gString::str (void) const
				{ return ptr_ ? ptr_ : blank_; }


#endif // _gString_h
