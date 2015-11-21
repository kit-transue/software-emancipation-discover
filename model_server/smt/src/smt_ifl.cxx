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
#include <ddict.h>
#include <msg.h>
#include <smt.h>
#include <symbolPtr.h>
#include <objArr.h>
#include <proj.h>
#include <transaction.h>

#include <astnodeStruct.h>
#include <locationStruct.h>
#include <relationStruct.h>
#include <symbolStruct.h>
#include <astnodeList.h>
#include <attributeList.h>
#include <locationList.h>
#include <uintList.h>

#include <dfa.h>
#include <dfa_db.h>

#include <smt_ifl.h>
#include <Interpreter.h>
#include <machdep.h>
#include <ctype.h>
#include "app.h"
//#include "ldr.h"
#include <db_intern.h>
#include <psetmem.h>
#include <charstream.h>
#include <driver_mode.h>

int is_template_symbol(char const *);
void proj_path_report_app(app *, projNode *prj = NULL);
static int smt_init_tokens (smtHeader* h);
static void reset_line_offset();
extern "C" int iff_parse(char const *);
astRoot* dfa_els_build(smtHeader*, astnodeStruct*);
int els_get_ast_code(astnodeStruct*);
int codeAttribute_get_code(Relational*);
void dd_set_baseclass_rel( ddElementPtr, ddElementPtr, int , ddProtection );
char const *els_include_path_ln(char const *path);
int els_find_location(astnodeStruct* a, int& start, int& len);
static int ifl_start_length(smtHeader* h, locationStruct* s, int& start_offset, int& length);

int els_parse_file(const symbolPtr&);
smtTree* smt_get_proper_scope(smtTree*);
extern int xref_hook_turn_off;
int dd_is_structure(ddKind k);
int dd_is_scoped_entity(ddKind kind);
int dd_set_entity_scope(ddElement *, ddElement *);
define_relation(auxtype_of_sym,sym_of_auxtype);
init_relation(auxtype_of_sym,MANY,NULL,sym_of_auxtype,MANY,NULL);
int local_variables_done;
void obj_delete_or_unload(Obj *root, int delete_flag);
extern "C" char const *current_smt_language;

class Handle_smtHeader : public Relational {
public:
  Handle_smtHeader();
  ~Handle_smtHeader();
};

define_relation(handle_of_header,header_of_handle);
init_relation(handle_of_header,1,NULL,header_of_handle,1,NULL);

Handle_smtHeader::Handle_smtHeader()
{
}


Handle_smtHeader::~Handle_smtHeader()
{
  smtHeader* h = (smtHeader*)get_relation(header_of_handle,this);
  if (h)
    obj_delete_or_unload(h,0);
}

static void post_to_delete(smtHeader* h)
{
  Handle_smtHeader* handle = new Handle_smtHeader();
  if (h)
    put_relation(handle_of_header,h,handle);
  obj_delete(handle);
}

#define AST_IGNORE 9999

static ddKind dd_convert_ifl_code(char const *kind, char const *name = NULL);

// This should be declared in dfa_cli.h, only there isn't any dfa_cli.h!
extern objSet * ast_codes;

objArr locals;

enum ElsAttribute {
  ELS_OTHER = 0,
  ELS_NAMELESS,
  ELS_STATIC,
  ELS_REGISTER,
  ELS_ASM,
  ELS_VIRTUAL,
  ELS_INLINE,
  ELS_OPERATOR,
  ELS_CONSTRUCTOR,
  ELS_DESTRUCTOR,
  ELS_PURE,
  ELS_MUTABLE,
  ELS_EXPLICIT,
  ELS_AUTO,
  ELS_TEMPLATE,
  ELS_PUBLIC,
  ELS_PROTECTED,
  ELS_PACKAGE_PROT,
  ELS_PRIVATE,
  ELS_USING,
  ELS_ADDRESS,
  ELS_REFERENCE,
  ELS_POINTER,
  ELS_OBJECT,
  ELS_CONST,
  ELS_VOLATILE,
  ELS_ARRAY,
  ELS_ARGUMENT,
  ELS_RETURN,
  ELS_ACTUAL,
  ELS_DECLARED,
  ELS_FULL,
  ELS_PARTIAL,
  ELS_C_PROTO,
  ELS_SIZE,
  ELS_TRANSIENT,
  ELS_PACKAGE,
  ELS_FINAL,
  ELS_SYNCHRONIZED,
  ELS_ABSTRACT,
  ELS_NATIVE,
  ELS_NUM_OF_ATTRS
  };

class ELS_att {
public:
  ELS_att (ElsAttribute, int val = 1);
  ~ELS_att();
  ElsAttribute get_attr() { return attr ;}
  int get_value() {return value;}
  static ElsAttribute find_by_name(char const *);
  static char const *els_attribute_name [ELS_NUM_OF_ATTRS];
private:
  ElsAttribute attr;
  int value;
};

ELS_att::ELS_att(ElsAttribute code, int val) : 
attr(code), value(val)
{
}

ELS_att::~ELS_att()
{
}

genArr(ELS_att);

class elsElement : public Relational {
public:
  virtual void	print(ostream& = cout, int level = 0) const;
  elsElement(char const *nm, ddKind knd);
  char const *get_name() const;
  ~elsElement();
  genString name;
  ddKind kind; // DD_LOCAL
  ddKind alt_kind;
  ddElement* dd;
  ddElement* get_dd();
  int decl_id;
  void store_attribute(ElsAttribute code, int val);
  genArrOf(ELS_att) attributes;
  define_relational(elsElement,Relational);
  int is_static() const;
};

init_relational(elsElement,Relational);

defrel_one_to_one(elsElement,els,smtTree,dec);
defrel_many_to_one(elsElement,els,ddElement,con);

init_rel_or_ptr(elsElement,els,NULL,smtTree,dec,NULL);
init_rel_or_ptr(elsElement,els,NULL,ddElement,con,NULL);

elsElement::elsElement(char const *nm, ddKind knd) 
     : name(nm), kind(knd), dd(NULL), decl_id(0)
{
  locals.insert_last(this);
}

elsElement::~elsElement()
{
  rem_rel_all();
}

char const *elsElement::get_name() const
{
  return name.str();
}

void	elsElement::print(ostream& os, int level) const
{
  smtTree *dec = els_get_dec(this);
  ddElement *con = els_get_con(this);
  os << get_name() << " dec " << dec;
  os << " con " << (con ? con->get_name() : "NULL") << endl;
}

void elsElement::store_attribute(ElsAttribute code, int val)
{
  ELS_att tmp(code,val);
  attributes.append(&tmp);
}

int elsElement::is_static() const {
  for (int jj = 0; jj < attributes.size(); ++jj) {
    ELS_att* att = attributes[jj];
    if (att->get_attr() == ELS_STATIC) {
      return 1;
    }
  }
  return 0;
}

static ddKind get_kind(Relational*el)
{
  return is_ddElement(el) ? ((ddElement*) el)->get_kind() : ((elsElement*)el)->kind;
}

static void els_move_rels(Relational * from, Relational *to)
{
  relArr& ra =  from->get_arr();
  int sz = ra.size();
  RelType* rels[100];
  int ii;
  RelType*rt;
  for(ii=0; ii<sz; ++ii){
    Relation * cr = ra[ii];
    rt = cr->get_rel_type();   
    rels[ii] = rt;
  }
  for(ii=0; ii<sz; ++ii){
    rt = rels[ii];
    Obj*members = from->get_rel(rt);
    if(members->relationalp()){
      Relational*rob = (Relational*) members;
      from->rem_rel(rt);
      to->put_rel(*rob, rt);
    } else {
      objSet set = members; // copy
      from->rem_rel(rt);      
      Obj*obj;
      ForEach(obj, set){
	Relational*rob = (Relational*) obj;
	to->put_rel(*rob, rt);
      }
    }
  }
}

static smtTree* els_get_mcall(smtTree* node)
{
  smtTree* retval = node;

  while (retval && retval->type != SMT_macrocall)
    retval = retval->get_parent();

  return retval;
}

static void els_map_local(ddElement*dd, elsElement*els)
{
  ddElement*context = els_get_con(els);
  smtTree * dec = els_get_dec(els);

  objSet tokens = els->get_rel(ref_smt_of_dd);  // copy
  ddElement*type= (ddElement*) els->get_rel(semtype_of_smt_decl);
  els->rem_rel_all();


  dec->put_rel(*dd, ref_dd_of_smt);

  if(tokens.size()){
    Obj*el;
    ForEach(el, tokens){
      smtTree*tok = (smtTree*)el;
      if(context)
	tok->put_rel(*context, user_of_used);
      if(tok->get_parent() == dec)
	{
	  dec->put_rel(*tok, id_of_decl);
	  ddElement* parm = (ddElement*)get_relation(def_dd_of_smt,dec);
	  if (parm)
	    parm->put_rel(*tok, ref_smt_of_dd);
	  else
	    dec->put_rel(*tok, reference_of_declaration);
	}
      else
	dec->put_rel(*tok, reference_of_declaration);
    }
  }

  if(type){
    dec->put_rel(*type, semtype_of_smt_decl);
    dd->put_rel(*type, semtype_of_smt_decl);
  }
}

static int set_rel_struct(Relational* tp)
{
  int retval = 0;
  objSet processed;
  objSet pend_set;

  pend_set.insert(tp);
  while(pend_set.size())
    {
      Obj* el=NULL;
      {
	ForEach(el,pend_set)
	  break;
      }
      pend_set.remove(el);
      processed.insert(el);
      Obj* next_lev = get_relation(sym_of_auxtype,(Relational*)el);
      ForEach(el,*next_lev)
	{
	  Relational * rel = (Relational*)el;
	  if(!processed.includes(el))
	    {
	      if(get_kind(rel) != DD_SEMTYPE)
		{
		  put_relation(struct_decl_of_ref,rel,tp);
		  retval++;
		}
	      pend_set.insert(rel);
	    }
	}
    }
  return retval;
}

//
// This function assumes, that h->ttablth is an 
// exact size of token table
// actually it is in most cases just capacity.
// It explains strange code around call to els_cn_na()
// in the file dfa_db.C 
//

smtTree * els_cn_na(smtHeader*h, int cn)
{
   smtTree* retval = NULL;
   
   smtTree** ttable = h->ttable;
   int l_ind =0, r_ind = h->ttablth-1;
   smtTree *left = ttable[l_ind], *right = ttable[r_ind];
   int lbeg, rbeg;
   if (cn >= (lbeg=left->tbeg) - left->spaces)
     if (cn < (rbeg=right->tbeg) + right->tlth + right->newlines)
       if (cn < lbeg + left->tlth + left->newlines)
	 retval = left;
       else if (cn >= rbeg - right->spaces)
	 retval = right;
       else
	 while(l_ind < r_ind)
	   {
	     int m_ind = (l_ind + r_ind) / 2;
	     smtTree *middle = ttable[m_ind];
	     int mbeg;
	     if(cn >= (mbeg=middle->tbeg) - middle->spaces)
	       if (cn < mbeg + middle->tlth + middle->newlines)
		 {
		   retval = middle;
		   break;
		 }
	       else
		 l_ind = m_ind;
	     else
	       r_ind = m_ind;
	   }

   return retval;
}

static int is_comment(smtTree* tok)
{
  int retval = 0;
  
  if(tok)
    {
      int tp = tok->extype;
      switch (tp)
	{
	case SMTT_el:
	  retval = 1;
	  break;

	default:
	  break;
	}
    }
  return retval;
}

static void els_build_comments(smtTree * root)
{
  Initialize(els_build_comments);

  smtTree* cur = root->get_first_leaf();
  smtTree* en = root->get_last_leaf();
  smtHeader* h = (smtHeader*)root->get_header();
  while(is_comment(en))
    en = en->get_next_leaf();

  int comm_flag = 0;

  smtTree*com1,*com2;

  for(;1; cur = cur->get_next_leaf()) {
    if(is_comment(cur)){
       if(comm_flag){
          com2 = cur;
       } else {
          comm_flag = 1;
          com1 = com2 = cur;
       }
    } else  if(comm_flag){
          int tn1 = com1->tnfirst;
          int tn2 = com2->tnfirst;
          smt_def_stmt(h, 0, SMT_comment, tn1, tn2);
          comm_flag = 0;
    }
    if(cur==en) break;
  } // for
}

