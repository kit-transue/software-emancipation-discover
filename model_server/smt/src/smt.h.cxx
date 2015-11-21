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
// File smt.h.C - SMODE classes - methods and functions.
//
#include <cLibraryFunctions.h> 
#include <msg.h>
#include <smt.h>
#include <objOper.h>
#include <transaction.h>
#include <representation.h>
#include <ddict.h>
#include <RTL_externs.h>
#include <proj.h>
#include <autolog.h>
#include <messages.h>

#include <psetmem.h>

#include <reference.h>
#include <cmd.h>
#include <machdep.h>
#include <errorBrowser.h>
#include <MemSeg.h>
#include <setprocess.h>
#include <driver_mode.h>

extern objSet sh_set;
extern int smt_editing_level;
void smt_unset (smtTree*);

extern int smt_debug_flag;
ostream & smt_get_map_stream();
extern void ste_smart_view_update (smtHeaderPtr, int with_text = 1);
extern int ste_temp_view_suspend (smtHeaderPtr);
init_relation(smt_goto_of,MANY,relationMode::DMW,smt_label_of,1,relationMode::MW);
init_relation(smt_declaration_of,MANY,relationMode::MW,smt_definition_of,1,relationMode::MW);
init_relation(declaration_of_reference,1,relationMode::MW,reference_of_declaration,MANY,relationMode::MW);
init_relation(name_of_smt_tree,1,relationMode::M,smt_tree_of_name,1,relationMode::M);
init_relation(scope_of_smtHeader,1,0,smtHeader_of_scope,MANY,relationMode::D);
init_relation(kr_pdecl_of_ref, 1, relationMode::MW, kr_ref_of_pdecl, 1, relationMode::MW);
init_relation(smt_includes,MANY,0,smt_included,MANY,0);
init_relation(xref_of_smt,1,relationMode::W,smt_of_xref,MANY,0);

init_relation(segment_of_smt,MANY,0,smt_of_segment,MANY,0);

init_relational(smtHeader, app);
init_relational(smtTree, appTree);
 
ddElement* smt_get_dd(smtTree*);
void enable_reparse_buttons(smtHeaderPtr, boolean);
int smt_modify_ast(smtHeader *);

extern "C" char const *paraset_realpath(char const *, char const *);
 
smtHeader** smtHeader::htable;
int smtHeader::htablth;
int smtHeader::newtabndx;

class smt_internal_copier : public objCopier
  {
  public:
    Relational * apply(Relational *);
  };
 
Relational * smt_internal_copier::apply(Relational * t)
  {
   Initialize(smt_internal_copier::apply);

  return new smtTree(checked_cast(smtTree,t));
  }

int smtHeader::add_to_htable(smtHeader* smt_header)
//
// Add new header to htable.
//
{
    Initialize(smtHeader::add_to_htable);

    int index = -1;

    if (htablth)
	for(int ii = 0; ii < newtabndx; ++ii)
	    if (!htable[ii]) {
		index = ii;
		break;
	    }

    if (index == -1)
	index = newtabndx++;

    if(index >= htablth)
    {
	const int alloc_incr = 32; // Amount by which to to grow htable.

	if(htablth)
	    htable = (smtHeader**)psetrealloc(
		(char *)htable, (htablth + alloc_incr) * sizeof(smtHeader*));
	else
	    htable = (smtHeader**)psetmalloc(
		(htablth + alloc_incr) * sizeof(smtHeader*));
	OSapi_bzero(htable + htablth, alloc_incr * sizeof(smtHeader*));
	htablth += alloc_incr;
    }
    htable[index] = smt_header;

    return index;
}

smtHeader::smtHeader(char const *lname, smtLanguage l, smtHeader * h)
  :  app(lname, App_SMT, smtLanguage (h ? h->language : l)), 
     arn(1),
     sh_filename (0), just_restored (0)
{
    Initialize(smtHeader::smtHeader(char const *,smtLanguage,smtHeader*));

    srcbuf = 0;                    // we set srcbuf to zero, to inhibit it being counted as loadedFile
    actual_mapping_priv = 1;
    set_actual_mapping(0);         // note, this setting is always a *change* because of previous line
    
    htindex = add_to_htable(this);
    smtTree * r;
    language = smtLanguage(h ? h->language : l);
    untok_flag = 0;
    // Clear tokens table
    ttable = 0;
    ttablth = 0;
    
    if (h)
    {
	smt_internal_copier cp;
	RelationalPtr copy = 
	    r = checked_cast(smtTree,(RelationalPtr)
			     (obj_copy_deep(* ((Obj *)h->get_root()), &cp)));
	src_size = src_asize = h->src_size;
	srcbuf = (char *)psetmalloc(src_size);
	OS_dependent::bcopy(h->srcbuf, srcbuf, src_size);
	parsed = 1;
    }
    else
    {
	r = new smtTree(SMT_file);          // Create root node
	srcbuf = NULL;
	parsed = src_size = src_asize = 0;
    }
    
    
    if (lname && *lname)
    {
	sh_filename = new char[strlen(lname) + 1];
	strcpy(sh_filename, lname);
	
	// make realpath out of logical name
	char const *physical_name = get_phys_name();
	char resolved_path[MAXPATHLEN];
	if (!paraset_realpath(physical_name, resolved_path))
	    strcpy (resolved_path, physical_name);
	
	// store real path in shared memory
	foreign = projHeader::find_working_module (lname) == 0;
    }
    else
    {
	foreign = 0;
    }
    
    put_root(r);
    set_ast_exist(0);
    set_arn(0);
    vrn = sam = scope = tok = 0;
    drn = 1;
    globals = 0;
    
    sh_set.insert(this);
    return;
}

