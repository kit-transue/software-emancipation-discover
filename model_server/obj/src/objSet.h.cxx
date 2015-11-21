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
////////////////////////////////   FILE objSet.h.c ////////////////////////
//
// -- Contains Collection related methods defined outside classes
//
#include <objSet.h>
#include <objCollection.h>
#ifndef _objArr_h
#include <objArr.h>
#endif

     objSet::objSet(Obj * el){
       if(! el) return;
       int coll = el->collectionp();
       if(coll==2){
          os_list = ((objSet *)el) -> os_list;
       } else if(coll){
          Obj*el1;
          int sz = el->size();
          if(sz==0) return;
          os_list = Set(sz);
          ForEach(el1, *el) 
              os_list.add(*el1);
       } else {
         os_list = Set(2);
         os_list.add(*el);
       }
     }
        int  objSet::collectionp() const {return 2;}
        void objSet::remove(objSet *x)  {os_list = os_list-x->os_list;}
        void objSet::remove_all () {os_list.removeAll();}
        int  objSet::includes(const Obj *x)const {return os_list.includes(*x);}
        unsigned objSet::size() const      {return os_list.size();}
        int objSet::empty() const {return (os_list.size() ? 0 : 1);}   

   void objSet::remove(const Obj *x) {
         if(x->collectionp())
             os_list = os_list - ((objSet*)x)->os_list;
         else
             os_list.remove(*x);
   }

   void  objSet::insert(Obj *x)  {
        if(x->collectionp()) 
                    insert((objSet*)x);
        else 
                    os_list.add( *x);
   }
 
   void objSet::insert(objSet *x) {
      Obj * el;
      ForEach(el,*x){
           insert(el);
      }
   }

void objSet::purge()  //  delete all set's elements.
{         
    Obj *el;
    ForEach( el, *this ) {
	delete el;
    }

    // boris May 11,1994  resets size to zero
    remove_all ();
}

        objSet objSet::operator&(const objSet& x) const
        {         objSet tmp;
                  tmp.os_list = (os_list & x.os_list);
                  return tmp;
        }

        objSet objSet::operator|(const objSet& x) const 
        {         objSet tmp;
                  tmp.os_list = (os_list | x.os_list);
                  return tmp;
        }

        objSet objSet::operator-(const objSet& x) const
        {         objSet tmp;
                  tmp.os_list = (os_list - x.os_list);
                  return tmp;
        }


//
// Useful for iterating over quasi-set (Obj*) returned by get_relation.
// Example:
//
// {objSet all_of_them;
//      ForEach(one_of_them, all_of_them=get_relation(dear_ones, me)) {
//	 dump(one_of_them);
//      }
//


       objSet& objSet::operator=(Obj* x)
       {
          objSet empty_set;
          if (x == NULL)             *this = empty_set;
          else if (x->collectionp()) *this = *((objSet*) x);
          else {
               *this = empty_set;
               os_list.add(*x);
          }
          return(*this);
       }

oneIter::oneIter(const Obj *const oo) : oc( oo), 
                       it(*( 
                                 (oo && ((Obj *)oo)->collectionp()) 
                               ? & (((objArr*)oo)->os_list)
                               : (ArrayOb*) &oc
                            )) {}

objSet::~objSet(){}  // cfront can not inline

objSet objSet::operator&=(const objSet& x) 
{os_list = os_list & x.os_list; return *this;}
           
objSet objSet::operator|=(const objSet& x) 
{os_list = os_list| x.os_list; return *this;}
          
objSet objSet::operator-=(const objSet& x) 
{os_list = os_list-x.os_list;  return *this;}

void objSet::put_signature(ostream& os) const
{
  os << size();
}
const char * objSet::objname() const
{
 return "objSet";
}


/*
   START-LOG-------------------------------------------

   $Log: objSet.h.cxx  $
   Revision 1.3 1998/08/10 18:20:17EDT pero 
   port to VC 5.0: removal of typename, or, etc.
Revision 1.2.1.6  1994/05/17  19:11:32  boris
Bug track: Scripting
Fixed a number of scripting problems

Revision 1.2.1.5  1994/03/22  20:14:05  mg
Bug track: 1
validation for dormant code

Revision 1.2.1.4  1994/02/07  17:42:42  mg
Bug track: 0
cmd_validate

Revision 1.2.1.3  1993/07/28  20:26:05  mg
uninlined virtuals

Revision 1.2.1.2  1992/10/09  18:55:32  boris
Fix comments

   History:  08/05/91   S.Spivakovksy   Initial coding (former objCollection.h)
   History:  10/09/91   S.Spivakovksy   Added purge set; moved inline methods.

   END-LOG---------------------------------------------

*/

