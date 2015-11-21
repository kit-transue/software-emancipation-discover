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
#include <metric.h>
#include <customize.h>
#include <xrefSymbol.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <ddict.h>
#include <xref.h>
#include <genError.h>
#include <systemMessages.h>
#include <charstream.h>
#include <dfa_db.h>
#include <RTL_externs.h>
#include <driver_mode.h>

void smt_fill_src_line_num (smtTree* root);
// MB: void debug_get_print_flags ();
extern int local_variables_done;

class MetricSorter {
  public:
  objArr sorted;
  MetricSorter(Obj*);
  ~MetricSorter();
};

MetricSorter::~MetricSorter()
{}

static int compare_offsets(const void* p1, const void* p2)
{
  smtTree* smt1 = p1 ? *(smtTree**)p1 : NULL;
  smtTree* smt2 = p2 ? *(smtTree**)p2 : NULL;
  int off1 = smt1 ? smt1->tbeg : -1;
  int off2 = smt2 ? smt2->tbeg : -1;
  int retval = (off1 == off2) ? 0 : (off1 > off2 ? 1 : -1);
  return retval;
}

MetricSorter::MetricSorter(Obj* source)
{
  Initialize(MetricSorter::MetricSorter(Obj*));

  Obj* el;
  ForEach(el,*source) {
    sorted.insert_last(el);
  }

  sorted.sort(compare_offsets);
}

bool MetricSet::save_table() {
    Initialize(MetricSet::save_table);
    
    set.table.compress();
    
    int size = set.table.size();
    FILE * file = OSapi_fopen(file_name, "wb");
    
    if (!file) return false;
    
    // what happens if new file is smaller than old !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    if (!OSapi_fwrite(set.table[0], sizeof(MLink), size, file)) {  
	OSapi_fclose(file);
	return false;
    }
    return true;
}



static int get_line_of_sym(symbolPtr& sym) 
{
  Initialize(get_line_of_sym);
  
  int retval = -1;

  if (!sym.isnull()) {
    ddElement* dd = NULL;
    if (sym.relationalp()) {
      Relational *rel = (Relational *)sym;
      if (rel && is_ddElement(rel)) {
	dd = (ddElement*)rel;
	if (!dd->get_is_def())
	  dd = NULL;
      }
    }
    if (dd || sym.is_loaded() || needLoad::ask()) {
      if (sym.get_kind() != DD_MODULE) {
	if (!dd)
	  dd = sym.get_def_dd();
	if (dd) {
	  retval = dd->xref_index;
	}
      }
    }
  }

  return retval;
}

void get_smt_of_sym(symbolPtr& sym, smtTree *&node) 
{
  Initialize(get_smt_of_sym);
  
  if (sym.isnull()) {
    node = NULL;
    return;
  }
  if (!node) {
    ddElement* dd = NULL;
    if (sym.relationalp()) {
      Relational *rel = (Relational *)sym;
      if (rel && is_ddElement(rel)) {
	dd = (ddElement*)rel;
	if (!dd->get_is_def())
	  dd = NULL;
      }
    }
    if (dd || sym.is_loaded() || needLoad::ask()) {
      if (sym.get_kind() != DD_MODULE) {
	if (!dd)
	  dd = sym.get_def_dd();
	if (dd) {
	  node = checked_cast(smtTree,get_relation(def_smt_of_dd,dd));
	}
      } else {
	app * def_app = sym.get_def_app();
	if (def_app && is_smtHeader(def_app)) 
	  node = checked_cast(smtTree, def_app->get_root());
      }
    }
  }
}

void get_smt_of_sym(symbolPtr& sym, smtTree *&node, smtHeader *& h) {
    Initialize(get_smt_of_sym);

    get_smt_of_sym(sym, node);
    if (node) h = checked_cast(smtHeader, node->get_header());
}


ddElement* dd_lookup_or_create(char const *ns, appTree* node, int xref_type, 
	               ddKind k, RelType* rel, char const *as_name);

