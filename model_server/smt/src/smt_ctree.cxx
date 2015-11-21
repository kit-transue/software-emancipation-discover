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
// File smt_ctree.C 
//
#include "objTree.h"
#include "cLibraryFunctions.h"
#include <msg.h>
#include "smt.h"
#include <psetmem.h>

static int setninf(smtHeader *, smtTree *, int, int);
static int tidy_tree_1(int, smtTree *, char *, smtHeader *);

void smtHeader :: tidy_tree()
  {
   Initialize(smtHeader :: tidy_tree);

  if (src_size == 0) 
      return;

  int i;
  char * newbuf = (char*) psetmalloc(src_size * 2);
  smtTree * root = checked_cast(smtTree,get_root());
  i = tidy_tree_1(0, root, newbuf, this);
  psetfree(srcbuf);
  srcbuf = newbuf;
  src_asize = src_size * 2;
  src_size = i;
  setninf(this, root, 0, 1);
  i = root->get_last_leaf()->tnfirst;// Last token number
  for(; i < ttablth; i++)
    ttable[i] = 0;
  enum_src ();
  }

static int tidy_tree_1(int a, smtTree * t, char * newbuf, smtHeader *h)
{
    smtTree * w;
    if (t->type == SMT_token && t->extype != SMTT_ref) {
        strncpy (newbuf + a, h->srcbuf + t->start (), t->length ());
        t->tbeg = a + t->spaces;
        a += t->length ();
	if (t->extype == SMTT_untok)
	    h->untok_flag = 1;
    } else {
        t->tbeg = 0;
        t->tlth = 0;
    }

    for (w = t->get_first();
         w;
         w = w->get_next())
        a = tidy_tree_1(a, w, newbuf, h);

    return a;
}

static int setninf(smtHeader * h, smtTree *t, int a, int tn)
{
    int l = 0;
    t->tnfirst = tn;
    t->tnmax = 0;
    if (t->type == SMT_token) {
        if( t->start () != a && t->extype != SMTT_ref)
	    msg("Invalid text address in SMT tree") << eom;
        if (t->extype != SMTT_ref) {
//	    t->tbeg = a;
            l = t->length ();
        }
        t->tnmax = tn;
        h->set_ttable(t);
    } else {
        smtTree* prev = NULL;
        for (smtTree* w = t->get_first ();
             w;
             prev = w, w = w->get_next()) {
            int i = setninf (h, w, a, tn);
            if (w->tnmax > 0) {
                tn = w->tnmax + 1;
                t->tnmax = w->tnmax;
            }
            if (!prev) {
                t->tbeg = w->istart ();
                t->spaces = w->spaces;
            }
            a += i;
            l += i;
        }
	if (prev) {
	    t->newlines = prev->newlines;
	    t->tlth = l - t->spaces - t->newlines;
	}
    }
    return l;
}