static astnodeStruct* ifl_get_child(astnodeStruct *a,int n)
{
  astnodeStruct * retval = NULL;

  if (a && a->children && a->children->size()>=n)
    retval = (*(a->children))[n-1];
  
  return retval;
}

static ddKind dd_convert_ifl_code(char const *kind, char const *name)
{
  ddKind retval = DD_BUGS;

  if (kind[0] == 'f' && !strcmp(kind, "file"))
    retval = DD_MODULE;
  else
    {
      kindAttribute* ka = kindAttribute::lookup(kind);
      
      if (ka)
	{
	  retval = ka->kind;
	  if (retval == DD_VAR_DECL && name && strstr(name, " @ "))
	    retval = DD_LOCAL;
	}
      else if (!strcmp (kind,"type"))
	retval = DD_SEMTYPE;
      else if (!strcmp (kind,"checksum"))
	retval = DD_CHECKSUM;
      else
	msg("UNKNOWN symbol: $1", error_sev) << kind << eom;
    }
  return retval;
}

static objArr symbols;
static smtHeader* smt_header_static;
static ddRoot* dd_root_static;
static projModule* proj_module_static;

inline Relational* symbols_get_symbol(int id)
{
  Relational *ret = NULL;
  if (id >= 0 && id < symbols.size())
    ret = (Relational*)symbols[id];
  if (!ret)
    msg("Reference to undefined SYM [$1]", warning_sev) << id << eom;
  return ret;
}

static int set_rel_struct()
{
  Initialize(set_rel_struct(void));

  int retval = 0;
  int sz = symbols.size();
  for(int ii = 1; ii < sz; ++ii)
    {
      Relational* sym = (Relational*)symbols[ii];
      if (sym && get_kind(sym) == DD_CLASS)
	retval += set_rel_struct(sym);
    }

  return retval;
}

static int els_init_module(projModule*mod)
{
  Initialize(els_init_module(projModule*));

  char const *fn = mod->get_phys_filename();
  projNode *proj = mod->get_project();

  char const *ln = mod->get_name();
  
  app* app = mod->get_app();
  smtHeader*smt_header = NULL;

  if (!app && !is_model_build())
    app = mod->restore_module();

  if (app && is_smtHeader(app))  		
    smt_header = (smtHeader*)app;

  if (!smt_header)
    {
      fileLanguage lng = mod->language();
      smt_header =  new smtHeader(ln, fn, lng, &proj);
    }
  else
    {
      smtHeader* junk = new smtHeader("", (fileLanguage)smt_header->language, (smtHeader*)0);
      smtTree* old_root = checked_cast(smtTree,smt_header->get_root());
      smtTree* r = new smtTree(SMT_file);
      smt_header->put_root(r);
      junk->put_root(old_root);
      ddRoot* old_dd_root = checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader, smt_header));
      if (old_dd_root) 
	{
	  smt_header->rem_rel(ddRoot_of_smtHeader);
	  put_relation(ddRoot_of_smtHeader, junk, old_dd_root);
	}
      post_to_delete(junk);
      if (smt_header->ttable)
	psetfree(smt_header->ttable);
      smt_header->ttable = 0;
      smt_header->ttablth = 0;
      if(smt_header->srcbuf)
	psetfree(smt_header->srcbuf);
      smt_header->srcbuf = NULL;
      smt_header->src_size = smt_header->src_asize = 0;
      smt_header->parsed = smt_header->src_size = smt_header->src_asize = 0;
      smt_header->set_ast_exist(0);
      smt_header->set_arn(1);
      smt_header->vrn = smt_header->sam = smt_header->scope = smt_header->tok = 0;
      smt_header->drn = 1;
      smt_header->globals = 0;
    }

  int load_succeeded = (smt_header->load () >= 0);
  if (!load_succeeded) {
    return 0;
  } else {    
    smt_init_tokens(smt_header);
  
    smt_header_static = smt_header;
    reset_line_offset();
    dd_root_static = dd_sh_get_dr(smt_header_static);
    smt_header_static->vrn = 1;
    dd_root_static->init_dds();
    proj_module_static = mod; 
    return 1;
  }
}

int smt_ifl_init (projModule*mod)
{
  xref_hook_turn_off = 1;
  symbols.removeAll();
  locals.removeAll();

  int err = els_init_module(mod);
  
  return err;
}

int smt_ifl_end()
{
  if (dd_root_static)
    ifl_ast_report_tree(NULL);

  int sz = locals.size();
  for(int jj=0; jj<sz; ++jj){
    elsElement*els = (elsElement*) locals[jj];
    delete els;
  }
  locals.removeAll();

  symbols.removeAll();
  xref_hook_turn_off = 0;
  return 0;
} 

char *trim_string_for_pmod(char const *str, int &length, int &lstart);

static int els_fix_swt_status(ddElement *dd)
{
  Initialize(els_fix_swt_status(ddElement*));
  int retval = 0;

  if (dd && dd->get_sw_tracking() == SWT_NEW) {
    if (dd->get_is_def() && dd->get_kind() == DD_MODULE) {
      dd->filler = SWT_UNCHANGE;
      retval = 1;
    } else {
      symbolPtr sym = dd->find_xrefSymbol();
      unsigned int swt_status = SWT_NEW;
      if (sym.xrisnotnull() && (swt_status=sym->get_attribute(SWT_Entity_Status , 2)) != SWT_NEW) {
	dd->filler = swt_status;
	retval = 1;
      }
    }
  }

  return retval;
}

static ddElement *get_current_dd_language() {
    if (current_smt_language == NULL) {
	return NULL;
    }
    int ll = strlen(current_smt_language);
    ddElement *current_dd_language
	= dd_root_static->add_def(DD_LANGUAGE, ll, current_smt_language,
				  0, "", 0, "");
    return current_dd_language;
}

int ifl_sym_insert(unsigned int identifier, symbolStruct * s)
{
  int retval = 0;

  while(symbols.size() <= identifier)
    symbols.insert_last(NULL);

  ddKind knd = dd_convert_ifl_code(s->kind, s->name);
  if (knd)
    {
      if(knd == DD_LOCAL){
        elsElement* els = new elsElement(s->name, knd);
	symbols[identifier] = els;
	els->alt_kind = dd_convert_ifl_code(s->kind);
      } else if (knd == DD_MODULE)
	{ 
	  char const *ns = (char const *)els_include_path_ln(s->name);
	  if (!ns)
	    ns = s->name;
	  genString ns2;
	  if (ns[0] != '\0' && ns[1] == ':' && (ns[2] == '/' || ns[2] == '\\')) {
	    // Avoid colon in module's logical name, for the sake of Developer Xpress.
	    ns2 += '/';
	    ns2 += tolower(ns[0]);
	    ns2 += "_drive";
	    ns2 += (ns + 2);
	    ns = ns2;
	  }
	  int nl = strlen(ns);
	  ddElement * dd = dd_root_static->lookup(DD_MODULE, ns);
	  if (!dd)
	    dd = dd_root_static->add_def(DD_MODULE, nl, ns, 0, "", 0, "");
	  dd->language = smt_header_static->language;
	  if (current_smt_language != NULL) {
	    put_relation(property_of_possessor, dd, get_current_dd_language());
	  }
	  symbols[identifier] = dd;
	  dd->get_xrefSymbol();
	  els_fix_swt_status(dd);
	}
      else
	{
	char const *ns = s->name;
	int nl   = strlen(ns);
	ddElement *dd;
	if(knd == DD_STRING) {
	    int lstart = 0;
	    ns         = trim_string_for_pmod(ns, nl, lstart);	    
	    dd         = dd_root_static->add_def(knd, nl, &ns[lstart], 0, "", 0, "");
	} else
	    dd = dd_root_static->add_def(knd, nl, ns, 0, "", 0, "");
	dd->language = smt_header_static->language;
	if (s->loc)
	  {
	    dd->set_is_def();
	    dd->set_def_file(smt_header_static->get_filename());
    	    if (s->loc->start_line > 0) {
	        dd->xref_index = s->loc->start_line;
	    }
	    els_fix_swt_status(dd);
	  }
	symbols[identifier] = dd;
	if(s->generated)
	  dd->set_comp_gen();
      }
    }
  retval = 1;
  return retval;
}

char const *ELS_att::els_attribute_name [ELS_NUM_OF_ATTRS] = {
  "other",
  "nameless",
  "static",
  "register",
  "asm",
  "virtual",
  "inline",
  "operator",
  "constructor",
  "destructor",
  "pure",
  "mutable",
  "explicit",
  "auto",
  "template",
  "public",
  "protected",
  "package",
  "private",
  "using",
  "address",
  "reference",
  "pointer",
  "object",
  "const",
  "volatile",
  "array",
  "argument",
  "return",
  "actual",
  "declared",
  "full",
  "partial",
  "c_proto",
  "size",
  "transient",
  "package",
  "final",
  "synchronized",
  "abstract",
  "native"
};

ElsAttribute ELS_att::find_by_name(char const *name)
{
  ElsAttribute retval = ELS_OTHER;

  for (int ii = 1; ii < ELS_NUM_OF_ATTRS; ++ii)
    {
      char const *nm = els_attribute_name[ii];
      if (name[0] == nm[0] && !strcmp(name, nm))
	{
	  retval = (ElsAttribute)ii;
	  break;
	}
    }
      
  return retval;
}

int els_update_attribute(ddElement* dd, ElsAttribute code, int val)
{
  switch (code)
    {
    case ELS_NAMELESS:
      break;

    case ELS_STATIC:
      if(val)
	dd->set_static();
      break;

    case ELS_REGISTER:
      break;

    case ELS_ASM:
      break;

    case ELS_VIRTUAL:
      if(val)
	dd->set_virtual();
      break;
      
    case ELS_INLINE:
      if(val)
	dd->set_inline();
      break;

    case ELS_OPERATOR:
      break;

    case ELS_CONSTRUCTOR:
      if(val)
	dd->set_cd();
      break;
      
    case ELS_DESTRUCTOR:
      if(val)
	dd->set_cd();
      break;
      
    case ELS_ABSTRACT:
    case ELS_PURE:
      if(val)
	dd->set_pure_virtual();
      break;

    case ELS_MUTABLE:
      break;
      
    case ELS_EXPLICIT:
      break;

    case ELS_AUTO:
      break;

    case ELS_TEMPLATE:
      break;
      
    case ELS_PUBLIC:
      if(val)
	dd->set_public();
      break;

    case ELS_PROTECTED:
      if(val)
	dd->set_protected();
      break;

    case ELS_PACKAGE_PROT:
      if(val)
	dd->set_package_prot();
      break;

    case ELS_PRIVATE:
      if(val)
	dd->set_private();
      break;
      
    case ELS_USING:
      break;
      
    case ELS_ADDRESS:
      break;
      
    case ELS_REFERENCE:
      if (val)
	dd->set_struct_rel_type(REFERENCE_TO);
      break;
      
    case ELS_POINTER:
      if (val)
	dd->set_struct_rel_type(POINTER_TO);
      break;
      
    case ELS_OBJECT:
      break;
      
    case ELS_FINAL:
    case ELS_CONST:
      if (val)
	dd->set_const();
      break;
      
    case ELS_VOLATILE:
      if(val)
	dd->set_volatile();
      break;
      
    case ELS_ARRAY:
      dd->set_struct_rel_type(POINTER_TO);
      break;
      
    case ELS_ARGUMENT:
      break;
      
    case ELS_RETURN:
      break;

    case ELS_ACTUAL:
      break;
      
    case ELS_DECLARED:
      break;
      
    case ELS_FULL:
      break;
      
    case ELS_PARTIAL:
      break;

    case ELS_C_PROTO:
      break;
      
    case ELS_NATIVE:
      if (val)
	dd->set_native();
      break;
      
    case ELS_SYNCHRONIZED:
      if (val)
	dd->set_synchronized();
      break;

    default:
      break;
  }

  return 1;
}

