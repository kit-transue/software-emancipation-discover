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

#ifndef _GRAPH_H
#define _GRAPH_H

/*	class definition for graph and associated member functions
 */
#include "defaultparser.h"
#include "globdef.h"


#define MAXGRAPHFUNCTIONS   2
typedef int (*PFI)(...);
typedef graph * (*PFG)(...);

typedef enum {high = TOPSORT_TOP, low = TOPSORT_BOTTOM} topsorts;
typedef enum {left_to_right = LEFT_TO_RIGHT, right_to_left = RIGHT_TO_LEFT, 
              bottom_to_top = BOTTOM_TO_TOP, top_to_bottom = TOP_TO_BOTTOM} orientations;
typedef enum {barycenter = LAYOUT_SUGI, tree = LAYOUT_TREE, isi = LAYOUT_ISI,
              constraints = LAYOUT_SUGICONSTR} layout_algorithms;
typedef enum {every = LAYOUT_EVERY, manual = LAYOUT_USER} layoutfrequencies;
typedef enum {no = NO, yes = YES} yes_or_no;


class graph {
    int x;                              // x position of edge window
    int y;                              // y position of edge window
    int xmax;                           // x size of display window
    int ymax;                           // y size of display window
    int xbase;				// x base offset of graph within window
    int ybase;				// y base offset of graph within window
    int xspace;				// x spacing between nodes
    int yspace;				// y spacing between nodes
    int zspace;				// z spacing between nodes
    int width;				// width of graph (or abstr. subgraph)
    int height;                         // height of graph (or abstr. subgraph)
    topsorts topsort;			// integer determining topsort method
					// i.e. when flexible, push nodes up/down
    orientations orientation;		// integer determining orientation
					// i.e. top-to-bottom, left-to-right
    layout_algorithms layoutalgorithm;  // integer determining layout type
    layoutfrequencies layoutfrequency;	// integer determining layout frequency
    int layoutparameter [8];		// parameters for sugiyama (etc.) layout

    int proper;				// proper-status
    float scaling;                      // scaling factor
					// The following items are not used in abstractions
    node *lastnode;			// last selected node
    node *nexttolastnode;		// prelast selected node
    nlist *selectednodes;		// list of selected nodes;
     
    char* type_name;
    class level* headlevel;		// pointer to head level structure 
					// for sugiyama (& friends) layout
    					// this provides access to a data struct
					// that has a separate list of nodes for
					// each level of the graph (see sugilevel.h)
    int dirty;				// ``dirty'' bit. True when graph
					// structure has been modified since
					// last layout and redisplay
    int maxlevel;
 
// graph structure related items
    node* headnode;			// pointer to head of list of all nodes
    node* tailnode;			// pointer to tail of list of all nodes
    edge* headedge;			// pointer to head of list of all edges
    edge* tailedge;			// pointer to tail of list of all edges

    node* rnode;			// node in graph for this abstraction

// friends
    // sets a node's abstraction status, creaes window for graph if necessary
    friend void setnodestatus(node*, int);

public:

// constructors and destructors
    graph(long =0);
    graph(char *tname =0);
    virtual ~graph();

    void copy(graph*);
    virtual void copy_vals (graph *);

// operations for nodes
    // adds node with spec. title/label to the graph
    // sugi. layout dummy <== d, abstr. dummy <== sgd
    // node's type given by type/typename

      node* add_node(int d =0, int sgd =0,int type =0, char* type_name =0);

    // adds node with everything specifiable in IDL
    node* add_node(node* n);

    // deletes this node from the graph
    void delete_node(node* n);

    // adds abstraction node with spec. title/label to the graph
    node* add_absnode(graph* rg, char* title, char* label =0);
    node* add_absnode(node *n);


    // test if a node is in this graph
    int find_node (node *n);

   // given a node's title, return all nodes
   // absnodes = 1 returns also abstraction-nodes
   // else abstraction-nodes aren't searched
   nlist* find_all_nodes(char *a, int absnodes = 0);

// operations for all nodes

    // does a node::delete on all nodes in the graph
    void delete_nodes_all();
    // sets all nodes to unreached
    void set_nodes_unreached();


// operations for edges
    // adds edge between two nodes s and t.
    // dummy <== d
    // edges's type given by type/typename
    // if check_back=0 it isn't checked, if edge is a cycle-edge
    edge* add_edge(node* s, node* t,  int d=0,int type =0, char* type_name =0, int check_back=1);

