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
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "edgeSet.h"

set::set(int s, int status) {
  size = (s + INTLENGTH - 1) / INTLENGTH;
  bitvector = new int[size];
  for (int c=0; c<size; c+=1) {
    bitvector[c] = status;
  }
}

set::set(set* s) {
  size = s->size;
  bitvector = new int[size];
  for (int i=0; i<size; i++) {
    bitvector[i] = s->bitvector[i];
  }
}

void set::op_and(set* s) {
  for (int i=0; i<size; i++) {
    bitvector[i] &= s->bitvector[i];
  }
}

void set::op_or(set* s) {
  for (int i=0; i<size; i++) {
    bitvector[i] |= s->bitvector[i];
  }
}

int set::disjoint(set* s) {
  int d = 0;
  for (int i=0; i<size; i++) {
    d &= (bitvector[i] & s->bitvector[i]);
  }
  return !d;
}

int set::includes(set* s) {
  int in = 0;
  for (int i=0; i<size; i++) {
    in |= (bitvector[i] ^ FULL) & s->bitvector[i]; 
  }
  return !in;
}

int set::limits(set* s) {
  int l = 1;
  for (int i=0; i<size; i++) {
    l &= (bitvector[i] - 1 == s->bitvector[i]);
    if ((!l) || (bitvector[i])) { return l; }
  }
  return 0;
}



set* set_variables::give() {
  set* new_set = new set(&var);
  for (int i=0; i<var.size; i++) {
    if (var.bitvector[i] > 0) {
      var.bitvector[i] <<= 1;
      break;
    }
    else if (var.bitvector[i] < 0) {
      var.bitvector[i] = 0x0;
      var.bitvector[i+1] = 0x1;
      break;
    }
  }
  return new_set;
}
