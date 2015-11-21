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
//++C
#ifndef ISO_CPP_HEADERS
#include <limits.h>
#else /* ISO_CPP_HEADERS */
#include <limits>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C

//++C++
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
//#include <pdumem.h>
//#include <pdustring.h>
//--OSPORT

//++ELS
#include <elsMapper.h>
#include <region.h>
#include <mappedFile.h>
#include <locationStruct.h>
#include <maplineStruct.h>
#include <position.h>
//--ELS

region::region(const location *l)
{
  if ( l && l->filename )
    {
      from_file = mappedFile::getFile(l->filename);
      start = new position(l, position::START);
      end   = new position(l, position::END);
    }
  else
    {
      from_file = 0;
      start     = 0;
      end       = 0;
    }
}

region::region(const mapline *ml, char *filename)
{
  if ( filename && ml )
    {
      from_file = mappedFile::getFile(filename);
      start     = new position(ml, position::START, filename);
      end       = new position(ml, position::END, filename);
    }
  else
    {
      from_file = 0;
      start     = 0;
      end       = 0;
    }
}

region::region(const position *p1, const position *p2)
{
  if ( p1 && p2 && p1->fromFile() == p2->fromFile() )
    {
      start     = new position(*p1);
      end       = new position(*p2);
      from_file = (mappedFile *)(p1->fromFile()); // cast is necessary (and OK) from const mappedFile * to mappedFile *
    }
  else
    {
      start     = 0;
      end       = 0;
      from_file = 0;
    }
}

region::region(const region &other)
{
  from_file = other.from_file;
  start     = new position(*other.start);
  end       = new position(*other.end);
}  

region::~region(void)
{
    delete start;  start = 0;
    delete end;  end = 0;
}

region &region::operator = (const region &other)
{
  if ( this != &other )
    {
      delete start;
      delete end;
        
      from_file = other.from_file;
      start     = new position(*other.start);
      end       = new position(*other.end);
    }

  return *this;
}

region::operator location(void) const
{
  location retval;

  if ( start && end )
    {
      start->toLocation(&retval, position::START);
      end->toLocation(&retval, position::END);
    }

  return retval;
}

region::regionOrder region::orderWith(const region *other) const
{
  regionOrder retval  = INVALID;

  if ( other && (other->fromFile() == fromFile()) )
    {
      if ( *end < *other->start )
	retval = PRECEDES;
      else if ( *start > *other->end )
	retval = FOLLOWS;
      else if  ((*start == *other->start) && (*end  == *other->end) )
	retval = IS_EXACTLY;
      else if ( (*start >= *other->start) && (*end <= *other->end) )
	retval = IS_CONTAINED_IN;
      else if ( (*other->start >= *start) && (*other->end <= *end) )
	retval = CONTAINS;
      else if ( (*start <= *other->start) && (*end <= *other->end) )
	retval = OVERLAPS_INTO;
      else if ( (*start >= *other->start) && (*end >= *other->end) )
	retval = OVERLAPS_OUT_OF;
      else
	retval = UNKNOWN;
    }

  return retval;
}

int region::split(const region *around, region **before, region **after) const
{
  int retval = -1;

  if ( around && before && after )
    {
      // to split regions, they must be from same file
      if ( around->fromFile() == fromFile() )
	{
	  // compute before region
	  if ( *start < *around->start )
	    {
	      position *beforeEnd = around->start->prev();
	      if ( beforeEnd )
		{
		  *before = new region(start, beforeEnd);
		  delete beforeEnd; beforeEnd = 0;
		}
	      else
		*before = 0;
	    }
	  else
	    *before = 0;

	  // compute after region
	  if ( *end > *around->end )
	    {
	      position *afterStart = around->end->next();
	      if ( afterStart )
		{
		  *after = new region(afterStart, end);
		  delete afterStart; afterStart = 0;
		}
	      else
		*after = 0;
	    }
	  else
	    *after = 0;
	  
	  // are these regions valid?
	  if ( *before && ! (*before)->valid() ) {
              delete *before; *before = 0; }
	  if ( *after && ! (*after)->valid() ) {
              delete *after; *after = 0;
          }
	  // if at least one new region was created, we are OK
	  if ( *before || *after )
	    retval = 1;
	}
    }

  return retval;
}

int region::valid(void) const
{
  int retval = 0;

  if ( start && end && from_file )
    if ( start->valid() && end->valid() )
      if ( *end >= *start )
	retval = 1;

  return retval;
}
    
void region::dumpTo(ostream &o) const
{
  start->dumpTo(o);
  o << ' ';
  end->dumpTo(o);
}