int smt_generate_local_vars(smtTree * node, smtHeader * h)
{
    Initialize(smt_generate_local_vars(smtTree*, smtHeader*));
  
    if(!(node && h)) return -1;

    smtTree * p;
    for ( p = node->get_first (); p; p = p->get_next()) {
	int type = p->type;
	if ((type == SMT_decl) || (type == SMT_pdecl)) {
	    //smtTree* dec = checked_cast(smtTree,get_relation(id_of_decl, node));
	    int check_local_flag = ((get_relation(def_dd_of_smt, p) == NULL) && (get_relation(decl_dd_of_smt, p) == NULL));
	    // get parameter decls and any decl with no ddElement associated with it
	    if (check_local_flag || (type == SMT_pdecl)) { 
		genString name = 0;
		ddElement* type = checked_cast(ddElement, get_relation(semtype_of_smt_decl, p));
		if (type) { 
		    name += type->get_name(); 
		    name += " ";
		}
		smtTree* id = checked_cast(smtTree, get_relation(id_of_decl, p));
		
		bool good = 0;
		if (id && id->tlth) {
		    good = 1;
		    genString nm;
		    nm.put_value(&h->srcbuf[id->tbeg], id->tlth);
		    name += nm;
		}
		
		ddElement * dd = NULL;	
		if (good) dd = dd_lookup_or_create((char *)name, (appTree*)h, 0, DD_LOCAL, 0, 0);
		if(dd) {
		    put_relation(ref_smt_of_dd, dd, p); 
		}	
	    }
	}
    }
    
    for ( p = node->get_first (); p; p = p->get_next()) 
	//if (p->type != SMT_decl)
	smt_generate_local_vars(p, h);
    return 0;
}

struct Untok_State {
  char const* buffer;
  int pos;
  int len;
  Untok_State(char const* txt, int of, int ln) : buffer(txt), pos(of), len(ln) {} 
  ~Untok_State() {}
};
  
static int untok_get_next_char(Untok_State* st)
{
  int retval = 0;

  int cpp_comment = 0;
  int c_comment = 0;
  char const * buffer= st->buffer;

  while (++st->pos < st->len) {
    if (c_comment) {
      if (buffer[st->pos] == '*' && buffer[st->pos+1] == '/') {
	st->pos++;
	c_comment = 0;
      }
    } else if (cpp_comment) {
      if (buffer[st->pos] == '\n')
	cpp_comment = 0;
    } else {
      if (buffer[st->pos] == '/' && buffer[st->pos+1] == '*') {
	c_comment = 1;
	st->pos++;
      } else if (buffer[st->pos] == '/' && buffer[st->pos+1] == '/') {
	cpp_comment = 1;
	st->pos++;
      } else {
	retval = buffer[st->pos];
	break;
      }
    }
  }
  
  return retval;
}

static int untok_empty(smtTree* node, smtHeader* h)
{
  int retval = 1;

  char const* buffer = &h->srcbuf[node->tbeg];
  int len = node->tlth;
  Untok_State st(buffer,-1,len);

  int c;

  while (retval && (c=untok_get_next_char(&st))) {
    switch (c) 
      {
      case ' ' : 
      case '\t' :
      case '\n' :
      case '\r' :
	break;
      
      default:
	retval = 0; 
      }
  }
  
  return retval;
}

static int untok_contains (char const* buffer, int buflen, char const* name, int len)
{
  int retval = 0;

  int match = 0;
  int c;

  Untok_State st(buffer,-1,buflen);

  while (c=untok_get_next_char(&st)) {
    if (c==name[match]) {
      match++;
      if (match == len) {
	retval = 1;
	break;
      }
    } else
      match = 0;
  }

  return retval;
}

// Count a particular token in SMT, ignoring "#if 0"'d out parts.
static int smt_token_count(smtTree * node, smtHeader * h, 
	int ext_type, char const * name, int len) {
    int cnt = 0;
    smtTree * p;

    bool macros = false;

    for ( p = node->get_first (); p && !macros; p = p->get_next()) 
      if (p->type == SMT_token) {
	if (p->extype == ext_type) {
	  if (p->tlth == len && strncmp(&h->srcbuf[p->tbeg], name, len) == 0)
	    cnt++;
	} else if (ext_type == SMTT_op && p->extype == SMTT_untok && p->tlth >= len) {
	  if (untok_contains(&h->srcbuf[p->tbeg], p->tlth, name, len))
	    cnt++;
	}
      } else if (p->type == SMT_m_if || p->type == SMT_m_else || p->type == SMT_m_endif)
	macros = true;

    if (macros) cnt = 0;     // Don't count tokens if there are #if's, etc.
    
    for ( p = node->get_first (); p; p = p->get_next()) 
      if (p->type != SMT_token)
	cnt += smt_token_count(p, h, ext_type, name, len);

    return cnt;
}

