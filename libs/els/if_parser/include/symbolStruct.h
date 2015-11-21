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
#ifndef _symbolStruct_h
#define _symbolStruct_h

#include <stdlib.h>

#ifndef _locationStruct_h
#include <locationStruct.h>
#endif

struct symbol 
{
  char           *name;     /* could be a filename */
  char           *kind;
  locationStruct *loc;
  int            generated;
  int            replaceable;
#ifdef __cplusplus
  symbol (const symbol &);
  symbol (void);
  ~symbol(void);
  symbol &operator = (const symbol &);            
#endif
#if defined(__cplusplus) && ! defined(USING_PURIFY)
  static void *operator new (size_t);
  static void  operator delete (void *);
#endif
};

typedef struct symbol symbolStruct;

#ifdef __cplusplus

extern "C"
{
  symbolStruct *CreateSymbolStruct(void);
  void          DestroySymbolStruct(symbolStruct *);
#ifdef COUNT_STRUCTS
  void            ReportSymbolStructs();
#endif
}

#else

  symbolStruct *CreateSymbolStruct();
  void          DestroySymbolStruct();
#ifdef COUNT_STRUCTS
  void            ReportSymbolStructs();
#endif

#endif

#endif
