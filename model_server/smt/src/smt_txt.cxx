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
// File smt_txt.C - SMODE - get text representation methods
//
 
#include <msg.h>
#include "smt.h"
#include "representation.h" 
#ifndef _objArr_h
#include <objArr.h>
#endif

#include <cLibraryFunctions.h>

objTree * get_subtree_next_leaf(objTree *, objTree *);
 
// Open SMT for reading source text with given mode
// it doesn't give: 1. Comments (1 newline instead of)
//		    2. Bacslash and newline combinations
static int newl; 
static int done_spaces;
static int beg_of_line;
static int put_newlines;
static int newline_in_string;

void smtHeader::get_init(smtTree * subroot)
  {
   Initialize(smtHeader::get_init);

  if (subroot == (smtTree*)-1)
      return;
  get_subtree = subroot ? subroot : checked_cast(smtTree,get_root());
  get_node = get_subtree->get_first_leaf();
  get_count = 0;
  newl = 0;
  done_spaces = 0;
  beg_of_line = 1;
  put_newlines = 1;
  }
 
 
// Read one character
char smtHeader::get_char()
{
//    Initialize (smtHeader::get_char);
    int c;
    smtTree  /* * comm, */  *next_node;
    /* Read from Source Mode Tree */

    for(c = 0; get_node != 0; c = 0) {

	if (newline_in_string == 1) {
	    newline_in_string = 0;
	    c = 'n';
	    break;
	} 
	
	if (beg_of_line) {
	    put_newlines = 1;
	    for (next_node = get_node;
		 next_node;
		 next_node = next_node->get_next_leaf(get_subtree)) {
		if (next_node->extype == SMTT_macro &&
		    !(next_node->tlth == 7 &&
		      (memcmp(srcbuf + next_node->tbeg, "VARCHAR", 7) == 0 ||
		       memcmp(srcbuf + next_node->tbeg, "varchar", 7) == 0)))
		    put_newlines = 0;
		if (next_node->newlines == 0)
		    newl++;
		 else
		    break;
		}
	    if (put_newlines)
		newl = 0;
	    beg_of_line = 0;
	}

	if (newl) {
	    newl--;
	    return '\n';
	}

	/* Skip comment */
        if (get_count == 0 &&
	    (get_node->extype == SMTT_comment ||
	     get_node->extype == SMTT_commentl))
	    get_count = get_node->tlth;
	
        /* Go to next leaf if this one isn't a token or has allready exhausted */
        if (get_node->type != SMT_token || get_count > get_node->tlth) {
	    if (get_node->newlines)
		beg_of_line = 1;
            get_node = get_node->get_next_leaf(get_subtree);
	    get_count = 0;
	    done_spaces = 0;
	    continue;
        }
 
	/* Output newline character if token exhausted just now */
	if (get_count == get_node->tlth) {
	    get_count++;
	    if (put_newlines || get_node->newlines)
		c = '\n';
	    else 
		c = ' ';
	    break;
	}
 
	if (get_count == 0 && done_spaces == 0 && get_node->spaces) {
	    c = ' ';
	    done_spaces = 1;
	    break;
	}

	/* Read character from buffer */
        c = srcbuf[get_node->tbeg + get_count++];
 
        /* Skip backslash + newline combination */
	if (c == '\\') 
	  {
	    int pos = get_count;
	    while (srcbuf[get_node->tbeg + pos] == '\r')
	      pos++;
	    if (pos < get_node->tlth)
	      if (srcbuf[get_node->tbeg + pos] == '\n') 
		{
		  get_count = pos+1;
		  continue;
		}
	  }

	/* Patch for multiple-line comments inside macro */
	if (c == '\n') 
	    if (get_node->extype == SMTT_macro) 
		c = ' ';
	    else if (get_node->extype == SMTT_string) {
		c = '\\';
		newline_in_string = 1;
	    }
	break;
	
    }
    return c;
}
 
