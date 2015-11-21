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
#include <general.h>
#include <genError.h>
#include <projHeader.h>
#include <proj.h>
#include <api_internal.h>
#include <Interpreter.h>
#include <attribute.h>
#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <stdarg.h>
#else /* ISO_CPP_HEADERS */
#include <cstdarg>
#endif /* ISO_CPP_HEADERS */
#include <metric.h>
#include <machdep.h>
#include <scopeMgr.h>
#include <XrefTable.h>
#include <mpb.h>
#include <msg.h>
#include <driver_mode.h>

extern "C" void parse_selection(int argc, char const **argv);
extern "C" int  DI_parse_error_code;
extern "C" int  use_whole_set;
extern "C" char selection_parse_error[];
symbolPtr astNode_get_node(char const *,int);
symbolPtr dataCell_get_node(char const *,int);

void lookup_scope_or_project (char const *, symbolPtr&);

int (*groupFunct)(char const* name, symbolArr* arr);
symbolPtr api_lookup_xrefSymbol(ddKind kind, char const *str);

struct instMod {
   projModule*mod;
   int line;
   int column;
};
static objNameSet * module_set;
static genArr(instMod) * instMod_arr;

static int api_find_file_try_module(char const *str, symbolPtr& sym)
{
    //logical name
    if(module_set){
	Relational* mod = module_set->lookup(str);
	if(mod) {
	    sym = mod;
	    return 1;
	}
    } 
    sym = api_lookup_xrefSymbol(DD_MODULE, str);
    if (sym.isnotnull()){
	int fileType = sym.get_language();
	//  (we have -1 if no link was found)
	if (fileType != -1)
	    return 1;
	
	symbolPtr proj_sym = api_lookup_xrefSymbol(DD_PROJECT, str);
	if(proj_sym.isnull())
	    return 1;
	else
	    sym = (Obj*)NULL;
    }
    
    // physical name
    projNode *proj  = NULL;
    projModule *mod = 0;
    genString ln;
	
    projHeader::fn_to_ln(str, ln, &proj);
    if(proj){
	mod = proj->find_module(ln);
        sym = mod;
	return mod != NULL;
    }

    // last chance, including ln with no model
    mod = projHeader::find_module(str);
    if(mod){
        sym = mod;
	return 1;
    }
    return 0;
}

static int api_find_file_try_project(char const *str, symbolPtr& sym)
{
    //boris: looks first for the scope if the scope_mode is not zero,
    //       otherwise looks first for the project
    lookup_scope_or_project (str, sym);

    // unit or scope
    if (sym.isnotnull() && sym.get_kind() == DD_SCOPE)
	return 1;

    // project
    if (sym.isnotnull() && sym.get_kind() == DD_PROJECT)
	if(strcmp(sym.get_name(), str) == 0)
	    return 1;
    
    sym = (Obj*)NULL;
    return 0;
}

symbolPtr api_find_file(char const *str, int try_module_first)
{
    symbolPtr result;

    if (try_module_first) {
	if (!api_find_file_try_module(str,result))
	    api_find_file_try_project(str,result);
    } else {
	if (!api_find_file_try_project(str,result))
	    api_find_file_try_module(str,result);
    }

    return result;
}

symbolPtr api_find_function(char const *str)
{
  return
    api_lookup_xrefSymbol(DD_FUNC_DECL, str);
} 

static kindAttribute* cur_attr = 0;
static int glob_attr = 0;

extern "C" void output_ddKind(char *text, int len, int global_flag)
{
  --len;
  int ch = text[len];
  text[len] = '\0';

  char const *name = text[0]=='(' ? text+1 : text;
  symbolAttribute* attr = symbolAttribute::get_by_name(name);

  text[len] = ch;

  if(!attr){
    msg("$1: not attribute\n") << text << eom;
    return;
  }
  if ((!is_kindAttribute(attr)) || (strcmp(attr->get_name(),"group")==0)) {
    msg("$1: not a kind\n") << text << eom;
    return;
  }  
  cur_attr = (kindAttribute*)attr;
  glob_attr = global_flag;
}

