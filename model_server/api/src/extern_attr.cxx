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
#include <externApp.h>
#include <externAttribute.h>

#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <machdep.h>
#include <Iterator.h>
#include <machdep.h>
#include "cLibraryFunctions.h"
#include <genStringPlus.h>
#include <scopeMgr.h>
#include <charstream.h>
#include "msg.h"

bool   extern_canonic_name(char const* name, genString& new_name);
extern Interpreter*  GetActiveInterpreter();
extern "C" char *cli_error_use();

extern char const *ATT_attr_tag(symbolPtr &);

inline char const *attribute_tag(symbolPtr &sym)
{
  return ATT_attr_tag(sym);
}

struct extStringEntry : public extEntry {
  genString val;
};

struct extIntEntry : public extEntry {
  int val;
};

char const* nameDict::name(const Object* oo) const
{
  return (char const*) ((extEntry*)oo)->tag;
}

extEntry* nameDict::lookup(char const*name) const
{
    Object*cur = 0;
    int idx = 0; 
    find(name, idx, cur); 
    return (extStringEntry*) cur;
}

void dictString::print(ostream&os, char fs) const
{
  Iterator it(*this);
  Object*o;
  while(o=it.operator++()){
    extStringEntry *en = (extStringEntry *) o;
    if(fs)
      os << (char *)en->tag << fs << (char *) en->val << '\n';
    else
      os << (char *)en->tag << '\n';
  }
  os << flush;
}

int dictString::insert(char const*val, symbolPtr&sym) 
{
  char const*tag = attribute_tag(sym);
  return raw_insert(tag, val);
}

int dictString::raw_insert(char const *tag, char const*val) 
{
  int is_new = 0;
  extStringEntry*en = (extStringEntry*)lookup(tag);
  if(!en){
    is_new = 1;
    en = new extStringEntry;
    en->tag = tag;
    nameDict::insert(en);
  }
  en->val = val;
  return is_new;
}

char const* dictString::value(char const*name) const
{
  extStringEntry*en = (extStringEntry*) lookup(name);
  return en ? en->val.str() : NULL;
}

init_relational(externAttribute,symbolAttribute);

int externAttribute::validate(symbolPtr&sym) const 
{  
  expr* op = filter();
  return op ? op->boolValue(sym) : 1;
}
 
expr*externAttribute::filter() const
{ 
  if( (filter_expr == NULL && filter_string.str())){
    Initialize(externAttribute::filter);
     ((externAttribute*)this)->filter_expr = api_parse_expression(filter_string.str());
     IF(filter_expr==NULL){
       Interpreter * i = GetActiveInterpreter();
       Tcl_AppendResult(i->interp, "expression parse error: \n", cli_error_use(), NULL);
       return 0;
     }
  }
  return filter_expr;
}

int externAttribute::validate(symbolArr&arr) const
{
  int sz = arr.size();
  int no_valid = 0;
  for(int ii=0;ii<sz;++ii){
    no_valid += validate(arr[ii]);
  }
  return sz - no_valid;
}

externAttribute::externAttribute(char const*name, char const*fl, nameDict*tb)
    : symbolAttribute(name), FS('#'), filter_string(fl), filter_expr(0), tbl(tb) 
{
  extAttrApp*ap = (extAttrApp*) extAttrApp::create(this);
  if(ap && !ap->being_loaded())
    ap->make_savable();
}

/*
 * extAttrApp related functions
 */

init_rel_or_ptr(extAttrApp,app,0,externAttribute,attr,0);
init_relational(extAttrApp,externApp);

externInfo* extAttrApp::type_info()
{
  static externInfo* inf = new externInfo(extAttrApp::create, "attr", "attribute", "ext");
  return inf;
}

extAttrApp::extAttrApp(char const *name, char const *fname, extStatus st)
   : externApp(name, fname, st) 

{
  get_all()->insert(this);
}

externInfo* extAttrApp::info() const
{
  return type_info();
}

