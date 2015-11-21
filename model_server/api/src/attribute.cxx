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
#include <attribute.h>
#include <ddKind.h>
#include <representation.h>
#include <projHeader.h>
#include <ddict.h>
#include <proj.h>
#include <symbolPtr.h>
#include <xrefSymbol.h>
#include <fileEntry.h>
#include <machdep.h>
#include "cLibraryFunctions.h"
#include <genString.h>
#include "charstream.h"
#include <dfa_db.h>
#include <smtRegion.h>
#include <externAttribute.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#ifndef __ldrList
#include <ldrList.h>
#include <genArrCharPtr.h>
#else
genArr(int);
#endif
#include <metric.h>
#include <fileCache.h>
#include <genStringPlus.h>
#include <ldrList.h>
#include <path.h>
#include <msg.h>

extern symbolPtr get_semtype_symbol(symbolPtr);
char const *xref_c_proto(const symbolPtr &src);
int xref_not_header_file(char const *full_path);
extern int project_name_to_fn(char const *, genString &);
extern int astNode_get_line_number(const symbolPtr &sym);
symbolPtr astXref_get_module(astXref*);
smtTree   *GetSmtFromMarker(symbolPtr& sym);
smtTree   *GetSmtFromMarker(smtMarker *sm);
smtRegion *GetRegion(symbolPtr& sym);
extern int builtinlist();
int ATT_strange(symbolPtr&);

class dataCell;
void dataCell_get_module(const symbolPtr &symbol, symbolPtr &module);
void dataCell_get_graph(dataCell *cell, symbolPtr &symbol);
dataCell *dataCell_of_symbol(const symbolPtr &symbol, bool load);
void dataCell_offload(dataCell *&cell, bool mode);

static objArr *kind_attr_array;
objSet * ast_codes;
objSet * ast_categories;
 
static char const *dis_dbg_cli = OSapi_getenv ("DIS_DBG_CLI");
#define DBG_CLI if(dis_dbg_cli)

init_relational(symbolAttribute,objDictionary);
init_relational(intAttribute,symbolAttribute);
init_relational(kindAttribute,intAttribute);
init_relational(xrefAttribute,intAttribute);
init_relational(astAttribute,xrefAttribute);
init_relational(metricAttribute,intAttribute);
init_relational(funcIntAttribute,intAttribute);
init_relational(stringAttribute,symbolAttribute);
init_relational(funcStringAttribute,stringAttribute);
//init_relational(moduleStringAttribute,stringAttribute);
init_relational(moduleIntAttribute,intAttribute);

static objSet *all_attrs;
static objSet *int_attrs;
static objSet *string_attrs;

bool api_def_less_kind(ddKind kind)
{
  switch (kind) {
  case DD_SCOPE:
  case DD_PROJECT:
  case DD_STRING:
  case DD_LOCAL:
  case DD_SEMTYPE:
  case DD_SYNTYPE:
    return true;
  default:
    return false;
  }
}

char const *ATT_filename( symbolPtr& sym)
{
    Initialize(ATT_filename);
    ddKind kind = sym.get_kind();

    char const *result = "";
    
    if (kind == DD_MODULE || kind == DD_SCOPE || kind == DD_EXTFILE) {
        result = sym.get_name();
    } else if (kind == DD_PROJECT) {
        projNode *proj = checked_cast(projNode, sym);
        result = proj->fake_name;
        if(!result || !result[0]){
          genString phys;
          proj->ln_to_fn_imp(proj->get_name(), phys);
          proj->fn_to_ln(phys, proj->fake_name);
          result = proj->fake_name;
        }
    } else {
      if(sym.is_instance()) {
        symbolPtr scope = sym.scope();
        symbolPtr mod;

        if (scope.xrisnotnull())
          mod = scope->get_def_file();
        if (mod.xrisnotnull()) 
          result = mod.get_name();
      } else if (sym.is_ast()) {
        astXref *axr = sym.get_astXref();

        if (axr) {
          symbolPtr mod = astXref_get_module(axr);

          if (mod.isnotnull())
            result = mod.get_name();
        }
      } else if (sym.is_dataCell()) {
        symbolPtr mod;
        dataCell_get_module(sym, mod);

        if (mod.isnotnull())
          result = mod.get_name();
      } else if (!api_def_less_kind(kind)) {
        symbolPtr xsym = sym.get_xrefSymbol();

        if (xsym.isnotnull()) {
          symbolPtr def_file = sym->get_def_file();

          if (def_file.isnotnull()) {
            result = def_file.get_name();
          }
          else if (kind == DD_CLASS || kind == DD_FUNC_DECL
                                    || kind == DD_VAR_DECL) {

            // This might be a template instantiation.
            symbolArr templates;
            xsym->get_link(has_template, templates);
            if (templates.size() > 0) {
              symbolPtr tmpl = templates[0].get_xrefSymbol();
              if (!tmpl.isnull()) {
                result = ATT_filename(tmpl);
              }
            }
          }
        }
      }
    }

    return result;
}

projModule * symbol_get_module( symbolPtr& sym)
{
  ddKind kind = sym.get_kind();
  if(api_def_less_kind(kind))
    return NULL;
  
  char const *ln = NULL;
  if (kind == DD_MODULE) {
    if(sym.relationalp()) {
      Relational* mmm = sym;
      if(is_projModule(mmm))
        return (projModule*) mmm;
      else if (is_app(mmm))
        return appHeader_get_projectModule((const app*)mmm);
    } else {
      ln = sym.get_name();
    }
  } else {
    ln = ATT_filename(sym);
  }
     
  if(!(ln && *ln))
    return NULL;

  return projHeader::find_module(ln);
}


symbolAttribute::symbolAttribute(char const *name)
    : objDictionary(name ? name : "")
{
  if(name){    
    if(!all_attrs)
      all_attrs = new objSet;
    all_attrs->insert(this);

  }
}

symbolAttribute::~symbolAttribute()
{
    all_attrs->remove(this);
}

intAttribute::intAttribute(char const *name)
    : symbolAttribute(name)
{
  if(name && name[0]){
    if(!int_attrs)
      int_attrs = new objSet;
    int_attrs->insert(this);
  }
  _buff[0] = '\0';
}

intAttribute::~intAttribute()
{
    int_attrs->remove(this);
}

stringAttribute::stringAttribute(char const *name)
    : symbolAttribute(name)
{
   if(!string_attrs)
     string_attrs = new objSet;

   string_attrs->insert(this);
}

stringAttribute::~stringAttribute()
{
    string_attrs->remove(this);
}

void kindAttribute::print(ostream& str, int) const
{
  str << get_name() << ' ' << ddKind_name(kind);
}

void xrefAttribute::print(ostream& str, int) const
{
  str << get_name() << ' ' << offset;
}

void astAttribute::print(ostream& str, int) const
{
  str << get_name() << ' ' << offset;
}

void metricAttribute::print(ostream& str, int) const
{
  str << get_name() << ' ' << (int)type;
}

void funcIntAttribute::print(ostream& str, int) const
{
  str << get_name() << ' ' << (void*) func;
}

void moduleIntAttribute::print(ostream& str, int) const
{
  str << get_name() << ' ' << (void*) func;
}
#ifdef XXX_moduleStringAttribute
void moduleStringAttribute::print(ostream& str, int) const
{
  str << get_name() << ' ' << (void*) func;
}
#endif
void funcStringAttribute::print(ostream& str, int) const
{
  str << get_name() << ' ' << (void*) func;
}

char const *intAttribute::value( symbolPtr& sym) const
{ 
    OSapi_sprintf((char *)_buff, "%d", test(sym));
    return _buff;
}

bool intAttribute::is_int() const { return 1;}

bool symbolAttribute::is_int() const { return 0;}

int symbolAttribute::test( symbolPtr& sym) const
{
    return (int)value(sym);
}

static objSet kinds;
kindAttribute::kindAttribute(char const *name, ddKind k, int x, char const *plu)
    : intAttribute(name), kind(k), is_xref(x), plural(plu)
{
  kinds.insert(this);
}

kindAttribute* kindAttribute::lookup(char const *name)
{
  return (kindAttribute*) obj_search_by_name(name, kinds);
}

int kindAttribute::test( symbolPtr& sym) const
{
  ddKind kk = sym.get_kind();
  if(kind == kk)
    return 1;

  if(kk==DD_AST || kk==DD_INSTANCE){
    if(is_xref){
      symbolPtr xsym = sym.get_xrefSymbol();
      if(xsym.isnotnull()){
        kk = xsym.get_kind();
        return kind == kk;
      }
    }
  }
  return 0;
}

bool get_type_sym(const symbolPtr &sym, symbolPtr &type);

int xrefAttribute::test( symbolPtr& sym) const
{
    symbolPtr xsym;
    if (offset == POINTER && sym.is_ast()) {
      symbolPtr type;
      if (get_type_sym(sym, type)) {
        xsym = type;
      }
    } else 
      xsym = sym.get_xrefSymbol();
    if(xsym.xrisnull())
        return 0;
    return xsym->get_attribute(offset,1);
}

