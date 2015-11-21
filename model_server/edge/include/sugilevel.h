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
#ifndef _SUGILEVEL_H
#define _SUGILEVEL_H

#include "matrix.h"

class levelitem;
class intersect;

class treeinfo {
 public:
  int abs_pos;           // absolute x coordinate;
  int rel_pos;           // x coordinate relative to father;
  int max_r_shift;       // maximum possible shift th the right;
  int nr_kids;           // number of sons;
  int half_x_width;      // half x width of node (border inclusive);
  int y_height;          // height of node (border inclusive);
  levelitem* first_son;  // pointer to the levelitem of the first successor
                         // of this node in th successorlist;
  treeinfo(int k, int x, int y, levelitem* fs) 
    { 
      abs_pos=0;  rel_pos=0; max_r_shift=0; 
      nr_kids=k; half_x_width=x; y_height=y; 
      first_son=fs;
    };
  ~treeinfo() {};
};

class levelitem {
  friend class level;
  node* p;
  level* item_level;
  levelitem* prev_item;
  levelitem* next_item;
  treeinfo* info;
public:
  levelitem(node*, level*, treeinfo*);
  ~levelitem();
  node* getnode() { return p; };
  level* getlevel() { return item_level; };
  levelitem* getprevitem() { return prev_item; };
  levelitem* getnextitem() { return next_item; };
  treeinfo* getinfo() { return info; };
  void print();
};

class level {
  graph* backgraph;
  int c_level;  			// level number
  int max_level;  			// max level number
  int c_items;  			// number of items at this level
  matrix* interconnection;  		// interconnection matrix between this 
					// level and next level
  matrix* row_connection;  		// connection matrix of nodes
                                        // on same level (horizontal)
  levelitem** items;                    // array of node pointers
  permutation* item_permutation;  	// gives the order of the items
  levelitem* first_sorted_item;  		// first sorted levelitem
  levelitem* first_item;  		// first levelitem
  levelitem* last_item;  		// last levelitem
  level* prev_level;    		// lower level number
  level* next_level;    		// higher level number
  edge_vector* upper_connectivity;    
  permutation* upper_priority;
  edge_vector* lower_connectivity;
  permutation* lower_priority;
  edge_vector* position;    			// center position of node
  edge_vector* half_x_width;    		// half horizontal size of node
  int y_height;    			// maximum vertical width
  edge_vector* depth;                        // z coordinates
  int stable;                           // true if all nodes on this level
                                        // are fixed by constraints
  intersect* headintersect;
  intersect* headconc;
 public:
  level(graph*);
  level(graph*, int);
  graph* getbackgraph()                 {return backgraph;}
  level* getnextlevel()			{return next_level;}
  level* getprevlevel()			{return prev_level;}
  level* getlevelwithnum(int i);
  edge_vector* getposition()			{return position;}
  edge_vector* getdepth()			{return depth;}
  int getclevel()			{return c_level;}
  int getcitems()			{return c_items;}
  levelitem* add_node(node* q);    	// insert q into right level
                                        // return inserted levelitem
  levelitem* replace_node(levelitem* p, node* q);    // replace levelitem p by node q
  void del_node(levelitem* li);    	// delete levelitem
  levelitem* add_node(node* q, treeinfo* ti); // insert q with treeinfo ti
                                        // into right level
  void create_ic_matrix();   		// creates interconnection matrix 
                            		// between this level and next level
                           		// must be done after last call of 
					// add_node
  void exchange_items(int,int);    	// exchanges the position of two items
                                  	// only item_permutation and
                                 	// interconnection matrix is changed
  void permute_items(permutation*);    	// permutes the position of items
                                      	// only item_permutation and 
                                     	// interconnection matrix is changed
  int reverse_rows(int);    		// reverse rows with equal barycenters
 		        		// returns true if rows are reordered
					// int = 1 means only count crossings
					// between this and next level down
					// int = 2 means only count crossings
					// between this and next level up&down
  int reverse_columns(int);    		// reverse columns with equal barycenters
		           		// returns true if columns are reordered
					// int = 1 means only count crossings
					// between this and next level down
					// int = 2 means only count crossings
					// between this and next level up&down
  int baryorder_rows();    		// barycentric ordering of rows of this
                          		// interconnection matrix
			 		// rows are only reordered if the 
					// number of crossings is decreased by 
					// this operation
		       			// returns true if rows are reordered
  int baryorder_rows_quick();    	// barycentric ordering of rows
                                	// without checking of crossings
  int baryorder_columns();    		// barycentric ordering of columns of 
					// this interconnection matrix
			    		// rows are only reordered if the 
					// number of crossings is decreased by 
					// this operation
		          		// returns true if columns are reordered
  int baryorder_columns_quick();    	// barycentric ordering of columns
                                   	// without checking of crossings
  int constr_rev_rows(int);             // same as reverse_rows but takes
                                        // also constraints into account
  int constr_rev_columns(int);          // same as reverse_columns but takes
                                        // also constraints into account
  int constr_order_rows();              // same as baryorder_rows but takes
                                        // also constraints into account
  int constr_order_columns();           // same as baryorder_columns but takes
                                        // also constraints into account
  int constr_order();                   // order nodes on this level according
                                        // to their constraints
  int constr_correction(permutation&);  // correct the permutation according
					// to the constraints
					// return 1 if permutation is changed
  void add_constraints();               // add constraints representing
                                        // the ordering of nodes          

