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
#ifndef _DataExchange_h
#define _DataExchange_h

#include <objArr.h>
#include <genString.h>
#include <WfNode.h>

class DataField;

class DataExchange : public objDictionary
{

  public:

                             DataExchange(const char *name = NULL)
                               : objDictionary(name) {}
 private:
    DataExchange(const DataExchange &that);
    //			       : objDictionary(that) { *this = that; }
 public:
    virtual                  ~DataExchange();

    DataExchange&            operator=(const DataExchange &that)
                               { fields = that.fields; return *this; }

    virtual int              Get(void *, const char *, genString &);
    virtual int              Set(void *, const char *, genString &);
    virtual int              Read(void *, const char *, genString &);
    virtual int              Write(void *, const char *, genString &);
    virtual DataField*       Find(const char *);
    virtual void             Add(DataField *);
    virtual void             Remove(DataField *);
    virtual void             Remove(const char *);
    virtual objArr*          GetAllFields();
    virtual int              GetFieldCount();

    define_relational(DataExchange, Relational);

  protected:

    objArr                   fields;

}; generate_descriptor(DataExchange, Relational);

typedef enum {
  DEX_DERIVED   = 0x1,
  DEX_INVISIBLE = 0x2
} DataFieldFlag;

enum {
  DEX_GET,
  DEX_SET,
  DEX_READ,    // get persistent
  DEX_WRITE    // set persistent
};

class DataField : public objDictionary
{

  public:

    typedef void (*DataFieldFunc) (int, genString &, void *);

  public:

                             DataField(const char *name, DataFieldFunc f, WfValueType *v,
                                       const char *t, int mask = 0)
			       : objDictionary(name), fn(f), vt(v), title(t), flags(mask) {}
 private:
    DataField(const DataField &that);
    //    : objDictionary(that) { *this = that; }
 public:
    virtual                  ~DataField();

    DataField&               operator=(const DataField &that)
                             {
                               fn    = that.fn;
                               vt    = that.vt ;
                               title = that.title;
                               flags = that.flags;
                               return *this;
			     }

    virtual void             Get  (void *ptr, genString &s)   { (*fn)(DEX_GET,   s, ptr); }
    virtual void             Set  (void *ptr, genString &s)   { (*fn)(DEX_SET,   s, ptr); }
    virtual void             Read (void *ptr, genString &s)   { (*fn)(DEX_READ,  s, ptr); }
    virtual void             Write(void *ptr, genString &s)   { (*fn)(DEX_WRITE, s, ptr); }

    virtual void             GetTypeNameShort(genString &);
    virtual void             GetTypeNameLong(genString &);

    virtual int              IsSetObj();

    define_relational(DataField, Relational);

    DataFieldFunc            fn;
    WfValueType*             vt;
    genString                title;
    int                      flags;

}; generate_descriptor(DataField, Relational);

void DEX_Bool    (int, genString &, int &);
void DEX_String  (int, genString &, genString &);
void DEX_Integer (int, genString &, int &);
void DEX_Symbols (int, genString &, symbolArr &);
void DEX_SymCnt  (int, genString &, symbolArr &);

#endif /* ! _DataExchange_h */