symbolPtr api_find_symbol_with_kind(char const *str)
{
 ddKind kind = cur_attr->kind;
 if(!glob_attr)
   cur_attr = 0;
 
 return
    api_lookup_xrefSymbol(kind, str);
 
}
symbolPtr api_find_symbol(char const *str)
{
  if(cur_attr)
    return api_find_symbol_with_kind(str);

  symbolPtr sym;

  sym = api_lookup_xrefSymbol(DD_FUNC_DECL, str);
  if(sym.isnotnull())
    return sym;

  sym = api_lookup_xrefSymbol(DD_VAR_DECL, str);
  if(sym.isnotnull())
    return sym;

  sym = api_lookup_xrefSymbol(DD_CLASS, str);
  if(sym.isnotnull())
    return sym;

  sym = api_lookup_xrefSymbol(DD_INTERFACE, str);
  if(sym.isnotnull())
    return sym;

  sym = api_lookup_xrefSymbol(DD_MACRO, str);
  if(sym.isnotnull())
    return sym;

  sym = api_lookup_xrefSymbol(DD_TYPEDEF, str);
  if(sym.isnotnull())
    return sym;

  sym = api_lookup_xrefSymbol(DD_ENUM, str);
  if(sym.isnotnull())
    return sym;

  sym = api_lookup_xrefSymbol(DD_ENUM_VAL, str);
  if(sym.isnotnull())
    return sym;

  sym = api_lookup_xrefSymbol(DD_SEMTYPE, str);
  if(sym.isnotnull())
    return sym;

  return
    api_lookup_xrefSymbol(DD_UNKNOWN, str);
} 

static symbolArr * cur_set;
static symbolArr * selection;
static symbolArr * temp;

static int  ignore_error;
static int  ignore_error_default = -1;  // warning

void cli_set_error_default(int def) {ignore_error_default = def;}

static void errmsg(char const* fmt, ...)
{
  va_list pvar;
  va_start(pvar, fmt);
  if(ignore_error != -1)
    vsprintf(selection_parse_error, fmt, pvar);
  else 
    vfprintf(stderr, fmt, pvar);
  va_end(pvar);
  DI_parse_error_code = -1;
}

static boolean allow_null_symbols = false;

void api_add_selection(symbolPtr sym)
{
    if (sym.isnotnull()){
	selection->insert_last(sym);
    }
    else {
        if (allow_null_symbols){
	  selection->insert_last(sym);
	}
        errmsg("One or more symbols could not be found.\n"); 
    }
}
extern "C" void output_filename(char const *str)
{
  api_add_selection(api_find_file(str,0));
}

extern "C" void output_NT_filename(char const *str)
{
	projNode *proj  = NULL;
	projModule *mod = 0;
	genString ln;
	char *dis_path = 0;

	create_path_2DIS( str, &dis_path );
	if ( !dis_path )
		return;
	
	projHeader::fn_to_ln(dis_path, ln, &proj);
	free(dis_path);

	if(proj){
	   mod = proj->find_module(ln);
	   if ( mod ) {
	      symbolPtr sym = mod;
          api_add_selection(mod);
       }
    }
}

static bool store_instance_info(int line, int line_offset, char const *str)
{
  projModule* mod =  (projModule*) module_set->lookup(str);
  if(!mod)
    return false;
  instMod*ins = instMod_arr->grow();
  ins->mod = mod;
  ins->line = line;
  ins->column = line_offset;
  return true;
}
#define NULL_LINE -9
extern "C" void output_reference(int off, char const *str)
{
  if(instMod_arr){
    store_instance_info(NULL_LINE, off, str);
  } else {
    api_add_selection(MetricElement::get_instance(str, off));
  }
}

extern "C" void output_reference_loff(int line, int line_offset, char const *str)
{
  if(instMod_arr){
    store_instance_info(line, line_offset, str);
  } else {
    api_add_selection(MetricElement::get_instance(str, line, line_offset));
  }
}
extern "C" void output_ast(int off, char const *str)
{
  symbolPtr sym = astNode_get_node(str,off);
  api_add_selection(sym);
}

static symbolPtr current_scope;
int current_scope_flag;

extern "C" void output_scope(char *str)
{
  genString nm = str;
  str = nm;
  str[nm.length() -1] = '\0';
  current_scope = api_find_file(str,1);
  current_scope_flag = 1;
}

