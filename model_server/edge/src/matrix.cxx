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
/*	Implements "reduce crossing" phase of Sugiyama layout
 *	Counts and reduces crossings given in matrix form
 */

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <strstream>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include "matrix.h"

/*
 * copy
 * copies vector a to this vector
 * size of a and this vector must be identical
 */
void 
edge_vector::copy(edge_vector* a) 
{
  for (int i=0; i<size; i++) v[i] = a->v[i];
}

/*
 * permute
 * permutes this vector according to the permutation p
 * this[i] becomes this[p[i]]
 * size of p and this vector must be identical
 */
void 
edge_vector::permute(permutation* p) 
{
  int* oldv = new int[size];
  int i;
  for (i=0; i<size; i++) {
    oldv[i]=v[i];
  };
  for (i=0; i<size; i++) {
    v[i]=oldv[p->v[i]];
  };
  delete oldv;
  oldv = 0;
}

/*
 * shift
 */
int edge_vector::shift(int from, int to) {
  int rem = v[from];
  if (to < from) {
    for (int i=from; i>to; i--) v[i]  = v[i-1];
    v[to] = rem;
    return 1;
  }
  else if (to > from) {
    for (int i=from; i<to; i++) v[i] = v[i+1];
    v[to] = rem;
    return 1;
  }
  else return 0;
}

int permutation::getp(int i)
{
    return v[i];
}

/*
 * permutation constructor
 * constructs a permutation vector of size s
 * initialized with the identical permutation
 */
permutation::permutation(int s) 
{
  size = s; 
  v = new int[size];
  for (int i=0; i<size; i++) {
    v[i]=i;
  }
}

/*
 * sort
 * this becomes the permutation to bring vector a into a non-decreasing order
 * size of a and this permutation must be identical
 */
void 
permutation::sort(edge_vector* a) 
{
  edge_vector* b = new edge_vector(a->size);
  b->copy(a);
  int i = size - 1;
  while (i) {
    int k = 0;
    for (int j=0; j < i; j++) {
      if (b->v[j] > b->v[j+1]) {
	exchange(j,j+1);
	b->exchange(j,j+1);
	k = j;
      }
    };
    i = k;
  };
  delete b;
  b = 0;
}

/*
 * read_permutation
 * reads a permutation from the standard input
 */
void 
permutation::read_permutation() 
{
  for (int i=0; i<size; i++) {
    msg("p[$1]=") << i << eom;
    cin >> v[i];
  }
}

/*
 * sort
 * returns the permutation to bring this vector into a non-decreasing order
 * mergesort algorithm
 */
permutation* 
floatvector::sort() 
{
  permutation* p = new permutation(size);
  permutation* newp = new permutation(size);
  for (int l=1; l<size; l*=2) {
    int pos = 0;
    for (int i=0; i<size; i+=2*l) {
      int a, b, a_end, b_end;
      b_end = (a_end = (b = (a = i) + l)) + l;
      if (a_end > size) { a_end = b_end = size; }
      else if (b_end > size) b_end = size;
      while ((a < a_end) && (b < b_end)) {
	if (v[(*p)[a]] <= v[(*p)[b]]) (*newp)[pos++] = (*p)[a++];
	else (*newp)[pos++] = (*p)[b++];
      };
      for (; a < a_end;) (*newp)[pos++] = (*p)[a++];
      for (; b < b_end;) (*newp)[pos++] = (*p)[b++];
    };
    permutation* h = p;
    p = newp;
    newp = h;
  };
  delete newp;
  newp = 0;
  return p;
}


/*
 * matrix constructor
 * consructs r x c matrix and initializes all elements to zero
 */
matrix::matrix(int r, int c) 
{
  rows=r; columns=c;
  m = new edge_vector[r];
  for (int i=0; i<r; i++) {
    m[i].size=c;
    m[i].v=new int[c];
    for (int k=0; k<c; k++) m[i].v[k]=0;
  }
}