bool (*ast_instance)(const symbolPtr &ast, symbolPtr &inst);
int astAttribute::test( symbolPtr& sym) const
{
    int ret = 0;
    symbolPtr inst;
    symbolPtr*pinst = NULL;
    if((sym.is_ast() || sym.is_dataCell() )&& ast_instance){
      if(ast_instance(sym, inst))
        pinst = &inst; 
    } else if (sym.is_instance()) {
      pinst = &sym;
    }

    if(pinst){
      MetricElement mel = pinst->get_MetricElement();
      if (!mel.isnull()) {
        uint len = 1;
        if (offset == ASSIGN_LEVEL_ATT || offset == VALUE_LEVEL_ATT )
          len = 4;
        ret = mel.get_ast_attribute(offset,len);
      }
    }
    return ret;
}

int metricAttribute::test( symbolPtr& sym) const
{
    int value;
    if (Metric::def_array[type]->type_is_int) 
      Metric::get_metric_nocompute(type, sym, &value);
    else
      value = -1;
    return value;
}

int funcIntAttribute::test( symbolPtr& sym) const
    { return (*func)(sym);}

int moduleIntAttribute::test( symbolPtr& sym) const
{ 
    projModule* mod = symbol_get_module(sym);
    if (mod) {
        return (*func)(*mod);
    };
    return 0;
}
#ifdef XXX_moduleStringAttribute
char const *moduleStringAttribute::value ( symbolPtr& sym) const
{ 
    projModule* mod = symbol_get_module(sym);
    if (mod) {
        char const *res = (*func)(*mod);
        if (res)
            return res;
    };
    return "\0";
}
#endif
char const *funcStringAttribute::value ( symbolPtr& sym) const
{ 
  char const *res = (*func)(sym);
  if (res)
    return res;
  return "\0";
}


Obj* symbolAttribute::get_string_attrs()
{
  return string_attrs;
}

Obj* symbolAttribute::get_int_attrs()
{
  return int_attrs;
}

Obj* symbolAttribute::get_bool_attrs() {
    return get_int_attrs() ;
}

Obj* symbolAttribute::get_all_attrs()
{
  return all_attrs;
}

symbolAttribute* symbolAttribute::get_by_name(char const *name)
{
  Initialize(symbolAttribute::get_by_name);
  Relational* rel = obj_search_by_name(name, *all_attrs);
  if (rel)
      return (symbolAttribute*) rel;
  
  else {
    externInfo::load();
    symbolAttribute*att = externAttribute::find(name);
    if (att)
      return att;
  }

// abbreviation, e.g 'mac' for 'macro'  
  int len = strlen(name);
  if(len < 3)
    return NULL;

  Obj * el;

  ForEach(el, *kind_attr_array) {
    symbolAttribute* cur = (symbolAttribute*) el;
    if ( 0 == strncmp(cur->get_name(), name, len) )
      return cur;
  }

  return NULL;
}

//----------------------------------------

char const * ATT_language( symbolPtr& sym)
{
    Initialize (ATT_language);

    int lang_id =  sym.get_language_local();
    
    char const *lan = NULL;
    if (lang_id == FILE_LANGUAGE_ELS || lang_id == FILE_LANGUAGE_UNKNOWN) {
        symbolPtr surrogate_file;
        if (sym.get_kind() == DD_MODULE && sym.is_def()) {
            surrogate_file = sym;
        }
        else if (sym.is_xrefSymbol()) {
            symbolPtr deffile;
            deffile = sym->get_def_file(); 

            if (deffile.xrisnotnull()) {
                surrogate_file = deffile;
            }
            else {
                /* could not find definition file, try files where referenced */
                symbolArr files;
                sym->get_ref_files(files);
                if (files.size() > 0) {
                    surrogate_file = files[0];
                }
            }
            lang_id = surrogate_file.get_language();
        } else if (sym.get_kind() == DD_MODULE 
                   && sym.relationalp() 
                   && is_projModule((Relational *)sym)) {
	     /* If this is a projModule then get its xrefSymbol. */
             projModule *mod = checked_cast(projModule, sym);
	     if (mod == 0) {
	         genString file_name;
	         sym.get_name(file_name);
	         msg("WARN: File $1 does not exist.") << file_name.str() << eom;
	     } 
	     else {
	         projNode *project = mod->get_project();
	         surrogate_file = mod->get_def_file_from_proj(project);
             }	     
        }
        else if (sym.is_ast()) {
          astXref *axr = sym.get_astXref();

          if (axr) {
            surrogate_file = astXref_get_module(axr);
            lang_id = surrogate_file.get_language();
          }
        }
        if ((lang_id == FILE_LANGUAGE_UNKNOWN
	     || lang_id == FILE_LANGUAGE_ELS) && surrogate_file.isnotnull()) {
            // queries work on the xrefSymbol
            symbolPtr xrfile = surrogate_file.get_xrefSymbol();

            if (xrfile.xrisnotnull()) {
                symbolArr result;
                xrfile->get_link_chase_typedefs(has_property, result);

                symbolPtr rsym;
                ForEachS(rsym, result) {
                    if ( rsym.get_kind() == DD_LANGUAGE) {
                        lan = rsym.get_name();
                    }
                }
            }
        }
    }
    if (lan == NULL) {
        lan = fileLanguage_get_name(lang_id);
        if (lan && strlen(lan) > 14)
            lan += 14; // remove FILE_LANGUAGE_
    }

    return lan;
}

char const * ATT_checksum( symbolPtr& file)
{
    Initialize (ATT_checksum);
    
    char const *cksum = NULL;

    // queries work on the xrefSymbol
    symbolPtr xrfile = file.get_xrefSymbol();

    if (xrfile.xrisnotnull()) {
        // cksum = char *checksum of file
        // Got this stuff from XrefQuery.C
        // Get all property links.
        symbolArr result;

        if (xrfile.get_kind() == DD_MODULE &&
                 file.relationalp() && 
                 is_projModule((Relational *)file)) {

            // If the object in question is a module, find the xrefsymbol 
            // with the right links.

            projModule *mod = checked_cast(projModule, file);
            if (mod == 0) {
                genString xrfile_name;
                xrfile.get_name(xrfile_name);
                msg("WARN: File $1 does not exist.") << xrfile_name.str() << eom;
            } 
            else {
                projNode *project = mod->get_project();
                xrfile = mod->get_def_file_from_proj(project);
                if(!xrfile.isnull())
                    xrfile->get_link_chase_typedefs(has_property, result);
            }
        }
        else {
            xrfile->get_link_chase_typedefs(has_property, result);
        }

        symbolPtr rsym;
        ForEachS(rsym, result) {
            if ( rsym.get_kind() == DD_CHECKSUM) {
                cksum = rsym.get_name();
            }
        }
    }

    if (cksum == NULL) {
        cksum = "";
    }
    return cksum;
}

static char const *ATT_pset_name( symbolPtr& sym)
{
    Initialize(ATT_pset_name);
 
    char const *pset = NULL;
    projModule*mod = symbol_get_module(sym);
    if(mod)
        pset = realOSPATH(mod->paraset_file_name());
    return pset;
}

int (*ast_declaration)(const symbolPtr&ast);
static int ATT_is_decl( symbolPtr& sym){
    Initialize(ATT_is decl);
    int val = 0;
    if (sym.is_instance()) {
      MetricElement mel = sym.get_MetricElement();
      if (!mel.isnull())
        val = (mel[0].type() == MET_DECL_INSTANCE);
    } else if(sym.is_ast() || sym.is_dataCell()) {
      if(ast_declaration)
        val = (*ast_declaration)(sym);
    }
    return val;
}

static char const *ATT_instance_scope( symbolPtr& sym){
    Initialize(ATT_instance_scope);
    char const *val = "\0";
    if (sym.is_instance()) {
      symbolPtr scope = sym.scope();
      if (scope.xrisnotnull())
        val = scope.get_name();
    }
    return val;
}

char const *ATT_instance_scopefile( symbolPtr& sym)
{
    Initialize(ATT_instance_info);
    char const *val = "\0";
    if (sym.is_instance()) {
      symbolPtr scope = sym.scope();
      symbolPtr mod;
      if (scope.xrisnotnull())
        mod = scope->get_def_file();
      if (mod.xrisnotnull()) 
        val = mod.get_name();
    }
    return val;
}

static int ATT_instance_line(symbolPtr& sym)
{
    Initialize(ATT_instance_line);
    if (sym.is_instance()) 
        return sym.line_number();
    return 0;
}

static int ATT_instance_column(symbolPtr& sym)
{
  Initialize(ATT_instance_column);

  int ret = 0;

  if (sym.is_instance())
    return sym.line_offset();

  return ret;
}