void remove_from_tokenized (Obj*);
void remove_from_changed (Obj*);
 
// SMT header destructor
smtHeader::~smtHeader()
{
  Initialize(smtHeader::~smtHeader());

  int had_srcbuf = 0;
  if(srcbuf) {
    had_srcbuf = 1;
    psetfree(srcbuf);
  }
  srcbuf = 0;
  loadedFiles::proj_remove_from_loaded(this);
  
  if(ttable)
    psetfree(ttable);
   
  if(sh_filename)
    delete sh_filename;
 
  // Remove header from table
  htable[htindex] = 0;
  if(globals)
    delete globals;
  remove_from_tokenized (this);
  if (!foreign && had_srcbuf) {
      cut_ast ();
  }
}
 

smtHeader::smtHeader(char const *ln, char const *phy_name, smtLanguage l, projNode**proj_ptr)
  :  app(ln ? ln : phy_name, App_SMT, l), 
     sh_filename (0), just_restored (0), arn(0)
{
    Initialize (smtHeader::smtHeader(char const *,char const *,smtLanguage));

    srcbuf = 0;                    // we set srcbuf to zero, to inhibit it being counted as loadedFile
    actual_mapping_priv = 1;
    set_actual_mapping(0);         // note, this setting is always a *change* because of previous line

    projNode* pr = proj_ptr ? *proj_ptr : projHeader::fn_to_project(phy_name, 1);
    htindex = add_to_htable(this);
 
    char const *lname = ln ? ln : phy_name;
    set_phys_name (phy_name, pr);
    language = l;
    untok_flag = 0;
    ttable = 0;
    ttablth = 0;
 
    smtTree* r = new smtTree(SMT_file);
    srcbuf = NULL;
    parsed = src_size = src_asize = 0;


    sh_filename = new char[strlen(lname) + 1];
    strcpy(sh_filename, lname);
    char resolved_path[MAXPATHLEN];
    if (!paraset_realpath(phy_name, resolved_path))
	strcpy (resolved_path, phy_name);
    projNode* home = projNode::get_home_proj ();
    int is_external = 1;
    if (pr && home->is_younger(pr) && pr->get_xref() && pr->get_xref()->is_writable())
        is_external = 0;

//    int is_external = (pr && pr->get_xref()) ? !(pr->get_xref()->is_writable()) : 1;
    if (ln) 
	foreign = is_external;
    else
	foreign = 1;
    put_root(r);
    set_ast_exist(0);
    set_arn(1);
    vrn = sam = scope = tok = 0;
    drn = 1;
    globals = 0;

    sh_set.insert(this);
//    loadedFiles::proj_add_to_loaded(this);  // add this file to the loadedFiles list     bugbug
}
 
// SMT tree constructor
smtTree::smtTree(int tp) : is_macro(0), old_string (0), temp_vis(0), was_type (tp)
{
  type = extype = tp;
  tbeg = -1;
  tlth = tnfirst = tnmax = 0;
  arn = vrn = 0;                // Reset AST and View regenerate flags
  ndm = 0;
  ref_ast = 0;
  src_line_num = 0;
  newlines = spaces = 0;

  oldStart = oldLength = -1;
}
 
// The special internal constructor for copying a module
smtTree::smtTree(smtTree * t) : is_macro(0), old_string (0), temp_vis(0), was_type (t->was_type)
{
  type = t->type;
  extype = t->extype;
  tbeg = t->tbeg;
  tlth = t->tlth;
  tnfirst = t->tnfirst;
  tnmax = t->tnmax;
  arn = vrn = 0;
  ndm = 0;
  ref_ast = 0;
  src_line_num = 0;
  oldStart = oldLength = -1;
}
 
// SMT tree destructor
smtTree::~smtTree()
{
  if (old_string)
      delete old_string;
  smt_unset(this);
}
 
app * smtTree::get_main_header()
{
  return get_header();
}
 
