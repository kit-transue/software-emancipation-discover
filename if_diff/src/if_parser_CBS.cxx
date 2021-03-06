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
#include <if_parser_cbs.h>
#include <elsMapper.h>
//--ELS

int build_map = 0;;

extern "C" int ast_report_tree_CB(astnodeList *)
{
  return 1;
}

extern "C" int atr_modify_CB(unsigned int, attributeList *)
{
  return 1;
}

extern "C" int err_message_CB(char *, location *)
{
  return 1;
}

extern "C" int rel_create_CB(relation *)
{
  return 1;
}

extern "C" int smt_file_CB(char *)
{
  return 1;
}

extern "C" int smt_implicit_CB(unsigned int, locationList *)
{
  return 1;
}

extern "C" int smt_keyword_CB(char *, locationList *)
{
  return 1;
}

extern "C" int smt_reference_CB(unsigned int, locationList *)
{
  return 1;
}

extern "C" int sym_insert_CB(unsigned int, symbol *)
{
  return 1;
}
	
extern "C" int transform_copy_CB(transformationList *l)
{
  int retval = 1;

  if ( build_map )
    {
      if ( l )
	{
	  int n = l->length();
	  for ( int i = 0; i < n && retval > 0; i++ )
	    if ( elsMapper::addTransformation((*l)[i], 1) < 0 )
	      retval = -1;
	}
      else
	retval = -1;
    }

  return retval;
}

extern "C" int transform_fixed_CB(transformationList *l)
{
  int retval = 1;

  if ( build_map )
    {
      if ( l )
	{
	  int n = l->length();
	  for ( int i = 0; i < n && retval > 0; i++ )
	    if ( elsMapper::addTransformation((*l)[i], 0) < 0 )
	      retval = -1;
	}
      else
	retval = -1;
    }

  return retval;
}
  
extern "C" int transform_lines_CB(maplineList *l)
{
  int retval = 1;

  if ( build_map )
    {
      if ( l )
	{
	  int n = l->length();
	  for ( int i = 0; i < n && retval > 0; i++ )
	    if ( elsMapper::addTransformation((*l)[i]) < 0 )
	      retval = -1;
	}
      else
	retval = -1;
    }

  return retval;
}

extern "C" int cpp_include_order_CB(stringList *)
{
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
