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
// History:   04/08/92     S.Spivakovksy      Modified set_display to avoid nodes overlapping.	
//            04/15/92     S.Spivakovksy      Added code to  count horizontal crossings.

#include <msg.h>

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <fstream>
#endif /* ISO_CPP_HEADERS */

#define EDGE_WEIGHT 3
static  int WEIGHT = EDGE_WEIGHT;

#include "all.h"
#include "sugilevel.h"
#include "defaultparser.h"
#include "globdef.h"

extern int debug;


extern permutation* aset_sort_columns(matrix *, matrix *);
extern permutation* aset_sort_rows(matrix *, matrix *);

/*
 * levelitem constructor
 * creates a levelitem for node q on level il 
 */
levelitem::levelitem(node* q, level* il, treeinfo* ti)
{
  p=q;
  item_level=il;
  prev_item=0;
  next_item=0;
  info=ti;
};

/*
 * levelitem destructor
 */
levelitem::~levelitem() 
{
  if (info) delete info;
  if (next_item) delete next_item;
}

levelitem* level::sortedlilist() 
{
    levelitem* head = 0;
    levelitem* tail = 0;
    levelitem* tmp;
    for (int i= 0; i < c_items; i++) {
	tmp = findlevelitem(i);
        levelitem* new_item = new levelitem(tmp->p,this,0);
	new_item->next_item = NULL;
	new_item->prev_item = tail;
        if (new_item->prev_item != NULL)
	    new_item->prev_item->next_item = new_item;
	if (i == 0) {
	    head = new_item;
  	}
        tail = new_item;
    }
    first_sorted_item = head;
    return head;
}

levelitem* level::findlevelitem(int count) 
{
  int i;
  int j;
  for (i=0; i < c_items; i++) {
	if (item_permutation->getp(count) == i) break;
  }
  levelitem* l = first_item;
  for (j=0; j < i; j++) {
    l = l->next_item;
  }
 return l;
}

/*
 * print levelitem
 */
void 
levelitem::print() 
{
  msg("LEVELITEM") << eom; 
  msg("prev_item = ") << eom; 
  msg("") << eom;
  msg("next_item = ") << eom; 
  msg("") << eom;
  if (info) {
    msg("abs_pos = $1   rel_pos = $2   max_r_shift = $3   nr_kids = $4") << info->abs_pos << eoarg << info->rel_pos << eoarg << info->max_r_shift << eoarg << info->nr_kids << eom;
    msg("half_x_width = $1   y_height = $2") << info->half_x_width << eoarg << info->y_height << eom;
    if (info->first_son) {
      msg("sons = ") << eom;
      levelitem* s = info->first_son;
      for (int i=0; i<info->nr_kids; i++) {
	s = s->prev_item;
      };
      msg("") << eom;
    }
  };
  msg("\n") << eom;
};


/*
 * level constructor
 * constructs a new level for graph bg
 */
level::level(graph* bg) 
{
  backgraph=bg;
  c_level=0;
  max_level=0;
  c_items=0;
  first_item=0; 
  last_item=0; 
  prev_level=0;
  next_level=0;
  interconnection=0;
  row_connection=0;
  item_permutation=0;
  items=0;
  upper_connectivity=0;
  upper_priority=0;
  lower_connectivity=0;
  lower_priority=0;
  position=0;
  half_x_width=0;
  depth=0;
  y_height=0;
  stable=0;
};

level* level::getlevelwithnum(int levelnumber) 
{
  for (level* l = this; l != 0; l = l->next_level) {
	if (l->c_level == levelnumber)
	    return l;
  };
  return NULL;
}

void level::del_node(levelitem* li) 
{
    if (li->prev_item != NULL)
	li->prev_item->next_item = li->next_item;
    if (li->next_item != NULL)
	li->next_item->prev_item = li->prev_item;
    // delete li;
}

