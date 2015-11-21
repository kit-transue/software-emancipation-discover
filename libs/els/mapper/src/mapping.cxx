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
//++C++
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
//#include <pdumem.h>
#include <parray.h>
//--OSPORT

//++ELS
#include <mapping.h>
#include <region.h>
#include <maplineStruct.h>
#include <position.h>
#include <mappedFile.h>
//--ELS

mapping::mapping(const region *src, const region *trg, mappingType t) : region(*trg)
{
  source = new region(*src);
  mapType = t;
}

mapping::mapping(const location *src, const location *trg, mappingType t) : region(trg)
{
  source = new region(src);
  mapType = t;
}

mapping::mapping(const mapline *ml) : region(ml, ml->targetFile)
{
  source = new region(ml, ml->sourceFile);
  mapType = LINES;
}

mapping::~mapping(void)
{
  delete source;
}

mapping::mapping(const mapping &other) : region(other)
{
  source  = new region(*other.source);
  mapType = other.mapType;
}

mapping &mapping::operator = (const mapping &other)
{
  if ( this != &other )
    {
      region::operator = (other);
      delete source;
      source  = new region(*other.source);
      mapType = other.mapType;
    }
  
  return *this;
}

int mapping::split(const region *around, mapping **before, mapping **after, mapping **over) const
{
  int retval = -1;

  if ( around && before && after && over )
    {
      // to split regions, they must be from same file
      if ( around->fromFile() == fromFile() )
	{
	  // split myself as a region
	  region *trg_before = 0;
	  region *trg_after  = 0;
	  region::split(around, &trg_before, &trg_after);

	  // determine what parts of the mappings overlap
	  // either completely, or partially
	  const position *over_start = 0;
	  const position *over_end   = 0;
	  region         *trg_over   = 0;
	  if ( *around->getEnd() > *end )
	    over_end = end;
	  else
	    over_end = around->getEnd();
	  if ( *around->getStart() < *start )
	    over_start = start;
	  else
	    over_start = around->getStart();
	  trg_over = new region(over_start, over_end);

	  // translate those into source regions
	  region *src_before = 0;
	  region *src_after  = 0;
	  region *src_over   = 0;
	  if ( mapType == FIXED )
	    src_before = src_after = src_over =  source; // fixed mapping will not be split
	  else
	    {
	      if ( trg_before )
		src_before = translate(trg_before);
	      if ( trg_after )
		src_after = translate(trg_after);
	      if ( trg_over )
		src_over = translate(trg_over);
	    }

	  // make sure that everything is OK
	  int error = 1;

	  if ( trg_before && src_before )
	    if ( (*before = new mapping(src_before, trg_before, mapType)) != 0 )
	      error = 0;
	    else
	      *before = 0;
	  else
	    *before = 0;

	  if ( src_after && trg_after )
	    if ( (*after = new mapping(src_after, trg_after, mapType)) != 0 )
	      error = 0;
	    else
	      *after = 0;
	  else 
	    *after = 0;

	  if ( src_over && trg_over )
	    if ( (*over = new mapping(src_over, trg_over, mapType)) != 0 )
	      error = 0;
	    else
	      *over = 0;
	  else 
	    *over = 0;

	  // check for errors
	  if ( ! error )
	    retval = 1;

	  // free memory
	  delete trg_before;  trg_before = 0;
	  delete trg_after;  trg_after = 0;
	  delete trg_over;  trg_over = 0;
	  if ( mapType != FIXED )
	    {
              delete src_before;   src_before = 0;
	      delete src_after;  src_after = 0;
	      delete src_over;  src_over = 0;
	    }
	}
    }

  return retval;
}

region *mapping::translate(const region *r) const
{
  region *retval = 0;

  if ( r )
    {
      if ( mapType != FIXED )
	{
	  position *x_start = translate(r->getStart());
	  position *x_end   = translate(r->getEnd());
	  
	  if ( x_start && x_end )
	    retval = new region(x_start, x_end);
	  
	  delete x_start;  x_start = 0;
	  delete x_end;  x_end = 0;
	}
      else
	{
	  retval = new region(*source);
	}
    }

  return retval;
}

position *mapping::translate(const position *p) const
{
  position *retval = 0;
  
  if ( mapType == LINES || mapType == COPY)  
    {
      position tmp(*p);

	retval = tmp.offsetWith(getStart(), source->getStart());

    }

  return retval;
}
	      
void mapping::dumpTo(ostream &o) const
{
  o << "MAP ";
  switch ( mapType )
    {
    case COPY:
      o << "copy ";
      break;
    case FIXED:
      o << "fixed ";
      break;
    case LINES:
      o << "lines ";
      break;
    }
  o << '"';
  if ( source )
    if ( source->fromFile() )
      {
	char *filename = source->fromFile()->filename();
	if ( filename )
	  o << filename;
      }
  o << "\" \"";
  if ( from_file )
    {
      char *filename = from_file->filename();
      if ( filename )
	o << filename;
    }
  o << "\" { ";
  source->dumpTo(o);
  o << " , ";
  region::dumpTo(o);
  o << " }";
}

int mapping::splitAllowed(const mapping *other) const
{
  int retval = 0;

  if ( other )
    if ( other->source )
      {
	switch ( mapType )
	  {
	  case LINES:
	    retval = 1;
	    break;
	  case FIXED:
	  case COPY:
	    if ( other->mapType == FIXED || other->mapType == COPY )
	      retval = 1;
	    break;
	  }
      }
  
  return retval;
}

int mapping::replacementAllowed(const mapping *other) const
{
  int retval = 0;

  if ( other )
    if ( other->source )
      {
	switch ( mapType )
	  {
	  case LINES:
	    retval = 1;
	    break;
	  case FIXED:
	    if ( other->mapType == FIXED || other->mapType == COPY )
	      retval = 1;
	    break;
	  case COPY:
	    if ( other->mapType == COPY )
	      retval = 1;
	    break;
	  }
      }

  return retval;
}