bool extAttrApp::externSave()
{
  Initialize(extAttrApp::externSave);
  externAttribute* att = app_get_attr(this);
  if(!att)
    return true;

  char const* fname = OSPATH(get_phys_name());

  if(!(fname && fname[0])){  //freak accident: created in start-up script before home proj is set
    genString physname;
    char const* name = get_name();
    externInfo* ti = type_info();
    ti->new_fname(name, physname);
    this->set_phys_name(physname);
    fname = OSPATH(get_phys_name());
  }

  bool dir_ok = make_dir(fname);
  if(!dir_ok) {
    msg("Failed to make directory for $1", error_sev) << fname << eom;
    return false;
  } 
  msg("saving $1: $2", normal_sev) << att->get_name() << eoarg << fname << eom;
  
  ofstream os(fname, ios::out);
  if(os.bad()){
    msg("Error while accessing directory: $1", error_sev) << fname << eom;
    return false;
  }
  genString  desc;
  att -> get_type(desc);
  os << (char *)desc;
  
  char const*ex = att->filter_string;
  if(ex) {
    os << " { " << ex << " }";
  }
  os << endl;

  att->dict()->print(os, att->FS);
  set_status(SAVED);
  return true;
}

externApp* extAttrApp::create(externAttribute* att)
{
  Initialize(extAttrApp::create(externAttribute* att));

  extAttrApp* ap = current;
  if(!ap){
    char const *name = att->get_name();
    ap = checked_cast(extAttrApp,create(name, NULL));
    ap->set_status(NEW);
  }
  app_put_attr(ap, att);
  return ap;
}

externApp* extAttrApp::create(char const* name, externFile* extf)
{
  char const * physname;
  genString pname;

  if (extf) {
    physname = extf->get_phys_filename();
  }
  else {
    externInfo* ti = type_info();
    ti->new_fname(name, pname);
    physname = pname.str();
  }
  extAttrApp * ap = new extAttrApp(name, physname, SAVED);
  ap->set_phys_name(physname);
  return ap;
}

extAttrApp* extAttrApp::current;
Interpreter*  GetActiveInterpreter();
externAttribute* extAttrApp::load()
{
  
  externAttribute*attr = app_get_attr(this);
  if(attr) 
    return attr;

  char const*name = get_name();
  char const*fname = OSPATH(get_phys_name());

  if(!(fname && fname[0])) return NULL;
  msg("Loading attribute $1: $2...", normal_sev) << name << eoarg << fname << eom;

  struct stat st;
  stat(fname, &st);

  FILE* fl = fopen(fname, "r");

  if(!fl){
    msg("Can not open attribute file $1", error_sev) << fname << eom;
    return NULL;
  }

  int  fsize = (int) st.st_size;
  char *buf = new char[fsize + 1];
  int sz = fread(buf, 1, fsize, fl);
  buf[fsize] = '\n';  // just in case no endline at the end

// get header (first line)
  sz = fsize + 1;

  int ii = 0;
  // attribute type
  char *atype = buf;

  int ch;
  while(((ch = buf[ii]) != ' ') && (ch != '\n'))
    ++ii;
  buf[ii++] = '\0';

  // body
  char const *body = "";
  if(ch != '\n') {
    body = buf + ii;  
    while(buf[ii] != '\n')
      ++ii;
    buf[ii++] = '\0';
  }
  // form the command 
  genString cmd;
  cmd.printf("attribute define %s -force %s %s", atype, name, body);
  Interpreter * i = GetActiveInterpreter();
  
  current = this;
  i->EvalCmd(cmd);
  current = NULL;

  attr = app_get_attr(this);
  if(attr) {
    nameDict*dict = attr->dict();   
  
    for(; ii<sz; ++ii){
      char *key = buf+ii;
      int key_len = 0;
      // key

      while(ii < sz && buf[ii] != attr->FS) {
	++ii;
	++key_len;
      }      
      if(ii >= sz)
	break;

      buf[ii] = '\0';
      char *val = buf + ii + 1;
      while(buf[ii] != '\n')
	++ii;
      buf[ii] = '\0';
      
      dict->raw_insert(key, val);
    }
  }

 fclose(fl);
 delete buf;

 msg(" done\n", normal_sev) << eom;
 return attr;

}


