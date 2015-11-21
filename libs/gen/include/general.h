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
#ifndef _general_h
#define _general_h

#define set_nil 0
#ifdef NULL
#undef NULL
#endif
#define NULL 0

#ifndef NEWCPP
#ifndef OLDCPP

#define __new__cpp__test__(x) 'x'
#if __new__cpp__test__(y) != 'y'
#define NEWCPP
#endif
#undef  __new__cpp__test__

#endif
#endif

#if defined(c_plusplus) || defined(__cplusplus)
     inline void Ignore(...){}
#    define CONST const
#else
#    define Ignore(x)
#    define CONST
#endif

#ifdef NEWCPP
#define quote(x) #x
#define paste(x,y) x##y
#define paste3(x,y,z) x##y##z
#define paste4(x,y,z,w) x##y##z##w
#define paste5(x,y,z,w,v) x##y##z##w##v
#else
#define quote(x) "x"
#define paste(x,y) x/* */y
#define paste3(x,y,z) x/* */y/* */z
#define paste4(x,y,z,w) x/* */y/* */z/* */w
#define paste5(x,y,z,w,v) x/* */y/* */z/* */w/* */v
#endif

#if defined(_WIN32) || defined(__cplusplus)
#define __ANSI_CPP__ 1

#ifndef __builtin_bool
#define __builtin_bool
#endif /* __builtin_bool */

#endif

#ifndef __builtin_bool

#if !defined(__cplusplus) || !defined(__GNUG__) || !defined(__GNUC__) || !defined(__GNUC_MINOR__) || !(__GNUC__ > 2 || (__GNUC__==2 && __GNUC_MINOR__ >= 6))
typedef unsigned int bool;
typedef bool boolean;
static CONST boolean false = 0;
static CONST boolean true = 1;
#else
typedef unsigned int boolean;
#endif

#else

/* Windows NT own bool, boolean */
#ifdef _MSC_VER
#include <yvals.h>
#else
#ifdef _WIN32
typedef unsigned char boolean; // compatible with rpcndr.h
#else
typedef bool boolean;
#endif
#endif

#endif /* __builtin_bool */

#endif

