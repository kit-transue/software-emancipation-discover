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
#ifndef _CONSTR_NET_H
#define _CONSTR_NET_H

class constr_net {
  friend class constr_node;
  null_dummy* neginf;               // smaller than any other node;
  null_dummy* posinf;               // bigger than any other node;
  null_dummy* leftmargin;           // smallest node;
  null_dummy* rightmargin;          // biggest node;
  constr_node* first_constr;        // header of list of all constraint nodes;
  int size;                         // number of nodes;
  int saved_size;                   // number of nodes saved by 'unification';
  interval limit;                   // interval that contains all nodes;
  int consistent;                   // true if network is consistent;
  constr_node_list linear_order;    // list of the constraint nodes ordered
                                    // in a way that all order constraints
                                    // are met;
  set_variables* var;               // generator for set variables;
  int debug;                        // if true print debugging information
                                    // while evaluating the network;
  dimension dim;                      // holds the dimension (X,Y,Z) of the
                                    // network;

  // functions to evaluate the network;
  void clear();                     // UNDO former computations before
                                    // evaluating network;
  int eval();                       // after completion of constraint input,
                                    // the node numbers and the order relations
                                    // are computed, 
				    // returns 1 if network is consistent,
                                    // 0 otherwise;
  set* give_var();                  // generate new set variable;
  
 public:
  constr_net();                     // create a new network;
  ~constr_net();                    // remove network;
  void set_dimension(dimension d);  // initialise dimension;
  
  constr_dummy* add_dummy();        // add a constraint node with no 
                                    // corresponding node;
  null_dummy* add_null_dummy();     // add a dummy constraint node with size 0;

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
  
  int exist(constr_node** a, node* owner, mode m = DO);  
                                    // add a node with no constraints;
  int strong_order(constr_node* a, constr_node* b, mode m = DO); 
                                    // add the constraint "a and all
                                    // its successors < b and all its
                                    // predecessors" to the network;
  int order(constr_node* a, constr_node* b, mode m = DO);
                                    // add the constraint "a < b" to the
                                    // network;
  int equal(constr_node* a, constr_node* b, mode m = DO);
                                    // add the constraint "a = b" to the
                                    // network;
  int neighbors(constr_node* a, constr_node* b, mode m = DO);
                                    // add the constraint "a+1 = b" to the
                                    // network; 
  int left_margin(constr_node* a, mode m = DO);
                                    // add the constraint "a at left margin" 
                                    // to the network;
  int right_margin(constr_node* a, mode m = DO);
                                    // add the constraint "a at right margin"
                                    // to the network;
  int box(constr_node* n, constr_node_list* nl, mode m = DO);
                                    // make node n an 'abstraction' node for
				    // all nodes in nl; 
  int bound(constr_node* n, int& distance, mode m = DO);
                                    // add the constraint "all nodes 
                                    // corresponding to abstraction node n
                                    // are situated in an area with a maximum
                                    // distance;
  int consistency(mode m = DO);     // make the network consistent;
  int range(constr_node* a, interval& i, mode m = DO);
                                    // set range of possible positions
                                    // for node a to i;
  int range(interval& i, mode m = DO);
                                    // set range of network to i;
  int order_ranges();               // compute the maximum range for all nodes,
                                    // return 0 if any range becomes empty,
                                    // 1 otherwise;

  void output();                    // print constraints in IDL format;
};

#endif
