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
//++OSPORT
#include <pdumem.h>
//--OSPORT

//++ELS
#include <locationStruct.h>
#include <relationStruct.h>
#include <symbolStruct.h>
#include <astnodeList.h>
#include <attributeList.h>
#include <locationList.h>
#include <maplineList.h>
#include <stringList.h>
#include <transformationList.h>
#include <uintList.h>
#include <elsMapper.h>
#include <if_parser_cbs.h>
//--ELS

//++IF_EXTRACTOR
#include <ifExtractor.h>
//--IF_EXTRACTOR

extern "C" int ast_report_tree_CB(astnodeList *a)
{
  int retval = -1;

  if ( ifExtractor::addAst(a) >= 0 )
    retval = 0;

  return retval;
}

extern "C" int atr_modify_CB(unsigned int i, attributeList *l)
{
  int retval = -1;

  if ( l )
    if ( ifExtractor::addAtr(i, l) >= 0 )
      {
	FREE_OBJECT(l);
	retval = 0;
      }

  return retval;
}

extern "C" int err_message_CB(char *s, location *l)
{
  int retval = -1;

  if ( s && l )
    {
      if ( ifExtractor::addErr(s, l) >= 0 )
	retval = 0;
    }

  return retval;
}

extern "C" int rel_create_CB(relation *r)
{
  int retval = -1;

  if ( r )
    if ( ifExtractor::addRel(r) >= 0 )
      retval = 1;  // addRel will copy what it wants out of the rel, so let the parser destroy it

  return retval;
}

extern "C" int smt_file_CB(char *)
{
  return 1;
}

extern "C" int smt_implicit_CB(unsigned int i, locationList *l)
{
  int retval = -1;

  if ( l )
    if ( ifExtractor::addSmt(i, 1, l) >= 0 )
      retval = 0;     

  return retval;
}

extern "C" int smt_keyword_CB(char *kwd, locationList *l)
{
  int retval = -1;
  
  if ( l )
    if ( ifExtractor::addKeyword(kwd, l) >= 0 )
      retval = 0;
  
  return retval;
}

extern "C" int smt_reference_CB(unsigned int i, locationList *l)
{
  int retval = -1;

  if ( l )
    if ( ifExtractor::addSmt(i, 0, l) >= 0 ) 
      retval = 0;

  return retval;
}

extern "C" int sym_insert_CB(unsigned int i, symbol *s)
{
  int retval = -1;

  if ( s )
    if ( ifExtractor::addSym(i, s) >= 0 )
      retval = 0;

  return retval;
}
	
extern "C" int transform_copy_CB(transformationList *l)
{
  int retval = 1;
  
  if ( l )
    {
      int n = l->length();
      for ( int i = 0; i < n && retval > 0; i++ )
	if ( elsMapper::addTransformation((*l)[i], 1) < 0 )
	  retval = -1;
    }
  else
    retval = -1;

  return retval;
}

extern "C" int transform_fixed_CB(transformationList *l)
{
  int retval = 1;
  
  if ( l )
    {
      int n = l->length();
      for ( int i = 0; i < n && retval > 0; i++ )
	if ( elsMapper::addTransformation((*l)[i], 0) < 0 )
	  retval = -1;
    }
  else
    retval = -1;

  return retval;
}
  
extern "C" int transform_lines_CB(maplineList *l)
{
  int retval = 1;
  
  if ( l )
    {
      int n = l->length();
      for ( int i = 0; i < n && retval > 0; i++ )
	if ( elsMapper::addTransformation((*l)[i]) < 0 )
	  retval = -1;
    }
  else
    retval = -1;

  return retval;
}

extern "C" int cpp_include_order_CB(stringList *)
{
  return 1;
}

extern "C" int iff_parser_preparse_CB(void)
{
  return elsMapper_reset();
}

extern "C" int iff_parser_postparse_CB(void)
{
  return 1;
}
