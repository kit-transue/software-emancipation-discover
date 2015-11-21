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
#ifndef _prototypes_h
#define _prototypes_h

/*
 * prototypes.h
 *
 * Macros for header files that can be used with C and C++.
 *
 * Please Keep this file compatible to C.
 */

#undef PROTOTYPES

#undef PROTO
#undef psetCONST
#undef EXTERN
#undef BEGIN_C_DECL
#undef END_C_DECL


/*          C++ 2.1 (+)  or         C++ 2.0     or         ANSI C   */
#if defined(__cplusplus) || defined(c_plusplus) || defined(__STDC__) && !defined (FLAG_NO_PROTO) 

#  define PROTOTYPES

#  define PROTO(x) x
#  define psetCONST const

/* K&R C */
#else

#  define PROTO(x) ()
#  define psetCONST

#endif


#ifdef __cplusplus

#  define EXTERN extern "C"
#  define BEGIN_C_DECL extern "C" {
#  define END_C_DECL }

#else

#  define EXTERN extern
#  define BEGIN_C_DECL
#  define END_C_DECL

#endif



#endif /* _prototypes_h */
