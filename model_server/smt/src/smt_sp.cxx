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
// WARNING:
// THIS FILE DEFINES LOTS OF FUNCTIONS CALLED FROM .l FILE.  Dont' trust
// the DISCOVER model for dormant code.

// File smt_sp.C
#include "objTree.h"

#include "smt.h"
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <ddict.h>
#include <cmd_enums.h>

extern "C"
  {
  void smt_shallow_parser_c();
  void smt_out_stmt_c(int, int, int, int);
  int smt_lex_token_c();
  void smt_lex_init_c();
  int smt_out_token_c(int begin, int length, int skip, int ttype);
  int sp_hdr_is_esql();
  };

static void lex_init(smtHeader *, int, int, smtTree* = 0);
static void smt_lex_end();
int cmd_available_p (int functionality);

#ifndef _WIN32
extern int smt_spylineno;
#else
extern "C" int smt_spylineno;
#endif
static smtHeader *header;
static char * newbuf;
static int inewbuf;
static int get_base;
static int smt_import_from_file;
static int was_parsed;
#ifdef _WIN32
extern "C"
{
#endif
char * smt_lex_input_buffer;
#ifdef _WIN32
}
#endif
int smt_was_nl;
static int spaces, newlines, position;
static smtTree * cur_token;
static smtTree *next_token;

void smt_build_comments(smtTree * root);

static int esql_hdr;

static void register_hdr_for_esql_check(smtHeader* h) {
    esql_hdr = (h->language == smt_ESQL_C || h->language == smt_ESQL_CPLUSPLUS);
}

int sp_hdr_is_esql() {
    return esql_hdr;
}

void smt_shallow_parser(smtHeader *h)
  {
  Initialize(smt_shallow_parser);
  int i;
  smt_import_from_file = 1;
  header = h;
  h->last_token = 0;
  was_parsed = h->parsed;
  h->parsed = 1;
  smtTree * t = checked_cast(smtTree,h->get_root());
  t = t->get_first();
  for(i = t->tbeg; i < t->tlth; i++)
    if(!isspace(h->srcbuf[i]))
      break;
  if(i == t->tlth)
    {
    if(i == 0)
    t->tbeg = 0;
    t->tlth = 0;
    t->extype = SMTT_el;
    t->tnfirst = t->tnmax = 1;
    h->set_ttable(t);
    smt_def_stmt(h, 0, 1, 1, SMT_comment);
    }
  else
    {
    register_hdr_for_esql_check(h);
    lex_init(header, 0, h->src_size);

    while(smt_lex_token_c()>0);
//    smt_shallow_parser_c();
    smt_lex_end();
    }
  h->tidy_tree();
  smt_build_comments(checked_cast(smtTree,h->get_root()));
  return;
  }

void smt_tokenizer(smtHeader *h, smtTree *token)
  {
  Initialize(smt_tokenizer);
  smtTree* par = token->get_parent();
  int from = token->tbeg;
  int lth =  token->tlth;

  smt_import_from_file = 0;
  was_parsed = 0;
  Assert(h->parsed);
  header = h;
  h->last_token = token;  //// was = 0;
  register_hdr_for_esql_check(h);
  lex_init(header, from, lth, token->get_next_leaf ());

  int type;
  int no_tok=0;
  while((type=smt_lex_token_c())>0)
      ++no_tok;

//  smt_shallow_parser_c();
  smt_lex_end();

  if(no_tok)
     smt_kill_comment(token);
///  else
///     token->arn = 0;

//  if(h->parsed || from < 0)
    h->tidy_tree();

  smt_build_comments(par);
  return;
  }



void smt_out_stmt_c(int mode, int type, int s1, int s2)
  {
  if(header->parsed)    // Do nothing if there is no parse tree
    smt_def_stmt(header, mode, type, s1, s2);
  }

