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
#ifndef	__attribute_h
#define __attribute_h

#include <genString.h>
#include <objDictionary.h>
#include <Relational.h>
#include <symbolPtr.h>
#include <ddSelector.h>
#include <metric.h>

class projModule;

class symbolAttribute : public objDictionary {
 public:
  symbolAttribute(const char*name);
  virtual ~symbolAttribute();
  ddSelector accepted_kinds;

  define_relational(symbolAttribute,objDictionary);

  virtual int test( symbolPtr&) const;
  virtual const char* value( symbolPtr&) const = 0;
  virtual bool boolValue(symbolPtr&) const = 0;

  virtual bool is_int() const; // { return 0;}
  bool is_bool() const  { return is_int();}

  static Obj* get_all_attrs();
  static Obj* get_int_attrs();
  static Obj* get_bool_attrs();
  static Obj* get_string_attrs();

  static symbolAttribute* get_by_name(const char* name);

  static      int once;
  static      int api_init_attrs();
};
RelClass(symbolAttribute);



class stringAttribute : public symbolAttribute {
  public:
    stringAttribute(const char* name);
    virtual ~stringAttribute();

    virtual bool boolValue(symbolPtr& sym) const {return (*value(sym) != '\0');};
    define_relational(stringAttribute,symbolAttribute);
};

RelClass(stringAttribute);

class intAttribute : public symbolAttribute {
  public:
    intAttribute(const char* name);
    virtual ~intAttribute();

    virtual const char* value( symbolPtr& sym) const;
    virtual bool boolValue(symbolPtr& sym) const {return test(sym);};
    virtual bool is_int() const; // { return 1;}
    
    char _buff[1024];
    define_relational(intAttribute,symbolAttribute);
};
RelClass(intAttribute);

class kindAttribute : public  intAttribute {

 public:

  kindAttribute(const char* name, ddKind k, int x, const char*plu);
  ddKind kind;
  int    is_xref;
  genString plural;
  static kindAttribute* lookup(const char*name);
  virtual int test(symbolPtr& sym) const;
  virtual void print(ostream& = cout, int level = 0) const;
  define_relational(kindAttribute,intAttribute);
};
RelClass(kindAttribute);

class xrefAttribute : public  intAttribute {
  public:

  xrefAttribute(const char* name, unsigned int off)
    : intAttribute(name), offset(off){}
  unsigned int offset;
  virtual int test( symbolPtr& sym) const;
  virtual void print(ostream& = cout, int level = 0) const;
  define_relational(xrefAttribute,intAttribute);
};
RelClass(xrefAttribute);

class astAttribute : public  xrefAttribute {

  public:

  astAttribute(const char* name, unsigned int off)
    : xrefAttribute(name,off){}
  virtual int test( symbolPtr& sym) const;
  virtual void print(ostream& = cout, int level = 0) const;
  define_relational(astfAttribute,xrefAttribute);
};
RelClass(astAttribute);


class metricAttribute : public  intAttribute {

  public:

  metricAttribute(const char* name, metricType mtype)
    : intAttribute(name), type(mtype){}
  metricType type;
  virtual int test( symbolPtr& sym) const;
  virtual void print(ostream& = cout, int level = 0) const;
 public:
  metricType get_type() { return type; }
  define_relational(metricAttribute,intAttribute);
};
RelClass(metricAttribute);

class funcIntAttribute : public  intAttribute {

  public:

  funcIntAttribute(const char* name, int (*function)(symbolPtr&))
      : intAttribute(name), func(function) {}
  int (*func)( symbolPtr&);
  virtual int test( symbolPtr& sym) const;
  virtual void print(ostream& = cout, int level = 0) const;
  define_relational(funcIntAttribute,intAttribute);
};
RelClass(funcIntAttribute);

class moduleIntAttribute : public  intAttribute {

  public:

   moduleIntAttribute(const char* name, int (*function)(projModule&))
      :intAttribute(name), func(function) {}
 int (*func)( projModule&);

  virtual int test( symbolPtr& sym) const;
  virtual void print(ostream& = cout, int level = 0) const;
  define_relational(moduleIntAttribute,intAttribute);
};
RelClass(moduleIntAttribute);
#ifdef moduleStringAttribute
class moduleStringAttribute : public  stringAttribute {

  public:

  moduleStringAttribute(const char* name, const char* (*function)(projModule&))
      :stringAttribute(name), func(function) {}
  const char* (*func)( projModule&);
  virtual const char* value ( symbolPtr& sym) const;
  virtual void print(ostream& = cout, int level = 0) const;
  define_relational(moduleStringAttribute,stringAttribute);
};
RelClass(moduleStringAttribute);
#endif
class funcStringAttribute : public  stringAttribute {

  public:

  funcStringAttribute(const char* name, const char* (*function)(symbolPtr&))
      : stringAttribute(name), func(function){}
  const char* (*func)( symbolPtr&);
  virtual const char* value ( symbolPtr& sym) const;
  virtual void print(ostream& = cout, int level = 0) const;
  define_relational(funcStringAttribute,stringAttribute);
};
RelClass(funcStringAttribute);


enum exprType { NO_TYPE, INT_TYPE, STRING_TYPE };

class expr : public Obj
{

  public:

  const char* ename;
  expr(const char*en) : ename(en), value_type(NO_TYPE) {}
  virtual ~expr() {}
  virtual  int value(const symbolPtr& sym) = 0;
  int boolValue(const symbolPtr& sym);
  virtual exprType valueType() const {return value_type;};
  virtual void setType() {};
  virtual void print(ostream& = cout, int level = 0) const = 0;

  virtual int nil_value() const; // {return 0;} // tree reached nil
  virtual bool is_tree() const;

  exprType value_type;

  static char error[1024];
};

inline int expr::boolValue(const symbolPtr& sym) 
{ int v = value(sym); return (value_type == STRING_TYPE) ? (v && *(char*)v) : v; }


extern "C" expr* api_parse_expression(const char*);

class exprAttribute : public intAttribute
{

  public:

  expr*op;
  define_relational(exprAttribute,intAttribute);

  exprAttribute(const char*name, expr*exp) 
    : intAttribute(name), op(exp) {}

  virtual void print(ostream& = cout, int level = 0) const;
  virtual int test( symbolPtr& sym) const;

  virtual const char* value( symbolPtr&) const;
  virtual bool boolValue(symbolPtr&) const;

  virtual bool is_int() const; // { return 0;}

};

// sioffe
typedef intAttribute boolAttribute;
typedef funcIntAttribute funcBoolAttribute;


char const * ATT_physical_filename(symbolPtr &sym);


#endif //__attribute_h