int ATT_line(symbolPtr& sym)
{
    ddKind kind = sym.get_kind();
    if(kind == DD_INSTANCE) {
        return sym.line_number();
    } else if(kind == DD_AST) {
        return astNode_get_line_number(sym);
    }
    int result = -1;
    symbolArr defs;
    sym.get_def_symbols(defs);
    if(defs.size() == 1)
        Metric::get_metric_nocompute(MET_START_LINE, defs[0], (void *) &result);
    if(result < 0 && sym.relationalp()){
        Relational *rel = (Relational *)sym;
        if(is_smtTree(rel)){
            smtTree *smt = (smtTree *)rel;
            result       = smt->src_line_num;
        } else { 
            symbolPtr xsym = sym.get_xrefSymbol();
            xsym.get_def_symbols(defs);
            if(defs.size() == 1)
                Metric::get_metric_nocompute(MET_START_LINE, defs[0], (void *) &result);
        }
    }   
    if (result < 0 && (kind == DD_CLASS || kind == DD_FUNC_DECL
                                        || kind == DD_VAR_DECL)) {
      // This might be a template instantiation.
      symbolArr templates;
      sym->get_link(has_template, templates);
      if (templates.size() > 0) {
        symbolPtr tmpl = templates[0].get_xrefSymbol();
        if (!tmpl.isnull()) {
          result = ATT_line(tmpl);
        }
      }
    }
    return result;
}

static int ATT_instance_offset(symbolPtr& sym)
{
    Initialize(ATT_instance_offset);
    if (sym.is_instance()) 
        return sym.char_offset();
    return 0;
}

static int ATT_smt_offset(symbolPtr& sym)
{
    smtTree *smt = GetSmtFromMarker(sym);
    return smt ? smt->istart() : 0;
}

static int ATT_offset(symbolPtr& sym)
{
  int result;
  ddKind kind = sym.get_kind();
  if(kind == DD_SMT)
    result =  ATT_smt_offset(sym);
  else {
    result =  sym.char_offset();
    if (result <= 0 && (kind == DD_CLASS || kind == DD_FUNC_DECL
                                         || kind == DD_VAR_DECL)) {
      // This might be a template instantiation.
      symbolArr templates;
      sym->get_link(has_template, templates);
      if (templates.size() > 0) {
        symbolPtr tmpl = templates[0].get_xrefSymbol();
        if (!tmpl.isnull()) {
          result = ATT_offset(tmpl);
        }
      }
    }
  }

  return result;
}

static int ATT_region_length(symbolPtr& sym)
{
    smtRegion *region = GetRegion(sym);
    if(region){
        smtTree *smt_start = GetSmtFromMarker(region);
        smtTree *smt_end   = GetSmtFromMarker(region->end);
        if(smt_start != NULL && smt_end != NULL){
            return(smt_end->start() + smt_end->length() - smt_start->start());
        }
    }
    return 0;
}

//
// Please note that "static genString" is used to store return value in most of
// the "ATT_" functions. When operator= is called next time the original 
// memory is free'd, making the current string pointer invalid. So make a copy 
// of the string if you want to save the result.
//

char const *ATT_instance_info( symbolPtr& sym)
{
    Initialize(ATT_instance_info);
    static genString info = "";
    info = "\0";
    if (sym.is_instance()) {
      symbolPtr scope = sym.scope();
      symbolPtr mod;
      if (scope.xrisnotnull())
        mod = scope->get_def_file();
      int line = sym.line_number();
      char const *mname = mod.get_name();
      if (mname) {
        info += mname;
        info += " : ";
      }       
      char buf[1024];
      OSapi_sprintf(buf,"%d\0",line);
      info += buf;
      info += "\0";
    }
    return (char const *) info;
}

char const *ATT_name( symbolPtr& sym)
{
    return sym.get_name();
}

static char const *proj_get_phys_name(projNode*pn)
{
  Initialize(proj_get_phys_name);
  IF(!pn)
    return 0;

  if (! pn->pr_name) {
    char const *name = pn->get_name();
    genString phname;

    pn->ln_to_fn_imp(name, phname, DIR_TYPE);

    if (phname.length())
      pn->pr_name = global_cache.add_entry(phname);
  }
  return
   pn->pr_name->get_name();
}

void scope_get_projects(symbolPtr& sym, symbolArr& res);

char const *ATT_physical_filename(symbolPtr &sym)
{
  Initialize(ATT_physical_filename);

  ddKind kind = sym.get_kind();

  char const *def_file = 0;
  static  genString phys_name;
  
  if (kind == DD_SCOPE) {
    symbolArr projs;
    scope_get_projects(sym, projs);
    if(projs.size() == 1) {
      projNode*pn = projNodePtr(RelationalPtr(projs[0]));
      return realOSPATH( (char const *) proj_get_phys_name(pn) );
    } else {
      return 0;
    }
  } else if (kind == DD_PROJECT) {
    if (sym.without_xrefp()) {
      projNode *pn = projNodePtr(RelationalPtr(sym));
      return realOSPATH( (char const *)proj_get_phys_name(pn) );
    }

    char const *proj_name = sym.get_name();
    project_name_to_fn(proj_name, phys_name);
    return realOSPATH( (char const *)(phys_name.str()) );  
  } else if (kind == DD_EXTFILE) {
    char const *fn = sym.get_name();
    return realOSPATH(fn);
  }

  symbolPtr dfs =  (symbolPtr)NULL_symbolPtr;
  if (kind == DD_MODULE) {
    dfs = sym;
  } else if (!api_def_less_kind(kind)) {
    if (sym.is_instance()) {
      symbolPtr scope = sym.scope();

      if (scope.xrisnotnull())
        dfs = scope->get_def_file();
    } else if (sym.is_ast()) {
      astXref *axr = sym.get_astXref();

      if (axr)
        dfs = astXref_get_module(axr);
    } else if (sym.is_dataCell())
      dataCell_get_module(sym, dfs);
    else {
      if (sym.xrisnotnull()) {
        dfs = sym->get_def_file();
        if (dfs.isnull() && (kind == DD_CLASS || kind == DD_FUNC_DECL
                                              || kind == DD_VAR_DECL)) {
          // This might be a template instantiation.
          symbolArr templates;
          sym->get_link(has_template, templates);
          if (templates.size() > 0) {
            symbolPtr tmpl = templates[0].get_xrefSymbol();
            if (!tmpl.isnull()) {
              dfs = tmpl->get_def_file();
            }
          }
        }
      }
    }
  }
     
  if (dfs.isnull())
    return 0;

  def_file = dfs.get_name();

  projModule*mod = NULL;
  if (def_file && *def_file) {
      projNode *pr = NULL;
      phys_name = NULL;
      //boris: no xref for projModule
      if (dfs.relationalp()) {
        Relational *rel = (Relational *)dfs;
        if(is_projModule(rel)) {
          mod = (projModule*)rel;
        } else if(is_app(rel)) {
          mod = appHeader_get_projectModule((app*) rel);
        } 
        if (mod)
          phys_name = mod->get_phys_filename();
      } else {
          pr = projHeader::get_pmod_project_from_ln(def_file);
          if (pr) {
            pr->ln_to_fn(def_file, phys_name);
          }
      }
  }
  if(phys_name.str() == NULL && def_file){
    mod = projHeader::find_module(def_file);
    char ch;
    if(mod)
      phys_name = mod->get_phys_filename();
    else if (strlen(def_file) >= 8 && def_file[0] == '/'
		  && strncmp(def_file+2, "_drive", 6) == 0
		  && ((ch = def_file[8]) == '/' || ch == '\\' || ch == '\0')) {
      // Convert /x_drive... to x:..., corresponding to hack in smt_ifl.cxx.
      phys_name.printf("%c:%s", def_file[1], def_file+8);
    }
  }
  if((!phys_name.str()) && def_file)
    return def_file;
  else
    return realOSPATH( (char const *)(phys_name.str()));
}

char const *ATT_project_filename( symbolPtr& sym)
{
  Initialize(ATT_project_filename);
  if (sym.xrisnull() && !sym.without_xrefp())
      return "\0";

  ddKind kind = sym.get_kind();

  
  if (kind == DD_PROJECT)
    return sym.get_name();

  char const *def_file = 0;
  static  genString pname;

  pname = 0;

  if(kind == DD_MODULE) {
    def_file = sym.get_name();
  } else {
    symbolPtr dfs =  (symbolPtr)NULL_symbolPtr;
    if (! sym.xrisnull())
      dfs = sym->get_def_file();
    if (! dfs.isnull())
      def_file = dfs.get_name();
  }
     
  if ( def_file && *def_file) {
      projNode *pr = NULL;
      //boris: no xref for projModule
      if (sym.without_xrefp()) {
          Relational *rel = (Relational *)sym;
          projModule *mod = checked_cast(projModule,rel);
          if (mod)
              pr = mod->get_project();
      } else
          pr = projHeader::get_pmod_project_from_ln(def_file);

      if (pr){
          genString fname;
          pr->ln_to_fn(def_file, fname);
          pr->fn_to_ln_imp(fname, pname);
      }
  }
  return pname;
}

char const *dfa_get_type(const symbolPtr& sym);
char const *ATT_type( symbolPtr& sym)
{
    Initialize(ATT_type);
    ddKind kind = sym.get_kind();

    if(kind == DD_AST || kind == DD_DATA)
      return dfa_get_type(sym);
    else if (kind == DD_INSTANCE){
      symbolPtr xsym = sym.get_xrefSymbol();
      if(xsym.isnull())
        return sym.get_name();
      kind = xsym.get_kind();
    }
    if (kind == DD_ENUM_VAL) {
      symbolArr dd_enums;
      sym->get_link(used_by, dd_enums);
      symbolPtr sy;
      ForEachS(sy, dd_enums) {
        if (sy.get_kind() == DD_ENUM) {
          return sy.get_name() ;
        }
      }
    } else if (kind == DD_LOCAL) {
      static ocharstream buf;
      buf.reset();
      buf << sym.get_name() << ends;
      char *type = buf.ptr();
      char *space = strrchr(type, ' ');
      if(space){
        *space = '\0';
        return type;
      }
    } else {
      symbolPtr semtype = get_semtype_symbol(sym);
      if (semtype.xrisnotnull())
        return semtype.get_name();
    }
    return "\0";
}

