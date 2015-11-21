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
// ObjRef.h.C
//------------------------------------------
// synopsis:
// Member functions of object referencer
//
// description:

// include files
#include "objRef.h"
#include "general.h"

bool objRef::operator==(const objRef& a) const
{
   return this->isEqual(a);
}

int objRef::compare(const Object& ) const
{

   return 1;
}

bool objRef::isEqual (const Object& a) const
{
    return (this->obj == ((objRef&)a).obj);
}

objRef::objRef (void *object)
{
   this->obj = object;
   this->ref_count = 0;

   this->msg = NULL;
   this->prop = NULL;
}

objRef::~objRef()
{
   if (this->msg)
	delete this->msg;

   if (this->prop)
	delete this->prop;
}
   
unsigned objRef::hash() const
{
   return ((unsigned int) this->obj) & 0x000000ff;
}

void objRef::Incr()
{
   this->ref_count++;
}

void objRef::Decr()
{
   this->ref_count--;
}

int objRef::RefCount()
{
   return this->ref_count;
}

void objRef::SetmsgSet (msgSet *msg)
{
   this->msg = msg;
}

void objRef::SetpropSet (propSet *prop)
{
   this->prop = prop;
}

msgSet *objRef::GetmsgSet ()
{
   return this->msg;
}

propSet *objRef::GetpropSet ()
{
   return this->prop;
}

/*

   START-LOG--------------------------------------

   $Log: objRef.cxx  $
   Revision 1.2 1995/07/27 20:20:15EDT rajan 
   Port
Revision 1.2  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.1  1992/10/10  20:56:13  builder
Initial revision


   END-LOG----------------------------------------

*/

