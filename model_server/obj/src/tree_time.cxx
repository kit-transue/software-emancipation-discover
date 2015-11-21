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
#include <msg.h>
#include "objTree.h"
#include "genError.h"

class intTree : public objTree {
  public:
    int value;
    define_relational(intTree, objTree);
    intTree(int vv) : value(vv) {};
    virtual void  print(ostream& st = cout, int level = 0) const;
#ifdef __GNUG__
    intTree (const intTree& tt) { *this = tt; }
#endif
    copy_member(intTree);
    
};

generate_descriptor(intTree, objTree);

init_relational(intTree, objTree);

extern void gen_print_indent(ostream&, int);

void intTree::print(ostream& st, int lev) const {
  Initialize(intTree::print);

  gen_print_indent(st, lev);
  st << value;  

//  Return;  Removed to eliminate compiler warning.
}


static intTree* build_int_tree(int wid, int dep, int& val){
 intTree *curr=0, *prev=0;
 intTree *root = new intTree(val++);
 if(dep > 0){
   for(int ii=0; ii<wid; ++ii){
       curr = build_int_tree(wid, dep-1, val);
       if(ii==0) root->put_first(curr);
       else prev->put_after(curr);
       prev = curr;
   }
 }
 return(root); 
}

static int tree_delete(objTree* tt){
 objTree *curr, *next;
 int count = 1;
 for(curr = tt->get_first(); curr; curr = next){
     next = curr->get_next();
     count += tree_delete(curr);
 }
 delete tt;
 return(count); 
}

define_relation(one_to_many, many_to_one);
init_relation(one_to_many, 1, 0, many_to_one, MANY, 0);

extern "C" int gettime();

extern "C" void tree_time(int nn){
  int no_iter = nn / 2;
  int ii;
  double start_time, end_time;
  double secs;

  intTree *t1, *t2, *t3;
  int val = 1;

  t1 = build_int_tree(2, 2, val);

  t2 = (intTree *)t1->get_first();
  t3 = (intTree *)t2->get_next();

  start_time = gettime();
  for(ii=0; ii<no_iter; ++ii){
    t2->put_first(t3);
    t1->put_first(t3);
  }
  end_time = gettime();
  secs = (end_time - start_time)/ 1000000.0;

  msg("time of $1 put_first $2") << nn << eoarg << secs << eom;
  
  start_time = gettime();
  for(ii=0; ii<no_iter; ++ii){
    t2->get_first();
    t1->get_next();
  }
  end_time = gettime();
  secs = (end_time - start_time)/ 1000000.0;

  msg("time of $1 get_first $2") << nn << eoarg << secs << eom;
  
  start_time = gettime();
  for(ii=0; ii<no_iter; ++ii){
    put_relation(one_to_many, t1, t2);
    rem_relation(one_to_many, t1, 0);
  }
  end_time = gettime();
  secs = (end_time - start_time)/ 1000000.0;

  msg("time of $1 put__rem__relation $2") << nn << eoarg << secs << eom;
  
delete t1;


  int depth = 2;
  int width = 10;
  while( (width * width) < nn) width *= 2;   // sqrt
  val = 1;

  start_time = gettime();
  t1 = build_int_tree(width, depth, val);
  end_time = gettime();
  secs = (end_time - start_time)/ 1000000.0;

  msg("time build $1 by $2  objTree $3") << width << eoarg << depth << eoarg << secs << eom;

  start_time = gettime();
  obj_delete (t1);
  end_time = gettime();
  secs = (end_time - start_time)/ 1000000.0;

  msg("time obj_delete $1 by $2  objTree $3") << width << eoarg << depth << eoarg << secs << eom;

  t1 = build_int_tree(width, depth, val);
  start_time = gettime();
  int count = tree_delete(t1);
  end_time = gettime();
  secs = (end_time - start_time)/ 1000000.0;

  msg("time delete $1 by $2 = $3 nodes of objTree $4") << width << eoarg << depth << eoarg << count << eoarg << secs << eom;

}
/*
   START-LOG-------------------------------------------

   $Log: tree_time.cxx  $
   Revision 1.4 2000/07/12 18:11:06EDT ktrans 
   merge from stream_message branch
Revision 1.2.1.2  1992/10/09  18:55:41  boris
Fix comments


   END-LOG---------------------------------------------

*/
