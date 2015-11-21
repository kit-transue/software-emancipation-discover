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
#ifndef _SET_H
#define _SET_H

#define EMPTY 0
#define FULL  0xFFFFFFFF
#define INTLENGTH 32

class set {
  int* bitvector;            // representation of set elements;
  int size;                  // number of bitvectors of length INTLENGTH;
 public:
  set(int s, int status);    // create a set of s elements;
  set(set*);                 // create a set as a copy of another;
  ~set() { delete bitvector;}  

  // set operators;
  void op_and(set* s);          // intersection with set s
  void op_or(set* s);           // union with set s;

  // predicates;
  int disjoint(set* s);      // true if disjoint with set s;
  int includes(set* s);      // true if set s is included;
  int limits(set* s);        // true if s includes exactly all elements
                             // that are  'smaller' (in the bitvector) than 
                             // the smallest element;

  friend class set_variables;
};

class set_variables {
  set var;                   // current set variable generated;
 public:
  set_variables(int s)       // create a set variabe generator for sets with
                             // s elements;
  : var(s,EMPTY) { var.bitvector[0] = 0x1; }

  ~set_variables() {}
  set* give();               // return new set variable;
};

#endif