extern bool dfa_get_type_sym(const symbolPtr &, symbolPtr &);

bool get_type_sym(const symbolPtr &srcsym, symbolPtr &type)
{
  bool ret = false;
  symbolPtr sym = srcsym;
  ddKind kind = sym.get_kind();

  // try to convert instances:
  if (kind == DD_INSTANCE) {
    symbolPtr xsym = sym.get_xrefSymbol();
    if (!xsym.isnull()) {
      kind = xsym.get_kind();
      sym = xsym;
    }
  }

  // ast nodes, data cells:
  if (kind == DD_AST || kind == DD_DATA) {
    if (dfa_get_type_sym(sym, type) == true)
      ret = true;
  }
  // enum values:
  else if (kind == DD_ENUM_VAL) {
    symbolArr dd_enums;
    sym->get_link(used_by, dd_enums);
    symbolPtr s;
    ForEachS(s, dd_enums) {
      if (s.get_kind() == DD_ENUM) {
        type = s;
        ret = true;
        break;
      }
    }
  }
  // strings, locals, everything else:
  else {
    // what if kind is still DD_INSTANCE?
    type = get_semtype_symbol(sym);
    if (type.xrisnotnull()) {
      ret = true;
    }
  }
  return ret;
}

int parse_args( symbolPtr& sym, char * &arguments, genArrOf(int)& indecies)

// returns the number of args
// parameters:
//   sym: the symbol (function) whose arguments you want
//   args: the string containing the argument declarations
//   indecies: an array of theoffset into the string where each argument begins

{
    int numArgs = 0;
    
    arguments = NULL;

    indecies.reset();
    
    ddKind kind = sym.get_kind();
    
    if (kind != DD_FUNC_DECL)
      return 0;

    char const *name       = sym.get_name();
    char const *args = strchr(name, '(');
    if(!args)
        args = xref_c_proto(sym);    
    if(!args)
        return 0;
    // whole string containing arguments enclosed in '('. Get rid of them
    args++;
    static genString buff = "";
    buff       = args;
    char *p    = buff;
    char *end = strrchr(p, ')');
    *end = '\0';

    // buff now contains argument string without enclosing '()'
    char *start = p;
    if(*p != '\0') {
        int temp = 0;  // add position for first argument
        indecies.append(&temp);
        numArgs++;
    }
    int parenNesting = 0;
    while(*p != '\0') {
        if(*p == '(')
            parenNesting++;
        else if(*p == ')')
            parenNesting--;
        else if(*p == ',' && parenNesting == 0){
            // found next argument skip whitespaces if any
            p++;
            while(*p != '\0' && isspace(*p)) p++;
            if(*p != '\0') {
                int temp = p - start;
                indecies.append(&temp);
                numArgs++;
            }
        }
        if(*p != '\0')
            p++;
    }
    arguments = start;
    return numArgs;
}

static void process_arg(char const *args, char const *start, char const *end, intArr& ind_arr)
{
  int *p = ind_arr.grow(2);
  p[0] = start - args;
  p[1] =  end - start + 1;
}

static char const *cli_get_args(const symbolPtr& sym)
{
  ddKind kind = sym.get_kind();
  if (kind != DD_FUNC_DECL) 
    return NULL;
  
  char const *name       = sym.get_name();
  char const *args = strchr(name, '(');
  if(!args)
        args = xref_c_proto(sym);    
  return args;
}

static char const *ATT_args( symbolPtr& sym)
{
  char const *args = cli_get_args(sym);
  return args ? args : "";
}

int cli_parse_function_args(const symbolPtr& sym, char const ** arguments, intArr* ind_arr)

// returns the number of args
// parameters:
//   sym: the symbol (function) whose arguments you want
//   args: the string containing the argument declarations
//   ind_arr: an array of (offset, len) into args

{
  if(arguments)
    *arguments   = NULL;
  if(ind_arr)
    ind_arr->reset();
    
  char const *args = cli_get_args(sym);
  if(!args)
    return 0;
  
  char const *start = NULL;
  char const *end = NULL;
  int no_args = 0;
  int parenNesting = 0;
  int ch;
  for(char const *p = args+1; (ch=*p); ++p) {
    if(ch == '(') {
      parenNesting++;
    } else if(ch == ')') {
      if(parenNesting == 0){
        if(start){
          if(ind_arr)
            process_arg(args, start, end, *ind_arr);
          ++ no_args;
        }
        break;
      } else {
        end = p;
      }
      parenNesting--;
    } else if(ch == ',' && parenNesting == 0){
      if(ind_arr)
        process_arg(args, start, end, *ind_arr);
      ++no_args;
      start = end = NULL;
    } else if (ch!=' '){
      end = p;
      if(!start)
        start = p;
    }
  }
  if(arguments)
    *arguments = args;
  return no_args;
}

int ATT_numargs( symbolPtr& sym)
{
    return cli_parse_function_args(sym, NULL, NULL);
}
int xref_convert_to_num (const symbolPtr& sym, int & pmod_num, int & offset);
char const *ATT_xscope(symbolPtr & sym)
{
    char const *retval = " ";
    scopeNode * sc = sym.get_scope();
    
    if (sc) {
        if (!is_Xref(sc)) {
            retval = sc->get_name();
        } else {
            Xref * Xr = (Xref *)sc;
            projNode * pr = Xr->get_projNode();
            retval = pr->get_name();
        }
    }

    return retval;
}

char const *ATT_rscope(symbolPtr & sym)
{
    char const *retval = " ";
    scopeNode * sc = sym.get_scope();
    
    if (sc) {
        if (!is_Xref(sc)) {
            retval = sc->get_root()->get_name();
        } else {
            Xref * Xr = (Xref *)sc;
            projNode * pr = Xr->get_projNode();
            retval = pr->get_name();
        }
    }

    return retval;
}

char const * ATT_kind(symbolPtr& sym);
static char const * ATT_itag(symbolPtr& sym)
{
  static genString buffer = "";
  ddKind kind = sym.get_kind();
  if(sym.is_xrefSymbol() && kind != DD_MODULE) {
    symbolPtr sym1 = sym.get_xrefSymbol();
    int pmod, off;
    int res = xref_convert_to_num (sym1, pmod, off);
    if (res) {
      buffer.printf("%d/%d\0", pmod, off);
      return buffer;
    }
  }

  switch (kind) {
  case DD_INSTANCE:
    {
      char const *lname = ATT_filename(sym);
      {
        int off = sym.char_offset();
        buffer.printf("%d:%s", off, lname);
      }
    }
    break;
  case DD_AST:
    {
      char const *lname = ATT_filename(sym);
      int off = sym.get_offset() & 0x00ffffff;
      buffer.printf("%d!%s", off, lname);
    }
    break;

  case DD_DATA:
    {
      dataCell *cell = dataCell_of_symbol(sym, false);

      if (cell) {
        symbolPtr symbol;
        dataCell_get_graph(cell, symbol);
        dataCell_offload(cell, false);
        int offset = sym.get_offset();
        offset &= 0x00FFFFFF;
        genString temp = ATT_itag(symbol);
        buffer.printf("%d?%s", offset, (char const *)temp);
      } else
        buffer = "";
    }

    break;

  case DD_BUGS:
    {
    char const *name = sym.get_name();
    char const *kind = ATT_kind(sym);
    buffer.printf( "%s@%s", kind, name);
    }
    break;
  default:
    buffer = sym.get_name();
    break;
  } 
  return buffer;
}

static char const **kind_array;

char const *ATT_kind( symbolPtr& sym)
{
    Initialize(ATT_kind);

    int ind = (int) sym.get_kind();
    char const *result = kind_array[ind];
    if(!result)
        result = ddKind_name(ind) +3;

    return result;
}

char const *ATT_java_kind( symbolPtr& sym)
{
    Initialize(ATT_java_kind);

    ddKind kind = sym.get_kind();
    if (kind == DD_CLASS) {
        return "class";
    }
    else if (kind == DD_FUNC_DECL) {
        return "method";
    }
    return ATT_kind(sym);
}

char const * ATT_etag(symbolPtr& sym)
{
  ddKind kind = sym.get_kind();
  if (sym.is_xrefSymbol() && kind != DD_MODULE) {
    static genString buffer = "";
    buffer = "\0";
    char const *n = sym.get_name();
    char const *k = ATT_kind(sym);
    if(kind == DD_LOCAL){
      buffer.printf("%s@%s", k, n);
      int len = buffer.length();
      char *bufptr = buffer;
      for(int ii=len-1; ii> 0; --ii){
        if(bufptr[ii] == ' ') {
          bufptr[ii] = '/';
          break;
        }
      }
      return buffer;
    } else if (kind == DD_STRING) {
      static genStringPlus buf;
      buf.reset();
      buf += '"';
      buf += n;
      buf += '"';
      return (char *) buf;
    } else {
      char const *lname = ATT_filename(sym);
      if(lname && lname[0])
        buffer.printf("%s@%s@%s", lname, k, n);
      else
        buffer.printf("%s@%s", k, n);
    }
    return buffer;
  } 
   
  return ATT_itag(sym);
}