int smt_out_token_c(int begin, int length, int skip, int ttype)
  {
  int tn1, tn2, /*  i, */ lcnt, ind;
  char * wc;
  if(was_parsed)
    {
    if(skip)
      return 0;
    else
      return header->cn_na(begin)->tnfirst;
    }

  begin += get_base;

  char * buffer =  header->srcbuf;
//  smt_was_nl = 0;
  if(header->parsed)
    // If tree exist - define SMT_token node on tree (or delete white space)
    {
    int tn;
    switch(skip)
      {
	int nl;	
      case 0:   // Normal token
        nl = 0;  // newlines are added later because they are at the end

        tn =  smt_def_token(header, begin, length, skip, ttype, nl, spaces);

        cur_token = header->tn_na(tn);

        lcnt = length;
        for(wc = buffer + begin; lcnt--; wc++)
          switch(*wc)
            {
            case '\n': case '\r': case '\f':
              position = 0;
              break;
            default:
              position++;
              break;
            }
        spaces = newlines = smt_was_nl = 0;

        
        if(smt_import_from_file)        // If it is import from a file
          {
          smtTree * t = header->tn_na(tn);
          t->src_line_num = smt_spylineno;
          }
        break;
      case -1:          // discard first character
        smt_def_token(header, begin, 1, 1, 0);
        tn = smt_def_token(header, begin + 1, length - 1, 0, ttype);
        break;
      case -2:          // replace character by space
        header->srcbuf[begin] = ' ';
        tn = smt_def_token(header, begin, length, 0, ttype);
        break;
      case 1:           // skip
        if(header->foreign)
          {
          smt_def_token(header, begin, length, 1, 0);
          tn = 0;
          break;
          }

        ind = begin;
        tn1 = tn2 = -1;
        lcnt = length;
        for(wc = buffer + begin; lcnt--; wc++,++ind)
          {
          int not_mapped_yet = 1;
          switch(*wc)
            {
            case '\n':
              if(smt_was_nl || ( ! cur_token) || spaces) {
                  // Make newline token
                  tn2 = smt_def_token(header, ind, 1, 0, SMTT_el, 1, spaces);
                  if(tn1 < 0)
                        tn1 = tn2;
                  not_mapped_yet = 0;
              } else {
                  cur_token->newlines = 1;
              }

              smt_was_nl ++;
              spaces = position = 0;
              newlines++;
              break;
            default:
              spaces++;
              position++;
              break;
            } // switch(*wc)

           if(not_mapped_yet)
                smt_def_token(header, ind, 1, 1, 0);    // Discard byte
          } // for
        tn = 0;
        break;
      }

    return tn;
    }
  else
    {
    // Else define token in new text buffer
    int beg = inewbuf;
    if(skip)
      return 0;

    for(char * pf = header->srcbuf + begin; length--; pf++)
      {
      newbuf[inewbuf++] = *pf;
      }

    newbuf[inewbuf++] = '\n';
    }
  return 0;
}

static void lex_init(smtHeader * h, int from, int lth, smtTree* next)
  {
  smt_lex_input_buffer = new char[lth + 1];
  strncpy(smt_lex_input_buffer, h->srcbuf + from, lth);
  smt_lex_input_buffer[lth] = 0;
  get_base = from;
  next_token = next;
  cur_token = NULL;
  smt_was_nl = 0;

  smt_lex_init_c();
  header = h;

  if(!header->parsed)
    {
    newbuf = new char[header->src_size * 2 + 1];
    // Size of just allocated buffer is enough even if every character is
    // token
    inewbuf = 0;
    }

  newlines = 0;
  spaces = 0;
  position = 0;
  return;
  }

static void smt_lex_end()
{
    Initialize(smt_lex_end);

    if (spaces) {
	if (next_token) 
	    next_token->spaces += spaces;
	else {
	    smtTree * root = checked_cast(smtTree,header->get_root());
	    smtTree * last = root->get_last_leaf();
	    smtTree * tn = db_new(smtTree,(SMT_token));
	    last->put_after(tn);
	    tn->tbeg = last->tbeg + last->tlth + last->newlines + spaces;
	    tn->tlth = 0;
	    tn->spaces = spaces;
	    tn->newlines = 0;
	    tn->extype = SMTT_el;
	    tn->tnfirst = tn->tnmax = last->tnfirst + 1;
	}
    }

    delete smt_lex_input_buffer;
    if(!header->parsed)
	newbuf[inewbuf] = 0;
}

