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
#ifndef _CONSTR_BOX_H
#define _CONSTR_BOX_H

class constr_dummy : public constr_node {
  // constraint node which is not linked to an external node, but is created
  // by the evaluation algorithm, it has the same properties as a normal
  // constraint node;
 public:
  constr_dummy(constr_net* n) : constr_node(n, 0) {};
  ~constr_dummy() {};
  int is_constr_dummy() {return 1;}
};

class null_dummy : public constr_dummy {
  // constraint dummy node that has size zero;
 public:
  null_dummy(constr_net* n) : constr_dummy(n) {};
  ~null_dummy() {};
  int delta() {return 0;};
  int bound(int& s, mode m = DO);
  int range(interval& i, mode m = DO) {return constr_dummy::range(i,m);};
  interval* range() {return constr_dummy::range();};
};

class constr_box : public constr_node {
  // 'cluster' of several constraint nodes, they are posed to lie
  // between the left and the right constraint dummy node. Left and right
  // may have a fixed distance (size). If the size == 0, all nodes are
  // equal to the dummy 'middle';
  friend class constr_node;
  constr_node_list contents;
  int size;
  null_dummy* left;
  constr_dummy* middle;
  null_dummy* right;
 public:
  constr_box(constr_net* n, constr_node_list* l, node* o);
  ~constr_box();

  int increase(constr_node_list* nl);
  int decrease(constr_node_list* nl);
  int includes(constr_node_list* nl);

  int strong_order(constr_node*, mode) {sorry; return 0;}
  int order(constr_node* a, mode m = DO);
  int equal(constr_node* a, mode m = DO);
  int left_neighbor(constr_node* a, mode m = DO);
  int left_margin(mode m = DO);
  int right_margin(mode m = DO);
  int range(interval& i, mode m = DO);
  int bound(int& s, mode m = DO);

  int is_constr_box() { return 1;}
  int is_upper_bound() {sorry; return 0;}
  interval* range();
};

/*
   START-LOG-------------------------------------------

   $Log: constr_box.h  $
   Revision 1.1 1993/07/28 19:45:43EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  19:03:18  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