extern "C" void output_group(char const *str,int flag_errors)
{
    if (groupFunct) {
	temp = new symbolArr();
	if (groupFunct(str,temp) == 0) {
	    cur_set = temp;
	} else {
	    if (flag_errors) {
		errmsg("Group '%s' does not exist\n", str); 
	    } else {
		use_whole_set = 0;
	    };
	};
    } else {
	if (flag_errors) {
	    errmsg("Group commands not availible"); 
	} else {
	    use_whole_set = 0;
	};
    };
}

extern "C" void output_semtype(char const *str)
{
  symbolPtr sym;
  if(cur_attr)
    sym = api_find_symbol_with_kind(str);
  else {
    sym = api_lookup_xrefSymbol(DD_SEMTYPE, str);
  }
  api_add_selection(sym);
  if (sym.isnull())
    errmsg("Semtype '%s' cannot be found.\n", str);    
}
extern "C" void output_field(char const *str)
{
  symbolPtr sym;
  if(cur_attr)
    sym = api_find_symbol_with_kind(str);
  else {
    sym = api_lookup_xrefSymbol(DD_FIELD, str);
    if(sym.isnull())
      sym = api_find_symbol(str);
  }
  api_add_selection(sym);
  if (sym.isnull())
    errmsg("Field '%s' cannot be found.\n", str);    
}

extern "C" void output_string(char *str)
{

  int len = OSapi_strlen(str);
  str[len-1] = '\0';
  symbolPtr sym = api_lookup_xrefSymbol(DD_STRING, str+1);
  str[len-1] = '"';
  api_add_selection(sym);

}
  
extern "C" void output_local(char *str)
{
  // LOCAL VARIABLE
  // replace '/' with ' '
  int len = OSapi_strlen(str);
  bool found = false;
  for(int ii=len-1; ii> 0; --ii){
    if(str[ii] == '/') {
      found = true;
      str[ii] = ' ';
      break;
    }
  }
  if(found){
    symbolPtr sym = api_lookup_xrefSymbol(DD_LOCAL, str);
    api_add_selection(sym);
  } else {
    api_add_selection((symbolPtr)NULL_symbolPtr);
  }    
}

extern "C" void output_symbol(char const *str)
{
    // TRY NORMAL SYMBOLS
    symbolPtr s = api_find_symbol(str);
    if (s.isnotnull()) {
	api_add_selection(s);
    } else {
	// TRY GROUP
	temp = new symbolArr();	    
	if (groupFunct  &&  (groupFunct(str,temp) == 0)) {
	    use_whole_set = 1; 	
	    cur_set = temp;
	} else {
	    delete temp;
	    temp = NULL;
	    api_add_selection(s);
	    errmsg("Symbol '%s' cannot be found.\n", str); 
	}
    }
}

extern "C" void output_function(char const *str)
{
    symbolPtr sym = api_find_function(str);
    if (sym.isnotnull()) {
      api_add_selection(sym);
      return;
    }
// member ptr
    sym = api_lookup_xrefSymbol(DD_CLASS, str);
    if (sym.isnotnull()) {
	api_add_selection(sym);
        return;
    } else {
      api_add_selection(sym);
      errmsg("Function '%s' cannot be found.\n", str); 
    }
}

extern "C" void output_template(char const *str)
{
    symbolPtr sym = api_lookup_xrefSymbol(DD_TEMPLATE, str);
    api_add_selection(sym);
    if (sym.isnull()) {
      errmsg(" '%s' cannot be found.\n", str); 
    }
}
extern "C" void output_member_ptr(char const *str)
{
    symbolPtr sym = api_lookup_xrefSymbol(DD_CLASS, str);
    if (sym.isnotnull()) {
	api_add_selection(sym);
        return;
    }

    sym = api_lookup_xrefSymbol(DD_FUNC_DECL, str);
    api_add_selection(sym);
    if (sym.isnull()) {
      errmsg(" member pointer '%s' cannot be found.\n", str); 
    }
}

extern "C" void append_array()
{
    if(cur_set)
	selection->insert_last(*cur_set);
    else
	DI_parse_error_code = -1;
}

