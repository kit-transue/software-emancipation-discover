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
#include "propSet.h"
#include "general.h"

bool propObj::operator==(const propObj& a) const
{
   return this->prop == a.prop;
}

int propObj::compare(const Object& ) const
{

   return 1;
}

bool propObj::isEqual (const Object& a) const
{
    return (*(propObj *)this == *(propObj*)&a) ;
}

propObj::propObj(int id, void *value)
{
   this->prop = id;
   this->value = value;
}

unsigned propObj::hash() const
{
   return this->prop;
}

void *propObj::get_value()
{
   return this->value;
}

void propObj::set_value (void *value)
{
   this->value = value;
}

// ----------- propSet class methods
propSet::~propSet()
{
   purge();
}

void propSet::add_property  (int id, void *value)
{
   propObj *new_prop;

   this->rem_property (id);

   new_prop = new propObj (id, value);

   insert (new_prop);
}

void *propSet::get_value (int id)
{
   propObj propkey(id, NULL);
   propObj *propobj;

   if ( (propobj = (propObj *) os_list.findObjectWithKey(propkey)) != Object::nil )
      return propobj->get_value();
   else
     return NULL;
}

void propSet::rem_property  (int id)
{
   propObj *new_prop;
   propObj *propobj;

   new_prop = new propObj (id, NULL);

   if ( (propobj = (propObj *) os_list.findObjectWithKey(*new_prop)) != Object::nil ) {
      remove (propobj);
      delete propobj;
   }

   delete new_prop;
}

/*
   START-LOG-------------------------------------------

   $Log: propSet.cxx  $
   Revision 1.2 1995/07/27 20:20:19EDT rajan 
   Port
Revision 1.2.1.4  1993/04/22  18:48:42  smit
Fix bug#3466

Revision 1.2.1.3  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:44:19  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
