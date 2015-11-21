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
#ifndef _CONSTR_MANAGER_H
#define _CONSTR_MANAGER_H

class constr_descriptor;

class constr_manager {

  constr_queue constraints;
  dimension dim;
  constr_net constr_1D;
  int consistent;

 public:

  constr_manager();
  ~constr_manager();
  void set_dimension(dimension);

  int equal      (constr_descriptor*, mode);
  int smaller    (constr_descriptor*, mode);
  int greater    (constr_descriptor*, mode);
  int neighbors  (constr_descriptor*, mode);
  int low_margin (constr_descriptor*, mode);
  int high_margin(constr_descriptor*, mode);
  int range      (constr_descriptor*, mode);
  int cluster    (constr_descriptor*, mode);
  int limit      (constr_descriptor*, mode);

  int consistency();

  int delete_constraint(char* name, constr_type type, nlist* nl, int i[2], 
			int d, int prio,
			int name_f, int type_f, int nl_f, int i_f, 
			int d_f, int prio_f);
  void delete_constraints(constr_type, node* = 0);

  void output(ostream &to, int indent);
};

#endif