levelitem* 
level::replace_node(levelitem* p, node* q)
{
  levelitem* return_item = 0;
  levelitem* tmp;
  int q_level = q->getlevel();
  if (q_level == c_level) {
    for (tmp = first_sorted_item; tmp != p; tmp = tmp->next_item);
    levelitem* new_item = new levelitem(q,this,0);
    new_item->prev_item = tmp->prev_item;
    new_item->next_item = tmp->next_item;
    if (tmp->prev_item != NULL) {
	tmp->prev_item->next_item = new_item;
    } else {
        first_sorted_item = new_item;
    }
    if (tmp->next_item != NULL) {
	tmp->next_item->prev_item = new_item;
    } else 
    return_item = new_item;
  }
  else {
    msg("error replacement on wrong level number $1") << q_level << eom;
  }
  return return_item;
};

/*
 * level constructor
 * constructs a new level with number c for graph bg
 */
level::level(graph* bg, int c) 
{
  backgraph=bg;
  c_level=c;
  c_items=0;
  first_item=0; 
  last_item=0; 
  prev_level=0;
  next_level=0;
  interconnection=0;
  row_connection=0;
  item_permutation=0;
  items=0;
  upper_connectivity=0;
  upper_priority=0;
  lower_connectivity=0;
  lower_priority=0;
  position=0;
  half_x_width=0;
  depth=0;
  y_height=0;
  stable=0;
};

/*
 * add_node
 * inserts node q into the level structure.
 * Creates new levels if necessary
 */
levelitem* 
level::add_node(node* q) { return add_node(q, 0); };

levelitem* 
level::add_node(node* q, treeinfo* ti) 
{
  levelitem* return_item = 0;
  int q_level = q->getlevel();
  if (q_level == c_level) {
    levelitem* new_item = new levelitem(q,this,ti);
    if (last_item) {
      last_item->next_item = new_item;
      new_item->prev_item = last_item;
    }
    else {
      first_item = new_item;
    }
    last_item = new_item;
    c_items++;
    return_item = new_item;
  }
  else {
    if (q_level > c_level) {
      if (next_level == 0) {
	next_level = new level(backgraph,c_level+1);
	next_level->prev_level = this;
      };
      return_item = next_level->add_node(q, ti);
    }
    else if (debug) {
      msg("node ") << eom;
      msg(" with illegal level number $1") << q_level << eom;
    }
  };
  return return_item;
};

/* 
 * create_ic_matrix
 * creates interconnection matrix for the edges between this level and next 
 * level.
 * For each edge (i,j) interconnection[i][j] is incremented by one.
 */
void 
level::create_ic_matrix() 
{

// debug = 1;
  item_permutation = new permutation(c_items);
  items = new levelitem*[c_items];
  levelitem* li;
  int r = 0;
  for (li = first_item; li; li = li->next_item) 
       items[r++] = li;

//  fill in row_connection matrix which reflects node connections
//  on the same level (horizontal connections); include last level

  row_connection = new matrix(c_items, c_items);
  li = first_item;
  for (r=0; r < c_items; r++) {
      slist_iterator_forw succlist(*(slist*) li->p->getsucclist());
      node* succ;
      while (succ=(node*)succlist()) {
	levelitem* compare_item = first_item;
	for (int c=0; c < c_items; c++) {
	  if (compare_item->p == succ) {
	    ((*row_connection)[r][c]) += WEIGHT;
	    break;
	  }
	  compare_item = compare_item->next_item;
	}
      }
      li = li->next_item;
  }

if (debug == 1){
   msg(" horizontal connection on level = $1\n") << c_level << eom;
   for (r = 0; r < c_items; r++)
	for (int c=0; c < c_items; c++) {
           msg("row_conn[ $1,$2] = $3\n") << r << eoarg << c << eoarg << (*row_connection)[r][c] << eom;
        }
}
  if (next_level != 0) {
    interconnection = new matrix(c_items,next_level->c_items);
    li = first_item;
    for (r=0; r < c_items; r++) {
      slist_iterator_forw succlist(*(slist*) li->p->getsucclist());
      node* succ;
      while (succ=(node*)succlist()) {
	levelitem* compare_item = next_level->first_item;
	for (int c=0; c < next_level->c_items; c++) {
	  if (compare_item->p == succ) {
	    (*interconnection)[r][c]++;
	    break;
	  };
	  compare_item = compare_item->next_item;
	};
      };
      slist_iterator_forw predlist(*(slist*) li->p->getpredlist());
      node* pred;
      while (pred=(node*)predlist()) {
	levelitem* compare_item = next_level->first_item;
	for (int c=0; c < next_level->c_items; c++) {
	  if (compare_item->p == pred) {
	    (*interconnection)[r][c]++;
	    break;
	  };
	  compare_item = compare_item->next_item;
	};
      };
      li = li->next_item;
    };
    next_level->create_ic_matrix();
  }
  else
    interconnection = 0;
}

