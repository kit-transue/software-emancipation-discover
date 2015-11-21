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
#ifndef _Properties_h
#define _Properties_h

#include <objSet.h>
#include <objDictionary.h>

typedef namedString Property;

class Properties : public objSet
{

  public:

                             Properties() : objSet(), defaults(NULL) {}
                             Properties(const Properties &that) : objSet(that), defaults(NULL) {}
                             ~Properties() {}

    void                     operator=(const Properties &that) { objSet::operator=(that); defaults = NULL; }

    void                     Put(const char *, const char *);
    void                     Put(Property &);
    const char*              Get(const char *);
    void                     Remove(const char *);
    void                     RemoveAll() {}

    Property*                FindProperty(const char *);
    Properties*              defaults;

};

class namedPropertySet : public objDictionary
{

  public:

    namedPropertySet(const char *name, Properties &props_) : objDictionary(name), props(props_) {}

    void set_value(Properties &props_) { props_ = props; }
    Properties &get_value() { return props; }

    define_relational(namedPropertySet, Relational);
    
  protected:

    Properties props;
};

generate_descriptor(namedPropertySet, Relational);

class GroupedProperties : public objSet
{
    GroupedProperties(const GroupedProperties &that);// : objSet(that) {}
  public:

                             GroupedProperties() : objSet() {}

                             ~GroupedProperties() {}

    void                     operator=(const GroupedProperties &that) { objSet::operator=(that); }

    void                     Put(const char *, const char *, const char *);
    void                     Put(const char *, Properties &props);
    void                     Put(namedPropertySet &);
    const char*              Get(const char *, const char *);
    bool                     Get(const char *, Properties &props);
    void                     Remove(const char *);
    void                     Remove(const char *, const char *);
    void                     RemoveAll() {}

    namedPropertySet*        FindGroup(const char *);

};

#endif /* ! _Properties_h */