static int is_comment(smtTree*tok)
{
  if(! tok) return 0;
  int tp = tok->extype;
  return
      tp == SMTT_lb
   || tp == SMTT_el
   || tp == SMTT_comment
   || tp == SMTT_commentl
  ;
}

char *trim_string_for_pmod(char const *str, int &length, int &lstart)
{
    static genString text;

    if(length > 16000)
	length = 16000; /* This limits string length that could be stored in a model. 
			   Also make sure that buffer to read (YYLMAX) string in api/src/read_group.l is bigger */
    if(str[0] == '"' && str[length - 1] == '"') {
	str++;
	length -= 2;
    } else if (str[0] == '\\' && str[1] == '"' &&
	       str[length-1] == '"' && str[length-2] == '\\') {
      str += 2;
      length -= 4;
    }
    text.put_value(str, length);
    return (char *)text;
}

ddElement * dd_find_string(smtHeader* smth, char* source, int length)
{
    Initialize(dd_find_string);
    
    ddElement* retval = NULL;

    int lstart = 0;
    source     = trim_string_for_pmod(source, length, lstart);
    // -- create an element for strlen(DD_STRING)
    int short_string_len = 2;
    if(cmd_available_p(cmdBuildShortStrings))
	short_string_len = 0;
    if ((length - lstart) > short_string_len)
	retval = dd_lookup_or_create(&source[lstart], (appTree *)smth, 0, DD_STRING, 0, 0);
    return retval;
}

void
smt_process_string(smtHeader* smth, smtTree* token)
{
    Initialize(smt_process_string);

    if (smth!=0 && token!=0) {
	int    lstart = 0;
	int    length = token->tlth-2;
	char * source = &(smth->srcbuf[token->tbeg+1]); // remove initial '"'
	if (length > 0) {
	    ddElement * dd = dd_find_string(smth, source, length);
	    if(dd)
		put_relation(ref_smt_of_dd, dd, token);
	}
    }
}

void smt_build_strings(smtTree * root)
{
  Initialize(smt_build_strings);
 
  smtTree* cur = root->get_first_leaf();
  smtTree* en = root->get_last_leaf();

  smtHeader* h = checked_cast(smtHeader, root->get_header());

  for(;;){
     if(cur->type==SMT_token && cur->extype==SMTT_string)
       smt_process_string(h, cur);
     if(cur==en)
       break;
     cur = cur->get_next_leaf();
  }
}

void smt_build_comments(smtTree * root)
{
  Initialize(smt_build_comments);

  smt_build_strings(root);
  // make parent temp
// Trung does it because parent is SMT_cdecl and smt_def_stmt will splice temp node

  smtTree* cur = root->get_first_leaf();
  smtTree* en = root->get_last_leaf();
  
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
          smt_def_stmt(header, 0, SMT_comment, tn1, tn2);
          comm_flag = 0;
    }
    if(cur==en) break;
  } // for
}

int need_merge (smtHeader*, smtTree*, smtTree*);

static void clear_relation (smtTree* node) 
{
    smtTree* child = node->get_first ();
    if (!child)
	node->rem_rel_all ();
    else
	for (; child; child = child->get_next ())
	    clear_relation (child);
}
	 
static smtTree* smt_get_top (smtTree* t, smtTree* s, int flag)
{
    if (!t || !s)
        return 0;
    smtTree* retval = t;
    smtTree* p = t->get_parent();
    if (flag == 1)
	while (p && p->get_first_leaf() == t) {
	    retval = p;
	    p = p->get_parent();
	}
    else
	while (p && p->get_last_leaf() == t) {
	    retval = p;
	    p = p->get_parent();
	}
    int found = 0;
    for (p = s; p; p = p->get_parent())
	if (p == retval) {
	    found = 1;
	    break;
	}
    return found ? retval : 0;
}    


