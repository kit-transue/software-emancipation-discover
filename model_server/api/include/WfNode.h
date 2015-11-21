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
#ifndef _WfNode_h
#define _WfNode_h

#include <Relational.h>
#include <objArr.h>
#include <Interpreter.h>
#include <AccessCatalog.h>
#include <genString.h>
#include <TclList.h>

class WfNodeCatalog : public AccessCatalog
{
  public:
    virtual int AppendItemCmd(int, char const *[], genString &);
    virtual int DescribeCmd(int, char const *[], genString &);
};

class WfParamCatalog : public AccessCatalog
{
  public:
    virtual int AppendItemCmd(int, char const *[], genString &);
    virtual int DescribeCmd(int, char const *[], genString &);
};

typedef enum {
  WF_DEFAULT,
  WF_DUMP
} WfPrintCode;

typedef genString WfCommand;

class WfValueType : public objDictionary {

  public:

  enum CODE {
      INT,
      STRING,
      ENUM,
      SET,
      BOOL
    };

    WfValueType(char const*nm);

    virtual CODE btype() const = 0;
    char const* bname() const;
    virtual bool validate(char const* /* value */) const {return true;}
    virtual bool isDerived() const {return false;}
    bool is(WfValueType*) const;
    bool is(char const*name) const;
    bool Print(TclList &);
    static WfValueType* lookup(char const*name);
    static int list(ostream& = cout);
  private:

    static objArr catalog;

};

class WfDerivedType : public WfValueType {

 WfCommand _validate;
 
public:
    void set_validate(char const*val) { _validate = val;}

    virtual CODE btype() const {return getBase()->btype();}
    virtual bool isDerived() const {return true;}
    WfDerivedType(char const* nm, WfValueType*base);
    WfDerivedType(char const* nm, WfValueType*base, char const*validator);
    virtual bool validate(char const*value) const;

    WfValueType* getBase() const;
    WfValueType* getSuper() const;
};

defrel_one_to_many(WfValueType,base,WfDerivedType,derived);
inline    WfValueType* WfDerivedType::getSuper() const
{ return derived_get_base(this);}

class WfSetType : public WfValueType {
 public:
    virtual CODE btype() const {return SET;}
    WfSetType(char const* nm) : WfValueType(nm) {}
};

class WfIntType : public WfValueType {
 public:
    virtual CODE btype() const {return INT;}
    WfIntType(char const* nm) : WfValueType(nm) {}
    virtual bool validate(char const*value) const;
};
class WfBoolType : public WfValueType {
 public:
    virtual CODE btype() const {return BOOL;}
    WfBoolType(char const* nm) : WfValueType(nm) {}
    virtual bool validate(char const*value) const;
};

class WfStringType : public WfValueType {
 public:
  virtual CODE btype() const {return STRING;}
  WfStringType(char const* nm) : WfValueType(nm) {}
};

class WfEnumType : public WfValueType {
 public:
  virtual CODE btype() const {return ENUM;}
  WfEnumType(char const* nm, char const*val, bool f) 
    : WfValueType(nm), eval_flag(f), list(val) {}
  
  int get_list(genString& val);

 private:
  bool eval_flag;
  genString list;
};

class WfParam : public objDictionary {

  public:

    typedef enum {
      M_OUT = 0x01,    // match outputs (default)
      M_INP = 0x02,    // match previous inputs
      M_TAG = 0x04     // only match when the tag is identical
    } MatchCode;

    WfParam(char const *, char const *, char const *, char const *, WfValueType *, int);

    static bool Init();
    static WfParam *Find(char const *);

    define_relational(WfParam, objDictionary);
    virtual void print(ostream& = cout, int = 0) const;

    static WfParamCatalog *catalog;

    genString def;
    genString vname;
    genString info;
    WfValueType *vt;
    int mc;

};

//defrel_one_to_one(WfParam,param,WfValueType,valuetype);

class WfNode : public objDictionary {

  public:

    WfNode(char const *, char const *, char const *, objArr&, objArr&, objArr&);
 
    static bool Init();
    static WfNode *Find(char const *);

    int GetInputs(objArr&) const;
    int GetOutputs(objArr&) const;
    int GetOptionals(objArr&) const;
    
    static WfNodeCatalog *catalog;
    
    define_relational(WfNode, objDictionary);
    virtual void print(ostream& = cout, int = 0) const;

    WfCommand command;
    genString info;

  private:

    objArr inputs;
    objArr outputs;
    objArr optionals;

};

class WfParamInstance : public Relational {

  public:

    WfParamInstance(WfParam *);

    static bool Equal(WfParamInstance *, WfParamInstance *);
    bool Ready() const;
    void Reset();
    void Revive();
    void SetVal(char const *);
    void SetDIVal(DI_object);
    void SetValLiteral(char const *);
    char const *GetStrVal() const;
    DI_object const GetDIVal() const;
    void Unset();
    void operator=(const WfParamInstance &);
    void Print(TclList &, WfPrintCode, char const *, bool = false);

  private:
    genString str_value;
    DI_object set_value;
    bool      active;
};

defrel_one_to_many(WfParam,param,WfParamInstance,instance);

class WfNodeInstance : public Relational {

  public:

    WfNodeInstance(WfNode *);
    WfNodeInstance(WfNode *, objArr &, objArr &, objArr &);
    ~WfNodeInstance();

    bool Ready() const;
    void Reset();
    void Destroy();  
    int  Run(Interpreter *);
    void Print(TclList &, WfPrintCode, bool = false, bool = false);
    void GetLastError(genString &err_str, genString &err_info);

    objArr inputs;
    objArr outputs;
    objArr optionals;

    int       retcode;
    genString error_string;
    genString error_info;
};

class WfSymbolArr : public Relational {

  public:

    WfSymbolArr(symbolArr * = NULL);
    symbolArr *arr;

};

defrel_one_to_many(WfNode,node,WfNodeInstance,instance);
defrel_many_to_one(WfParamInstance,pi,WfSymbolArr,symarr);

#endif /* !_WfNode_h */