int smtTree::get_extype() const
{
  return extype;
}
 
int smtTree::get_node_type()
{
  return type;
}
 
char const *smtTree::get_name() const
{
   Initialize(smtTree::get_name);

  smtHeader* sh = checked_cast(smtHeader, get_header());
  if (!sh)
    return NULL;
  char *ns = new char[tlth+2];
  strncpy(ns, sh->srcbuf+tbeg, tlth);
  ns[tlth] = '\0';
  return ns;
}
 
 
 
// Expand SRC buffer
void smtHeader::src_exp_buffer(int n)
{
  srcbuf = srcbuf == 0 ? (char *)psetmalloc(src_asize + n) :
  (char *)psetrealloc(srcbuf, src_asize + n);
  src_asize += n;
}
 
// Load SMT (Load file into memory).
int smtHeader::load()
{
   Initialize(smtHeader::load);

  struct OStype_stat statbuf;
  int file, flth;
  if(parsed || srcbuf)                    // Already loaded ?
    return 0;                   // yes - return
  smtTree * t = checked_cast(smtTree,get_root()->get_first());
  if(t)
  {
    t->remove_from_tree();
    obj_delete(t);
  }
  parsed = 0;
  smtTree * tn = db_new(smtTree, (SMT_token));
  get_root()->put_first(tn);
  char const *get_file = get_filename();
  if (OSapi_access(get_phys_name(), R_OK)) {
    msg("ERROR: Cannot open \"$1\":$2") << get_phys_name() << eoarg << OSapi_strerror(errno) << eom; 
    return -1; // return if we do not have read permission
  }
  if(OSapi_stat(get_phys_name(), &statbuf) != 0) // Read file parameters
    return -1;
  flth = (int)statbuf.st_size;
  srcbuf = (char *)psetmalloc(flth + 1); //Allocate buffer
  if(srcbuf == 0)
    return -1;
  src_size = src_asize = flth;
  srcbuf[flth] = '\0';
  file = OSapi_open(get_phys_name(), O_RDONLY);
  if(file < 0 || OSapi_read(file, srcbuf, flth) != flth)
  {
    if(file >= 0)OSapi_close(file);
    psetfree(srcbuf);
    srcbuf = 0;
    src_size = src_asize = 0;
    return -1;
  }
   OSapi_close(file);
  tn->tbeg = 0;
  tn->tlth = src_size;
  tn->tnfirst = 1;
  tn->tnmax = 0;
  drn = 1;
  return 0;
}
 
// Open SMT from string
int smtHeader::load(char const *text)
{
  if(srcbuf)
    return 0;
  smtTree * tn = db_new(smtTree, (SMT_token));
  get_root()->put_first(tn);
  // allocate buffer and copy text
  srcbuf = (char *)psetmalloc(strlen(text)+1);
  if(srcbuf == 0)
    return -1;
  src_size = src_asize = strlen(text);
  strcpy(srcbuf, text);
  tn->tbeg = 0;
  tn->tlth = src_size;
  tn->tnfirst = 1;
  tn->tnmax = 0;
  char const *get_file = get_filename();
  msg("STATUS: SMT: GENERATED: $1") << (get_file ? get_file : "<Temporary>") << eom;
  drn = 1;
  return 0;
}
extern void set_batch_cpp_errors(int);
int smtHeader::parse()
{
  if (!srcbuf)
    return -1;
  char const *get_file = get_filename();
genError*error;
int terr = 0;
genTry {
  if(parsed)
  {
    smt_shallow_parser(this);
    msg("STATUS: SMT: REPARSED: $1") << (get_file ? get_file : "<Temporary>") << eom;
    vrn = 1;
    modify_view();
    return 0;
  }
   
  foreign = 0;
  smt_shallow_parser(this);
  msg("STATUS: SMT: Tokenized: $1") << (get_file ? get_file : "<Temporary>") << eom;

  if (smt_debug_flag) {
      ostream & os = smt_get_map_stream();
      os << "SMT: Tokenized: " << (char *)(get_file ? get_file : "<Temporary>") << endl;
  }
} genCatch(error){
  terr = 3;
  set_batch_cpp_errors(terr);
} genEndtry;

  drn = 1;
   
  if(get_file)
    set_modified();
   
  return terr;
}
 
static void smt_cut_ast(smtTree *t)
{
    smt_unset (t);
    for(smtTree * w = t->get_first (); w; w = w->get_next ())
	smt_cut_ast(w);

}

void smtTree::disconnect_from_ast ()
{
    if (this)
	smt_cut_ast (this);
}

static void smt_cut_ast_one_direction(smtTree *t)
{
#ifndef _WIN32
  if (!is_gui()) return;
#endif
    if (t) {
	t->ref_ast = 0;
	for(smtTree * w = t->get_first (); w; w = w->get_next ())
	    smt_cut_ast_one_direction(w);
    }
}
 