extern "C" void output_ind(int ind)
{
    if(cur_set && (ind > 0) && (cur_set->size() >= ind))
	api_add_selection((*cur_set)[ind - 1]);
    else {
      api_add_selection((symbolPtr)NULL_symbolPtr);
      errmsg("Bad set index: %d\n",ind);
    }
}
extern "C" void output_set_element(int set_ind, int el_ind)
{
    const symbolArr* arr = get_DI_symarr(set_ind);
    if(arr && (el_ind > 0) && (arr->size() >= el_ind))
	api_add_selection((*arr)[el_ind - 1]);
    else {
      api_add_selection((symbolPtr)NULL_symbolPtr);
      errmsg("Bad set index: %d\n",el_ind);
    }
}

extern "C" void output_obj(int ind)
{
    cur_set = get_DI_symarr(ind);
}

symbolArr * get_top_list();
extern "C" void output_roots()
{
    symbolArr * roots = get_top_list();
    int sz = roots->size();
    for(int ii = 0; ii<sz; ++ii)   
	api_add_selection((*roots)[ii]);
}

int
parse_selection(int ignore_default,
		int argc,
		char const **argv,
		symbolArr *set,
		symbolArr &result)
{
  Initialize(parse_selection(int, int, char const **, symbolArr *, symbolArr &));

  temp                = NULL;
  DI_parse_error_code = 0;
  ignore_error        = ignore_default;
  use_whole_set       = 0;
  selection           = &result;
  cur_set             = set;

  if (argc == 0)
    selection->insert_last(*set);
  else {
    parse_selection(argc, argv);

    if (use_whole_set) {
      if (cur_set)
	selection->insert_last(*cur_set);
      else
	DI_parse_error_code = -1;
    }
  }

  if (temp) {
    delete temp;
    temp = NULL;
  }

  if (ignore_error) {
    DI_parse_error_code = 0;
    ignore_error = ignore_error_default;
  }

  return DI_parse_error_code;
}

int parse_selection(int argc, char const **argv, symbolArr *set, symbolArr& result)
{
  return parse_selection(ignore_error_default, argc, argv, set, result);
}

struct lexState {
 char *buf;
 int   cur_ind ;
 int   cur_arg ;
 char ** cur_argv ;
 int   cur_argc ;
 int   use_whole_set       ;
 int   DI_parse_error_code ;
};
extern "C" void lex_state_save(lexState*);
extern "C" void lex_state_restore(lexState*);
extern void proj_get_modules(projNode *proj, objArr& modules, int depth); 
extern int scopeMgr_proj_is_special (projNode *proj);
void api_collect_modules(objNameSet&mods)
{
  Initialize(api_collect_modules);
  projNode *proj;

  int ind = is_model_server() ? 1 : 0;
  mpb_mpb_segment(30, 1);
  for(;proj=projList::search_list->get_proj(ind);ind++) {
    if (scopeMgr_proj_is_special(proj))
      continue;

    objArr children;
    proj_get_modules(proj, children, 0);
    int sz = children.size();
    for(int ii=0; ii<sz; ++ii){
      mods.insert((Relational*) children[ii]);
    }
  }
  mpb_mpb_segment(0);
}

static int instMod_compare(const void* p1, const void* p2)
{
  instMod*i1 = (instMod*) p1;
  instMod*i2 = (instMod*) p2;
  int diff = i1->mod - i2->mod;
  if(diff)
    return diff;
  diff = i1->line - i2->line;
  if(diff)
    return diff;
  diff = i1->column - i2->column;
  return diff;
}

struct instInd {
  int ind;
  int line;
  int column;
};

static int instInd_compare(const void* p1, const void* p2)
{
  instInd*i1 = (instInd*) p1;
  instInd*i2 = (instInd*) p2;
  int  diff = i1->line - i2->line;
  if(diff)
    return diff;
  diff = i1->column - i2->column;
  return diff;
}


bool module_get_instances(symbolPtr& mod, symbolArr& instances);
// char *gen_array_provide(char *ptr, int len, int& capa, int width);
static int *offtable = 0;
static int offtablesize = 0;
void offtable_reset()
{
  gen_arr_free((char *)offtable);
  offtablesize = 0;
  offtable = 0;
}