static int smt_node_count(smtTree * node, int type) {
//    Initialize(smt_node_count);

    int cnt;
    
    if (node->type == type) 
      cnt = 1;
    else
      cnt = 0;
    
    if (node->type != SMT_token) {
	for (smtTree * p = node->get_first(); p; p = p->get_next()) 
	  cnt += smt_node_count(p, type);
    }  
    return cnt;
}

static int depth_recursive(smtTree * node) {
//    Initialize(depth_recursive);
    
    int cnt = 0;
    
    switch (node->type) {
      case SMT_token:
	break;
	
      case SMT_if:
      case SMT_switch:
      case SMT_nstdloop:
	cnt ++ ;
	
	// Fall through
	
      default:
	{
	    int max = 0;
	    int subdepth;
	    for (smtTree * p = node->get_first (); p; p = p->get_next()) {
		subdepth = depth_recursive(p);
		if (subdepth > max) max = subdepth;
	    }
	    cnt += max;
	}
	break;
    }
    return cnt;
}

static int dang_elseifs_recursive(smtTree * node) {
//    Initialize(dang_elseifs_recursive);

    int cnt = 0;
    smtTree * p;
    
    if (node->type == SMT_if) {
	bool elses = false;
	bool elseifs = false;
	for (p = node->get_first (); p; p = p->get_next()) {
	    if (p->type == SMT_else_if_clause) elseifs = true;
	    if (p->type == SMT_else_clause) elses = true;
	}
	if (elseifs && !elses) cnt ++;
    }
    if (node->type != SMT_token)
      for (p = node->get_first (); p; p = p->get_next()) 
	cnt += dang_elseifs_recursive(p);
    return cnt;
}

void exec_lines_recursive(smtTree * node, int & count, int & last, bool count_ifdefs, smtHeader* &head) {
//    Initialize(exec_lines_recursive);

    smtTree * p;

    bool macros = false;
    
/*
    if (!count_ifdefs) {
	for ( p = node->get_first (); p && !macros; p = p->get_next()) 
	  if (p->type == SMT_m_if || p->type == SMT_m_else || p->type == SMT_m_endif)
	    macros = true;
    }
*/

    for ( p = node->get_first (); p; p = p->get_next()) 
      switch (p->type) {
	case SMT_comment:
	  break;
	case SMT_token:
		if (!macros) 
		{
			int exttype = p->get_extype();
			// do not count lines with only '{' or '}' as executable lines
			if (count_ifdefs || (exttype != SMTT_begin && exttype != SMTT_end  && exttype != SMTT_untok))
			{
			  if (exttype == SMTT_untok) {
			    if (!head)
			      head = (smtHeader*)p->get_header();
			    if (untok_empty(p,head))
			      break;
			  }
			  smt_fill_src_line_num(p);
			  int line = p->get_linenum();
			  if (line > last) 
			    {
			      last = line;
			      count ++;
			    }
			}
		}
	  break;
	
	case SMT_m_if:
	case SMT_m_endif:
	case SMT_m_else:
		if (!count_ifdefs)
			macros = true;
	case SMT_fdecl:
		if (!count_ifdefs) 
			break;

	default:
		exec_lines_recursive(p, count, last, count_ifdefs, head);
      }
}

void Metric::node_metric(symbolPtr& sym, smtTree *& node, void * res, int type) {
    Initialize(Metric::node_metric);

    get_smt_of_sym(sym, node);
    if (!node) return;

    *(int*)res = smt_node_count(node, type);
}
    
void Metric::token_metric(symbolPtr& sym, smtTree *& node, void * res, int ext_type, char const *name) {
    Initialize(Metric::token_metric);

    smtHeader * h;
    get_smt_of_sym(sym, node, h);
    if (!node) return;

    *(int*)res = smt_token_count(node, h, ext_type, name, strlen(name));
}

