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
#ifndef _genWild_h
#define _genWild_h

//------------------------------------------
// genWild.h
//------------------------------------------

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef _RegExp_h
#include <RegExp.h>
#endif

// explaration for the next enum is in the source file genWild.h.C
enum
{
    COMPLICATE_LEVEL_0 = 0,
    COMPLICATE_LEVEL_1,
    COMPLICATE_LEVEL_2,
    COMPLICATE_LEVEL_3,
    COMPLICATE_LEVEL_4
};

// class WildRegexp extends the semantics of Regexp by converting
// un-quoted patterns into shell-glob (i.e. wildcard) patterns.
class WildRegexp : public Regexp
{
  public:
    WildRegexp(const char*);
    WildRegexp(const char*, int complicate);
    ~WildRegexp();
};


class wild_buf
{
  public:
    Regexp*         ptr;
    genString       wild;

    wild_buf(char*);
    ~wild_buf();

    int match(const char* str, int size);
};

typedef wild_buf* wild_ptr;

inline int wild_buf::match(const char* str, int size)
{
    return ptr->Match(str, size, 0);
}


extern const char* glob_to_regexp(const char* p, int complicate);
void gen_wild_put(char*, wild_ptr*, int&);
int  gen_wild_match(char*, wild_ptr*, int&);
void gen_wild_print(wild_ptr*, int);


/*
   START-LOG-------------------------------------------

   $Log: genWild.h  $
   Revision 1.5 1995/07/27 20:21:33EDT rajan 
   Port
 * Revision 1.2.1.7  1994/07/13  15:49:29  so
 * Bug track: 7750
 * fix bug 7750
 *
 * Revision 1.2.1.6  1994/05/23  20:43:22  so
 * fast projHeader::fn_to_ln
 *
 * Revision 1.2.1.4  1993/05/07  13:35:55  so
 * Skip the backslash.
 *
 * Revision 1.2.1.3  1993/05/01  20:39:33  glenn
 * Add class WildRegexp.  Remove glob_to_regexp.
 *
 * Revision 1.2.1.2  1993/04/30  19:20:56  glenn
 * Add glob_to_regexp.
 * Add destructor for wild_buf.
 *
   END-LOG---------------------------------------------
*/

#endif // _genWild_h
