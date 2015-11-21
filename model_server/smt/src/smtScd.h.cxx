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
// File smtScd.h.C - SMODE classes - methods and functions.
//

#include "smt.h"
//#include "steRefNode.h"
#include "ddict.h"
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "appMarker.h"

init_relation(startnode_of_reference, 1, relationMode::S, reference_of_startnode, MANY, NULL);
init_relation(endnode_of_reference, 1, relationMode::S, reference_of_endnode, MANY, NULL);
defifl_many_to_one(appTree,reference,appTree,root);
init_rel_or_ptr(appTree,reference,relationMode::S,appTree,root,relationMode::DC);

static int char_is_alphanum(int ch)
{
  if((ch >= 'a') && (ch <= 'z')) return 1;
  if((ch >= 'A') && (ch <= 'Z')) return 1;
  if((ch >= '0') && (ch <= '9')) return 1;
  return (ch == '_');
}

inline int chars_are_separated(int ch1, int ch2)
{
  return char_is_alphanum(ch1) != char_is_alphanum(ch2);
}

// Note:  this function is identical to a static function of same name in smt_modify.C !!!!
static int char_is_separator(int ch)
{
  switch(ch){
    case ' ':
    case '\t':
    case '\n':
    case ';':
    case ',':
    case '?':
    case '~':
    case '}':
    case '{':
    case '(':
    case ')':
    case '[':
    case ']':
      return 1;

    default:
      ;
  }
  return 0;
}


static int token_is_separator(smtTree*tt, char * buf)
{
  if(! tt) return 1;

  switch(tt->extype) {
  
  case SMTT_el:
  case SMTT_lb:
  case SMTT_begin:
  case SMTT_end:
  case SMTT_grouping:
  case SMTT_string:
    return 1;

  case SMTT_op:
    if (tt->tlth > 1)
        return 1;
    else
        return char_is_separator(buf[tt->tbeg]);

  default:
    ;
 }
 return 0;
}

static int start_is_separator(smtTree*tt, char * buf)
{
  if(!tt) return 1;
  if(tt->spaces) return 1;
  if(char_is_separator(buf[tt->tbeg])) return 1;
  return token_is_separator(tt, buf);
}

static int end_is_separator(smtTree*tt, char * buf)
{
  if(!tt) return 1;
  if(tt->newlines) return 1;
  if(tt->tbeg + tt->tlth == 0) return 0;
  if(char_is_separator(buf[tt->tbeg + tt->tlth - 1])) return 1;
  return token_is_separator(tt, buf);
}

static int  tokens_are_separated(smtTree*t1, smtTree*t2, char * buf)
{
 if(! t1) return 1;
 if(! t2) return 1;

 if(t1->newlines || t2->spaces) return 1;

 if(end_is_separator(t1, buf) || start_is_separator(t2, buf)) return 1;

 if (t1->tbeg + t1->tlth == 0) return 0;
 char ch1 = buf[t1->tbeg + (t1->tlth-1)];
 char ch2 = buf[t2->tbeg];

 if(chars_are_separated(ch1, ch2)) return 1;
 return 0;
}


inline int string_is_empty(char * st, int len)
{
 return st ? (len<1 || (len==1 && st[0]==' ')) : 1;
}

// returns:   meat_len - length of the middle 'not white' part of the string
//                  st - amount of whites infront
//                  en - amount of whites after
//
//       if (st > 0) str[st - 1]    -- first meat char
//
static int map_whites (const char *buf, int len, int& st, int& en)
{
    Initialize(map_whites);
    st = en = 0; int meat_len = 0;
    if (!buf || !buf[0]) return meat_len;

    enum _States {
	_Init,
	_Whites,
	_Meat
    };

    int my_state = _Init; int cur_whites = 0; char c = 0;
    for ( int ii = 0 ; ii < len && (c = buf[ii]) ; ii ++ ) {
	switch (my_state) {
	case _Init:
	    if (isspace(c)) {
		cur_whites ++ ;
	    } else {
		if (!st) st = cur_whites;
		cur_whites  = 0;
		my_state    = _Meat;
	    }
	    break;
	case _Whites:
	    if (isspace(c)) {
		cur_whites ++ ;
	    } else {
		cur_whites = 0;
		my_state   = _Meat;
	    }
	    break;
	case _Meat:
	    if (isspace(c)) {
		cur_whites = 1; 
		my_state   = _Whites;
	    }
	    break;
	}
    }

    if (my_state = _Init)
	meat_len = 0;
    else {
	en = cur_whites;
	meat_len = len - st - en;
    }
    return meat_len;
}