void Metric::cyclomatic_complexity(symbolPtr& sym, smtTree *& node, Relational*, void * res) {
    Initialize(cyclomatic_complexity);
    
    smtHeader * h;
    
    get_smt_of_sym(sym, node, h);

    if (!node) return;

    *(int*)res = smt_node_count(node, SMT_then_clause)       +
      smt_node_count(node, SMT_case_clause)                  +
	smt_node_count(node, SMT_else_if_clause)             +
	  smt_node_count(node, SMT_nstdloop)                 -
	    smt_token_count(node, h, SMTT_kwd, "default", 7) + 1;

}

void Metric::returns(symbolPtr& sym, smtTree *& node, Relational*, void * res) {

    token_metric(sym, node, res, SMTT_kwd, "return");
}

void Metric::switches(symbolPtr& sym, smtTree*& node, Relational*, void* res) {

    node_metric(sym, node, res, SMT_switch);
}

void Metric::loops(symbolPtr& sym, smtTree*& node, Relational*, void* res) {

    node_metric(sym, node, res, SMT_nstdloop);
}

void Metric::ifs(symbolPtr& sym, smtTree*& node, Relational*, void* res) {

    node_metric(sym, node, res, SMT_if);
}

void Metric::defaults(symbolPtr& sym, smtTree*& node, Relational*, void* res) {

    token_metric(sym, node, res, SMTT_kwd, "default");
}

void Metric::dang_switch(symbolPtr& sym, smtTree*& node, Relational* met, void* res) {
    Initialize(Metric::dang_switch);

    // Num. of switches - num. of defaults = num. of switches without defaults.
    int a = compute_one_metric_int(MET_SWITCHES, sym, node, met);
    int b = compute_one_metric_int(MET_DEFAULTS, sym, node, met);
    if (a>=0 && b>=0) *(int*)res = a - b;
}

// Number of if-blocks that have else-if clause(s) but no else clause.
void Metric::dang_elseif(symbolPtr& sym, smtTree*& node, Relational*, void* res) {
    Initialize(Metric::dang_elseif);

    get_smt_of_sym(sym, node);
    if (!node) return;

    *(int*)res = dang_elseifs_recursive(node);
}

void Metric::fanin(symbolPtr& sym, smtTree*&, Relational*, void* res) {
    Initialize(Metric::fanin);
    
    symbolArr links;
    sym->get_link(has_superclass, links);
    
    *(int*)res = links.size();
}

void Metric::fanout(symbolPtr& sym, smtTree*&, Relational*, void* res) {
    Initialize(Metric::fanout);
    
    symbolArr links;
    sym->get_link(has_subclass, links);
    
    *(int*)res = links.size();
}

void Metric::myers(symbolPtr& sym, smtTree*& node, Relational* met, void* res) {
    Initialize(Metric::myers);

    smtHeader * h;
    get_smt_of_sym(sym, node, h);
    if (!node) return;
    
    *(int*)res = compute_one_metric_int(MET_CC, sym, node, met) + 
      smt_token_count(node, h, SMTT_op, "&&", 2) +
	smt_token_count(node, h, SMTT_op, "||", 2) +
	  smt_token_count(node, h, SMTT_op, "?", 1);
}

void Metric::number_of_lines(symbolPtr& sym, smtTree *& node, Relational *, void * res) {
    Initialize(Metric::number_of_lines);

    get_smt_of_sym(sym, node);
    if (!node) return;
    smtTree * leaf;
    int n;

    smt_fill_src_line_num(node);
    int num = node->get_linenum();
    if (num > 0) 
      *(int*)res = node->get_last_line(leaf, n) - num;
}

void Metric::uncom_lines(symbolPtr& sym, smtTree *& node, Relational *, void * res) {
    Initialize(Metric::uncom_lines);

    get_smt_of_sym(sym, node);
    if (!node) return;

    int last = -1;
    *(int*)res = 0;
    smtHeader* head = NULL;
    exec_lines_recursive(node, *(int*)res, last, true, head);
}

void Metric::exec_lines(symbolPtr& sym, smtTree *& node, Relational *, void * res) {
    Initialize(Metric::uncom_lines);

    get_smt_of_sym(sym, node);
    if (!node) return;

    int last = -1;
    *(int*)res = 0;
    smtHeader* head = NULL;
    exec_lines_recursive(node, *(int*)res, last, false, head);
}

