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
#ifdef COUNT_STRUCTS
#include <iostream.h>
#endif

#include <parray.h>
#ifdef USE_EXTERN_LIB
#include <pdustring.h>
#include <pdumem.h>
#else
#include <pdutil.h>
#endif
#include <symbolStruct.h>
#include <locationStruct.h>

#define SYMBOL_STRUCTS_TO_ALLOCATE 30000

static parray freeSyms(SYMBOL_STRUCTS_TO_ALLOCATE);

#ifdef COUNT_STRUCTS
static int max_count = 0;
static int current_count = 0;
#endif

symbol::symbol(void)
{
}

symbol::symbol(const symbol &other)
{
  name        = pdstrdup(other.name);
  kind        = pdstrdup(other.kind);
  replaceable = other.replaceable;
  generated   = other.generated;
  if ( other.loc )
    loc = new location(*other.loc);
  else
    loc = 0;
}

symbol::~symbol(void)
{
  FREE_MEMORY(name);
  FREE_MEMORY(kind);
  FREE_OBJECT(loc);
}

#ifndef USING_PURIFY
// override of new and delete
void *symbol::operator new (size_t)
{
  symbolStruct *retval = 0;

  if ( freeSyms.size() == 0  )
    {
      symbolStruct *more_Syms = ::new symbolStruct [ SYMBOL_STRUCTS_TO_ALLOCATE ];
      if ( more_Syms )
	{
	  for ( int i = 0; i < SYMBOL_STRUCTS_TO_ALLOCATE; i++ )
	    {
	      freeSyms.insert((void *)&more_Syms[i]);
	    }
	}
    }
  
  retval = (symbolStruct *) freeSyms[freeSyms.size()-1];
  freeSyms.remove(freeSyms.size()-1);
  
#ifdef COUNT_STRUCTS
  current_count++;
  if ( current_count > max_count )
    max_count = current_count;
#endif

  return retval;
}  
#endif

symbol &symbol::operator = (const symbol &other)
{
  name        = pdstrdup(other.name);
  kind        = pdstrdup(other.kind);
  loc         = new location(*other.loc);
  generated   = other.generated;
  replaceable = other.replaceable;
  return *this;
}

#ifndef USING_PURIFY
void symbol::operator delete(void *sym)
{
  if ( sym )
    {
      freeSyms.insert((void *)sym);
#ifdef COUNT_STRUCTS
      current_count--;
#endif
    }
}      
#endif

symbolStruct *CreateSymbolStruct(void)
{
  return new symbolStruct;
}

void DestroySymbolStruct(symbolStruct *sym)
{
  delete sym;
}

#ifdef COUNT_STRUCTS
void ReportSymbolStructs(void)
{
  cout << "Maximum number of symbol structs in use at one time: " << max_count << "\n";
  cout << "Current number of symbol structs in use: " << current_count << "\n";
}
#endif

