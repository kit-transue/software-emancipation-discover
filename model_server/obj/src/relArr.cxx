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

#include "genError.h"
#include "relArr.h"  
#include "objCollection.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <psetmem.h>
#include <machdep.h>

// Copy constructor
relArr::relArr(relArr & s)
  {
  length = capacity = s.length;
  pairs = capacity ? (Relation *)psetmalloc(capacity * sizeof(Relation)) : 0;
  if(length)
    OS_dependent::bcopy(s.pairs, pairs, length * sizeof(Relation));
  }

void relArr::free()
     {if (pairs) ::psetfree(pairs); 
      length=0; 
      capacity=0; 
      pairs=NULL;
     }

  void relArr::realloc(int sz)
     {
      int rem = sz % 4; 
      if(rem) sz += (4-rem); 
      capacity=sz;
      int byte_size = sz * sizeof(Relation);
      if(pairs) 
              pairs = (Relation*) ::psetrealloc((char *)pairs, byte_size);
      else
              pairs = (Relation*) ::psetmalloc(byte_size);
     }

  Relation* relArr::operator[](int ind) const
     { 
       if(ind < 0 || ind >= int(length)) return NULL; // Added cast to length.
       else return pairs + ind;
     }

  Obj *     relArr::operator[](const RelType * rt) const
    {
     for(int ii=0; ii<int(length); ++ii) // Added cast to length.
       if(pairs[ii].rel_type==rt)return pairs[ii].members;  
     return NULL;
    }

  Relation * relArr::grow(int sz)
    {
      int new_size = length + sz;
      if(new_size > int(capacity)) realloc(new_size);  // Added cast to capacity.
      length = new_size;
      return pairs + (length - sz);
    }

  Relation * relArr::append(RelType * new_type, Obj * new_members)
   {
      Relation * new_rel = grow(1);
      
      new_rel->rel_type = new_type;
      if(new_members->collectionp()) {
          new_rel->members = db_new(objSet,(new_members));
      } else {
          new_rel->members = new_members;
      }
      
      return(new_rel);    
   }

  void relArr::remove(Relation* el){ remove(index(el)); }
  void relArr::remove(RelType* el){ remove(index(el)); }

  void relArr::remove (int ind) 
    {
      if((ind < 0) || ind >= int(length)) return; // Added cast to length.
      shrink(ind, 1);
    }
  void relArr::shrink(int ind,  int len)
    { 
      int new_length = length - len;
      int ii, jj;
      for(ii=ind, jj=ind+len; ii<new_length; ++ii, ++jj) pairs[ii] = pairs[jj];
      length = new_length;
    }
  int relArr::index(const Relation * el) const
    {
      int ii = el - pairs;
      if(ii>=int(length)) return -1; // Added cast to length.
      return ii;
    }
    
  int relArr::index(const RelType * rt) const
    {
      for(int ii=0; ii<int(length); ++ii)  // Added cast to length.
	  if(rt==pairs[ii].rel_type) return ii;
      return -1;
    }
 
/*
   START-LOG-------------------------------------------

   $Log: relArr.cxx  $
   Revision 1.6 2000/07/10 23:07:26EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.6  1994/01/13  03:06:41  builder
*** empty log message ***

Revision 1.2.1.5  1994/01/13  01:09:55  kws
Use psetmalloc

Revision 1.2.1.4  1993/07/22  16:45:37  harry
HP/Solaris Port

Revision 1.2.1.3  1993/04/22  22:16:26  smit
check for pairs before freeing it.

Revision 1.2.1.2  1992/10/09  18:55:38  boris
Fix comments


   END-LOG---------------------------------------------

*/