// When managing symbols in an external attribute dictionary (.attr),
// convert ast nodes into offset (instance-like) notation, so that
// they can be merged and preserved when the file changes:

char const *ATT_attr_tag(symbolPtr &sym)
{
  char const *tag = NULL;

  // in ATT_itag we use the same buffer size:
  static genString buf = "";
  buf = "\0";

  ddKind kind = sym.get_kind();

  if (kind == DD_AST) {
    int offset = sym.char_offset();
    char const *lname = ATT_filename(sym);
    buf.printf("%d:%s", offset, lname);
    tag = buf;
  } else {
    tag = ATT_etag(sym);
  }

  return tag;
}


int ATT_method(symbolPtr& sym)
{
    int result = 0;
    if (sym.get_kind() == DD_FUNC_DECL) {
      symbolPtr xsym = sym.get_xrefSymbol();
      if(xsym.xrisnull()) {
        result = 0;
      } else {
        result = xsym->get_attribute(METHOD,1);
      }
    }
    return result;
}

int ATT_cfunction(symbolPtr& sym)
{
    int result = 0;
    if (sym.get_kind() == DD_FUNC_DECL) {
        char const *name = sym.get_name();
        if (name && *name && !(strchr(name, '('))) {
            result = 1;
        }
    }
    return result;
}

char const *ATT_cname( symbolPtr& sym)
{
  Initialize(ATT_cname);
  char const *result = NULL;
  ddKind kind = sym.get_kind();
  char const *name = sym.get_name();
  if(kind == DD_AST || kind == DD_INSTANCE ||kind == DD_DATA){
    symbolPtr xsym = sym.get_xrefSymbol();
    if(xsym.isnull())
      return name;
    kind = xsym.get_kind();
    name = xsym.get_name();
  }
  if(!name || !*name)
    return NULL;
  switch(kind) {

  case DD_LOCAL:
    result = strrchr(name, ' ');
    if(result)
      ++result;
    else 
      result = name;
    break;
  case DD_MODULE: case DD_PROJECT: case DD_SCOPE:
    result = strrchr(name, '/');
    if(result)
      ++result;
    else 
      result = name;
    break;

  case DD_STRING: case DD_NUMBER:
    result = name;
    break;

  case DD_TEMPLATE:
    {
    char const *pStart = name;
    char const *pEnd = NULL;
    char const *pCur = pStart;
    char const *pLastSpace = NULL;
    int nestingTriang = 0;
    int nestingRound = 0;
    int ch;
    while(ch=*pCur) {
      if((ch==':' || ch=='.') && nestingTriang <= 0 && nestingRound <= 0) {
        pStart = pCur + 1;
      } else if (ch == '<') {
        nestingTriang++;
      } else if (ch == '>') {
        nestingTriang--;
      } else if (ch == '(') {
           if(nestingTriang <= 0 && nestingRound <= 0) {
              pEnd = pCur;
           } else {
             nestingRound++;
           }
      } else if (ch == ')') {
          nestingRound--;
      } else if (ch == ' ') {
          pLastSpace = pCur;
      } else if (ch == '@') {
        pEnd = pLastSpace;
      }
      ++pCur;
	  if(pEnd != NULL && pCur > pEnd) break;
    }
    if(pEnd == NULL) pEnd = pCur;
    static genString buf = "";
    buf.put_value(pStart, (pEnd-pStart));
    result = buf;
    }
    break;

  default:
    {
    char const *tail = NULL;
    if(kind==DD_FUNC_DECL)
      tail = strchr(name, '(');
    if(!tail){
      tail = strchr(name, '@');
      if(tail){
        while(tail[-1] == ' ')
          --tail;
      }
    }
    if(tail){
      static genString buf = "";
      buf.put_value(name, (tail-name) );
      name = buf;
    }

    char const *start = name;
    char const *cur = name;
    int ch;
    int nesting = 0;
    while(ch=*cur) {
      ++cur;
      if((ch==':' || ch=='.') && nesting <= 0)
        start = cur;
      else if (ch == '<')
        nesting += 1;
      else if (ch == '>')
        nesting -= 1;
    }
    result = start;
    }
    break;
  }
  return result;
}

char const *ATT_scope( symbolPtr& sym)
{
  ddKind kind=sym.get_kind();
  if(kind==DD_MODULE||kind==DD_PROJECT||kind==DD_SCOPE)
    return NULL;

  int end = 0;
  int index = 0;
  static genString buff = "";
  buff = "\0";
  char const *name = sym.get_name();
  int i=0;
  while ((name[i] != '(') && (name[i] != '\0')) {
    if ((name[i] == ':') && (name[i+1] == ':')) 
      end = index;
    else if ((name[i] == '.')) 
      end = index;
    index++;
    buff += name[i++];
  }
  char *result = buff;
  result[end] = '\0';
  return result;
}

char const *ATT_note(symbolPtr& sym)
{
    Initialize(ATT_note);
    char const *result = "\0";

    ddElement* dd = sym.get_def_dd();
    if (dd)
      {
        result = dd->def;
      }

    return result;
}

char const *ATT_c_proto(symbolPtr& sym)
{
    Initialize(ATT_c_proto);
    static genString result = "";
    result = "\0";

    char const *ptr = xref_c_proto(sym);
    if (ptr)
      {
        result = ptr;
        // Why the magic 63 byte restriction on size I don't know.
        // But I preserved this limitation when I converted to genstring.
        // Peter.
        if ( result.length() > 59 )
        {
          char *resptr = result;
          resptr[59] = '\0';
        }
        result += " \"C\"";
      }
    return result;
}

static int metric_arg_get_fun(symbolPtr& sym, MetricElement& mel, symbolPtr& result)
{
    int retval = 0;
    int sym_offset = mel.get_value(MET_USED_BY_SYMBOL);
    if (sym_offset) {
        MetricSet* mp = sym.get_MetricSetPtr();
        Xref* xr = NULL;
        projNode* p = set_get_proj(mp);
        if (p)
            xr = p->get_xref();
        if (xr) {
            result.put_info(xr,sym_offset);
            retval = 1;
        }
    }
    return retval;
}

static int metric_is_io(symbolPtr& sym, MetricElement& mel)
{
    int retval = 0;

    symbolPtr xsym;
    metric_arg_get_fun(sym,mel,xsym);
    symbolPtr def_sym = xsym.get_def_symbol();
    if (def_sym.isnull())
        retval = 1;

    return retval;
}

int ATT_io(symbolPtr& sym)
{
    Initialize(ATT_io);

    int retval = 0;

    if (sym.is_instance()) {
        MetricElement mel = sym.get_MetricElement();
        if (!mel.isnull() && mel.get_ast_attribute(ARG_ATT,1))
            retval = metric_is_io(sym,mel);
    }

    return retval;
}

int ATT_call(symbolPtr& sym)
{
    Initialize(ATT_call);

    int retval = 0;

    if (sym.is_instance()) {
        MetricElement mel = sym.get_MetricElement();
        if (!mel.isnull() && mel.get_ast_attribute(ARG_ATT,1))
            retval = !metric_is_io(sym,mel);
    }

    return retval;
}

char const *ATT_argof (symbolPtr& sym)
{
    Initialize(ATT_argof);

    char const *result = "";

    if (sym.is_instance()) {
        MetricElement mel = sym.get_MetricElement();
        if (!mel.isnull() && mel.get_ast_attribute(ARG_ATT,1)) {
            symbolPtr xsym;
            metric_arg_get_fun(sym,mel,xsym);
            result = ATT_itag(xsym);
        }
    }

    return result;
}

char const *ATT_astcategory (symbolPtr& sym)
{
    Initialize(ATT_astcategory);

    static genString buff = "";
    buff = "\0";

    if (sym.is_instance()) {
        MetricElement mel = sym.get_MetricElement();
        if (!mel.isnull()) {
            int need_comma = 0;
            if(mel.get_ast_attribute(ASSIGN_ATT,1)) {
                if (need_comma)
                    buff +=",";
                buff += "assignment";
                need_comma = 1;
            }
            if(mel.get_ast_attribute(VALUE_ATT,1)) {
                if (need_comma)
                    buff += ",";
                buff += "value";
                need_comma = 1;
            }
            if(mel.get_ast_attribute(ADDR_ATT,1)) {
                if (need_comma)
                    buff += ",";
                buff += "address";
                need_comma = 1;
            }
            if(mel.get_ast_attribute(DECL_ATT,1)) {
                if (need_comma)
                    buff += ",";
                buff += "declaration";
                need_comma = 1;
            }
            if(mel.get_ast_attribute(ARG_ATT,1)) {
                if (need_comma)
                    buff += ",";
                if (metric_is_io(sym,mel))
                    buff += "extcall";
                else
                    buff += "call";
                need_comma = 1;
            }
        }
    }
    
    return buff;
}



