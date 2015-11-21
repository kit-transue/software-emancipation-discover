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
////////////////////////////////   FILE objCollection.h  //////////////////////
//
// -- Contains Collection related Class Definitions                  
#ifndef _objCollection_h
#define _objCollection_h

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "objDb.h"

#include "Set.h"
#include "ArrayOb.h"
#include "Iterator.h"

// single Object Collection to support ForEach for all cases

class oneCollection : public Collection
{
    public:
      const Obj  *obj_ptr;

      virtual bool isEqual(const Object& x) const; // compares pointers
      virtual unsigned hash() const;    // return (unsigned) this

      oneCollection(const Obj * const);
 private:
      oneCollection(const oneCollection& r);//{ *this = r; }
 public:
      ~oneCollection() { };
      Object* doNext(Iterator&) const;

// --- fake methods to make it non-abstract

      virtual Object* add(Object& ) {return 0; }

      Object *& at(int) { return *((Object **)0); }

      const Object *const & at(int) const 
        { return *((const Object **)0); }

      virtual unsigned occurrencesOf(const Object& ) const {return 0;}
      virtual Object* remove(const Object& ) {return 0;}
      virtual void removeAll() {; }
      virtual unsigned size() const {return 0; } 

//    get_id is a fake function; to avoid multiple inheritance
      int get_id() const { cout<<"from oneCollection; should not be called"<<endl;
                           return 0; }
};


#ifndef __objSet
#include <objSet.h>
#endif
#ifdef _objArr_h
#include <objArr.h>
#endif
#ifndef __objArr_Int
#include <objArr_Int.h>
#endif

struct oneIter {
   oneCollection oc;
   Iterator it;
   oneIter(const Obj *const oo);
 };

#define ForEach(ee,coll)                                                   \
    for( oneIter paste(ee,_it)(&(coll));                                   \
         ++ paste(ee,_it).it, ee = (Obj *) paste(ee,_it).it();             \
       )
    
// The following is used to retrieve the first element of a collection:
//    ForEach(ee,coll) break;   
// This method produces a compiler warning.  "End of for not reached..."
// GetFirst() performs the same task w/o the warning.
#define GetFirst(ee,coll) {  oneIter paste(ee,_it)(&(coll)); ++paste(ee,_it).it; ee = (Obj *) paste(ee,_it).it(); }
    

#endif // _objCollection_h

/*
   START-LOG-------------------------------------------

   $Log: objCollection.h  $
   Revision 1.8 2001/01/12 10:57:12EST sschmidt 
   Port to new SGI compiler
 * Revision 1.2.1.4  1993/12/08  19:48:24  trung
 * Bug track: 0
 * change header
 *
 * Revision 1.2.1.3  1993/07/28  20:23:28  mg
 * uninlined virtuals
 *
 * Revision 1.2.1.2  1992/10/09  18:58:12  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
