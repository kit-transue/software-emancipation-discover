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
#ifndef _SMT_IFL_H_
#define _SMT_IFL_H_

int ifl_sym_insert(unsigned int, symbolStruct*);
int ifl_rel_create(relationStruct*);
int ifl_smt_reference(unsigned int,locationList *);
int ifl_smt_keyword(char const*,locationList *);
int ifl_ast_report_tree(astnodeList *);
int ifl_atr_modify(unsigned int i, attributeList *l);

enum IF_rel {
  IF_REL_OTHER = 0,
  IF_REL_CONTEXT,
  IF_REL_CALL,
  IF_REL_READ,
  IF_REL_WRITE,
  IF_REL_TYPE,
  IF_REL_ARGUMENT,
  IF_REL_ENUMERATE,
  IF_REL_SUBCLASS,
  IF_REL_FRIEND,
  IF_REL_CATCH,
  IF_REL_THROW,
  IF_REL_SPECIALIZE,
  IF_REL_INCLUDE,
  IF_REL_DEFINE,
  IF_REL_EXPAND,
  IF_REL_IMPORT,
  IF_REL_CHECKSUM,
  IF_REL_NUM_RELATIONS
};

#endif