void smtHeader::cut_ast()
{
   Initialize(smtHeader::cut_ast);

   cut_ast_internal ();
}

void smtHeader::cut_ast_internal()
{
   Initialize(smtHeader::cut_ast_intermal);

   Obj* rs = get_relation(segment_of_smt,this);
   if (rs) {
       Obj* r1;
       RelSegment* r=NULL;
       {ForEach(r1,*rs){
	   r=checked_cast(RelSegment,r1);
	   break;
       }}
       if (r) {
	   Obj* hs = get_relation(smt_of_segment,r);
	   if (hs) {
	       smtHeader* h;
	       Obj* el;
	       {ForEach(el,*hs){
		   h = checked_cast(smtHeader,el);
		   if (try_relation (tree_of_header, h) == DB_NULL)
		       continue;
		   h->disconnect_from_ast();
		   h->set_ast_exist(0);
		   int save_act_mp = h->get_actual_mapping();
		   h->set_arn (1);
		   h->set_actual_mapping(save_act_mp);
		   if(h->globals)
		       delete h->globals;
		   h->globals = 0;
	       }}
	   }
	   r->rem_rel_all();
	   mem_segments* m = r->memseg;
	   if (m->name) {
#ifndef _WIN32	       
               if (is_model_build())
		   OSapi_unlink(m->name);
	       else
		   OSapi_truncate(m->name,0);
#else
	       delete m->mseg;
               OSapi_unlink(m->name);
#endif /*_WIN32*/
	   }
	   m->mseg = NULL;
	   mem_segments::current = NULL;
       }
   }
}

extern void ste_set_freeze_status (int);

static objArr modified_fn;
void modified_functions_clear ()
{
    modified_fn.removeAll ();
}
 
#ifdef XXX_smtHeader__make_ast
void smtHeader::make_ast()
{
   Initialize(smtHeader::make_ast);
   VALIDATE_id_smtHeader(this);
   modified_functions_clear();

  sam = 0;
  
  if (language == FILE_LANGUAGE_ELS)
    {
      if (!get_arn() && !src_modified)
	{
	  enable_reparse_buttons(this, 0);
	  ste_set_freeze_status (1);
	  return;
	}
      else
	{
	  symbolPtr fsym = this;
	  int els_parse_file(const symbolPtr& mod);
	  els_parse_file(fsym);
	  return;
	}
    }
  else if(ast_exist && !get_arn())
  {      
      enable_reparse_buttons(this, 0);
      ste_set_freeze_status (1);
      return;
  }

  projNode* old_current = make_current();

   errorBrowserClear();
   start_transaction(){
       ste_temp_view_suspend (this);
       restore_tokenize (this);
       smt_modify_ast(this, 1);
       ste_smart_view_update( this, 0 );
   }end_transaction();
   errorBrowser_show_errors();  // It will automatically pop up if there is an errors.
 
  projNode::set_current_proj (old_current);
}
#endif 
// Find SMT node address for given token number
smtTree * smtHeader::tn_na(int tn)
{
  if(tn <= 0 || tn > ttablth)
    return 0;
  return ttable[tn - 1];
}
 
// find SMT node address for given character number in tree
smtTree * smtHeader::cn_na(int cn)
{
   Initialize(smtHeader::cn_na);

   smtTree *w;
   
   if(last_token &&
      cn >= last_token->tbeg &&
      cn - last_token->tbeg < last_token->tlth)
       return last_token;
   
   for (smtTree *t = checked_cast(smtTree,get_root()); t; t = w) {
       for (w = t->get_first(); w; w = w->get_next())
	   if (cn >= w->tbeg && cn - w->tbeg < w->tlth) 
	       break;
       if (w && w->type == SMT_token) {
	   last_token = w->get_next_leaf();
	   return w;
       }
   }
   return 0;
}
 
// find SMT node address for given character number (point) in ttable
smtTree * smtHeader::cp_na(int pnt)
{
   Initialize(smtHeader::cp_na);
   if (ttablth == 0 )
       return NULL;

   int st = 0, en = -1; 
   // count the last token index in ttable
   smtTreePtr rt = checked_cast(smtTree,get_root());
   if (rt)
       en = rt->tnmax - 1;

   if (en == -1)
       return NULL;

   smtTreePtr a_tok = NULL;
   smtTreePtr t1 = ttable[st];
   smtTreePtr t2 = ttable[en];

   if (!t1 || !t2 || pnt < t1->start() || pnt > (t2->start() + t2->length()))
       return NULL;
   else if (pnt >= t1->start() && pnt - t1->start() < t1->length())
       a_tok = t1;
   else if (pnt >= t2->start() && pnt - t2->start() <= t2->length())
       a_tok = t2;
   else if ((en - st) <= 1)
       return NULL;

   int ind = st;
   for ( ; a_tok == NULL && en > st ; ) {
       if ((en - st) == 1)
	   ind = (ind == st) ? en : st;
       else
	   ind = st + (en - st)/2;

       if ((t1 = ttable[ind])) {
	   if ((pnt >= t1->start() && pnt - t1->start() < t1->length())) 
	       a_tok = t1;
	   else {
	       if (ttable[ind] && pnt > ttable[ind]->start())
		   st = ind;
	       else
		   en = ind;
	   }
       } else
	   break;
   }
   return a_tok;
}

