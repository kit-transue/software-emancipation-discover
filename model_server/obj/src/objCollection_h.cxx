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
////////////////////////////////   FILE objCollection_h.C  //////////////////////
//
// sysnopsis : Contains oneCollection class methods.
//
// description: Constructor creates a collection containing a single
//              object. doNext method is used by NIH Iterator to advances
//              the pointer to the next object in collection. In this 
//              case it returns object pointer at the first call and
//              returns 0 at the second call to terminate the For loop.
//
// methods :   oneCollection::oneCollection, oneCollection::doNext;
//
#include "objCollection.h"

bool oneCollection::isEqual(const Object& x) const
                                  {return (void *)this == (void *)&x ;}
unsigned oneCollection::hash() const {return (unsigned) this;}

  oneCollection::oneCollection(const Obj * const oo )
  {
      obj_ptr = oo;
  }

  Object* oneCollection::doNext(Iterator& pos) const
  {
  //  single Object Collection

      if (pos.index++ == 0)                // first call - return Object
          return (Object *)obj_ptr; 

      else                               // second call - end of For Loop
          return 0;
  }
/*
   START-LOG-------------------------------------------

   $Log: objCollection_h.cxx  $
   Revision 1.1 1993/07/28 19:51:47EDT builder 
   made from unix file
Revision 1.2.1.2  1992/10/09  18:55:15  boris
Fix comments

   Revision 1.2.1.1  92/10/07  20:50:27  smit
   *** empty log message ***

   Revision 1.2  92/10/07  20:50:25  smit
   *** empty log message ***

   Revision 1.1  92/10/07  18:21:17  smit
   Initial revision

   Revision 1.1  92/10/07  17:56:55  smit
   Initial revision

   Revision 1.2  92/01/18  21:19:59  aharlap
   *** empty log message ***
   
   Revision 1.1  92/01/18  21:07:02  aharlap
   Initial revision
    
   History:   10/13/91     S.Spivakovksy      Initial coding.


   END-LOG---------------------------------------------

*/