static void instance_cannot_find(char const*ln, int line, int column)
{
  char const* head = "cannot find instance ";
  if(line != NULL_LINE)
    msg("$1$2,$3:$4\n") << head << eoarg << line << eoarg << column << eoarg << ln << eom;
  else
    msg("$1$2:$3\n") << head << eoarg << column << eoarg << ln << eom;
}

static void  process_char_offset(char const *ln, instMod*pi, int ninst, symbolArr&instances, symbolArr& res)
{
  int no_inst = instances.size();
  // build HUGE table: table[i] = instance s.t instance.offset = i, NULL is not_avail

  offtable_reset();
  offtable = (int*)gen_array_provide((char *)offtable, 10*no_inst, 
				     offtablesize, sizeof(int));
  int ii;
  int max_off = 0;
  for(ii=0; ii< no_inst; ++ii){
    symbolPtr& sym = instances[ii];
    int off = sym.char_offset();
    if (off > max_off)
	max_off = off;

    int len = off + 1;
    if(offtablesize < len)
      offtable = (int*)gen_array_provide((char *)offtable, len,
					 offtablesize, sizeof(int));
    
    offtable[off] = ii+1;  // to avoid NULL ambiguity; 
  }
  for(ii=0; ii<ninst; ++ii){
    int off = pi[ii].column;
    if (off > max_off) {
      instance_cannot_find(ln, -(max_off*10), off);
      continue;
    }
	
    int ind = offtable[off];
    if(ind == 0 ){  // no exact match
       int start = off+1;
       int fin = off-4;
       if(fin <0)
	 fin = 0;

       for (int cur = start; cur >=fin; --cur){
	 int locind = offtable[cur];
	 if(locind > 0){
	   ind = locind;
	   break;
	 }
       }
    }
    if(ind > 0 )  // found
      res.insert_last(instances[ind-1]);
    else
      instance_cannot_find(ln, NULL_LINE, off);
  }
  offtable_reset();
}
genArr(instInd);
static void  process_line_column(char const *ln, instMod*pi, int ninst, symbolArr&instances, symbolArr& res)
{

  int no_inst = instances.size();
  int start_line = pi[0].line;
  int end_line = pi[ninst-1].line;
  
  int range = end_line - start_line + 1;
  int * modOffsets = new int[range];
  int kk;
  for( kk=0; kk < range; ++kk)
    modOffsets[kk] = -1;
  int * instOffsets = new int[range];
  for( kk=0; kk < range; ++kk)
    instOffsets[kk] = -1;

  int ii, cur_line;
  for(ii=0, cur_line = -1; ii<ninst; ++ii){
    int line = pi[ii].line;
    if(line != cur_line){
      cur_line = line;
      modOffsets[line - start_line] = ii;
    }
  }
  instIndArr instarr;
  for(ii=0; ii<no_inst; ++ii){
    symbolPtr& sym = instances[ii];
    int line = sym.line_number();
    int line_inc = line - start_line;
    if(line_inc >= 0 && line_inc < range && modOffsets[line_inc] != -1){
      instInd* p = instarr.grow();
      p->ind = ii;
      p->line = line;
      p->column = sym.line_offset();
    }
  }  

  int no_ind = instarr.size();
  OSapi_qsort(instarr[0], no_ind, sizeof(instInd), instInd_compare);

  for(ii=0, cur_line = -1; ii<no_ind; ++ii){
    int line = instarr[ii]->line;
    if(line != cur_line){
      cur_line = line;
      instOffsets[line - start_line] = ii;
    }
  }
  for(ii=0; ii<ninst; ++ii){
     int line = pi[ii].line;
     int line_inc = line-start_line;
     int st = instOffsets[line_inc];
     if(st == -1)
       continue;

     int column = pi[ii].column;
     for (int kk =st; kk<no_ind; ++kk){
       instInd* iip = instarr[kk];
       if(line != iip->line){
	 instance_cannot_find(ln, line, column);
	 break;
       }
       int cur_col = iip->column;
       if(cur_col >= column){
	 int ind = iip->ind;
	 symbolPtr& insym = instances[ind];
	 res.insert_last(insym);
	 break;
       }
     }
   }
  delete modOffsets;
  delete instOffsets;
}
 
