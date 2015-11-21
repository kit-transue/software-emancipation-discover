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
//#include <pdustring.h>
//#include <pdumem.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <locationStruct.h>
#include <mappedFile.h>
#include <mappingList.h>
#include <mappingType.h>
#include <region.h>
#include <mapping.h>
//--ELS

////////////////////////////////////////////////////////////////////////////////
//
// Public methods
//
////////////////////////////////////////////////////////////////////////////////

mappingList::mappingList(int n) : parray(n)
{
}

mappingList::~mappingList(void)
{
  for ( int i = size() - 1; i >= 0; i-- )
    {
      mapping *m = (*this)[i];
      delete m;
      m = 0;
    }
}

int mappingList::insert(mapping *m)
{
  int retval = -1;

  if ( m )
    {
      int low  = 0;
      int high = length() - 1;
      int done = 0;

      if ( high == -1 ) // i.e. length == 0
	{
	  parray::insert((void *)m);
	  retval = 1;
	  done = 1;
	}

      while ( ! done )
	{
	  int mid = (low + high) / 2;
	  mapping *mid_elem = (*this)[mid];
	  region::regionOrder ord = m->orderWith(mid_elem);

	  switch ( ord )
	    {
	    case region::PRECEDES:
	      {
		high = mid - 1;
		if ( high < low )
		  {
		    parray::insert((void *)m, low);
		    done = 1;
		    retval = 1;
		  }
		break;
	      }
	      
	    case region::FOLLOWS:
	      {
		low = mid + 1;
		if ( low > high )
		  {
		    parray::insert((void *)m, high + 1);
		    done = 1;
		    retval = 1;
		  }
	      break;
	      }

	    case region::IS_EXACTLY:
	      {
		if ( mid_elem->replacementAllowed(m) )
		  {
                    delete mid_elem; mid_elem = 0;
		    remove(mid);
		    parray::insert((void *)m, mid);
		    retval = 1;
		  }
		else {
                    delete m; m = 0; }
		done = 1;
		break;
	      }
	      
	    case region::CONTAINS:
	      {
		mapping *before = 0;;
		mapping *after  = 0;
		mapping *over   = 0;
		if ( m->split(mid_elem, &before, &after, &over) >= 0 )
		  {
		    if ( before )
		      insert(before);
		    if ( after )
		      insert(after);
		    if ( over )
		      insert(over);
		    retval = 1;
		  }
		delete m; m = 0;
		done = 1;
		break;
	      }

	    case region::IS_CONTAINED_IN:
	    case region::OVERLAPS_OUT_OF:
	    case region::OVERLAPS_INTO:
	      {
		mapping *before = 0;
		mapping *after  = 0;
		mapping *over   = 0;
		if ( mid_elem->split(m, &before, &after, &over) >= 0 )
		  {
                    delete mid_elem; mid_elem = 0;
		    remove(mid);
		    parray::insert((void *)over, mid);
		    if ( before )
		      insert(before);
		    if ( after )
		      insert(after);
		    insert(m);
		    retval = 1;
		  }
		else {
                    delete m; m = 0; }
		done = 1;
		break;
	      }

	    case region::UNKNOWN:
	      {
		mappingError(mid_elem, m, NO_FILE_INCONSISTANT_REPORTING);
		delete m; m = 0;
		done = 1;
		break;
	      }

	    case region::INVALID:
	    default:
	      {
		mappingError(mid_elem, m, INVALID_COMPARASON_ATTEMPTED);
		delete m; m = 0;
		done = 1;
		break;
	      }
	    }

	}
    }

  return retval;
}