/* 
 * exchange_items
 * exchanges positions of two items on this level:
 * Has effect on item_permutation and the interconnection matrices.
 */
void 
level::exchange_items(int p1, int p2) 
{
  item_permutation->exchange(p1,p2);
  if (prev_level) prev_level->interconnection->exchange_columns(p1,p2);
  if (next_level) interconnection->exchange_rows(p1,p2);
  if (row_connection != 0){
      row_connection->exchange_columns(p1, p2);
      row_connection->exchange_rows(p1, p2);
  }
}

/* 
 * permute_items
 * permutes positions of all items on this level according to permutation p:
 * Has effect on item_permutation and the interconnection matrices.
 * size of permutation p and number of items on this level must be identical.
 */
void 
level::permute_items(permutation* p) 
{
  item_permutation->permute(p);
  if (prev_level) prev_level->interconnection->permute_columns(p);
  if (next_level) interconnection->permute_rows(p);
  if (row_connection != 0){
      row_connection->permute_columns(p);
      row_connection->permute_rows(p);
  }
}

/* 
 * reverse_rows 
 * items with equal barycenters on this level exchange their positions
 */
int 
level::reverse_rows(int CC) 
{
  int changed = 0;
  register float curr_bc = interconnection->barycenter_row(0);
  register float next_bc;
  int delta_k;
  register int r;
  for (r=0; r < c_items - 1; r++) {
    next_bc = interconnection->barycenter_row(r+1);
    if (curr_bc >= next_bc) {
      delta_k = interconnection->crossings_rev_rows(r);
      if (row_connection != 0)
             delta_k += (row_connection->symm_cross_rev_rows(r));
      if ((CC == 2) && (prev_level)) 
	delta_k += (prev_level->interconnection->crossings_rev_columns(r));
      if ( delta_k >= 0) {
	exchange_items(r,r+1);
	changed = 1;
      }
    }
    curr_bc = next_bc;
  };
  return changed;
}

/* 
 * reverse_columns
 * items with equal barycenters on next level exchange their positions
 */
int 
level::reverse_columns(int CC) 
{
  int changed = 0;
  register float curr_bc = interconnection->barycenter_column(0);
  register float next_bc;
  register int delta_k;
  register int c;
  for (c=0; c < next_level->c_items - 1; c++) {
    next_bc = interconnection->barycenter_column(c+1);
    if (curr_bc >= next_bc) {
      delta_k = interconnection->crossings_rev_columns(c);
      if ((CC == 2) && (next_level->next_level))
	delta_k = next_level->interconnection->crossings_rev_rows(c);
      if (next_level->row_connection != 0)
          delta_k += (next_level->row_connection->symm_cross_rev_rows(c));
      if (delta_k >= 0) {
	next_level->exchange_items(c,c+1);
	changed = 1;
      }
    }
    curr_bc = next_bc;
  };
  return changed;
}


/* 
 * baryorder_rows
 * order items on this level according to their lower barycenters
 * but only if the number of crossings decreases
 */