int els_process_attribute(Relational* sym, ElsAttribute code, char const *value)
{
  if (code == ELS_C_PROTO)
    {
      if(is_ddElement(sym))
	{
	  ddElement * ep = (ddElement*)sym;
	  if (value)
	    ep->set_def(value);
	}
    }
  else 
    {
      int val = 1;
      if (value)
	val = OSapi_atoi(value);
      if (is_elsElement(sym))
	((elsElement*)sym)->store_attribute(code, val);
      else
	els_update_attribute((ddElement*)sym, code, val);
    }

  return 1;
}

int ifl_atr_modify(unsigned int i, attributeList *l)
{
  Relational* sym = symbols_get_symbol(i);

  if ( sym && l )
    for ( int idx = 0; idx < l->size(); idx++ )
      if ( (*l)[idx]->name )
	{
	  ElsAttribute code = ELS_att::find_by_name((*l)[idx]->name);
	  if (code)
	    els_process_attribute(sym, code,(*l)[idx]->value);
	}
  
  return 1;
}

static char const *ifl_rel_names[IF_REL_NUM_RELATIONS] = {
  "other",
  "context",
  "call",
  "read",
  "write",
  "type",
  "argument",
  "enumerate",
  "subclass",
  "friend",
  "catch",
  "throw",
  "specialize",
  "include",
  "define",
  "expand",
  "import",
  "checksum"
};

static IF_rel ifl_convert_rel (char const *name)
{
  IF_rel retval = IF_REL_OTHER;
  if (name)
    for (int ii = 1; ii < IF_REL_NUM_RELATIONS; ++ii)
      {
	char const *nm = ifl_rel_names[ii];
	if (name[0] == nm[0] && !strcmp(name, nm))
	  {
	    retval = (IF_rel)ii;
	    break;
	  }
      }

  return retval;
}

static int dd_process_ifl_rel(Relational* src, Relational* trg, IF_rel if_rel, attributeList* attributes)
{
  RelType * rel = NULL;
  int do_attributes = 0;
  ddKind k;

#if 0
  int src_is_def = is_ddElement(src) ? ((ddElement*)src)->get_is_def() : 1;
#endif

  switch (if_rel)
    {
    case IF_REL_CONTEXT:
      rel = used_of_user;
      if (is_ddElement(trg)
	  && ((k = get_kind(src)) == DD_CLASS || k == DD_INTERFACE))
	{
	  ((ddElement*)trg)->set_from_method();
          /*Do not ignore "context" relation if src is not definition*/  
#if 0
	  if (!src_is_def)
	    if (is_ddElement(src))
	      {
		ddElement* src_dd = (ddElement*)src;
		char const *name = src_dd->get_name();
		if (!is_template_symbol(name))
		  rel = NULL;
	      }
#endif
	  if (rel)
	    {
	      if (is_ddElement(src)) {
		ddElement* src_dd = (ddElement*)src;
		ddElement* trg_dd = (ddElement*)trg;
		if (trg_dd->get_kind() == DD_FIELD)
		  {
		    objTree* last = src_dd->get_last();
		    if(last)
		      last->put_after(trg_dd);
		    else
		      src_dd->put_first(trg_dd);
		  }
	      }
	  }
	}
      if (attributes)
	do_attributes = 1;

      if (is_ddElement(src) && is_ddElement(trg))
	{
	  ddElement* src_dd = (ddElement*)src;
	  ddElement* trg_dd = (ddElement*)trg;
	  if ((dd_is_structure(src_dd->get_kind())
               || src_dd->get_kind() == DD_PACKAGE) &&
	      dd_is_scoped_entity(trg_dd->get_kind()))
	    dd_set_entity_scope(src_dd, trg_dd);
	}
      break;
      
    case IF_REL_CALL:
      rel = used_of_user;
      break;
      
    case IF_REL_TYPE:
      put_relation(sym_of_auxtype, src, trg);
      if(get_kind(trg) == DD_FUNC_DECL) {
	if (attributes)
	  for ( int idx = 0; idx < attributes->size(); idx++ )
	    if ((*attributes)[idx]->name &&
		ELS_att::find_by_name((*attributes)[idx]->name) == ELS_RETURN) {
	      rel = smt_decl_of_semtype;
	      break;
	    }
      } else
	rel = smt_decl_of_semtype;
      do_attributes = 1;
      break;
      
    case IF_REL_ARGUMENT:
      rel = user_of_used;
      break;

    case IF_REL_ENUMERATE:
#if 0
      if (src_is_def)
#endif
	rel = used_of_user;
      break;

    case IF_REL_SUBCLASS:
      {
	ddProtection prot = DD_PUBLIC;
	int is_virtual = 0;
	if (attributes)
	  for ( int idx = 0; idx < attributes->size(); idx++ )
	    if ( (*attributes)[idx]->name )
	      {
		ElsAttribute code = ELS_att::find_by_name((*attributes)[idx]->name);
		switch (code)
		  {
		  case ELS_PUBLIC:
		    prot = DD_PUBLIC;
		    break;
		    
		  case ELS_PROTECTED:
		    prot = DD_PROTECTED;
		    break;
		    
		  case ELS_PACKAGE_PROT:
		    prot = DD_PACKAGE_PROT;
		    break;
		    
		  case ELS_PRIVATE:
		    prot = DD_PRIVATE;
		    break;
		    
		  case ELS_VIRTUAL:
		    is_virtual = 1;
		    break;
		    
		  default:
		    break;
		  }
	      }
	dd_set_baseclass_rel((ddElement*)src,(ddElement*)trg,is_virtual,prot);
	rel = superclassof;
      }
      break;
      
    case IF_REL_FRIEND:
      rel = friend_of;
      break;
      
    case IF_REL_CATCH:
      rel = used_of_user;
      break;

    case IF_REL_THROW:
      rel = used_of_user;
      break;

    case IF_REL_SPECIALIZE:
      rel = template_of_instance;
      break;

    case IF_REL_INCLUDE:
      rel = used_of_user;
      break;

    case IF_REL_DEFINE:
      if (is_ddElement(trg) && 
	  (get_kind(trg) == DD_MACRO || get_kind(trg) == DD_TYPEDEF
				     || get_kind(trg) == DD_ENUM
				     || get_kind(trg) == DD_ENUM_VAL
                                     || get_kind(trg) == DD_VAR_DECL
                                     || get_kind(trg) == DD_FUNC_DECL
                                     || get_kind(trg) == DD_CLASS) &&
	  is_ddElement(src) && get_kind(src) == DD_MODULE) {
	ddElement* macro = (ddElement*)trg;
	ddElement* file = (ddElement*)src;
	char const *filename = file->get_name();
	if (filename)
	  macro->set_def_file_with_good_name (filename);
      }
      break;

    case IF_REL_EXPAND:
      rel = has_friend;
      break;
      
    case IF_REL_CHECKSUM:
      rel = possessor_of_property;
      break;

    default:
      break;
    }

  if (attributes && do_attributes)
    for ( int idx = 0; idx < attributes->size(); idx++ )
      if ( (*attributes)[idx]->name )
	{
	  ElsAttribute code = ELS_att::find_by_name((*attributes)[idx]->name);
	  if (code)
	    {
	      if (code == ELS_SIZE && rel == smt_decl_of_semtype)
		{
		  char const *val = (*attributes)[idx]->value;
		  if(val && is_ddElement(src))
		    {
		      ddElement * src_dd = (ddElement*)src;
		      genString full_name;
		      full_name.printf("%s(%s)",src_dd->get_name(),val);
		      ddElement* arr_tp = dd_root_static->add_def(DD_SEMTYPE,full_name.length(),(char const *)full_name,0,"",0,"");
		      arr_tp->language = smt_header_static->language;
		      src = arr_tp;
		    }
		}
	      els_process_attribute(trg, code,(*attributes)[idx]->value);
	    }
	}

  if (rel && (src != trg || rel != smt_decl_of_semtype))
    put_relation(rel, src, trg);

  return 1;
}

static Relational* els_add_size(Relational* src, attributeList* attributes)
{
  Relational * retval = src;

  for ( int idx = 0; idx < attributes->size(); idx++ )
    if ((*attributes)[idx]->name && !strcmp((*attributes)[idx]->name,"size"))
      {
	char const *val = (*attributes)[idx]->value;
	if(val && is_ddElement(src))
	  {
	    ddElement * src_dd = (ddElement*)src;
	    genString full_name;
	    full_name.printf("%s(%s)",src_dd->get_name(),val);
	    retval = dd_root_static->add_def(DD_SEMTYPE,full_name.length(),(char const *)full_name,0,"",0,"");
	    ((ddElement*)retval)->language = smt_header_static->language;
	    break;
	  }
      }
  
  return retval;
}

int ifl_rel_create(relationStruct *s)
{
  if ( s )
    {
      IF_rel if_rel = ifl_convert_rel (s->name);
      if ( s->source && s->target)
	for ( int i = 0; i < s->source->size(); i++ )
	  for ( int j = 0; j < s->target->size(); j++ )
	    {
	      Relational * src = symbols_get_symbol((*(s->source))[i]);
	      Relational * trg = symbols_get_symbol((*(s->target))[j]);
	      if (src && trg) 
		dd_process_ifl_rel(src, trg, if_rel, s->attributes);
	    }
    }

  return 1;
}

void XREF_insert_module(app* ah);

int  smt_new_create_ttable(smtHeader*);

static int smt_init_tokens (smtHeader* h)
{
  h->last_token = NULL;
  smtTree *root = (smtTree*)h->get_root();
  smtTree* first = root->get_first();
  if(first)
    {
      h->last_token = first;
      first->extype = SMTT_untok;
      first->tbeg = 0;
      first->tlth = h->src_size;
    }
  return 1;
}

smtTree* smt_find_insertion_point (smtHeader *h, int offset, int length)
{
  if (!h->last_token)
    {
      smtTree* root = (smtTree*)h->get_root();
      h->last_token = root->get_first_leaf();
    }

  smtTree* cur = h->last_token;
  int direction = 1; // forward
  if (cur->tbeg > offset)
    direction = -1; // backward
  while (cur)
    {
      if (direction == 1)
	if (cur->tbeg + cur->tlth > offset)
	  break;
	else
	  cur = cur->get_next_leaf();
      else
	if (cur->tbeg <= offset)
	  break;
	else
	  cur = cur->get_prev_leaf();
    }

  if (cur)
    h->last_token = cur;

  if (cur && cur->tbeg +cur->tlth < offset + length)
    {
      cur = NULL;
    }
  return cur;
}

static genArr(int) line_offset;
static int line_offset_initialized = 0;

static void reset_line_offset()
{
  line_offset.reset();
  line_offset_initialized = 0;
}

static int els_valid_position(int line, int column)
{
  int retval = 1;
  if (line > line_offset.size())
    {
      retval = 0;
      char const *ph_name = smt_header_static->get_phys_name();
      msg("Line $1 out of range for $2", warning_sev)
	<< line << eoarg
	<< ph_name << eom; 
    }
  else if (column && line < line_offset.size() && column + *line_offset[line-1] > *line_offset[line])
    {
      retval = 0;
      char const *ph_name = smt_header_static->get_phys_name();
      msg("Position $1/$2 out of line for $3", warning_sev)
	<< line << eoarg
	<< column << eoarg
	<< ph_name << eom;
    }
  else if (line == line_offset.size())
    if (column + *line_offset[line-1] >= smt_header_static->src_size)
      {
	retval = 0;
	char const *ph_name = smt_header_static->get_phys_name();
	msg("Position $1/$2 out of line for $3", warning_sev)
		<< line << eoarg
		<< column << eoarg
		<< ph_name << eom;
      }

  return retval;
}

