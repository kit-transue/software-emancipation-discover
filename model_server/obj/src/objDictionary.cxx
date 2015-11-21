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
#include <objDictionary.h>
#include <objArr.h>

#define MAKE_POS(v) (v & 0x7fffffff) /* make number positive */

unsigned string_hash(const char * sym)
{
   register int r = 0;

   unsigned const char * cur = (const unsigned char *)sym;
   while(cur[0])

     r = (r * 113) + *cur++;

   return MAKE_POS(r);
}

bool str_equal (const char * st1, const char * st2)
{
  if((! st1) && (! st2)) return 1;
  if((! st1) || (! st2)) return 0;
  return (strcmp(st1, st2)==0);
}

init_relational(namedString,Relational);
init_relational(namedInt,Relational);
init_relational(objDictionary,Relational);
init_relational(objCharPtr,Relational);
init_relational(objNamed,Relational);


bool objNamed::isEqual(const Object& oo) const
{
  underInitialize(objNamed::isEqual);
   Relational *rp = RelationalPtr(&oo);
   Assert(rp->relationalp());
   char const *n1 = this->get_name();
   char const *n2 = rp->get_name();
   return str_equal(n1, n2);
}

objNamed::~objNamed()
{
  this->rem_rel_all();
}

unsigned objNamed::hash() const
{
  return  string_hash(get_name());
}

char const *objCharPtr::get_name() const
{
  return  entry;
}

char const * objDictionary::get_name() const
{
  return  entry.str();
}

Relational * obj_search_by_name(const char * entry, objSet&dict)
{
    static objCharPtr od(0);
    od.entry = (char*)entry;
    Object * oo = dict.os_list.findObjectWithKey(od);
    return RelationalPtr(oo);
}

Relational* obj_search_by_name(const char * entry, objArr& oa)
{
 Initialize(obj_search_by_name);

  int sz = oa.size();
  for(int ii=0; ii<sz; ++ii){
     Relational* rp = RelationalPtr(oa[ii]);
     Assert(rp);
     Assert(rp->relationalp());
     char const *name = rp->get_name();
     if(str_equal(entry, name))
        return rp;
  }

  return NULL;
}

void namedInt::set_value (int val) 
{
    value = val;
}

int namedInt::get_value () 
{
    return value;
}

void namedString::set_value (const char *val)
{
    value = val;
}

const char *namedString::get_value ()
{
    return value;
}