  void init_finetuning();    		// computes upper and lower 
					// connectivities and priorities
                           		// of this level and all next levels
			  		// dummynodes are given the 
			 		// maximum connectivity + 10
                        		// inits horizontal positions of nodes
 		       			// must be done after barycentric 
					// ordering
  void adjust_down();    		// adjusts nodes regarding the 
					// barycenters of previous level
  void adjust_careful_down();    	// as adjust_down, but with careful 
					// shifting
  void adjust_up();    			// adjusts nodes regarding the 
					// barycenters of next level
  void adjust_careful_up();    		// as adjust_up, but with careful 
					// shifting
  void constr_init_finetuning();        //
  void constr_adjust_down();            //
  void constr_adjust_up();              // take constraints into account
  void constr_adjust_careful_down();    //
  void constr_adjust_careful_up();      //
  void constr_2halfD();                 //
  void constr_Z();                      //
  void setstable(int i) { stable = i; }
  int isstable() { return stable; }

  int shift(int,int,int,edge_vector*); 	// shifts the specified item if possible
                                    	// returns the real shift
  void last_adjust();    		// shifts all nodes to the left margin 
					// so that no negative positions exist
  void set_display();    		// set x, y coordinates of this level 
					// and all next levels for sugiyama 
                                        // layout
  void set_tree_display();              // set x, y coordinates of this level
                                        // and all next levels for treelayout
  levelitem* get_item(int p) {return items[(*item_permutation)[p]];}
                        		// returns item at the given position 
  int crossings(int =0);    	        // returns number of crossings of 
					// edges of this level and all 
					// following levels
  levelitem* getfirstitem()             { return first_item; };
  int getheight()                       { return y_height; };
  void print();    			// print this level an all next levels
                  			// with ordered items
  ~level();
  void remove_empty_levels();           // there might be levels without any
                                        // nodes in it: delete all leading
					// levels and insert dummies into 
                                        // the others
  intersect* getheadintersect()		{ return headintersect; }
  intersect* getheadconc()		{ return headconc; }
  void setheadintersect(intersect* i)	{ headintersect = i; }
  void setheadconc(intersect* i)	{ headconc = i; }
  levelitem* sortedlilist();
  levelitem* findlevelitem(int);
};

/*
   START-LOG-------------------------------------------

   $Log: sugilevel.h  $
   Revision 1.2 2000/07/10 23:02:49EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2.1.2  1992/10/09  19:03:33  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/
#endif
