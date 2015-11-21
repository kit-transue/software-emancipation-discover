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
///////////////////////////   FILE objDb.h.c  /////////////////////////////////
//
// -- Contains database calls and methods
//
//----------------------------------------------------------------------------

#include "objRelation.h"
#include "genError.h"

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <cmd.h>

 int Obj::compare(const Object& x) const
               {shouldNotImplement("compare"); return -1; }


Obj::Obj() : kind(0), obj_id(1) {}

Obj::Obj (const Obj &x) : kind(0)
  {
//  Initialize(Obj::Obj);
//  Assert(x.obj_id > 0);		// Should be existent Obj instance
    put_id( x.get_id());
  }

Obj::~Obj()
  {
//  Initialize(Obj::~Obj);
 static  char *_my_name = "Obj::~Obj";
  Assert(get_id() >= -1);
  put_id( -2);
  }

bool Obj::isEqual(const Object& x) const            
                                  {return (void *)this == (void *)&x ;}    
unsigned Obj::hash() const {return (unsigned) this;}
bool Obj::operator==(const Obj& x) const {return (this == &x); }

bool Obj::relationalp() const { return 0;}
int  Obj::collectionp() const { return 0;}
unsigned  Obj::size() const {return 1; }

int Obj::includes(const Obj* ob) const {
  if(ob->collectionp()){
     int size = ob->size();
     if(size > 1) return 0;
     else if(size==0)  return 1;
     else return ob->includes(this);
  } else  {
     return (void *)ob == (void *) this;
  }
}

// validation protocol
//extern int cmd_validate_string_value(const char * newval, const char * oldval);

void  Obj::print(ostream&, int) const {}
void Obj::put_signature(ostream& ostr) const 
{
  print(ostr);
}

validation_function Obj::val_signature() const
 {
   return cmd_validate_string_value;
 }
const char * Obj::objname() const
{
  //    return "Obj";
  return obj_type((void*) this);
}

/* 
   $Log: objDb.h.cxx  $
   Revision 1.9 2000/07/10 23:07:14EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.9  1994/07/26  15:29:02  mg
Bug track: NA
fixed compilation error

Revision 1.2.1.8  1994/02/07  17:40:49  mg
Bug track: 0
cmd_validate

Revision 1.2.1.7  1994/02/03  01:31:44  mg
New validation stuff

Revision 1.2.1.6  1993/07/28  20:26:05  mg
uninlined virtuals

Revision 1.2.1.5  1992/12/09  00:43:05  mg
split kind out of obj_id

Revision 1.2.1.4  1992/12/07  23:25:41  mg
relationalp

Revision 1.2.1.3  1992/11/20  19:31:28  boris
Removed Initialize() macro

*/
