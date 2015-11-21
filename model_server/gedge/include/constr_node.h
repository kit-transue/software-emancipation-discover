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
// S. Spivakovsky - 08/07/92 - corrected equal_info destructor

#ifndef _CONSTR_NODE_H
#define _CONSTR_NODE_H

class constr_node_list : public slist{
 public:
  constr_node_list() {}
  constr_node_list(class constr_node* a):slist(a) {}
  ~constr_node_list() {}
  void insert(constr_node* a) {slist::insert(a);}
  void insertlist(constr_node_list* ll) {slist::insertlist((slist*) ll);}
  void append(constr_node* a){slist::append(a);}
  void appendlist(constr_node_list* ll) {slist::appendlist((slist*) ll);}
  int find(constr_node* a) {return slist::find(a);}
  int del(constr_node* a) {return slist::del(a);}
  constr_node* get() {return (constr_node*) slist::get();}
  constr_node* first() {return (constr_node*) slist::first();}
  constr_node* second() {return (constr_node*) slist::second();}
  void clear() {slist::clear();}
};

class environment_info {
  // the information kept in this class is common to all nodes which have
  // fixed relative positions, i.e. that are linked by imm_pred, imm_succ, 
  // or equal links;
  friend class constr_node;
  friend class null_dummy;
  friend class equal_info;
  int size;                    // length of this environment;
  interval range;              // range of positions where the first element
                               // of this environment lies;
 public:
  environment_info() : range()       
    { size = 0; }
  environment_info(int h, int l) : range(l,h)
    { size = 0; }
  ~environment_info() {}
};

class equal_info {
  // the information kept in this class is common to all nodes which are
  // known to have exactly the same position;
  friend class constr_node;
  friend class null_dummy;
  set* nr;                     // number of node as bit vector;
  set* preds;                  // set of node numbers which must be
                               // predecessors of this node;
  constr_node_list succs;      // list of successors that must stand right
                               // to this node;
  int in_degree;               // number of incoming edges (order relations);
  int touched;                 // number of incoming edges already evaluated;
  equal_info* prev;            // left neighbor position;
  equal_info* next;            // right neighbor position;
  environment_info* environment;  // set of nodes that are in a fixed relative
                               // position to this node;
  int offset;                  // distance to the first node in 
                               // corresponding environment;
  constr_node_list equals;     // list of all nodes equal to this;
  int v;
  int v_set;
 public:

  equal_info() : succs(), equals()
    {  
      nr = 0; preds = 0; in_degree = 0; touched = 0; prev = 0; next = 0;
      environment = 0; offset = -1; v = 0; v_set = 0;
    }

  ~equal_info();

  void make_environment();
                               // create a new environment for a linked list
                               // of neighbor positions starting at the first
			       // position;
  interval* range()            // return the range of this position;
  { return new interval(environment->range.low + offset,
			environment->range.high + offset); }
  void set_value(int i) { v=i; v_set=1; }
  int  get_value(int& i) { i=v; return v_set; }
  void reset_value() { v_set=0; }
};

class neighbor_info {
  // the information kept in this class is common to all nodes which are
  // in the same neighborhood relationship, i.e. that are linked by 
  // imm_pred/succ links;
  friend class constr_node;
  friend class null_dummy;
  constr_node* head;           // head of neighbors;
  constr_node* tail;           // tail of neighbors;
  constr_node_list strong_succs;  // list of successors that must stand right
                               // to this neighborhood;
  int strong_in_degree;        // number of incoming strong edges (strong
                               // order relations);
  int touched;                 // number of incoming strong  edges already 
                               // evaluated;
 public:
  neighbor_info() : strong_succs()
    { head = 0; tail = 0; strong_in_degree = 0; touched = 0; }
  ~neighbor_info() { }
  void make();                 // compute the strong_succs list and the
                               // strong_in_degree of this neighbor_info
                               // be aware of strong successors that are
			       // in this neighbor_info;
};