void Metric::starting_line(symbolPtr& sym, smtTree *& node, Relational*, void * res) {
  Initialize(Metric::starting_line);
  
  int num = -1;

  get_smt_of_sym(sym, node);

  if (node) {
    smt_fill_src_line_num(node);
    num = node->get_linenum();
  }
  
  if (num <= 0)  { // try more
    num = get_line_of_sym (sym);
  }

  if (num > 0)
    *(int*)res = num;
}

void Metric::bytelen(symbolPtr& sym, smtTree *& node, Relational *, void * res)
{
    Initialize(Metric::bytelen);

    smtHeader * h;
    get_smt_of_sym(sym, node, h);
    if (!node) return;

    *(int*)res = (node->type == SMT_file) ? node->length() : node->ilength();
}

void Metric::depth(symbolPtr& sym, smtTree *& node, Relational *, void * res) {
    Initialize(Metric::depth);

    get_smt_of_sym(sym, node);
    if (!node) return;

    *(int*)res = depth_recursive(node);
}

//sudha:09/29/97 calculate line offsets  
void get_line_offsets(objArr_Int *smt_line_off, smtHeader *h)
{
   smt_line_off->insert_last(0);
   if (h) {
     int i;
     char ch;
		 for ( i=0; i < h->src_size; i++ ){
		   ch = h->srcbuf[i];
       if (ch == '\n')
          smt_line_off->insert_last(i+1);
     } 
   }
 }

extern int dfa_prepare_instances(smtHeader* h);
extern int projNode_set_pmod_names(projNode *);
void MetricSet::set_metrics_of_smtHeader(smtHeader * h, bool from_build) 
{
    Initialize(set_metrics_of_smtHeader);
    if (!h) return;

    //MB: debug_get_print_flags ();

		objArr_Int smt_line_off;
    get_line_offsets(&smt_line_off, h);

    projNode * pr = h->get_project();

    //boris: 082696 Made sure, that sym_name exists for the project
    for ( ; pr && !pr->get_xref (1) ; pr = pr->find_parent () )
	;

    if (!pr || !projNode_set_pmod_names(pr)) {
	msg("WARN: Metrics Error: cannot find pmod project for file $1") << h->get_name() << eom;
	return;
    }

    dfa_prepare_instances(h);
    MetricSet * set = proj_get_set(pr);
    if (!set) set = new MetricSet(pr);  // If no set is associated with project -- create one.
    
    symbolPtr fsym ( h );		
    // Find the symbol corresponding to this file
    
    fsym = fsym.find_xrefSymbol();
    if (fsym.isnull()) return;
    ddElement *file_dd=checked_cast(ddElement,get_relation(dd_of_appHeader,h));

    MetricElement fmel_old;
    set->find_element(fsym, fmel_old);
    
    int old_offset = -1;

    FILE * file = NULL;
    if (set->file_name.not_null()) {
	struct OStype_stat buf;
	if (OSapi_stat(set->file_name, &buf) == 0)
	  file = OSapi_fopen(set->file_name, "rb+");
	else
	  file = OSapi_fopen(set->file_name, "wb+");
    }
    if (fmel_old.isnotnull()) {
      if (is_gui()) {
	symbolArr removed;
	fmel_old.find_inst_between_offsets(fsym,0,-1,removed);
	symbolPtr rem;
	ForEachS(rem,removed) {
	  xref_notifier_report(-1,rem);
	}
      }
	// Clear whatever we had in file for this symbol
	if (file) {
	    if (OSapi_fseek(file, (long)fmel_old.get_offset()*sizeof(MLink), SEEK_SET) == 0) {
		MLink lnk;
		int old_size = fmel_old.get_size();
		lnk.assign(MET_FREE, 1, old_size);
		OSapi_fwrite(&lnk, sizeof(MLink), 1, file);
	    }
	}
	old_offset = fmel_old.get_offset() ;  /* Next time we look for space, start with the
					     old location */
 	set->remove_element(fsym);  /* removes recursively everything fsym used to define,
				       and fsym itself. */
	// From this point on, fmel_old cannot be used
    }
    MetricElement fmel(fsym);

    MetricElement bmel(MET_CHILDREN);



    // make list of all ddElements refered to in file (for instance lists)
    objArr defs;
    objArr refs;

    // first generate ddElements for local vars
    if (!local_variables_done) 
      smt_generate_local_vars(checked_cast(smtTree,h->get_root()),h);

    ddRoot * dr = dd_sh_get_dr(h);
    if (dr == 0) return;
    for (int i = 0; i < MAX_HASH; ++i ) {
      for (hash_element* he = dr->dd_hash[i]; he; he = he->next) {
	ddElement * dd = he->el;
	if (dd->get_is_def()) {
	  if (dd != file_dd)
	    defs.insert_last(dd);
	} else {
	  Obj* ob = get_relation(ref_smt_of_dd, dd);
	  //don't include compiler generated elements
	  if ( (ob && ob->size() > 0) || (dd->kind == DD_LOCAL) )
	    refs.insert_last(dd);
	}
      }
    }

    Obj *el;
    ForEach(el, defs) {
	ddElement * del = (ddElement*)el;
	symbolPtr sym = del;
	ddKind knd = sym.get_kind();
	MetricElement mel(sym);
	mel.compute_set_values(sym);
	mel.add_instance_list(del,fsym,&smt_line_off);
	
	if (mel.get_size()>2) {
	    // There is something in mel	  
	    bmel.append(mel);
	}
    }

    
    // add instance lists for elements referenced, but not defined in file
    int size = refs.size();
    for (int j=0; j < size; j++) { 
      ddElement * del = (ddElement*) refs[j];
      symbolPtr sym = del;
      MetricElement mel(sym);
      mel[0].assign(MET_UNDEFINED_ELEMENT, 1, 2);
      mel.add_instance_list(del,fsym,&smt_line_off);
      if (mel.get_size()>2) {
	// There is something in mel	  
	bmel.append(mel);
      }
    }

    fmel.compute_set_values(fsym);

    fmel.add_instance_list(file_dd,fsym,&smt_line_off);
    fmel.append(bmel);
    
    // Now add to the hash table all the elements in the file.
    
    set->add_element(fmel, old_offset);
    
    // Now write the modified symbol into file
    if (file) {
	OSapi_fseek(file, (long)fmel.get_offset()*sizeof(MLink), SEEK_SET);
	int size = fmel.get_size();
	if (fmel.get_offset() + size < set->get_table()->size()) 
	  size +=1;   // add 1 to get possible MET_FREE appended to element
	OSapi_fwrite(&fmel[0], sizeof(MLink), size, file);    
	OSapi_fclose(file);
	file = NULL;
    }
    if (is_gui()) {
      symbolArr added;
      fmel.find_inst_between_offsets(fsym,0,-1,added);
      symbolPtr add;
      ForEachS(add,added) {
	xref_notifier_report(1,add);
      }
    }
}

