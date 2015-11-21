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
#include "all.h"
#include "constr.h"
extern int debug;

constr_3D::constr_3D(graph* g) { 
  backgraph = g;
  for (dimension d = X; d <= Z; d = (dimension) (d + 1)) manager[d].set_dimension(d);
}

constr_3D::~constr_3D() {}

int constr_3D::equal  
  (constr_type t, dimension d, node* n1, node* n2, mode m, int priority) 
{
  nlist nl(n1); nl.append(n2);
  constr_descriptor* cd = new constr_descriptor
    ((constr_function) &constr_manager::equal, 
     t, "equal", d, &nl, 0, -1, priority);
  return manager[d].equal(cd, m);
}

int constr_3D::smaller
  (constr_type t, dimension d, node* n1, node* n2, mode m, int priority) 
{ 
  nlist nl(n1); nl.append(n2);
  constr_descriptor* cd = new constr_descriptor
    ((constr_function) &constr_manager::smaller, 
     t, "smaller", d, &nl, 0, -1, priority);
  return manager[d].smaller(cd, m);
}

int constr_3D::greater
  (constr_type t, dimension d, node* n1, node* n2, mode m, int priority) 
{
  nlist nl(n1); nl.append(n2);
  constr_descriptor* cd = new constr_descriptor
    ((constr_function) &constr_manager::greater, 
     t, "greater", d, &nl, 0, -1, priority);
  return manager[d].greater(cd, m);
}

int constr_3D::neighbors
  (constr_type t, dimension d, node* n1, node* n2, mode m, int priority) 
{
  nlist nl(n1); nl.append(n2);
  constr_descriptor* cd = new constr_descriptor
    ((constr_function) &constr_manager::neighbors, 
     t, "neighbors", d, &nl, 0, -1, priority);
  return manager[d].neighbors(cd, m);
}

int constr_3D::low_margin 
  (constr_type t, dimension d, node* n1, mode m, int priority) 
{
  nlist nl(n1);
  constr_descriptor* cd = new constr_descriptor
    ((constr_function) &constr_manager::low_margin, 
     t, "low margin", d, &nl, 0, -1, priority);
  return manager[d].low_margin(cd, m);
}

int constr_3D::high_margin
  (constr_type t, dimension d, node* n1, mode m, int priority) 
{
  nlist nl(n1);
  constr_descriptor* cd = new constr_descriptor
    ((constr_function) &constr_manager::high_margin,
     t, "high margin", d, &nl, 0, -1, priority);
  return manager[d].high_margin(cd, m);
}

int constr_3D::range 
  (constr_type t, dimension d, node* n1, interval& i, mode m, int priority)
{
  nlist nl
;
  if (n1) nl.append(n1);
  constr_descriptor* cd = new constr_descriptor
    ((constr_function) &constr_manager::range, 
     t, "range", d, &nl, &i, -1, priority);
  int r = manager[d].range(cd, m);
  i = cd->i;
  return r;
}

int constr_3D::cluster
  (constr_type t, dimension d, node* n1, nlist* nl, mode m, int priority) 
{
  nl->insert(n1);
  constr_descriptor* cd = new constr_descriptor
    ((constr_function) &constr_manager::cluster,
     t, "cluster", d, nl, 0,-1, priority);
  return manager[d].cluster(cd, m);
}

int constr_3D::limit
  (constr_type t, dimension d, node* n1, int& distance, mode m, int priority) 
{
  nlist nl(n1);
  constr_descriptor* cd = new constr_descriptor
    ((constr_function) &constr_manager::limit, 
     t, "limit", d, &nl, 0, distance, priority);
  int l = manager[d].limit(cd, m); 
  distance = cd->distance;
  return l;
}

int constr_3D::above(constr_type t, nlist* nl, mode m, int priority) 
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= smaller(t,Y,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::below(constr_type t, nlist* nl, mode m, int priority) 
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= greater(t,Y,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::left(constr_type t, nlist* nl, mode m, int priority) 
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= smaller(t,X,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::right(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= greater(t,X,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::in_front(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= smaller(t,Z,n1,n2,m,priority) & equal(t,X,n1,n2,m,priority) 
      & equal(t,Y,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::behind(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= greater(t,Z,n1,n2,m,priority) & equal(t,X,n1,n2,m,priority)
      & equal(t,Y,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::equal_line(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= equal(t,Y,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::equal_column(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= equal(t,X,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::equal_position(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= equal(t,X,n1,n2,m,priority) & equal(t,Y,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::top_margin(constr_type t, nlist* nl, mode m, int priority) 
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n;
  while (n = (node*) s()) {
    r &= low_margin(t,Y,n,m,priority);
  }
  return r;
}

int constr_3D::bottom_margin(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n;
  while (n = (node*) s()) {
    r &= high_margin(t,Y,n,m,priority);
  }
  return r;
}

int constr_3D::left_margin(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n;
  while (n = (node*) s()) {
    r &= low_margin(t,X,n,m,priority);
  }
  return r;
}

int constr_3D::right_margin(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n;
  while (n = (node*) s()) {
    r &= high_margin(t,X,n,m,priority);
  }
  return r;
}

int constr_3D::upper_neighbor(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= equal(t,X,n1,n2,m,priority) & neighbors(t,Y,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::lower_neighbor(constr_type t, nlist* nl, mode m, int priority) 
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= equal(t,X,n1,n2,m,priority) & neighbors(t,Y,n2,n1,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::left_neighbor(constr_type t, nlist* nl, mode m, int priority)
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= equal(t,Y,n1,n2,m,priority) & neighbors(t,X,n1,n2,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::right_neighbor(constr_type t, nlist* nl, mode m, int priority) 
{
  int r = 1;
  slist_iterator_forw s(*(slist*) nl);
  node* n1 = (node*) s();
  node* n2;
  while (n2 = (node*) s()) {
    r &= equal(t,Y,n1,n2,m,priority) & neighbors(t,X,n2,n1,m,priority);
    n1 = n2;
  }
  return r;
}

int constr_3D::consistency() {
  int c = manager[X].consistency();
  c &= manager[Y].consistency();
  c &= manager[Z].consistency();
  return c;
}

void constr_3D::delete_constraints(dimension d, constr_type t, node* n) {
  manager[d].delete_constraints(t,n);
}

void constr_3D::delete_constraints(constr_type t, node* n) {
  manager[X].delete_constraints(t,n);
  manager[Y].delete_constraints(t,n);
  manager[Z].delete_constraints(t,n);
}

void constr_3D::output(ostream &to, int indent) {
  delete_constraints(LAYOUT_TEMP);
  delete_constraints(LAYOUT_PERM);

  manager[X].output(to, indent);   
  manager[Y].output(to, indent);   
  manager[Z].output(to, indent);
}
