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
//++ELS
#include <if_parser_cbs.h>
#include <astnodeStruct.h>
#include <astnodeList.h>
#include <locationStruct.h>
#include <relationStruct.h>
#include <symbolStruct.h>
#include <attributeList.h>
#include <locationList.h>
#include <maplineList.h>
#include <stringList.h>
#include <transformationList.h>
//--ELS

//++SMT
#include <smt_ifl.h>
//--SMT

extern "C" int sym_insert_CB(unsigned int i, symbolStruct *s)
{
  ifl_sym_insert(i,s);
  return 1;
}

extern "C" int atr_modify_CB(unsigned int i, attributeList *l)
{
  ifl_atr_modify(i, l);
  return 1;
}

extern "C" int rel_create_CB(relationStruct *s)
{
  ifl_rel_create(s);
  return 1;
}

extern "C" int smt_reference_CB(unsigned int i, locationList *l)
{
  ifl_smt_reference(i,l);
  return 1;
}

extern "C" int smt_implicit_CB(unsigned int, locationList *)
{
  return 1;
}

extern "C" int smt_keyword_CB(char *s, locationList *l)
{
  ifl_smt_keyword(s,l);
  return 1;
}

extern "C" int cpp_include_order_CB(stringList *)
{
  return 1;
}

extern "C" int err_message_CB(char *, locationStruct *)
{
  return 1;
}

extern "C" int transform_copy_CB(transformationList *)
{
  return 1;
}

extern "C" int transform_fixed_CB(transformationList *)
{
  return 1;
}

extern "C" int transform_lines_CB(maplineList *)
{
  return 1;
}

extern "C" int ast_report_tree_CB(astnodeList *a)
{
  ifl_ast_report_tree(a);
  return 1;
}

int ifl_smt_file();

extern "C" int smt_file_CB(char *)
{
  ifl_smt_file();
  return 1;
}

extern "C" int iff_parser_preparse_CB(void)
{
  return 1;
}

extern "C" int iff_parser_postparse_CB(void)
{
  return 1;
}