void MetricSet::get_metrics_of_sym_aux(symbolPtr def_file, symbolPtr sym) {
    Initialize(MetricSet::get_metrics_of_sym_aux);

    symbolPtr xsym = sym.get_xrefSymbol();
    if(xsym.isnotnull() && xsym.is_loaded_or_cheap()){
	bool was_loaded = def_file.is_loaded();
	app * def_app = sym.get_def_app();
	if (def_app && is_smtHeader(def_app)) 
	    set_metrics_of_smtHeader(checked_cast(smtHeader, def_app));
	if (def_app && !was_loaded) 
	    obj_unload(def_app);
    }
}

void MetricElement::add_instance_list(ddElement* ddel, symbolPtr& scope, objArr_Int *smt_line_off) {
    Initialize (MetricElement::add_instance_list);
    
    if (ddel == NULL) return;
    if (ddel->get_kind() == DD_LOCAL) {
      add_local_var_instance_list(ddel,scope, smt_line_off);
      return;
    }

    symbolPtr xsym = ddel->get_xrefSymbol();
    if (xsym.xrisnull())
      return;

    Obj* ob = get_relation(ref_smt_of_dd, ddel);
    MetricSorter msrt(ob);
    MetricElement bmel(MET_INSTANCE_BLOCK);
    Obj* el;
    {
	ForEach(el,msrt.sorted) {
	    smtTreePtr stree = checked_cast(smtTree, el);
	    if (stree->type == SMT_token) {
	      MetricElement imel(MET_INSTANCE);
	      generate_instance(imel, stree, ddel, scope, smt_line_off);
	      bmel.append(imel);
	    }
	}
    }
    if (bmel.get_size() > 1)
      append(bmel);
}


