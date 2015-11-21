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
#ifndef __objRef_h
#define __objRef_h

#include "msgSet.h"
#include "propSet.h"

class objRef : public Obj {

  public:
        bool operator==(const objRef&) const;
        bool operator!=(const objRef& a) const { return !(*this==a); }
        int compare(const Object&) const;
        unsigned hash () const;

private:
    void *obj;             // pointer to object itself
    int  ref_count;        // times referenced
    msgSet *msg;           // messages for this reference
    propSet *prop;         // Properties for this object

public:
    objRef (void *object); // constructor
 private:
    objRef(const objRef& r);// : Obj (r)
    //      { 
    //	obj = r.obj;
    //	ref_count = r.ref_count;
    //	msg = r.msg;
    //	prop = r.prop;
    //      }
 public:
    ~objRef ();            // Destructor
    bool isEqual (const Object& a) const;
    void Incr ();
    void Decr ();
    int RefCount();

    void SetmsgSet (msgSet *msg);    // sets message object for this reference
    void SetpropSet (propSet *prop); // sets property object for this reference
    msgSet *GetmsgSet ();            // gets message object of this reference
    propSet *GetpropSet ();          // gets property object of this reference
};

/*
   START-LOG-------------------------------------------

   $Log: objRef.h  $
   Revision 1.1 1993/07/28 19:45:21EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  18:53:12  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