void smtHeader::check_margin (smtTree *& a_tok, int& off, int end_flag) 
{
    Initialize(smtHeader::check_margin);
    if (!a_tok)
	return;

    smtTreePtr wrk;
    if (end_flag && off == 0) {
	if ((wrk = a_tok->get_prev_leaf())) {
	    a_tok = wrk;
	    off = -1;
	}
    } else if (!end_flag && (off == -1 || off == a_tok->length())) {
	if ((wrk = a_tok->get_next_leaf())) {
	    a_tok = wrk;
	    off = 0;
	}
    }

    if (a_tok && a_tok->length() == 0) {
	if (end_flag)
	    for ( ; a_tok && a_tok->length() == 0; a_tok = a_tok->get_prev_leaf() ) 
		;
	else
	    for ( ; a_tok && a_tok->length() == 0; a_tok = a_tok->get_next_leaf() ) 
		;
    }
}

smtHeader * smtHeader::find(char const *ln)
{
//  Initialize (smtHeader::find);

    if(ln == 0 || *ln == 0)
        return( 0);
   
    smtHeader *h;
    char const *lname;
    int i;
   
    if (*ln == '-' || *ln == '+') ++ln;
    if (*ln == '-' || *ln == '+') ++ln;
   
    if(isdigit(*ln)) {
	i = OSapi_atoi(ln);
	h = htable[i];
	return( h);
    }
   
    for(i = 0; i < newtabndx; i++) {
	h = htable[i];
	if(h == 0)
	    continue;
	lname = h->get_filename();
	if ((lname && strcmp(lname, ln) == 0)) {
	    return(h);
	}
    }

    // safety net
    h = smtHeader::find_from_realpath (ln);
    return 0;
}
 
// finds smtHeader that matches a realpath
smtHeader * smtHeader::find_from_realpath(char const *rp)
{
    Initialize (smtHeader::find_from_realpath);
    smtHeader *h;
    char const *lname;
    int i;
   
    if(rp == 0 || *rp == 0) return 0;
   
    if (*rp == '-' || *rp == '+') ++rp;
    if (*rp == '-' || *rp == '+') ++rp;
   
    if(isdigit(*rp))
    {
	i = OSapi_atoi(rp);
	if(i >= newtabndx)
	    return 0;
	h = htable[i];
	return( h);
    }

    char const *physical_name = NULL;
    char resolved_path[MAXPATHLEN];
    char callers_realpath[MAXPATHLEN];

    if (!paraset_realpath( rp, callers_realpath))
	strcpy (callers_realpath, rp);

    for(i = 0; i < newtabndx; i++)
    {
	h = htable[i];
	if(h == 0)
	    continue;
	lname = h->get_filename();

	// make realpath out of logical name
	physical_name = h->get_phys_name();

	if (!physical_name) continue;

	// match with the physical path itself
	if(strcmp(physical_name, rp) == 0)
	    return(h);

	if (!paraset_realpath(physical_name, resolved_path))
	    strcpy (resolved_path, physical_name);

	if(strcmp(resolved_path, callers_realpath) == 0)
	    return(h);

    }
    return(0);
}

 
char const *smtHeader::get_name() const
{
  Initialize(smtHeader::get_name);

  smtHeader* This = checked_cast(smtHeader, this);
  return(This->get_filename()); // get_filename is non-const, sigh...
}
 
void smtHeader::set_ast_exist(int f)
{
  ast_exist = f;
}
 
void smtHeader::set_arn(int f)
{
  void add_to_pending_lists(smtHeader *);
  if(f && get_actual_mapping()) add_to_pending_lists(this);
  if (f != arn && is_gui())
    enable_reparse_buttons(this, f);
  arn = f;
  //  Dave Note: the following line does not quite make sense to me;
  //    It feels like it should only be called when f is true.
  //    But the code apparently worked, so I am not changing it.
  //    A better way is probably for the parent to call set_actual_mapping()
  //    explicitly if it needs to, and we would only call it when f is true.
  set_actual_mapping(!f);
}

