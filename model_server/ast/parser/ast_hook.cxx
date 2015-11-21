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
#include <cLibraryFunctions.h>

#define GCC_2_6
#include <machdep.h>
#include "genError.h"
#include <string.h>
#include <stdlib.h>

#include <ast_hook.h>
#include <astTree.h>


//extern astType *current_class_type;

#undef IF

#ifndef C_PARSER
#include "ast_cplus_tree.h"
#ifdef GCC_2_6
#include "parse.h"
#else
#include "cp-parse.h"
#endif /* GCC_2_6 */
#else
#include "c-parse.h"
#endif

//#include "genArr.h"

#include "ast_hook.h"
#include "ast_shared_hook.h"

//extern "C" char* getenv (char*);
static char* hook_gcc = OSapi_getenv ("HOOK_GCC");
static char* hook_debug = OSapi_getenv ("HOOK_DEBUG");
static char* hook_report = hook_debug ? hook_debug : OSapi_getenv ("HOOK_REPORT");

extern "C" astTree *lang_name_c, *current_lang_name;
extern "C" char ** tree_code_name;
extern "C" int reinstantiating_template();

static int ast_extern_lang;

// The following class maintains a ring of references to allow
// non-linear processing of various elements.  The only current
// use is for things like "void (*f)(int* p)"; the processing for
// "p" occurs before the processing for "f", meaning that a
// single variable for "last_indirect_ref" would get "f" wrong.
// The ring allows maintaining a history and searching for the
// particular reference in a given token range.
// -- wmm, 960422.

class ref_ring {
public:
   ref_ring();
   operator astTree*();
   astTree* operator->();
   astTree* operator()(int first_line, int last_line,
		       const char* ref_file);
   astTree* operator=(astTree* node);
private:
   enum { RING_SIZE = 64 };
   astTree* nodes[RING_SIZE];
   size_t cur_node;
};

ref_ring::ref_ring(): cur_node(0) {
   for (size_t i = 0; i < RING_SIZE; i++) {
      nodes[i] = NULL;
   }
}

ref_ring::operator astTree*() {
   return nodes[cur_node];
}

astTree* ref_ring::operator->() {
   return nodes[cur_node];
}

astTree* ref_ring::operator()(int first_line, int last_line,
			      const char* ref_file) {
   for (size_t i = 0; i < RING_SIZE; i++) {
      size_t ref_node = (cur_node - i) % RING_SIZE;
      astTree* ref = nodes[ref_node];
      int lineno = (ref) ? -ref->TREE_DEBUG() : -1;
      if (lineno >= first_line && lineno <= last_line) {
	 if (!ref->filen || !ref_file ||
	     (strcmp(ref->filen, ref_file) == 0)) {
	    cur_node = ref_node;
	    return ref;
	 }
      }
   }
   return NULL;
}

astTree* ref_ring::operator=(astTree* ref) {
   cur_node = (cur_node + 1) % RING_SIZE;
   nodes[cur_node] = ref;
   return ref;
}

// The ref_id_present class speeds up the search of ref_ids in ast_rep_ref_ids.
// The search is a linear search, which tends to find the correct id very
// quickly if present, but otherwise it takes a very long time looking
// through the array unsuccessfully.  The class is based on hashing.
// Lookup provides a single id node rather than a chain, to save space 
// and because multiple entries for a hash value occurring 1/8 of the
// time aren't a concern.  If lookup indicates multiple id nodes then the
// ref_ids array is searched.                   -- sturner, 980512
//
class ref_id_present {
public:
       ref_id_present();
       ~ref_id_present();
  void reset();
  void add(ident *);
  int lookup(int, char *, astIdentifier *, ident *&);
private:
  int hash(int, char *, astIdentifier *);
  void reallocate(int);
  void insert(ident *);
  
  ident **data;
  int data_size;
  int used_size;
  static ident multi_nid;
};

//static astTree * last_array_ref = 0;
ref_ring last_indirect_ref;
ref_id_present ref_ids_hash;   // sturner, 980511
static astTree * last_type_decl = 0;
static astTree * last_call_expr = 0;

#ifndef C_PARSER
static astTree * last_destructor_expr = 0;
#endif

static char* hook_file_name = 0;
#ifndef C_PARSER
static astTree * last_scope_ref = 0;
#endif

#ifndef JOINER
#define JOINER '.'
#endif

extern int yydebug;
static int hook_prt_init()
{
 int opt;
// if (hook_report) yydebug = 1;
 if(!hook_report || hook_report[0] == 0) 
    opt = 0;
 else if (strlen(hook_report) > 1){
    hook_file_name = hook_report;
    opt = 1;
 } else {
   opt = 2;
 }
 return opt;
}
int ast_hook_typedecl_is_def (astDecl *decl);
static int hook_prt_option = hook_prt_init();
static int set_len(int = 3);
extern "C" void lineprint (astTree*);
void lineprint (astTree *, int );
extern void ast_hook_prt(int nn);
extern "C" astTree * get_current_decl ();
extern "C" int no_init_hook;
#ifdef C_PARSER
int instantiate_member_template_var = 0;
#else
extern "C" int instantiate_member_template_var;
#endif
static int ast_processing_default_fn = 0;
static astDecl *global_traverse_expr = 0;
static char * trn(char * name);
static int test_prt(char * fn)
{
  if(!fn) return 1;
  int answer = 1;  
  if(hook_prt_option == 1){
    answer =  (strcmp(hook_file_name, trn(fn)) == 0);
  } else if(hook_prt_option == 2) {
    int len = strlen(fn);
    answer = (fn[len-1] != 'h');
  } 
  return answer;
}
static int test_prt(astTree*tt)
{
  return tt ? test_prt(TREE_FILENAME(tt)) : 0;
}

// extern char *input_filename;

#undef DBG
#define DBG(tt) if(hook_debug && test_prt(tt))

#define my_input_filename yylloc.text

static char * trn(char * name)
{
  if(!name) return name;
  char * nn = name;
  while(*(++name))
     if(*name == '/')
        nn = name + 1;
  return nn;
}

static int is_operator(astDecl * decl)
{
  astIdentifier * id = decl->DECL_ASSEMBLER_NAME();
  return id ? TREE_ASET_FLAG(id) : 0;
}
   
static treeArr decl_spec;
void ast_decl_spec_init() {decl_spec.reset();}
extern "C" void ast_specs_prt(){
  int sz = decl_spec.size();
  cout << endl << "------ declspecs ----- " << decl_spec.size() / 4 << endl;
  for(int ii=0; ii<sz; ii+=4){
   if(test_prt(*decl_spec[ii+3])){
    cout << (int) *decl_spec[ii+1] << ' ' << (int) *decl_spec[ii+2] << " == ";
    lineprint(*decl_spec[ii+3]);
   }
  }
}

static treeArr call_globals;
void ast_call_init() { call_globals.reset ();}

extern "C" void ast_calls_prt(){
  int sz = call_globals.size();
  cout << endl << "------ call_globals ----- " << call_globals.size() /3 << endl;
  cout << " caller current callee" << endl;
  int one_line = (hook_gcc) ? 0 :1;
  for(int ii=0; ii<sz; ii+=3){
   if(test_prt(*call_globals[ii])){
    cout << '[' << ii/3 << ']' << endl;
    lineprint(*call_globals[ii], one_line);
    lineprint(*call_globals[ii+1], one_line);
    lineprint(*call_globals[ii+2], 1);
   }
  }
}

static treeArr file_globals;
void ast_file_init() { file_globals.reset ();}

extern "C" void ast_globals_prt(){
  int sz = file_globals.size();
  cout << endl << "------ file_globals ----- " << file_globals.size() << endl;
  for(int ii=0; ii<sz; ii+=1){
    if(test_prt(*file_globals[ii]))
      lineprint(*file_globals[ii]);
  }
}


/* used to detect '=' in global var declaration
and process entities refered accordingly in ast_put_ref_ids.
Since getdecls() will return currently processed var decl in this case.
*/
static int explicit_var_init = 0;
extern "C" void set_explicit_var_init(int i)
{
  explicit_var_init = i;
}

int get_explicit_var_init()
{
  return explicit_var_init;
}


static treeArr tmp_global_vars;

extern "C" void process_global_vars()
{
  int sz = tmp_global_vars.size();
  for (int ii = 0; ii < sz; ii += 2) {
    astDecl * cgv = (astDecl *) *tmp_global_vars[ii];
    astDecl * decl = (astDecl *) *tmp_global_vars[ii+1];
    if (cgv && cgv->DECL_INITIAL() && TREE_ASM_WRITTEN(cgv)) {
      if ((cgv->DECL_INITIAL()->TREE_CODE() != ERROR_MARK) || 
	  /* to catch static global variables */
	  (TREE_PUBLIC(cgv) == 0 && cgv->TREE_TYPE() &&
	   AGGREGATE_TYPE_P(cgv->TREE_TYPE()))) {
	call_globals.append ((tree*)&decl);
	call_globals.append ((tree*)&cgv);
	call_globals.append ((tree*)&decl);
      }
    }
  }
}
#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype

#endif

extern "C" YYSTYPE yyval; 
extern "C" YYLTYPE yylloc;		
extern "C" int yychar;
extern "C" YYLTYPE *yyls;
extern "C" YYLTYPE *yylsp;
extern "C" int * yylen_ptr;
YYLTYPE yylsp_prev;

typedef
struct tinstLevel
{
  astTree *classname;
  int line;
  char *file;
  tinstLevel *next;
  YYLTYPE yylloc;
  YYLTYPE yylsp;
} tinstLevel;

#ifndef C_PARSER
static int ast_functional_cast = 0;
extern "C" void set_ast_functional_cast( int i)
{
  ast_functional_cast = i;
}
extern "C" int get_ast_functional_cast()
{
  return ast_functional_cast;
}
extern "C" void save_yylloc(tinstLevel *n)
{
  n->yylloc = yylloc; n->yylsp = *yylsp;
}
extern "C" void restore_yylloc(tinstLevel *n)
{
  yylloc = n->yylloc;
  yylsp_prev = n->yylsp;
}
extern "C" astTree *find_decl_list(astTree *);

extern "C" void set_new_val_declspecs(astTree * classname)
{
  if (classname) {
    astTree * tr = find_decl_list(classname);
    if (tr) {
      TREE_START_LINE(tr) = yylsp->first_line;
      TREE_END_LINE(tr) = yylsp->last_line;
    } else
	/* nested enums do not get "linenum" set: lookahead is not at ;,
	   and ast_semicolon_hook() does not cover nested entities. So, one
	   has to get ; after the type gets processed */
	if (last_type_decl!=0 && classname->TREE_CODE()==ENUMERAL_TYPE) {
	    if (last_type_decl->TREE_CODE() == TYPE_DECL) {
		astDecl * decl = (astDecl *)last_type_decl;
		if (decl->get_linenum()==0 &&
		    decl->TREE_TYPE()==classname &&
		    yylloc.first_column==';' && yylloc.first_line>0)

		    decl->set_linenum(yylloc.first_line);
	    }
	}
  }
}
extern "C" void save_yylsp(astTree * classname)
{
    if (classname && ((astIdentifier*)classname)->IDENTIFIER_TEMPLATE ())
      set_new_val_declspecs(classname);
    yylsp_prev = *yylsp;
    yylsp_prev.timestamp = -1;
}

extern "C" void ast_class_templ_specialization_hook(astDecl *d)
{
  if (d && (d->TREE_CODE() == TYPE_DECL) &&
      (yylsp[0].last_column == '{') && 
      (TREE_END_LINE(d) == yylsp[0].last_line)) {
    if (yylsp[-1].last_line >= TREE_START_LINE(d))
      TREE_END_LINE(d) = yylsp[-1].last_line;
  }
}

extern "C" void adjust_yylsp()
{
  if ((yylsp_prev.timestamp < 0) || !yylsp_prev.first_line || !yylsp_prev.last_line) return;
  int depth = set_len();
  if (depth == 0) return;
  for(int ii=0; ii<depth; ++ii)
      if ((yylsp[-ii].first_column == yylsp_prev.first_column) &&
	  (yylsp[-ii].timestamp == yylsp_prev.timestamp)) {
	  yylsp[-ii].last_column = yylsp_prev.last_column;
	  yylsp[-ii].last_line = yylsp_prev.last_line;
	  yylsp_prev.timestamp = -1;
	break;
      }
}


#else
extern "C" void adjust_yylsp() {}
#endif
extern "C" int ast_input_currently_not_valid()
{
  return ((yychar == -2) && (yylloc.first_line == 0));
}

extern "C" void ast_hook_adjust_nested_type_specs(astTree *cds, astTree * types)
{
    if (cds == 0 || types == 0) return;
    int end_l = TREE_END_LINE(types);
    if (end_l <= 0) return;
    int len = (*yylen_ptr >= 3)? set_len(*yylen_ptr): set_len();
    for (int ii = 0; ii < len; ++ii) {
	if ((yylsp[-ii].last_line == end_l) && (yylsp[-ii].last_column == '}')) {
	    TREE_START_LINE(cds) = yylsp[-ii].first_line;
	    TREE_END_LINE(cds) = yylsp[-ii].last_line;
	    break;
	}
    }
}

/* 960606 kit transue: blocks built by compstmtend don't know about their opening brace: */
extern "C" void ast_hook_adjust_compstmt_begin(astTree *block)
{
    if (block && (block->TREE_CODE() == BLOCK || block->TREE_CODE() == LET_STMT))
        for (int ii = 0; ii < *yylen_ptr; ++ii) {
            if (yylsp[-ii].first_column == '{') {
	        TREE_START_LINE(block) = yylsp[-ii].first_line;
	        break;
            }
        }
}

static int ast_declspec_is_extern(astTree* spec)
{
   static astIdentifier * id_extern = (astIdentifier *) get_identifier("extern");

   if (spec == 0 || spec->TREE_CODE() != TREE_LIST)   return 0;

   for (astList *t = (astList*)spec; t; t = (astList*)t->TREE_CHAIN())
	  if (t->TREE_VALUE () == id_extern)
 		  return 1;
   return 0;
}


static void put_declspecs(int np1, int np2, astTree* ds, astDecl* d)
{

// there is no significance to ds

  if(ds) TREE_FILENAME(ds) = NULL;
  int is_extern = 0;
  astTree* ddss = ds;
  if(ds) if(d->TREE_CODE()==VAR_DECL || d->TREE_CODE()==FUNCTION_DECL){
    is_extern = ast_declspec_is_extern(ds);
    if(! is_extern) ddss = 0;
  }
  decl_spec.append((tree*)&ddss);
  decl_spec.append((tree*)&np1);
  decl_spec.append((tree*)&np2);
  decl_spec.append((tree*)&d);

  DBG(d){
    cout << " ast_declspecs_hook " << np1 << ' ' << np2 << 
                   " extern " << is_extern << endl ;
    if(ds) lineprint(ds);
    if(d)  lineprint(d);
  }
}

#if 0
static char * old_hook = OSapi_getenv("HOOK_OLD");

inline int is_untokenized_file(char * fn)
 { return old_hook ? 0 : (fn[1] == '-');}
#else
inline int is_untokenized_file(char * fn)
 { return (fn[1] == '-');}
#endif
#define is_untokenized_tree(t) (!(t->start_line))
inline is_decl(tree_code code)
{
  return (code == FIELD_DECL || code == FUNCTION_DECL || code == VAR_DECL
	  || code == CONST_DECL || code == FIELD_DECL || code == TYPE_DECL);
}

inline is_generated_tree()
{
 return current_function_decl && is_untokenized_tree(current_function_decl);
}
static int ast_put_global_decl(astTree * decl)
{
// dont put macro-expansions
  int put_flag = 1;

  if(is_untokenized_tree(decl)) return 0;

#if 0
  if(
      (decl->start_line < decl->end_line)
       || (decl->start_line != yylloc.first_line)
     )
       file_globals.append(&decl);
  else 
       put_flag = 0;
#endif

// always append and filter out later on the aset side. 
	file_globals.append(&decl);
	DBG(decl){
	    cout << "ast_put_global_decl yylloc=";
	    cout << yylloc.first_line << " flag=" << put_flag << endl;
	    lineprint(decl);
	}
  return put_flag;
}

extern char * ast_type_str(int code);
extern "C" char * ast_typename(int code) {return ast_type_str(code);}
extern "C" int ast_is_global_level ();

static int set_len(int min_lev){
//  yylloc.text = input_filename;

  if(is_generated_tree()) return 0;
  if(! yylloc.text) return 0;
  if(is_untokenized_file(yylloc.text)) return 0;

  if(! yylen_ptr) return 0;
  if(! yylsp) return 0;
  if( (*yylen_ptr) < 0) return 0;

  int len = yylsp - yyls + 1;

//  typedef struct{} AAA,*AP; AP ap; len=2, *yylen_ptr = 3
//  if(len < *yylen_ptr) return 0;
  if(len < min_lev) return 0;   // 4 is an empiric const from mg 
  return len;
}

inline int set_len(astTree*ttt){
  return is_untokenized_tree(ttt) ? 0 : set_len();
}


  identArr     ref_ids;

void ast_init_ref_ids()
{
  ref_ids.reset();
  ref_ids_hash.reset(); // sturner, 980511
}

extern "C" void set_no_id_hook(int);
extern "C" void ast_set_no_id_hook ()
{
  set_no_id_hook(1);
}

int ast_could_be_ambiguous_ref(enum tree_code cod)
{
  return  (cod == FIELD_DECL || cod == CONST_DECL || cod == PARM_DECL || cod == VAR_DECL
	   || cod == FUNCTION_DECL || cod == TYPE_DECL);
}

#ifndef C_PARSER
#if 0 /* 05-09-95 * NY */
extern "C" astDecl * ast_get_current_static_data_member();
#else
extern "C" astList * ast_get_shadowed_to_hook ();
#endif /* 0 */
#endif /* ndef C_PARSER */

extern "C" astTree * getdecls();
extern "C" int errorcount;
extern "C" astDecl * ast_get_current_global_var_decl()
  /*
  probably will need to save these to then end of parsing,
  then check for error mark on each decl node then, instead of
  using errorcount.
  */
{
  astDecl *var_decl = 0;

  int glob = ast_is_global_level();
  if (!glob) {
#ifndef C_PARSER
#if 0 /* 05-09-95 * NY */
    var_decl = ast_get_current_static_data_member();
#else
    astDecl *vd;
    astList *li = ast_get_shadowed_to_hook ();
    if (li && (li->TREE_CODE () == TREE_LIST)) {
      vd = (astDecl*) li->TREE_PURPOSE ();
      if (vd && (vd->TREE_CODE () == IDENTIFIER_NODE)) {
	vd = (astDecl*) ((astIdentifier*)vd)->IDENTIFIER_CLASS_VALUE();
	if (vd && (vd->TREE_CODE () == VAR_DECL) &&
	    vd->DECL_CONTEXT () &&
	    vd->DECL_CONTEXT ()->TREE_CODE () == RECORD_TYPE)
	  var_decl = vd;
      }
    }
#endif /* 0 */
#endif /* ndef C_PARSER */
    glob = (var_decl != 0);
  }
  else
    var_decl = (astDecl *) getdecls();

  if (glob && var_decl && (var_decl->TREE_CODE() == VAR_DECL) &&
	var_decl->DECL_INITIAL() &&
	(var_decl->DECL_INITIAL()->TREE_CODE() == ERROR_MARK) &&
	(TREE_ASM_WRITTEN(var_decl) == 0) /* && (errorcount == 0) */ )
      return var_decl;
  return NULL;
}

int miss_in_tr_expr(astDecl *decl)
{
  if (decl && decl->TREE_CODE() == TYPE_DECL ||
      decl->TREE_CODE() == CONST_DECL )
    return 1;
  else if (decl && decl->TREE_CODE() == VAR_DECL &&
	   decl->DECL_INITIAL() &&
	   TREE_CONSTANT(decl->DECL_INITIAL()))
    return 1;
  return 0;
}

astDecl * need_to_post_process_var(astDecl *decl)
{
  astDecl *cgv = ast_get_current_global_var_decl();
  if (cgv && decl && miss_in_tr_expr(decl))
    return cgv;
  return 0;
}


 void ast_put_ref_ids( int lineno, char * filename,
		       astDecl *decl, astIdentifier *id)
{

   if(lineno==0){
#ifndef _ASET_OPTIMIZE
      cout << "ast_put_ref_ids: lineno==0 " << endl;
      lineprint(decl);
#endif
      return;
   }
#ifndef C_PARSER
   if (0/*processing_template_defn*/) return;
   if (instantiate_member_template_var) return;
   if (decl->TREE_CODE() == TYPE_DECL && decl->artificial_flag &&
       decl->ignored_flag) {
      // This is a constructed type for the fully-qualified name of a nested
      // type.  Here we try to map it back to the original decl.
      astTree* orig = decl->TREE_TYPE()->TYPE_NAME();
      if (orig && orig->TREE_CODE() == TYPE_DECL) {
	 decl = (astDecl*) orig;
      }
   }
#endif

   if ((decl->TREE_DEBUG() == 0) && TREE_START_LINE(decl) && TREE_END_LINE(decl) &&
#if 0 /* 10.25.95 NY */
       (decl->DECL_SOURCE_LINE() >= lineno))
#else
       (decl->get_linenum() >= lineno))
#endif
          decl->set_DEBUG(- lineno); 