static bool smt_intersect(smtTree* stmt, smtTree* t, smtTree* node)
{
    for(smtTree* p = node; p && p != stmt; p = p->get_parent())
	if (p == t)
	    return 1;
    return 0;
}

static void smt_split_before (smtTree* stmt, smtTree* end)
{
    if (!stmt)
	return;
    smtTree* par = stmt->get_parent();
    smtTree* prev = 0;
    smtTree* first;
    while (first = stmt->get_first()) {
	while  (smt_intersect(stmt, first, end)) {
	    first = first->get_first();
	    if (!first)
		return;
	}	    
	if (!prev)
	    if (par) 
		stmt->put_before(first);
	    else {
		stmt->put_first(first);
		stmt = first->get_next();
	    }
	else
	    prev->put_after(first);
	prev = first;
	prev->arn = 1;
    }
}

static void smt_split_after (smtTree* stmt, smtTree* begin)
{
    Initialize(smt_split_after);

    if (!stmt)
	return;
    smtTree* par = stmt->get_parent();
    smtTree* prev = 0;
    smtTree* last;
    while (last = checked_cast(smtTree,stmt->get_last())) {
	while  (smt_intersect(stmt, last, begin)) {
	    last = checked_cast(smtTree,last->get_last());
	    if (!last)
		return;
	}	    
	if (!prev)
	    if (par)
		stmt->put_after(last);
	    else {
		objTree* tmp = stmt->get_last();
		if (tmp != last) {
		    tmp->put_after(last);
		    stmt = last->get_prev();
		} else
		    return;
	    }
	else
	    prev->put_before(last);
	prev = last;
	prev->arn = 1;
    }
}

static void smt_kill_file (smtTree* node, int is_root = 1)
{
    smtTree *n;
    smtTree *t;
    for (t = node->get_first(); t; t = n) {
	n = t->get_next();
	smt_kill_file(t, 0);
    }
    if (!is_root && (node->get_node_type () == SMT_file 
		     || node->get_node_type () == SMT_temp && node->was_type ==SMT_file)) {
	smtTree* p = node->get_parent();
	if (node->get_first ())
	    smt_splice (node);
	else {
	    t->remove_from_tree();
	    obj_delete (node);
	}
	if (p && p->get_parent()) {
	    p->was_type = p->type;
	    p->type = SMT_temp;
	}
    }
}

static void smt_kill_dummy_comments (smtTree* root)
{
    smtTree* tok = root->get_first_leaf ();
    smtTree* next;
    for (; tok && (next = tok->get_next_leaf ()); tok = next)  {
	if (tok->newlines)
	    continue;
	if (next->extype != SMTT_el || next->tlth || next->spaces)
	    continue;
	smtTree* empty = next;
	next = next->get_next_leaf ();
	tok->newlines = 1;
	smt_cut (empty);
    }
}

