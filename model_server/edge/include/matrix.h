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
#ifndef _MATRIX_H
#define _MATRIX_H

/* 	class definition for vector, permutation, matrix
 *	used for reducing and counting crossings in sugiyama layout
 */

class permutation;

class edge_vector {
protected:
  friend class matrix;
  friend class permutation;
  int size;
  int* v;
public:
  // constructors and destructors
  edge_vector() 				{size = 0; v = 0;};
  edge_vector(int sz) 			{size = sz; v = new int[size];};
  ~edge_vector() 				{if (v) delete v; v = 0;};

  int& operator[](int i) 		{return v[i];};
  void copy(edge_vector*);
  void exchange(int p1,int p2) 		{int h=v[p1]; v[p1]=v[p2]; v[p2]=h;};
  void permute(permutation*);
  int shift(int from, int to);
};

class permutation : public edge_vector {
public:
  // constructors and destructors
  permutation(int);
  ~permutation() 			{if (v) delete v; v = 0; };

  void sort(edge_vector*);
  void read_permutation();
  int getp(int i);
};

class floatvector {
  int size;
  float* v;
public:
  // constructors and destructors
  floatvector(int s) 			{size = s; v = new float[size];};
  ~floatvector() 			{if (v) delete v; v = 0; };

  float& operator[](int i) 		{return v[i];};
  void exchange(int p1,int p2) 		{float h=v[p1]; v[p1]=v[p2]; v[p2]=h;};
  permutation* sort();
};

class matrix {
  int rows;
  int columns;
  edge_vector* m;
public:
  // constructors and destructors
  matrix() 				{rows=0; columns=0; m=0;};
  matrix(int,int);
  ~matrix();

  edge_vector& operator[](int i) 		{return m[i];};
  void copy(matrix*);
  int crossings();
  int crossings_quick();
  int crossings_rev_rows(int);
  int crossings_rev_rows_quick(int);
  int crossings_rev_columns(int);
  int crossings_rev_columns_quick(int);
  void exchange_rows(int,int);
  void exchange_columns(int,int);
  void permute_rows(permutation*);
  void permute_columns(permutation*);
  float barycenter_row(int);
  int barycenter_row(int,edge_vector*);
  float barycenter_column(int);
  int barycenter_column(int,edge_vector*);
  permutation* sort_rows();
  permutation* sort_columns();
  int connectivity_row(int);
  int connectivity_column(int);

  int symm_crossings();
  int symm_cross_rev_rows(int);

  friend permutation * aset_sort_columns(matrix *, matrix *);
  friend permutation * aset_sort_rows(matrix *, matrix *);
};
/*
   START-LOG-------------------------------------------

   $Log: matrix.h  $
   Revision 1.4 2000/07/10 23:02:47EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2.1.2  1992/10/09  19:03:25  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