/*
 * matrix destructor
 */
matrix::~matrix() 
{
#if 0
  for (int i=0; i<rows; i++) {
    if (m[i].v != 0)
      delete m[i].v;
    m[i].v = 0;
  };
  delete m;
#else
  delete [] m;
#endif
  m = 0;
}

/*
 * copy
 * makes this matrix a copy of matrix n
 * rows and columns of n and this matrix must be identical
 */
void 
matrix::copy(matrix* n) 
{
  rows=n->rows;
  columns=n->columns;
  m = new edge_vector[rows];
  for (int i=0; i<rows; i++) {
    m[i].size=columns;
    m[i].v=new int[columns];
    for (int k=0; k<columns; k++) m[i].v[k]=n->m[i].v[k];
  }
}

/*
 * crossings
 * computes the number of crossings
 * algorithm:
 * matrix[i][j] represents the number of edges between position i on the upper
 * level and position j on the lower level, call it (i,j).
 * The edges crossing edge (i,j) from right to left are exactly those
 * edges (k,l) with k > i and l < j. You get the number of those edges
 * by adding all elements of the submatrix[k][l] with k > i and l < j.
 * The product of the elements of the submatrix and m[i][j] yields the 
 * number of crossings.
 * To compute all crossings you must iterate over all elements of the matrix.
 * This computation time is linear in the size of the matrix.
 */
int 
matrix::crossings() 
{
  register int i , k , b , submatrix , s;
  s = 0;
  edge_vector subrow(rows);
  for (i=0; i<rows; i++) subrow[i] = 0;
  for (k=1; k<columns; k++) {
    subrow[rows-1] += m[rows-1][k-1];
    submatrix = subrow[rows-1];
    for (i=rows-2; i>=0; i--) {
      if (b = m[i][k]) {
	s += submatrix;
	if (--b) s += b * submatrix;
      }
      subrow[i] += m[i][k-1];
      submatrix += subrow[i];
    }
  }
  return s;
}

// matrix crossings - # of nodes crossed by horizontal edges
// at the same level - matrix is not symmetrical any more

int matrix::symm_crossings()
{
  register int i, j, k, total;
  total = 0;

//   j = # of nodes between nodes i and k; #rows = # columns

// from llower to upper indexes

   for (i=0; i<rows; i++) {
       for (j=1, k=i+2; k<rows; k++, j++)
           total += ( m[i][k] * j );
   }

// from upper to lower indexes

   for (i=rows-1; i > 1; i--) {
       for (j= rows - 2, k= 0; k < i - 1; k++, j--)
           total += ( m[i][k] * j );
   }
   return total;
}
// same as above for symmetrical matrix - node crossings on one level
// when r switched with r+1 the difference involves only 4
// nodes : (r-1, r, r+1, r+2) -> (r-1, r+1, r, r+2)

int matrix::symm_cross_rev_rows(int r)
{
  int before_crossings = 0;
  int after_crossings = 0;

// since matrix is not symmetrical any more consider both
// directions separately; only one of m[i,j] / m[j,i] != 0

  if (r != 0){
      before_crossings += m[r-1][r+1];  // crossing r
      before_crossings += m[r+1][r-1];
      after_crossings += m[r-1][r];     // crossing r+1
      after_crossings += m[r][r-1];
  }
  if (r < (rows - 2) ){                // r+2 exists
      before_crossings += m[r][r+2];   // crossing r+1
      before_crossings += m[r+2][r];
      after_crossings += m[r+1][r+2];  // crossing r
      after_crossings += m[r+2][r+1];
  }
  int  s = after_crossings - before_crossings;
  return s;
}


/*
 * crossings_quick
 * assumes that there is at most one edge from position i on the upper level
 * and position j on the lower level. This avoids a multiplication but
 * does not give the exact number of crossings if the assumption is broken.
 */
