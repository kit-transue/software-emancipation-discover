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
#ifndef _if_parser_cbs_h
#define _if_parser_cbs_h

#include "astnodeStruct.h"
#include "locationStruct.h"
#include "maplineList.h"
#include "relationStruct.h"
#include "stringList.h"
#include "symbolStruct.h"
#include "transformationList.h"

#ifdef __cplusplus
extern "C"
{
#endif
  int sym_insert_CB(unsigned int, symbolStruct *);
  int atr_modify_CB(unsigned int, attributeList *);
  int rel_create_CB(relationStruct *);
  int smt_reference_CB(unsigned int, locationList *);
  int smt_implicit_CB(unsigned int, locationList *);
  int smt_keyword_CB(char *, locationList *);
  int cpp_include_order_CB(stringList *);
  int err_message_CB(char *, locationStruct *);
  int transform_copy_CB(transformationList *);
  int transform_fixed_CB(transformationList *);
  int transform_lines_CB(maplineList *);
  int ast_report_tree_CB(astnodeList *);
  int smt_file_CB(char *);
  int iff_parser_preparse_CB(void);
  int iff_parser_postparse_CB(void);
#ifdef __cplusplus
}
#endif

extern int build_map;

#endif

