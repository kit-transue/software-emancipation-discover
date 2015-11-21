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
//
////////////////////////////////////////////////////////////////////////////////

//++OSPORT
#include <dirFSet.h>
#ifndef USE_EXTERN_LIB
#include <pdutil.h>
#else
#include <pdustring.h>
#include <pdumem.h>
#include <pdupath.h>
#endif
//--OSPORT

//++ADS
#include <parray.h>
#include <dirFSet.h>
//--ADS

//++IF_PARSER
#include <ifFileCache.h>
//--IF_PARSER

// Internal persistant cache object (all cache objects within a process share 
// the same cache data)
static ifFileCache theCache;

dirFSet *ifFileCache::df        = 0;
parray  *ifFileCache::files     = 0;
int      ifFileCache::leftOffAt = 0;
int      ifFileCache::refcount  = 0;

ifFileCache::ifFileCache(void)
{
  if ( ! files )
    files = new parray(32);

  if ( ! df )
    df = new dirFSet(0);

  refcount++;
}

ifFileCache::~ifFileCache(void)
{
  refcount--;
}

int ifFileCache::insert(char *file, char **relative_name, char **absolute_name)
{
  int retval = 0;
  
  //
  // The steps in this algorithm are
  // 1. look through the real path pointers for the pointer value of "file"
  //    if found, then cachedFile is file
  // 2. look though (with strcmp) the unresolved names we've seen so far,
  //    to see if we've seen it before, if found, cachedFile is the real counterpart
  // 3. resolve the real path of file. look through (with strcmp) the resolved files
  //    to see if there's a match.  If there is a match, cachedFile is the value from
  //    the real part of the struct, we create a new struct to hold the new unresovled
  //    filename and free the resolved name
  // 4. If the resolved name was not found at all, create a new struct, set the read
  //    to a copy of file, set the real to the resolved, insert struct into list,
  //    cachedFile is the resolved filename
  //
  //    Note: all lookups are done starting with the index where the last hit was found.
  //          this heuristic should lead to best performance given an average IF file.
  //


  if ( file && relative_name && absolute_name )
    {
      int i = 0;
      int n = files->size();

      // look to see if pointer value in file is already in the real list
      for ( i = leftOffAt; i < n && ! retval; i++ )
	{
	  readReal *r = (readReal *)(*files)[i];
	  if ( r )
	    if ( file == r->real )
	      {
		retval         = 1;
		leftOffAt      = i;
		*absolute_name = r->real;
		*relative_name = r->real;
	      }
	}
      for ( i = 0; i < leftOffAt && ! retval; i++ )
	{
	  readReal *r = (readReal *)(*files)[i];
	  if ( r )
	    if ( file == r->real )
	      {
		retval      = 1;
		leftOffAt   = i;
		*absolute_name = r->real;
		*relative_name = r->real;
	      }
	}

      if ( ! retval )
	{
	  // look trough unresolved names
	  for ( i = leftOffAt; i < n && ! retval; i++ )
	    {
	      readReal *r = (readReal *)(*files)[i];
	      if ( r )
		if ( pdstrcmp(file, r->read) == 0 )
		  {
		    retval         = 1;
		    leftOffAt      = i;
		    *absolute_name = r->real;
		    *relative_name = r->read;
		  }
	    }
	  for ( i = 0; i < leftOffAt && ! retval; i++ )
	    {
	      readReal *r = (readReal *)(*files)[i];
	      if ( r )
		if ( pdstrcmp(file, r->read) == 0 )
		  {
		    retval      = 1;
		    leftOffAt   = i;
		    *absolute_name = r->real;
		    *relative_name = r->read;
		  }
	    }
	  
	  if ( ! retval )
	    {
	      char *resolved = 0;
	      if ( df->xtract_df(file) ) 
		resolved = df->rldrlf(0);
	      if ( ! resolved )
		resolved = pdstrdup(file);
	      readReal *newr = new readReal;
	      if ( resolved && newr )
		{
		  // strcmp to resolved filenames
		  for ( i = leftOffAt; i < n && ! retval; i++ )
		    {
		      readReal *r = (readReal *)(*files)[i];
		      if ( r )
			if ( pdstrcmp(r->real, resolved) == 0 )
			  {
 			    if ( pdstrcmp(file, r->real) == 0 )
			      newr->read  = r->real;
			    else
			      newr->read  = pdstrdup(file);
			    newr->real  = r->real;
			    files->insert((void *)newr);

			    retval      = 1;
			    leftOffAt   = i;
			    *absolute_name = newr->real;
			    *relative_name = newr->read;
			  }
		    }
		  for ( i = 0; i < leftOffAt && ! retval; i++ )
		    {
		      readReal *r = (readReal *)(*files)[i];
		      if ( r )
			if ( pdstrcmp(r->read, resolved) == 0 )
			  {
 			    if ( pdstrcmp(file, r->real) == 0 )
			      newr->read  = r->real;
			    else
			      newr->read  = pdstrdup(file);
			    newr->real  = r->real;
			    files->insert((void *)newr);

			    retval      = 1;
			    leftOffAt   = i;
			    *absolute_name = newr->real;
			    *relative_name = newr->read;
			  }
		    }
		  
		  if ( retval )
		    {
		      FREE_MEMORY(resolved);
		    }
		  else
		    {
		      newr->read  = pdstrdup(file);
		      newr->real  = resolved;
		      files->insert((void *)newr);

		      retval      = 1;
		      leftOffAt   = files->size() - 1;
		      *absolute_name = newr->real;
		      *relative_name = newr->read;
		    }
		}
	    }
	}
    }

  if ( ! retval )
    {
      if ( absolute_name )
	*absolute_name = 0;
      if ( relative_name )
	*relative_name = 0;
    }

  return retval;
}



//
// C interface
//

extern "C" int ifFileCache_insertFile(char *file, char **rel, char **abs)
{
  return ::theCache.insert(file, rel, abs);
}