int 
level::baryorder_rows() 
{

  int k = interconnection->crossings();
if (debug)
   msg("bary_rows; k = $1\n") << k << eom;

  matrix* m1 = new matrix;
  matrix* symm1 = new matrix;

  m1->copy(interconnection);
  if (row_connection != 0){
      symm1->copy(row_connection);
      k+= (row_connection->symm_crossings());
if (debug)
    msg("bary_rows; k+ = $1\n") << k << eom;
  }
  permutation* p = aset_sort_rows(interconnection, row_connection);
  m1->permute_rows(p);
  int k1 = m1->crossings();
if (debug){
    msg("bary_rows; k1 = $1\n") << k << eom;
}
  if (row_connection != 0){
      symm1->permute_rows(p);      // to permute rows of symm. matrix
      symm1->permute_columns(p);   // permute columns too
      k1 +=  (symm1->symm_crossings() );
if (debug){
    msg("bary_rows; k1+ = $1\n") << k << eom;
}
  }
  delete m1;
  delete symm1;
  if ( k > k1 ) {
    permute_items(p);  
    return 1;
  }
  else return 0;
}

/* 
 * baryorder_rows_quick
 * order items on this level according to their lower barycenters
 */
int 
level::baryorder_rows_quick() 
{
  permutation* p = interconnection->sort_rows();
  permute_items(p);  
  return 1;
}

/* 
 * baryorder_columns
 * order items on next level according to their upper barycenters
 * but only if the number of crossings decreases
 */
int 
level::baryorder_columns() 
{
  int k = interconnection->crossings();
  if ( next_level->row_connection != 0)
      k += (next_level->row_connection->symm_crossings());

  matrix* m1 = new matrix;
  matrix* symm1 = new matrix;

  m1->copy(interconnection);
  if ( next_level->row_connection != 0)
       symm1->copy(next_level->row_connection);

//  permutation* p = interconnection->sort_columns();

  permutation* p = aset_sort_columns(interconnection, 
                             next_level->row_connection);
  m1->permute_columns(p);

  int k1 = m1->crossings();
  if ( next_level->row_connection != 0 ){
       symm1->permute_rows(p);     // to permute rows of symm. matrix
       symm1->permute_columns(p);  // permute columns too
       k1 += (symm1->symm_crossings());
  }
  delete m1;
  delete symm1;
  if ( k > k1 ) {
    next_level->permute_items(p);
    return 1;
  }
  else return 0;
}

/* 
 * baryorder_columns_quick
 * order items on next level according to their upper barycenters
 */
int 
level::baryorder_columns_quick() 
{
  permutation* p = interconnection->sort_columns();
  next_level->permute_items(p);
  return 1;
}

/* 
 * set_display
 * sets the absolute positions of all nodes in their window
 */
void 
level::set_display() 
{
  level* l = this;
  int y = backgraph->getybase();
  while (l) {
    edge_vector* p = l->position;
//    edge_vector* pp = l->half_x_width;
    edge_vector* z = l->depth;
    for (int x=0; x < l->c_items; x++) {
      levelitem* li = l->get_item(x);
      li->p->setx( (*p)[x] + (*z)[x] * backgraph->getzspace());
      // set height s.t. centered around middle of tallest node in this level;
      // NO, make then top alligned - sergey

      li->p->sety(y - li->p->getheight()/2 -
		  (*z)[x] * backgraph->getzspace());
    }
    y += l->y_height + backgraph->getyspace();
    l = l->next_level;
  }
};

#define DUMMYPRI 1000
#define STRAIGHTPRI 10000
/* 
 * init_finetuning
 * computes the initial positions of all items on each level taking
 * into regard the size of the items and the space between them
 * dummynodes are given a higher priority (by DUMMYPRI higher)
 * nodes with layout constraint are given the highest priority
 * (by STRAIGHTPRI * node->getstraight() higher)
 */