objNameSet *extAttrApp::get_all()
{
  return extAttrApp::type_info()->get_apps();
}

extAttrApp *extAttrApp::get(char const*name)
{
  extAttrApp *ap = (extAttrApp*) extAttrApp::get_all()->lookup(name);
  if(!ap){
    genString new_name;
    bool changed_name = extern_canonic_name(name, new_name);
    if(changed_name)
      ap = (extAttrApp*) extAttrApp::get_all()->lookup(new_name);
  }
  return ap;
}





objNameSet& get_all_extern_attributes(void)
{
  return *extAttrApp::get_all();
}





init_relational(extStringAttribute,externAttribute);


int extStringAttribute::set(char const*val, symbolPtr&sym)
{
   return table()->insert(val, sym);
}

int extStringAttribute::set(char const*val, symbolArr&arr)
{
  if(validate(arr))
    return TCL_ERROR;
  int sz = arr.size();
  for(int ii=0;ii<sz;++ii){
    set(val, arr[ii]);  
  }
  externApp*app = attr_get_app(this);
  if(app) app->set_status(externApp::MODIFIED);
  return TCL_OK;
}

int extStringAttribute::test( symbolPtr& sym) const
{
      return (int) value(sym);
}
bool extStringAttribute::boolValue( symbolPtr& sym) const
{
  char const*val = value(sym);
  return val && val[0];
}
char const *extStringAttribute::value( symbolPtr&sym) const
{
   char const*val = 0;
   if(validate(sym)){
     char const  *tag = attribute_tag(sym);
     val = table()->value(tag);
   }
   return val ? val : "";
}


//////  int

void dictInt::print(ostream&os, char fs) const
{
  Iterator it(*this);
  Object*o;
  while(o=it.operator++()){
    extIntEntry *en = (extIntEntry *) o;
    if(fs)
      os << (char *)en->tag << fs << en->val << '\n';
    else
      os << (char *)en->tag << '\n';
  }
  os << flush;
}

int dictInt::raw_insert(char const*tag, char const*val)
{
   int ival = OSapi_atoi(val);
   return insert_int(tag, ival);
}
int dictInt::insert(int ival, symbolPtr&sym) 
{
  char const*tag = attribute_tag(sym);
  return insert_int(tag, ival);
}

int dictInt::insert_int(char const *tag, int ival)
{
  int is_new = 0;
  extIntEntry*en = (extIntEntry*)lookup(tag);
  if(!en){
    is_new = 1;
    en = new extIntEntry;
    en->tag = tag;
    nameDict::insert(en);
  }
  en->val = ival;
  return is_new;
}

int dictInt::value(char const*name) const
{
  extIntEntry*en = (extIntEntry*) lookup(name);
  return en ? en->val : -1;
}

init_relational(extIntAttribute,externAttribute);

int extIntAttribute::set(int val, symbolPtr&sym)
{
   return table()->insert(val, sym);
}

int extIntAttribute::set(int val, symbolArr&arr)
{
  if(validate(arr))
    return TCL_ERROR;
  int sz = arr.size();
  for(int ii=0;ii<sz;++ii){
    set(val, arr[ii]);  
  }
  externApp*app = attr_get_app(this);
  if(app) app->set_status(externApp::MODIFIED);

  return TCL_OK;
}

char const *extIntAttribute::value( symbolPtr& sym) const
{
   int iv = test(sym);
   OSapi_sprintf((char *)buffer, "%d", iv);
   return buffer;
}

int extIntAttribute::test( symbolPtr& sym) const
{ 
   int val = 0;
   if(validate(sym)){
     char const  *tag = attribute_tag(sym);
     val = table()->value(tag);
   }
   return val;
}

///   end int
///  enum