void MetricElement::add_local_var_instance_list(ddElement* ddel, symbolPtr& scope, objArr_Int *smt_line_off) {
    Initialize (MetricElement::add_local_var_instance_list);
    
    if (ddel == NULL) return;
    if (ddel->get_kind() != DD_LOCAL) return;

    symbolPtr xsym = ddel->get_xrefSymbol();
    if (xsym.xrisnull())
      return;
    Obj* ob= get_relation(ref_smt_of_dd, ddel);
    MetricSorter msrt(ob);
    
    MetricElement decBlock(MET_LOCAL_DEC_BLOCK);    
    Obj * el;
    {ForEach(el,msrt.sorted) {
	smtTreePtr stree = checked_cast(smtTree, el);
	smtTreePtr smtDecl = checked_cast(smtTree, get_relation(id_of_decl,stree));
	
	if (smtDecl && smtDecl->type == SMT_token) {
	    MetricElement dmel(MET_DECL_INSTANCE);
	    generate_instance(dmel, smtDecl, ddel, scope, smt_line_off);
	    

	    Obj* ob2 = get_relation(reference_of_declaration, stree);
	    MetricSorter msrt2(ob2);
	    
	    MetricElement instBlock(MET_INSTANCE_BLOCK);    
	    Obj* el2;
	    {ForEach(el2,msrt2.sorted){
		smtTreePtr smtInst = checked_cast(smtTree, el2);
		if (smtInst != smtDecl && smtInst->type == SMT_token) {
		    MetricElement imel(MET_INSTANCE);
		    generate_instance(imel, smtInst, ddel, scope, smt_line_off);
		    instBlock.append(imel);
		}
	    }}
	    if (instBlock.get_size() > 1)
		dmel.append(instBlock);
	    
	    decBlock.append(dmel);
	}
    }}
    if (decBlock.get_size() > 1)
      append(decBlock);
}

void MetricElement::generate_instance(MetricElement& imel, smtTree* stree, 
				       ddElement* ddel, symbolPtr mod, objArr_Int *smt_line_off)

//   This method will put the standard instnace information into imel.
// stree is the particular instnace of ddel, and scope is the containing object.
{
  Initialize(MetricElement::generate_instance);

  imel.set_value(MET_INSTANCE_OFFSET, stree->tbeg);
  imel.set_value(MET_INSTANCE_LINE, stree->src_line_num);
  imel.set_value(MET_INSTANCE_LENGTH, stree->tlth); /* NEW */
  if (smt_line_off) {
    int loff = -1;
    if (stree->src_line_num > 0)
      loff = stree->tbeg - (*(smt_line_off))[stree->src_line_num - 1];
    imel.set_value(MET_INSTANCE_LN_OFFSET, loff);
  }
  smtHeader* h = (smtHeader*)stree->get_header();
  astRoot * ast = (astRoot*)get_relation(astRoot_of_smtHeader,h);
  if (ast) {
      int index = ast->smts.index(stree);
      if(index >= 0) {
	  imel.set_value(MET_AST_CATEGORY,ast->data[index]->ast_category);
	  if (ast->data[index]->offset)
	      imel.set_value(MET_USED_BY_SYMBOL, ast->data[index]->offset);
      }  
  }
  symbolPtr xsym = ddel->get_xrefSymbol();
  if (xsym.xrisnotnull()) 
    imel.set_value(MET_INSTANCE_SYMBOL, xsym.get_offset());
  
  
  bool added_scope = false;
  smtTree*world = checked_cast(smtTree, stree->get_world());
  if(world){
    ddElement* dd = smt_get_dd(world);
    if (dd && (dd->get_kind() != DD_PARAM_DECL) && dd->get_is_def()) {
      symbolPtr s = dd->get_xrefSymbol();
      if (s.xrisnotnull()){
	imel.set_value(MET_INSTANCE_SCOPE, s.get_offset());
	added_scope = true;
      }
    }
  }
  if (!added_scope && xsym.xrisnotnull()) {
    mod = mod.get_xrefSymbol();
    if (mod.xrisnotnull()){
      imel.set_value(MET_INSTANCE_SCOPE,mod.get_offset());
      added_scope = 1;
    }
  }
  //if (!added_scope) cout << "  ############# no scope added for " << xsym.get_name() << endl;
}


