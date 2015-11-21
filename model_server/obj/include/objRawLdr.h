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
#ifndef _objRawLdr_h
#define _objRawLdr_h

//------------------------------------------
// objRawLdr.h
//------------------------------------------
//
// class:
//     objRawLdr
//
// description: app header for arbitrary Raw UNIX ascii file.
//

#ifndef _objOper_h
#include "objOper.h"
#endif

#include "ldrHeader.h"
#include "objRawApp.h"

class objRawLdr : public ldrHeader{
  public:
   virtual void build_selection(const ldrSelection&, OperPoint&);
   
   define_relational(objRawLdr,ldrHeader);

   objRawLdr(objRawAppPtr, void *);

#ifdef ___GNUG__
   objRawLdr(const objRawLdr& oo) { *this = oo ;}
#endif
   copy_member(objRawLdr);

   void insert_obj(objInserter *, objInserter *);
   void remove_obj(objRemover *, objRemover *);
};
   generate_descriptor(objRawLdr,ldrHeader);

#endif 

/*
   START-LOG-------------------------------------------

   $Log: objRawLdr.h  $
   Revision 1.2 1997/05/21 12:08:19EDT twight 
   Bug track: 14168
   Reviewed by: mb
   
   Changed parameter from 'ldrSelection& const' to 'const ldrSelection&'.
// Revision 1.1  1992/12/11  22:10:17  boris
// Initial revision
//

   END-LOG---------------------------------------------

*/