void mappingList::mappingError(const mapping *m1, const mapping *m2, errorReason r)
{
  if ( m2 && m1 )
    {
      cerr << "Mapping Error: ";
      switch ( r )
	{
	case NO_REPLACEMENT:
	  cerr << "cannot replace mapping:" << '\n' << '\t';
	  m1->dumpTo(cerr);
	  cerr << '\n';
	  cerr << "with mapping:" << '\n' << '\t';
	  m2->dumpTo(cerr);
	  cerr << '\n';
	  break;
	  
	case NO_CONTAINS:
	  cerr << "cannot insert mapping:" << '\n' << '\t';
	  m2->dumpTo(cerr);
	  cerr << '\n';
	  cerr << "into mapping:" << '\n' << '\t';
	  m1->dumpTo(cerr);
	  cerr << '\n';
	  break;
	      
	case NO_IS_CONTAINED_IN:
	  cerr << "cannot insert mapping:" << '\n' << '\t';
	  m1->dumpTo(cerr);
	  cerr << '\n';
	  cerr << "into mapping:" << '\n' << '\t';
	  m2->dumpTo(cerr);
	  cerr << '\n';
	  break;
	      
	case NO_OVERLAPS_INTO:
	  cerr << "mappings overlap:" << '\n' << '\t';
	  m1->dumpTo(cerr);
	  cerr << '\n';
	  cerr << "into mapping:" << '\n' << '\t';
	  m2->dumpTo(cerr);
	  cerr << '\n';
	  break;
	      
	case NO_OVERLAPS_OUT_OF:
	  cerr << "mappings overlap:" << '\n' << '\t';
	  m1->dumpTo(cerr);
	  cerr << '\n';
	  cerr << "out of mapping:" << '\n' << '\t';
	  m2->dumpTo(cerr);
	  cerr << '\n';
	  break;
	      
	case NO_FILE_INCONSISTANT_REPORTING:
	  cerr << "was unable to compare mappings:" << '\n' << '\t';
	  m1->dumpTo(cerr);
	  cerr << "and" << '\n' << '\t';
	  cerr << '\n';
	  m2->dumpTo(cerr);
	  cerr << "without access to files." << '\n';
	  break;
	      
	case INVALID_COMPARASON_ATTEMPTED:
	  cerr << "invalid comparasion of mappings attempted:" << '\n' << '\t';
	  m1->dumpTo(cerr);
	  cerr << '\n';
	  cerr << "and" << '\n' << '\t';
	  m2->dumpTo(cerr);
	  cerr << '\n';
	  break;
	  
	default:
	  cerr << "*ERROR* Invalid error code passed to error routine!" << '\n';
	  break;
	}
    }
}

void mappingList::dumpTo(ostream &o)
{
  o << "MAPPING LIST:\n";

  int n = length();
  for ( int i = 0; i < n; i++ )
    {
      o << "   [" << i << "]: ";
      (*this)[i]->dumpTo(o);
      o << "\n";
    }
}
  
int mappingList::extract(region *within, region **result)
{
  int retval = -1;

  if ( within && result )
    {
      *result = 0;

      int low  = 0;
      int high = length() - 1;
      int done = 0;

      while ( ! done )
	{
	  int mid                 = (low + high) / 2;
	  mapping *mid_elem       = (*this)[mid];
	  region::regionOrder ord = within->orderWith(mid_elem);

	  if ( ord == region::IS_EXACTLY || ord == region::IS_CONTAINED_IN )
	    {
	      *result = mid_elem->translate(within);
	      done = 1;
	    }
	  else if ( ord == region::PRECEDES )
	    {
	      high = mid - 1;
	      if ( high < low )
		done = 1;
	    }
	  else if ( ord == region::FOLLOWS )
	    {
	      low = mid + 1;
	      if ( low > high )
		done = 1;
	    }
	  else
	    {
	      done = 1;
	    }
	}
      
      if ( *result )
	retval = 1;
    }
  
  return retval;
}
  
const position *mappingList::getFileBoundary(char *filename)
{
  const position *retval = 0;

  if ( filename )
    {
      for ( int i = size() - 1; i >= 0 && ! retval; i-- )
	{
	  mapping *m = (*this)[i];
	  if ( m )
	    {
	      const region *src = m->getSource();
	      if ( src )
		{
		  const mappedFile *f = src->fromFile();
		  if ( f )
		    {
		      char *from = f->filename();
		      if ( from )
			{
			  // we know that "from" is from the fileCache mechanism, so
			  // the other better be as well
			  if ( filename == from )
			    {
			      retval = m->getEnd();
			    }
			}
		    }
		}
	    }
	}
    }

  return retval;
}