#ifndef C_PARSER
  /* hooks for new parser some time get confused for field with same name
     assuming here the latter ones are better.
   */
  int ref_ids_size = ref_ids.size();
   if (ref_ids_size) {
     ident *old_id = ref_ids[ref_ids_size - 1];
     if (old_id && (old_id->lineno == lineno) && (old_id->id == id) &&
	 old_id->decl && ast_could_be_ambiguous_ref(old_id->decl->TREE_CODE()) &&
	 (old_id->cur_decl == (astDecl *) get_current_decl())) {
       old_id->decl = decl;
       return;
     }
   }
#endif
   
   ident * nid = ref_ids.grow(1);

   nid-> lineno = lineno;
   nid-> filename = filename;
   nid-> decl = decl;
   nid-> id = id;
   nid->cur_decl = (astDecl*)get_current_decl();
   ref_ids_hash.add(nid); // sturner, 980511

   /* capture casting to typedecl in global var init */
#ifndef C_PARSER
   if (get_explicit_var_init()) {
#endif
     astDecl *cgv = need_to_post_process_var(decl) ;
     if (cgv) {
       tmp_global_vars.append((tree *) &cgv);
       tmp_global_vars.append((tree *) &decl);
     }
#ifndef C_PARSER
   }
#endif
 }


extern "C" void ast_ref_ids_prt(){
  int sz = ref_ids.size();
  cout << endl << "------ ref_ids ----- " << ref_ids.size() << endl;
  for(int ii=0; ii<sz; ii++){
   if(test_prt(ref_ids[ii]->filename)){
    cout << ref_ids[ii]->lineno << ' ' << trn(ref_ids[ii]->filename) << " == ";
    lineprint(ref_ids[ii]->decl);
   }
  }
}

 int ast_rep_ref_ids( int lineno, char * filename,
	       astDecl *old_decl, astDecl *new_decl, astIdentifier *id)
{
   // Hash table helps avoid unnecessary linear search of ref_ids,
   // sturner, 980511
   ident *found_nid;
   int found_decls = ref_ids_hash.lookup(lineno, filename, id, found_nid);
   if (lineno != 0 && filename != 0 &&
          (found_decls == 0 || found_decls == 1 &&
#ifndef C_PARSER
	(found_nid->decl == new_decl)
#else
        (found_nid->decl != old_decl)
#endif
       ))
     return -1;

   int size = ref_ids.size();
   ident * nid;
   for(int ii = size-1; ii>=0; --ii){
      nid = ref_ids[ii];
      if(
           ((lineno==0) || (nid-> lineno == lineno))
        && ((filename==0) || (nid-> filename == filename))
#ifndef C_PARSER
	&& (nid->decl != new_decl)
#else
        && (nid-> decl == old_decl)
#endif
        && (nid-> id == id)
        ) {
#ifndef C_PARSER	
	if (nid->decl && nid->decl->TREE_CODE() == TEMPLATE_DECL)
	  return -1;
#endif	  
	  nid->decl = new_decl;
       break;
     }
   }
   return ii;
 }

  static    astIdentifier * cur_id_node = 0;
  static    int             cur_id_line = 0;
  static    astDecl       * cur_id_decl = 0;

// The following variable and functions are intended to allow callers of
// various functions to enable or disable setting the preceding three
// variables; at this writing, this capability is used only by
// make_anon_name() in calling ast_constructor_hook and init_attributes()
// when registering all the attribute keywords.

  static    bool	    saving_cur_id = true;

extern "C" void ast_disable_saving_cur_id() {
   saving_cur_id = false;
}

extern "C" void ast_enable_saving_cur_id() {
   saving_cur_id = true;
}

extern "C" void debug_cur_id() {
   cout << "cur_id_node=" << cur_id_node << ", cur_id_line=" <<
	 cur_id_line << ", cur_id_decl=" << cur_id_decl << endl;
}

  static    astTree        * cur_call = 0;

static void ast_update_id
   ( int id_line, astDecl *old_decl, astDecl *new_decl, int is_ref=0)
{
  astIdentifier * id = new_decl->ID_NODE();

  if(id_line==0){
     if(id == cur_id_node){
         id_line = cur_id_line;
     }
  }

  if(id_line==0){
     DBG(new_decl){
       cout << "ast_update_id: id_line==0 " << endl;
     }
     return;
  }

// set debug_info field to -id

  if(!is_ref)
      new_decl->set_DEBUG( -id_line);

  if(old_decl==0){
       ast_put_ref_ids(id_line, my_input_filename, new_decl, id);
  } else if( new_decl != old_decl ) {
       ast_rep_ref_ids(id_line, my_input_filename, old_decl, new_decl, id);
  }

  DBG(new_decl) {
    if(old_decl==0){
        cout << "put_id_hook " << id_line << ' ' << my_input_filename << 
                                            ' ' << new_decl << endl;
    } else if( new_decl != old_decl ) {
        cout << "rep_id_hook " << id_line << ' ' << my_input_filename << 
             " old " << old_decl << " new " << new_decl << endl;
    }
  }
}

static astTree * call_expr_of_fp_decl(astDecl*decl, astTree*tt)
{

  astType* tp = decl->type;
  astType*ptp = 0;

  if(tp->TREE_CODE() == POINTER_TYPE){
      ptp = tp->type;
  } 
 
  if(ptp && ptp->TREE_CODE() == FUNCTION_TYPE){
      if(tt && tt->TREE_CODE() == INDIRECT_REF){
         astTree* ttt = ((astExp1*)tt)->TREE_OPERAND(0);
         if(ttt && ttt->TREE_CODE() == CALL_EXPR)
             tt = ttt;
      }
  }
  return tt;
}

extern "C" void ast_hook_ref_ids (astDecl *) // decl)
{
//  astIdentifier *id = decl->ID_NODE ();
//  ast_put_ref_ids( TREE_END_LINE (decl), TREE_FILENAME (decl), decl, id);
}

static int is_unknown(int type)
{
  return  
      (type > 400)
   || (type == 0)
  ;
}

static int is_id(int type)
{
 return (
     (type == IDENTIFIER)
       );
}

static int is_name(int type)
{
 return (
     (type == IDENTIFIER)
  || (type == TYPENAME)
#ifndef C_PARSER
#ifndef GCC_2_6
  || (type == SCOPED_TYPENAME)
#endif /* GCC_2_6 */
#endif
       );
}

static bool ignore_lookahead = false;

extern "C" void ast_ignore_lookahead() {
   ignore_lookahead = true;
}

extern "C" void ast_resume_lookahead() {
   ignore_lookahead = false;
}

static int ast_find_id_from_depth(int depth)
{

  if (yylloc.first_column == '>') {
      // This is a template id, which causes problems for the
      // processing below in the case where an id is used in the
      // template argument list (e.g., "foo<bar>" will give the
      // line number of "bar" instead of "foo".  The best we can
      // do is emulate the processing of set_lineno().
      if (*yylen_ptr) {
	  return yylsp[1 - *yylen_ptr].first_line;
      }
      return yylsp[0].first_line;
  }

  if(!ignore_lookahead && is_id(yylloc.first_column)) {
    if(is_id(yylsp[0].last_column))     //  struct A    aa;
       return yylsp[0].last_line;       //        lsp   lloc
    else if (is_id(yylsp[0].first_column))     //  struct A    aa;
       return yylsp[0].first_line;       //        lsp   lloc
    else
       return yylloc.first_line;
  }

  if(is_unknown(yylloc.first_column))
       return yylloc.first_line;

  for(int ii=0; ii<depth; ++ii)
    if(is_id(yylsp[-ii].last_column))
       return yylsp[-ii].last_line;
    else if(is_id(yylsp[-ii].first_column))
       return yylsp[-ii].first_line;

// FAIL: sometimes yyloc.first_line is still ID
//  return yylloc.first_line;
  return 0;

}

static int ast_find_id(int depth = 2, astIdentifier * id = 0)
{
  if(!id) if(depth > 2) depth = 3;
  if (ignore_lookahead && depth == 0) depth = 2;

  return ast_find_id_from_depth(depth);
}

extern "C" void ast_update_id_line(astTree * td)
{

  if (td == 0 || td->TREE_DEBUG()) return;
  
  int id_line = ast_find_id(*yylen_ptr, 0);
  if (id_line)
    td->set_DEBUG(-id_line);
}


static int ast_find_name(int depth = 2)
{
  if(depth > 2) depth = 3;
  
  if(is_name(yylloc.first_column))
    if(is_name(yylsp[0].first_column))     //  struct A    aa;
       return yylsp[0].first_line;       //        lsp   lloc
    else
       return yylloc.first_line;

  if(is_unknown(yylloc.first_column))
       return yylloc.first_line;

  for(int ii=0; ii<depth; ++ii)
    if(is_name(yylsp[-ii].last_column))
       return yylsp[-ii].last_line;
    else if(is_name(yylsp[-ii].first_column))
       return yylsp[-ii].first_line;

// FAIL: sometimes yyloc.first_line is still ID
//  return yylloc.first_line;
  return 0;
}

static int ast_find_function_id()
{
  if (cur_call) {
#ifndef C_PARSER
    if (last_scope_ref && (((astExp1*)cur_call)->TREE_OPERAND(0) == last_scope_ref))
      return last_scope_ref->end_line;
#endif
      return cur_call->start_line;
  }
  return 0;
#if 0

  int depth = *yylen_ptr + 1;

  for(int ii=0; ii<depth; ++ii)
    if(is_id(yylsp[-ii].first_column))
       return yylsp[-ii].first_line;
// FAIL
  return 0;
#endif
}

static int is_typespec(int val){
  return
      (val == TYPESPEC)
    ||(val == TYPE_QUAL)
//// was here when called by ajust_var_decl    ||(val == TYPENAME)
////#ifndef C_PARSER
////    ||(val == SCOPED_TYPENAME)
////#endif
#ifndef C_PARSER
    ||(val == AGGR)
#else 
    ||(val == STRUCT)
    ||(val == UNION)
#endif
    ||(val == SCSPEC)
/// was here when called by ajust_var_decl()   ||(val == IDENTIFIER)
  ;
}

static int is_rec_type(int val){
  return
#ifndef C_PARSER
    (val == AGGR)
#else 
    (val == STRUCT)
    ||(val == UNION)
#endif
  ;
}

#ifndef C_PARSER
static void ajust_call_expr (int /* len */, astTree* node) 
{
    if(yylsp[0].last_column == LEFT_RIGHT){  // steDocoment::steDocument(){}
           node->end_line = yylsp[0].last_line;
           if(yylsp[0].first_column == IDENTIFIER)
              node->start_line = yylsp[0].first_line;
           else
              node->start_line = yylsp[-1].last_line;
           DBG(node)
                {
                cout << " ast_ajust_call_expr 2" << endl;
                lineprint(node);
                ast_hook_prt(2);
                }
     } else  if(yylsp[0].last_column == ')'){
         if(yylsp[0].first_column == '(') {
             node->start_line = yylsp[0].first_line;
         } else {
	   /* !! < T.D 09.17.94 */
	   int guess_start_line = 0;
	   for (int ce = -3; ce >= -6; --ce) {
	     if (yylsp[ce].first_column == '(' || (yylsp[ce].last_column == '('))
	       ;
	     else {
	       if ((yylsp[ce].last_column == IDENTIFIER) &&
		   (yylsp[ce].first_column == SCOPE))
		 node->start_line = yylsp[ce].first_line;
	       else if (yylsp[ce].last_column == ';')
		 ;
	       else
		 node->start_line = yylsp[ce].last_line;
	       break;
	     }
	   }
             DBG(node)
                {
                cout << " ast_ajust_call_expr 4" << endl;
                lineprint(node);
                ast_hook_prt(4);
                }
         }
     } else DBG(node) {
       cout << "Error in ast_constructor_hook with call_expr" << endl;
     }
}
#endif

#ifndef C_PARSER
static int ast_find_call_id (int nn, astTree * exp) 
{
  
  if(nn==0) {
    for (int ce = -3; ce >= -6; --ce) {
      if ((exp->start_line == yylsp[ce].first_line) &&
	  ((yylsp[ce].last_column == IDENTIFIER) && (yylsp[ce].first_column == SCOPE)))
	  return yylsp[ce].last_line;
    }
    return exp->start_line;
  }

  if(yylsp[-(nn-1)].first_column == IDENTIFIER) // member function
     --nn;
  /* x.operator ++() */
  else if ((yylsp[-(nn-1)].first_column == OPERATOR) && (yylsp[-(nn-1)].last_column == TYPESPEC))
    return yylsp[-(nn-1)].last_line;

  if(yylsp[-(nn)].last_column == SCOPE)
    --nn;
  /* xp->X::~X() */
  if ((yylsp[-(nn)].last_column == IDENTIFIER) &&
      (yylsp[-(nn)].first_column == IDENTIFIER) &&
      (yylsp[-(nn)].last_line > TREE_START_LINE(exp)) &&
      (yylsp[-(nn)].last_line < TREE_END_LINE(exp)) &&      
      (yylsp[-(nn)].last_line > yylsp[-(nn)].first_line))
    return yylsp[-(nn)].last_line;

  /*
   * inline constructor
   * inline AA(){}
   */
  if ((yylsp[-nn].last_column == IDENTIFIER) &&
      (yylsp[-nn].first_column == SCSPEC)) {
      TREE_START_LINE(exp) = yylsp[-nn].last_line;
      return yylsp[-nn].last_line;
  }

  return yylsp[-(nn)].first_line;
}
#else
static int ast_find_call_id (int , astTree * exp) 
{
  return exp->start_line;
}
#endif


#ifndef C_PARSER
#define SOME_COLUMN last_column
#else
#define SOME_COLUMN first_column
#endif

static void ajust_var_decl(int len, astTree* node){
// moves back start_line from IDENTIFIER to TYPESPEC
    int   start_ind = (*yylen_ptr) - 1;
    if(start_ind < 0) start_ind = 0;
    for(int ii=start_ind; ii<len-2; ++ii){
           if(yylsp[-ii].first_column == IDENTIFIER){
	           node->start_line = yylsp[-ii].first_line;
                   if(yylsp[-(1+ii)].last_column == ';') break;
                   if(yylsp[-(1+ii)].first_column == IDENTIFIER) continue;

                   if(
                         is_typespec(yylsp[-(1+ii)].first_column)
                     &&  is_typespec(yylsp[-(1+ii)].last_column)
                     )
                            node->start_line = yylsp[-(1+ii)].first_line;
                   else
                            node->start_line = yylsp[-ii].first_line;
                   break;
           } else if(is_typespec(yylsp[-ii].first_column)
                      ||(yylsp[-ii].first_column == '~')){    // destructor
                   node->start_line = yylsp[-ii].first_line;
                   break;
           }
     }
}

// moves back start_line from IDENTIFIER to EXTERN 
#ifndef C_PARSER
static void ajust_func_decl(int len, astTree* node)
  {
    int   start_ind = (*yylen_ptr) - 1;
    if(start_ind < 0) start_ind = 0;

// destructors are correct
    
    if(yylsp[-start_ind].first_column == '~') 
                 return;

    if(yylsp[-start_ind].first_column == SCSPEC) 
         if(yylsp[-(start_ind-1)].first_column == '~') 
              return;

    for(int ii=start_ind; ii<len-1; ++ii){
           if(yylsp[-ii].last_column == EXTERN_LANG_STRING ){
                 node->start_line = yylsp[-ii].last_line - 1;
//                   break;
                 return;
           }
     }

  }
#else
static void ajust_func_decl(int , astTree*)
{}
#endif

static void set_lineno(astTree* node){
  if(*yylen_ptr != 0){
//    if (yylsp[0].last_line >= yylsp[1-*yylen_ptr].first_line) {
      node->start_line = yylsp[1-*yylen_ptr].first_line;
      node->end_line = yylsp[0].last_line;
//    }
    node->filen = yylsp[0].text;
  } else {
     node->start_line = yylsp[0].first_line;
     node->end_line = yylsp[0].last_line;
     node->filen = yylloc.text;
  }


}

static int list_separator(int col)
{
 return
     (col == ')') || (col == ',') || (col == ';')
 ;
}
inline int is_separator(int col)
{
 return
     (col == ',') || (col == ';')
 ;
}

static int decl_separator(int col)
{
 return
     (col == ',') || (col == ';')
 ;
}

static astTree* ds_templ = 0;
extern "C" void set_declspecs_hook_template(astTree *ds)
{
#ifndef C_PARSER
  if (0/*processing_template_defn*/) return;
#endif
  ds_templ = ds;
}

extern "C" astIdentifier *get_identifier_template(astType *dtype)
{
  if (!dtype) return 0;
  astDecl *decl = (astDecl*) dtype->TYPE_NAME();
  astIdentifier *arg = (astIdentifier *) ((decl)?decl->DECL_ARGUMENTS():0);
  if (arg && (arg->TREE_CODE() == IDENTIFIER_NODE) &&
      arg->IDENTIFIER_TEMPLATE())
    return arg;
  return 0;
}

astTree* possible_template_declspec(astTree *ds)
{
  if (ds && (ds->TREE_CODE() == TREE_LIST)) {
    astTree * val = ((astList*)ds)->TREE_VALUE();
    if (val && (val->TREE_CODE() == RECORD_TYPE))
      return get_identifier_template((astType*) val);
  }
  return 0;
}

astTree* get_declspecs_hook(astTree* ds)
{
  if (
#ifndef C_PARSER
     0/*processing_template_defn*/ ||
#endif
!ds ||
      (ds->TREE_CODE() != TREE_LIST)) return 0;
  if (ds_templ && possible_template_declspec(ds) == ((astList*)ds_templ)->TREE_VALUE())
    return ds_templ;
  else
    return 0;
}

extern "C" int is_declspec_id(int col)
{
  return ((col == IDENTIFIER) || (col == '>') || 
#ifndef C_PARSER
	(col == EXTERN_LANG_STRING) ||
	(col == AGGR) ||
#endif
	  (col == TYPESPEC) || (col == SCSPEC) || 
	  (col == ENUM) || (col == TYPE_QUAL) );
}
#ifndef C_PARSER

extern "C" int is_real_declspec_id_start(int col)
{
  return ( (col == EXTERN_LANG_STRING) ||
	(col == AGGR) ||
	  (col == TYPESPEC) || (col == SCSPEC) || 
	  (col == ENUM) || (col == TYPE_QUAL) );
}

extern "C" int illegal_declspec_id_end(int col)
{
  return (col == TYPE_QUAL);
}

static int ast_adjust_declspecs(int &st, int &en, int cur)
{
  if ((yylsp[-cur].first_line != st) || (yylsp[-cur].last_line != en)) return cur;
  if ((yylsp[-(cur + 1)].first_line == yylsp[-(cur + 1)].last_line) &&
      (yylsp[-(cur + 1)].first_column == TYPESPEC)) {
    st = en = yylsp[-(cur + 1)].first_line;
    cur++;
  }
  if ((yylsp[-(cur + 1)].first_line == st) && (yylsp[-(cur + 1)].first_column == AGGR) &&
      (yylsp[-(cur + 1)].last_column == EXTERN_LANG_STRING));
  else if (yylsp[-(cur + 1)].first_line != en) return cur;
  int len = (*yylen_ptr >= 3)? set_len(*yylen_ptr): set_len();
  while (cur <= len) {
    if ((yylsp[-(cur +1)].last_column == EXTERN_LANG_STRING) &&
	is_declspec_id(yylsp[-(cur + 1)].first_column)) {
      st = yylsp[-(cur + 1)].first_line;
      cur ++;
    }
    else
      break;
  }
  return cur;
}

int is_cons_or_destructor(astFunction_decl *fdecl)
{
 
  return ((fdecl->TREE_CODE() == FUNCTION_DECL) && 
          (DECL_CONSTRUCTOR_P(fdecl) || 
           DESTRUCTOR_NAME_P(fdecl->DECL_ASSEMBLER_NAME())));
}

int is_scspec(int ln)
{
  int depth = (*yylen_ptr);
  for (int i = 0; i < depth; ++i) {
    if ((yylsp[-i].first_line == ln) &&  (yylsp[-i].first_column == SCSPEC))
      return 1;
    else if ((yylsp[-i].last_line == ln) && (yylsp[-i].last_column == SCSPEC))
      return 1;
  }	
  return 0;
}

#endif
static int ast_declspecs_is_wrong(int st, int en)
{
  if (st == 0 || en == 0) return 0;
  int depth =  (*yylen_ptr >= 3)? set_len(*yylen_ptr): set_len();
  for(int ii=0; ii < depth ; ++ii)
    if (((yylsp[-ii].first_line == st) && 
	 is_declspec_id(yylsp[-ii].first_column)) &&
	((yylsp[-ii].last_line == en) &&
	 (is_declspec_id(yylsp[-ii].last_column) || yylsp[-ii].last_column == '}')))
      return 0;
  return 1;
}


