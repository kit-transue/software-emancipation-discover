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
////////////////////////////////   FILE objArr.h.c ////////////////////////
//
// -- Contains methods of objArr class - array of pointers to Objects
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "objArr.h"
#include "objCollection.h"
#include "genError.h"
#include "symbolArr.h"
#include <objString.h>
int objArr::operator==(const objArr& x) const 
{
    if(_length != x._length) return 0;
    for(int ii=0; ii<_length; ++ii)
	if(! ( os_list[ii]->isEqual(* x.os_list[ii]))) return 0;
    return 1;
}

int objArr::operator!=(const objArr& x) const 
{
    if(_length != x._length) return 1;
          
    for(int ii=0; ii<_length; ++ii)
	if(! (os_list[ii]->isEqual(* x.os_list[ii]))) return 1;
    return 0;
}


void objArr::operator=( const objArr& x)
{ os_list = x.os_list; _length = x._length;}

int  objArr::collectionp() const  {return 1;}

int objArr::provide(int sz){
    if(sz < 0)  return 0;
    int len = os_list.size();
    if(sz <= len) return len;

    int new_size = len ? len : 1;
    do new_size *= 2;
    while (new_size < sz);
    os_list.reSize(new_size);
    return new_size;            
}

objArr::objArr(const symbolArr & so) : os_list (so.size ()), _length (0)
{
  Initialize(objArr::objArr);
    int sz = so.size();
    for(int ii=0; ii<sz; ++ii)
          insert_last((Obj *)so[ii]);
}

void objArr::removeAll() {
    for(int ii=0; ii<_length; ++ii)
	os_list[ii] = NULL;
    _length = 0;
}
objArr:: objArr(int sz) : os_list(sz), _length(0){}

objArr:: objArr(const objArr &x) :
os_list(x.os_list), _length(x._length){}

void objArr::insert_first(const Obj* x) {
   Initialize(objArr::insert_first);

   insert_after(-1, x);
}

objArr::objArr(const objSet &x) : os_list(x.size())
{
    Obj * el;
    int ii = 0;
    ForEach(el, x) {
	os_list [ii++] = (Object*) el;
    } 
    _length = x.size();
}

void objArr::insert_after(int loc, const Obj* x) 
//
//      increase Array size by 1, shift elements to the right and add new one
//
  
{
//   if (loc > _length - 1) {
   if (loc + 1 > _length) {
      insert_last(x);
      return;
   }

   if (loc < -1) {
      loc = -1;
   }

    provide(_length + 1);
    for(int ii = _length; ii > loc + 1; --ii)
	os_list[ii] = os_list[ii-1];
    os_list[loc + 1] = (Object *)x;
    ++_length;
}


void objArr::insert_last(const Obj* x) 
//
//      increase Array size by 1 and add a new one at the end
//
{
    provide(_length+1);
    os_list[_length] = (Object *)x;
    ++ _length;
}

//      remove element defined by given pointer

void objArr::remove(const Obj* x)
{
    for(int ii=0; ii < _length; ++ii){
	if (os_list[ii] != (Object *)x ) continue;

	for(; ii < _length -1 ; ++ii ) 
	    os_list[ii] = os_list[ii+1];
	--_length;
	os_list[_length] = NULL;
	break;
    }
}

//      remove element defined by given index

void objArr::remove(int ind)
{
    for(int ii = ind; ii < _length - 1; ++ii ) 
	os_list[ii] = os_list[ii+1];
    --_length;
    os_list[_length] = NULL;
}

Obj * objArr::pop()
{
    if (_length == 0)
	return NULL;
    -- _length;
    Obj *el = (Obj *)os_list[_length];
    os_list[_length] = NULL;
    return el;
}

void objArr::insert_all(const objArr& x)
{
    int new_size = _length + x.size();
    provide(new_size);

    int ii = _length;
    Obj * el;
    ForEach(el, x) {
	os_list[ii] = (Object*) el;
	++ii;
    }
    _length = new_size;
}


        int objArr::includes(const Obj *x) const
//
//      Returns 1 if Obj pointer in question belongs to array
//      (does not count # of occurences)

{
    if(x->collectionp()){
	Obj *el;
	ForEach(el, *x){
	    if(! includes(el)) return 0;
	}
	return 1;
    } else {       
	for(int ii=0; ii < _length; ++ii){
	    if (os_list[ii]-> isEqual(*x )) return 1;
	}
	return 0;
    }
}


void objArr::sort(sortfunctionPtr f) {
    Object **v;
    v = &os_list.elem(0);
    qsort(v, _length, sizeof(Object *), f);
}

objArr::objArr() : os_list(), _length(0) {}
objArr::~objArr(){}

void objArr::put_signature(ostream& os) const
{
  os << size();
}
const char * objArr::objname() const
{
 return "objArr";
}

objString::~objString(){}

// note: f should be  int f(Object **, Object **)
/*
  START-LOG-------------------------------------------

   $Log: objArr.h.cxx  $
   Revision 1.4 2000/07/10 23:07:10EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.11  1994/05/05  21:11:00  aharlap
*** empty log message ***

Revision 1.2.1.10  1994/02/07  17:39:51  mg
Bug track: 0
cmd_validate

Revision 1.2.1.9  1993/07/28  20:26:05  mg
uninlined virtuals

Revision 1.2.1.8  1993/07/10  00:39:58  boris
Fixed objArr::insert_after() that did not work for insert_first();

Revision 1.2.1.7  1993/07/02  13:57:32  wmm
Fix bug 3810 by only extracting superclass members if requested by the
user, and then only for the selected classes.

Revision 1.2.1.6  1993/05/24  01:00:07  wmm
Fix bugs 3244, 3243, 3252, and 3152.

Revision 1.2.1.5  1992/12/15  01:04:27  aharlap
fixed copy constructor from symbolArr

Revision 1.2.1.4  1992/12/09  20:44:10  aharlap
added interface for new xref

Revision 1.2.1.3  1992/11/03  20:47:00  oak
Stopped 'remove' routine from reading off the
end of an object array.

Revision 1.2.1.2  1992/10/09  18:55:14  boris
Fix comments

  
   History:   04/17/91     S.Spivakovksy      Initial coding.
              08/05/91	   S.Spivakovsky      Rewrote to switch to NIH.
              09/03/91     S.Spivakovksy      More NIH related changes.
              09/12/91     S.Spivakovksy      Minor clean up (negative cases).
              02/12/92     S.Spivakovksy      Added remove by index method.
              06/22/92     ranger             Added sort method


   END-LOG---------------------------------------------

*/