static void  instances_in_module(projModule*mod, instMod*pi, int ninst, symbolArr& res)
{
  char const *ln = mod->get_name();
  if(ninst <=3 ) {
    symbolPtr sym;
    for(int ii=0; ii<ninst; ++ii){
      int line = pi[ii].line;
      int column = pi[ii].column;
      if(line == NULL_LINE)
	sym = MetricElement::get_instance(ln, column);
      else 
	sym = MetricElement::get_instance(ln, line, column);

      if(sym.isnotnull())
	res.insert_last(sym);
      else
       instance_cannot_find(ln, line, column);
    }
    return;
  }

  symbolPtr sym = mod;

  symbolArr instances;    
  module_get_instances(sym, instances);
 
  int no_inst = instances.size();
  if(no_inst==0)
    return;

  // find char offsets: line = NULL_LINE
   
  int no_charoff = 0;
  instMod* char_pi = pi;
  while(no_charoff < ninst && pi->line == NULL_LINE){
    ++no_charoff;
    ++pi;
  }
  ninst -= no_charoff;

  if(ninst > 0)
    process_line_column(ln, pi, ninst, instances, res);
  if(no_charoff > 0)
    process_char_offset(ln, char_pi, no_charoff, instances, res);
  
}

static void process_instances(instModArr& arr, symbolArr& res)
{
  int sz = arr.size();
  if (sz == 0)
      return;

  int width = sizeof(instMod);
  OSapi_qsort(arr[0], sz, width, instMod_compare);
  // for each module
  int cur_start=0;
  projModule* cur_mod = arr[0]->mod;
  for(int cur_ind = 1; ; ++cur_ind){
    if(cur_ind == sz || arr[cur_ind]->mod != cur_mod){
      instances_in_module(cur_mod, arr[cur_start], cur_ind - cur_start, res);
      if(cur_ind == sz)
	break;
      cur_start = cur_ind;
      cur_mod = arr[cur_start]->mod;
    }
  }
}


int parse_selection(char const *str, symbolArr& result)
{
    Initialize(parse_selection);
    symbolArr*save_selection = selection;
    symbolArr *save_cur_set = cur_set;
    symbolArr *save_temp = temp;
    int ignerr =   ignore_error;
    
    lexState state;
    lex_state_save(&state);  // XXX: use reentrant parser instead?  See flex.info::Multiple input buffers.
    ignore_error = 1;

    temp                = NULL;
    DI_parse_error_code = 0;
    use_whole_set       = 0;
    selection           = &result;
    cur_set = NULL;
    cur_attr = 0;
    glob_attr = 0;

    int buf_len = OSapi_strlen(str);
    if(buf_len > 1000)
    {
      module_set = new objNameSet;
      instMod_arr = new instModArr;
      api_collect_modules(*module_set);
    }
    parse_selection(1, &str);
    if (temp) delete temp;
    temp = NULL;

    lex_state_restore(&state);
    ignore_error = ignerr;
    selection = save_selection;
    cur_set = save_cur_set;
    temp = save_temp;
    cur_attr = 0;
    glob_attr = 0;

    if(instMod_arr){
      process_instances(*instMod_arr, result);
      delete instMod_arr;
      instMod_arr = NULL;
    }

    if(module_set){
      delete module_set;
      module_set = NULL;
    }

    return 0;
}

int parse_selection_with_nulls(char const *str, symbolArr& result)
{
  boolean old = allow_null_symbols;
  allow_null_symbols = true;
  int r = parse_selection(str,result);
  allow_null_symbols = old;
  return r;
}


extern "C" void output_pointer(int ptr)
{
  symbolPtr sym;
  if(symbolPtr_is_symbol(ptr)){
    xrefSymbol * xr = (xrefSymbol*) ptr;
    sym = xr;
  } else {
    Obj* obj =(Obj*) ptr;
    sym = obj;
  }
    api_add_selection(sym);
}
extern "C" void output_symbolPtr(int ptr)
{
    api_add_selection(*(symbolPtr*)ptr);
}

int xref_convert_from_num(symbolPtr & sym, int pmod_num, int offset);

extern "C" void output_xrefSymbol(int pmod, int off)
{
    symbolPtr sym;
    int res = xref_convert_from_num(sym, pmod, off);
    api_add_selection(sym);

}