void restore_tokenize (smtHeader* h)
{
    Initialize (restore_tokenize);
    if (!h || !h->src_size || !(h->language == smt_CPLUSPLUS || h->language == smt_C))
	return;
    smtTree* root = checked_cast(smtTree,h->get_root ());
    if (!root) return;
    if (!h->srcbuf) return;

    smtTree *token, *par, *t1, *t2, *next, *curr, *t1p, *t2p;
    int from, lth, rm_token, rm_t2, sp, nl, touched = 0, type = 0;
    for (token = root->get_first_leaf ();
         token;
         token = next) {
        next = token->get_next_leaf ();
        if (token->extype == SMTT_untok) {
            int splitted = 0;
            par = token->get_parent ();
            char* old_string = token->old_string;
            token->old_string = NULL;
            t1p = token->get_prev_leaf (par);
            t2p = token->get_next_leaf (par);
	    t1 = token->get_prev_leaf ();
	    t2 = token->get_next_leaf ();
            rm_token = 0;
            rm_t2 = 0;
            sp = nl = 0;
            if (!type && t1 && (need_merge (h, t1, token)) ) {
		par = (smtTree*)obj_tree_common_root (par, t1);
                from = t1->start();
                lth = t1->length() + token->length();
		t1->tbeg = t1->start();
		t1->spaces = 0;
		t1->newlines = 0;
		h->last_token = t1;
                t1->tlth = lth;
                rm_token = 1;
            } else {
                from = token->start();
                lth = token->length();
                h->last_token = token;
            }
            if (t2 && (type || (need_merge (h, token, t2)))) {
		par = (smtTree*)obj_tree_common_root (par, t2);
                lth += t2->length();
                h->last_token->tlth = lth;
                rm_t2 = 1;
            }
            if (rm_token) {
                token->tbeg = from + lth;
                token->tlth = 0;
		token->spaces = 0;
		token->newlines = 0;
                token->extype = 0;
            }
            if (rm_t2) {
                next = next ? next->get_next_leaf () : 0;
                t2->tbeg = from + lth;
                t2->tlth = 0;
                t2->extype = 0;
		t2->spaces = 0;
		t2->newlines = 0;
            }
            smt_import_from_file = 0;
            touched = 1;
            was_parsed = 0;
            header = h;
	    register_hdr_for_esql_check(h);
            lex_init (header, from, lth, next);
            int no_tok=0;
            curr = h->last_token;
            while ((type=smt_lex_token_c ())>0)
                ++no_tok;
            smt_lex_end ();
            if (rm_token && token->extype == 0) {
		smtTree *del;
		for (del = token;
		     del && !del->get_prev () && !del->get_next ();
		     del = del->get_parent ());
                if (del) {
		    del->remove_from_tree ();
		    obj_delete (del);
		}
            }
            if (rm_t2 && t2->extype == 0) {
		smtTree *del;
		for (del = t2;
		     del && !del->get_prev () && !del->get_next ();
		     del = del->get_parent ());
		if (del) {
		    del->remove_from_tree ();
		    obj_delete (del);
		}
            }
            if (type) {
		next = next ? cur_token : 0;
		if (old_string)
		    delete old_string;
		continue;
	    }	
            if (!rm_token && !rm_t2) {
                if (old_string && *old_string) {
		    if (!t1p) {
                        smtTree *stmt;
                        if (h->last_token 
			    && h->last_token->tlth 
			    == strlen (old_string)
			    && !strncmp (old_string, h->srcbuf +
					 h->last_token->tbeg,
					 h->last_token->tlth)) {
			    smtTree* tkn = h->last_token;
			    if (token != tkn && (stmt = smt_get_top (token, tkn, 1))) {
				tkn->arn = 0;
				smt_split_before(stmt, tkn);
				splitted = 1;
				par = stmt;
				smtTree* pp = par->get_parent();
				if (pp && pp != root)
				    par = pp;
				no_tok = 0;
			    }
			}
		    } else if (!t2p) {
			if (token->tlth == strlen (old_string)
			    && !strncmp (old_string, h->srcbuf + token->tbeg, token->tlth)) {
			    smtTree* stmt;
			    smtTree* last = h->last_token;
			    if (last == 0)
				last = par->get_last_leaf(); 
			    if (token != last && (stmt = smt_get_top (last, token, 0))) {
				token->arn = 0;
				smt_split_after(stmt, token);
				splitted = 1;
				par = stmt;
				smtTree* pp = par->get_parent ();
				if (pp && pp != root)
				    par = pp;
				no_tok = 0;
			    }
			}
		    }
		}
            }	
            delete old_string;
            if (no_tok)
                smt_kill_comment (curr);
            if (!splitted) {
                for (; par && !(par->type == SMT_temp && par->arn == 1) && !smt_is_statement(par);
                     par = par->get_parent());
                if (par) {
		    par->type = SMT_temp;
		    par->arn = 1;
		}
            }
        }
    }
    if (touched) {
        h->set_modified ();
	smt_kill_dummy_comments (root);
	smt_kill_file(root);
        h->tidy_tree ();
        smt_build_comments (root);
    }
    h->untok_flag = 0;
}