smtTree * smtTree::replace_text (void const *data, int node_type,
					 int ref, int format)
{
    Initialize(smtTree::replace_text);

    smtTree * t = NULL, * t1 = NULL,  * tn, *prev;
    smtHeader * h = checked_cast(smtHeader,get_header());
    char * buffer = h->srcbuf;
    smtTree * new_this = this;

    char * dt = (char *) data;
    int new_data_len = strlen(dt);	
    int is_empty = string_is_empty(dt, new_data_len);

    int touch_dd = 1;

    if (format) {
	if ((!get_next () || !get_prev ())) {
	    if (!old_string) {
		old_string = new char[tlth + 1];
		strncpy (old_string, h->srcbuf + tbeg, tlth);
		old_string[tlth] = '\0';
	    }
	}
	int org_st, org_en, new_st, new_en; 
	char *org_dt = h->srcbuf + tbeg;

	int org_ln = map_whites (org_dt, tlth, org_st, org_en);       // returns meat_len
	int new_ln = map_whites (dt, new_data_len, new_st, new_en);

	if ( org_ln == new_ln ) {    // do not set modified if only white spaces changed
	    if (!org_ln || strncmp ((org_dt + org_st), (dt + new_st), org_ln) == 0)
		touch_dd = 0;
	}
    }
    
    char * new_dt = 0;
    if (!is_empty && !format) {
	if (dt[new_data_len-1]==' ') {
	    --new_data_len;
	    dt[new_data_len]='\0';
	}
	
	if (this->tbeg == -1) {   // new unformatted staff
	    prev = this->get_prev();
	    int sps = 0;
	    int newl = 0;
	    if(prev) {
		t = prev->get_first_leaf();
		sps = t->spaces;
		t1 = prev->get_last_leaf();
		newl = t1->newlines;
	    }

	    this->newlines = newl;
	    this->spaces = sps;
	} 
	
	if ((extype != SMTT_untok) && (spaces || newlines)) {
	    int new_length = new_data_len + spaces + newlines;
	    new_dt = dt = new char[new_length + 1];
      
	    int ii;
	    if (this->tbeg == -1) {
		if (t)
		    for(ii=0; ii<spaces; ++ii)
		    dt[ii] = buffer[t->start () + ii];
	    } else 
		for(ii=0; ii<spaces; ++ii)
		    dt[ii] = buffer[start () + ii];
	    
	    strncpy(dt+spaces, (char*) data, new_data_len);

	    for(ii=new_data_len+spaces; ii < new_length; ++ii)
		dt[ii] = '\n';
	    
	    dt[new_length] = '\0';
	    new_data_len = new_length;
	}

	if (extype == SMTT_untok && tbeg >= 0) { //boris: keep old white chars for untoks
	    int ii = 0, jj = 0, old_len = length();
	    char ch;
	    
	    new_dt = new char [new_data_len + old_len + 10]; // allocate more than enough
	    
	    // white chars before from old;	    
	    for ( jj = 0 ; jj < old_len && (ch = buffer[tbeg + jj]) ; jj++ ) {
		if (ch == ' ' || ch == '\t' || ch == '\n')
		    new_dt[ii++] = ch;
		else
		    break;
	    }
	    
            // body from new;
	    for ( jj = 0 ; jj < new_data_len ; jj++)
		new_dt[ii++] = dt[jj];

	    // white chars after from old;
	    for (jj = 1 ; (jj <= old_len) && (ch = buffer[tbeg + (old_len - jj)]) ; jj++) {
		if (ch == ' ' || ch == '\t' || ch == '\n')
		    ;
		else
		    break;
	    }
	    
	    if (jj < old_len)
		for (jj = old_len - jj + 1 ; jj < old_len && (ch = buffer[tbeg + jj]) ; jj++)
		    new_dt[ii++] = ch;
	    
	    new_dt[ii] = 0;
	    
	    dt = new_dt;
	    new_data_len = strlen(new_dt);
	}
    }				// ! format

//  int touch_fl =  (ref == 0 && *(unsigned char *)dt >= ' ');
    int touch_fl = 1;		// boris: I do not know is it right...
    smt_touch(this, touch_fl);	// Mark that View & AST should be regenerated
    ndm = 1;

    // delete all children if exist
    for(t = checked_cast(smtTree,this->get_first()); t; t = tn)
    {
	tn = checked_cast(smtTree,t->get_next());
	smt_remove(t);
    }

    if(node_type)
    {
	// Make new node with type = node_type and
	// create single token node under it
	type = extype = was_type = node_type;
	t = db_new(smtTree, (SMT_token));
	put_first(t);
    }
    else
	t = this;		// changed to token

    t->type = SMT_token;
    t->was_type = SMT_token;
  
    if(ref)
    {
	t->extype = SMTT_ref;
	t->tbeg = t->tlth = 0; 
	smt_tidy_level(checked_cast(smtTree,t->get_parent()));
	reference_put_root(t, checked_cast(appTree, (Relation *)data));
	put_relation(startnode_of_reference, t, (Relational *)data);
	put_relation(endnode_of_reference, t, (Relational *)data);
	new_this = t;
    } else {
	if(*dt == 0)		// If empty string - set one space -
	    dt = " ";		// it will be deleted propertly by tokenizer

	new_data_len = strlen(dt);

	if(is_empty)		// remove = replace with " "
	    if(new_data_len > 1)
		-- new_data_len;

	if(h->src_asize - h->src_size < new_data_len) // Expand buffer if necessary
	    h->src_exp_buffer(new_data_len - (h->src_asize - h->src_size) + 64); 
 
	int st = h->src_size;
	if(new_data_len)
	    strncpy(&h->srcbuf[st], dt, new_data_len);

	t->tbeg = h->src_size;
	t->tlth = new_data_len;
	t->spaces = 0;
	t->newlines = 0;
	t->extype = SMTT_untok; h->untok_flag = 1;
	h->src_size += new_data_len;

	if(new_dt)
	    delete new_dt;
    }

    if (touch_dd) smtHeader::set_touched_ddElements(t);

    return  new_this;
}

/***********/

appTreePtr smtTree::get_world () 
{
  Initialize(smtTree::get_world);
  
  // smtTree::get_world returns the uppest parent under root.
  
  smtTree *root = checked_cast(smtTree,this->get_root());
  if (this == root)
    return this;
  
  smtTree *n1, *n2;
  for (n1 = this; n1; n1=n2){
    ddElement*dd = checked_cast(ddElement, get_relation(def_dd_of_smt,n1));
    if (dd && (dd->get_kind() != DD_PARAM_DECL) )
	return n1;
    n2 = n1->get_parent();
    if (n2 == root)
      return n1;
  }
  return this;
}

int need_merge (smtHeader* h, smtTree* first, smtTree* second)
{
    return !tokens_are_separated (first, second, h->srcbuf);
}

