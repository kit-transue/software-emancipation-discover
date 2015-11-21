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
#include <pdumem.h>
//--OSPORT

//++SEV
#include <reportItem.h>
#include <descParser.h>
#include <cstring.h>
#include <descParser.yacc.h>
#include <reportStream.h>
//--SEV

reportItem::reportItem(char const *_name, char const *_desc_index)
{
  __name         = _name;
  __desc_index   = _desc_index;
  current_stream = 0;
}

reportItem::~reportItem(void)
{
  FREE_MEMORY(__name);
  FREE_MEMORY(__desc_index);
  FREE_OBJECT(current_stream);
}

//
// default method of sending node description to stream
//
int reportItem::parseDesc(char const *desc, reportStream *rs)
{
  int retval = -1;
  
  descParser *dp = 0;
  
  if ( rs ) // we don't check desc, because the parser will know what to do with a nonexistant description
    {
      // get the description parser
      if ( dp = getParser() )
	{
	  // parse the description (will make appropritate callbacks)
	  current_stream = rs;
	  retval = dp->parse(this, desc);
	  current_stream = 0;
	}
    }

  return retval;
}

//
// default implementation returns default description parse
//
descParser *reportItem::getParser(void)
{
  return new descParser;
}

//
// Default callback is to send string to current stream
//
int reportItem::onString(char const *str)
{
  int retval = -1;

  if ( str && current_stream )
    retval = current_stream->add_string(str);

  return retval;
}

//
// By default the reportItem handles the TEXT and CODE tags
// as well as the PARA_END and LINE_END tags
//
int reportItem::onTag(int t)
{
  int retval = -1;

  if ( current_stream )
    {
      switch ( t )
	{
	case TEXT_MODE:
	  {
	    retval = current_stream->set_mode(reportStream::TEXT);
	    break;
	  }
	case CODE_MODE:
	  {
	    retval = current_stream->set_mode(reportStream::CODE);
	    break;
	  }
	case LINE_END:
	  {
	    retval = current_stream->end_line();
	    break;
	  }
	case PARA_END:
	  {
	    retval = current_stream->end_paragraph();
	    break;
	  }
	default:
	  {
	    break;
	  }
	}
    }

  return retval;
}