    // adds edge between the two nodes which have these names 
    // finds pointers to nodes with these names, then invokes (above) add_edge
  edge* add_edge(char* sname, char* tname, int d=0,
	int type =0, char* type_name =0);
//    edge *add_edge (edge *);   buggy, and no refs, so it's removed
    // returns edge which is between nodes s and t
    edge* find_edge(node* s, node* t);
    // returns edge which is between the two nodes which have these names
    edge* find_edge(char* s, char* t);

   // does an edge::delete on all edges in the graph
    void delete_edges_all();
    // does an edge::layout on all edges in the graph
    void layout_edges_all();

// operations concerning subgraph relationships

    node* find_absnode(graph* g);

// operations for graph

					// including various levels of 
					// abstraction it may be in

 
    void setheadnode(node* n)		{headnode = n;}
    node* getheadnode()                 {return headnode;}

    void settailnode(node* n)		{tailnode = n;}
    node* gettailnode()                 {return tailnode;}

    void setheadedge(edge* e)		{headedge = e;}
    edge* getheadedge()                 {return headedge;}

    void settailedge(edge* e)		{tailedge = e;}
    edge* gettailedge()                 {return tailedge;}

// operations for graph layout
    void layout();			// layout graph acc. to layouttype

    // Sugiyama layout algorithm
    void Sugilayout(int BC_I =0, int BC_II =0, int PR =0, int SORTCC =0, int CC =0, int START_LEVEL =0);

    // Constraint layout algorithm based on Sugiiyama layout
    void Constraintlayout(int BC_I =0, int BC_II =0, int PR =0, int SORTCC =0, int CC =0, int INCR =0, int EXTENT =0, int START_LEVEL =0);


    // Edge Crossing Phase of Sugiyama Layout
    void Sugibary(int BC_I, int BC_II, int SORTCC, int CC);
    // Edge Crossing Phase of Constraint Layout
    void Constraintbary(int BC_I, int BC_II, int SORTCC, int CC, int INCR);
    void Finetuning(int PR, int START_LEVEL);		// Finetuning phase of Sugiyama layout
    void Constraintfinetuning(int PR, int START_LEVEL); // Finetuning phase of Constraint layout

    void Isilayout();			// ISI layout algorithm
    void Treelayout();			// Tree layout algorithm
    void Planarlayout();		// Planar layout algorithm
    void Nolayout();			// no layout algorithm 
    void compute_size();		// Calc. size of zoomed-in abstraction

    void setlayouttype(layout_algorithms i);  
    layout_algorithms getlayouttype()           	{return layoutalgorithm;}

    void setlayoutfreq(layoutfrequencies i);
    layoutfrequencies getlayoutfreq()           	{return layoutfrequency;}

    void setlayoutpar (int i, int val);
    int getlayoutpar (int i)            {return layoutparameter [i]; }

    void setheadlevel(level* l)		{headlevel = l;}
    level* getheadlevel ()		{return headlevel;}

    void setorientation(orientations i);
    orientations getorientation()		{return orientation;}

    void settopsort(topsorts i);
    topsorts gettopsort()		{return topsort;}

    void setdirty(int i)		{dirty = i;}
    int getdirty()			{return dirty;}

    void setxbase(int i);
    int getxbase()                      {return xbase;}

    void setybase(int i);
    int getybase()                      {return ybase;}

    void setxspace(int i);
    int getxspace()                     {return xspace;}

    void setyspace(int i);
    int getyspace()                     {return yspace;}

    void setzspace(int i);
    int getzspace()                     {return zspace;}

    void setx(int);
    int getx()                          {return x;}

    void sety(int);
    int gety()                          {return y;}

    void setxmax(int);
    int getxmax()                       {return xmax;}

    void setymax(int);
    int getymax()                       {return ymax;}


    void setwidth(int i)                {width = i;}
    int getwidth()                      {return width;}

    void setheight(int i)               {height = i;}
    int getheight()                     {return height;}

    void setscaling (float f)           {scaling = f;}
    float getscaling ()                 {return scaling;}

    void setmaxlevel(int l)             {maxlevel = l;}  
    int getmaxlevel()                   {return maxlevel;}

    void setproper (int i)		{ proper = i; }
    int isproper ()			{ return proper; }

     node* getrnode()                    {return rnode;}

    // used for selection

    node *getlastnode()			{ return lastnode;}
    void setlastnode (node *n)		{ lastnode = n; }

    node * getnexttolastnode ()		{ return nexttolastnode;}
    void setnexttolastnode (node *n)	{ nexttolastnode = n;}

    nlist *getselectednodes ()		{ return selectednodes;}
    void setselectednodes (nlist *nl)	{ selectednodes = nl;}




    // constraint related items and functions
    class constr_3D* constraints;
};

#endif