int
extEnum::index(char const*val) const
{
  int sz = values.size();
  for(int ii=0; ii<sz; ++ii){
     if(OSapi_strcmp(val, *values[ii]) == 0)
       return ii;
  }
  return -1;
}

void extEnum::print(ostream&os) const
{
  int  sz = values.size();
  for(int ii=0; ii<sz; ++ii)
    os << *(values[ii]) << ' ';
  os << endl;
}

void extEnum::append(char const *s, int len)
{
  char *w = new char[len+1];
  OSapi_strncpy(w,s,len);
  w[len] = '\0';
  values.append(&w);
}

extEnum *api_build_enum(char const *val_list)
{
  extEnum*list = new extEnum;
  char const *en=val_list;
  while(*en) {
    char const *st = en;
    while(*st==' ') {++st; ++en;}
    while(*en && *en!=' ') ++en;
    if(st != en) {
      int len = en - st;
      list->append(st, len);
    }
  }
  return list;
}

init_relational(extEnumAttribute,extIntAttribute);

void extEnumAttribute::get_type(genString&st) const
{
  int argc; char const **argv;
  info(&argc, &argv);

  st =  "enum {";
  for(int ii=0; ii<argc; ++ii){
     st += ' ';
     st += argv[ii];
   }
  st += " }";
}

void extEnumAttribute::print(ostream&os, int) const 
{
  int argc; char const **argv;
  info(&argc, &argv);
  os << "enum " << get_name() << ':';
  for(int ii=0; ii<argc; ++ii)
     os << ' ' << argv[ii];
  os << endl;
  tbl->print(os, FS);
}

char const *extEnumAttribute::value( symbolPtr&sym) const
{
   char const *val;
   if(validate(sym)){
     int ind = extIntAttribute::test(sym);
     val = enm->value(ind);
   } else {
     val = default_out;
   }
   return val;
}

/// end enum

static int check_attribute(Interpreter*i, char const*tp, char const*aname, extAttrApp *ap, char const*ex)
{
  if(ap && !ap->being_loaded()){
    Tcl_AppendResult(i->interp, "attribute ", tp, " ", aname, ": already exists", NULL);
    return TCL_ERROR;
  }

  int res = TCL_OK;

  if(!ex)
    return res;

  if(ap && ap->being_loaded())
    return res;

  expr *expression = api_parse_expression(ex);

  if(expression == NULL){
    Tcl_AppendResult(i->interp, aname, " {", ex, "} \n parse error: ", cli_error_use(), NULL);
    res = TCL_ERROR;
  } else {
    delete expression;
  }
  return res;
}

static int attribute_enumCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{

    Interpreter *i        = (Interpreter *)cd;

    if(argc < 3 || argc > 5) {
	Tcl_AppendResult(interp, "Usage: add_enum att-name val-list [att-expression]", NULL);
	return TCL_ERROR;
    };
    
    int force = 0;
    if( strcmp(argv[1], "-force") == 0 )
      force = 1;

    char const  *aname = argv[1+force];
    genString new_name;
    bool changed = extern_canonic_name(aname, new_name);
    if(changed)
      aname = new_name;
    extAttrApp *ap = extAttrApp::get(aname);

    char const *val_list = argv[2+force];
    char const *att_expr = (argc == (3+force)) ? NULL : argv[3+force];
 
    int tcl_res  = check_attribute(i, "enum", aname, ap, att_expr);
    if(tcl_res != TCL_OK)
      return tcl_res;

    extEnum  *en = api_build_enum(val_list);
    char const *dout = "N/A";

    // Try to find it first.  If the attr file exists it will open it and create the attribute
    // via this command but will use the -force option.
    symbolAttribute *attr = NULL;
    if( !force )
      attr = externAttribute::find(aname);
    if(!attr)
      attr = new extEnumAttribute(aname, att_expr, en, dout);

    tcl_res = TCL_OK;
    return tcl_res;
}