// Output text value of the node to stream
static void pnode(const smtTree *t, ostream& s)
  {
   Initialize(pnode);

  if(t->type == SMT_token && t->tlth != 0 && t->extype != SMTT_el)
    {
    char * txt = new char[t->tlth+1];
    smtHeader * h = checked_cast(smtHeader,t->get_header());
    if(h)
      {
      strncpy(txt, &(h->srcbuf[t->tbeg]),t->tlth);
      txt[t->tlth] = 0;
      s << txt;
      }
    else msg("SMT: invalid tree - header not found") << eom;
    delete txt;
    }
  }
 
void smtTree::put_signature(ostream& s) const
{
  if(type != SMT_token){
    s << smt_token_name(type)+4 << '.' << tnfirst << '-' << tnmax;
  } else {
    s << smt_token_name(extype)+5 << '.' << tnfirst << '.';
    pnode(this, s); 
  }

}
// Print node (for debugging purposes)
void smtTree::print(ostream& s, int ) const
  {
  s << "avmn "<< (boolean)arn << (boolean) vrn << (boolean) ndm << 
	(boolean)is_macro ;
   
  if(ref_ast)
     s << ' ' << ref_ast;
 
  s <<" tp " << (int)type << "/" << (int)extype <<
       ' ' << smt_token_name(type);
  if(type != extype)
       s << "/" << smt_token_name(extype);
 
  s <<  " tn=(" << tnfirst << "," << tnmax << ")[" << tbeg << "," <<
       tlth << "]{" << (unsigned)newlines << "," << (unsigned)spaces << "}: `";
  pnode(this, s);
  s << "'";
  }
 
// Print header (for debugging purposes)
void smtHeader::print(ostream& s, int ) const
  {
   Initialize(smtHeader::print);
  smtHeaderPtr This = checked_cast(smtHeader,this);  // get_filename() isn't const, sigh...
  s << "avsdtf " << (boolean)arn << (boolean)vrn << (boolean)sam << 
	(boolean)drn << 
	(boolean)tok << (boolean)foreign
             << " file \"" << This->get_filename() << "\"";
  }
 
void smtTree::send_string (ostream& s) const 
{
    Initialize(smtTree::send_string);

    switch (type) {
      case SMT_token:
	{
	smtHeader * h = checked_cast(smtHeader, get_header());
 
	if(!h)
	    return;
 
	switch (extype) {
	  case SMTT_el:
	    break;

	  case SMTT_lb:
	    s << '\n';
	    break;
 
	  case SMTT_untok:
	    {
	    for (int i = 0; i < tlth; i++) 
	      {
		char c = h->srcbuf[tbeg + i];
		if (c > ' ')
		  s << c;
	      }
	    s << ' ';
	    }
	    break;
 
	  default:
	    if (tlth > 0) {
		char * txt = new char[tlth + 1];
		strncpy(txt, &(h->srcbuf[tbeg]), tlth);
		txt[tlth] = 0;
		s << txt;
		delete txt;
            }
	    s << ' ';
        }
	}
	break;

      default:
	{
	objTree* t = (objTree*)this;
	for (objTree * p = t->get_first (); p; p = p->get_next())
	    p->send_string (s);
	}
	break;
    }
}
 
static void colltok(smtTree * t, objArr * a)
  {
   Initialize(colltok);

  if(t->type == SMT_token)
    a->insert_last(t);
  else
    for(smtTree * w = t->get_first();
        w;
        w = w->get_next())
      colltok(w, a);
  return;
  }
     
 
objArr * smtTree::get_tok_list(int )
  {
  Initialize(smtTree::get_tok_list);
  objArr * a = new objArr;
  colltok(this, a);
  ReturnValue(a);
  }
 
//objTree * ___get_subtree_next_leaf(objTree *root, objTree *node)
//  {
//  objTree * t, * w;
//  for(t = node; t; t = w)
//    {
//    w = t->get_first();
//    if(w)
//      continue;
//    if(t != node)
//      break;
//    for(; t != 0 && t != root; t = t->get_parent())
//      {
//      w = t->get_next();
//      if(w)
//        break;
//      }
//    if(t == root)
//      return 0;
//    }
//  return t;
//  }
