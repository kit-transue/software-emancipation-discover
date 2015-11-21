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

//++OSPORT
#include <pdustring.h>
//--OSPORT

//++SEV
#include <queryFolder.h>
#include <reportStream.h>
#include <sectionHeader.h>
#include <reportTemplate.h>
#include <cstring.h>
#include <descParser.yacc.h>
#include <charOpts.h>
//--SEV

queryFolder::queryFolder(char const *_name, const char *_desctag, char const *_score, char const *_weight) 
    : qualityItem(_name, _desctag, _score, _weight, pdstrdup("N/A"), pdstrdup("N/A"), pdstrdup("N/A")),
      reportItemList(16)
{
}

queryFolder::~queryFolder(void)
{
}


int queryFolder::serialize(reportStream *stream, reportTemplate *tagfile)
{
  int retval = -1;

  if ( stream )
    {
      // start section
      sectionHeader sh;
      sh.title = name();
      stream->start_section(&sh);
      
      // send description
      parseDesc(tagfile->getDesc(desc()), stream);
      
      // send children
      int n = numItems();
      for ( int i = 0; i < n; i++ )
	{
	  reportItem *child = getItem(i);
	  if ( child )
	    {
	      child->serialize(stream, tagfile);
	    }
	}
      
      // end section
      stream->end_section(0);
      
      retval = 1;
    }
  
  return retval;
}

int queryFolder::onTag(int t)
{
  int retval = -1;

  if ( current_stream )
    {
      switch ( t )
	{
	case QUALITY_TABLE:
	  retval = current_stream->print_quality_table(this, this);
	  break;
	  
	case QUALITY_GRAPH:
	  retval = current_stream->graph_quality(this, this);
	  break;
	  
	default:
	  {
	    // pass the rest down to base class
	    retval = qualityItem::onTag(t);
	    break;
	  }
	}
    }
  
  return retval;
}