int 
matrix::crossings_quick() 
{
  register int i , k , submatrix , s;
  s = 0;
  edge_vector subrow(rows);
  for (i=0; i<rows; i++) subrow[i] = 0;
  for (k=1; k<columns; k++) {
    subrow[rows-1] += m[rows-1][k-1];
    submatrix = subrow[rows-1];
    for (i=rows-2; i>=0; i--) {
      s += submatrix;
      subrow[i] += m[i][k-1];
      submatrix += subrow[i];
    }
  }
  return s;
}

/*
 * crossings_rev_rows
 * computes difference of the number of crossings before and after reversing
 * rows r and r+1
 * The algorithm follows from the algorithm to compute all crossings but is
 * linear in the number of columns.
 */
int 
matrix::crossings_rev_rows(int r)
{
  register int s, i, subrow0, subrow1;
  s = subrow0 = subrow1 = 0;
  for (i=1; i<columns; i++) {
    subrow0 += m[r][i-1];
    subrow1 += m[r+1][i-1];
    s += m[r][i] * subrow1 - m[r+1][i] * subrow0;
  } 
  return s;
}

/*
 * crossings_rev_rows_quick
 * computes difference of the number of crossings before and after reversing
 * rows r and r+1 by assuming that at most one edge exists between two nodes
 */
int 
matrix::crossings_rev_rows_quick(int r)
{
  register int s, i, subrow0, subrow1;
  s = subrow0 = subrow1 = 0;
  for (i=1; i<columns; i++) {
    subrow0 += m[r][i-1];
    subrow1 += m[r+1][i-1];
    if (m[r][i]) s += subrow1;
    if (m[r+1][i]) s -= subrow0;
  } 
  return s;
}

/*
 * crossings_rev_columns
 * computes difference of the number of crossings before and after reversing
 * columns c and c+1
 * The algorithm follows from the algorithm to compute all crossings but is
 * linear in the number of rows.
 */
int 
matrix::crossings_rev_columns(int c)
{
  register int s, i, subcolumn0, subcolumn1;
  s = subcolumn0 = subcolumn1 = 0;
  for (i=1; i<rows; i++) {
    subcolumn0 += m[i-1][c];
    subcolumn1 += m[i-1][c+1];
    s += m[i][c] * subcolumn1 - m[i][c+1] * subcolumn0;
  } 
  return s;
}

/*
 * crossings_rev_columns_quick
 * computes difference of the number of crossings before and after reversing
 * columns c and c+1 by assuming that at most one edge exists between two nodes
 */
int 
matrix::crossings_rev_columns_quick(int c)
{
  register int s, i, subcolumn0, subcolumn1;
  s = subcolumn0 = subcolumn1 = 0;
  for (i=1; i<rows; i++) {
    subcolumn0 += m[i-1][c];
    subcolumn1 += m[i-1][c+1];
    if (m[i][c]) s += subcolumn1; 
    if (m[i][c+1]) s -= subcolumn0;
  } 
  return s;
}

/*
 * permute_rows
 * permutes the rows of this matrix according to permutation p.
 * this[i][..] becomes this[p[i]][..].
 * size of p and rows of this matrix must be identical.
 */
void 
matrix::permute_rows(permutation* p) 
{
  int** oldr = new int*[rows];
  int r;
  for (r=0; r<rows; r++) oldr[r]=m[r].v;
  for (r=0; r<rows; r++) m[r].v=oldr[p->v[r]];
  delete oldr;
  oldr = 0;
}

/*
 * permute_columns
 * permutes the columns of this matrix according to permutation p.
 * this[..][j] becomes this[..][p[j]].
 * size of p and columns of this matrix must be identical.
 */
void 
matrix::permute_columns(permutation* p) 
{
  int* oldv = new int[columns];
  for (int r=0; r<rows; r++) {
    int c;
    for (c=0; c<columns; c++) {
      oldv[c]=m[r].v[c];
    };
    for (c=0; c<columns; c++) {
      m[r].v[c]=oldv[p->v[c]];
    }
  };
  delete oldv;
  oldv = 0;
}