static int attribute_stringCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{

    Interpreter *i        = (Interpreter *)cd;

    if(argc < 2 || argc > 4) {
	Tcl_AppendResult(interp, "Usage: add_string att-name [att-expression]", NULL);
	return TCL_ERROR;
    };

    int force = 0;
    if( strcmp(argv[1], "-force") == 0 )
      force = 1;

    char const *aname = argv[1+force];
    genString new_name;
    bool changed = extern_canonic_name(aname, new_name);
    if(changed)
      aname = new_name;
    extAttrApp *ap = extAttrApp::get(aname);

    char const *att_expr = (argc == (2+force)) ? NULL : argv[2+force];

    int tcl_res  = check_attribute(i, "string", aname, ap, att_expr);
    if(tcl_res != TCL_OK)
      return tcl_res;

    // Try to find it first.  If the attr file exists it will open it and create the attribute
    // via this command but will use the -force option.
    symbolAttribute *attr = NULL;
    if( !force )
      attr = externAttribute::find(aname);

    if( !attr )
      attr = new extStringAttribute(aname, att_expr);

    tcl_res = TCL_OK;
    return tcl_res;
}

static int attribute_intCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{

    Interpreter *i        = (Interpreter *)cd;

    if(argc < 2 || argc > 4) {
	Tcl_AppendResult(i->interp, "Usage: add_int att-name [att-expression]", NULL);
	return TCL_ERROR;
    };

    int force = 0;
    if( strcmp(argv[1], "-force") == 0 )
      force = 1;

    char const *aname = argv[1+force];
    genString new_name;
    bool changed = extern_canonic_name(aname, new_name);
    if(changed)
      aname = new_name;
    extAttrApp *ap = extAttrApp::get(aname);

    char const *att_expr = (argc==(2+force)) ? NULL : argv[2+force];

    int tcl_res  = check_attribute(i, "int", aname, ap, att_expr);
    if(tcl_res != TCL_OK)
      return tcl_res;

    // Try to find it first.  If the attr file exists it will open it and create the attribute
    // via this command but will use the -force option.
    symbolAttribute *attr = NULL;
    if( !force )
      attr = externAttribute::find(aname);

    if( !attr )
      attr = new extIntAttribute(aname,att_expr);

    tcl_res = TCL_OK;
    return tcl_res;
}


static int attribute_tempCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{

  Interpreter *i = (Interpreter *)cd;
  
  if(argc < 2) {
    Tcl_AppendResult(i->interp, "'attribute temp' must be followed by either 'int', 'enum' or 'string'", NULL);
    return TCL_ERROR;
  };

  int result = TCL_OK;

  if (strcmp(argv[1], "int") == 0) {
    result = attribute_intCmd(cd,interp,argc-1,argv+1);
  } else if (strcmp(argv[1], "enum") == 0) {
    result = attribute_enumCmd(cd,interp,argc-1,argv+1);
  } else if (strcmp(argv[1], "string") == 0) {
    result = attribute_stringCmd(cd,interp,argc-1,argv+1);
  } else {
    Tcl_AppendResult(i->interp, "'attribute temp' must be followed by either 'int', 'enum' or 'string'", NULL);
    return TCL_ERROR;
  };
  
  if ((result == TCL_OK) && (argc >= 3)) {
    char const *att_name = argv[2];
    extAttrApp *ap = extAttrApp::get(att_name);
    if (ap) {
      ap->set_status(externApp::TEMP);
    } else {
    Tcl_AppendResult(i->interp, "'attribute temp': had difficulty making attribute temporary", NULL);
    return TCL_ERROR;
    }
  }    
  return result;
}