static smtHeader* last_map = 0;
static caddr_t last_srcbuf = 0;
void smtHeader::set_actual_mapping(uint f)
//  Change the "actual_mapping" bit of smtHeader
//  This bit represents the status of whether the smtHeader is currently
//    correctly reparsed.  It is turned on during a successful restore,
//    and turned off during either a restore of a pset with errors, or
//    when some changes are made in smode to a loaded file.
// Note also that srcbuf is used is a flag to indicate that the file is
//    really loaded.  Dummy smtHeader are created for some header files
//    for internal purposes.  But if srcbuf exists, so does the full smt.
{
    Initialize(smtHeader::set_actual_mapping);
//    VALIDATE_id_smtHeader(this);

    IF(f>1)
	;
    if (ph_name.str() == 0) return;
    bool old_map = actual_mapping_priv;
    actual_mapping_priv = f;
    if (!is_model_build())
    if (this != last_map || srcbuf != last_srcbuf || f != old_map) {
	loadedFiles::proj_add_to_loaded(this);   // add it to loaded list, if it is not already there
	//  if flag is true, remove this file from the unparsedFiles rtl
	//  if flag is false, and srcbuf nonzero, add this file to that rtl
	if (srcbuf || f)
	    loadedFiles::change_reparse_flag(this, !f);
    }
    last_map = this;
    last_srcbuf = srcbuf;
}


char const *smtHeader::get_name(smtTree * t) const
{
  Initialize(smtHeader::get_name);

  smtTree * x;
  ddElement * dd;
  // Find subtree which contines given node and has relation to DD
  for(x=t;
      x && (dd = smt_get_dd( t)) == 0;
      x = checked_cast(smtTree,x->get_parent()));
  return(x ? dd->get_name() : this->get_name());
}
 
void smtHeader::suspend_parsing()
{
  sam = 1;
}
 
 
int smtHeader::test_status()
{
  return (sam || smt_editing_level > 0) + 2 * (ast_exist && get_arn() == 0);
}
 
void smtTree::clear_arn()
{
  if (is_model_build()) return;
    arn = 0;
    for (smtTree* x = get_first(); x; x = x->get_next())
	x->clear_arn();
}
 
void smtTree::clear_vrn()
{
  if (is_model_build()) return;
    vrn = ndm = 0;
    for (smtTree* x = get_first(); x; x = x->get_next())
	x->clear_vrn();
}
 
 
// The copy constructor for SMT subtree
// It makes a copy of subtree with a aditional dummy header
static void smt_copy(smtTree *, smtTree const *, int);
smtTree::smtTree(const smtTree& rf) : is_macro(0), old_string (0)
{
  Initialize(smtTree::smtTree);
  
  smtTree& r = (smtTree&)rf;
  smtHeader * h = checked_cast(smtHeader,r.get_header());
  h->tidy_tree ();
  smtHeader * nh = new smtHeader("*clipboard*", (smtLanguage)h->language);
  nh->put_root(this);
  nh->src_asize = nh->src_size = r.length ();
  nh->srcbuf = (char *)psetmalloc(nh->src_size);
  strncpy(nh->srcbuf, h->srcbuf + r.start (), r.length ());
  smt_copy(this, &r, r.start ());
  oldStart  = rf.oldStart;
  oldLength = rf.oldLength;
}
 
static void smt_copy(smtTree *to, smtTree const *from, int beg)
{
  Initialize(smt_copy);

  to->tbeg = from->tbeg - beg;
  to->tlth = from->tlth;
  to->tnfirst = to->tnmax = 0;
  to->type = from->type;
  to->was_type = from->was_type;
  to->extype = from->extype;
  to->spaces = from->spaces;
  to->newlines = from->newlines;
  smtTree * p = 0;
  for(smtTree * t = ((smtTree*)from)->get_first();
      t;
      t = t->get_next())
  {
    smtTree * n = new smtTree;
    if(p)
      p->put_after(n);
    else
      to->put_first(n);
    smt_copy(n, t, beg);
    p = n;
  }
}
 
// copies smt tree for specified header
appTreePtr smtTree::rel_copy_for_header (appPtr, objCopier *cp)
{
  Initialize (smtTree::rel_copy_for_header);
   
  smtTreePtr r_val = NULL;
   
  smtTreePtr starter_obj = checked_cast(smtTree, cp->get_starter_obj());
   
  if (!is_smtTree (starter_obj)) return (NULL);
   
  smtHeader * h = checked_cast(smtHeader,this->get_header());
  if (starter_obj == this) {
    // create smt header
    h->tidy_tree ();
    smtHeader *nh = db_new (smtHeader, ("", (smtLanguage)h->language));

    nh->src_asize = nh->src_size = h->src_size - start ();
    nh->srcbuf = (char *)psetmalloc (nh->src_size);
    OS_dependent::bcopy (h->srcbuf + start (), nh->srcbuf, nh->src_size);

    r_val = new smtTree();
    nh->get_root()->put_first(r_val);
     
  } else
    r_val = db_new (smtTree,());
   
  r_val->tbeg = tbeg - starter_obj->start ();
  r_val->tlth = tlth;
  r_val->tnfirst = r_val->tnmax = 0;
  r_val->type = type;
  r_val->was_type = was_type;
  r_val->extype = extype;
  r_val->newlines = newlines;
  r_val->spaces = spaces;
   
  return (r_val);
}
 

