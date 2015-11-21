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

#include <iostream>
#include <set>
using namespace std;

#include "maplineStruct.h"
#include "transformationStruct.h"
#include "elsMapper.h"
#include "locationStruct.h"
#include "locationList.h"
#include "position.h"
#include "region.h"
#include "mapping.h"
#include "mappingList.h"
#include "mappingType.h"
#include "mappedFile.h"


//++DEBUG
#if defined(DEBUG_ELSMAPPING)
#include <iostream.h>
#endif
//--DEBUG

// initialization of statics
typedef set<elsMapper *> MapperSet;
static MapperSet *currentMaps  = 0;
int      elsMapper::showbadlocs    = 0;
int      elsMapper::showunmappings = 0;

////////////////////////////////////////////////////////////////////////////////
//
// protected constructors (no public ones)
//
////////////////////////////////////////////////////////////////////////////////

// this constructor should never be called, but it needs to
// be defined, as it has been declared in the "protected"
// section, so as to instruct the compiler not to generate
// a default, public, empty contructor
elsMapper::elsMapper(void) : forFile(0)
{
}

elsMapper::elsMapper(const char *filename)
{
  // store the filename
  forFile = filename;

  //
  // add itself to the static array of map objects
  //
  if ( ! currentMaps ) {
      // if this is the first, create the currentMaps array
      currentMaps = new MapperSet;
  }
  currentMaps->insert(this);

  // init list of mappings for this object
  mappings = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// public destructor
//
////////////////////////////////////////////////////////////////////////////////

elsMapper::~elsMapper(void)
{
  // remove myself from the list of current maps
  if ( currentMaps )
    {
      currentMaps->erase(this);
      if ( currentMaps->size() == 0 ) {
          delete currentMaps;  currentMaps = 0; }
    }

  // destroy my mapping list
  delete mappings;  mappings = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// public static methods
//
////////////////////////////////////////////////////////////////////////////////


//
// This method us used to add the informtion from a transformtion struct into
// the mapping information.  It is 1 of 2 ways to add information to this list
//
// retvals: -1 error, 1 success
//
int elsMapper::addTransformation(const transformationStruct *trans, int copyFlag)
{
  int           retval  = -1;
  elsMapper    *mapper  = 0;

  if ( trans && trans->source && trans->target )
    {
      if ( (mapper = findMapper(trans->target->filename)) == 0 )
	mapper = new elsMapper(trans->target->filename);
      
      if ( mapper )
	{
	  // Adjust the end of the target, if it's not given
	  if ( trans->target->end_line == 0 && trans->target->end_column == 0 )
	    {
	      position *srcStart = new position(trans->source, position::START);
	      position *srcEnd   = new position(trans->source, position::END);
	      position *trgStart = new position(trans->target, position::START);
	      position *trgEnd   = 0;
	      if ( srcStart && srcEnd && trgStart )
		if ( (trgEnd = srcEnd->offsetWith(srcStart, trgStart)) != 0 )
		  trgEnd->toLocation(trans->target, position::END);
	      delete srcStart;  srcStart = 0;
	      delete srcEnd;  srcEnd = 0;
	      delete trgStart;  trgStart = 0;
	      delete trgEnd;  trgEnd = 0;
	    }

	  mapping *newEntry = new mapping(trans->source, trans->target, copyFlag ? COPY : FIXED);
	  if ( newEntry )
	    if ( ! mapper->mappings )
	      {
		if ( mapper->mappings = new mappingList(1024) )
		  {
		    mapper->mappings->insert(newEntry);
		    retval = 1;
		  }
	      }
	    else
	      {
		mapper->mappings->insert(newEntry);
		retval = 1;
	      }
	}
    }

  return retval;
}

//
// This method us used to add the informtion from a map line struct into
// the mapping information.  It is 1 of 2 ways to add information to this list
//
// retvals: -1 error, 1 success
//
int elsMapper::addTransformation(const maplineStruct *ml)
{
  int           retval    = -1;
  elsMapper    *mapper    = 0;

  if ( ml && ml->sourceFile && ml->targetFile )
    {
      if ( (mapper = findMapper(ml->targetFile)) == 0 )
	mapper = new elsMapper(ml->targetFile);

      if ( mapper )
	{
	  mapping *newEntry = new mapping(ml);
	  if ( newEntry )
	    if ( ! mapper->mappings )
	      {
		if ( mapper->mappings = new mappingList(1024) )
		  {
		    mapper->mappings->insert(newEntry);
		    retval = 1;
		  }
	      }
	    else
	      {
		mapper->mappings->insert(newEntry);
		retval = 1;
	      }
	}
    }

  return retval;
}

//
// This method is used to discard all mapping information currently gathered
// mainly used when resetting the parser to start anew.
//
int elsMapper::reset(void)
{
  int retval = 1;

  // free all the mapping information
  if ( currentMaps ) {
      while (!currentMaps->empty()) {
          delete *currentMaps->begin();  // beware invalidation: removes self from map!
      }
      delete currentMaps;  currentMaps = 0;
  }
 
  // free all the mapped files
  mappedFile::reset();

  return retval;
}

//
// C interface to above function
//

extern "C" int elsMapper_reset(void)
{
  return elsMapper::reset();
}

//
// this method is used to answer the question:
// "the mapped location l (filename, position tuple) came originally from what location?
//
//
int elsMapper::getTrueLoc(const location *mappedLoc, location **result)
{
  int retval = -1;

  if ( mappedLoc && result )
    {
      if ( *result = new location )
	{
	  // initialize the result
	  (*result)->start_line   = 0;
	  (*result)->start_column = 0;
	  (*result)->end_line     = 0;
	  (*result)->end_column   = 0;
	  (*result)->filename     = 0;

	  // get a region class that represents the single point that
	  // is the start of the region, and a second one that is the end point
	  // (if there is an end point)
	  position p1(mappedLoc, position::START);
	  position p2(mappedLoc, position::END);
	  region *startpoint = 0;
	  region *endpoint   = 0;
	  if ( p1.valid() )
	    startpoint = new region(&p1, &p1);
	  if ( p2.valid() )
	    endpoint = new region(&p2, &p2);

	  // get temp locations for the start and end points from helper fctn
	  location *startlocpoint = 0;
	  location *endlocpoint = 0;

	  // do unmapping if we at least have a start point
	  if ( startpoint )
	    {
	      // unmap the start position
	      if ( elsMapper::getTrueLoc2(startpoint, &startlocpoint) >= 0 )
		{
		  (*result)->start_line   = startlocpoint->start_line;
		  (*result)->start_column = startlocpoint->start_column;
		  (*result)->filename     = startlocpoint->filename;
		  retval = 1;
		}
	      
	      // only unmap the end if it's not null(that's not an error)
	      if ( endpoint )
		{
		  retval = -1;
		  if ( elsMapper::getTrueLoc2(endpoint, &endlocpoint) >= 0 )
		    {
		      (*result)->end_line   = endlocpoint->end_line;
		      (*result)->end_column = endlocpoint->end_column;
		      retval = 1;
		    }
		}
	    }
	  delete startlocpoint;  startlocpoint = 0;
	  delete endlocpoint;  endlocpoint = 0;
	  delete startpoint;  startpoint = 0;
	  delete endpoint;  endpoint = 0;
	  if ( retval != 1 ) {
              delete *result;  *result = 0;}
	}
    }

  if ( retval != 1 && result )
    *result = 0;

  if ( showunmappings )
    {
      if ( mappedLoc )
	{
	  ((location *)mappedLoc)->print(cout,1);
	  cout << " ==> ";
	  if ( result && *result )
	    (*result)->print(cout,1);
	  cout << '\n';
	}
    }

  if ( showbadlocs )
    {
      if ( result && *result )
	{
	  position start(*result, position::START);
	  if ( ! start.exists() )
	    {
	      cout << "Location invalid: (start bad) ";
	      (*result)->print(cout,1);
	      cout << " unmapped from ";
	      ((location *)mappedLoc)->print(cout, 1);
	      cout << '\n';
	    }
	  if ( (*result)->end_line != 0 && (*result)->end_column != 0 )
	    {
	      position end(*result, position::END);
	      if ( ! end.exists() )
		{
		  cout << "Location invalid: (end bad)   ";
		  (*result)->print(cout,1);
		  cout << " unmapped from ";
		  ((location *)mappedLoc)->print(cout, 1);
		  cout << '\n';
		}
	    }
	}
    }
      
  return retval;
}

int elsMapper::getTrueLoc2(region *reg, location **result)
{
  int        retval       = -1;
  elsMapper *mapper       = 0;

  if ( reg && result )
    {
      if ( *result = new location )
	{
	  // initialize the result
	  (*result)->start_line   = 0;
	  (*result)->start_column = 0;
	  (*result)->end_line     = 0;
	  (*result)->end_column   = 0;
	  (*result)->filename     = 0;

	  // Get mapper for region's file
	  if ( ((mapper = findMapper(reg->fromFile()->filename())) == 0) || (mapper->mappings == 0) )
	    {
	      // If no mapper for that region, then region to the final result
	      **result = (location)(*reg);
	      retval = 1;
	    }
	  else
	    {
	      // otherwise, use the mapping tabe to get the result
	      region    *unmapped_reg = 0;

	      // extract the the mapping list
	      if ( mapper->mappings->extract(reg, &unmapped_reg) >= 0 )
		if ( ! unmapped_reg )
		  {
		    // if no unmapping came back, add region to result
		    **result = (location)(*reg);
		    retval = 1;
		  }
		else
		  {
		    // if a mapping came back, get the true location of that instead (multi-phase mappings)
		    delete *result;  *result = 0; // get rid of the struct we allocated, as we will create another in the next phase
		    retval = getTrueLoc2(unmapped_reg, result);
		  }

	      delete unmapped_reg;
	    }
	  
	  if ( retval != 1 ) {
              delete *result;  *result = 0; }
	}
    }
	  
  return retval;
}

const position *elsMapper::getFileBoundary(char *filename)
{
    const position *retval = 0;

    if ( filename ) {
        for (MapperSet::iterator mapper = currentMaps->begin();
             mapper != currentMaps->end();
             ++mapper) {
            if ( (*mapper)->mappings ) {
                const position *tmp = (*mapper)->mappings->getFileBoundary(filename);
                if ( tmp ) {
                    if ( ! retval || *retval < *tmp ) {
                        retval = tmp;
		    }
		}
	    }
	}
    }
  
    return retval;
}

void elsMapper::dumpTo(ostream &o) const
{
  o << "------------------------------------------------------------\n";
  o << "ELSMAPPER for file \"" << forFile << "\"\n";
  mappings->dumpTo(o);
  o << "------------------------------------------------------------\n";
}

void elsMapper::dumpAllMappingInfo(ostream &o)
{
  o << "------------------------------------------------------------\n";
  o << "--------------------- ALL MAPPING INFO ---------------------\n";
  o << "------------------------------------------------------------\n";

  for (MapperSet::iterator em = currentMaps->begin();
       em != currentMaps->end();
       ++em) {
      if ( *em )
          (*em)->dumpTo(o);
  }
  o << "------------------------------------------------------------\n";
  o << "--------------------- ALL MAPPING INFO ---------------------\n";
  o << "------------------------------------------------------------\n";
}

extern "C" void elsMapper_dumpAllMappingInfo(void)
{
  elsMapper::dumpAllMappingInfo(cout);
}

void elsMapper::showBadLocs(int flag)
{
  if ( flag )
    showbadlocs = 1;
  else
    showbadlocs = 0;
}

void elsMapper::showAllUnmappings(int flag)
{
  if ( flag )
    showunmappings = 1;
  else
    showunmappings = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// private methods
//
////////////////////////////////////////////////////////////////////////////////

//
// this method finds the map object that contains mapping
// data for "filename".
//
// retvals: 0 if not found, address if found
//
elsMapper *elsMapper::findMapper(const char *filename)
{
  elsMapper *retval = 0;
  elsMapper *tmpval = 0;

  if ( filename ) {
      for (MapperSet::iterator mapper = currentMaps->begin();
           retval == 0 && mapper != currentMaps->end();
           ++mapper) {
	  if ( (*mapper)->forFile == filename ) {
              retval = *mapper;
          }
      }
  }
  
  return retval;
}