static void els_print_location(locationStruct *s, genString &loc)
{
  genString startpos, endpos;

  if (s->start_line)
    startpos.printf("%d/%d", s->start_line, s->start_column);
  else
    startpos.printf("%d", s->start_column);
  
  if (s->end_line)
    endpos.printf("%d/%d", s->end_line, s->end_column);
  else if (s->end_column)
    endpos.printf("%d", s->end_column);

  loc.printf("%s %s", startpos.str(), endpos.str());
}  

static int ifl_start_length(smtHeader* h, locationStruct* s, int& start_offset, int& length)
{
  if (!line_offset_initialized)
    {
      int offset = -1;
      char const *src = h->srcbuf;
      int size = h->src_size;
      line_offset.append(&offset);
      for(int ii = 0; ii < size; ++ii)
	{
	  if (src[ii] == '\n')
	    line_offset.append(&ii);
	}
      if (src[size-1] != '\n')
	line_offset.append(&size);
      line_offset_initialized = 1;
    }

  if (s->start_line) 
    if (els_valid_position(s->start_line,s->start_column))
      start_offset = *line_offset[s->start_line - 1] + s->start_column;
    else
      start_offset = -1;
  else if (s->start_column)
    start_offset = s->start_column - 1;
  
  if (start_offset >= 0)
    if (s->end_line)
      if (s->end_column)
	{
	  if (els_valid_position(s->end_line,s->end_column))
	    {
	      int end_offset =  *line_offset[s->end_line - 1] + s->end_column;
	      length = end_offset + 1 - start_offset;
	    }
	}
      else
	{
	  if (els_valid_position(s->end_line + 1,0))
	    {
	      int end_offset =  *line_offset[s->end_line];
	      length =  end_offset - start_offset;
	    }
	}
    else if (s->end_column)
      length = s->end_column;
  
  if (start_offset >= h->src_size || start_offset + length > h->src_size)
    {
      char const *ph_name = h->get_phys_name();
      genString loc;
      els_print_location(s, loc);
      msg("Wrong location for $1: $2", warning_sev)
	<< ph_name << eoarg
	<< loc.str() << eom;
      start_offset = -1;
    }
	
  return start_offset != -1;
}

int els_find_location(astnodeStruct* a, int& start, int& len)
{
  int retval = 0;

  start = -1;
  len = -1;

  if (a->location)
    {
      locationStruct * loc = (*a->location)[0];
      retval = ifl_start_length(smt_header_static, loc, start, len);
    }

  if (start >= 0 && len <=0) 
    len = 0;
  
  return retval;
}

static smtTree* smt_new_def_token(smtHeader* h, locationStruct* s, 
	int extype, char const *name = NULL)
{
  smtTree* retval = NULL;
  int start_offset = -1;
  int length = -1;
  
  ifl_start_length(h, s, start_offset, length);
  if (length <= 0)
    if(name)
      length = strlen (name);
      
  if (start_offset != -1)
    {
      smtTree* untok = smt_find_insertion_point(h, start_offset, length);
      if (untok)
	{
	  int end = untok->tbeg + untok->tlth;
	  if (untok->tbeg == start_offset)
	    {
	      retval = untok;
	    }
	  else
	    {
	      untok->tlth = start_offset - untok->tbeg;
	      retval = new smtTree(SMT_token);
	      retval->tbeg = start_offset;
	      untok->put_after(retval);
	    }

	  retval->tlth = length;
	  retval->extype = extype;

	  if (start_offset + length < end)
	    {
	      smtTree *rest = new smtTree(SMT_token);
	      rest->extype = SMTT_untok;
	      rest->tbeg = start_offset + length;
	      rest->tlth = end - rest->tbeg;
	      retval->put_after(rest);
	    }
	}
    }
  
  return retval;
}

char const *ATT_cname( symbolPtr& sym);

// A value of -1 may be passed in for (*is_local_static),
// in which case dd_ifl_get_cname probes for itself.
static char const *dd_ifl_get_cname (Relational* dd, int is_local_static)
{
  char const *name = dd->get_name();
  char const *cname =  name;
  symbolPtr sym = dd;
  static genString buffer;
  char const *delim = strstr(name," @ ");
  if (delim)
    {
      if (is_local_static < 0) {
	if (is_elsElement(dd)) {
	    elsElement *elsp = (elsElement *)dd;
	    is_local_static = elsp->is_static();
	  }
	else {
	  is_local_static = 0;
	  }
	}
      if (is_local_static) {
	// Preserve the entire name, but eliminate pesky {}, transforming
	// curly braces to parentheses.
	char *buf = new char[strlen(name) + 1];
	strcpy(buf, name);
	for (char *p = buf; *p != '\0'; p += 1) {
	  if (*p == '{') {
	    *p = '(';
	  }
	  else if (*p == '}') {
	    *p = ')';
	  }
	}
	buffer.put_value(buf, strlen(name));
	delete [] buf;
      }
      else {
	buffer.put_value(name, delim - name);
      }
      cname = buffer;
    }
  else if (is_ddElement(dd))
    {
      switch (((ddElement*)dd)->language)
	{
	case smt_CPLUSPLUS:
	case smt_C:
	  cname = ATT_cname(sym);
	  break;
	  
	case FILE_LANGUAGE_ELS:
	  for(cname = name + strlen (name); cname > name; --cname )
	    if (cname[-1] == '.')
	      break;
	  
	default:
	  break;
	}
    }

  return cname;
}

static int kind_val(ddKind knd)
{
  int retval = 1;
  
  switch (knd)
    {
    case DD_MACRO:
      retval = 2;
      break;
      
    case DD_NUMBER:
      retval = 0;
      break;

    default:
      break;
    }

  return retval;
}

static int compare_kinds (ddKind kind1, ddKind kind2)
{
  int val1 = kind_val(kind1);
  int val2 = kind_val(kind2);
  return val2 - val1;
}

int ifl_smt_reference(unsigned int i, locationList *l)
{
  Relational * dd = symbols_get_symbol(i);
  if (dd)
    if ( l )
      {
	for ( int i = 0; i < l->size(); i++ )
	  {
	    locationStruct *s = (*l)[i];
	    if (s)
	      {
		int extype = SMTT_ident;
		if (get_kind(dd) == DD_STRING)
		  extype = SMTT_string;
		char const *cname = dd_ifl_get_cname (dd, -1);
		smtTree *ref = smt_new_def_token (smt_header_static, s, extype, cname);
		if (ref)
		  {
		    Relational* old_dd = (Relational*)get_relation(ref_dd_of_smt,ref);
		    int do_it = 0;
		    if(!old_dd)
		      do_it = 1;
		    else if (old_dd != dd)
		      {
			ddKind knd = get_kind(dd);
			ddKind old_knd = get_kind(old_dd);
			int cmp_knds = compare_kinds(old_knd,knd);
			if (cmp_knds > 0)
			  do_it = 1;
		      }
		    
		    if (do_it)
		      put_relation(ref_smt_of_dd, dd, ref);
		  }
	      }
	  }
      }
  else
    {
      msg("ifl_smt_reference : can not find SYM [$1]", warning_sev) << i << eom;
    }
  
  return 1;
}

int  smt_new_create_ttable(smtHeader* h)
{
  smtTree* root = (smtTree*)h->get_root();
  int toknum = 0;

  for (smtTree* tok = root->get_first_leaf(); tok; tok = tok->get_next_leaf())
    {
      ++toknum;
      tok->tnfirst = tok->tnmax = toknum;
    }

  h->parsed = 1;
  h->tidy_tree();
  els_build_comments(root);
  h->last_token = NULL;
  h->set_modified();
  h->ttablth = toknum;
  return toknum;
}

int ifl_smt_keyword(char const *s, locationList *l)
{
  if ( l )
    for ( int i = 0; i < l->size(); i++ )
      {
	locationStruct * loc = (*l)[i];
	smtTree *kwd = smt_new_def_token (smt_header_static, loc, SMTT_kwd, s);
      }

  return 1;
}

static int ifl_complete_tokens(smtHeader* h, astnodeList* an);
static int compress_tokens(smtHeader*);
static int dfa_get_ast_code(astnodeStruct *a);
static int ifl_first_last(smtHeader* h, astnodeStruct* a,int& tfirst, int& tlast)
{
  tfirst = tlast = 0;
  smtTree *t1 = NULL, *t2 = NULL;
  if (a->location)
    {
      locationStruct * loc = (*a->location)[0];
      int start = -1, length = -1;
      ifl_start_length(h, loc, start, length);
      if (start >= 0)
	{
	  t1 = els_cn_na(h,start);
	  if (length > 0)
	    t2 = els_cn_na(h,start + length - 1);
	}
    }

  if (t1)
    tfirst = t1->tnfirst;
  if (t2)
    tlast = t2->tnmax;
  else if(t1)
    tlast = t1->tnmax;

  return 1;
}

static int ifl_start_end(smtHeader* h, astnodeStruct* a,int& start, int& end)
{
 if (a->location)
    {
      locationStruct * loc = (*a->location)[0];
      int st = -1, len = -1;
      ifl_start_length(h, loc, st, len);
      if (st >= 0 && (st < start || start == -1))
	start = st;
      if (len > 0 && st + len -1 > end)
	end = st+len-1;
    }

 if (a->children)
   for (int i = 0; i < a->children->size(); i++)
     {
       astnodeStruct* ch = (*(a->children))[i];
       ifl_start_end(h, ch, start, end);
     }

 return 1;
}

static int ifl_first_last_deep(smtHeader* h, astnodeStruct* a,int& tfirst, int& tlast)
{
  int start = -1, end = -1;
  ifl_start_end(h,a,start,end);

  tfirst = tlast = 0;
  smtTree *t1 = NULL, *t2 = NULL;
  if (start >= 0)
    {
      t1 = els_cn_na(h,start);
      if (end > 0)
	t2 = els_cn_na(h,end);
    }


  if (t1)
    tfirst = t1->tnfirst;
  if (t2)
    tlast = t2->tnmax;
  else if(t1)
    tlast = t1->tnmax;

  return 1;
}

typedef astnodeStruct* astnodePtr;
genArr(astnodePtr);

class astStack : public genArrOf(astnodePtr) {
public:
  astnodeStruct* get_value(int level);
};

astnodeStruct* astStack::get_value(int level)
{
  unsigned int sz = size();
  int ind = sz - level;
  astnodePtr* p = operator [] (ind);
  astnodeStruct* retval = p ? *p : NULL;
  return retval;
}

static astStack ast_stack_array;

#define MAP_DOWN(a) \
      do {\
        if ( a->children ) {\
          ast_stack_array.append(&a); \
          for ( i = 0; i < a->children->size(); i++ ) \
	    smt_map_ifl((*(a->children))[i],a,next_context); \
          ast_stack_array.pop(); \
	} \
      } while (0)
			  
static astnodeStruct* els_get_id_node(astnodeStruct* a)
{
  astnodeStruct* retval = NULL;

  int code = els_get_ast_code(a);
  int which = -1;
  
  switch(code)
    {	  
    case MC_VAR_DECL:
    case MC_FUNCTION_DECL:
    case MC_FIELD_DECL:
    case MC_TYPE_DECL:
    case MC_CONST_DECL:
    case DFA_CURSOR_DECL:
      which = 1;
      break;
      
    case MC_PARM_DECL:
      which = 2;
      break;
      
    default:
      break;
    }

  if (which > 0)
      retval = ifl_get_child(a,which);

  return retval;
}

int els_get_id_location(astnodeStruct* a)
{
  int start = -1, len = -1;

  astnodeStruct* ch = els_get_id_node(a);

  if (ch)
    {
      els_find_location(ch,start,len);
    }

  if (start < 0)
    start = 0;

  return start;
}

int els_get_identifier(astnodeStruct* a)
{
  int retval = a->identifier;

  if (!retval)
    {
      astnodeStruct* ch = els_get_id_node(a);
      if (ch)
	retval = ch->identifier;
    }

  return retval;
}