static int ast_find_declspecs(int &st, int & en, int general)
{
  st = en = 0;
  int depth = (general) ? *yylen_ptr -1 : set_len();
  if (!general) {
    for(int ii=0; ii < depth; ++ii) {
      if(is_declspec_id(yylsp[-ii].first_column) &&
	 (yylsp[-ii].last_column == '>')) {
	st = yylsp[-ii].first_line;
	en = yylsp[-ii].last_line;
	return ii;
      }
    }
  }
  else {
    depth = (*yylen_ptr >= 3)? set_len(*yylen_ptr): set_len();
    for(int ii=0; ii <= depth; ++ii) {
      if(is_declspec_id(yylsp[-ii].first_column) &&
	 is_declspec_id(yylsp[-ii].last_column) 
#ifndef C_PARSER
	 && (!illegal_declspec_id_end(yylsp[-ii].last_column))
#endif
	 ) {
	st = yylsp[-ii].first_line;
	en = yylsp[-ii].last_line;

#ifndef C_PARSER
        if (yylsp[-ii].first_line == yylsp[-ii].last_line) {
	  if (((yylsp[-(ii+1)].first_column == AGGR) || (yylsp[-(ii+1)].first_column == ENUM)) &&
	      (yylsp[-(ii+1)].last_column == IDENTIFIER)) {
	    st = yylsp[-(ii + 1)].first_line;
	    en = yylsp[-(ii + 1)].last_line;
	  }
	}
	else if ((yylsp[-ii].first_column == IDENTIFIER) &&
		 (yylsp[-ii].last_column == IDENTIFIER) &&
		 (yylsp[-ii].last_line != yylsp[-ii].first_line) &&
		 is_declspec_id(yylsp[-(ii+1)].first_column) &&
		 is_declspec_id(yylsp[-ii].last_column)) {
	  st = yylsp[-(ii + 1)].first_line;
	  en = yylsp[-(ii + 1)].last_line;
	}
	ast_adjust_declspecs(st, en, ii);
	break;
#endif
      }
    }
    return (ii <= depth)?ii:0;
  }
  return 0;
}


extern "C" void set_processing_default_fn(int i)
{
  ast_processing_default_fn = i;
}

extern "C" int get_processing_default_fn()
{
  return ast_processing_default_fn;
}

extern "C" void ast_declspecs_hook(astTree* ds, astDecl* d)
{

  if(is_untokenized_tree(d)) return;

  int np1, np2;

  if(ds){
     np1 = TREE_START_LINE(ds);
     np2 = TREE_END_LINE(ds);
  } else {
     np1 = 0;
     np2 = 0;
  }
#ifndef C_PARSER

  if (0/*processing_template_defn*/) return;
  if (instantiate_member_template_var) return;
#if 0 /* 960611 kit transue: use artificial flag: */
  if (d->DECL_SOURCE_LINE() == 0) return; // DECL_ARTIFICIAL(d);
#else
  if (d->artificial_flag) return; // 
#endif /* 960611 */
//
// Do not generate hooks for default member functions (constructors, 
// copy, etc.).
//
  if (get_processing_default_fn()) return;

  if (ast_extern_lang) {
      int nn = *yylen_ptr;
      if(nn > 0){
	  int fc = yylsp[-nn].first_column;
	  if((fc==IDENTIFIER) || is_typespec(fc)){
	      ++nn;
	      if(yylsp[-nn].first_column==EXTERN_LANG_STRING) {
		  np1 = yylsp[-nn].first_line;
		  astTree * next = ds->TREE_CHAIN();
		  if(next)
		      np2 = next->end_line;
		  else
		      np2 = yylsp[-nn].last_line;
	      }
	  }
      }
  }
  if (ds && (ds->TREE_CODE() == TREE_LIST)) {
    astIdentifier * idn = (astIdentifier *) ((astList*) ds)->TREE_VALUE();
    if (idn && idn->TREE_CODE() == IDENTIFIER_NODE)
      if (idn->IDENTIFIER_TEMPLATE()) {
        int st, en;
	ast_find_declspecs(st, en, 0);
	if (st && en) {
	  np1 = st;
	  np2 = en;
        }
      } else if (d->TREE_CODE () == FUNCTION_DECL) {
	astTree *trrr = lookup_name (idn, -2);
	if (trrr && trrr->TREE_CODE () == TYPE_DECL) {
	  astTree * type1 = trrr->TREE_TYPE();
	  if (type1 && type1->TREE_CODE () == FUNCTION_TYPE) {
	      /*
	       * typedef int (FType) ();
               * FType foo;
	       */
	    TREE_START_LINE (d) = TREE_END_LINE (d);
	    d->set_DEBUG (-TREE_END_LINE (d));
	  }
	}
      }
  }
  
  int declspecs_is_wrong = 0;
  
  if ((d->TREE_CODE() == FUNCTION_DECL) && is_cons_or_destructor((astFunction_decl *)d) &&
      (np1  == np2) && is_scspec(np1)) {
  }
  else if (ast_declspecs_is_wrong(np1, np2)) {
    declspecs_is_wrong = 1;
    int st, en;
    int ii = ast_find_declspecs(st, en, 1);
    if (st && en) {
      if ((d->TREE_CODE() == FIELD_DECL) && (en >= TREE_START_LINE(d)) && 
	  is_declspec_id(yylsp[-(ii+1)].first_column) &&
	  is_declspec_id(yylsp[-(ii+1)].last_column)) {
	st = yylsp[-(ii+1)].first_line;
	en = yylsp[-(ii+1)].last_line;
      }

      np1 = st;
      np2 = en;
      if (ds) {
	TREE_START_LINE(ds) = st;
	TREE_END_LINE(ds) = en;
      }
    }
    else {
      np1 = np2 = 0;
    }
  }
  if (ds && (d->TREE_CODE() == VAR_DECL) && (np2 < np1)) {
    astTree* dds = get_declspecs_hook(ds);
    if (dds && (TREE_START_LINE(ds) == TREE_START_LINE(dds)) ) {
      np2 = TREE_END_LINE(dds);
      TREE_END_LINE(ds) = np2;
    }
  }
  if (ds && declspecs_is_wrong &&
      (d->TREE_CODE() == VAR_DECL) && (TREE_START_LINE(d) <= np2)) {
    astType* dtype = d->TREE_TYPE();
    if ((np1 == yylsp_prev.first_line) && yylsp_prev.last_line)
	np2 = TREE_END_LINE(ds) = yylsp_prev.last_line;
  }
  if (ds && declspecs_is_wrong &&
      (d->TREE_CODE() == FUNCTION_DECL) && (TREE_START_LINE(d) <= np2)) {
    if (is_declspec_id(yylsp[-2].last_column) && 
	is_declspec_id(yylsp[-2].first_column)) {
      TREE_START_LINE(ds) = np1 = yylsp[-2].first_line;
      TREE_END_LINE(ds) = np2 = yylsp[-2].last_line;      
    }
  }
#endif
#ifndef C_PARSER
  if ((np1 > np2) || (declspecs_is_wrong && (np2 > TREE_START_LINE(d))))
#else
  if (np1 > np2)
#endif
    put_declspecs(0, 0, ds, d);
  else
    put_declspecs(np1, np2, ds, d);
}


int token_is_id(int id_line)
{
  int len = set_len(*yylen_ptr);
  for (int ii = 0; ii < len; ++ii) {
    if(is_id(yylsp[-ii].last_column) && (id_line == yylsp[-ii].last_line))
      return 1;
    else if(is_id(yylsp[-ii].first_column) && (id_line == yylsp[-ii].first_line))
      return 1;
  }
  return 0;
}

static int call_from_toplevi = 0;
void set_call_from_toplevi() { call_from_toplevi = 1;}
void unset_call_from_toplevi() { call_from_toplevi = 0;}
int get_call_from_toplevi() {return call_from_toplevi;}

static astExp3 * last_caller;
static int global_thing_len = strlen("_GLOBAL_");
inline int is_global_call()
{
#ifndef C_PARSER
  int ret = 0;
  if (current_function_decl) {
    astIdentifier * idn = current_function_decl->DECL_NAME();
    char * name = idn->IDENTIFIER_POINTER();
    if (idn && (idn->IDENTIFIER_LENGTH() >= global_thing_len) &&
	name[global_thing_len] == JOINER &&
	!strncmp(name, "_GLOBAL_", global_thing_len))
      ret = 1;
  }
  else
    ret = get_call_from_toplevi();
  return ret;
#else
  return get_call_from_toplevi();
#endif
}

extern "C" void ast_set_traverse_expr_hook(astDecl *decl)
{
  global_traverse_expr = decl;
}

astDecl *get_global_traverse_expr()
{
  return global_traverse_expr;
}


extern "C" void traverse_expr(astTree *);
extern "C" int which_parser();
extern "C" void ast_toplevi_traverse_expr(astDecl * decl)
{
  if (decl->TREE_CODE() == VAR_DECL
#if 0
      && (which_parser() == 0 ||
       !(decl->TREE_TYPE() && AGGREGATE_TYPE_P(decl->TREE_TYPE())))
#endif
      ) {
    if (decl->DECL_INITIAL()) {
      set_call_from_toplevi();
      ast_set_traverse_expr_hook(decl);
      if (decl->DECL_INITIAL()->TREE_CODE() != ERROR_MARK)
	traverse_expr(decl->DECL_INITIAL());
#if 0
      if (DECL_CONSTRUCTOR(decl))
	traverse_expr(DECL_CONSTRUCTOR(decl));
#endif
      ast_set_traverse_expr_hook(0);
      unset_call_from_toplevi();
    }
  }
}

