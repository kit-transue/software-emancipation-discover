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
#ifndef _relArr_h
#define _relArr_h

#include <sys/types.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */


typedef unsigned short sint;

class RelType;
class Obj;

class Relation
{
    public:
       RelType     *rel_type;
       Obj         *members;            // set of all objects (or one)
                                        // related to owner by this type
       RelType *get_rel_type()               {return rel_type; }
       Obj     *get_members()                {return members;  }
       void    put_members(Obj *memb)        {members = memb;}
};

class relArr {
  sint capacity;  
  sint length;
  Relation * pairs;

public:
  relArr() : capacity(0), length(0), pairs(0) {}

  relArr(relArr&);
  ~relArr() 
  {if (pairs) ::free(pairs);}
  
  void free();
  void realloc(int sz);

  Relation * grow(int);
  Relation * append(RelType * new_type, Obj * new_members);
  Relation* operator[](int) const;  // returnes POINTER !!!!!
  Obj *     operator[](const RelType * rt) const;
  unsigned  size() const {return length; }

  void remove(Relation* el);
  void remove(RelType* el);
  void remove (int ind);

  void shrink(int ind,  int len);
  int index(const Relation * el)const;
  int index(const RelType * el)const;
 };

#endif // _relArr_h

/*
   START-LOG-------------------------------------------

   $Log: relArr.h  $
   Revision 1.6 2000/07/10 23:07:05EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2.1.3  1993/04/22  22:19:09  smit
 * check for non null pairs before freeing.
 *
 * Revision 1.2.1.2  1992/10/09  18:58:21  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