void 
level::init_finetuning() 
{
  half_x_width = new edge_vector(c_items);
  depth = new edge_vector(c_items);
  int c;
  for (c=0; c<c_items; c++) {
    levelitem* li = get_item(c);
    if ((backgraph->getorientation() == left_to_right) || (backgraph->getorientation() == right_to_left)) {
      (*half_x_width)[c] = li->p->getheight()/2 + li->p->getborderwidth();
      int y = li->p->getwidth() + 2*li->p->getborderwidth();
      if (y_height < y) y_height = y;
    } else {				// normal top-to-bottom orientation
      (*half_x_width)[c] = li->p->getwidth()/2 + li->p->getborderwidth();
      int y = li->p->getheight() + 2*li->p->getborderwidth();
      if (y_height < y) y_height = y;
    }
    (*depth)[c]=0;
  };
  position = new edge_vector(c_items);
  (*position)[0] = (*half_x_width)[0];
  for (c=1; c<c_items; c++) {
    (*position)[c] = (*position)[c-1] + (*half_x_width)[c-1]
      + backgraph->getxspace() + (*half_x_width)[c];
  };
  if (prev_level) {
    upper_connectivity = new edge_vector(c_items);
    for (int i=0; i<c_items; i++) {
      // set upper priority for normal node;
      (*upper_connectivity)[i] = 
	prev_level->interconnection->connectivity_column(i);
      // increase priorities for dummy nodes and nodes with layout constraints;
      levelitem* li = get_item(i);
      if (li->p->isdummy()) {
	// dummy node;
	int dummypri;
	if (i < (c_items/2)) dummypri = i; else dummypri = c_items - i;
	(*upper_connectivity)[i] +=
                ( prev_level->c_items + DUMMYPRI + dummypri );
      };
      if (li->p->getstraight()) {
	// layout constraint, node must be on a straight line;
	int straightpri;
	if (i < (c_items/2)) straightpri = i; else straightpri = c_items - i;
	(*upper_connectivity)[i] += 
	  STRAIGHTPRI * li->p->getstraight()+ straightpri;
      };
    };
    upper_priority = new permutation(c_items);
    upper_priority->sort(upper_connectivity);
  };
  if (next_level) {
    lower_connectivity = new edge_vector(c_items);
    for (int i=0; i<c_items; i++) {
      // set lower priority for normal node;
      (*lower_connectivity)[i] = interconnection->connectivity_row(i);
      // increase priorities for dummy nodes and nodes with layout constraints;
      levelitem* li = get_item(i);
      if (li->p->isdummy()) {
	int dummypri;
	if (i < (c_items/2)) dummypri = i; else dummypri = c_items - i;
	(*lower_connectivity)[i] += next_level->c_items + DUMMYPRI + dummypri;
      };
      if (li->p->getstraight()) {
	// layout constraint, node must be on a straight line;
	int straightpri;
	if (i < (c_items/2)) straightpri = i; else straightpri = c_items - i;
	(*lower_connectivity)[i] += next_level->c_items + 
	  STRAIGHTPRI * li->p->getstraight() + straightpri;
      }
    };
    lower_priority = new permutation(c_items);
    lower_priority->sort(lower_connectivity);
    next_level->init_finetuning();
  }
}

/*
 * find_cp_node_pos
 * looks for a node with layout constraint cp (node->getstraight() == cp)
 * on level l which is connected with node q
 * if none is found, found is set to false
 */
int 
find_cp_node_pos(level* l, node* n, int cp, int& found) {
  int pos = 0;
  int pos_c = 0;
  found = 0;
  for (int i=0; i<l->getcitems(); i++) {
    node* p = l->get_item(i)->getnode();
    if (p->getstraight() == cp) {
      if ((l->getbackgraph()->find_edge(n,p)) || 
	  (l->getbackgraph()->find_edge(p,n))) {
	found = 1;
	pos += (*(l->getposition()))[i];
	pos_c++;
      }
    }
  }
  if (found) pos = pos/pos_c;
  return pos;
}