extern "C" void ast_call_hook( astExp3 *caller, astDecl *callee,
                                                            astDecl *id_decl )
{

  int len = set_len();
  astDecl * call_context = current_function_decl;
  if(len==0) {
    if (!is_global_call()) return;
    astDecl * td = get_global_traverse_expr();
    if (td && td->TREE_CODE() == VAR_DECL)
      call_context = get_global_traverse_expr();
  }
  if (callee->TREE_CODE() != FUNCTION_DECL) return;
/* 960611 kit transue.  Want to avoid registering calls twice, but still
    want to do any adjustments, so we move this to later:
  if (caller == last_caller) return;
  last_caller = caller;
*/

  astDecl *old_decl = id_decl;

// call id_hook for implicit declaration
   if(TREE_START_LINE(callee) == 0)
      if(TREE_END_LINE(callee) != -1){ 	// for optimiziation
         TREE_END_LINE(callee) = -1;
         id_decl = 0;
      }

  if(
      (TREE_START_LINE(caller) == 0) ||
      (TREE_START_LINE(caller) == 1) ){
// implicit constructor of base class 
      TREE_START_LINE(caller) = 0;
      TREE_END_LINE(caller) = 0;
      caller->set_DEBUG(0);
  } else {

// update id
     int id_line = - caller->TREE_DEBUG(); // ast_find_id();

    
     astDecl * new_decl = callee;
     astIdentifier * id = new_decl->ID_NODE();

     if (id_line > 0) {
#ifndef C_PARSER
       char * name = (id) ? id->IDENTIFIER_POINTER() :0;
       if (name && (name[0] == '_') && (name[1] == '_') && token_is_id(id_line)) {
	 caller->set_DEBUG(0);
       }
       else
#endif	 
#ifndef C_PARSER
       if(!id_decl){
#if 0 /* 10.25.95 * NY */
	 if (new_decl && new_decl->DECL_SOURCE_LINE()) { // not decl_artificial
#else
	 if (new_decl && new_decl->get_linenum()) { // not decl_artificial
#endif
           int rep = -1;
           if (old_decl && (new_decl != old_decl))
	     rep = ast_rep_ref_ids(id_line, my_input_filename, old_decl, new_decl, id);
           if (rep < 0) /* no repeat */
	     ast_put_ref_ids(id_line, my_input_filename, new_decl, id);
         }
#else
       if(id_decl == 0){
	 ast_put_ref_ids(id_line, my_input_filename, new_decl, id);
#endif
       } else if( new_decl != old_decl ) {
#ifndef C_PARSER
#if 0 /* 10.25.95 * NY */
	 if (new_decl && new_decl->DECL_SOURCE_LINE()) // not decl_artificial
#else
	 if (new_decl && new_decl->get_linenum()) // not decl_artificial
#endif /* 0 */
#endif
	   ast_rep_ref_ids(id_line, my_input_filename, old_decl, new_decl, id);
       }
     }
  }

  // TREE_OPERAND(caller, 1) == CONVERT_EXPR , then zeros lineno
/* 960611 kit transue.  Now that any adjustments are made, make sure we don't
    register the call twice: */
  if (caller == last_caller) return;
  last_caller = caller;
/* end 960611 */
  call_globals.append ((tree*)&caller);
  call_globals.append ((tree*)&call_context);
  call_globals.append ((tree*)&callee);

  DBG(caller){
     cout << endl << " ast_call_hook: current caller callee old_decl:" << endl;
     lineprint(call_context);
     lineprint(caller);
     lineprint(callee);
     lineprint(id_decl);
     ast_hook_prt(*yylen_ptr + 2);
  }
}

extern "C" void ast_base_init_hook
 (astType* base, astExp3 *caller, astDecl *callee, astDecl *id_decl)
{

  if(TREE_START_LINE(caller) == 1){
          TREE_START_LINE(caller) = 0;
  }
  /* 960611 kit transue: the 2.7.2 parser has a habit of saying that the
  caller is the rparen of the initializer.  (It used to be zero).  This is
  OK for navigation, but wreaks havoc with change prop.  So, we'll change
  it to zero here.  For lack of enough data, consider a one token call
  sufficiently suspicious.  */
  if(TREE_START_LINE(caller) == yylsp[0].last_line &&
  	TREE_START_LINE(caller) == TREE_END_LINE(caller)) {
     TREE_START_LINE(caller) = 0;
  } /* 960611 */
  DBG(current_function_decl){
     cout << endl << " ast_base_init_hook: " << endl;
     lineprint(caller);
     lineprint(base);
  }

/* !! T.D. 10.22.94
   id is ambiguous bw constructor and field or base class init, decl18.C 
   */
  caller->set_DEBUG(- TREE_START_LINE(caller));
/*  caller->set_DEBUG(0); */
/* !! T.D. 10.22.94  */
  
  if(TREE_FILENAME(caller)==0)
     TREE_FILENAME(caller) = yylloc.text;

  ast_call_hook(caller, callee, id_decl);
}

extern "C" void ast_destructor_call_hook( astExp3 *caller, astDecl *callee,
                                                            astTree* block )
{

  int len = set_len();
  if(len==0) return;
  caller->start_line = caller->end_line = 0;

  call_globals.append ((tree*)&caller);
  call_globals.append ((tree*)&current_function_decl);
  call_globals.append ((tree*)&callee);

  DBG(caller){
     cout << endl << " ast_destructor_call_hook: current caller callee block:"
          << endl;
     lineprint(current_function_decl);
     lineprint(caller);
     lineprint(callee);
     lineprint(block);
     ast_hook_prt(*yylen_ptr + 2);
  }
}

static int check_op_id(int code, int pos)
{
  pos = -pos;
  if(yylsp[pos].first_column == code)
     return yylsp[pos].first_line;
  else if(yylsp[pos].last_column == code)
     return yylsp[pos].last_line;
  else
     return 0;
}

static int find_op_id(int code, int pos, int arity)
{
  int len = *yylen_ptr - 1;
  if(len < arity ) len = arity;
  
  int line = check_op_id(code, len - pos);
  if(line) return line;

  for(int ii = 0; ii<len; ++ii) {
     line = check_op_id(code, ii);
     if(line) return line;
  }
  return 0;
}
#ifndef C_PARSER

extern "C" astTree* ansi_opname[];
static void set_op_id(astTree*call_expr, int code, int pos, int arity)
{
  int lex_code = 0;

  if (call_expr == 0)
    return;
  if(arity==0) {  // assign expr
     arity = 2;
     if(code==NOP_EXPR)
        lex_code = '=';
     else
        lex_code = ASSIGN;
  } else {
     lex_code =  - ansi_opname[code] -> TREE_DEBUG();
  }

  int line = find_op_id(lex_code, pos, arity);
  if((line == 0) && (lex_code == '='))
     line = find_op_id(ASSIGN, pos, arity);
  call_expr->set_DEBUG(-line);
  TREE_ASET_FLAG(call_expr) = 1;

 DBG(call_expr) {
   cout << " ast_op_hook " << line << ' ' << tree_code_name[code] << ' '
        << ast_type_str(lex_code) << " ar " << arity << " pos " << pos << endl;
   lineprint(call_expr);
   ast_hook_prt(*yylen_ptr + 2);
 }
}

extern "C" void ast_op_new_hook(astTree * call_expr, int code)
{
 int pos = 0;
 int arity = 2;
 
 set_op_id(call_expr, code, pos, arity);
}

extern "C" void ast_op_delete_hook(astTree * call_expr, int code)
{

 int pos = 0;
 int arity = 1;

 set_op_id(call_expr, code, pos, arity);
}

extern "C" void ast_op_post_hook(astTree * call_expr, int code)
{
 int pos = 1;
 int arity = 1;

 set_op_id(call_expr, code, pos, arity);
}

extern "C" void ast_op_unary_hook(astTree * call_expr, int code)
{
 int pos = 0;
 int arity = 1;

 set_op_id(call_expr, code, pos, arity);
}
extern "C" void ast_op_binary_hook(astTree * call_expr, int code)
{
 int pos = 1;
 int arity = 2;

 set_op_id(call_expr, code, pos, arity);
}

extern "C" void ast_op_assign_hook(astTree * call_expr, int code)
{
 int pos = 1;
 int arity = 0;

 set_op_id(call_expr, code, pos, arity);
}
#endif

static void semicolon_type_hook(astDecl *decl)
{
  astDecl * prev = (astDecl *) last_type_decl;
  last_type_decl = decl;

  if(yylloc.first_column != ';') {
      TREE_ASET_FLAG(decl) = 1;
#if 0 /*10.25.95*/  /* bad since parser uses decl_source_line */
      decl->SET_DECL_SOURCE_LINE(0);
#else
      decl->set_linenum (0);
#endif
  } else {
      TREE_ASET_FLAG(decl) = 0;
#if 0  /*10.25.95*/ /* bad since parser uses decl_source_line */
      decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
#else
      decl->set_linenum (yylloc.first_line);
#endif
      /* nested class :struct outer {struct inner { }; } */
      if (prev && (prev->TREE_TYPE() == decl->TREE_TYPE()) &&
	  (TREE_START_LINE(prev) == TREE_START_LINE(decl)) &&
	  (TREE_END_LINE(prev) == TREE_END_LINE(decl)) &&
	  /* also make sure the file names are the same too */
	  ((TREE_FILENAME(prev) == TREE_FILENAME(decl)) ||
	   (TREE_FILENAME(prev) && TREE_FILENAME(decl) &&
	    !strcmp(TREE_FILENAME(prev), TREE_FILENAME(decl))) )) {
	TREE_ASET_FLAG(prev) = 0;
#if 0 /*10.25.95*/   /* bad since parser uses decl_source_line */
	prev->SET_DECL_SOURCE_LINE(yylloc.first_line);
#else
	prev->set_linenum (yylloc.first_line);
#endif
      }
  }
}

int line_is_id(int id_line)
{
    int depth = (*yylen_ptr >= 3)? set_len(*yylen_ptr): set_len();
    if ((id_line == yylloc.first_line) && is_id(yylloc.first_column)) return yylloc.first_line;
    if ((id_line == yylsp[0].first_line) &&  is_id(yylsp[0].first_column))
       return yylsp[0].first_line;
    if ((id_line == yylsp[0].last_line) &&  is_id(yylsp[0].last_column))
       return yylsp[0].last_line;

    for(int ii=0; ii<depth; ++ii)
	if((id_line == yylsp[-ii].first_line) &&  is_id(yylsp[-ii].last_column))
	    return yylsp[-ii].last_line;
	else if((id_line == yylsp[-ii].last_line) &&  is_id(yylsp[-ii].first_column))
	    return yylsp[-ii].first_line;
  return 0;
}

int is_cond_sym(int id)
{
  return (id == ':' || id == '\?');
}

// The following variable and functions are intended to allow callers of
// various functions to enable or disable setting hooks normally created by
// ast_constructor_hook.  This is currently used to disable registration of the
// extra decl at the global level in set_nested_typename.

  static    bool	    saving_constructor_hook = true;

extern "C" void ast_disable_saving_constructor_hook() {
   saving_constructor_hook = false;
}

extern "C" void ast_enable_saving_constructor_hook() {
   saving_constructor_hook = true;
}

extern "C" 
void ast_constructor_hook(astTree *node, enum tree_code code)
{


 TREE_SMT_NODE (node) = 0;

 int len = set_len(1);
 if(len==0)
   {
     node->start_line = 0;
     node->end_line = 0;
#if 0
  /* bad since parser uses decl_source_line */
     if (is_decl(code))
       ((astDecl *)node)->SET_DECL_SOURCE_LINE(0);
#endif
     node->filen = yylloc.text; 
     return;
   }

  int nn = *yylen_ptr - 1;
  if(nn<0) nn=0;

 if(code != IDENTIFIER_NODE){
   if(len < 3) 
       return;
 } else {
#ifndef C_PARSER
      if(yylsp[-nn].first_column == OPERATOR) {
         TREE_ASET_FLAG(node) = 1;
      }
#endif
 }
  set_lineno(node);

  int global_flag = ast_is_global_level();
  int id_line;


  switch(code){
    case FUNCTION_DECL:
{
  astDecl * decl = (astDecl*) node;   
#if 0 /* 10.25.95 */ /* bad since parser uses decl_source_line */
  if(is_separator(yylloc.first_column))
     decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
  else if(is_separator(yylloc.last_column))
     decl->SET_DECL_SOURCE_LINE(yylloc.last_line);
#else
  if(is_separator(yylloc.first_column))
     decl->set_linenum (yylloc.first_line);
  else if(is_separator(yylloc.last_column))
     decl->set_linenum (yylloc.last_line);
#endif
}
         if(decl_separator(yylsp[0].last_column)){
             node->end_line = yylsp[-1].last_line;
         }
         if (is_typespec(yylsp[-nn].first_column)){
#ifndef C_PARSER
	   if ((yylsp[-nn].first_column == SCSPEC) && 
	       (yylsp[-nn].first_line == TREE_START_LINE(node)))
	     TREE_START_LINE(node) =  yylsp[-nn].last_line;
	   else
#endif
             node->start_line = yylsp[-(nn-1)].first_line;
         } else {
             int fc = yylsp[-(nn-1)].first_column;
             if(
                  ( fc == '*')      // indirect_ref
               || ( fc == '&')      // addr expr
                )
             node->start_line = yylsp[-(nn-1)].first_line;
         }

         TREE_ASET_FLAG(node) = (yylloc.first_column == '{');

         id_line = ast_find_function_id();
	 node->set_DEBUG(-id_line);

         if (instantiate_member_template_var) {
           TREE_START_LINE(node) = TREE_END_LINE(node) = 0;
	   node->set_DEBUG(0);
	 }
	 ast_put_global_decl (node);

	 break;
#ifndef C_PARSER
       case TEMPLATE_DECL:
       {
          astDecl * decl = (astDecl*) node;   
#if 0 /* 10.25.95 */ /* bad since parser uses decl_source_line */
          if(is_separator(yylloc.first_column))
             decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
	  else if(is_separator(yylloc.last_column))
             decl->SET_DECL_SOURCE_LINE(yylloc.last_line);
#else
          if(is_separator(yylloc.first_column))
             decl->set_linenum (yylloc.first_line);
	  else if(is_separator(yylloc.last_column))
             decl->set_linenum (yylloc.last_line);
#endif
         if(decl_separator(yylsp[0].last_column) ||
	    (yylsp[0].last_column == '{') ){
             node->end_line = yylsp[-1].last_line;
         }
         if (is_typespec(yylsp[-nn].first_column)){
             node->start_line = yylsp[-(nn-1)].first_line;
         } else {
             int fc = yylsp[-(nn-1)].first_column;
             if(
                  ( fc == '*')      // indirect_ref
               || ( fc == '&')      // addr expr
                )
             node->start_line = yylsp[-(nn-1)].first_line;
         }

         id_line = ast_find_id(); 
         if (id_line == 0) 
           id_line = ast_find_function_id();
         node->set_DEBUG(-id_line);
         semicolon_type_hook((astDecl*)node);
	 ast_put_global_decl (node);
      }
	 break;
#endif

// now move back start_line from IDENTIFIER to TYPESPEC

    case CONST_DECL:
         if(yylsp[-nn].first_column==IDENTIFIER)
            id_line = yylsp[-nn].first_line;
         else
            id_line = ast_find_id();
         node->set_DEBUG(-id_line);    
	 /* --- fall through --- */

    case VAR_DECL:
         if(global_flag)
        	 ast_put_global_decl (node);
	 /* --- fall through --- */

#ifdef C_PARSER
    case PARM_DECL:
#endif

    case FIELD_DECL:

	{
	  astDecl * decl = (astDecl*) node;   
#if 0 /* 10.25.95 * NY */ /* bad since parser uses decl_source_line */
	  if(is_separator(yylloc.first_column))
	     decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
	  else if(is_separator(yylloc.last_column))
	     decl->SET_DECL_SOURCE_LINE(yylloc.last_line);
#else
	  if(is_separator(yylloc.first_column))
	     decl->set_linenum (yylloc.first_line);
	  else if(is_separator(yylloc.last_column))
	     decl->set_linenum (yylloc.last_line);
#endif
	}

         if(decl_separator(yylsp[0].last_column)){
             node->end_line = yylsp[-1].last_line;
////         if(is_typespec(yylsp[-nn].first_column))
#ifndef C_PARSER
	     if (yylsp[-(nn-1)].first_column != END_OF_SAVED_INPUT)
#else
	     if (1)
#endif
	       node->start_line = yylsp[-(nn-1)].first_line;
	     else if (yylsp[-nn].first_column == IDENTIFIER)
	       node->start_line = yylsp[-nn].first_line;
         }
         break;

    case TREE_VEC:
//         id_line = ( nn==0 ? node->end_line : yylsp[-nn].last_line);
         id_line = yylsp[-nn].last_line;
         node->set_DEBUG(-id_line);
         break;

    case FUNCTION_TYPE:
    case METHOD_TYPE:
         node->start_line = last_call_expr->start_line;
         node->end_line = last_call_expr->end_line;
         node->set_DEBUG(last_call_expr->TREE_DEBUG());
         break;

    case RECORD_TYPE:
// find structure name to insert it in id_array later
         node->start_line = ast_find_id(len);
         node->end_line = node->start_line;

         break;

    case TYPE_DECL:
    {
         if(*yylen_ptr == 0){
             if(yylsp[0].first_column=='{')
                node->start_line = yylsp[-1].first_line;
         }

// forward decl struct A;  make decl_file=0
         
    
         TREE_ASET_FLAG(node) = (yylloc.first_column != ';');

         astDecl * decl = (astDecl *) node;
#if 0 /* 10.25.95 * NY */ /* bad since parser uses decl_source_line */
         decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
#else
         decl->set_linenum (yylloc.first_line);
#endif
	 int type_id_line = ast_find_id(nn);
	 if (type_id_line)
	   decl->set_DEBUG(- type_id_line);	   
	 else if (line_is_id(decl->end_line))
	   decl->set_DEBUG(-  decl->end_line);
#ifndef C_PARSER
	 ast_class_templ_specialization_hook(decl);
#endif
	 /*  960417 Kit Transue:  avoid registering classnames beginning with :: */
	 if(saving_constructor_hook) {
	   if(global_flag)
	     ast_put_global_decl (node);

	   last_type_decl = node;
	 }

         break;      
    }

    case LABEL_DECL:
       if((*yylen_ptr) != 0) {  // default or regular label
          if(yylsp[-nn].first_column == DEFAULT){
                node->end_line = yylsp[-(nn-1)].first_line;
          } else if(yylsp[-nn].first_column == IDENTIFIER) {
             // not stable
          }
       } else {                 // case
          node->start_line = yylsp[-2].first_line;
       }
 
       break;

    case LET_STMT:
      if(yylloc.last_column == '}') {
          node->end_line =  yylloc.last_line;          
      }
      if(yylsp[0].first_column == '{') {
          node->start_line =  yylsp[0].first_line;
      }

      break;

    case IF_STMT:
      node->end_line = yylsp[-1].last_line;  // ')'
      break;

    case LOOP_STMT:
      if(yylsp[-nn].first_column == DO) {
#ifdef C_PARSER
          node->end_line = yylsp[-nn].last_line;      // WHILE
#else
          node->end_line = yylsp[-(nn-3)].last_line;  // WHILE
#endif
      } else {
#ifdef C_PARSER /* 960610 kit transue */
          node->end_line = yylsp[-1].last_line;  // ')'
#else
	  /* yylsp[-1] may contain the dangling open brace from a compstmtend: */
	  if (yylsp[-1].last_column != ')')
	     node->end_line = yylsp[-2].last_line;
	     /* the above might be better as a search backwards to -*yylenptr */
	  else
	     node->end_line = yylsp[-1].last_line;  // ')'
#endif /* 960610 */
      }
      break;


    case IDENTIFIER_NODE:
#ifndef C_PARSER
      if(yylsp[-nn].first_column == OPERATOR) {
         TREE_ASET_FLAG(node) = 1;
         break;
      }
#endif
      id_line = ast_find_id();
      if(id_line){
	 if (saving_cur_id) {
	    cur_id_node = (astIdentifier*)node;
	    cur_id_line = id_line;
	    cur_id_decl = 0;
	 }

         node->start_line = id_line;
         node->end_line = id_line;
      }
      break;

    case TREE_LIST:

      int ii;

      for(ii=0; ii<nn; ++ii)
           if(! list_separator(yylsp[-ii].first_column))break;

      for(; ii<nn; ++ii)
           if(list_separator(yylsp[-(ii+1)].first_column))
                 break;

      node-> start_line = yylsp[-ii].first_line;
             
      id_line = ast_find_id();
//      if(!id_line) id_line = cur_id_line;

      if(id_line == cur_id_line) {
         if(cur_id_decl && (cur_id_decl->TREE_CODE() == TYPE_DECL))
               id_line = 0;
      }
      node->set_DEBUG(-id_line);
      break;

    case INDIRECT_REF:
    {
      int next = nn + 1;
      if(yylsp[-next].first_column == '(')
         node->start_line = yylsp[-next].first_line;

      if(yylsp[-nn].first_column == IDENTIFIER)
         id_line = yylsp[-nn].first_line;
      else {
         id_line = ast_find_id();
         if (id_line==0 && last_indirect_ref(node->start_line,
					     node->end_line,
					     node->filen)){
            id_line = - last_indirect_ref->TREE_DEBUG();
         }
      }
      node->set_DEBUG(-id_line);

      last_indirect_ref = node;

      break;
    }
	
    case ARRAY_REF:
//      last_array_ref = node;

      if(yylsp[-nn].first_column == IDENTIFIER)
         id_line = yylsp[-nn].first_line;
      else
         id_line = ast_find_id();
      node->set_DEBUG(-id_line);
      break;

#ifndef C_PARSER
    case BASE_INIT_EXPR:  // comment out
    if(0 && last_call_expr && (last_call_expr->end_line==node->end_line)
        && (last_call_expr->start_line != 1)  // implicit
        && (last_call_expr->start_line != 0)  // implicit
    ){
       if(yylsp[0].first_column==IDENTIFIER){
          last_call_expr->start_line = yylsp[0].first_line;
          last_call_expr->set_DEBUG(- yylsp[0].first_line);
       }
    }
    DBG(node){
      cout << "last_call_expr ";
      lineprint(last_call_expr);
    }
    break;
#endif

#ifndef C_PARSER
    case SCOPE_REF:
      last_scope_ref = node;
      break;
#endif

    case CALL_EXPR:
      if (*yylen_ptr != 0 && yylsp[1 - *yylen_ptr].first_column == ':'
	  && yylsp[0].first_column == IDENTIFIER) {
	 // is base init (e.g., "D::D() : B(1) { }"); default setting
	 // of start_line includes the ":", which is wrong.
	 node->start_line = yylsp[0].first_line;
      }
      last_call_expr = node;
	 /* --- fall through --- */
#ifndef C_PARSER
    case NEW_EXPR:
#endif
      if(yylsp[0].last_column==';')
         node->end_line = yylsp[-1].last_line;

#ifndef C_PARSER
      if(*yylen_ptr==0){
         ajust_call_expr(len, node);
      } else if(yylsp[-nn].first_column == '~') {  // destructor
         last_destructor_expr = node;
         node->set_DEBUG(-yylsp[-nn].last_line);
         break;
      } else if(yylsp[-nn].first_column == OPERATOR) {  // operator
         node->set_DEBUG(-yylsp[-nn].last_line);
         TREE_ASET_FLAG(node) = 1;
         break;
      } else if(yylsp[-nn].first_column == NEW) {  //  new
         static astTree * was_new = 0;
         if(was_new && (TREE_START_LINE(was_new) == TREE_START_LINE(node)))
         {    // constructor
           was_new = 0;
           for(int jj=1; jj<3; ++jj){
             if(yylsp[-(nn-jj)].first_column == IDENTIFIER) {  // class
                node->set_DEBUG(-yylsp[-(nn-jj)].first_line);
                TREE_START_LINE(node) = yylsp[-(nn-jj)].first_line;
                TREE_ASET_FLAG(node) = 1;
                break;                
             }
           }
          } else {  // operator new
            was_new = node;
            node->set_DEBUG(-yylsp[-(nn)].first_line);
            TREE_ASET_FLAG(node) = 1;
          }
         break;
      } else if(yylsp[-nn].first_column == LEFT_RIGHT) {  //  cast
         node->set_DEBUG(0);
         TREE_ASET_FLAG(node) = 1;
         break;
      }
      else if (TREE_START_LINE(node) && nn && (yylsp[-nn].last_column == '>') &&
	       (yylsp[-nn].first_column != IDENTIFIER)) {
	TREE_START_LINE(node) = yylsp[-(nn -1)].first_line;
      }
      /* x.f() ? ... : ....; */
      else if (code == NEW_EXPR && (nn > 1) && is_cond_sym(yylsp[-(nn - 1)].first_column) &&
	       (TREE_START_LINE(node) == yylsp[-nn].first_line) && (yylsp[-nn].last_column == LEFT_RIGHT))
      TREE_END_LINE(node) = yylsp[-nn].last_line;
#endif
      if(last_indirect_ref(yylsp[-nn].first_line, yylsp[-nn].last_line, yylsp[-nn].text)
         && (yylsp[-nn].first_column == '(') 
         && (yylsp[-nn].last_column  == ')') )
        {    // int (*fp) ()

           id_line = - last_indirect_ref->TREE_DEBUG();
           last_indirect_ref = 0;
        } else {
           id_line = ast_find_call_id(nn, node);
	   if (id_line < node->start_line) {
	      // probably base init: default handling includes the ":", which is wrong.
	      id_line = node->start_line;
	   }
        }

      node->set_DEBUG(-id_line);

      cur_call = node;
      break;
    case CASE_STMT:
       if(yylsp[-1].first_column == ')')  // switch(..)
           node->end_line = yylsp[-1].first_line;
       break;

    case INTEGER_CST:
    case REAL_CST:
      if (yylsp[0].first_column != CONSTANT)
        TREE_START_LINE(node) = TREE_END_LINE(node) = yylloc.first_line;
      else
        TREE_START_LINE(node) = TREE_END_LINE(node) = yylsp[0].first_line;
      break;
   
    case STRING_CST:
      if (yylsp[0].first_column != STRING)
        TREE_START_LINE(node) = TREE_END_LINE(node) = yylloc.first_line;
      else
        TREE_START_LINE(node) = TREE_END_LINE(node) = yylsp[0].first_line;
      break;
   
    default: 
      break;
  }
  if(node->start_line > node->end_line){    // last resort 
  if (!possible_template_declspec(node)) {  // deal with this in declspecs hook
#if 0
       int el = node->start_line;           // can happen in macro expansion
       node->start_line =  node->end_line;
//       node->end_line =  yylloc.last_line;          
       node->end_line =  el;
#else
       node->start_line = node->end_line;
       if (code == CALL_EXPR) node->set_DEBUG(0);
#endif
     }
  }

#ifndef C_PARSER
  if (instantiate_member_template_var || 0/*processing_template_defn*/) {
    TREE_START_LINE(node) = TREE_END_LINE(node) = 0;
#if 0
    if (code == FIELD_DECL || code == FUNCTION_DECL || code == VAR_DECL
	|| code == CONST_DECL || code == FIELD_DECL || code == TYPE_DECL)
    ((astDecl*)node)->SET_DECL_SOURCE_LINE(0);
#endif
  }
#endif


#ifndef _WIN32
  DBG(node){
    char * code_name = tree_code_name[code];

    cout << endl << "global " << global_flag;
    cout << " code " << code << ' ' << code_name 
               << " len " << len << " yylen " << *yylen_ptr << endl;

    cout << "RESULT: ";
    lineprint (node);

    ast_hook_prt(*yylen_ptr + 2);
  }
#endif
}


extern "C" void ast_extern_var_decl_hook(astDecl *decl)
  /*
    put locally declared extern var decl into global arr
  */
{
  if (!decl || (decl->TREE_CODE() != VAR_DECL) || 
#ifndef C_PARSER
      !DECL_THIS_EXTERN(decl) ||
#else
      !DECL_EXTERNAL(decl) ||
#endif
      ast_is_global_level() )
    return;
  ast_put_global_decl(decl);
  DBG(decl) {
    cout << "ast_extern_var_decl_hook " << endl;
    lineprint(decl);
    ast_hook_prt(*yylen_ptr + 2);
  }
}

extern "C" void ast_implicit_decl_hook(astDecl *new_decl, astIdentifier *)
{
  TREE_START_LINE (new_decl) = 0;
  TREE_END_LINE (new_decl) = 0;
}

extern "C" void ast_define_label_hook (astDecl* decl, char* filename, int line)
{
  if(is_untokenized_file(filename)) return;

  TREE_FILENAME (decl) = filename;
  TREE_END_LINE (decl) = line;
  int len = (*yylen_ptr >= 3)? set_len(*yylen_ptr): set_len();
  for (int ii = 0; ii < len ; ++ii)
    if (yylsp[-ii].first_column == IDENTIFIER)
      break;
  if (ii < len)
    TREE_START_LINE (decl) = yylsp[-ii].first_line;

  DBG(decl){
    cout << "ast_label_hook " << line << endl;
    lineprint(decl);
    ast_hook_prt(*yylen_ptr + 2);
  }
}

extern "C" void ast_goto_hook (astDecl* decl, astIdentifier* id)
{
  if(is_untokenized_file(yylloc.text)) return;

  int len = (*yylen_ptr >= 3)? set_len(*yylen_ptr): set_len();
  for (int ii = 0;ii < len; ++ii)
    if ( yylsp[-ii].first_column == IDENTIFIER )
      break;
  if (ii < len)
    ast_put_ref_ids (yylsp[-ii].first_line, my_input_filename, decl, id);
  DBG(decl){
    cout << "ast_goto_hook " << yylsp[-ii].first_line << endl;
    lineprint(decl);
    ast_hook_prt(*yylen_ptr + 2);
  }
}

// extern "C" void ast_error_hook (char* file, int err_code){}

extern "C" void ast_else_hook (astStmt* stmt)
{
  if(is_untokenized_file(yylloc.text)) return;

  // find token ELSE
  for(int ii=0; ii<4; ++ii){
    if(yylsp[-ii].first_column == ELSE){
        stmt->SET_STMT_SOURCE_LINE(yylsp[-ii].first_line);
        break;
    }
  }
  DBG(stmt){
    cout << "ast_else_hook " << stmt << ' ' << yylsp[-1].first_line << endl;
    ast_hook_prt(*yylen_ptr + 2);
  }
}

#ifndef C_PARSER
int ast_decl_is_oper(astDecl *decl);
#endif

static int last_id_tok = 0;
#ifndef C_PARSER
static void ast_id_hook_int(astIdentifier *id, astDecl * cur_decl, int is_field)
#else
static void ast_id_hook_int(astIdentifier *id, astDecl * cur_decl, int)
#endif
{

  if(!id) return;
  if(!cur_decl) return;
  if(!TREE_FILENAME(cur_decl)) return;

  int len = set_len(2); // =3 for the rest
  if(len==0) return;

  int id_line;
  int is_oper = 0;
#ifndef C_PARSER
 if(! is_field){
     /* tree_aset_flag does not have right info sometime? */
     is_oper = TREE_ASET_FLAG(id) || ast_decl_is_oper(cur_decl); 

  if( (cur_decl->TREE_CODE() != FIELD_DECL) &&
      (cur_decl->TREE_CODE() != CONST_DECL) && 
      (cur_decl->TREE_CODE() != TYPE_DECL) && 
      /* avoid prev cases, don't understand next two lines */
      (yylloc.first_column == LEFT_RIGHT && yylloc.last_column == LEFT_RIGHT)
    ) {// CALL_EXPR
   DBG(cur_decl){
    cout << endl<< " ast_id_hook CALL_EXPR ID cur_decl" 
          << " yylen " << *yylen_ptr << " len " << len << endl;

    lineprint (id);
    lineprint(cur_decl);
    ast_hook_prt(2);

   }
   return;
  }
 }
#endif
  
  if(is_oper) {
#if 0
    int ind = (*yylen_ptr - 2);

    if(ind >= 0 && yylsp[-ind].first_column != IDENTIFIER)
       id_line = yylsp[-ind].first_line;
    else if(yylsp[-(ind+1)].first_column != IDENTIFIER)
       id_line = yylsp[-(ind+1)].first_line;
    else 
#endif
       id_line = 0;

  } else {

    if((yylloc.first_line == yylloc.last_line)
       && (yylloc.first_column == yylloc.last_column)) {
          id_line = ast_find_id(3);	
	  if ((id_line == 0) && (*yylen_ptr > 3))
	    id_line = ast_find_id_from_depth(*yylen_ptr);
    }
    else
          id_line = yylloc.first_line;
  }

  /* id_line is wrong here, will try to reset this later in init_hook*/
   if(id_line && (cur_decl->TREE_DEBUG() == 0) &&
      (cur_decl->TREE_CODE() == VAR_DECL) &&
      (id_line != yylloc.first_line) &&
#if 0 /* 10.25.95 * NY */
      TREE_START_LINE(cur_decl) && cur_decl->DECL_SOURCE_LINE() &&
      (id_line > cur_decl->DECL_SOURCE_LINE()))
#else
      TREE_START_LINE(cur_decl) && cur_decl->get_linenum () &&
      (id_line > cur_decl->get_linenum ()))
#endif
     return;


  if ( id_line){	
    ast_put_ref_ids( id_line, my_input_filename, cur_decl, id);
    if (saving_cur_id) {
       cur_id_node = id;
       cur_id_line = id_line;
       cur_id_decl = cur_decl;
    }
  }

  DBG(cur_decl){

    cout << endl<< " ast_id_hook ID " << id_line << " oper " << is_oper
          << " yylen " << *yylen_ptr << " len " << len << endl;
    lineprint (cur_decl);
    lineprint (id);

    ast_hook_prt(2);
  }
}

static void ast_constr_id_hook(astIdentifier *id, astDecl *cur_decl)
{

// #ifndef C_PARSER

  if(!id) return;
  if(!cur_decl) return;
  if(!TREE_FILENAME(cur_decl)) return;

  int len = set_len(2); // =3 for the rest
  if(len==0) return;

  int id_line = 0;
  int is_oper = 1;

  DBG(cur_decl){

    cout << endl<< " ast_constr_id_hook ID " << id_line << " oper " << is_oper
          << " yylen " << *yylen_ptr << " len " << len << endl;
    lineprint (cur_decl);
    lineprint (id);

    ast_hook_prt(len+2);
  }
// #endif
}

extern "C" void ast_change_id_hook
            (astIdentifier *id, astDecl * olddecl, astDecl *newdecl) 
{
// struct AA{AA(int);}; int AA(); AA a1=AA(), a2=AA(), a3(AA());

    int len = set_len();
    if(len==0) return;

    int curtok = yylloc.timestamp;
    int off = (curtok==last_id_tok) ? 2 : 1;
    
    if(olddecl == newdecl) return;
    
    int size = ref_ids.size();
    if(size < 1) return;

    int line_no =0;
    int nn = *yylen_ptr;

    int flag = 0;

    if(nn>0){   // AA aa(args);
       line_no = yylsp[-(nn-1)].first_line;
       for(int ii=1; ii<3; ++ii){
          if(ii == size) break;
          ident * nid = ref_ids[size-ii];
          if(nid->lineno != line_no) 
             continue;
          if((nid-> decl != olddecl) || (nid-> id != id)) 
             continue;
             
          nid->decl = newdecl;
          flag = 1;
          break;
       }
    } else if(size >= off) {  // AA aa = 1;
       ident * nid = ref_ids[size-off];
       if((nid-> decl == olddecl) && (nid-> id == id)){
           nid->decl = newdecl;
           flag = 1;
       }
    }
    DBG(newdecl) {
       cout << " ast_change_id_hook flag " << flag << " off " << off
                 << " ln " << line_no << " old new id " << endl;
       lineprint (olddecl);
       lineprint (newdecl);
       lineprint(id);
 
       ast_hook_prt(*yylen_ptr + 2);

    }
}

#ifndef C_PARSER
extern "C" int ast_accessing_scope()
{
  return (yylsp[0].last_column == SCOPE);
}
#endif

extern "C" int ast_accessing_field()
{
  return ((yylsp[0].last_column == '.') || (yylsp[0].last_column == POINTSAT)
#ifndef C_PARSER
       || (yylsp[0].last_column == SCOPE)
#endif
);

}
extern "C" void ast_id_hook(astIdentifier *id, astDecl * cur_decl, int is_constr)
{
  last_id_tok = yylloc.timestamp;
  if(is_constr)
    ast_constr_id_hook(id, cur_decl);
  else
    ast_id_hook_int(id, cur_decl, 0);
}

#ifndef C_PARSER
extern "C" void force_ast_id_hook(astIdentifier *id, astDecl * cur_decl)
{
  int tmp = processing_template_defn;
  processing_template_defn = 0;
  if (!cur_decl  && cur_decl->TREE_TYPE() &&
      (cur_decl->TREE_TYPE()->TREE_CODE() == UNINSTANTIATED_P_TYPE))
    return;
  ast_id_hook(id, cur_decl, 0);
  processing_template_defn = tmp;
}
#else
extern "C" void force_ast_id_hook(astIdentifier *, astDecl * )
{

}
#endif

extern "C" void ast_c_struct_id_hook (astIdentifier* id, astDecl* typedecl)
{
  ast_id_hook_int (id, typedecl, 0);
}

extern "C" void ast_struct_friend_hook(astDecl * cl, astDecl*decl)
{
  if(is_untokenized_file(yylloc.text)) return;

//  decl->SET_DECL_SOURCE_LINE(0);
  TREE_ASET_FLAG(decl) = 0;

#ifndef C_PARSER
  astIdentifier * name = 0;
  if (decl)
    name = (astIdentifier *) decl->DECL_ASSEMBLER_NAME();
  if (name)
    ast_id_hook(name, decl, 0);
#endif
  DBG(decl){

    cout << "\n ast_struct_friend_hook yylen " << endl;

    lineprint (decl);
    lineprint (cl);
    
    ast_hook_prt(*yylen_ptr + 2);
  }
}

#ifdef C_PARSER
extern "C" int in_parm_level_p();
extern "C" void ast_c_id_hooks(astDecl *decl)
{
  if (in_parm_level_p()) {
    astIdentifier * idn = decl->DECL_NAME();
    if (idn == 0) {
      astType * ty = (astType *) decl->TREE_TYPE();
      if (ty)
	idn = (astIdentifier *) ty->TYPE_NAME();
    }
    if (idn && idn->TREE_CODE() == IDENTIFIER_NODE)
      ast_c_struct_id_hook (idn, decl);
  }
}
#endif

extern "C" void ast_struct_expr_hook(astDecl * decl)
{
  if(is_untokenized_file(yylloc.text)) return;
#ifndef C_PARSER
//  decl->SET_DECL_SOURCE_LINE(0);
#else
#if 0 /* 10.25.95 * NY */
    decl->SET_DECL_SOURCE_LINE(0); 
#else
    decl->set_linenum (0);
#endif
    if (last_type_decl == decl)
      ast_c_id_hooks(decl);
#endif
  last_type_decl = decl;

  DBG(decl){

    cout << "\n ast_struct_expr_hook yylen " << endl;

    lineprint (decl);
    
    ast_hook_prt(*yylen_ptr + 2);
  }
}

extern "C" void ast_enum_expr_hook(astDecl * decl)
{
  if(is_untokenized_file(yylloc.text)) return;
#ifndef C_PARSER
#else
#if 0 /* 10.25.95 * NY */
    decl->SET_DECL_SOURCE_LINE(0); 
#else
    decl->set_linenum (0);
#endif
#endif

  last_type_decl = decl;

  DBG(decl){

    cout << "\n ast_enum_expr_hook yylen " << endl;

    lineprint (decl);
    
    ast_hook_prt(*yylen_ptr + 2);
  }
}

astDecl * last_nameless_struct = 0;
// finish struct, class, union
extern "C" void ast_struct_hook(astDecl * decl, astDecl * id_decl)
{
  int len = set_len();
  if(len==0) return;


  astTree* node = decl;
  astType* tp = decl->type;
  int new_end_line;

#ifndef C_PARSER
  if (processing_template_decl || 0/*processing_template_defn*/) {
    TREE_START_LINE(decl) = TREE_END_LINE(decl) = 0;
    return;
  }
  if (processing_template_defn) {
     decl->artificial_flag = 1;
  }
#endif

// check if originally defined in different file

//  if ( TREE_FILENAME(decl) != my_input_filename ){
      ast_put_global_decl (decl);
//  }
  
#ifdef C_PARSER
  set_lineno(node);  
#endif

  // call id_hook on itself

//  int id_line = tp->end_line;
  int id_line = node->end_line;

  int template_name = 0;

  // find '{' and '}' to mark tp

  if(yylloc.first_column == '{')
     new_end_line = yylloc.first_line;
  else if (yylsp[0].last_line < yylsp[0].first_line &&
	   yylsp[0].last_column == '}')
     new_end_line = yylsp[0].last_line;
  else
     new_end_line = yylsp[0].first_line;
  /* A.H. 09.19.96 */
  /* Here example where we had bad smt mapping :
     file A.C:
#include "b.h"
class 
B



{
}
;
      file b.h (empty line significant):











class
B
;
    */

   
  int new_new_end_line = new_end_line; 

  int nn = *yylen_ptr;
  for(int ii=nn-1; ii>0; --ii){ 
       if(yylsp[-ii].first_column == '{'){
           tp->start_line = yylsp[-ii].first_line;
	   new_end_line = node->end_line;
           if(yylsp[-(ii+1)].last_column != '{')
               new_end_line = yylsp[-(ii+1)].last_line;
#ifndef C_PARSER
           else if (yylsp[-(ii+1)].first_column == AGGR)
	     template_name = 1;
#endif
	   else
	     new_end_line = yylsp[-(ii+1)].first_line;
	   if (new_end_line > node->start_line &&
	       !reinstantiating_template()) {
	      node->end_line = new_end_line;
	   }
           break;
       }
  }

  if (new_new_end_line > tp->start_line) {
     tp->end_line = new_new_end_line;
  }

#ifdef C_PARSER
  id_line = node->end_line;
#endif

  astIdentifier * id = decl->DECL_NAME();
  if(id){
      char * name = decl->get_name();
      if(name[0] == JOINER)
          id_line = 0;   
  }
#ifdef C_PARSER
  else {
    astIdentifier * id = decl->ID_NODE();
    char * name = (id)?id->IDENTIFIER_POINTER():0;
    if((name == 0) || name[0] == JOINER) {
      id_line = 0;
      last_nameless_struct = decl;
    }
  }
#endif

  if (template_name)
    ;
  else if (id_line)
      ast_update_id(id_line, id_decl, decl);
  else 
      decl->set_DEBUG(0);

  semicolon_type_hook(decl);


  DBG(decl){

    cout << "\n ast_struct_hook yylen " << *yylen_ptr << " len " << len ;
            cout << " id_decl " << id_decl << endl;

    lineprint (decl);
    
    if(id_decl) lineprint(id_decl);

    lineprint (tp);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

static bool processing_default_inlines = false;

extern "C" void ast_processing_default_inlines(int tf) {
   processing_default_inlines = tf;
}

extern "C" int need_call_swap_decl(astTree *oldt, astTree *newt)
{
   if (reinstantiating_template()) {
      return 0;
   }

  if (oldt && newt && (oldt->TREE_CODE() == TYPE_DECL)) {
    astIdentifier *idn = ((astDecl *)oldt)->DECL_NAME();
    if (idn && idn->IDENTIFIER_TEMPLATE()) {
       if (processing_default_inlines) {
	  return 0;
       }
      if (TREE_START_LINE(oldt) == TREE_START_LINE(newt))
	return 0;
      else if ( (TREE_START_LINE(newt) == TREE_END_LINE(newt)) && 
		(TREE_START_LINE(oldt) != TREE_END_LINE(oldt)))
	return 0;
    }
  }
  if (yychar == (int) ';')
    return 0;
#ifndef C_PARSER
  else if (ast_hook_typedecl_is_def ((astDecl *)oldt))
    return 0;
#endif
/*
  else if ((yylloc.first_column == '{') ||
         (yylloc.first_column == ':'))
    return 1;
*/
  else
    return 1;
}
extern "C" void ast_swap_tree_info(astTree * oldtree, astTree * newtree);

//
// returns: -1: error, 0: not a local value, >0: the level of nesting
//
extern "C" int
ast_type_local_value(astTree * decl)
{
    int ret_val = -1;

    if (decl) {
	tree_code code = decl->TREE_CODE();
	if (code==TYPE_DECL || code==FUNCTION_DECL || code==VAR_DECL ||
	    code==FIELD_DECL || code==CONST_DECL) {
	    decl = ((astDecl *)decl)->DECL_CONTEXT(); ret_val = 0;
	} else if (code==RECORD_TYPE || code==UNION_TYPE ||
		   code==ENUMERAL_TYPE || code==QUAL_UNION_TYPE) {
	    decl = ((astType *)decl)->context; ret_val = 0;
	}
	if (!ret_val) {
	    for (int nested=1; decl!=0 && ret_val<=0; nested++) {
		code = decl->TREE_CODE();
		if (code != TYPE_DECL) {
		    if (code == FUNCTION_DECL)
			ret_val = nested;
		    else if (code==RECORD_TYPE || code==UNION_TYPE ||
			     code==ENUMERAL_TYPE || code==QUAL_UNION_TYPE)
			decl = ((astType *)decl)->context;
		    else
			decl = (astTree *)0;
		} else
		    decl = ((astDecl *)decl)->DECL_CONTEXT();
	    }
	}
    }

    return ret_val;
}

//
// returns: -1: error, 0: a supported local declaration, >0: not supported
//
extern "C" int
ast_type_not_supported(astTree * decl)
{
    int ret_val = -1;

    if (decl) {
	ret_val = 0;

	int val = ast_type_local_value(decl);
	if (val == 1) {
	    tree_code code = decl->TREE_CODE();
	    if (code==FUNCTION_DECL || code==VAR_DECL)
		val = 0;
	}
	if (val > 0)
	    ret_val = val;
    }

    return ret_val;
}

//
// returns: -1: error/bad input, >=0: number of values (const_decls) for the particular "enum"
//
extern "C" int
ast_const_hook(astDecl * context)
{
    int ret_val = -1;

    if (context) {
	if (!ast_type_local_value(context)) {
	    astType * type = context->TREE_TYPE();
	    if (type->TREE_CODE() == ENUMERAL_TYPE) {
		ret_val = 0;

		for (astList *list=(astList *)(type->TYPE_VALUES());
		     list!=0;
		     list=(astList *)(list->TREE_CHAIN())) {
		    astIdentifier * id_decl = (astIdentifier *)(list->TREE_PURPOSE());
		    if (id_decl) {
			astDecl * decl = (astDecl *)(id_decl->IDENTIFIER_GLOBAL_VALUE());
			if (!decl)
			    decl = (astDecl *)(id_decl->IDENTIFIER_CLASS_VALUE());
			if (decl) {
			    if (decl->TREE_CODE() == CONST_DECL) {
				ident * nid = ref_ids.grow(1);

				if (nid) { 
				    nid->lineno		= TREE_START_LINE(decl);
				    nid->filename	= TREE_FILENAME(decl);
				    nid->decl		= decl;
				    nid->id 		= id_decl;
				    nid->cur_decl	= context;
				    ref_ids_hash.add(nid); // sturner, 980511

				    ret_val++;
				}
			    }
			}
		    }
		}
	    }
	}
    }

    DBG(context) {
	cout << "\n ast_const_hook() got " << ret_val << " elements" << endl;
    }

    return ret_val;
}

// finish enum
extern "C" void ast_enum_hook(astDecl * decl, astDecl * id_decl)
{
  int len = set_len();
  if(len==0) return;


//  astTree* node = decl;
  astType* tp = decl->type;
  
// check if originally defined in different file

//  if ( TREE_FILENAME(decl) != my_input_filename ){
      ast_put_global_decl (decl);
//  }

  if(yylsp[-(*yylen_ptr - 2)].first_column == IDENTIFIER){
  // next_to_last

     int id_line = yylsp[-(*yylen_ptr - 2)].first_line;
     ast_update_id(id_line, id_decl, decl);
  }
  // find '{' and '}' to mark tp

  if (yylloc.first_column != '}' &&
      yylsp[0].first_line > yylsp[0].last_line &&
      yylsp[0].last_column == '}') {
     tp->end_line = yylsp[0].last_line;
  }
  else tp->end_line = yylloc.first_line; // '}'


  int nn = *yylen_ptr;
  for(int ii=0; ii<nn; ++ii){ 
       if(yylsp[-ii].first_column == '{'){
           tp->start_line = yylsp[-ii].first_line;
           decl->end_line = yylsp[-(ii+1)].last_line;
       } else if(yylsp[-ii].first_column == ENUM) {
           decl->start_line = yylsp[-ii].first_line;
	   TREE_FILENAME(decl) = yylsp[-ii].text;
           break;
       }
  }

  astIdentifier * id = decl->DECL_NAME();
  if(id){
      char * name = decl->get_name();
      if(name[0] == JOINER)
         decl->set_DEBUG(0);
  }
	
  semicolon_type_hook(decl);

  // -- report all elements of the processed "enum"
  (void) ast_const_hook(decl);

  DBG(decl){

    cout << "\n ast_enum_hook yylen " << *yylen_ptr << " len " << len 
                                                       << " decl" << endl;
    lineprint (decl);

    cout << "id_decl " << id_decl << endl;
    if(id_decl) lineprint(id_decl);

    cout << " type " << endl;

    lineprint (tp);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

astType *ast_get_base_type(astType *tp)
{
  while (tp && ((tp->TREE_CODE() == POINTER_TYPE) || (tp->TREE_CODE() == REFERENCE_TYPE)))
    tp = tp->TREE_TYPE();

  return tp;
}

// finish typedef
extern "C" void ast_typedef_hook(astDecl* decl, astDecl* id_decl, astTree*tt)
{
  int len = set_len();
  if(len==0) return;

  astType* tp = decl->type;
  astType*ptp = 0;

//  if(tp->TREE_CODE() == POINTER_TYPE){
      ptp = tp->type;
//  } 
#ifdef C_PARSER
//  else 
   if(tp->TREE_CODE() == RECORD_TYPE){
    if(last_nameless_struct && last_nameless_struct->type == tp){
       DECL_CONSTRUCTOR(last_nameless_struct) = decl; // ->DECL_NAME();
       last_nameless_struct = 0;
    }
  }
#endif
  int id_line;
  if(tt){
     id_line = - tt->TREE_DEBUG();
     if(tt->TREE_CODE() == INDIRECT_REF){
         decl->start_line = tt->start_line;
#ifndef C_PARSER
         astType * ty = ast_get_base_type(tp);
         if (ty && (ty->TREE_CODE() == RECORD_TYPE || ty->TREE_CODE() == OFFSET_TYPE ||
		    ty->TREE_CODE() == METHOD_TYPE))
	   id_line = TREE_END_LINE(tt);
#endif
         if(ptp && (ptp->TREE_CODE() == RECORD_TYPE)) // kluge
             decl->end_line = tt->end_line;
     } else if (tt->TREE_CODE() == CALL_EXPR) {
	 TREE_START_LINE(decl) = TREE_START_LINE(tt);
     }
  } else {
     id_line = ast_find_id();
  }

  decl->set_DEBUG(-id_line);
#if 0
  ast_update_id(id_line, id_decl, decl);
#endif

  int ii=*yylen_ptr - 1;
  for(; ii< len; ++ii)
    if(yylsp[-ii].first_column == SCSPEC){
         int st  = yylsp[-ii].first_line;
         int en  = yylsp[-ii].last_line;
       
         put_declspecs(st, en, NULL, decl);
         break;
    }

  if(!tt) for(ii=1; ii< len; ++ii) {
    if(is_separator(yylsp[-ii].first_column))
         continue;

    decl->start_line  = yylsp[-ii].first_line;
    decl->end_line    = yylsp[-ii].last_line;
#if 0 /* 10.25.95 * NY */ /* bad since parser uses decl_source_line */
    decl->SET_DECL_SOURCE_LINE(yylsp[-(ii-1)].first_line); 
#else
    decl->set_linenum (yylsp[-(ii-1)].first_line); 
#endif
    break;
  }

  semicolon_type_hook(decl);


  DBG(decl){
    cout << "\n ast_typedef_hook yylen " << *yylen_ptr << " len " << len ;
         cout << " id_decl " << id_decl << " tp ptp tt " << tt << endl;

    lineprint (decl);

    if(id_decl) lineprint(id_decl);
    lineprint (tp);
    if(ptp) lineprint(ptp);
    if(tt) lineprint(tt);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

// finish var
static astDecl* last_hooked_var = 0;
extern "C" void ast_var_hook(astDecl * decl, astDecl * id_decl, astTree * tt)
{
  last_hooked_var = decl;
  int len = set_len();
  if(len==0) return;

  tt = call_expr_of_fp_decl(decl, tt);

  if(tt){
    int tree_code = tt->TREE_CODE();
    if(tree_code == IDENTIFIER_NODE){
        tt = 0;
    } else if (tree_code == CALL_EXPR){
        astTree * ttt = ((astExp2*)tt)->TREE_OPERAND(0);
        if(ttt->TREE_CODE() == INDIRECT_REF)
          tt = ttt;
    }
  }
#ifndef C_PARSER
  int depth = (*yylen_ptr > 2) ? *yylen_ptr : 2;
  int id_line = ast_find_id(depth, decl->DECL_NAME());
#else
  int id_line = ast_find_id(3);
#endif
  ast_update_id(id_line, id_decl, decl);
#ifndef C_PARSER
  if (id_decl && (id_decl == decl))
    ast_rep_ref_ids(id_line, my_input_filename, id_decl, decl, decl->DECL_NAME());    
#endif

  if(tt){
    if (tt->TREE_DEBUG()) {
      decl->set_DEBUG(tt->TREE_DEBUG());
#ifndef C_PARSER
      if (tt->TREE_CODE() == CALL_EXPR) {
	astTree * ttt = ((astExp2*)tt)->TREE_OPERAND(0);
	if (ttt && ttt->TREE_CODE() == SCOPE_REF)
	  decl->set_DEBUG(- TREE_END_LINE(ttt));
      }
#endif
    }
  }
  astType* tp = decl->type;
  if(tt && tp->TREE_CODE() == POINTER_TYPE){
    astType* ptp = tp->type;
    if(ptp->TREE_CODE() == FUNCTION_TYPE){
      if(  (tt->TREE_CODE() == CALL_EXPR)
        || (tt->TREE_CODE() == INDIRECT_REF) )
            decl->start_line = tt->start_line; 
    }
  }
#ifndef C_PARSER
  if (tt && tp 
      && tp->TREE_CODE () == RECORD_TYPE 
      && TYPE_LANG_SPECIFIC(tp)->type_flags.ptrmemfunc_flag)
      if (tt->TREE_CODE() == CALL_EXPR || tt->TREE_CODE() == INDIRECT_REF)
	  decl->start_line = tt->start_line; 
#endif

#if 0 /* 10.25.95 * NY */ /* bad since parser uses decl_source_line */
  decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
#else
    decl->set_linenum (yylloc.first_line);
#endif
  
  DBG(decl)
    {

    cout << "\n ast_var_hook yylen " << *yylen_ptr << " len " << len ;
       cout << " id_decl " << id_decl << " tt " << tt << endl;

    lineprint (decl);

    if(id_decl) lineprint(id_decl);

    lineprint (tp);
    if(tt) lineprint(tt);

    ast_hook_prt(*yylen_ptr + 5);
  }
}

extern "C" void ast_first_parm_hook(astTree * list)
{
  if (!list || (list->TREE_CODE() != TREE_LIST)) return;
  if (*yylen_ptr < 3) return;
  if (yylsp[-3].first_column != '(') return;
  TREE_START_LINE(list) = yylsp[-2].first_line;
  TREE_END_LINE(list) = yylsp[-2].last_line;
}

// finish parm
extern "C" void ast_parm_hook(astDecl * decl, astDecl * id_decl, 
                                   astTree * tl, int ansi_flag, astTree * tt)
{
  int len = set_len();
  if(len==0) return;

  if(tt && tt->TREE_CODE() == IDENTIFIER_NODE) tt = 0;

  int id_line = 0;

#ifndef C_PARSER
  if (decl) {
      char * name = (decl->DECL_NAME())? decl->DECL_NAME()->IDENTIFIER_POINTER():0;
      if (name && (name[0] == '_') && (name[1] == 'c') && (name[2] == 't') &&
	  (strcmp(name, "_ctor_arg") == 0))
	  return;
  }
   ansi_flag = 1;
#endif

 TREE_ASET_FLAG(decl) = (! ansi_flag);

 if(ansi_flag){
   tt = call_expr_of_fp_decl(decl, tt);
   TREE_START_LINE(decl) = TREE_START_LINE(tl);
   TREE_END_LINE(decl) = TREE_END_LINE(tl);
   TREE_FILENAME(decl) = TREE_FILENAME(tl);
//   decl->SET_DECL_SOURCE_FILE(0);  // ansi
   decl->set_TREE_PREV(0);         // ansi
 
   astIdentifier * id = decl->DECL_NAME();
   if(id){
      if(tt)
         id_line = - tt->TREE_DEBUG();
      if(id_line==0)
         id_line = - tl->TREE_DEBUG();
      if (id_line == 0) {
	 astTree* purp = ((astList*) tl)->TREE_PURPOSE();
	 if (purp && purp->TREE_CODE() == TREE_LIST) {
	    astTree* val = ((astList*) purp)->TREE_VALUE();
	    if (val) {
	       if (val->TREE_CODE() == CALL_EXPR) {
		  id_line = -val->TREE_DEBUG();
	       }
	       else if (val->TREE_CODE() == INDIRECT_REF) {
		  if (last_indirect_ref(TREE_START_LINE(tl),
					TREE_END_LINE(tl),
					tl->filen)) {
		     id_line = -last_indirect_ref->TREE_DEBUG();
		  }
	       }
	    }
	 }
      }
      ast_update_id(id_line, id_decl, decl);
   } else {
      decl->set_DEBUG(0);  // no name
   }
 } else { 

// call hook id for arg in arg list  for K&R: foo(x)   int x;  {...}
//                                                tl    decl

    if(!tl) return;
    id_line = TREE_START_LINE(tl);
    if(id_line)
       ast_update_id(id_line, id_decl, decl);

    decl->set_TREE_PREV((astTree *)id_line);  // K&R
 
    if(decl->start_line) { // explicit k&r decl
       id_line = (tt ? - tt->TREE_DEBUG() : ast_find_id());
       ast_update_id(id_line, id_decl, decl);
    } else {
       decl->set_DEBUG(0);
    }

 }

    if(decl->TREE_CODE() != PARM_DECL){
      cout << "ast_parm_hook:*** error: WRONG TYPE ****" << endl;
      lineprint (decl);
    }
  DBG(decl){
    cout << "\n ast_parm_hook yylen " << *yylen_ptr << " len " << len ;
    cout << " id_decl " << id_decl << " ansi_flag " << ansi_flag 
                << " tt " << tt << endl;

    if(decl->TREE_CODE() != PARM_DECL)
      cout << "*** error: WRONG TYPE ****" << endl;

    lineprint (decl);
    lineprint (tl);

    if(id_decl) lineprint(id_decl);
    if(tt)  lineprint(tt);
    ast_hook_prt(*yylen_ptr + 2);
  }
}

#ifndef C_PARSER
static int is_destructor_line(int nn)
{
 return 
     (yylsp[-nn].first_column =='~') 
  && (yylsp[-nn].last_column == LEFT_RIGHT)
 ;
}

static int is_destructor()
{
  int nn = *yylen_ptr - 1;

  if(nn==0) return 0;

  if( is_destructor_line(nn)) return 1;

  if( nn > 0 && is_destructor_line(nn-1)) return 1;

  return 0;
}
#endif
#ifndef C_PARSER
extern "C" void ast_class_template_id_hook(astIdentifier *idt)
{
  if (!idt || (idt->TREE_CODE() != IDENTIFIER_NODE)) return;

  astDecl * decl = (astDecl*) idt->IDENTIFIER_GLOBAL_VALUE();
  if (!decl || (decl->TREE_CODE() != TEMPLATE_DECL)) return;
  
  int depth = *yylen_ptr;
  if (depth < 3) return;
  
  if (is_id(yylsp[1-*yylen_ptr].first_column) &&
      (yylsp[1-*yylen_ptr + 1].first_column == '<')) {
    int id_line = yylsp[1-*yylen_ptr].first_line;
    ast_put_ref_ids(id_line, my_input_filename, decl, idt);
  }
}
#else
extern "C" void ast_class_template_id_hook(astIdentifier *)
{}
#endif

extern "C" int F_DECL_TEMPLATE_IS_CLASS(astDecl*);
#ifndef C_PARSER
extern "C" void ast_template_hook(astDecl* decl)
{
  int ii;
  int id_line = 0;
  int depth = set_len();
  if (depth == 0) return;
  if (F_DECL_TEMPLATE_IS_CLASS(decl)) {
    for(ii=0; ii<depth; ++ii)
      if ((yylsp[-ii].first_column == AGGR) &&
	  is_id(yylsp[-ii].last_column) &&
	  (yylsp[-(ii+1)].first_column == TEMPLATE)) {
	TREE_START_LINE(decl) =  yylsp[-(ii+1)].first_line;
	TREE_END_LINE(decl) = yylsp[-ii].last_line;
	id_line = yylsp[-ii].last_line;
	break;
      }
  }
  else {
    for(ii=0; ii<depth; ++ii)
      if ((yylsp[-ii].first_column == '{') &&
	  (yylsp[-ii].last_column == ')')) {
	TREE_END_LINE(decl) =  yylsp[-ii].last_line;
	id_line = ast_find_function_id();
	break;
      }
  }
}
#else
extern "C" void ast_template_hook(astDecl*)
{}
#endif

// start method
extern "C" void ast_method_hook(astDecl* decl, astTree* tt)
{
  int len = set_len(decl);
  if(len==0) return;

  if(tt){
    if( tt->TREE_CODE() == IDENTIFIER_NODE) 
      tt = 0;
    else if(tt==decl)
      tt = 0;
    else
      decl->start_line = tt->start_line;
  }

  while(tt && (tt->TREE_CODE() != CALL_EXPR))
     tt = ((astExp1*)tt)->TREE_OPERAND(0);
  if(tt)
    decl->set_DEBUG(tt->TREE_DEBUG());

  astType* tp = decl->type;
  astTree *id = decl->DECL_NAME();
  astTree * as_id = decl->DECL_ASSEMBLER_NAME();
#ifndef C_PARSER
  if (processing_template_decl || 0/*processing_template_defn*/)
    TREE_START_LINE(decl) = TREE_END_LINE(decl) = 0; 
  if (processing_template_defn) {
     decl->artificial_flag = 1;
  }
#endif

  DBG(decl){

    cout << "\n ast_method_hook yylen " << *yylen_ptr << " len " << len ;
    cout << " tt " << tt << endl;

    lineprint (decl);
    lineprint(tp);

    if(id) lineprint(id);
    if(as_id) lineprint(as_id);
    if(tt) lineprint(tt);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

// finish function
extern "C" void ast_function_hook(astDecl* decl, astDecl* id_decl, astTree* tt)
{
  // suppress hooks for when processing template
#ifndef C_PARSER
  if (processing_template_decl || 0/*processing_template_defn*/) {
    TREE_START_LINE(decl) = TREE_END_LINE(decl) = 0; 
    return;
  }
  if (get_processing_default_fn()) return;
  if (processing_template_defn) {
     decl->artificial_flag = 1;
  }
#endif
  int len = set_len(decl);
  if(len==0) return;

  if(tt){
    if( tt->TREE_CODE() == IDENTIFIER_NODE) 
      tt = 0;
    else if(tt==decl
	    )
      tt = 0;
#ifndef C_PARSER
    /* class A { inline A();} */
    else if (is_cons_or_destructor((astFunction_decl *)decl) && is_scspec(TREE_START_LINE(tt)))
      ;
#endif
    else
      decl->start_line = tt->start_line;
  }

  while(tt && (tt->TREE_CODE() != CALL_EXPR))
     tt = ((astExp1*)tt)->TREE_OPERAND(0);

  astType* tp = decl->type;
  astTree *id = decl->DECL_NAME();

  int id_line;

#ifndef C_PARSER
  if(is_destructor()) {
     id_line = - last_destructor_expr->TREE_DEBUG();
     last_destructor_expr = 0;
  } else
#endif

  if(tp->TREE_CODE() == FUNCTION_TYPE){
    if ((TREE_START_LINE(decl) <= (- decl->TREE_DEBUG())) &&
	(TREE_END_LINE(decl) >= (- decl->TREE_DEBUG())))
      id_line = - decl->TREE_DEBUG();
    else if(tt)
      id_line =  - tt->TREE_DEBUG();
#ifndef C_PARSER
    else if (last_call_expr && (TREE_START_LINE(last_call_expr) == TREE_START_LINE(decl)) &&
	     (TREE_END_LINE(last_call_expr) == TREE_END_LINE(decl)) &&
	     (decl->DECL_NAME() == ((astExp1 *)last_call_expr)->TREE_OPERAND(0)))
      id_line = - last_call_expr->TREE_DEBUG();
#endif
    else if(tp)
      id_line =  - tp->TREE_DEBUG();
    else 
      id_line = 0;

    if(id_line) 
      decl->set_DEBUG(-id_line);

    id_line = - decl->TREE_DEBUG();
  } else {   // METHOD_TYPE
      astTree * scope = DECL_CONSTRUCTOR(decl);      
      if ((TREE_START_LINE(decl) <= (- decl->TREE_DEBUG())) &&
	  (TREE_END_LINE(decl) >= (- decl->TREE_DEBUG())))
	id_line = - decl->TREE_DEBUG();
      else if(scope)
         id_line = TREE_END_LINE(scope);
      else {
           id_line = - decl->TREE_DEBUG();
         DBG(decl) {
            cout << "ast_function_hook: no scope for method" << endl;
         }
      }
  }

#ifndef C_PARSER
  if (id_line && ((id_line < TREE_START_LINE(decl)) || (id_line > TREE_END_LINE(decl)))) {
    decl->set_DEBUG(0);
    id_line = (tt && (tt->TREE_CODE() == CALL_EXPR)) ? (-tt->TREE_DEBUG()) : 0;
    if (id_line && ((id_line >= TREE_START_LINE(decl)) && (id_line <= TREE_END_LINE(decl))))
      decl->set_DEBUG(-id_line);
  }
  else
#endif  
   
  ast_update_id(id_line, id_decl, decl);

  DBG(decl){

    cout << "\n ast_function_hook yylen " << *yylen_ptr << " len " << len ;
    cout << " id_decl " << id_decl << " tt " << tt << endl;

    lineprint (decl);

    lineprint(tp);

    if(id_decl) lineprint(id_decl);
    if(tt) lineprint(tt);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

// finish field
extern "C" void ast_field_hook(astDecl * decl, astDecl * id_decl, astTree* tt)
{
  int len = set_len(decl);
  if(len==0) return;

  tt = call_expr_of_fp_decl(decl, tt);

  if(tt && tt->TREE_CODE() == IDENTIFIER_NODE) tt = 0;

  int id_line;
  int id_flag;

  astType* tp = decl->type;
  astType*ptp = 0;

  if(tp->TREE_CODE() == POINTER_TYPE){
      ptp = tp->type;
  } 
  int save_id_line = 0;
  astIdentifier * id = decl->DECL_NAME();
  if(id){
      char * name = decl->get_name();
      if(name[0] == JOINER)
          id_flag = 0;   
      else {
         id_line = ast_find_id(len, id);
	 save_id_line = id_line;
         id_flag = 1;
      }
  } else {
      id_flag = 0;
  }

  if(tt) { /* if(tt->TREE_CODE() == INDIRECT_REF) */
     id_line = - tt->TREE_DEBUG();
#ifndef C_PARSER
     if (tt->TREE_CODE() == CALL_EXPR) {
       astTree * ttt = ((astExp2*)tt)->TREE_OPERAND(0);
       if (ttt && (ttt->TREE_CODE() == SCOPE_REF))
	 id_line = TREE_END_LINE(ttt);
       else if (ttt && (ttt->TREE_CODE() == INDIRECT_REF))
	 id_line = - ttt->TREE_DEBUG();
     }
#endif
  }

  if(ptp && ptp->TREE_CODE() == FUNCTION_TYPE){
      if(tt && tt->TREE_CODE() == CALL_EXPR)
         decl->start_line = tt->start_line;
  } 

#ifndef C_PARSER
  if (id_flag && (id_line == 0))
    id_line = save_id_line ;
#endif

  if(id_flag)
      ast_update_id(id_line, id_decl, decl);
  else {  // union with no name
      decl->set_DEBUG(0);
      decl->start_line = 0;
      decl->end_line = 0;
  }
#ifndef C_PARSER
  if (processing_template_decl || 0/*processing_template_defn*/)
    TREE_START_LINE(decl) = TREE_END_LINE(decl) = 0;
  if (processing_template_defn) {
     decl->artificial_flag = 1;
  }
#endif

  DBG(decl){

    cout << "\n ast_field_hook yylen " << *yylen_ptr << " len " << len ;
    cout << " id_decl " << id_decl << " tp " << tp << " tt " << tt << endl;

    lineprint (decl);

    if(id_decl) lineprint(id_decl);
    if(tp) lineprint(tp);
    if(tt) lineprint(tt);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

// initialization of variable

static astDecl * last_var_decl = 0;

extern "C" void ast_adjust_last_var_decl()
{
  if (last_var_decl && (yylloc.first_column == ';'))
    for (int ii = 0; ii < *yylen_ptr ; ++ii)
      if ( (TREE_START_LINE(last_var_decl) == yylsp[-ii].first_line) &&
	   (TREE_END_LINE(last_var_decl) == yylsp[-ii].last_line)) {
#if 0 /* 10.25.95 * NY */
	last_var_decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
#else
	last_var_decl->set_linenum (yylloc.first_line);
#endif
	break;
      }
}

#ifndef C_PARSER
extern "C" void ast_init_value_var_hook(astDecl * var_d, astTree * expr)
{
  if (var_d == 0 || expr == 0) return;
  if ((var_d->TREE_CODE() != VAR_DECL) || (expr->TREE_CODE() != ADDR_EXPR)) return;
  int id_line = ast_find_id_from_depth(*yylen_ptr);
  if ((id_line == 0) || (id_line <= TREE_START_LINE(var_d)) || 
      (id_line > TREE_END_LINE(var_d))) return;
  astTree * ttt = ((astExp1*)expr)->TREE_OPERAND(0);
  if (ttt && (ttt->TREE_CODE() == FUNCTION_DECL))
    ast_put_ref_ids(id_line, my_input_filename, (astDecl *)ttt, ((astDecl *)ttt)->DECL_ASSEMBLER_NAME());
}

#endif

inline int is_static_data_member(astDecl *decl)
{
  return (decl && decl->TREE_CODE() == VAR_DECL &&
	  decl->DECL_CONTEXT() &&
	  decl->DECL_CONTEXT()->TREE_CODE() == RECORD_TYPE);
}

extern "C" void ast_init_var_hook(astDecl * decl)
{
  if(!decl)
    return;
  int len = set_len(decl);
  if(len==0) return;


  int ll = - (*yylen_ptr - 1);
  last_var_decl = decl;

  int id_line = ast_find_id_from_depth(*yylen_ptr);
  astIdentifier *id = decl->DECL_NAME();
  if (id && (decl->TREE_DEBUG() == -id_line) && (cur_id_node != id)) {
    ast_put_ref_ids( id_line, my_input_filename, decl, id);
    if (saving_cur_id) {
       cur_id_node = id;
       cur_id_line = id_line;
       cur_id_decl = decl;
    }
  }

  decl->start_line = yylsp[ll].first_line;
  decl->end_line = yylsp[0].last_line;

#if 0 /* 10.25.95 * NY */
  decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
#else
  decl->set_linenum (yylloc.first_line);
#endif

  /* catch static data member here since it is not going to be 
  on list of global vars in toplevi */
  if (is_static_data_member(decl) && decl->DECL_INITIAL()) {
    ast_set_traverse_expr_hook(decl);
    traverse_expr(decl->DECL_INITIAL());
    ast_set_traverse_expr_hook(0);
  }

  DBG(decl){

    cout << "\n ast_init_var_hook yylen " << *yylen_ptr << " len " << len 
                                                       << " decl" << endl;
    lineprint (decl);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

// default value of parm

extern "C" void ast_init_parm_hook(astDecl * decl, astTree *tl)
{
  int len = set_len(decl);
  if(len==0) return;

  if(decl->TREE_CODE() != PARM_DECL){
     cout << "ast_init_parm_hook: WRONG type " << endl;
     lineprint(decl);
     return;
  }

  decl->end_line = tl->end_line;

  DBG(decl){

    cout << "\n ast_init_parm_hook yylen " << *yylen_ptr << " len " << len 
                                                       << " decl" << endl;
    lineprint (decl);
    if(tl) lineprint (tl);
    ast_hook_prt(*yylen_ptr + 2);
  }
}

// reference of a field of struct (x.a or x->a)
extern "C" void ast_component_hook(astExp2* exp, astDecl* id_decl) 
{
  int len = set_len();
  if(len==0) return;

// do not report field initialization in constructors

#ifndef C_PARSER  
  if(yylloc.first_column=='{')
    if(yylsp[0].last_column==')')
       return;
#endif

  astDecl* decl = (astDecl*)exp->TREE_OPERAND(1);
#if 1
  int id_line = ast_find_id();
  exp->set_DEBUG(-id_line);
  ast_update_id(id_line, id_decl, decl, 1);
#else
  astIdentifier* id = decl->DECL_NAME();
  ast_id_hook_int(id, decl, 1);
#endif
   DBG(decl){
     astType* tp = decl->type;

     cout << "\n ast_component_hook yylen " << *yylen_ptr << " len " << len 
                                                        << endl;
     lineprint (decl);
     lineprint (id_decl);
     lineprint (tp);
 
     ast_hook_prt(*yylen_ptr + 2);
   }
}


void dd_swap_func_decl(astDecl * f1, astDecl * f2)
{
  if(is_untokenized_tree(f1) && is_untokenized_tree(f2)) return;

  astTree* decl;
  int f1_i = -1;
  int f2_i = -1;
  for (int i = decl_spec.size() -1; 
	(((f1_i < 0) || (f2_i < 0)) && (i>0)); 
	i -= 4) {

    decl = *(astDecl**)decl_spec[i];

    if (decl == f1)
      f1_i = i;
    if (decl == f2)
      f2_i = i;
   }
   if ((f1_i >= 0) && (f2_i >= 0)) {
     decl = *(astDecl**)decl_spec[f1_i];
     *(decl_spec[f1_i]) = *decl_spec[f2_i];
     *(decl_spec[f2_i]) = decl;
   }
   else if ((f1_i < 0) && (f2_i >= 0)) {
     decl = *(astDecl**)decl_spec[f2_i];
     if (TREE_START_LINE(decl) < 1) 
       *(decl_spec[f2_i]) = f1;
   }
}

void ast_swap_specs(astDecl * f1, astDecl * f2)
{
  if(is_untokenized_tree(f1) && is_untokenized_tree(f2)) return;

  astDecl * decl;
  int f1_i = -1;
  int f2_i = -1;
  for (int i = decl_spec.size() -1; 
	(((f1_i < 0) || (f2_i < 0)) && (i>0)); 
	i -= 4) {

    decl = *(astDecl**)decl_spec[i];

    if (decl == f1){
        f1_i = i;
        *decl_spec[i] = f2;
    } else if (decl == f2){
        f2_i = i;
        *decl_spec[i] = f1;
    }
 }
}

extern "C" void ddPushFunctionPair(astFunction_decl *, astFunction_decl *);


extern "C" void ast_swap_tree_info(astTree * oldtree, astTree * newtree)
  {
    
//  if(is_untokenized_tree(oldtree) && is_untokenized_tree(newtree)) return;
    int st_l = TREE_START_LINE (newtree);
    int en_l = TREE_END_LINE (newtree);
    char *file_nm = TREE_FILENAME (newtree);
    int dbg  = newtree->TREE_DEBUG();
    int aset_f = TREE_ASET_FLAG(newtree);

    TREE_START_LINE (newtree) = TREE_START_LINE (oldtree);
    TREE_END_LINE (newtree) = TREE_END_LINE (oldtree);
    TREE_FILENAME (newtree) = TREE_FILENAME (oldtree);
    newtree->set_DEBUG(oldtree->TREE_DEBUG());
    TREE_ASET_FLAG(newtree) =TREE_ASET_FLAG(oldtree);

    TREE_START_LINE (oldtree) = st_l;
    TREE_END_LINE (oldtree) = en_l;
    TREE_FILENAME (oldtree) = file_nm;
    oldtree->set_DEBUG(dbg);
    TREE_ASET_FLAG(oldtree) = aset_f;
}

extern "C" void ast_swap_decl_info(astDecl * olddecl, astDecl * newdecl)
  {

    //  if(is_untokenized_tree(olddecl) && is_untokenized_tree(newdecl)) return;

    ast_swap_tree_info(olddecl, newdecl);

#if 0 /* 11.06.95 */
    int decl_ln = newdecl->DECL_SOURCE_LINE();
    //    char * decl_fl = newdecl->DECL_SOURCE_FILE();
    newdecl->SET_DECL_SOURCE_LINE(olddecl->DECL_SOURCE_LINE());
    //    newdecl->SET_DECL_SOURCE_FILE(olddecl->DECL_SOURCE_FILE());

    olddecl->SET_DECL_SOURCE_LINE(decl_ln);
#else
    int decl_ln = newdecl->get_linenum ();
    newdecl->set_linenum (olddecl->get_linenum ());
#endif
    //    newdecl->SET_DECL_SOURCE_FILE(olddecl->DECL_SOURCE_FILE());

    olddecl->set_linenum (decl_ln);
    //    olddecl->SET_DECL_SOURCE_FILE(decl_fl);
#if 0
    if (olddecl && (olddecl->TREE_CODE() == TYPE_DECL))
      last_type_decl = olddecl;
#endif

  }

extern "C" void ast_init_array_class_var_hook
 (astDecl * olddecl,  astDecl * newdecl, astTree*tt)
{

  DBG(newdecl){
       cout << " ast_init_array_class_var_hook  old new tt" << endl;
       lineprint (olddecl);
       lineprint (newdecl);
       lineprint(tt);
 
       ast_hook_prt(*yylen_ptr + 2);
  }
}

static void ast_move_decl_hook(astDecl * olddecl,  astDecl * newdecl)
{
  if(is_untokenized_tree(olddecl) && is_untokenized_tree(newdecl)) return;

  DBG(newdecl){
       cout << " ast_move_decl_hook  old new " << endl;
       lineprint (olddecl);
       lineprint (newdecl);
  }

  ast_swap_decl_info(olddecl, newdecl);

  ast_swap_specs(olddecl, newdecl);
  
  if ( TREE_FILENAME(olddecl) != TREE_FILENAME(newdecl) )
    if ( ast_is_global_level() )
      ast_put_global_decl (olddecl);
}

static int decl_is_static_data_member(astDecl* decl)
{
 return ( (decl->TREE_CODE() == VAR_DECL) &&
	  decl->DECL_CONTEXT() &&
	  (decl->DECL_CONTEXT()->TREE_CODE() == RECORD_TYPE));
}

extern "C" void ast_move_var_hook(astDecl * olddecl,  astDecl * newdecl)
{

  if(is_untokenized_tree(olddecl) && is_untokenized_tree(newdecl)) return;

  if(olddecl->TREE_CODE() != VAR_DECL){
      cout << "ast_move_var_hook:*** error: WRONG old TYPE ****" << endl;
      lineprint (olddecl);
  }

  if(newdecl->TREE_CODE() != VAR_DECL){
      cout << "ast_move_var_hook:*** error: WRONG new TYPE ****" << endl;
      lineprint (newdecl);
  }

  int len = set_len();
  if(len==0) return;

// var hook was not called from the parser
  if(newdecl!=last_hooked_var)
       ast_var_hook(newdecl, olddecl, 0);

  ast_move_decl_hook(olddecl,  newdecl);
#ifndef C_PARSER
  if(decl_is_static_data_member(newdecl) && DECL_LANG_SPECIFIC(newdecl))
    {
      DECL_MAIN_VARIANT(newdecl) = newdecl;
      ast_put_global_decl(olddecl);
    }
#endif
}

extern "C" void ast_scope_var_hook( astDecl * olddecl,  astDecl * newdecl){
  int len = set_len(2);
  if(len==0) return;

  int  id_line = ast_find_id(3);

  ast_update_id (id_line, olddecl, newdecl, 1);
 
   DBG(newdecl){
    astType* tp = newdecl->type;

    cout << "\n ast_scope_var_hook ID " << id_line << " old new tp" << endl;
    lineprint (olddecl);
    lineprint (newdecl);
    lineprint (tp);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

extern "C" void ast_move_function_hook(astFunction_decl * olddecl, 
                                            astFunction_decl * newdecl)
{

  if(is_untokenized_tree(olddecl) && is_untokenized_tree(newdecl)) return;

  if(olddecl->TREE_CODE() != FUNCTION_DECL){
      cout << "ast_move_hook:*** error: WRONG old TYPE ****" << endl;
      lineprint (olddecl);
  }

  if(newdecl->TREE_CODE() != FUNCTION_DECL){
     cout << "ast_move_hook:*** error: WRONG new TYPE ****" << endl;
     lineprint (newdecl);
  }

  if (olddecl->TREE_CODE () != FUNCTION_DECL 
              || newdecl->TREE_CODE () != FUNCTION_DECL)
  return;

  ast_move_decl_hook(olddecl,  newdecl);

  int len = set_len();
  if(len==0) return;

  astType* oldtp = olddecl->type;
  astType* newtp = newdecl->type;
  
  if(oldtp != newtp) DBG(newdecl) {
     cout << "ast_move_function_hook: oldtp != newtp" << endl;
     lineprint(oldtp);
     lineprint(newtp);
  }

#ifndef C_PARSER
  if(oldtp->TREE_CODE() == METHOD_TYPE){
      astTree * oldscope = DECL_CONSTRUCTOR(olddecl);      
      astTree * newscope = DECL_CONSTRUCTOR(newdecl);      
 
      DECL_CONSTRUCTOR(olddecl) = newscope;
      DECL_CONSTRUCTOR(newdecl) = oldscope;
      
  }
#endif

  // ddPushFunctionPair(olddecl, newdecl);
}

void ast_hook_prt(int nn){

  int ii;
  int len = yylsp - yyls + 1;
  if(nn > len) nn = len;
  if(current_function_decl){
    cout << "cur_fun_decl ";
    lineprint(current_function_decl);
  }
  cout << "AST_STACK: " << 
       "cur_id_line " << cur_id_line << " yylen " << *yylen_ptr << endl;
  cout << "   ";
  cout << yylloc.first_line << ' ' << ast_type_str(yylloc.first_column) << ' ';
  cout << yylloc.last_line << ' ' << ast_type_str(yylloc.last_column) << ' ';
  cout << yylloc.text << " token " << yylloc.timestamp << endl;

  for(ii=0; ii<nn; ++ii){ 
       cout << yylsp[-ii].first_line << ' ' << 
                  ast_type_str(yylsp[-ii].first_column) << ' ';
       cout << yylsp[-ii].last_line << ' ' << 
                  ast_type_str(yylsp[-ii].last_column) << ' ';
       cout << yylsp[-ii].text << " token " << yylsp[-ii].timestamp << endl;
  }

}

extern "C" void ast_hook_start_langc(){}
extern "C" void ast_hook_end_langc(){}
void ast_init_pairlist ();
extern "C" void parser_free (void*);

extern "C" void ast_init_hook ()
{
  ast_call_init();
  ast_file_init();
  ast_init_ref_ids ();
  ast_init_pairlist ();
  ast_decl_spec_init();
  ast_extern_lang = 0;
}
  
int ast_hook_strcmp(char *f1, char *f2)
{
  if (f1 == f2) return 0;
  if (f1 == 0 || f2 == 0) return 1;

  while (*f1 && *f2 && (*f1 != ':')) {
    if (*f1 != *f2) return *f1 - *f2;
    f1 ++;
    f2 ++;
  }
  return 0;
}

// struct A;
// enum B {} ;
extern "C" void ast_semicolon_hook(astDecl * decl){
 if(is_untokenized_tree(decl)) return;

//  if(TREE_ASET_FLAG(decl) == 0)
//       return;
//  if(! decl->DECL_SOURCE_LINE())
//       return;

  if((decl == last_type_decl) && (TREE_ASET_FLAG(decl) == 1)){

#if 0 /* 10.25.95 * NY */ /* bad since parser uses decl_source_line */
      decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
#else
      decl->set_linenum (yylloc.first_line);
#endif

      TREE_ASET_FLAG(decl) = 0;
  }
#ifndef C_PARSER
  else {
  int depth =  *yylen_ptr;
  for(int ii=0; ii < depth ; ++ii)
    if ((yylsp[-ii].first_line == TREE_START_LINE(decl)) && 
	(yylsp[-ii].last_line == TREE_END_LINE(decl))) {
#if 0  /* 10.25.95 * NY */ /* bad since parser uses decl_source_line */
	decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
#else
	decl->set_linenum (yylloc.first_line);
#endif
	TREE_ASET_FLAG(decl) = 0;
	break;
    }
  }
 if (decl && last_type_decl && (decl != last_type_decl) && decl->TREE_TYPE() &&
     (decl->TREE_TYPE() == last_type_decl->TREE_TYPE()))
   if (ast_hook_typedecl_is_def ((astDecl *)last_type_decl))
     ast_swap_decl_info(decl, (astDecl *) last_type_decl);
#endif

  DBG(decl){

    cout << "\n ast_semicolon_hook yylen " << *yylen_ptr << 
                             " decl last_decl" << endl;

    lineprint (decl);

    lineprint (last_type_decl);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

// arr = {...};
extern "C" void ast_var_semicolon_hook(astDecl * decl){

 if(is_untokenized_tree(decl)) return;

//  if(TREE_ASET_FLAG(decl) == 0)
//       return;
#if 0 /* 10.25.95 */ /* bad since parser uses decl_source_line */
  decl->SET_DECL_SOURCE_LINE(yylloc.first_line);
#else
  decl->set_linenum (yylloc.first_line);
#endif
//  decl->SET_DECL_SOURCE_FILE(0);
//  TRE_ASET_FLAG(decl) = 0;
 
  DBG(decl){

    cout << "\n ast_var_semicolon_hook yylen " << *yylen_ptr << endl;

    lineprint (decl);

    ast_hook_prt(*yylen_ptr + 2);
  }
}

struct pairlist
{
  pairlist * next;
  astFunction_decl *p1;
  astFunction_decl *p2;
};

static pairlist *declist = NULL;
extern "C" char* parser_malloc(size_t);
void ddPushFunctionPair(astFunction_decl *fdef, astFunction_decl *fdec)
{
  pairlist *p = (pairlist*)parser_malloc(sizeof(pairlist));
  p->p1 = fdef;
  p->p2 = fdec;
  p->next = declist;
  declist = p;
}

void ast_init_pairlist ()
{
  for (struct pairlist* p = declist; p; p = declist)
    {
      declist = p->next;
      parser_free (p);
    }
}

//
// returns: -1: error, >=0: the number of removed declarations
//
extern "C" int
ast_type_rm_local_or_const(treeArr * decl_arr)
{
    int ret_val = -1;

    if (decl_arr) {
 	ret_val = 0;

	int dest = 0;
	int size = decl_arr->size();
	for (int src=0; src<size; src++) {
	    astTree ** decl_src = (astTree **)((*decl_arr)[src]);
	    if ((ast_type_not_supported(*decl_src)<=0) &&
		((*decl_src)->TREE_CODE()!=CONST_DECL)) {
		if (dest != src)
		    *((astTree **)((*decl_arr)[dest])) = *decl_src;
		dest++;
	    } else
		ret_val++;
	}
	if (dest < size)
	    decl_arr->shrink(dest, size-dest);
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of removed declarations
//
extern "C" int
ast_type_rm_local_calls(treeArr * decl_arr)
{
    int ret_val = -1;

    if (decl_arr) {
 	ret_val = 0;

	int dest = 0;
	int blck = 0;
	int size = decl_arr->size();
	for (int src=0; src<size; src++) {
	    astTree ** decl_src = (astTree **)((*decl_arr)[src]);
	    if (ast_type_not_supported(*decl_src) <= 0) {
		if (dest != src)
		    *((astTree **)((*decl_arr)[dest])) = *decl_src;
		dest++;
	    } else {
		dest    -= blck;
		src     += (2-blck);
		ret_val += 3;
	    }
	    blck++;
	    if (blck == 3)
		blck = 0;
	}
	if (dest < size)
	    decl_arr->shrink(dest, size-dest);
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of removed declarations
//
extern "C" int
ast_type_rm_local_decls(treeArr * decl_arr)
{
    int ret_val = -1;

    if (decl_arr) {
 	ret_val = 0;

	int dest = 0;
	int size = decl_arr->size();
	for (int src=0; src<size; src+=4) {
	    if (ast_type_not_supported(*((astTree **)((*decl_arr)[src+3]))) <= 0) {
		if (dest != src) {
		    for (int i=0; i<4; i++)
			*((astTree **)((*decl_arr)[dest+i])) = *((astTree **)((*decl_arr)[src+i]));
		}
		dest += 4;
	    } else
		ret_val += 4;
	}
	if (dest < size)
	    decl_arr->shrink(dest, size-dest);
    }

    return ret_val;
}

//
// returns: -1: error, >=0: the number of removed declarations
//
extern "C" int
ast_type_rm_local_refs(identArr * idents)
{
    int ret_val = -1;

    if (idents) {
	ret_val = 0;

	int dest = 0;
	int size = idents->size();
	for (int src=0; src<size; src++) {
	    ident * decl_src = (*idents)[src];
	    if (ast_type_not_supported(decl_src->decl) <= 0) {
		if (dest != src)
		    *((*idents)[dest]) = *decl_src;
		dest++;
	    } else
		ret_val++;
	}
	if (dest < size)
	    idents->shrink(dest, size-dest);
    }

    return ret_val;
}

//
// returns: >=0: the number of removed declarations
//
extern "C" int
ast_type_rm_local(struct ast_shared_hook * hook)
{
    int ret_val = 0;

    // -- get the hooks
    treeArr * globals = (treeArr *)0;
    treeArr * calls   = (treeArr *)0;
    treeArr * decls   = (treeArr *)0;
    identArr *idents  = (identArr *)0;
    if (hook) {
	globals = (treeArr *)(&hook->file_globals);
	calls   = (treeArr *)(&hook->call_globals);
	decls   = (treeArr *)(&hook->decl_spec);
	idents  = (identArr *)(&hook->ref_ids);
    } else {
	globals = &file_globals;
	calls   = &call_globals;
	decls   = &decl_spec;
	idents  = &ref_ids;
    }

    // -- clean declarations up
    if (globals)
	ret_val += ast_type_rm_local_or_const(globals);
    if (calls)
	ret_val += ast_type_rm_local_calls(calls);
    if (decls)
	ret_val += ast_type_rm_local_decls(decls);
    if (idents)
	ret_val += ast_type_rm_local_refs(idents);
    // -- debugging information
    if (hook_debug) {
	cout << "\nast_type_rm_local(...) excluded " << ret_val << endl;
    }

    return ret_val;
}

extern "C" void assign_ast_shared_hook (struct ast_shared_hook *p)
{
  declist = 0; //p->declist;
  OS_dependent::bcopy (&p->file_globals, &file_globals, sizeof (file_globals));
  OS_dependent::bcopy (&p->call_globals, &call_globals, sizeof (call_globals));
  OS_dependent::bcopy (&p->ref_ids, &ref_ids, sizeof (ref_ids));
  OS_dependent::bcopy (&p->decl_spec, &decl_spec, sizeof(decl_spec));
  if (!no_init_hook)
    ast_init_hook ();	
}

extern "C" void save_ast_shared_hook (struct ast_shared_hook *p)
{
  p->declist = 0; //declist;
  OS_dependent::bcopy (&file_globals, &p->file_globals, sizeof (file_globals));
  OS_dependent::bcopy (&call_globals, &p->call_globals, sizeof (call_globals));
  OS_dependent::bcopy (&ref_ids, &p->ref_ids, sizeof (ref_ids));
  OS_dependent::bcopy (&decl_spec, &p->decl_spec, sizeof(decl_spec));
}

extern "C" void ast_hook_parmlist (astTree*result, astTree*valtail)
{  
  TREE_START_LINE (result) = TREE_START_LINE (valtail);
  TREE_END_LINE (result) = TREE_END_LINE (valtail);
  TREE_FILENAME (result) = TREE_FILENAME (valtail);
}

extern "C" void ast_finish_hook()
{
    // -- remove local types, type_decls, etc.
    (void) ast_type_rm_local((struct ast_shared_hook *)0);
 if(hook_report){
  ast_globals_prt();
  ast_specs_prt();
  ast_calls_prt();
  ast_ref_ids_prt(); 
 }
}

extern "C" void ast_set_default_lines_hook (astTree * node)
{
  if (node)
    TREE_START_LINE (node) = TREE_END_LINE (node) = 0;
}

extern "C" astTree * get_current_decl ()
{
  if (current_function_decl)
    return current_function_decl;
#ifndef C_PARSER
  else
    if (current_class_type){
      astTree * tname = ((astType*)current_class_type)->TYPE_NAME();
      if (tname && tname->TREE_CODE() == TYPE_DECL)
	return tname;
      else
	if (TYPE_STUB_DECL (current_class_type) &&
	    TYPE_STUB_DECL (current_class_type)->TREE_CODE() == TYPE_DECL)
	return TYPE_STUB_DECL (current_class_type);	  
      }
#endif
  return 0;
}

extern "C" void ast_temp_hook (astTree* node, int  flag)
{
    // flag = 0 if node is decl of temp var in stack memory
    // flag = 1 if node is decl of  temp var in static memory
    // flag = 3 if node is decl of temp var with two __

    if (!node)
	return;

    DBG(node) {
	cout << "ast_temp_hook flag " << flag << endl;
	lineprint (node);
    }

    ast_set_default_lines_hook (node);
}
    

void lineprint (astTree *node, int nl)
{
  if (!node) return; 
  astIdentifier * id;

  int dbgf = (hook_gcc && (TREE_START_LINE(node) == 0)) ? 0 : node->TREE_DEBUG();
  int dll = (hook_gcc && (TREE_START_LINE(node) == 0)) ? 0 : 1;
  char * dfn = (hook_gcc && (TREE_START_LINE(node) == 0)) ? "(inc)" : trn(TREE_FILENAME(node));
  enum tree_code node_code = node->TREE_CODE();

  cout << TREE_START_LINE(node) << ' ' << TREE_END_LINE(node) <<
         ' ' << dfn <<
   ' ' << node << ' ' << tree_code_name[node_code] <<       
     " dbg " << dbgf << " flg " << (boolean) TREE_ASET_FLAG(node);

  switch(node_code){
    case FUNCTION_DECL:
    case VAR_DECL:
    case CONST_DECL:
    case FIELD_DECL:
    case TYPE_DECL:
#ifndef C_PARSER
  case TEMPLATE_DECL:
#endif

    case PARM_DECL:
    {
         astDecl * decl = (astDecl*)node;
#if 0 /* 10.25.95 * NY */
         int decl_line = (dll) ? decl->DECL_SOURCE_LINE() : 0;
#else
         int decl_line = (dll) ? decl->get_linenum () : 0;
#endif
         cout << " dl " << decl_line;

         if(node_code==PARM_DECL)
            cout << " pl " << (int)decl->TREE_PREV();

         id = decl->DECL_NAME();
         if(id){
             cout << " nm " << decl->get_name() << " op " << is_operator(decl);
         } else {
              cout << " id " << id ;
         }
//         cout << " U " << decl->used_flag;
         break;
    }

    case IDENTIFIER_NODE:
         id = (astIdentifier*) node;
         cout << " pnt " << id->IDENTIFIER_POINTER();
         break;
  }
  if (nl)
    cout << endl;
  else 
    cout << " // ";
}

extern "C" void lineprint (astTree *node)
{
  lineprint(node, 1);

}

extern "C" void ast_constr_new_hook(astTree * node)
{
 int len = set_len(1);
 if(len==0) return;


  int nn = *yylen_ptr - 1;
  if(nn<0) nn=0;

  node->set_DEBUG(-yylsp[-(nn)].first_line);
  TREE_ASET_FLAG(node) = 1;

 DBG(node){
    cout << "ast_constr_new_hook " << endl;
    lineprint(node);
    ast_hook_prt(*yylen_ptr + 2);
  }
}

extern "C" void ast_constr_constr_hook(astTree * node)
{

 int len = set_len(1);
 if(len==0) return;


  int nn = *yylen_ptr - 1;
  if(nn<0) nn=0;

  int new_with_arg = 0;   // new(arg) AAA();
  int do_arg = 0;
  for(int jj=1; jj<nn; ++jj){
    int tp = yylsp[-(nn-jj)].first_column;
    if(!do_arg){
       if(tp=='('){
          new_with_arg = do_arg = 1;
          continue;
       } else if(tp == IDENTIFIER) {  // class
                node->set_DEBUG(-yylsp[-(nn-jj)].first_line);
                TREE_START_LINE(node) = yylsp[-(nn-jj)].first_line;
                TREE_ASET_FLAG(node) = 1;
                break;                
       }
   } else if (tp==')') {
      do_arg = 0;
   }
 }
 DBG(node){
    cout << "ast_constr_constr_hook with_arg " << new_with_arg << endl;
    lineprint(node);
    ast_hook_prt(*yylen_ptr + 2);
  }
}

extern "C" void ast_constr_conversion_hook(astTree * node)
{
  node->start_line = 0;
  node->end_line = 0;

 DBG(node){
    cout << "ast_constr_conversion_hook " << endl;
    lineprint(node);
    ast_hook_prt(*yylen_ptr + 2);
  }
}

extern "C" void ast_global_var_hook(astTree * node)
{
  if (!node) return;
  if (node->TREE_CODE() == VAR_DECL || node->TREE_CODE() == CONST_DECL ||
       node->TREE_CODE() == FUNCTION_DECL || node->TREE_CODE() == FIELD_DECL) {
    if (set_len() == 0 && !is_global_call()) return;
    astDecl * call_context = get_global_traverse_expr();
    if (call_context && is_untokenized_file(TREE_FILENAME(call_context)))
      return;
    /* temp. abusing call_global */
    if ((node != call_context) && call_context && call_context->TREE_CODE() == VAR_DECL) {
      call_globals.append ((tree*)&node);
      call_globals.append ((tree*)&call_context);
      call_globals.append ((tree*)&node);
    }
  }
}

extern "C" void ast_def_var_decl_hook(astDecl * decl)
{
    if (decl && TREE_FILENAME(decl))
	if (is_untokenized_tree (decl) && is_untokenized_file (TREE_FILENAME(decl))) {
	    DBG(decl){
		cout << "ast_def_var_decl_hook yylloc=" << yylloc.first_line << endl;
		lineprint(decl);
	    }
	    file_globals.append((tree*)&decl);
	}
}

#ifndef C_PARSER
extern "C" astTree * my_lookup_name(astTree *, int);
extern "C" void ast_overqualified_id_hook(astTree * d1, astTree * d, astTree *idn)
{
  
  if (d1->TREE_CODE () == IDENTIFIER_NODE) {
    astTree * var_d = my_lookup_name(d1, -2);
    ast_scope_var_hook ((astDecl*) var_d, (astDecl *)d);
  }
  /* T.D. 09.29.94 */
  if (idn && (idn->TREE_CODE() == IDENTIFIER_NODE)) {
    char *st = ((astIdentifier*)idn)->IDENTIFIER_POINTER();
    if (!st || (st[0] == '_' && st[1] == '_')) ;

    else if ((d->TREE_CODE() == CONST_DECL) || (d->TREE_CODE() == VAR_DECL) || (d->TREE_CODE() == TYPE_DECL))
      ast_id_hook((astIdentifier *)idn, (astDecl *) d, 0);

    else if ((d->TREE_CODE() == COMPONENT_REF) || (d->TREE_CODE() == OFFSET_REF) || 
	     (d->TREE_CODE() == ARRAY_REF) || (d->TREE_CODE() == CP_OFFSET_REF))
      ast_id_hook((astIdentifier *)idn, (astDecl *) ((astExp2 *)d)->TREE_OPERAND(1), 0);
  }
}


extern "C" void ast_overqualified_id_hook1(int call_id, astTree *d, astIdentifier *idn)
{
  if (call_id == 0) return;
#if 0
  astTree *fdec = ((astExp1 *)d)->TREE_OPERAND (0);
  if (fdec) {
    if (is_decl(fdec->TREE_CODE()))
      ast_id_hook(idn, (astDecl *) fdec, 0);
    else {
      fdec = ((astExp1 *)fdec)->TREE_OPERAND (0);
      if (fdec && fdec->TREE_CODE() == FUNCTION_DECL)
	ast_id_hook(idn, (astDecl *) fdec, 0);  
    }
  }
#endif
  d->set_DEBUG(-call_id);
}

extern "C" void ast_notype_qualified_id(astTree *d, astTree *d2)
{
  if (d  && d->TREE_CODE() == SCOPE_REF &&
      d2 && d2->TREE_CODE() == IDENTIFIER_NODE) {
    astTree *fdec = ((astExp1 *)d)->TREE_OPERAND (0);
    astTree *var_d = ((astIdentifier *)d2)->IDENTIFIER_CLASS_VALUE();
    if (var_d && (var_d->TREE_CODE() == VAR_DECL))
      ast_id_hook((astIdentifier *) d2, (astDecl *) var_d, 0 );
  }
}

extern "C" void ast_nested_name_specifier(astTree *idn)
{
  if (!idn || (idn->TREE_CODE() != IDENTIFIER_NODE)) return;
  astDecl * idn_decl = (astDecl *) ((astIdentifier*)idn)->IDENTIFIER_GLOBAL_VALUE();
  if (idn_decl == 0)
    idn_decl = (astDecl *) ((astIdentifier*)idn)->IDENTIFIER_LOCAL_VALUE();
  if (idn_decl && idn_decl->TREE_CODE() == TYPE_DECL)
    ast_id_hook((astIdentifier *) idn, idn_decl, 0);
}

int get_code_from_id(astIdentifier *idn)
{
  if (!idn) return 0;
  char * name = idn->IDENTIFIER_POINTER();
  if (name && (strlen(name) > 2) && (name[0] == '_') && (name[1] == '_'))
    return (- idn->TREE_DEBUG());
  return 0;
}

inline int get_ambig_code(int code)
{
  if (code == LEFT_RIGHT)
    return (int) ')';
  else 
    return 0;
}

extern "C" void set_id_line_for_op_expr(astExp3 * expr, astIdentifier * idn)
{
  if (!expr || !idn || (idn->TREE_CODE() != IDENTIFIER_NODE) ||      (expr->TREE_CODE() != CALL_EXPR) || (expr->TREE_CODE() != NEW_EXPR))
    return;
  int col = get_code_from_id(idn);
  if (!col) return;
  int id_line = 0;
  int depth = (*yylen_ptr >= 3) ? set_len(*yylen_ptr): set_len();
  for(int ii=0; ii<depth; ++ii)
    if (id_line = check_op_id(col, ii))
      break;
  expr->set_DEBUG(-id_line);
}

extern "C" void set_default_line_for_op_expr(astTree *var, astTree * expr, astTree *idn)
{
  if (!var || !expr || !idn) return;
  if (!(var->TREE_CODE() == VAR_DECL || var->TREE_CODE() == CALL_EXPR || var->TREE_CODE() == INDIRECT_REF)
      || expr->TREE_CODE() != CALL_EXPR
      || idn->TREE_CODE() != IDENTIFIER_NODE) return;
  
  char *id_name = ((astIdentifier *)idn)->IDENTIFIER_POINTER();
  /* 4 == "__op" */
  if (!id_name || (strlen(id_name) < 5) || !(id_name[0] == '_' && id_name[1] == '_' && 	
					     id_name[2] == 'o' && id_name[3] == 'p'))
    return;
  ast_set_default_lines_hook (expr);
  expr->set_DEBUG(0);
}

extern "C" void ast_nested_type_hook(astIdentifier * idn)
{
  if (!idn || (idn->TREE_CODE() != IDENTIFIER_NODE)) return;

  astDecl * decl = (astDecl *) idn->IDENTIFIER_CLASS_VALUE();
  if (!decl)
    decl = (astDecl *) idn->IDENTIFIER_GLOBAL_VALUE();
  if (decl && (decl->TREE_CODE() == TYPE_DECL))
    ast_id_hook(idn, decl, 0);
  astType *idn_type = (astType *) idn->TREE_TYPE() ; /* IDENTIFIER_TYPE_VALUE(); */

  int id_line = 0;
  if (idn_type && idn_type->TYPE_BUILT_IN()) {
    if((yylloc.first_line == yylloc.last_line)
       && (yylloc.first_column == yylloc.last_column)) {
      id_line = ast_find_id(3);	
      if ((id_line == 0) && (*yylen_ptr > 3))
	id_line = ast_find_id_from_depth(*yylen_ptr);
    }
    else
      id_line = yylloc.first_line;
    if ( id_line){	
       if (saving_cur_id) {
	  cur_id_node = idn;
	  cur_id_line = id_line;
       }
      ident * nid = ref_ids.grow(1);

      nid-> lineno = id_line;
      nid-> filename = my_input_filename;
      nid-> decl = (astDecl *) idn;
      nid-> id = idn;
      nid->cur_decl = (astDecl*)get_current_decl();
      ref_ids_hash.add(nid); // sturner, 980511
    }
  }
}

extern "C" void ast_default_nw_expr(astTree *exp)
{
  if (exp->TREE_CODE() == TARGET_EXPR) {
    astTree *pval = ((astExp2 *)exp)->TREE_OPERAND(1);
    if (pval && (pval->TREE_CODE() == NEW_EXPR))
      ast_set_default_lines_hook (pval);
  }
}

extern "C" void ast_hook_after_type_component_declarator0(astTree *idn, astDecl *decl)
{
  if (!idn || !decl) return;
  if ((idn->TREE_CODE() != IDENTIFIER_NODE) || (decl->TREE_CODE() != TYPE_DECL)) return;

  int id_line = 0;
  if((yylloc.first_line == yylloc.last_line)
     && (yylloc.first_column == yylloc.last_column)) {
    id_line = ast_find_id(3);	
    if ((id_line == 0) && (*yylen_ptr > 3))
      id_line = ast_find_id_from_depth(*yylen_ptr);
  }

  int ref_ids_size = ref_ids.size();
  if (ref_ids_size) {
    ident *old_id = ref_ids[ref_ids_size - 1];
    if (old_id && (old_id->lineno == id_line) && (old_id->id == idn) &&
	old_id->decl && ast_could_be_ambiguous_ref(old_id->decl->TREE_CODE()) &&
	(old_id->cur_decl == (astDecl *) get_current_decl())) {
      old_id->decl = decl;
      return;
    }
  }

}

extern "C" void ast_nested_typedef_hook(astDecl *td, astTree *dl)
{
  if (!td || !dl) return;
  if (td->TREE_CODE() != TYPE_DECL) return;
  if (dl->TREE_CODE() == CALL_EXPR)
    dl = ((astExp1 *) dl)->TREE_OPERAND(0);
  if (dl && (dl->TREE_CODE() == INDIRECT_REF))
    ast_typedef_hook(td, td, dl);
}
extern "C" void ast_set_no_id_hook_for_get_id()
{
  if ((yylloc.first_line == yylloc.last_line) &&
      (yylloc.first_column == ';'))
    set_no_id_hook(1);
}

int ast_id_is_oper(astIdentifier * idn)
{
  if (!idn) return 0;
  char *id_name = idn->IDENTIFIER_POINTER();
  /* 4 == "__op" */
  int id_len = (id_name) ? strlen(id_name) :0;
  return (get_code_from_id(idn) || (id_len > 4 && id_name[0] == '_' && id_name[1] == '_' && 	
				    id_name[2] == 'o' && id_name[3] == 'p'));
}

int ast_decl_is_oper(astDecl *decl)
{
    return (!decl) ? 0: ast_id_is_oper(decl->DECL_NAME());
}

extern "C" void set_default_line_for_nw_expr(astExp2 *nexp)
  /* set lines info = 0 for conversion non trivial operator */
{
  if (!nexp || nexp->TREE_CODE() != NEW_EXPR) return;
  
  astExp1 * exp1 = (astExp1 *) nexp->TREE_OPERAND(0);
  if (!exp1 || exp1->TREE_CODE() != ADDR_EXPR) return;
  astDecl *fdecl = (astDecl *) exp1->TREE_OPERAND(0);
  if (!exp1 || fdecl->TREE_CODE() != FUNCTION_DECL) return;
  astIdentifier *idn = (astIdentifier *) fdecl->DECL_NAME();
  if (ast_id_is_oper(idn)) { /* set debug_info to 0 if it is wrong */
    int id_line = -nexp->TREE_DEBUG();
    for(int ii=0; ii<*yylen_ptr; ++ii) {
      if((is_id(yylsp[-ii].last_column) && (yylsp[-ii].last_line == id_line)) ||
	 (is_id(yylsp[-ii].first_column) && (yylsp[-ii].first_line == id_line))) {
	nexp->set_DEBUG(0);    
      }
    }
  }
  if (!ast_id_is_oper(idn) || get_code_from_id(idn))
    return;
  ast_set_default_lines_hook (nexp);
  nexp->set_DEBUG(0);
}

extern "C" void ast_typed_declspecs1_hook(astTree *fr, astType *type, char next_char)
  /* handle friend class a; */
{

  if (!fr || !last_type_decl || (last_type_decl->TREE_TYPE() != type)) return;
  if (next_char == ';' && (TREE_START_LINE(last_type_decl) == yylsp[0].first_line) &&
      (TREE_END_LINE(last_type_decl) == yylsp[0].last_line)) {
    ast_semicolon_hook((astDecl *) last_type_decl);
    if (fr->TREE_CODE() == TREE_LIST) {
      astIdentifier *idn = (astIdentifier *) ((astList *)fr)->TREE_VALUE();
      if (idn && idn->TREE_CODE() == IDENTIFIER_NODE) {
	if (idn->IDENTIFIER_POINTER() && (strcmp(idn->IDENTIFIER_POINTER(), "friend") == 0))
	  ast_declspecs_hook(0, (astDecl *) last_type_decl);
      }
    }
  }
}

int ast_hook_typedecl_is_def (astDecl *decl)
/*
  this is a dup of the function ast_typedecl_is_def in ast/src/ast_dd.c
 */
{
  astType * type;
  int is = 0;

  if (decl == 0) return 0;
  if (decl->TREE_CODE () != TYPE_DECL) return 0;

  type = decl->TREE_TYPE ();
  if (type == 0)    return 0;

  switch (type->TREE_CODE ())
    {
    case RECORD_TYPE:
    case UNION_TYPE:
    case ENUMERAL_TYPE:
      if (TYPE_LANG_SPECIFIC(type) &&
       (CLASSTYPE_IMPLICIT_INSTANTIATION(type) ||
	  CLASSTYPE_EXPLICIT_INSTANTIATION(type)))
	is = 0;
      else if (type->TYPE_NAME() &&
	  type->TYPE_NAME()->TREE_CODE()  == TYPE_DECL)
	{
	  if ((type->TYPE_NAME () == decl) && type->TYPE_SIZE ())
	    is = 1;
	}
      else
	if ((TYPE_STUB_DECL(type) == decl) && type->TYPE_SIZE ())
	  is = 1;
      break;
    }
  return is;
}


extern "C" void ast_copy_tree_info_hook(astTree * oldtree, astTree * newtree)
{
    int len = set_len(1);
    if(len==0 && (TREE_START_LINE(newtree) == 0)) {
      if ((oldtree->TREE_CODE() == TYPE_DECL) &&
	  ast_hook_typedecl_is_def((astDecl *)oldtree)) {
	TREE_START_LINE (newtree) = TREE_START_LINE (oldtree);
	TREE_END_LINE (newtree) = TREE_END_LINE (oldtree);
	TREE_FILENAME (newtree) = TREE_FILENAME (oldtree);
	newtree->set_DEBUG(oldtree->TREE_DEBUG());
	TREE_ASET_FLAG(newtree) =TREE_ASET_FLAG(oldtree);
      }
      int size = file_globals.size();
      for(int ii = size-1; ii>=0; --ii){
	astDecl * decl = (astDecl *) (*file_globals[ii]);
	if (decl == oldtree) {
	  file_globals.append(&newtree);
	  break;
	}
      }
    }
}

extern "C" void ast_copy_tree_info (astTree *newtree, astTree *oldtree)
{
  if (newtree && oldtree)
    {
      TREE_START_LINE (newtree) = TREE_START_LINE (oldtree);
      TREE_END_LINE (newtree) = TREE_END_LINE (oldtree);
      TREE_FILENAME (newtree) = TREE_FILENAME (oldtree);
      newtree->set_DEBUG( oldtree->TREE_DEBUG());
    }
}  

extern "C" void ast_copy_call_info (astTree *newtree, astTree *oldtree)
{
  /* this call is already mark as not interested */
  if (newtree && oldtree && (newtree->TREE_CODE() == CALL_EXPR) &&
      (TREE_START_LINE(newtree) == 0) && TREE_END_LINE(newtree))
    ;
  else ast_copy_tree_info(newtree, oldtree);
}  


extern "C" astTree *copy_node(astTree *);
extern "C" void ast_nested_struct_global_def(astTree *typ)
/*
  class A::B { ...
*/
{
  if (!typ || !(typ->TREE_CODE() == RECORD_TYPE ||typ->TREE_CODE() == UNION_TYPE )) return;
  astDecl * decl = (astDecl *) ((astType *)typ)->TYPE_NAME();
  if (decl->DECL_CONTEXT() && (decl->DECL_CONTEXT()->TREE_CODE() == RECORD_TYPE||decl->DECL_CONTEXT()->TREE_CODE() == UNION_TYPE)) {
    astDecl * copied = (astDecl *) copy_node(decl);
    copied->set_TREE_PREV((tree) -1);
    if (need_call_swap_decl(decl, copied))
    {
      ast_copy_tree_info_hook (decl, copied);
      ast_swap_decl_info (decl, copied);
    }
  }
}

extern "C" void ast_set_default_expr(astExp1 * expr)
{
  if (!expr || (expr->TREE_CODE() != CALL_EXPR)) return;
  /* do we really need to set start and end line to 0 ? */
  ast_set_default_lines_hook (expr);
  expr->set_DEBUG(0);
}
extern "C" int ast_is_default_call_expr(astTree *expr)
{
  if (!expr) return 1;
  return (expr->TREE_CODE() == CALL_EXPR && TREE_START_LINE(expr) == 0 &&
	  TREE_END_LINE(expr));
}

#endif

extern "C" void ast_extern_lang_hook(int val)
{
    ast_extern_lang = val;
}

extern "C" int ast_is_extern_lang() {
   return ast_extern_lang != 0;
}

static int ast_parenthesized_id;

extern "C" int ast_parenthesized_hook(int mode)
{
    if (mode == 1)
	ast_parenthesized_id = cur_id_line;

    return ast_parenthesized_id;
}

//============================================================================
// implementation of class ref_id_present    -- sturner, 980511
//
ref_id_present::ref_id_present()
  : data_size(0),
    used_size(0),
    data(0)
{}

ref_id_present::~ref_id_present()
{
  if (data != 0)
    free((char *)data);
}

// must be called before using add/lookup
void ref_id_present::reset()
{
  reallocate(63);
}

void ref_id_present::add(ident *nid)
{
  // Keep the table sparse, because a false hit is so costly.
  if (used_size >= data_size / 4)
    reallocate(data_size * 4 + 3);
  insert(nid);
  used_size += 1;
}

int ref_id_present::lookup(int lineno, char *filename, astIdentifier *id,
                                                       ident *&nid)
{
  int h = hash(lineno, filename, id);
  nid = data[h];
  return nid == 0 ? 0 : nid == &multi_nid ? 2 : 1;
}

// 0 <= return < data_size
int ref_id_present::hash(int lineno, char *filename, astIdentifier *id)
{
   typedef unsigned long ul;
   ul r = ((ul)filename ^ (ul)lineno ^ (ul)id >> 1) % (ul)data_size;
   return r;
}

// new_data_size must be positive and larger than ref_ids.size().
void ref_id_present::reallocate(int new_data_size)
{
  if (data != 0)
    free((char *)data);
  data_size = new_data_size;
  data = (ident **)malloc(data_size * sizeof(ident *));
  for (int k = 0; k < data_size; k += 1) data[k] = 0;
  used_size = ref_ids.size();
  for (int i = used_size-1; i >= 0; --i) {
    ident *nid = ref_ids[i];
    insert(nid);
  }
}

void ref_id_present::insert(ident *nid)
{
  int h = hash(nid->lineno, nid->filename, nid->id);
  if (data[h] == nid) {
  }
  else if (data[h] == 0) {
    data[h] = nid;
  }
  else {
    data[h] = &multi_nid;
  }
}

ident ref_id_present::multi_nid;

/*
   START-LOG-------------------------------------------

   $Log: ast_hook.C  $
   Revision 1.53 1998/05/14 08:57:49EDT Scott Turner (sturner) 
   tweak performance fix changes, to make it compile on sun4
Revision 1.2.1.125  1994/07/20  19:30:53  mg
pointer to function


*/