int smtTree::untok_length_in_lines ()
{
    Initialize(smtTree::untok_length_in_lines);
    smtHeader *h = checked_cast(smtHeader,get_header());
    char const *buf = h->srcbuf;
    buf = buf + start();
    int newln_count = 0;
    int stuff_following_newln = 0;
    int len = length();
    for (int ii = 0 ; ii < len ; ii++) {
    	char ch = buf[ii];
	if (ch == '\n') {
	    newln_count++ ;
    	    stuff_following_newln = 0;
	}
    	else if (!isspace(ch)) {
    	    stuff_following_newln = 1;
	}
    }

    return newln_count + stuff_following_newln;
}

//
// actually, returns the right next line after the last_leaf
//
int smtTree::get_last_line (smtTree *& last_leaf, int &last_len)
{
    Initialize(smtTree::get_last_line);
    if (get_first())
	last_leaf = get_last_leaf();
    else
	last_leaf = this;

    int len = last_leaf->src_line_num;
    if (last_leaf->extype == SMTT_untok)
	last_len = last_leaf->untok_length_in_lines ();
    else
	last_len = 1; //sudha:09/23/97 fix bug where number of lines is wrong if 
                      //last token on a line is not immediately followes by \n

    return len+last_len;
}

int smtTree::contains_line (int line_num, smtTree *& the_tok)
{
    Initialize(smtTree::contains_line);

    the_tok = NULL;
    if (src_line_num == line_num) {
	the_tok = this;
	return 1;
    }

    smtTree *nxt = get_next();
    if (nxt) {
	if (line_num > src_line_num && line_num < nxt->src_line_num)
	    return 1;
	else
	    return 0;
    }

    smtTree *last_lf = NULL;
    int last_len = 0;;

    int end_line = get_last_line (last_lf, last_len);
    if (line_num < end_line) {
	if (line_num < last_lf->src_line_num)
	    return 1;

	if (line_num < last_lf->src_line_num + last_len) {
	    the_tok = last_lf;
	    return 1;
	}
    }
    return 0;
}

smtTree *smtTree::subtree_at_line (int line_num)
{
    Initialize (smtTree::subtree_at_line);
   
    if (src_line_num > line_num)
	return(NULL);         // some thing wrong with linenum

    smtTree *st = get_first();
    smtTreePtr found = NULL;

    if (st == NULL) {
	if (contains_line(line_num, found))
	    found = this;

	return found;
    }

    for ( ; st ; st = st->get_next()) {
	if (st->contains_line(line_num, found)) {
	    if (!found)
		found = st->subtree_at_line(line_num);

	    break;
	}
    }

    return found;
}
 
 
 
smtTree *smtHeader::tree_at_line (int line_num)
{
  Initialize (smtHeader::tree_at_line);
   
  smtTreePtr root = checked_cast(smtTree, get_root());
   
  return (root ? root->subtree_at_line(line_num) : NULL);
}
 
static void tokenize_tree(smtTree *);

void smtHeader::tok_mode(int m)
{
  Initialize(smtHeader::tok_mode);

  if(m)
  {
    tok = 1;
    return;
  }
  if(tok == 0)
    return;
  tokenize_tree(checked_cast(smtTree,get_root()));
  tok = 0;
}
 
void tokenize_tree(smtTree *r)
{
  Initialize(tokenize_tree);

  smtTree * t;
  do
  {
    for(t = checked_cast(smtTree,r->get_first_leaf());
        t && t->extype != SMTT_untok;
        t = checked_cast(smtTree,t->get_next_leaf()));
    if(t)
    {
      smt_tokenizer(checked_cast(smtHeader,t->get_header()), t);
    }
  }
  while(t);
}
 
static RelType ** rtab[] =
{&tree_of_header, &ddRoot_of_smtHeader, 0};
 
int smtHeader::make_tree()
{
    Initialize (smtHeader::make_tree);

    if (parsed)
	return 0;
    
    smtTree* r = checked_cast (smtTree, get_root());
    rem_relation(tree_of_header, this, 0);
    r->rem_rel_all();
    put_relation(tree_of_header, this, r);

    load ();
    parse ();
    int i = 0;
    r = checked_cast(smtTree, get_root());
    r->clear_arn();
    r->clear_vrn();
    if (i == 0) 
	set_imported ();
    set_ast_exist (i == 0);
    set_arn ( i != 0);
    
    return i;
}

 
#define TTSIZE 512
void smtHeader::expand_ttable()
{
  int size = ttablth ? ttablth : TTSIZE;
  int l = (ttablth + size) * sizeof(smtTree*);
  if(ttablth)
    ttable = (smtTree **)psetrealloc((char *)ttable, l);
  else
    ttable = (smtTree **)psetmalloc(l);
  OSapi_bzero(ttable + ttablth, size * sizeof(smtTree*));
  ttablth += size;
}