extern smtTree *dis_get_node_with_file_offset (char const *f_name, int lin, int off, int tab);

projModule * symbol_get_module( symbolPtr& sym);

// Returns next token and next token offset for the given node and length
// st_offset - inside offset of a start_node (0 by default);
// end_flag  - 1: seeking end margin of a region, (0 by default);
smtTreePtr smt_get_token_by_offset (smtTreePtr st, int off, int& new_off,
                                    int end_flag, int st_offset)
{
    Initialize(smt_get_token_by_offset);
    smtHeaderPtr hd;
    if (!st || !(hd = checked_cast(smtHeader,st->get_header())))
	return NULL;
 
    int pnt = st->start() + st_offset + off;
    smtTreePtr nd = hd->cp_na (pnt);
    if (nd) {
	new_off = pnt - nd->start();
	hd->check_margin (nd, new_off, end_flag);
    }
 
    return nd;
}
 
smtTree* inst_get_token( symbolPtr& sym )
{
    Initialize( inst_get_token );
    
    MetricElement mel = sym.get_MetricElement();
    MetricSet* mset = sym.get_MetricSetPtr();
    
    smtTree* retval = NULL;
    if (mel.isnotnull()) 
	if ((mel[0].type() == MET_INSTANCE) ||
	    (mel[0].type() == MET_DECL_INSTANCE)) {
	    symbolPtr isym(&mel,mset);
	    symbolPtr scope = isym.scope();
	    symbolPtr fsym;
	    if (scope.xrisnotnull())
		fsym = scope->get_def_file();
	    projModule *mod = NULL;
	    if (fsym.isnotnull())
		mod = symbol_get_module (fsym);
	    int off = mel.get_value(MET_INSTANCE_OFFSET);
	    if(mod && off >= 0){
		appPtr app = mod->restore_module();
		if(app && is_smtHeader(app)){
		    smtHeader *h = (smtHeader *)app;
		    retval       = h->cp_na(off);
		}
	    }
	}
    return retval;
}

smtTree* inst_get_statement( symbolPtr& sym )
{
    Initialize( inst_get_statement );
    smtTree* ptree = NULL;
    if (sym.is_instance()) {
	ptree = inst_get_token( sym );
	while (ptree){
	    if(smt_is_statement(ptree))
		break;
	    int type = ptree->get_node_type();
	    switch(type){
	    case SMT_fdecl:
	    case SMT_cdecl:
	    case SMT_list_decl:
	    case SMT_list_pdecl:
	    case SMT_macrocall:
		return ptree;
	    }
	    smtTree* par = ptree->get_parent();
	    if(!par)
		break;
	    if(par->get_parent() == NULL)
		break;
	    ptree = par;
	}
    }
    return ptree;
}


char const *get_statement_of_inst(symbolPtr& sym)
{
  Initialize(get_statement_of_inst);
  char const *result = NULL;

  if (sym.is_instance()) {

    smtTree* ptree = inst_get_statement( sym );
    
    if (ptree) {
      static ocharstream os;
      os.reset();
      os << '\0';
      os.reset();

      for(smtTree * x = checked_cast(smtTree,ptree->get_first_leaf());
	  x && x->subtree_of(ptree);
	  x = checked_cast(smtTree,x->get_next_leaf()))
      {
	x->send_string(os);
      }
      os << '\0';
      result = os.ptr();
    }
  }
  return result;
}


//NEW BLOCK BEGIN

extern char const* ATT_instance_scopefile( symbolPtr& sym);

char const *inst_get_region(symbolPtr& sym)
{
  Initialize(inst_get_region);
  
  static  char buf[1024];
  buf[0] = NULL;

  if (sym.is_instance()){
        char const* lf =  ATT_instance_scopefile( sym);
        smtTree* stree = inst_get_statement( sym );
	if( stree && lf ) {

	    int start = stree->istart();
	    int end   = start + stree->ilength();

      	    OSapi_sprintf( buf , "\n%s %d %d" ,  lf , start , end );
	}
    }

  return buf;
}
//NEW BLOCK END

