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
#include <DataExchange.h>
#include <Interpreter.h>
#include <symbolArr.h>
#include <machdep.h>

init_relational(DataExchange, Relational);
init_relational(DataField, Relational);

DataExchange::~DataExchange()
{
}

DataField::~DataField()
{
}

int DataExchange::Get(void *p, const char *key, genString &s)
{
  int ret = 0;

  DataField *field = Find(key);
  if (field) {
    field->Get(p, s);
    ret = 1;
  }

  return ret;
}

int DataExchange::Set(void *p, const char *key, genString &s)
{
  int ret = 0;

  DataField *field = Find(key);
  if (field) {
    field->Set(p, s);
    ret = 1;
  }

  return ret;
}

int DataExchange::Read(void *p, const char *key, genString &s)
{
  int ret = 0;
 
  DataField *field = Find(key);
  if (field) {
    field->Read(p, s);
    ret = 1;
  }

  return ret;
}

int DataExchange::Write(void *p, const char *key, genString &s)
{
  int ret = 0;

  DataField *field = Find(key);
  if (field) {
    field->Write(p, s);
    ret = 1;
  }

  return ret;
}

DataField *DataExchange::Find(const char *key)
{
  DataField *field = (DataField *)obj_search_by_name(key, fields);
  return field;
}

void DataExchange::Add(DataField *field)
{
  fields.insert(field);
}

void DataExchange::Remove(DataField *field)
{
  fields.remove(field);
}

void DataExchange::Remove(const char *key)
{
  DataField *field = Find(key);
  if (field)
    fields.remove(field);
}

objArr *DataExchange::GetAllFields()
{
  return &fields;
}

int DataExchange::GetFieldCount()
{
  return fields.size();
}

void DEX_Bool(int m, genString &s, int &v)
{
  if (m == DEX_SET || m == DEX_WRITE) {
    if (OSapi_strcasecmp(s, "1")      == 0  ||
        OSapi_strcasecmp(s, "true")   == 0  ||
        OSapi_strcasecmp(s, "yes")    == 0  ||
        OSapi_strcasecmp(s, "on")     == 0  ||
        OSapi_strcasecmp(s, "indeed") == 0)
      v = 1;
    else
      v = 0;
  } else {
    if (v == 0)
      s = "0";
    else
      s = "1";
  }
}

void DEX_String(int m, genString &s, genString &v)
{
  if (m == DEX_SET || m == DEX_WRITE) {
    v = s;
  } else {
    s = v;
  }
}

void DEX_Integer(int m, genString &s, int &v)
{
  if (m == DEX_SET || m == DEX_WRITE) {
    v = OSapi_atoi((char *)s);
  } else {
    s.printf("%d", v);
  }
}

extern const char *ATT_etag(symbolPtr&);
extern int parse_selection(char const *, symbolArr &);

void DEX_Symbols(int m, genString &s, symbolArr &v)
{
  Interpreter *shell = GetActiveInterpreter();

  if (shell) {
    if (m == DEX_SET) {
      v.removeAll();
      shell->ParseArgString((char *)s, v);
    } else if (m == DEX_WRITE) {
      v.removeAll();
      parse_selection(s, v);
    } else if (m == DEX_GET) {
      DI_object obj;
      DI_object_create(&obj);
      symbolArr *syms = get_DI_symarr(obj);
      if (syms) {
        int sz = v.size();
        for (int i = 0; i < sz; i++) {
          syms->insert_last(v[i]);
        }
        // It is unfortunate that we have to send the result
        // directly to the interpreter from this routine,
        // instead of performing a proper serialization. But
        // for objects, serialization doesn't really make
        // sense. Using true Tcl objects to capture the
        // symbols here doesn't work for us because the
        // client isn't necessarily expecting to see a list.
        // And even if it were, streaming multiple types of
        // objects along with symbols onto the same Tcl list
        // object loses the symbols anyway!   

        shell->SetDIResult(obj);

        // int len;
        // s = Tcl_GetStringFromObj(obj, &len);
      }
    } else if (m == DEX_READ) {
      int sz = v.size();
      if (sz > 1) {
        s += "\n";
        for (int i = 0; i < sz; i++) {
          s += ATT_etag(v[i]);
          s += "\n";
        }
      } else if (sz == 1) {
        s += ATT_etag(v[0]);
      }
    }
  }
}

void DEX_SymCnt(int m, genString &s, symbolArr &v)
{
  if (m == DEX_GET || m == DEX_READ) {
    s.printf("%d", v.size());
  }
}

void DataField::GetTypeNameShort(genString &typeName)
{
  if (vt)
    typeName = vt->get_name();
}

void DataField::GetTypeNameLong(genString &typeName)
{
  if (vt) {
    TclList list;
    vt->Print(list);
    typeName = list;
  }
}

int DataField::IsSetObj()
{
  return (vt->btype() == WfValueType::SET);
}