/* 
 * adjust_down
 * adjusts items on this level in the order of their upper priority
 * only items with equal or lower priority are moved when it is attempted to
 * adjust an item to his position given by its upper barycenter
 */
void 
level::adjust_down() 
{
  for (int c = c_items - 1; c >= 0; c--) {
    int nr = (*upper_priority)[c];
    int cp = get_item(nr)->getnode()->getstraight();
    int new_pos;
    if (cp == 0) {
      new_pos = prev_level->interconnection->
	barycenter_column(nr,prev_level->position);
    }
    else {
      int found;
      new_pos = find_cp_node_pos(prev_level,get_item(nr)->getnode(),cp,found);
      if (!found) new_pos = prev_level->interconnection->
	barycenter_column(nr,prev_level->position);
    }
    shift(nr, new_pos - (*position)[nr], (*upper_connectivity)[nr],
	  upper_connectivity);
  };
}

/* 
 * adjust_up
 * adjusts items on this level in the order of their lower priority
 * only items with equal or lower priority are moved when it is attempted to
 * adjust an item to his position given by its lower barycenter
 */
void 
level::adjust_up() 
{
  for (int c = c_items - 1; c >= 0; c--) {
    int nr = (*lower_priority)[c];
    int cp = get_item(nr)->getnode()->getstraight();
    int new_pos;
    if (cp == 0) {
      new_pos = 
	interconnection->barycenter_row(nr,next_level->position);
    }
    else {
      int found;
      new_pos = find_cp_node_pos(next_level,get_item(nr)->getnode(),cp,found);
      if (!found) new_pos = interconnection->
	barycenter_row(nr,next_level->position);
    }
    shift(nr, new_pos - (*position)[nr], (*lower_connectivity)[nr],
	  lower_connectivity);
  };
}

/* 
 * adjust_careful_down
 * only items with lower priority are moved
 */
void 
level::adjust_careful_down() 
{
  for (int c = c_items - 1; c >= 0; c--) {
    int nr = (*upper_priority)[c];
    int cp = get_item(nr)->getnode()->getstraight();
    int new_pos;
    if (cp == 0) {
      new_pos = prev_level->interconnection->
	barycenter_column(nr,prev_level->position);
    }
    else {
      int found;
      new_pos = find_cp_node_pos(prev_level,get_item(nr)->getnode(),cp,found);
      if (!found) new_pos = prev_level->interconnection->
	barycenter_column(nr,prev_level->position);
    }
    shift(nr, new_pos - (*position)[nr], (*upper_connectivity)[nr]-1,
	  upper_connectivity);
  };
}

/* 
 * adjust_careful_up
 * only items with lower priority are moved
 */
void 
level::adjust_careful_up() 
{
  for (int c = c_items - 1; c >= 0; c--) {
    int nr = (*lower_priority)[c];
    int cp = get_item(nr)->getnode()->getstraight();
    int new_pos;
    if (cp == 0) {
      new_pos = 
	interconnection->barycenter_row(nr,next_level->position);
    }
    else {
      int found;
      new_pos = find_cp_node_pos(next_level,get_item(nr)->getnode(),cp,found);
      if (!found) {
	new_pos = interconnection->
	  barycenter_row(nr,next_level->position);
      }
    }
    shift(nr, new_pos - (*position)[nr], (*lower_connectivity)[nr]-1,
	  lower_connectivity);
  };
}

/* 
 * shift
 * shifts item with number nr sh units to left if sh is positive,
 * else to the left
 * but only if the priority prior is not lower than the priority of the
 * neighbor
 */