/*
 * barycenter_row
 * returns the barycenter of row r
 * that means the lower barycenter of item on position r
 */
float 
matrix::barycenter_row(int r) 
{
  int b1=0;
  int b2=0;
  for (int l=0; l<columns; l++) {
    b1 += l * m[r].v[l];
    b2 += m[r].v[l];
  };
  if (b2) return float(b1) / b2; else return r;
}

/*
 * barycenter_row
 * returns the weighted barycenter of row r
 * that means the lower barycenter of item on position r
 * size of vector weight and columns of this matrix must be identical
 */
int 
matrix::barycenter_row(int r, edge_vector* weight) 
{
  int b1=0;
  int b2=0;
  for (int l=0; l<columns; l++) {
    b1 += weight->v[l] * m[r].v[l];
    b2 += m[r].v[l];
  };
  if (b2) return b1 / b2; else return 0;
}

/*
 * barycenter_column
 * returns the weighted barycenter of column c
 * that means the upper barycenter of item on position c
 */
float 
matrix::barycenter_column(int c) 
{
  int b1=0;
  int b2=0;
  for (int k=0; k<rows; k++) {
    b1 += k * m[k].v[c];
    b2 += m[k].v[c];
  };
  if (b2) return float(b1) / b2; else return c;
}

/*
 * barycenter_column
 * returns the weighted barycenter of column c
 * that means the upper barycenter of item on position c
 * size of vector weight and rows of this matrix must be identical
 */
int 
matrix::barycenter_column(int c, edge_vector* weight) 
{
  int b1=0;
  int b2=0;
  for (int k=0; k<rows; k++) {
    b1 += weight->v[k] * m[k].v[c];
    b2 += m[k].v[c];
  };
  if (b2) return b1 / b2; else return 0;
}

/*
 * exchange_rows
 * exchanges rows r1 and r2 of this matrix
 */
void 
matrix::exchange_rows(int r1, int r2) 
{
  int* h = m[r1].v;
  m[r1].v = m[r2].v;
  m[r2].v = h;
}

/*
 * exchange_columns
 * exchanges columns c1 and c2 of this matrix
 */
void 
matrix::exchange_columns(int c1, int c2) 
{
  for (int i=0; i<rows; i++) {
    int h = m[i].v[c1];
    m[i].v[c1]=m[i].v[c2];
    m[i].v[c2]=h;
  }
}

/*
 * sort_rows
 * returns the permutation to bring the rows of this matrix in such an order
 * that the barycenters of the rows are in non-decreasing order
 */
permutation* 
matrix::sort_rows() 
{
  floatvector bc(rows);
  for (int r=0; r<rows; r++) bc[r]=barycenter_row(r);
  return bc.sort();
}

/*
 * sort_columns
 * returns the permutation to bring the columns of this matrix in such an order
 * that the barycenters of the columns are in non-decreasing order
 */
permutation* 
matrix::sort_columns() 
{
  floatvector bc(columns);
  for (int c=0; c<columns; c++) bc[c]=barycenter_column(c);
  return bc.sort();
}

/*
 * connectivity_row
 * returns the lower connectivity of item on position r
 * that means the number of edges leaving item r downward
 */
int 
matrix::connectivity_row(int r) 
{
  int cr = 0;
  for (int c=0; c<columns; c++) cr += m[r].v[c];
  return cr;
}

/*
 * connectivity_column
 * returns the upper connectivity of item on position c
 * that means the number of edges coming into item c from above
 */
int 
matrix::connectivity_column(int c) 
{
  int cc = 0;
  for (int r=0; r<rows; r++) cc += m[r].v[c];
  return cc;
}

/*
   START-LOG-------------------------------------------

   $Log: matrix.cxx  $
   Revision 1.6 2000/07/12 18:08:46EDT ktrans 
   merge from stream_message branch
Revision 1.2.1.3  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:57:45  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
