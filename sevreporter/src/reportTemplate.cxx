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
////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////

//++C
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C

//++OSPORT
#include <pdumem.h>
#include <pdufile.h>
#include <pdustring.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++SEV
#include <reportTemplate.h>
#include <reportTemplateParser.h>
//--SEV

reportTemplate::reportTemplate(void)
{
  assocs = 0;
}

reportTemplate::~reportTemplate(void)
{
  if ( assocs )
    {
      for ( int i = assocs->size() - 1; i >= 0; i-- )
	{
	  assoc *a = (assoc *)(*assocs)[i];
	  FREE_MEMORY(a->tag);
	  FREE_MEMORY(a->text);
	  FREE_MEMORY(a);
	}
      FREE_OBJECT(assocs);
    }
}

int reportTemplate::read_file(char const *file)
{
  int retval = -1;

  if ( file )
    retval = reportTemplateParse((char *)file, &assocs);

  return retval;
}

char const *reportTemplate::getDesc(char const *tag)
{
  char const *retval = 0;

  if ( tag && assocs )
    {
      for ( int i = 0; i < assocs->size(); i++ )
	{
	  assoc *a = (assoc *)(*assocs)[i];
	  if ( a )
	    if ( pdstrcmp(tag, a->tag) == 0 )
	      retval = a->text;
	}
    }

  return retval;
}
