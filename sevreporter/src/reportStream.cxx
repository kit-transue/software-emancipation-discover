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
// REPORT STREAM
//
////////////////////////////////////////////////////////////////////////////////

//++OSPORT
#include <pdustring.h>
#include <pdumem.h>
#include <bufFile.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++SEV
#include <reportStream.h>
//--SEV

//
// Class statics
//

int const reportStream::max_section_depth = 1024;

//
// create structures for section nesting and depth
//

reportStream::reportStream(void)
{
  mode                   = TEXT;
  section_depth          = 0;
  last_was_section_start = 0;
  section_counters       = new int[max_section_depth];
  for ( int i = 0; i < max_section_depth; i++ )
    section_counters[i]  = 1;
  section_counters[0]    = 0;
}

reportStream::~reportStream(void)
{
  delete [] section_counters;
}

////////////////////////////////////////////////////////////////////////////////
//
// Method implementation
//
////////////////////////////////////////////////////////////////////////////////

//
// Switch modes
//

reportStream::textmode reportStream::set_mode(textmode m)
{
  textmode retval = mode;
  mode = m;
  return retval;
}

//
// Put a character onto the stream
//

int reportStream::add_char(char c)
{
  char s[2];
  s[0] = c;
  s[1] = 0;

  return add_string(s);
}

//
// Put a string onto the stream
//

int reportStream::add_string(char const *str)
{
  int retval = -1;
  
  char *copy = pdstrdup(str);
  
  if ( copy )
    {
      char temp   = 0;
      char *start = copy;
      char *end   = copy;

      while ( *start )
	{
	  // find continuous sequence of like characters
	  while ( *end && identify_type(*start) == identify_type(*end) )
	    end++;
	  
	  // terminate string
	  temp = *end;
	  *end = 0;
	  
	  // report string
	  if ( identify_type(*start) == WHITESPACE )
	    handle_whitespace(mode, start);    // pure virt implemented in derived class
	  else
	    handle_text(mode, start);          // pure virt implemented in derived class
	  
	  // move up for next iteration
	  *end  = temp;
	  start = end;
	}
      retval = 1;
    }

  FREE_MEMORY(copy);
  return retval;
}

//
// handle new section counter
//

int reportStream::start_of_section(void)
{
  int retval = -1;

  if ( section_counters )
    {
      if ( last_was_section_start )
	{
	  if ( ++section_depth >= max_section_depth )
	    section_depth = max_section_depth - 1;
	  section_counters[section_depth] = 1;
	}
      else
	section_counters[section_depth]++;
      
      last_was_section_start = 1;
      retval = 1;
    }

  return retval;
}

//
// handle end of section counter
//

int reportStream::end_of_section(void)
{
  int retval = -1;

  if ( section_counters )
    {
      if ( ! last_was_section_start )
	{
	  section_counters[section_depth] = 1;
	  section_depth--;
	  if ( section_depth < 0 )
	    section_depth = 0;
	}
      last_was_section_start = 0;
    }
  
  return retval;
}

//
// requests for section section_depth
//

int reportStream::get_section_depth(void)
{
  int retval = -1;

  if ( section_counters )
    retval = section_depth;
  
  return retval;
}


//
// request for number for section x
//

int reportStream::get_section_number(int x)
{
  int retval = -1;

  if ( section_counters )
    if ( x >= 0 && x < max_section_depth )
      retval = section_counters[x];

  return retval;
}

//
// definition of whitespace chars
//

reportStream::texttype reportStream::identify_type(char c)
{
  texttype retval = NONWHITESPACE;

  if ( c <= 32 || c >= 128 )
    retval = WHITESPACE;

  return retval;
}

