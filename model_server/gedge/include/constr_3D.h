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
#ifndef _CONSTR_3D_H
#define _CONSTR_3D_H

class constr_3D {
  ///////////////////////////////////////////////////////////////////////////
  // class to manage 3-dimensional constraints between objects of type node;
  // constraints on the absolute and relative position of nodes are posiible;
  // for each dimension (X,Y,Z) the constraints are managed independently;

  graph* backgraph;           // graph for which constraints are defined
  constr_manager manager[3];  // 3 separate constraint managers for
                              // dimensions (X,Y,Z)

 public:

  constr_3D(graph* g);        // constructor
  ~constr_3D();               // destructor

  ////////////////////////////////////////////////////////////////////////////
  // functions to manipulate constraints:
  ////////////////////////////////////////////////////////////////////////////
  // each function has a parameter 'mode' which might have one of the three
  // values: 'DO':    define a new constraint, i.e. add this constraint into
  //                  the constraint network;
  //         'UNDO':  delete this constraint from the constraint network;
  //         'QUERY': ask if this constraint exists;
  ////////////////////////////////////////////////////////////////////////////
  // each function has a parameter 'priority' which might be any integer value;
  // in case of inconsistencies among constraints, constraints with lower
  // priorities are inactivated until a consistent solution is reached;
  ////////////////////////////////////////////////////////////////////////////
  // each function returns 1 if operation was successful (mode = DO / UNDO),
  //                         if constraint holds (mode QUERY),
  //                       0 if operation failed (mode = DO / UNDO),
  //                         if constraint does not hold;
  ////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  // functions;
  ////////////////////////////////////////////////////////////////////////////
  int consistency();
  // evaluate constraint networks and find a consistent solution according
  // to the priorities of the constraints;

  ////////////////////////////////////////////////////////////////////////////
  // elementary constraint functions:

  int equal      
    (constr_type t, dimension d, node* n1, node* n2, mode m, int priority = 0);
  // d-coordinate of nodes n1 and n2 are equal;

  int smaller    
    (constr_type t, dimension d, node* n1, node* n2, mode m, int priority = 0);
  // d-coordinate of node n1 is smaller than that of node n2;

  int greater    
    (constr_type t, dimension d, node* n1, node* n2, mode m, int priority = 0);
  // d-coordinate of node n1 is greater than that of node n2;

  int neighbors 
    (constr_type t, dimension d, node* n1, node* n2, mode m, int priority = 0);
  // d-coordinate of node n1 is by one smaller than that of node n2;

  int low_margin 
    (constr_type t, dimension d, node* n1, mode m, int priority = 0);
  // node n1 has the smallest d-coordinate of all nodes;

  int high_margin
    (constr_type t, dimension d, node* n1, mode m, int priority = 0);
  // node n1 has the greatest d-coordinate of all nodes;

  int range      
    (constr_type t, dimension d, node* n1, interval& i, mode m, int priority =0);
  // d-coordinate of node n1 lies in the interval i;

  int cluster
    (constr_type t, dimension d, node* n1, nlist* nl, mode m, int priority=0);
  // make node n1 a cluster node for all nodes in nodelist nl;

  int limit   
    (constr_type t, dimension d, node* n1, int& distance, mode m, int priority = 0);
  // size of node n1 in dimension d must not exceed distance (only makes sense
  // for clusters); 

  ////////////////////////////////////////////////////////////////////////////
  // higher level constraint functions;

  int above   (constr_type t, nlist* nl, mode m, int priority = 0);
  int below   (constr_type t, nlist* nl, mode m, int priority = 0);
  int left    (constr_type t, nlist* nl, mode m, int priority = 0);
  int right   (constr_type t, nlist* nl, mode m, int priority = 0);
  int in_front(constr_type t, nlist* nl, mode m, int priority = 0);
  int behind  (constr_type t, nlist* nl, mode m, int priority = 0);

  int equal_line    
    (constr_type t, nlist* nl, mode m, int priority = 0);
  int equal_column  
    (constr_type t, nlist* nl, mode m, int priority = 0);
  int equal_position
    (constr_type t, nlist* nl, mode m, int priority = 0);

  int top_margin    (constr_type t, nlist* nl, mode m, int priority = 0);
  int bottom_margin (constr_type t, nlist* nl, mode m, int priority = 0);
  int left_margin   (constr_type t, nlist* nl, mode m, int priority = 0);
  int right_margin  (constr_type t, nlist* nl, mode m, int priority = 0);

  int upper_neighbor
    (constr_type t, nlist* nl, mode m, int priority = 0);
  int lower_neighbor
    (constr_type t, nlist* nl, mode m, int priority = 0);
  int left_neighbor 
    (constr_type t, nlist* nl, mode m, int priority = 0);
  int right_neighbor
    (constr_type t, nlist* nl, mode m, int priority = 0);

  ////////////////////////////////////////////////////////////////////////////
  // other functions:


  // useful for the graph representation language;

  void delete_constraints(dimension d, constr_type t, node* n = 0);
  // if n != 0
  // then delete all constraints on node n of constraint type t and dimension d,
  // else delete all constraints of constraint type t and dimension d;

  void delete_constraints(constr_type t, node* n = 0);
  // if n != 0
  // then delete all constraints on node n of constraint type t,
  // else delete all constraints of constraint type t;

  void output(ostream &to, int indent);
  // print constraints in GRL format 
  // to output stream 'to' 
  // with left margin indentation 'indent';

};


#endif