int 
level::shift(int nr, int sh, int prior, edge_vector* connectivity) 
{
  if (sh > 0) {
    if (nr == c_items-1) {
      (*position)[nr] += sh;
      return sh;
    }
    else {
      int r_margin = (*position)[nr] + (*half_x_width)[nr];
      int next_l_margin = (*position)[nr+1] - (*half_x_width)[nr+1];
      if (r_margin + backgraph->getxspace() + sh < next_l_margin) {
	(*position)[nr] += sh;
	return sh;
      }
      else {
	int s = next_l_margin - backgraph->getxspace() - r_margin - 1;
	(*position)[nr] += s;
	if (prior < (*connectivity)[nr+1]) return s;
	else {
	  int ss = shift(nr+1,sh-s,prior,connectivity);
	  (*position)[nr] += ss;
	  return s + ss;
	}
      }
    }
  }
  else {
    if (sh < 0) {
      if (nr == 0) {
	(*position)[nr] += sh;
	return sh;
      }
      else {
	int l_margin = (*position)[nr] - (*half_x_width)[nr];
	int prev_r_margin = (*position)[nr-1] + (*half_x_width)[nr-1];
	if (l_margin - backgraph->getxspace() + sh > prev_r_margin) {
	  (*position)[nr] += sh;
	  return sh;
	}
	else {
	  int s = prev_r_margin - l_margin + backgraph->getxspace() + 1;
	  (*position)[nr] += s;
	  if (prior < (*connectivity)[nr-1]) return s;
	  else {
	    int ss = shift(nr-1,sh-s,prior,connectivity);
	    (*position)[nr] += ss;
	    return s + ss;
	  }
	}
      }
    }
    else return 0;
  }
}

/* 
 * last_adjust
 * moves the whole graph to the left margin
 */
void 
level::last_adjust() 
{
  int shift = 9999;
  level* l = this;
  edge_vector* p;
  edge_vector* pp;
  while ( l ) {
    p = l->position;
    pp = l->half_x_width;
    if ( (*p)[0] - (*pp)[0] < shift ) shift = (*p)[0] - (*pp)[0];
    l = l->next_level;
  };
  l = this;
  while ( l ) {
    for (int c=0; c < l->c_items; c++) {
      p = l->position;
      (*p)[c] -= shift;
    };
    l = l->next_level;
  }
}

/* 
 * crossings
 * returns number of crossings of all edges among the following levels
 */
int 
level::crossings(int deb) 
{
  level* l = this;
  int i;
  int c = 0;
  while (l->next_level) {
    i = l->interconnection->crossings() + 
        l->row_connection->symm_crossings();
    if (deb) msg("$1 crossings at level $2") << i << eoarg << l->c_level << eom;
    c += i;
    l = l->next_level;
  }
//  count horizontal connections on last level

  if (l->row_connection)
      c += l->row_connection->symm_crossings();

  return c;
}

/* 
 * printross
 * prints this and the following levels to the standard output
 */
void 
level::print() 
{
  msg("Level : $1 , $2 items\n") << c_level << eoarg << c_items << eom;
  for (levelitem* l=first_item; l; l = l->next_item) 
    l->print();
  if (next_level) next_level->print();
};
/*
  printf("Level : %d , %d items\n", c_level, c_items);
  for (int r=0; r<c_items; r++) {
    int position = (*item_permutation)[r];
    levelitem* l=first_item;
    for (int i=0; i<position; i++) {
      l = l->next_item;
    };
    l->p->print();
  };
  printf("\n");
  if (next_level) {
    printf("Interconnection matrix\n");
    interconnection->print();
    printf("crossings : %d\n", interconnection->crossings());
    if (upper_connectivity) {
      printf("upper connectivity : ");
      upper_connectivity->print();
    };
    if (upper_priority) {
      printf("upper priority : ");
      upper_priority->print();
    };
    if (lower_connectivity) {
      printf("lower connectivity : ");
      lower_connectivity->print();
    };
    if (lower_priority) {
      printf("lower priority : ");
      lower_priority->print();
    };
    printf("\n\n");
    next_level->print();
  };
};
*/


/* 
 * level destructor
 */