static int attribute_setCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i       = (Interpreter *)cd;
  char const *value = argv[1];

  if (argc < 3  || (argc == 3 && argv[2][0] == '-' && argv[2][1] == 'i')) {
    Tcl_AppendResult(i->interp, "wrong number of arguments", NULL);
    return TCL_ERROR;
  }
  
  char const *att_name = argv[1];
  char const *att_val  = argv[2];
  int special = 0;
  int start = 3;
  if(argv[2][0] == '-' && argv[2][1] == 'i'){
      special = 1;
      start   = 4;
      att_val = argv[3];
  }

  symbolArr src_arr;
  if(i->ParseArguments(start, argc, argv, src_arr) != 0)
    return TCL_ERROR;

  symbolAttribute *attr = externAttribute::find(att_name);
  if(!attr) {
    Tcl_AppendResult(i->interp, "attribute does not exist", NULL);
    return TCL_ERROR;
  }

  int tcl_res = TCL_OK;
  if(is_extEnumAttribute(attr)) {
    extEnumAttribute *eattr = (extEnumAttribute*)attr;
    int val;
    if(special) {
      char const *ptr = NULL;
      val = strtol(att_val, (char **)&ptr, 10);
      if(ptr[0] != '\0'){
	tcl_res = TCL_ERROR;
	Tcl_AppendResult(i->interp, "value must be an integer", NULL);
      }
    } else {
	if(att_val[0] == '\0'){
	    val = -1;
	} else {
	    val = eattr->enm->index(att_val);
	    if(val<0){
		tcl_res = TCL_ERROR;
		Tcl_AppendResult(i->interp, "value must be an enum value", NULL);
	    }
	}
    }
    if(tcl_res == TCL_OK){
      tcl_res   = ((extIntAttribute*)attr)->set(val, src_arr);
      if(tcl_res != TCL_OK) {
	Tcl_AppendResult(i->interp, "some entities cannot have this attribute", NULL);
      }
    }
  } else if(is_extStringAttribute(attr)) {
    tcl_res   = ((extStringAttribute*)attr)->set(att_val, src_arr);
    if(tcl_res != TCL_OK) {
      Tcl_AppendResult(i->interp, "some entities cannot have this attribute", NULL);
    }
  } else if(is_extIntAttribute(attr)) {
    char const *ptr=NULL;
    int val = strtol(att_val, (char **)&ptr, 10);
    if(ptr[0] != '\0'){
      tcl_res = TCL_ERROR;
      Tcl_AppendResult(i->interp, "value must be an integer", NULL);
    } else {
      tcl_res   = ((extIntAttribute*)attr)->set(val, src_arr);
      if(tcl_res != TCL_OK) {
	Tcl_AppendResult(i->interp, "some entities cannot have this attribute", NULL);
      }
    }
  } else {
    Tcl_AppendResult(i->interp, "attribute is not external", NULL);
    tcl_res =  TCL_ERROR;
  }
  
  return tcl_res;
}

static int print_attr_args(Interpreter*i, char const*cmd, char const*title, char const*list)
{
  ostream&os = i->GetOutputStream();
  os << cmd << ": " << title << " Must be one of:" << endl;
  Tcl_AppendResult(i->interp, list, NULL);
  return TCL_OK;
}

#define subcommand(nm) if(OSapi_strcmp(cmd, quote(nm)) == 0) \
  return paste3(attribute_,nm,Cmd)(cd, interp, argc-1, argv+1)
#define if_equal(nm) if(OSapi_strcmp(cmd, quote(nm))==0)

static char const *attribute_commands = "info print set int enum string";
static char const *attribute_types = "int string enum";

static int attribute_defineCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i       = (Interpreter *)cd;
  char const *cmd = argv[1];

  subcommand(int);
  subcommand(enum);
  subcommand(string);
 
  print_attr_args(i, argv[0], "wrong subcommand", attribute_types);
  return TCL_ERROR;
}

void attribute_info_list(Interpreter*i, objNameSet& list)
{
  genStringPlus answ;
  Iterator it(list);
  Object*o;
  while(o=it.operator++()){
    Relational *el = (Relational *) o;
   
    answ += ' ';
    answ += el->get_name();
  }
  Tcl_AppendResult(i->interp, (char *)answ, NULL);
}
objNameSet extIntAttribute::all;
objNameSet extEnumAttribute::all;
objNameSet extStringAttribute::all;


