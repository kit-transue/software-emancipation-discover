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
/*
 * S Y M _ H A S H . C
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mikhail Gurevich and Mark B. Kaminsky
 *
 * This file contains the interface to the Disover hash routinues
 * used by the SQL parser for its symbol tables.
 *
 * TBD: need to add a symhash_delete().
 */

#include "sql_symbol.h"		// symbol table entry

#include <malloc.h>		// system alloc
#include <Hash.h>		// Discover nameHash routines

// routine for used by nameHash for realloc
char* psetrealloc_handler(char* old_ptr, unsigned size)
{
   return ( (char*)realloc(old_ptr, size));
}

// routine for used by nameHash for allocate
char *psetmalloc_handler (unsigned sz)
{
     return (char *) malloc(sz);
}

// class symhash
// derived from Discover nameHash
//
// Use sym_key as hash key (name)
// 
class symhash : public nameHash
{
  virtual const char* name(const Object* oo) const 
    { return ((SQL_SYM*) oo) ->sym_key;}

  virtual bool isEqualObjects(const Object& o1, const Object&o2) const
    {return &o1 == &o2;}

};

// create a new hash table
extern "C" symhash * symhash_create ()
{
  return new symhash;
}

// lookup name in hash table
extern "C" SQL_SYM * symhash_lookup (const char * name, symhash * h)
{  
  int idx = 0;      
  Object*cur = 0;
  h->find(name, idx, cur);
  return (SQL_SYM*) cur;
}

// add a symbol to hash table
extern "C" void symhash_add (SQL_SYM * sym, symhash * h)
{
  h->add(*((Object*)sym));
}