char const *ATT_vname( symbolPtr& sym)
{
    Initialize(ATT_vname);
    char const *result = "\0";
        if (ATT_cfunction(sym)) {
            result = sym.get_name();
        } else {
            int start = 0;
            int index = 0;
            static genString buff = "";
            buff = "\0";
            char const *name = sym.get_name();
            int i=0;
            bool flag = 0;
            while (name[i] != '\0') {
                if (name[i] == '(')  {flag = 1;};
                if (!flag && (name[i] == ':') && (name[i+1] == ':')) 
                  start = index + 2;
                else if (!flag && (name[i] == '.')) 
                  start = index + 2;
                // index is used to track when the :: or . is found.
                index++;
                buff += name[i++];
            };
            result = buff;  // Convert to char *
            result += start;
        };
    return result;
}

int ATT_pmod( symbolPtr& sym)
{
    Initialize(ATT_pmod);
    ddKind kind = sym.get_kind();

    if (kind == DD_PROJECT) {
        return (sym.get_projNode()->get_xref() != 0);
    };
    return 0;
}

symbolPtr xref_header_file_included_from(char const *fn);
int ATT_included ( projModule& mod)
{
  Initialize(ATT_included);
  char const *path = mod.get_name();
  return 
    xref_header_file_included_from(path).isnotnull();
}

int ATT_header ( projModule& mod)
{
    Initialize(ATT_header);
    char const *path = mod.get_name();
    return (!xref_not_header_file(path));
}

int module_needs_reparse(projModule *module);
int ATT_outdated( projModule& mod) 
{
    int result = module_needs_reparse(&mod);
    if(!result){
        app *appHeader = (app *)get_relation(appHeader_of_projectModule, &mod);
        if(appHeader){
            result = appHeader->needs_to_be_saved();
        }
    }
    return result;
}

bool module_is_not_viewable(projModule *);
int ATT_viewable(projModule &mod)
{
    if (module_is_not_viewable(&mod)) {
        return 0;
    }
    else {
        return 1;
    }
}

init_relational(exprAttribute,intAttribute);

int exprAttribute::test( symbolPtr& sym) const
{
      return op->value(sym);
}

char const *exprAttribute::value( symbolPtr& sym) const
{
    char const *result;
    if (op->valueType() == STRING_TYPE) {
        result = (char const *)op->value(sym);
    } else {
        result = intAttribute::value(sym);
    };
    return result;
}

bool exprAttribute::boolValue( symbolPtr& sym) const
{
    bool result;
    if (op->valueType() == STRING_TYPE) {
        char const *st = (char const *)op->value(sym);
        result = st && *st;
    } else {
        result = intAttribute::boolValue(sym);
    };
    return result;
}

bool exprAttribute::is_int() const
{
    return (op->valueType() != STRING_TYPE);
}

void exprAttribute::print(ostream& str, int) const
{
  str << get_name() << ' ';
  op->print(str);
}

extern "C" symbolAttribute * test_expr();

extern "C" int initialize_random_junk ();
extern "C" void cli_error_reset();

static void add_expr_attr(char const *aname, char const *aexpr)
{
  expr *expression = api_parse_expression(aexpr);
  if(aexpr)
    new exprAttribute(aname,expression);
}


char const *ATT_attribute(symbolPtr& any_sym)
{
    Initialize(ATT_attribute);
    static genStringPlus str;
#define add_comma(string) if(string.size() != 0) string += ", "; else string += '[';

    str.reset();
    symbolPtr sym = any_sym;
    if(!sym.is_xrefSymbol())
        sym = sym.get_xrefSymbol();
    if(sym.isnotnull()) {
        for(int i = PRIV_ATT; i < NEXT; i++){
            if(sym->get_attribute(i, 1))
                switch(i){
                case PRIV_ATT: add_comma(str); str += "PRIVATE"; break;
                case PAKG_ATT: add_comma(str); str += "PACKAGE"; break;
                case PROT_ATT: add_comma(str); str += "PROTECTED"; break;
                case VIRT_ATT: add_comma(str); str += "VIRTUAL"; break;
                case PVIR_ATT: add_comma(str); str += "PURE VIRTUAL"; break;
                case STAT_ATT: add_comma(str); str += "STATIC"; break;
                case CNST_ATT: add_comma(str); str += "CONST"; break;
                case INLI_ATT: add_comma(str); str += "INLINE"; break;
                case VOLT_ATT: add_comma(str); str += "VOLATILE"; break;
                case NATV_ATT: add_comma(str); str += "NATIVE"; break;
                case SYNC_ATT: add_comma(str); str += "SYNCHRONIZED"; break;
                case METHOD:
                    {
                        if (sym.get_kind() != DD_FIELD) {
                            add_comma(str);
                            if (sym.get_kind() == DD_VAR_DECL)
                                str += "MEMBER";
                            else 
                                str += "METHOD";
                        }
                        break;
                    }
                case POINTER:  add_comma(str); str += "POINTER"; break;
                case REF:      add_comma(str); str += "REFERENCE"; break;
                case COMP_GEN_ATT: add_comma(str); str += "GENERATED"; break;
                }
        }
        if(str.size() != 0)
            str += ']';
    }
    return str;
}

// Examine portion of name preceding @, skipping what's nested
// in parentheses, to tell whether name is within a class,
// package, or namespace.  A top level name can be handled
// however specified.
static bool is_name_of_member(char const *name, bool is_java, bool what_if_top)
{
    if (name == NULL) {
        return false;
    }
    int nesting = 0;
    for (char const *p = name;; p += 1) {
        char ch = *p;
        if (ch == '\0') {
            return what_if_top;
        }
        else if (ch == '(') {
            nesting += 1;
        }
        else if (ch == ')') {
            if (nesting > 0) {
                nesting -= 1;
            }
        }
        else if (nesting == 0) {
            if (ch == '@') {
                return false;
            }
            else if (ch == '.' && is_java) {
                return true;
            }
	    else if (ch == ':' && p > name && !is_java && p[-1] == ':') {
		return true;
	    }
        }
    }
}

// Return whether the symbol is a member, for ppp
// (public/protected/private/package) purposes.
// For the sake of the hack's efficiency, we assume that if
// someone is querying ppp then it's for Java or C++, and so
// it's OK to discern the language the fast way.
//
int ATT_oo_member(symbolPtr &sym)
{
    symbolPtr xsym = sym;
    if(!xsym.is_xrefSymbol()) {
        xsym = xsym.get_xrefSymbol();
    }
    if(xsym.isnotnull()) {
        if (xsym->get_attribute(METHOD, 1)) {
	    return 1;
	}
	ddKind kind = xsym.get_kind();
	if (kind == DD_CLASS || kind == DD_UNION || kind == DD_TYPEDEF) {
	    // Assume we can tell whether it's Java based on just whether it's ELS.
	    // Determine this from the name, for want of a better way.
	    bool is_java = (xsym.get_language() == FILE_LANGUAGE_ELS);
	    return (is_name_of_member(xsym.get_name(), is_java, is_java) ? 1 : 0);
	}
    }
    return 0;
}

static char const *ATT_java_attribute(symbolPtr& any_sym)
{
    Initialize(ATT_java_attribute);

    static genStringPlus str;
    str.reset();
    
    symbolPtr sym = any_sym;
    if(!sym.is_xrefSymbol())
        sym = sym.get_xrefSymbol();
    if(sym.isnotnull()) {
        if(!(sym->get_attribute(PRIV_ATT, 1)
             || sym->get_attribute(PAKG_ATT, 1)
             || sym->get_attribute(PROT_ATT, 1))) {
            // It's public only if it's within a class or package scope.
            // Determine this from the name, for want of a better way.
            if (is_name_of_member(sym.get_name(), true, true)) {
                add_comma(str); str += "PUBLIC";
            }
        }
        for(int i = PRIV_ATT; i < NEXT; i++){
            if(sym->get_attribute(i, 1))
                switch(i){
                case PRIV_ATT: add_comma(str); str += "PRIVATE"; break;
                case PAKG_ATT: add_comma(str); str += "PACKAGE"; break;
                case PROT_ATT: add_comma(str); str += "PROTECTED"; break;
                case PVIR_ATT: add_comma(str); str += "ABSTRACT"; break;
                case STAT_ATT: add_comma(str); str += "STATIC"; break;
                case CNST_ATT: add_comma(str); str += "FINAL"; break;
                case NATV_ATT: add_comma(str); str += "NATIVE"; break;
                case SYNC_ATT: add_comma(str); str += "SYNCHRONIZED"; break;
                case METHOD:
                    {
                        if (sym.get_kind() != DD_FIELD && sym.get_kind() != DD_VAR_DECL) {
                            add_comma(str);
                            str += "METHOD";
                        }
                        break;
                    }
                case COMP_GEN_ATT: add_comma(str); str += "GENERATED"; break;
                }
        }
        if(str.size() != 0)
            str += ']';
    }
    return str;
}