int dataCell_convert_from_offset(symbolPtr &data, symbolPtr &symbol, int offset);

extern "C" void output_dataCell(int offset,
				int pmod,
				int off)
{
  symbolPtr symbol, data;
  int res = xref_convert_from_num(symbol, pmod, off);
  res = dataCell_convert_from_offset(data, symbol, offset);
  api_add_selection(data);
}

extern "C" void output_cell(int offset,
			    char const *str)
{
  symbolPtr data;
  symbolPtr symbol = api_find_file(str, 1);
  int res = dataCell_convert_from_offset(data, symbol, offset);
  api_add_selection(data);
}

extern "C" void output_error_handler(int err)
{
  ignore_error = err;
}

extern "C" void output_itag(int ind)
{
  xrefSymbol * xr = (xrefSymbol*) ind;
  symbolPtr sym = xr;
  api_add_selection(sym);
}

int strcmp_name(ddKind, char const *s1, char const *s2);
symbolPtr lookup_xrefSymbol(ddKind kind, char const* fn, char const* root);

symbolPtr api_lookup_xrefSymbol(ddKind kind, char const *name)
{
  Initialize(api_lookup_xrefSymbol);
  if(!current_scope_flag)   
    return lookup_xrefSymbol(kind, name);
  
  current_scope_flag = 0;
  ddKind skind = current_scope.get_kind();
  char const *sname = current_scope.get_name();
  symbolPtr sym;
  switch (skind) {
  case DD_PROJECT:
    sym = lookup_xrefSymbol(kind, name, sname);
    break;
  case DD_SCOPE:
    {
      scopeNode*sc = checked_cast(scopeNode,current_scope);
      sym = lookup_xrefSymbol(kind, name);
      symbolArr defs;
      sc->lookup_all(sym,defs);
      // Had to cast NULL_symbolPtr to satisfy SGI compiler
      // [03/17/97, abover]:
      sym = defs.size() ? defs[0] : (symbolPtr)NULL_symbolPtr;
      break;
    }
  case DD_MODULE:
    { 
      projNode*home = projNode::get_current_proj();
      Xref* xr = home->get_xref();
      symbolArr arr;
      sym = xr->find_symbol(arr, kind, name, sname);
      break;
    }
  }
  return sym;
}

int Interpreter::ParseArguments(int ign, int start_pos, int argc, char const *argv[], symbolArr& res)
{
    static Tcl_Obj *name = Tcl_NewStringObj("CurSet", -1);
    DI_object di_cur_set = Tcl_ObjGetVar2(interp, name, NULL, TCL_GLOBAL_ONLY);
    symbolArr *cur_set   = get_DI_symarr(di_cur_set);
    int result           = parse_selection(ign, argc - start_pos, &argv[start_pos], cur_set, res);
    if(result != 0)
	Tcl_SetResult(interp, selection_parse_error, TCL_VOLATILE);
    return result;
}
int Interpreter::ParseArguments(int start_pos, int argc, char const *argv[], symbolArr& res)
{
  return ParseArguments(ignore_error_default, start_pos, argc, argv, res);
}

static int lookup_symbolCmd(Interpreter*i, int argc, char const *argv[])
{
  if(argc != 3){
    ostream&os = i->GetOutputStream();
    os << "usage: lookup_symbol <kind> <name>" << endl;
    return (argc==1) ? TCL_OK : TCL_ERROR;
  }
  char const *kind = argv[1];
  char const *name = argv[2];
  symbolAttribute* attr = symbolAttribute::get_by_name(kind);
  if((!attr) ||(!is_kindAttribute(attr))){
    ostream&os = i->GetOutputStream();
    os << kind << ": not a kind" << endl;
    return TCL_ERROR;
  }
  ddKind k = ((kindAttribute*)attr)->kind;
  symbolPtr sym = api_lookup_xrefSymbol(k, name);
  DI_object result;
  DI_object_create(&result);
  if(sym.isnotnull()){
    symbolArr*arr =  get_DI_symarr(result);
    arr->insert_last(sym);
  }
  i->SetDIResult(result);
  return TCL_OK;
}
static int add_commands()
{
  new cliCommand("lookup_symbol", lookup_symbolCmd);
  return 0;
}
static int add_commands_dummy = add_commands();

