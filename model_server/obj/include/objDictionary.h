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
#ifndef __objDictionary
#define __objDictionary

// include files

#ifndef __objSet
#include <objSet.h>
#endif
#ifndef _genString_h      
#include <genString.h>
#endif
#ifndef _objRelation_h      
#include <objRelation.h>
#endif      

class objNamed : public Relational  // abstract class 
{
 public:
   ~objNamed();
   define_relational(objNamed,Relational);
   unsigned hash() const;
   bool isEqual(const Object&) const;
   char const *get_name() const = 0;
};
  generate_descriptor(objNamed,Relational);

class objCharPtr : public objNamed
{
 public:
   char* entry;
   define_relational(objCharPtr,Relational);
   objCharPtr(char*en) : entry(en) {}
   char const *get_name() const;
};
  generate_descriptor(objCharPtr,Relational);

class objDictionary : public objNamed
{
    genString entry;
  public:
    define_relational(objDictionary,Relational);
    objDictionary(const char* en) : entry(en) {}
    char const *get_name() const;
};
  generate_descriptor(objDictionary,Relational);
 
Relational * obj_search_by_name(const char * entry, objSet&dict);
Relational* obj_search_by_name(const char * entry, objArr& oa);
//void obj_sort_by_name(objArr& oa);

class namedInt : public objDictionary 
{
    int value;
  public:
    define_relational(namedInt,Relational);
    void set_value(int);
    int get_value();
    namedInt(const char* name, int val);//XXX unused; impl ": objDictionary(name), value(val) {}"
};
  generate_descriptor(namedInt,Relational);

class namedString : public objDictionary 
{
    genString value;
  public:
    define_relational(namedString,Relational);
    void set_value(const char*val);
    const char *get_value();
    namedString(const char* name, const char* val) : objDictionary(name), value(val) {}
};
    generate_descriptor(namedString,Relational);

#endif