class constr_node {
protected:
  friend class neighbor_info;
  friend class null_dummy;
  friend class constr_box;
  class node* owner;           // owner of constraint node;
  constr_box* cluster;         // enclosing cluster;
  constr_node* imm_pred;       // constraint node which is known explicitely
                               // to be the immediate predecessor of this node;
  constr_node* imm_succ;       // constraint node which is known explicitely
                               // to be the immediate successor of this node;
  interval limit;              // interval that includes this node;
  constr_node_list equals;     // list of nodes which have same position;
  constr_node_list strong_succs;  // list of successors that must stand right
                               // to this node and his immediate successors;
  constr_node_list succs;      // list of successors that must stand right
                               // to this node;
  constr_node_list strong_preds;  // list of predecessors that must stand left
                               // to this node and his immediate predecessors;
  constr_node_list preds;      // list of predecessors that must stand left
                               // to this node;
  int leftmargin;              // true if node must stand at the left margin;
  int rightmargin;             // true if node must stand at the right margin;
  neighbor_info* neighborhood; // information for a class of neighbor nodes;
  equal_info* position;        // information for a class of equal nodes,
                               // they have the same position in the 
                               // constraint net;
  int in_degree;               // number of incoming edges (order relations);
  int strong_in_degree;        // number of incoming strong edges (strong
                               // order relations);
  int own_position;            // flag to indicate if thevariable position is 
                               //owned by the class. (02-02-94)
 public:
  constr_node(class constr_net* n, node* o);
  virtual ~constr_node();
  class constr_net* net;       // global constraint network;
  constr_node* next;           // global constraint node list;

  // functions to evaluate the network;
  void make_neighborhood();    // construct the neighbor_info for a
                               // class of nodes;
  int make_positions(equal_info* p = 0, equal_info* l = 0, equal_info* r = 0);
                               // construct the equal_info, then 
                               // recursively visit other
			       // nodes, return number of nodes that are
			       // economized by making classes;
  void make_environment();     // create environment_info and compute offset
                               // of nodes inside the environment;
  virtual int order_ranges_forw(int fixed);
                               // restricts the lower limit of the successor
			       // nodes to be one unit larger than this lower
			       // limit,
			       // returns 0 if any range becomes empty,
			       // 1 otherwise;
  virtual int order_ranges_backw(int fixed);
                               // restricts the upper limit of this node
			       // to be one unit smaller than the minimum
			       // upper limit of its successors,
                               // returns 0 if any range becomes empty,
                               // 1 otherwise;

  virtual int delta()          {return 1;};
  void touch(set*);            // visit this node during evaluation of the
                               // constraint network,
                               // return 1 if consistent, 0 otherwise;
  int is_evaluated();          // true if the evaluation of the constraint
                               // network reached this node correctly;
  void clear();                // undo computations before new evaluation
                               // of the network;

  // functions on constraints,
  // usually 3 possibilities:
  // - DO    : introduce new constraint into the network,
  //           return 1 if successful, 0 otherwise,
  // - UNDO  : remove constraint from network,
  //           return 1 if constraint was removed or did not exist, 
  //           0 if constraint could not be removed,
  // - QUERY : ask if constraint is satisfied,
  //           return 1 if true, 0 otherwise;
  // the comments below always describe the 'DO' operation only;

  virtual int strong_order(constr_node* a, mode m = DO);  
                               // adds constraint node a to the strong_succs 
                               // list of this node;
  virtual int order(constr_node* a, mode m = DO);
                               // adds constraint node a to the succs list
                               // of this node;
  virtual int equal(constr_node* a, mode m = DO);
                               // adds constraint node a to the equal list
                               // of this node;
  virtual int left_neighbor(constr_node* a, mode m = DO);
                               // makes left neighbor to node a,
  virtual int left_margin(mode m = DO);
                               // marks this node to be at the left margin;
  virtual int right_margin(mode m = DO);
                               // marks this node to be at the right margin;
  virtual int range(interval& i, mode m = DO);
                               // set range of possible positions to 
                               // interval i;
  virtual int bound(int& s, mode m = DO);
                               // node must has maximum size s;
  // other queries;
  virtual int is_constr_dummy(){return 0;}
  virtual int is_constr_box()  {return 0;}
  constr_node *node2box(constr_node_list* nl); 
                               // change this node to a constraint box;
  virtual int is_upper_bound();
                               // true if all nodes with smaller numbers
                               // are predecessors;
  virtual interval* range()    // return range of possible positions;
  { 
    if (position) return position->range();
    else return new interval(&limit);
  }

                               // access to attribute `value';
  void set_value(int i) { position->set_value(i); }
  int  get_value(int& i) { return position->get_value(i); }
  void reset_value() { position->reset_value(); }

constr_box* get_cluster() { return cluster; }  // access to enclosing cluster;

};


#endif