void els_handle_decl(smtTree*smt,  astnodeStruct* a, Relational* con, int is_parm = 0)
{
  if(!smt)
    return;
  int id = -1;
  int which = -1;
  if(smt->type == SMT_pdecl)
    which = 2;
  else 
    which = 1;
  astnodeStruct* ch = ifl_get_child(a,which);
  Relational *dd = NULL;
  if (ch){
    id = ch->identifier;
    if(id > 0)
      dd = symbols_get_symbol(id);
  }
  if(!dd)
    return;

  if(is_ddElement(dd)){
  } else { // DD_LOCAL or static
    elsElement*els = (elsElement*) dd;
    ddElement* context = ((ddElement*)con);
    els_put_dec(els,smt);
    els_put_con(els,context);
    if (is_parm && context)
      {
	char const *name = dd_ifl_get_cname(els, 0);
	int namelen = name ? strlen(name) : 0;
	genString tmp_name(name);
	ddElement* last = (ddElement*)context->get_last();
	ddElement* parm = dd_root_static->add_field(context, last,
						    DD_PARAM_DECL, 0,
						    namelen, (char const *)tmp_name,
						    0, "",
						    namelen, (char const *)tmp_name);
	put_relation(def_smt_of_dd, parm, smt);
	parm->set_is_def();
      }
  }
}

static int process_ppp(smtHeader* h, smtTree* cbody, char const *fn)
{
  smtTree *cur = cbody->get_first(), *next;
  
  for(; cur; cur = next)
    {
      next = cur->get_next();
      if(cur->type == SMT_token && next && next->type == SMT_token && cur->extype == SMTT_kwd)
	{
	  char const *txt = h->srcbuf+cur->tbeg;
	  if(txt[0]=='p' && 
	     (!strncmp(txt,"public",6) ||
	      !strncmp(txt,"private",7) ||
	      !strncmp(txt,"package",7) ||
	      !strncmp(txt,"protected",9)) &&
	     h->srcbuf[next->tbeg] == ':')
	    {
	      int s1 = cur->tnfirst;
	      int s2 = next->tnfirst;
	      next = next->get_next();
	      smt_mark_stree(0,2,SMT_stmt,s1,fn,s2,fn,h);
	    }
	}
    }

  return 1;
}

static smtTree* smt_ifl_mark_pdecl(astnodeStruct* a, astnodeStruct* par, char const *fname)
{
  smtTree* smt = NULL;
  int tfirst, tlast;
  int is_in_list = 0, is_first_in_list = 0, list_first=0, list_last=0, count=0; 
  if (par)
    {
      int i, ending = 0;
      for(i=0; i<par->children->size(); i++)
	{
	  astnodeStruct* cur = (*par->children)[i];
	  int code = dfa_get_ast_code(cur);
	  if(code==MC_PARM_DECL)
	    {
	      int first, last;
	      ifl_first_last(smt_header_static, cur, first, last); 
	      if(first > 0 && list_first == first)
		count++;
	      else if (!ending)
		{
		  count = 1;
		  list_first = first;
		  list_last = last;
		}
	      else
		break;
	      if (a == cur)
		{
		  ending = 1;
		  if (count == 1)
		    is_first_in_list = 1;
		}
	    }
	  else if (!ending)
	    {
	      count = 0;
	      list_first = 0;
	      list_last = 0;
	    }
	  else
	    break;
	}
      if (count > 1)
	is_in_list = 1;
    }
  if (is_in_list)
    {
      if (is_first_in_list)
	smt_mark_stree(0,2,SMT_list_decl,list_first,fname, list_last,fname,smt_header_static);
      astnodeStruct* ch = ifl_get_child(a,2);
      if (ch)
	{
	  ifl_first_last(smt_header_static, ch, tfirst, tlast); 
	  smt=smt_mark_stree(0,2,SMT_pdecl,tfirst,fname,tlast,fname,smt_header_static);
	}
    }
  else
    {
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_pdecl,tfirst,fname,tlast,fname,smt_header_static);
    }
  return smt;
}

typedef struct ast_context {
  Relational *ep;
  int identifier;
} ast_context;

