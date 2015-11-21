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
#include <Properties.h>

init_relational(namedPropertySet, Relational);

Property *Properties::FindProperty(const char *key)
{
  Property *prop = (Property *)obj_search_by_name(key, *this);

  return prop;
}

void Properties::Put(const char *key, const char *value)
{
  Property *prop = FindProperty(key);

  if (prop) {
    prop->set_value(value);
  } else {
    prop = new Property(key, value);
    insert(prop);
  }
}

void Properties::Put(Property &newProp)
{
  Put(newProp.get_name(), newProp.get_value());
}

const char *Properties::Get(const char *key)
{
  const char *value = NULL;

  Property *prop = FindProperty(key);

  if (prop)
    value = prop->get_value();
  else if (defaults)
    value = defaults->Get(key);

  return value;
}

void Properties::Remove(const char *key)
{
  Property *prop = FindProperty(key);
  
  if (prop)
    remove(prop);
}

namedPropertySet *GroupedProperties::FindGroup(const char *group)
{
  namedPropertySet *propset = (namedPropertySet *)obj_search_by_name(group, *this);

  return propset;
}

void GroupedProperties::Put(const char *group, const char *key, const char *value)
{
  namedPropertySet *propset = FindGroup(group);

  if (!propset) {
    Properties props;
    props.Put(key, value);
    propset = new namedPropertySet(group, props);
    insert(propset);
  } else {
    propset->get_value().Put(key, value);
  }
}

void GroupedProperties::Put(const char *group, Properties &props)
{
  namedPropertySet *propset = FindGroup(group);

  if (!propset) {
    Properties props;
    propset = new namedPropertySet(group, props);
    insert(propset);
  } else {
    propset->set_value(props);
  }
}

void GroupedProperties::Put(namedPropertySet &groupset)
{
  Put(groupset.get_name(), groupset.get_value());
}

const char *GroupedProperties::Get(const char *group, const char *key)
{
  const char *value = NULL;

  namedPropertySet *propset = FindGroup(group);

  if (propset)
    value = propset->get_value().Get(key);

  return value;
}

bool GroupedProperties::Get(const char *group, Properties &props)
{
  bool ret = false;

  namedPropertySet *propset = FindGroup(group);

  if (propset)
    props = propset->get_value();

  return ret;
}

void GroupedProperties::Remove(const char *group)
{
  namedPropertySet *propset = FindGroup(group);

  if (propset)
    remove(propset);
}

void GroupedProperties::Remove(const char *group, const char *key)
{
  namedPropertySet *propset = FindGroup(group);  

  if (propset) 
    propset->get_value().Remove(key);
}