static int curr_lineno;

static void enum_subtree (smtTree* node, char const *srcbuf)
{
    smtTree* t = node->get_first ();
    if (t) {
	node->src_line_num = 0;
	for (; t; t = t->get_next ())
	     enum_subtree (t, srcbuf);
    } else {
	node->src_line_num = curr_lineno;
	for (smtTree* p = node->get_parent ();
	     p && ! p->src_line_num;
	     p = p->get_parent ())
	    p->src_line_num = curr_lineno;
	for (int nn = 0; nn < node->tlth; ++nn)
	    if (srcbuf[node->tbeg + nn] == '\n') {
		curr_lineno++;
	    }
	if (node->newlines) {
	    curr_lineno+=node->newlines;
	}
    }
}

void smtHeader::enum_src ()
{
    Initialize (smtHeader::enum_src);

    smtTree* root = checked_cast (smtTree, get_root ());
    curr_lineno = 1;

    enum_subtree (root, srcbuf);
}

static struct smt_asynch_data pset_async;


extern "C" void set_smt_asynch_data (void* h, void* h1, smt_asynch_type op, int dt)
{
    if (cmd_current_journal_name || cmd_execute_journal_name)
	return;
    pset_async.make_ast_header = h;
    pset_async.parse_header = h1;
    pset_async.oper = op;
    pset_async.data = dt;
}

extern "C" void clear_smt_asynch_data ()
{
    pset_async.make_ast_header = 0;
    pset_async.parse_header = 0;
    pset_async.oper = NOT_FILLED;
    pset_async.data = 0;
}

extern "C" smt_asynch_data* get_smt_asynch_data ()
{
    return &pset_async;
}


extern "C" int check_smt_asynch_data ()
{
    if (cmd_current_journal_name || cmd_execute_journal_name)
	return 0;
    smt_asynch_data* ad = get_smt_asynch_data ();
    if (!ad) return 0;
    return (ad->oper != NOT_FILLED);
}

int smt_debugable_node_type(int smt_type)
{
  return (smt_type == SMT_stmt
	  || smt_type == SMT_decl
	  || smt_type == SMT_if
	  || smt_type ==  SMT_ifelse
	  || smt_type ==  SMT_else
	  || smt_type ==  SMT_for
	  || smt_type ==  SMT_while
	  || smt_type ==  SMT_do
	  || smt_type ==  SMT_switch  	  
	  || smt_type ==  SMT_case
	  || smt_type ==  SMT_default
	  || smt_type ==  SMT_break
	  || smt_type ==  SMT_continue
	  || smt_type ==  SMT_return
	  || smt_type ==  SMT_goto
	  || smt_type ==  SMT_dowhile	  
	  || smt_type ==  SMT_stmt
	  || smt_type ==  SMT_clause
	  || smt_type ==  SMT_else_clause  	  
	  || smt_type ==  SMT_else_if_clause 	  
	  || smt_type ==  SMT_then_clause  	  
	  || smt_type ==  SMT_case_clause
	  || smt_type ==  SMT_macrocall
	  || smt_type ==  SMT_try_catch  	  	  
	  || smt_type ==  SMT_try_clause  
	  || smt_type ==  SMT_catch_clause);
}

void smtHeader::disconnect_from_ast ()
{
    if (parsed) {
	smtTree* r = (smtTree*)get_root (); 
	smt_cut_ast_one_direction (r);
    }
}

int smtTree::construct_symbolPtr(symbolPtr& sym) const
{
    int retval = 0;

    ddElement * dd = smt_get_dd ((smtTree*)this);

    if (dd) {
	sym = (Obj*)dd;
	retval = 1;
    }

    return retval;
}

void app_set_just_restored(Hierarchical * ah, int val)
{
  if(ah)
    if(is_smtHeader (ah)) {
      smtHeader* smt = (smtHeader*)ah;
      smt->just_restored = val;
    }
}  
       
void enable_reparse_buttons(smtHeaderPtr smt_header, boolean enabled);

void equate_set_modified ( appTreePtr el ){
 Initialize(equate_set_modified);
    if ( el == NULL )
       return;

    appPtr ahd = el->get_header();
    ahd->set_modified();
    if ( is_smtHeader(ahd) ){
       checked_cast(smtHeader,ahd)->suspend_parsing();
       enable_reparse_buttons( checked_cast(smtHeader,ahd), 1);
    }
}

int smtTree::get_linenum () const
{
    return src_line_num;
}