extern int ATT_hidden(symbolPtr&sym);
extern int ATT_exe(symbolPtr&sym);
extern int ATT_dll(symbolPtr&sym);
extern int ATT_writable(symbolPtr&sym);

int symbolAttribute::once = 0;

static void kind_array_init()
{
  int sz = (int) NUM_OF_DDS + 3;  // not to crash if up to 3 new ddKind is added
  kind_array = new char const *[sz];
  for(int ii=0; ii<sz; ++ii)
    kind_array[ii] = NULL;

  if (kind_attr_array == 0)
    kind_attr_array = new objArr;
}

int Interpreter::SetupSelector(ddSelector& sel, int argc, char const *argv[])
{
  if(argc <= 1 ||  argv[1][0] != '-'){
    sel.add_all();
    return 1;
  }

  int argpos;
  ostream& os = GetOutputStream();

  for(argpos=1; (argpos < argc) && (argv[argpos][0] == '-'); ++argpos) {
    char const *arg = argv[argpos] + 1;  //skip "-"
    if(strcmp(arg, "all") == 0) {
      sel.add_all();
      continue;
    }

    int   arglen = strlen(arg);
    if(arglen < 3) {
      os << "Too short category specified: " << arg << endl;
      return -1;
    }
    Obj*el;
    bool mismatch = true;
    ForEach(el, *kind_attr_array){
      kindAttribute*attr = (kindAttribute*) el;
      char const *plur =attr->plural;
      if(strncmp(plur, arg, arglen)==0){
        sel.add(attr->kind);
        mismatch = false;
        break;
      }
    }
    if(mismatch){
      os << "Unknown category specified: " <<  argv[1] << endl;
      return -1;
    }
  }

  return argpos;
}

static void kind_append(char const *str, ddKind k, int is_xref = 1, char const *p=0)
{
   int ind = (int)k;
   kindAttribute* attr = new kindAttribute(str, k, is_xref, p);
   kind_attr_array->insert_last(attr);
   if(kind_array[ind] == NULL)
     kind_array[ind] = str;

   if(!p)
     attr->plural.printf("%ss", str);
}

int Init_ast_part();

unsigned char dd_icon (ddKind kind)
{
    switch (kind) {
      case DD_MODULE:    return PIX_DOC_UNKNOWN + 1;
      case DD_PROJECT:   return PIX_FOLDER + 1;
      case DD_SCOPE:     return PIX_FOLDER + 1;
      case DD_TYPEDEF:   return PIX_XTYPEDEF;
      case DD_SEMTYPE:   return PIX_XSEMTYPE;
      case DD_MACRO:     return PIX_XMACRO;
      case DD_FIELD:     return PIX_XFIELD;
      case DD_VAR_DECL:  return PIX_XVARIABLE;
      case DD_FUNC_DECL: return PIX_XFUNCTION;
      case DD_ENUM:      return PIX_XENUMLIST;
      case DD_ENUM_VAL:  return PIX_XENUMITEM;
      case DD_CLASS:     return PIX_XCLASS;
      case DD_UNION:     return PIX_XUNION;
      case DD_SUBSYSTEM: return PIX_XSUBSYSTEM;
      case DD_LINKSPEC:  return PIX_XLINKSPEC;
      case DD_INSTANCE:  return PIX_INSTANCE;
      case DD_AST:       return PIX_INSTANCE;
      case DD_LOCAL:     return PIX_LOCAL;
      case DD_STRING:    return PIX_STRING;
      case DD_BUGS:      return PIX_DEFECT;
      case DD_INTERFACE: return PIX_XINTERFACE;
      case DD_COMPONENT: return PIX_XCOMPONENT;
      case DD_PACKAGE:   return PIX_XPACKAGE;
      default:           return PIX_XUNKNOWN;
    }
}

static char const *ATT_kind_icon(symbolPtr& sym)
{
  static char icon[2] = "?";
  ddKind kind = sym.get_kind();
  *icon = dd_icon(kind);
  return icon;
}

int symbolAttribute::api_init_attrs()
{
  if(kind_array)
    return 1;

  kind_array_init();
    initialize_random_junk ();
    cli_error_reset();

    // kind_append used as a selector lookup array for commands "defines" 
    // and "uses".  Lookup starts from first 3 matching characters so do 
    // not append at the beginning of the array unless it's really necessary
    kind_append("typedef", DD_TYPEDEF);
    kind_append("macro", DD_MACRO);
    kind_append("var", DD_VAR_DECL);
    kind_append("variable", DD_VAR_DECL);
    kind_append("field", DD_FIELD);
    kind_append("funct", DD_FUNC_DECL);
    kind_append("function", DD_FUNC_DECL);
    kind_append("enum", DD_ENUM);
    kind_append("evalue", DD_ENUM_VAL);
    kind_append("struct", DD_CLASS);  // we want "struct" to be printed for DD_CLASS (instead of "class")
    kind_append("structure", DD_CLASS);
    kind_append("class", DD_CLASS, 1, "classes");
    kind_append("union", DD_UNION);
    kind_append("group", DD_SUBSYSTEM);
    kind_append("module", DD_MODULE, 0);
    kind_append("file", DD_MODULE, 0);
    kind_append("proj", DD_PROJECT, 0);
    kind_append("project", DD_PROJECT, 0);
    kind_append("semtype", DD_SEMTYPE);
    kind_append("syntype", DD_SYNTYPE);
    kind_append("templ", DD_TEMPLATE);
    kind_append("template", DD_TEMPLATE);
    kind_append("constant", DD_NUMBER);
    kind_append("string", DD_STRING);
    kind_append("namespace", DD_NAMESPACE);
    kind_append("interface", DD_INTERFACE);
    kind_append("component", DD_COMPONENT);
    kind_append("stringl", DD_STRING, 1, "strings");
    kind_append("local", DD_LOCAL);
    kind_append("scope", DD_SCOPE, 0);
    kind_append("instance", DD_INSTANCE, 0);
    kind_append("smt", DD_SMT, 0);
    kind_append("region", DD_REGION, 0);
    kind_append("ast", DD_AST, 0);
    kind_append("cell", DD_DATA, 0);

    kind_append("semtype", DD_SEMTYPE, 0);
    kind_append("relation", DD_RELATION, 0);

    kind_append("package", DD_PACKAGE);
    kind_append("cursor", DD_CURSOR);
    kind_append("table", DD_SQL_TABLE);
    kind_append("exception", DD_EXCEPTION);
    kind_append("label", DD_LABEL);
    kind_append("externref", DD_EXTERN_REF);
    kind_append("linktype", DD_LINKSPEC);
    kind_append("checksum_kind", DD_CHECKSUM);
    kind_append("language_kind", DD_LANGUAGE);

    kind_append("function_tmpl", DD_TEMPLATE);
    kind_append("struct_tmpl", DD_TEMPLATE);
    kind_append("class_tmpl", DD_TEMPLATE);
    kind_append("union_tmpl", DD_TEMPLATE);
    kind_append("variable_tmpl", DD_TEMPLATE);
    kind_append("field_tmpl", DD_TEMPLATE);
    kind_append("enum_tmpl", DD_TEMPLATE);
    kind_append("constant_tmpl", DD_TEMPLATE);
    kind_append("typedef_tmpl", DD_TEMPLATE);
    kind_append("evalue_tmpl", DD_TEMPLATE);

    kind_append("bug", DD_BUGS);
    kind_append("version", DD_VERSION);
  
    new xrefAttribute("static",    STAT_ATT);
    new xrefAttribute("virtual",   VIRT_ATT);
    new xrefAttribute("private",   PRIV_ATT);
    new xrefAttribute("package_prot", PAKG_ATT);
    new xrefAttribute("protected", PROT_ATT);
    new xrefAttribute("purevirt", PVIR_ATT);
    new xrefAttribute("const",  CNST_ATT);
    new xrefAttribute("member",    METHOD);
    new xrefAttribute("pointer",   POINTER);
    new xrefAttribute("reference", REF);
    new xrefAttribute("generated", COMP_GEN_ATT);
    new xrefAttribute("inline", INLI_ATT);
    new xrefAttribute("volatile", VOLT_ATT);
    new xrefAttribute("native", NATV_ATT);
    new xrefAttribute("synchronized", SYNC_ATT);
    new xrefAttribute("final", CNST_ATT);
    new xrefAttribute("abstract", PVIR_ATT);
    new xrefAttribute("cdtor", CONS_DES);

    new astAttribute("assignment", ASSIGN_ATT);
    new astAttribute("value", VALUE_ATT);
    new astAttribute("addr", ADDR_ATT);
    new astAttribute("astdeclaration", DECL_ATT);
    new astAttribute("argument", ARG_ATT);
    new astAttribute("alevel", ASSIGN_LEVEL_ATT);
    new astAttribute("vlevel", VALUE_LEVEL_ATT);

    new funcStringAttribute("type" , ATT_type);
    new funcStringAttribute("kind_icon" , ATT_kind_icon);
    new funcStringAttribute("arguments" , ATT_args);
    new funcStringAttribute("args" , ATT_args);
    new funcStringAttribute("kind" , ATT_kind);
    new funcStringAttribute("java_kind" , ATT_java_kind);
    new funcStringAttribute("name", ATT_name);
    new funcStringAttribute("cname", ATT_cname);
    new funcStringAttribute("vname", ATT_vname);
    new funcStringAttribute("context", ATT_scope);
    new funcStringAttribute("argof", ATT_argof);
    new funcStringAttribute("astcategory", ATT_astcategory);
 
    new funcStringAttribute("itag" , ATT_itag);
    new funcStringAttribute("etag" , ATT_etag);
    new funcStringAttribute("attr_tag", ATT_attr_tag);
    new funcIntAttribute("numargs" , ATT_numargs);
    new funcIntAttribute("io" , ATT_io);
    new funcIntAttribute("call" , ATT_call);
    new funcIntAttribute("cfunction" , ATT_cfunction);
    new funcIntAttribute("strange" , ATT_strange);
    new funcStringAttribute("xscope" , ATT_xscope);
    new funcStringAttribute("rscope" , ATT_rscope);
    new funcIntAttribute("method" , ATT_method);
    new funcIntAttribute("oo_member", ATT_oo_member);
    new funcIntAttribute("hidden" , ATT_hidden);
    new funcIntAttribute("exe" , ATT_exe);
    new funcIntAttribute("dll" , ATT_dll);
    new funcIntAttribute("writable" , ATT_writable);
        
    new funcStringAttribute("lname" , ATT_filename);
    new funcStringAttribute("fname" , ATT_physical_filename);
    new funcStringAttribute("pname" , ATT_project_filename);
    new funcStringAttribute("language" , ATT_language);     
    new funcStringAttribute("checksum" , ATT_checksum);     
    new funcStringAttribute("psetfile" , ATT_pset_name);


    new funcStringAttribute("inst_info" , ATT_instance_info);
    new funcStringAttribute("inst_scope" , ATT_instance_scope);
    new funcIntAttribute("inst_line" , ATT_instance_line);
    new funcIntAttribute("inst_column" , ATT_instance_column);
    new funcIntAttribute("inst_offset" , ATT_instance_offset);
    new funcIntAttribute("declaration" , ATT_is_decl);
    new funcIntAttribute("smt_offset" , ATT_smt_offset);
    new funcIntAttribute("line" , ATT_line);
    new funcIntAttribute("offset" , ATT_offset);
    new funcIntAttribute("region_length" , ATT_region_length);

    new moduleIntAttribute("included" , ATT_included);  
    new moduleIntAttribute("header" , ATT_header);      
    new moduleIntAttribute("outdated" , ATT_outdated);  
    new moduleIntAttribute("viewable" , ATT_viewable);  
    new funcIntAttribute("haspmod" , ATT_pmod);           

    new funcStringAttribute("note", ATT_note);
    new funcStringAttribute("c_proto", ATT_c_proto);

    new funcStringAttribute("attributes", ATT_attribute);
    new funcStringAttribute("java_attributes", ATT_java_attribute);
    // Now define an attribute for each metric.

    for (int type = 0; type < NUM_OF_METRICS; type ++) {
        genString attr_name = "met_";
        attr_name += Metric::def_array[type]->abbrev;
        new metricAttribute((char const *)attr_name, Metric::def_array[type]->type);
    }

    // sioffe: Some sorting/formatting attributes.

    new funcStringAttribute("ff", sym_get_form_str);
    new funcStringAttribute("ox", sym_get_extension);
    new funcStringAttribute("ot", sym_get_symname);        //category??
    new funcStringAttribute("of", sym_get_filename);
    new funcStringAttribute("dn", sym_get_filename);       //used
    new funcIntAttribute   ("ol", sym_get_line);
    new funcStringAttribute("os", sym_get_symname);        //used
    new funcStringAttribute("on", sym_get_symname);

// derived attrs

    int temp = builtinlist();
    DBG_CLI test_expr();

    add_expr_attr("public", "oo_member && !(private || protected || package_prot)");
    add_expr_attr("ppp", "oo_member ? private ? @private : protected ? @protected : package_prot ? @package : @public : @\0");

    Init_ast_part();
    return 0;
}

