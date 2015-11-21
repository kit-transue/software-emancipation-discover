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
#ifndef _externAtribute_h
#define _externAtribute_h

#include <Hash.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <general.h>
#include <genError.h>
#include <Interpreter.h>
#include <attribute.h>
#include <scopeMgr.h>

#include <externApp.h>

struct extEntry {
  genString tag;
};
extern char const *ATT_etag(symbolPtr&);
extern "C" char *cli_error_use();

class nameDict : public nameHash
{
public:
  virtual char const *name(const Object *oo) const ;
  extEntry *lookup(char const *tag) const;
  extEntry *lookup(symbolPtr&sym) const {return lookup(ATT_etag(sym));}

  void insert(extEntry*sym) { add(*(Object*) sym);}
  virtual void print(ostream&os=cout, char fs = ' ') const = 0;
  virtual int raw_insert(char const*key, char const*val) = 0;
};

class externAttribute : public symbolAttribute {
public:
  define_relational(externAttribute,symbolAttribute);
  externAttribute(char const*name, char const*fl, nameDict*tb);

  int validate(symbolPtr&sym) const;
  int validate(symbolArr&) const;
  void print(ostream&os=cout, int level=0) const {tbl->print(os);}

  virtual void get_type(genString&) const = 0;
  expr*filter() const;
  nameDict *dict() { return tbl; }
  char FS;
  static externAttribute*find(char const *name);

  genString filter_string;

protected:
  expr *filter_expr;
  nameDict *tbl;
};
RelClass(externAttribute);

class objNameSet;
class extAttrApp : public externApp {
 public:
   define_relational(extAttrApp,externApp);
   static externApp *create(externAttribute *att);
   static externApp *create(char const*name, externFile *extf);

   virtual bool externSave();
   virtual externInfo *info() const;
   static  externInfo *type_info();
   externAttribute *load();
   bool being_loaded() { return this == current;}
   static extAttrApp *get(char const*name);
   static objNameSet *get_all();
 protected:
   extAttrApp(char const *name, char const *fname, extStatus);
   static extAttrApp *current;

};

defrel_one_to_one(extAttrApp,app,externAttribute,attr);






class dictString : public nameDict {
public:
  char const *value(char const *tag) const;
  char const *value(symbolPtr&sym) const {return value(ATT_etag(sym));}
  int insert(char const*val, symbolPtr&sym);
  void print(ostream&os=cout, char fs= ' ') const;
  virtual int raw_insert(char const*key, char const*val);
};

class extStringAttribute : public externAttribute {
public:
  define_relational(extStringAttribute,externAttribute);
  extStringAttribute(char const*name, char const*fl)
    : externAttribute(name, fl, new dictString) {all.insert(this);}
  ~extStringAttribute() {all.remove(*this);}
  virtual bool is_int() const { return false;}
  virtual bool is_bool() const { return false;}

  virtual char const *value( symbolPtr&) const;
  virtual int test( symbolPtr& sym) const;
  virtual bool boolValue(symbolPtr&) const;

  int set(char const*val, symbolArr&arr);
  static objNameSet all;
  virtual void get_type(genString&str) const {str = "string";}

protected:  
  dictString *table() const {return (dictString*)tbl;}
public:
  int set(char const*val, symbolPtr&);
};
RelClass(extStringAttribute);

class dictInt : public nameDict {
public:
  int value(char const *tag) const;
  int value(symbolPtr&sym) const {return value(ATT_etag(sym));}
  int insert(int val, symbolPtr&sym);
  void print(ostream&os=cout, char fs= ' ') const;
  virtual int raw_insert(char const*key, char const*val);
  int insert_int(char const *tag, int ival);
};


class extIntAttribute : public externAttribute {
public:
  define_relational(extIntAttribute,externAttribute);
  extIntAttribute(char const*name, char const*fl, int is_enum=0)
    : externAttribute(name, fl, new dictInt) {if(!is_enum) all.insert(this);}
  ~extIntAttribute() { all.remove(*this);}
  virtual bool is_int() const { return true;}
  virtual bool is_bool() const { return true;}

  virtual char const *value( symbolPtr&) const;
  virtual int test( symbolPtr& sym) const;
  virtual bool boolValue(symbolPtr& sym) const {return test(sym);}

  int set(int val, symbolArr&arr);
  static objNameSet all;
  virtual void get_type(genString&str) const {str = "int";}

protected:  
  dictInt *table() const {return (dictInt*)tbl;}
  int set(int val, symbolPtr&);
  int  default_in;
  int  default_out;
  char buffer[12];  // for conversion of int to char*
};
RelClass(extIntAttribute);

struct extEnum {
  typedef char const *chrp;
  genArr(chrp)  values;
  bool valid(char const *val) const {return index(val) >= 0;}
  int index(char const*) const;
  char const *value(int ind) const
    {return (ind >= 0 && ind < values.size()) ? *values[ind] : "";}
  void append(char const *,int);
  void print(ostream&os=cout) const;
  void info(int*len, char const ***vals) const {*len=values.size(); *vals=values[0];}
};


class extEnumAttribute : public extIntAttribute {
public:
  extEnumAttribute (char const*name, char const*fl, extEnum*en, char const*dout)
    : extIntAttribute(name, fl, 1), enm(en), default_out(dout) {all.insert(this);}
  ~extEnumAttribute() {all.remove(*this);}
  virtual bool is_int() const { return false;}
  virtual bool is_bool() const { return false;}

  virtual char const *value( symbolPtr&) const;
  virtual int test( symbolPtr& sym) const {return (int) value(sym);}
  virtual bool boolValue(symbolPtr& sym) const 
         {char const*val = value(sym); return val&&val[0];}

  void info(int*len, char const ***vals) const {enm->info(len, vals);}
  void print(ostream&os=cout, int level=0) const;

  static objNameSet all;
  virtual void get_type(genString&) const;

  extEnum*enm;
  genString default_out;
  define_relational(extEnumAttribute,extIntAttribute);
};
RelClass(extEnumAttribute);

#endif