void smt_map_ifl(astnodeStruct* a, astnodeStruct* par,ast_context *context=NULL)
{
  char const *fname = smt_header_static->get_filename();
  int code = dfa_get_ast_code(a);
  int i;
  int tfirst, tlast;
  smtTree* smt = NULL;
  Relational *ep = NULL;
  int identifier = -1;
  int is_def = 0;
  int smt_type;
  astnodeStruct *ch = NULL;
  ast_context *next_context = context;
  ast_context local_context = {NULL, -1};
  int is_elsif = 0;

  switch (code)
    {
    case DFA_LIST_DECL:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_list_decl,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;

    case DFA_DECLSPEC:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_declspec,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;
      
    case DFA_CURSOR_DEF:
    case DFA_FUNCTION_DEF:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_fdef,tfirst,fname,tlast,fname,smt_header_static);
      local_context.identifier = a->identifier;
      next_context = &local_context;
      MAP_DOWN(a);
      break;
      
    case CTOR_INIT:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt=smt_mark_stree(0,2,SMT_expr,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;

    case DFA_CURSOR_DECL:
    case MC_FUNCTION_DECL:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      if (tfirst <= 0 || tlast <= 0)
	ifl_first_last_deep(smt_header_static, a, tfirst, tlast);
      {
	int title_end = tlast;
	if(par)
	  {
	    if (par->node && dfa_get_ast_code(par) == DFA_FUNCTION_DEF)
	      {
		int last = par->children->size();
		astnodeStruct* ctor_init = ifl_get_child(par,last-1);
		if(ctor_init&&dfa_get_ast_code(ctor_init)==CTOR_INIT)
		  {
		    int ctor_first,ctor_last;
		    ifl_first_last(smt_header_static,ctor_init, ctor_first, ctor_last);
		    if(ctor_last > title_end)
		      title_end = ctor_last;
		  }
	      }
	  }
	smt_mark_stree(0,2,SMT_title,tfirst,fname,title_end,fname,smt_header_static);
      }
      if(par)
	{
	  if (par->node && (dfa_get_ast_code(par) == DFA_FUNCTION_DEF
	      || dfa_get_ast_code(par) == DFA_CURSOR_DEF ))
	    {
	      for(i=0; i< par->children->size(); i++)
		if((*(par->children))[i] == a)
		  {
		    astnodeStruct* spec = 0;
		    if (i > 0)
		      {
			astnodeStruct *prev = (*(par->children))[i-1];
			if (prev->node && dfa_get_ast_code(prev)==DFA_DECLSPEC)
			  spec = prev;
		      }
		    if (!spec && i +1 < par->children->size())
		      {
			astnodeStruct *next = (*(par->children))[i+1];
			if (next->node && dfa_get_ast_code(next)==DFA_DECLSPEC)
			  spec = next;
		      }
		    if (spec)
		      {
			int spec_first, spec_last;
			ifl_first_last(smt_header_static, spec, spec_first, spec_last);
			if (spec_last > 0 && spec_first >0)
			  {
			    if (tfirst > spec_first)
			      tfirst = spec_first;
			    if (tlast < spec_last)
			      tlast = spec_last;
			  }
		      }
		  }
	    }
	}
      smt = smt_mark_stree(0,2,SMT_fdecl,tfirst,fname,tlast,fname,smt_header_static);
      if (a->identifier)
	identifier = a->identifier;
      else
	{
	  astnodeStruct* ch = ifl_get_child(a,1);
	  if (ch)
	    identifier = ch->identifier;
	}
      if (identifier > 0)
	{
	  Relational* ep = symbols_get_symbol(identifier);
 	  is_def = a->definition;
	  if (is_def)
	    {
	      smtTree* par_smt = smt ? smt->get_parent() : NULL;
	      if (par_smt && par_smt->get_node_type() == SMT_title)
		par_smt = par_smt->get_parent();
	      if (par_smt && par_smt->get_node_type() == SMT_fdef) {
		put_relation(def_smt_of_dd, ep, par_smt);
		context->ep = ep;
	      }
	      if (context != NULL && context->identifier == identifier) {
		context->ep = ep;
	      }
	      while(par_smt && par_smt->get_node_type() != SMT_cdecl)
		par_smt = par_smt->get_parent();
	      if (par_smt)
		put_relation(decl_smt_of_dd, ep, smt);
	    }
	  else
	    put_relation(decl_smt_of_dd, ep, smt);
	}
      MAP_DOWN(a);
      break;

    case MC_PARM_DECL:
      smt=smt_ifl_mark_pdecl(a,par,fname);
      els_handle_decl(smt, a, context?context->ep:NULL, 1);
      MAP_DOWN(a);
      break;
      
    case MC_CALL_EXPR:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_expr,tfirst,fname,tlast,fname,smt_header_static);
      
      if ( a->children )
	{
	  astnodeStruct* fun = ifl_get_child(a,1);
	  if (fun && fun->identifier)
	    {
	      Relational* ep = symbols_get_symbol(fun->identifier);
	      if(ep && smt && smt->type != SMT_macrocall)
		put_relation(smt_of_dd, ep, smt);
	    }
	  for ( i = 1; i < a->children->size(); i++ ) 
	    {
	      astnodeStruct *arg = ifl_get_child(a,i+1);
	      ifl_first_last(smt_header_static, arg, tfirst, tlast);
	      if (tfirst && tlast)
		smt_mark_stree(0,2,SMT_expr,tfirst,fname,tlast,fname,smt_header_static);   
	    }
	}
      MAP_DOWN(a);
      break;
      
    case MC_VAR_DECL:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      if (par && dfa_get_ast_code(par) == DFA_LIST_DECL)
	{
	  smt=smt_mark_stree(0,2,SMT_decl,tfirst,fname,tlast,fname,smt_header_static);
	  identifier = a->identifier;
	  if (!identifier)
	    {
	      ch = ifl_get_child(a,1);
	      if (ch)
		identifier = ch->identifier;
	    }
	  if (identifier)
	    ep = (ddElement*)symbols_get_symbol(identifier);
	  if (ep)
	    if (a->definition)
	      {
		if (is_elsElement(ep)) {
		  elsElement *elsp = (elsElement *)ep;
		  if (elsp->is_static()) {
		    ep = elsp->get_dd();
		  }
		}
		put_relation(def_smt_of_dd, ep, smt);
		if (!context) 
		  {
		    next_context = &local_context;
		    local_context.ep = ep;
		    local_context.identifier = identifier;
		  }
	      }
	    else
	      put_relation(decl_smt_of_dd, ep, smt);
	  els_handle_decl(smt, a, context?context->ep:NULL);
	}
      else
	{
	  smt=smt_mark_stree(0,2,SMT_list_decl,tfirst,fname,tlast,fname,smt_header_static);
	  ch = ifl_get_child(a,1);
	  if (ch)
	    {
	      ifl_first_last(smt_header_static, ch, tfirst, tlast);
	      smtTree* decl =smt_mark_stree(0,2,SMT_decl,tfirst,fname,tlast,fname,smt_header_static);
	      els_handle_decl(decl, a, context?context->ep:NULL);
	    }
	}
      MAP_DOWN(a);
      break;

    case MC_FIELD_DECL:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      if (par && dfa_get_ast_code(par) == DFA_LIST_DECL)
	{
	  smt=smt_mark_stree(0,2,SMT_decl,tfirst,fname,tlast,fname,smt_header_static);
	  identifier = a->identifier;
	  if (!identifier)
	    {
	      ch = ifl_get_child(a,1);
	      if (ch)
		identifier = ch->identifier;
	    }
	  if (identifier)
	    ep = (ddElement*)symbols_get_symbol(identifier);
	  if (ep)
	    {
	      if (a->definition)
		put_relation(def_smt_of_dd, ep, smt);
	      put_relation(decl_smt_of_dd, ep, smt);
	    }   
	  els_handle_decl(smt, a, context?context->ep:NULL);
	}
      else
	{
	  smt=smt_mark_stree(0,2,SMT_list_decl,tfirst,fname,tlast,fname,smt_header_static);
	  ch = ifl_get_child(a,1);
	  if (ch)
	    {
	      ifl_first_last(smt_header_static, ch, tfirst, tlast);
	      smtTree* decl =smt_mark_stree(0,2,SMT_decl,tfirst,fname,tlast,fname,smt_header_static);
	      els_handle_decl(decl, a, context?context->ep:NULL);
	    }
	}
      MAP_DOWN(a);
      break;

    case MC_CONST_DECL:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_enum_field,tfirst,fname,tlast,fname,smt_header_static);
      identifier = a->identifier;
      if (identifier)
	ep = (ddElement*)symbols_get_symbol(identifier);
      if (ep)
	if (a->definition)
	  put_relation(def_smt_of_dd, ep, smt);
	else
	  put_relation(decl_smt_of_dd, ep, smt);
      els_handle_decl(smt, a, context?context->ep:NULL);
      MAP_DOWN(a);
      break;
 
    case EXCEPTION_DECL:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_list_decl,tfirst,fname,tlast,fname,smt_header_static);
      ch = ifl_get_child(a,1);
      if (ch)
	{
	  ifl_first_last(smt_header_static, ch, tfirst, tlast);
	  smt_mark_stree(0,2,SMT_decl,tfirst,fname,tlast,fname,smt_header_static);
	}
      els_handle_decl(smt, a, context?context->ep:NULL);
      MAP_DOWN(a);
      break;


    case USING_DECL:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_decl,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;

    case MC_NAMESPACE_DECL:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_cdecl,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;

    case TEMPLATE_FCN:
    case TEMPLATE_CLASS:
    case TEMPLATE_STRUCT:
    case TEMPLATE_UNION:
    case TEMPLATE_STATIC_DATA_MBR:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt_type = SMT_cdecl;
      identifier = a->identifier;
      if (identifier)
	ep = symbols_get_symbol(identifier);
      is_def = a->definition;
      smt = smt_mark_stree(0,2,smt_type,tfirst,fname,tlast,fname,smt_header_static);
      if (ep && is_def)
	put_relation(def_smt_of_dd, ep, smt);
      else if (ep)
	put_relation(decl_smt_of_dd, ep, smt);
      MAP_DOWN(a);
      break;

    case TEMPLATE_HEADER:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt_mark_stree(0,2,SMT_title,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;
      
    case TEMPLATE_BODY:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt_mark_stree(0,2,SMT_cbody,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;

    case TYPE_DECL_MODIFIED:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      identifier = a->identifier;
      is_def = a->definition;
      if (identifier)
	ep = symbols_get_symbol(identifier);
      smt = smt_mark_stree(0,2,SMT_cdecl,tfirst,fname,tlast,fname,smt_header_static);
      if (ep && is_def)
	put_relation(def_smt_of_dd, ep, smt);
      else if (ep)
	put_relation(decl_smt_of_dd, ep, smt);
      ch = ifl_get_child(a,2);
      if (ch) 
	{
	  int ch_first, ch_last=0;
	  ifl_first_last_deep(smt_header_static,ch,ch_first,ch_last);
	  if (ch_last > 0)
	    smt_mark_stree(0,2,SMT_title,tfirst,fname,ch_last,fname,smt_header_static);
	  
	  ch = ifl_get_child(a,3);
	  if (ch && !ch->location && dfa_get_ast_code(ch) == MC_COMPOUND_EXPR) 
	    smt_mark_stree(0,2,SMT_cbody,ch_first+1,fname,tlast,fname,smt_header_static);
	}
      MAP_DOWN(a);
      break;
      
    case MC_TYPE_DECL:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt_type = SMT_decl;
      ch = ifl_get_child(a,1);
      identifier = a->identifier;
      if (!identifier && ch)
	identifier = ch->identifier;
      if (identifier)
	ep = symbols_get_symbol(identifier);
      if (ep)
	{
	  is_def = a->definition;
	  int ch_first, ch_last=0;
	  ddKind knd = get_kind(ep);
	  switch(knd)
	    {
	    case DD_CLASS:
	    case DD_UNION:
	    case DD_PACKAGE:
	    case DD_SQL_TABLE:
	      ifl_first_last(smt_header_static, ch, ch_first, ch_last);
	      smt_type = SMT_cdecl;
	      break;
	      
	    case DD_ENUM:
	      ifl_first_last(smt_header_static, ch, ch_first, ch_last);
	      smt_type = SMT_edecl;
	      break;

	    default:
	      break;
	    }
	  if(ch_last)
	    smt_mark_stree(0,2,SMT_title,tfirst,fname,ch_last,fname,smt_header_static);
	}
      smt = smt_mark_stree(0,2,smt_type,tfirst,fname,tlast,fname,smt_header_static);
      if (ep && is_def)
	put_relation(def_smt_of_dd, ep, smt);
      else if (ep)
	put_relation(decl_smt_of_dd, ep, smt);
      MAP_DOWN(a);
      break;
      
    case MC_COMPOUND_EXPR:
      smt_type=SMT_block;
      if (par && (dfa_get_ast_code(par) == MC_TYPE_DECL ||
		  dfa_get_ast_code(par) == TYPE_DECL_MODIFIED)) {
	identifier = par->identifier;
	if (identifier)
	  ep = symbols_get_symbol(identifier);
	ddKind knd = DD_UNKNOWN;
	if (ep)
	  knd = get_kind(ep);
	if (knd == DD_ENUM)
	  smt_type = SMT_ebody;
	else
	  smt_type = SMT_cbody;
      }
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      if (tfirst > 0 &&  tlast > 0)
	smt=smt_mark_stree(0,2,smt_type,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      if(smt && smt_type == SMT_cbody)
	process_ppp(smt_header_static,smt,fname);
      break;
      
    case MC_GOTO_STMT:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_goto,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;
      

    case PROXY_IMPORT:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_decl,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;
      
      
    case MC_EXPR_STMT:
    case ASSIGN_STMT:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_stmt,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;
      
    case MC_RETURN_STMT:
      if(!a->generated)
	{
	  ifl_first_last(smt_header_static, a, tfirst, tlast); 
	  smt=smt_mark_stree(0,2,SMT_stmt,tfirst,fname,tlast,fname,smt_header_static);
	  MAP_DOWN(a);
	}
      break;

    case DFA_WHERE:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_title,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;

    case MC_IF_STMT:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      if (par)
	{
	  int par_code = dfa_get_ast_code(par);
	  if (par_code == MC_IF_STMT)
	    {
	      int last = par->children->size();
	      if (last > 2 && a == ifl_get_child(par,last))
		is_elsif = 1;
	    }
	}
      if (!is_elsif)
	smt=smt_mark_stree(0,2,SMT_if,tfirst,fname,tlast,fname,smt_header_static);
      ch = ifl_get_child(a,1);
      if (ch)
	{
	  int title_first, title_last;
	  ifl_first_last(smt_header_static, ch, title_first, title_last);
	  astnodeStruct* then_node  = ifl_get_child(a,2);
	  int ch_first, ch_last;
	  if(then_node)
	    ifl_first_last_deep(smt_header_static, then_node, ch_first, ch_last);
	  if (ch_first && ch_first - 1 >= title_last)
	    title_last = ch_first - 1;
	  if (!is_elsif)
	    {
	      if (title_last)
		smt_mark_stree(0,2,SMT_title,tfirst,fname,title_last,fname,smt_header_static);
	      if (ch_last)
		smt_mark_stree(0,2,SMT_then_clause,tfirst,fname,ch_last,fname,smt_header_static);
	    }
	  else
	    {
	      int else_loc = tfirst;
	      for (; else_loc > 0; --else_loc)
		{
		  smtTree* t = smt_header_static->ttable[else_loc-1];
		  char const *txt =  smt_header_static->srcbuf+t->tbeg;
		  if (t->extype == SMTT_kwd && t->tlth == 4 && 
		      ((txt[0]=='e'&&txt[1]=='l'&&txt[2]=='s'&&txt[3]=='e')||
		       (txt[0]=='E'&&txt[1]=='L'&&txt[2]=='S'&&txt[3]=='E')))
		    break;
		}
	      if (else_loc)
		tfirst = else_loc;
	      if (title_last)
		smt_mark_stree(0,2,SMT_title,tfirst,fname,title_last,fname,smt_header_static);
	      if (ch_last)
		smt_mark_stree(0,2,SMT_else_if_clause,tfirst,fname,ch_last,fname,smt_header_static);
	    }
	}
      MAP_DOWN(a);
      if (a->children)
	{
	  int last = a->children->size();
	  if (last > 2) {
	    astnodeStruct* ch = ifl_get_child(a,last);
	    int ch_code = dfa_get_ast_code(ch);
	    if (ch_code != ELSIF_CLAUSE && ch_code != MC_IF_STMT)
	      {
		int ch_first, ch_last;
		ifl_first_last(smt_header_static, ch, ch_first, ch_last);
		// Should be replaced by reporting ELSE location in .if file
		int else_loc = ch_first;
		for (; else_loc > 0; --else_loc)
		  {
		    smtTree* t = smt_header_static->ttable[else_loc-1];
		    char const *txt =  smt_header_static->srcbuf+t->tbeg;
		    if (t->extype == SMTT_kwd && t->tlth == 4 && (
                        (txt[0]=='e'&&txt[1]=='l'&&txt[2]=='s'&&txt[3]=='e')||
                        (txt[0]=='E'&&txt[1]=='L'&&txt[2]=='S'&&txt[3]=='E')))
		      break;
		  }
		if (else_loc && else_loc > tfirst)
		  ch_first = else_loc;
		smt_mark_stree(0,2,SMT_else_clause,ch_first,fname,ch_last,fname,smt_header_static);
	      }
	  }
	}
      break;

    case ELSIF_CLAUSE:
      ifl_first_last(smt_header_static, a, tfirst, tlast); 
      smt=smt_mark_stree(0,2,SMT_else_if_clause,tfirst,fname,tlast,fname,smt_header_static);
      ch = ifl_get_child(a,1);
      if (ch)
	{
	  int ch_first, ch_last;
	  ifl_first_last(smt_header_static, ch, ch_first, ch_last);
	  if (ch_last)
	    smt_mark_stree(0,2,SMT_title,tfirst,fname,ch_last,fname,smt_header_static);
	}
      MAP_DOWN(a);
      break;
      
    case TRY_BLOCK:
    case MICROSOFT_TRY:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt=smt_mark_stree(0,2,SMT_try_catch,tfirst,fname,tlast,fname,smt_header_static);
      ch = ifl_get_child(a,1);
      if (ch)
	{
	  int ch_first, ch_last;
	  ifl_first_last(smt_header_static, ch, ch_first, ch_last);
	  smt_mark_stree(0,2,SMT_try_clause,tfirst,fname,ch_last,fname,smt_header_static);
	  if(ch_first > tfirst)
	    smt_mark_stree(0,2,SMT_title,tfirst,fname,ch_first-1,fname,smt_header_static);
	}
      MAP_DOWN(a);
      break;
      
    case CATCH:
    case FINALLY:
      {
	ifl_first_last(smt_header_static, a, tfirst, tlast);
	ch = ifl_get_child(a,2);
	int ch_first=0, ch_last=0;
	if (ch)
	  ifl_first_last(smt_header_static, ch, ch_first, ch_last);
	if (!tfirst)
	  {
	    tlast = ch_last;
	    astnodeStruct* try_clause = ifl_get_child(par,1);
	    int tr_first, tr_last;
	    ifl_first_last(smt_header_static, try_clause, tr_first, tr_last);
	    tfirst = tr_last + 1;
	  }
	smt=smt_mark_stree(0,2,SMT_catch_clause,tfirst,fname,tlast,fname,smt_header_static);
	if (ch && ch_first > tfirst)
	  smt_mark_stree(0,2,SMT_title,tfirst,fname,ch_first-1,fname,smt_header_static);
      }
      MAP_DOWN(a);
      break;
      
    case MC_SWITCH_STMT:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt=smt_mark_stree(0,2,SMT_switch,tfirst,fname,tlast,fname,smt_header_static);
      ch = ifl_get_child(a,2);
      if(ch)
	{
	  int ch_first, ch_last;
	  ifl_first_last(smt_header_static, ch, ch_first, ch_last);
	  if (ch_first - 1 > tfirst)
	    tlast = ch_first - 1;
	  smt_mark_stree(0,2,SMT_title,tfirst,fname,tlast,fname,smt_header_static);
	}
      MAP_DOWN(a);
      break;

    case CASE_VALUES:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      if (tfirst <= 0)
	{
	  ifl_first_last_deep(smt_header_static, a, tfirst, tlast);
	}
      smt_mark_stree(0,2,SMT_title,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;

    case SWITCH_CLAUSE:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      if (tfirst <= 0)
	{
	  ifl_first_last_deep(smt_header_static, a, tfirst, tlast);
	}
      smt=smt_mark_stree(0,2,SMT_case_clause,tfirst,fname,tlast,fname,smt_header_static);
      MAP_DOWN(a);
      break;
     
    case EXIT_STMT:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt=smt_mark_stree(0,2,SMT_break,tfirst,fname,tlast,fname,smt_header_static);
      break;

    case PACKAGE_BODY:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt=smt_mark_stree(0,2,SMT_cdecl,tfirst,fname,tlast,fname,smt_header_static);
      if (a->children)
	{
	  astnodeStruct *ch = (*(a->children))[0];
	  if (ch)
	    {
	      int ch_first = -1, ch_last = -1;
	      ifl_first_last(smt_header_static, ch, ch_first, ch_last);
	      if (ch_first > 0)
		smt_mark_stree(0,2,SMT_title,tfirst,fname,ch_first,fname,smt_header_static);
	    }
	}

      MAP_DOWN(a);
      break;
      
    case MC_DO_STMT:
    case MC_FOR_STMT:
    case MC_WHILE_STMT:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt=smt_mark_stree(0,2,SMT_nstdloop,tfirst,fname,tlast,fname,smt_header_static);
      if(a->children)
	{
	  int b = a->children->size();
	  astnodeStruct* body = ifl_get_child(a,b);
	      int ch_first = -1, ch_last = -1;
	      ifl_first_last(smt_header_static, body, ch_first, ch_last);
	      if (ch_first > 0)
		smt_mark_stree(0,2,SMT_title,tfirst,fname,ch_first-1,fname,smt_header_static);

	}
      MAP_DOWN(a);
      break;

    case DO_WHILE_STMT:
      ifl_first_last(smt_header_static, a, tfirst, tlast);
      smt=smt_mark_stree(0,2,SMT_nstdloop,tfirst,fname,tlast,fname,smt_header_static);
      if(a->children)
	{
	  int b = a->children->size();
	  astnodeStruct* body = ifl_get_child(a,b);
	  int ch_first = -1, ch_last = -1;
	  ifl_first_last(smt_header_static, body, ch_first, ch_last);
	  if (ch_first > 0)
	    smt_mark_stree(0,2,SMT_title,tfirst,fname,ch_first-1,fname,smt_header_static);
	  if (ch_last > 0)
	    smt_mark_stree(0,2,SMT_title,ch_last+1,fname,tlast,fname,smt_header_static);
	}
      MAP_DOWN(a);
      break;

    case DFA_REF_DD:
      {
	Relational* used = symbols_get_symbol(a->identifier);
	if (used)
	  {
	    if(next_context && next_context->ep && used != next_context->ep)
	      put_relation(used_of_user,next_context->ep,used);

	    locationStruct * loc = NULL;

	    if (a->location)
	      loc = (*a->location)[0];

	    if (!loc) {
	      int level = 1;
	      while (1) {
		astnodeStruct* parent = ast_stack_array.get_value(level);
		if (!parent)
		  break;
		if (parent->location)
		  loc = (*parent->location)[0];
		if (loc)
		  break;
		level++;
	      }
	    }

	    if (loc)
	      {
		int start = -1, length = -1;
		ifl_start_length(smt_header_static, loc, start, length);
		smtTree* t = NULL;
		if (start >= 0)
		  t = els_cn_na(smt_header_static,start);
		smtTree* mcall = els_get_mcall (t);
		if (mcall)
		  {
		    ddElement *macro = smt_get_dd(mcall);
		    if (macro && macro->get_kind() == DD_MACRO)
		      put_relation(has_friend,macro,used);
		  }
	      }
	  }
      }
      break;
      
    default:
      MAP_DOWN(a);
      break;
    }
}

void els_fix_locals()
{
  int sz = locals.size();
  for(int ii=0; ii<sz; ++ii){
    elsElement*els = (elsElement*) locals[ii];
    ddElement*dd = els->get_dd();
    if(dd){
      for (int jj = 0; jj <els->attributes.size(); ++jj)
	{
	  ELS_att* att = els->attributes[jj];
	  els_update_attribute(dd,att->get_attr(),att->get_value());
	}
      if (els_get_dec(els))
	els_map_local(dd, els);
      else
	els_move_rels(els, dd);
    }
  }
}

enum Els_Cpp_Code {
  ELS_CPP_OTHER = 0,
  ELS_CPP_INCLUDE,
  ELS_CPP_DEFINE,
  ELS_CPP_MACRO_CALL,
  ELS_CPP_MACRO_ARG,
  ELS_CPP_RAW_ARG,
  ELS_CPP_PROCESSED_ARG,
  ELS_CPP_UNDEF,
  ELS_CPP_IFDEF,
  ELS_CPP_IFNDEF,
  ELS_CPP_THEN,
  ELS_CPP_ELSE,
  ELS_CPP_IF,
  ELS_CPP_ELIF,
  ELS_CPP_NULL_OPERAND,
  ELS_CPP_LAST
  };

static char const *Els_Cpp_Code_names[] = { 
  "other", 
  "cpp_include",
  "cpp_define",
  "cpp_macro_call",
  "cpp_macro_arg",
  "cpp_raw_arg",
  "cpp_processed_arg",
  "cpp_undef",
  "cpp_ifdef",
  "cpp_ifndef",
  "cpp_then",
  "cpp_else",
  "cpp_if",
  "cpp_elif",
  "cpp_null_operand"
  };

static Els_Cpp_Code get_cpp_code(char const *name)
{
  Els_Cpp_Code retval = ELS_CPP_OTHER;
  
  for(int ii = 1; ii < ELS_CPP_LAST; ++ii)
    if(!strcmp(name,Els_Cpp_Code_names[ii]))
      {
	retval = ( Els_Cpp_Code)ii;
	break;
      }
  
  return retval;     
}

static void els_process_macro_def(ddElement*dd, astnodeStruct* an)
{
  int sz = an->children->size();
  int no_arg = sz - 1;

  astnodeStruct * body = (*(an->children))[no_arg];
  char const *body_str = body->value;
  
  if (!body_str || strlen(body_str) > 60)
    return;

  ocharstream buf;
  if(no_arg > 0){
    buf << '(';
    for(int ii=0; ii<no_arg; ++ii){
       astnodeStruct * arg = (*(an->children))[ii];
       if(ii>0)
	 buf << ',';
       if(arg->value) 
	 buf << arg->value;
    }
    buf << ')';
    buf << ' ';
  } else {
    dd->set_const();
  }

  buf << body_str;
  buf << '\0';
  char *ns   = buf.ptr();

  int nl     = strlen(ns);
  ddElement* impl_dd = dd_root_static->add_def(DD_VERSION, nl, ns, 0, "", 0, "");
  impl_dd->put_rel(*dd, user_of_used);
}

smtTree* smt_map_cpp (astnodeStruct* an)
{
  char const *fname = smt_header_static->get_filename();
  int code = get_cpp_code(an->node);
  
  int tfirst, tlast;
  smtTree* node = NULL;
  ddElement * dd = NULL;
  int go_down = 0;
  switch (code)
    {
    case ELS_CPP_INCLUDE:
      ifl_first_last(smt_header_static, an, tfirst, tlast);
      if (tfirst > 0 && tlast >0)
	node = smt_def_stmt(smt_header_static,2, SMT_m_include,tfirst,tlast);
      if (node && an->identifier)
	{
	  smtTree* tok = node->get_first();
	  if (tok->extype == SMTT_untok)
	    tok->extype = SMTT_macro;
	  dd = (ddElement*)symbols_get_symbol(an->identifier);
	  if (dd && tok)
	    put_relation(ref_smt_of_dd, dd, tok);
	}
      break;

    case ELS_CPP_DEFINE:
      ifl_first_last(smt_header_static, an, tfirst, tlast);
      if (tfirst > 0 && tlast >0)
	node = smt_def_stmt(smt_header_static,2, SMT_m_define,tfirst,tlast);
      if (node && an->identifier)
	{
	  dd=(ddElement*)symbols_get_symbol(an->identifier);
	  if (dd){
	    put_relation (def_smt_of_dd, dd, node);
	    if(an->children)
	      els_process_macro_def(dd, an);
	  }
	}
      break;

    case ELS_CPP_MACRO_CALL:
      ifl_first_last(smt_header_static, an, tfirst, tlast);
      if (tfirst > 0 && tlast >0)
	node = smt_mark_stree(0, 2, SMT_macrocall, tfirst, fname,tlast, fname, smt_header_static);
      if (node && an->identifier)
	{
	  dd=(ddElement*)symbols_get_symbol(an->identifier);
	  if (dd)
	    put_relation(smt_of_dd, dd, node);
	}
      go_down = 1;
      break;


    case ELS_CPP_MACRO_ARG:
      ifl_first_last(smt_header_static, an, tfirst, tlast);
      if (tfirst > 0 && tlast >0)
	node = smt_mark_stree(0, 2, SMT_macroarg, tfirst, fname,tlast, fname, smt_header_static);
      go_down = 1;
      break;

    default:
      go_down = 1;
      break;
    }

  if (go_down && an->children)
    for (int ii = 0; ii<an->children->size(); ++ii)
      {
	astnodeStruct* ch = (*(an->children))[ii];
	smt_map_cpp(ch);
      }

  return node;
}

static int is_main_ast(astnodeStruct* an)
{
  int retval = 1;

  if (an)
    {
      astnodeStruct* child = ifl_get_child(an,1);
      if (child && child->node)
	{
	  char *code = child->node;
	  if (code[0]=='c' && 
	      code[1]=='p' &&
	      code[2]=='p' &&
	      code[3]=='_')
	    retval = 0;
	}
    }

  return retval;
}

static int els_check_macros(smtTree* root, ddElement* context = NULL)
{
  Initialize(els_check_macros);
  int retval = 0;

  if (root->get_node_type() == SMT_macrocall) {
    smtTree* cexpr = root;
    smtTree *glob_ent = smt_get_proper_scope(cexpr);
    ddElement *dd = 0;
    if (glob_ent && (dd = smt_get_dd(glob_ent))) {
      used_put_user(cexpr, dd);
      retval++;
    }
  }
  
  if (context) {
    ddElement *ref_dd = checked_cast(ddElement,get_relation(ref_dd_of_smt,root));
    if (ref_dd && ref_dd != context && ref_dd->get_kind() != DD_LOCAL) {
      used_put_user(ref_dd, context);
      retval++;
    }
  }

  if (!context) {
    ddElement* cont = checked_cast(ddElement,get_relation(def_dd_of_smt,root));
    if (cont) {
      ddKind knd = cont->get_kind();
      if (knd == DD_FUNC_DECL ||
	  knd == DD_VAR_DECL ||
	  knd == DD_CURSOR)
	context = cont;
    }
  }
  for(smtTree* cur = root->get_first(); cur; cur = cur->get_next())
    retval += els_check_macros(cur,context);

  return retval;
}

int ifl_ast_report_tree(astnodeList *a)
{
  ifl_complete_tokens(smt_header_static, a);

  int length = a ? a->size() : 0;
  int i;

  for ( i = 0; i < length; i++ )
    {
      astnodeStruct *an = (*a)[i];
      if (!is_main_ast(an))
	smt_map_cpp(an);
    }

  for ( i = 0; i < length; i++ )
    {
      astnodeStruct *an = (*a)[i];
      if (is_main_ast(an))
	{
	  dfa_els_build(smt_header_static, an);
	  ast_stack_array.reset();
	  smt_map_ifl(an,NULL,NULL);
	}
    }

  set_rel_struct();
  els_fix_locals();
  els_check_macros((smtTree*)smt_header_static->get_root());
  smt_header_static->set_arn(0);
  dd_root_static = NULL;

  return 1;
}

static int mark_tokens(smtHeader* h, char *arr, astnodeStruct* an)
{
  int i;
  if (an->location)
    {
      for (i = 0; i < an->location->size(); i++ )
	{
	  locationStruct * loc = (*an->location)[i];
	  int start = -1, length = -1;
	  ifl_start_length(h, loc, start, length);
	  if (start >= 0)
	    {
	      arr[start] = 1;
	      if (length > 0)
		arr[start+length] = 1;
	    }
	}
    }

  if ( an->children )
    {
      for ( i = 0; i < an->children->size(); i++ )
	{
	  astnodeStruct* a = (*(an->children))[i];
	  if (a)
	    mark_tokens(h,arr,a);
	}
    }

  return 1;
}

static int ifl_complete_tokens(smtHeader* h, astnodeList* l)
{
  if (l) {
    int size = h->src_size;
    char *arr = new char[size+1];
    memset(arr, 0, size + 1) ;
    
    int length = l->size();
    for(int idx=0; idx < length; ++idx)
      {
	astnodeStruct *an = (*l)[idx];
	mark_tokens(h,arr,an);
      }

    smtTree* root = (smtTree*)h->get_root();
    smtTree* cur = root->get_first_leaf();
    for (int pos = 0; pos < size; ++pos)
      {
	if (arr[pos])
	  {
	    while (cur && cur->tbeg + cur->tlth <= pos)
	      cur = cur->get_next_leaf();
	    if (cur && cur->tbeg < pos)
	      {
		smtTree *t = new smtTree(SMT_token);
		t->extype = SMTT_untok;
		t->tbeg = pos;
		t->tlth = cur->tbeg + cur->tlth - pos;
		cur->tlth = pos - cur->tbeg;
		cur->put_after(t);
		cur = t;
	      }
	  }
      }
    delete arr;
  }
  compress_tokens(h);
  smt_new_create_ttable(h);
  
  return 1;
}

static int compress_tokens (smtHeader* h)
{
  Initialize(compress_tokens);

  smtTree* root = (smtTree*)h->get_root();
  char *srcbuf = h->srcbuf;
  smtTree *prev = NULL, *cur = root->get_first_leaf(), *next;

  unsigned int max_spaces = (1 << (sizeof (cur->spaces) * 8)) - 1;

  for (; cur; prev = cur, cur = next)
    {
      next = cur->get_next_leaf();
      if (prev && !prev->newlines)
	{
	  int prev_is_white = 1;
	  while (prev->tlth > 0)
	    {
	      char c =  srcbuf[prev->tbeg + prev->tlth - 1];
	      if ((c == ' ' || c == '\t') && cur->spaces <  max_spaces)
		{
		  prev->tlth--;
		  cur->spaces++;
		}
	      else
		{
		  prev_is_white = 0;
		  while(prev->tlth)
		    {
		      if(srcbuf[prev->tbeg + prev->tlth - 1] == '\n')
			{
			  if (!prev->newlines)
			    prev->newlines = 1;
			  else
			    {
			      smtTree * nl = new smtTree(SMT_token);
			      nl->tbeg = prev->tbeg+prev->tlth-1;
			      nl->tlth = 1;
			      nl->spaces =0;
			      nl->newlines =0;
			      nl->extype = SMTT_el;
			      prev->put_after(nl);
			    }
			  prev->tlth--;
			}
		      else
			break;
		    }
		  break;
		}
	    }
	  if (prev_is_white)
	    {
	      cur->spaces += prev->spaces;
	      prev->remove_from_tree();
	      obj_delete(prev);
	    }
	}  
      while (cur->tlth > 0)
	{
	  char c = srcbuf[cur->tbeg];
	  if ((c == ' ' || c == '\t') && cur->spaces <  max_spaces)
	    {
	      cur->tlth--;
	      cur->tbeg++;
	      cur->spaces++;
	    }
	  else
	    break;
	}
      while (cur->tlth > 0)
	{
	  char c = srcbuf[cur->tbeg+cur->tlth-1];
	  if (c == '\n')
	    {
	      if (!cur->newlines)
		cur->newlines = 1;
	      else
		{
		  smtTree * nl = new smtTree(SMT_token);
		  nl->tbeg = cur->tbeg+cur->tlth-1;
		  nl->tlth = 1;
		  nl->spaces =0;
		  nl->newlines =0;
		  nl->extype = SMTT_el;
		  cur->put_after(nl);
		}
	      cur->tlth--;
	    }
	  else
	    break;
	}

      if (next && !next->spaces)
	{
	  while (next->tlth)
	    {
	      if (srcbuf[next->tbeg] == '\n')
		{
		  if (!cur->newlines)
		    cur->newlines++;
		  else
		    {
		      smtTree * nl = new smtTree(SMT_token);
		      nl->tbeg = next->tbeg;
		      nl->tlth = 1;
		      nl->spaces =0;
		      nl->newlines =0;
		      nl->extype = SMTT_el;
		      cur->put_after(nl);	    
		    }
		  next->tbeg++;
		  next->tlth--;
		}
	      else
		break;
	    }
	}
    }

  if (prev && !prev->tlth && !prev->newlines && !prev->spaces &&
      prev != root->get_first_leaf())
    {
      prev->remove_from_tree();
      obj_delete(prev);
    }

  return 1;
}

// ===========================================================================
// Get the AST code based on the node type that appeared in the IF.
// The node type may indicate a symbol kind (e.g. "variable") in which case
// the AST code is DFA_REF_DD.
// Returns AST_IGNORE if it's not a type we're concerned with.
//
static int dfa_get_ast_code(astnodeStruct* a)
{
  int retval = AST_IGNORE;

  char const *name = a->node;

  if (name)
    {
      Relational* rel = obj_search_by_name(name, *ast_codes);
      if (rel)
	retval = codeAttribute_get_code(rel);
      else
	{
	  ddKind kind = dd_convert_ifl_code(name);
	  if (kind != DD_BUGS)
	    retval = DFA_REF_DD;
	}
    }

  return retval;
}

ddElement* els_get_dd(int id)
{
  ddElement * retval = NULL;
  
  Relational* rel = symbols_get_symbol(id);
  
  if (rel)
    {
      if (is_ddElement(rel))
	retval = (ddElement*)rel;
      else if (is_elsElement(rel))
	{
	  elsElement* els = (elsElement*)rel;
	  retval = els->get_dd();
	}
    }

  return retval;
}

ddElement* elsElement::get_dd()
{
  if (!dd)
    {
      int this_is_static = is_static();
      char const *cname = dd_ifl_get_cname(this, this_is_static);
      ddElement*type = (ddElement*) get_rel(semtype_of_smt_decl);
      genString buf;
      if (type && !this_is_static)
	{
	  char const *tname = type->get_name();
	  buf.printf("%s %s", tname, cname);
	  cname = buf;
	}
      else
        {
	  buf.printf("%s", cname);
	}
      ddKind kind = this_is_static ? alt_kind : DD_LOCAL;
      dd = dd_lookup_or_create((char const *)buf, (appTree*)smt_header_static, 0, kind, 0, 0);
    }
  return dd;
}

int els_get_decl_id(int id)
{
  int retval = 0;

  Relational* rel = symbols_get_symbol(id);
  if (rel && is_elsElement(rel))
    retval = ((elsElement*)rel)->decl_id;

  return retval;
}

void els_set_decl_id(int id, int val)
{
  Relational* rel= symbols_get_symbol(id);

  if (rel && is_elsElement(rel))
    ((elsElement*)rel)->decl_id = val;
}

int els_get_ast_code(astnodeStruct* a)
{
  int retval = dfa_get_ast_code(a);

  if (retval >=LAST_DFA_TREE_CODE)
    retval = -1;

  if (retval == DFA_REF_DD)
    {
      int id = a->identifier;
      if (id)
	{
	  Relational* rel = symbols_get_symbol(id);
	  if (rel && get_kind(rel) == DD_LOCAL)
	    retval = DFA_REF_DFA;
	}
    }

  return retval;
}
  
Interpreter* InitializeCLIInterpreter();
int cli_eval_string(char const *);


static void 
iff_parse_usage(Interpreter *i)
{
  ostream &os = i->GetOutputStream();

  os << "usage: iff_parse iff_fname" << endl;
}

int els_parse(char const *fname, projModule*mod)
{
  Initialize(els_parse__constchar *fname_projModule*mod);
  int res = 0;
  start_transaction()
    {
      Initialize(els_parse__iff_parse__char *fname);
      if (!smt_ifl_init(mod)) {
  	  return 0;
      } else {
	res = iff_parse(fname);
	smt_ifl_end();
      }    
    }
  end_transaction();

  smtHeader*h = smt_header_static;
  smt_header_static = NULL;

  if(res == 1){
    local_variables_done = 1;
    XREF_insert_module(h);
    local_variables_done = 0;
    ::db_save(h, NULL);

    projNode* proj = mod->get_project();
    projectNode_put_appHeader (proj, h);
    projectModule_put_appHeader (mod, h);
    char const *path = mod->get_phys_filename();
    h->set_phys_name (path);

    if(is_model_build())
       obj_unload(h);
    else
      {
	loadedFiles::proj_add_to_loaded(h);
	proj_path_report_app(h, proj);
      }
  } else {
    obj_unload(h);
  }
  return res;
}
projModule * symbol_get_module( symbolPtr& sym);
extern "C" int 
smtIFL_iff_parseCMD(ClientData cd, 
	Tcl_Interp *interp, 
	int argc, 
	char const *argv[])
{
  Initialize(iff_parseCmd);
  Interpreter *i = (Interpreter *) cd;
  if (argc == 1) {
    iff_parse_usage(i);
    return TCL_OK;
  }
  
  projModule * mod = NULL;

  if (argc > 2 && *argv[2] == '/')
    mod = projHeader::find_module(argv[2]);

  if (!mod) {
    symbolArr arr;
    IF(i->ParseArguments(2, argc, argv, arr) != 0)
      return TCL_ERROR;
    
    IF(arr.size() != 1)
      return TCL_ERROR;
    
    symbolPtr& sym = arr[0];
    mod = symbol_get_module(sym);
  }

  IF(!mod)
    return TCL_ERROR;

  char const *fname   = argv[1];
  int res = els_parse(fname, mod);
  int retval = TCL_OK;
  if (res != 1)
    retval = TCL_ERROR;

  return retval;
}

int els_parse_file(const Relational* mod)
{
  if (is_projModule((Relational*)mod))
    {
      app* ah = ((projModule*)mod)->get_app();
      if(ah && is_smtHeader(ah) && !((smtHeader*)ah)->arn)
	return TCL_OK;
    }
  symbolPtr fsym = (Relational*)mod;
  return els_parse_file(fsym);
 }

int ifl_smt_file()
{
  Initialize(ifl_smt_file);

  if (current_smt_language) {
    fileLanguage lang = FILE_LANGUAGE_ELS;
    if (!strcmp(current_smt_language, "c"))
      lang = FILE_LANGUAGE_C;
    else if (!strcmp(current_smt_language, "c++"))
      lang = FILE_LANGUAGE_CPP;
    else if (!strcmp(current_smt_language, "esql_c"))
      lang = FILE_LANGUAGE_ESQL_C;
    else if (!strcmp(current_smt_language, "esql_cpp"))
      lang = FILE_LANGUAGE_ESQL_CPP;
    else if (!strcmp(current_smt_language, "cpp") ||
	    (!strcmp(current_smt_language, "esql")))
      return 0; // does not mean anything, do no change language

    if (smt_header_static)
      smt_header_static->language = lang;
    if (proj_module_static) {
      proj_module_static->language(lang);
      if (current_smt_language != NULL && current_smt_language[0] != '\0') {
        symbolPtr def_file = proj_module_static->get_def_file_from_proj();
        ddElement *dd_file = def_file.get_def_dd();
        put_relation(property_of_possessor, dd_file, get_current_dd_language());
      }
    }
  }

  return 1;
}