static int att_initializer = symbolAttribute::api_init_attrs();
extern objArr * cli_comm_array;

int filterDiscoverCmds(Tcl_Interp * interp);

int filterDiscoverCmds(Tcl_Interp * interp)
{
  char const *seps= " \n\t";
    
  genString gsNewResult(""); //for better perfmance, initialize this to the size of interp->result
  static genArrCharPtr gacpExcludedCommands;
  static int nFileRead=0; //when we read the excluded discover commands this will be = to 1
  
//first parse file
  if (!nFileRead)
    {
      const int LINE_LEN =80;
      char cCurLine[LINE_LEN+1];
      char *pCurLine = cCurLine;
      cCurLine[LINE_LEN]='\0';
      
      genString gFileName=OSapi_getenv("PSETHOME");
      char const *fName="/lib/exclcmds.txt";
      gFileName+=fName;
      FILE * fCurFile=OSapi_fopen(gFileName, "r");
      if (fCurFile)
        {
          while( OSapi_fgets(pCurLine, LINE_LEN, fCurFile)!=NULL)
            {
              for (int i=strlen(pCurLine)-1; i>=0; i--)
                if (isspace(pCurLine[i])) 
                  {
                    pCurLine[i] = '\0';
                  }
                else
                  i = 0;
              //append it to the gacpExcludedCommands somehow
              //create a new buffer for it...
              char *pBuffer= new char[strlen(pCurLine)+1];
              strcpy(pBuffer, pCurLine);
              gacpExcludedCommands.append(&pBuffer);
            }
          OSapi_fclose(fCurFile);
        }
      nFileRead=1;
    
    }
  //then go through the commands and remove them.
  char *buf = strdup(Tcl_GetStringResult(interp));
  char *pDisCmd=strtok(buf, seps);
  int bNeedsSpace=0;
  while (pDisCmd)
    {
      int z=0;
      int nAddCmd=1;
      for (z=0; z< gacpExcludedCommands.size(); z++)
        {
          if (strcmp(pDisCmd, *gacpExcludedCommands[z])==0)
              { nAddCmd=0;
                break;
              }
        }
      //spaces are added beforehand to ensure that the finished string will
      // fit in the original buffer.
      if (nAddCmd)
        {if (bNeedsSpace)
           {gsNewResult+=" ";}
         else
           bNeedsSpace=1;
         gsNewResult+=pDisCmd;
       }
     pDisCmd=strtok(NULL, seps);
    }
  free(buf);
  // then replace the original list...
  Tcl_SetResult(interp, (char *)gsNewResult, TCL_VOLATILE);
  return TCL_OK;
}

int dis_infoCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
    int nMustConvertResult = 1;
    int nMustFilterCmds    = 0;
    int bPattern           = 0; //true if we are doing a pattern match
    int nRet               = TCL_ERROR;
    Interpreter*i          = GET_INTERPRETER(interp);
    if (argc < 2) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         " {attributes astcodes astcategories discover kinds ...} ?arg arg ...?\"", (char const *) NULL);
        return TCL_ERROR;
    }

    char const *cmd = argv[1];
    int length     = strlen(cmd);
    int c          = cmd[0];
  
    if (argc==3){
        //we are a bit sloppy here--we don't parse the full set of
        //string match capabilities
        char const *p=strchr(argv[2], '*');
        char const *q=strchr(argv[2], '?');
        bPattern=(p||q);
    }

    Obj* attrs;
    if ((c == 'a') && (strncmp(cmd, "attributes", length)) == 0) {
        attrs = symbolAttribute::get_all_attrs();
    } else if((c == 'a') && (strncmp(cmd, "astcodes", length)) == 0) {
        attrs = ast_codes;
    } else if((c == 'a') && (strncmp(cmd, "astcategories", length)) == 0) {
        attrs = ast_categories;
    } else if ((c == 'c') && (strncmp(cmd, "commands", length))==0){
        nRet = (*i->orig_info_cmd.proc)(i->orig_info_cmd.clientData, interp, argc, argv);
        if ((argc == 2 || bPattern) && nRet != TCL_ERROR){
            nMustFilterCmds=1;
            nMustConvertResult=0;
        }
        else
            return nRet;
    } else if((c == 'd') && (strncmp(cmd, "discover", length)) == 0){
        attrs = cli_comm_array;
        if (argc == 2 || bPattern) //this is just to cover the case info discover
            nMustFilterCmds=1;
    } else if((c == 'k') && (strncmp(cmd, "kinds", length)) == 0) {
        attrs = &kinds;
    } else {
        return (*i->orig_info_cmd.proc)(i->orig_info_cmd.clientData, interp, argc, argv);
    }
  
    if (argc > 3) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                         " ", cmd, " ?pattern?\"", (char const *) NULL);
        return TCL_ERROR;
    }

    if (nMustConvertResult){
        Obj * el;
        ForEach(el, *attrs){
            symbolAttribute*attr = (symbolAttribute*) el;
            char const *name = attr->get_name();
            if ((argc == 3) && !Tcl_StringMatch(name, argv[2])) {
                continue;
            }
            Tcl_AppendElement(interp, name);
        }
    }
    nRet=TCL_OK;
   
    if(nMustFilterCmds)
        nRet=filterDiscoverCmds(interp);
    return nRet;
}