externAttribute *externAttribute::find(char const*name)
{
  externAttribute*attr = NULL;
  extAttrApp *ap = extAttrApp::get(name);
  if(ap) 
    attr = ap->load();
  return attr;
}

static int attribute_infoCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i  = (Interpreter *)cd;
   
  if(argc < 2) {
      if(extAttrApp::get_all() == NULL)
	  return TCL_OK;
      attribute_info_list(i, *extAttrApp::get_all());
      return TCL_OK;
  }
  char const *cmd = argv[1];
  int  retcode = TCL_OK;
  if_equal(int) {
     attribute_info_list(i, extIntAttribute::all);
  } else if_equal(enum) {
     attribute_info_list(i, extEnumAttribute::all);
  } else if_equal(string) {
     attribute_info_list(i, extStringAttribute::all);
  } else {  
   // must be name
   externAttribute*attr = externAttribute::find(cmd); 
   if(attr){
     genString desc;
     attr -> get_type(desc);
     Tcl_AppendResult(i->interp, (char *)desc, NULL);
   } else {
     Tcl_AppendResult(i->interp, "wrong attribute info subcommand", NULL);
     retcode = TCL_ERROR;
   }
  }
  return retcode;
}

static int attribute_printCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i       = (Interpreter *)cd;

  char const *att_name = argv[1];

  symbolAttribute *attr = externAttribute::find(att_name);
  if(!attr) {
    symbolAttribute *sap = symbolAttribute::get_by_name(att_name);
    if(sap){
      ostream& os = i->GetOutputStream();
      sap->print(os);
      os << endl;
      return TCL_OK;
    } else {
      Tcl_AppendResult(i->interp, "attribute does not exist", NULL);
      return TCL_ERROR;
    }
  }

  node_prt(attr);  
  return TCL_OK;
}

static int attribute_saveCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i       = (Interpreter *)cd;

  if(argc != 2)
    return TCL_ERROR;

  char const *aname = argv[1];

  extAttrApp *ap = extAttrApp::get(aname);
  if(!ap) {
    Tcl_AppendResult(i->interp, "attribute does not exist", NULL);
    return TCL_ERROR;
  }

  ap->externSave();
  return TCL_OK;
}

int parse_selection(char const *, symbolArr& result);
static int attribute_getCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i       = (Interpreter *)cd;

  if(argc != 2)
    return TCL_ERROR;

  char const *att_name = argv[1];

  externAttribute *attr = externAttribute::find(att_name);
  if(!attr) {
    Tcl_AppendResult(i->interp, "attribute does not exist", NULL);
    return TCL_ERROR;
  }

  ocharstream buf;
  attr->dict()->print(buf, '\0');
  buf << '\0';

  DI_object target;
  DI_object_create(&target);
  symbolArr&arr =  *get_DI_symarr(target);
  char const *tags = buf.ptr();
  parse_selection(tags, arr);
  i->SetDIResult(target); 

  return TCL_OK;
}

static int attribute_existsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i       = (Interpreter *)cd;

  if(argc != 2)
    return TCL_ERROR;

  char const *name = argv[1];

  extAttrApp *ap = extAttrApp::get(name);
  Tcl_AppendResult(i->interp, ap ? "1" : "0", NULL);

  return TCL_OK;
}

static int attributeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i = (Interpreter *)cd;

  externInfo::load();
  if(argc<2) {
    print_attr_args(i, argv[0], "subcommand required", attribute_commands);
    return TCL_OK;
  }
  char const *cmd = argv[1];
  subcommand(info);
  subcommand(set);
  subcommand(save);
  subcommand(get);
  subcommand(int);
  subcommand(enum);
  subcommand(string);
  subcommand(exists);
  subcommand(print);
  subcommand(define);
  subcommand(temp);
 
  print_attr_args(i, argv[0], "wrong subcommand", attribute_commands);
  return TCL_ERROR;
}  



static int add_commands()
{
    new cliCommandInfo("attribute", attributeCmd);
    return 0;
}
static int add_commands_dummy = add_commands();