level::~level() 
{
  if (first_item) delete first_item;
  if (interconnection) delete interconnection;
  if (row_connection) delete row_connection;
  if (item_permutation) delete item_permutation;
  if (items) delete items;
  if (upper_connectivity) delete upper_connectivity;
  if (upper_priority) delete upper_priority;
  if (lower_connectivity) delete lower_connectivity;
  if (lower_priority) delete lower_priority;
  delete position;
  delete half_x_width;
  delete depth;
  if (next_level) delete next_level;
}

void level::remove_empty_levels() {
  level* l = this;
  while (l->prev_level) l = l->prev_level;    // go to top level;
  
  while ((l->next_level) && (l->c_items == 0)) {
    // level l must be deleted;
    level* del = l;
    l = l->next_level;
    del->prev_level = 0;
    del->next_level = 0;
    delete del;
    l->prev_level = 0;
    l->backgraph->setheadlevel(l);
  }

  while (l->next_level) {
    if (l->c_items == 0) {
      // insert dummy node;
      char* dummykey = new char[10];
      int dummycount = 0;
      sprintf(dummykey,"empty-%d", dummycount++);
      node* n = backgraph->add_node(1);
      n->setlevel(l->c_level);
      l->add_node(n);
    }
    l = l->next_level;
  }
}


permutation* aset_sort_columns(matrix *m1, matrix *m2)
{
  if (m2 == NULL)
      return ( m1->sort_columns() );

  if (m1->columns != m2->columns)      // rows can be different
      msg("error from aset_sort_columns ") << eom;

  int columns = m1->columns;

// consider horizontal connections too
	
  int c, k;
  int a1, a2, b1, b2;

  floatvector bc(columns);

  for (c=0; c < columns; c++) {
    a1 = a2 = 0;
    for (k=0; k<m1->rows; k++) {
         a1 += k * m1->m[k][c];
         a2 += m1->m[k][c];
    }

//  second matrix - horizontal connections
    
    b1 = b2 = 0;
    for (k=0; k< m2->rows; k++) {
         b1 += k *  m2->m[k][c] ;
         b2 += m2->m[k][c];
    }
    if (a2 + b2) 
        bc[c] = float (a1 + b1) / float (a2 + b2);
    else
        bc[c] = c;
  }
  return bc.sort();
}




permutation* aset_sort_rows(matrix *m1, matrix *m2)
{
  if (m2 == NULL)
      return ( m1->sort_rows() );

  if (m1->rows != m2->rows)         // # of columns can be different
      msg("error from aset_sort_rows ") << eom;

  int rows = m1->rows;

// consider horizontal connections too

  int r, k;
  int a1, a2, b1, b2;

  floatvector bc(rows);

  for (r=0; r < rows; r++) {
    a1 = a2 = 0;
    for (k=0; k<m1->columns; k++) {
         a1 += k * m1->m[r][k];
         a2 += m1->m[r][k];
    }

//  second matrix - horizontal connections
    
    b1 = b2 = 0;
    for (k=0; k< m2->columns; k++) {
         b1 += (k  * m2->m[r][k] );
         b2 += (m2->m[r][k] );
    }
if (debug){
    msg("a1 = $1 a2 = $2 b1 = $3 b2= $4\n") << a1 << eoarg << a2 << eoarg << b1 << eoarg << b2 << eom;
}
    if (a2 + b2) 
        bc[r] = float (a1 + b1) / float (a2 + b2);
    else
        bc[r] = r;
  }
if (debug){
    msg(" sort_rows\n") << eom;
    for (r=0; r < rows ; r++)
       msg(" bc[$1] = $2\n") << r << eoarg << bc[r] << eom;
}
  return bc.sort();
}

/*
   START-LOG-------------------------------------------

   $Log: sugilevel.cxx  $
   Revision 1.5 2000/07/12 18:08:48EDT ktrans 
   merge from stream_message branch
Revision 1.2.1.3  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:57:52  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
