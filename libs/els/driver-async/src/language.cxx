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
//++OSPORT
#include <pdumem.h>
#include <pdustring.h>
#include <pdupath.h>
#include <pdudir.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <language.h>
//--ELS

////////////////////////////////////////////////////////////////////////////////
//
// Static initialization
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _WIN32
const char * const  language::ldfSearchPath = "$PSETHOME/lib/langinfo/LDF";
#else
const char * const  language::ldfSearchPath = "%PSETHOME%\\lib\\langinfo\\LDF";
#endif
const char * const  language::ldfSearchPref = "DIS_Model.ldfPath";
parray             *language::cachedLDFs    = 0;

////////////////////////////////////////////////////////////////////////////////
//
// Constructors / Destructors
//
////////////////////////////////////////////////////////////////////////////////

language::language(const char *filename)
{
  const char *suffix = 0;
  ldfInfo    *info   = 0;
  
  if ( ! cachedLDFs )
    cachedLDFs = new parray(0);

  ldfIndex = -1; // set to error to start with
  
  if ( (suffix = getSuffix(filename)) != 0 )
    if ( (info = getLDFInfo(suffix)) != 0 )
      ldfIndex = cachedLDFs->find((void *)info);
}

language::~language(void)
{
}


////////////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
////////////////////////////////////////////////////////////////////////////////

const char *language::name(void) const
{
  const char *retval = 0;
  ldfInfo    *info   = 0;

  if ( cachedLDFs && ldfIndex >= 0 )
    if ( (info  = (ldfInfo *)(*cachedLDFs)[ldfIndex]) != 0 )
      retval = info->getPref(ldfInfo::langNamePref);

  return retval;
}

const char *language::discoverName(void) const
{
  const char *retval = 0;
  ldfInfo    *info   = 0;

  if ( cachedLDFs && ldfIndex >= 0 )
    if ( (info  = (ldfInfo *)(*cachedLDFs)[ldfIndex]) != 0 )
      retval = info->discoverName();

  return retval;
}


////////////////////////////////////////////////////////////////////////////////
//
// Private methods
//
////////////////////////////////////////////////////////////////////////////////

const char *language::getSuffix(const char *filename) const
{
  const char *retval = 0;

  if ( filename )
    {
      const char *ptr = &filename[pdstrlen(filename)];
      while ( *ptr != '.'  && ptr != filename )
	ptr--;
      
      if ( *ptr == '.' )
	retval = ptr;
    }

  return retval;
}

language::ldfInfo *language::getLDFInfo(const char *suffix)
{
  ldfInfo *retval = 0;

  if ( suffix )
    {
      if ( (retval = getCachedLDF(suffix)) == 0 )    // first look in the cache
	if ( cacheLDF(suffix) >= 0 )                 // if not in cache, get one and add it to cache
	  retval = getCachedLDF(suffix);             // if successfully added to cache, get from the cache
    }

  return retval;
}

language::ldfInfo *language::getCachedLDF(const char *suffix) const
{
  ldfInfo *retval = 0;
  ldfInfo *info   = 0;

  if ( suffix )
    {
      if ( cachedLDFs )
	for ( int i = cachedLDFs->size() - 1; i >= 0 && retval == 0; i-- )  // for each LDF in the cache
	  if ( (info = (ldfInfo *)(*cachedLDFs)[i]) != 0 )             
	    if ( info->handlesSuffix(suffix) )                              // ask if it handles suffix X
	      retval = info;                                                // if yes, then we want this one
    }

  return retval;
}

int language::cacheLDF(const char *suffix)
{
  int      retval = -1;
  ldfInfo *info   = 0;
  parray   filenames(0);

  if ( suffix )
    {
      if ( getUnloadedLDFs(&filenames) >= 0 )
	{
	  int n = filenames.size();
	  int found = 0;
	  for ( int i = 0; i < n && ! found; i++ )
	    if ( (info = new ldfInfo((const char *)filenames[i])) != 0 )
	      if ( cachedLDFs->insert((void *)info) < 0 )
		FREE_OBJECT(info);
	      else if ( info->handlesSuffix(suffix) )
		{
		  found  = 1;
		  retval = 0;
		}
	  
	  pdstring_delete(&filenames);
	}
    }
  
  return retval;
}

int language::getUnloadedLDFs(parray *filenames) const
{
  int   retval     = -1;
  char *searchPath = 0;

  if ( filenames )
    {
      if ( getLDFSearchPath(&searchPath) >= 0 )
	{
	  parray  dirs(0);
	  parray  files(0);
	  int     ndirs    = 0;
	  int     nfiles   = 0;
	  char   *fullname = 0;
	  
	  ndirs = pdpathX(searchPath, &dirs);
	  for ( int i = 0; i < ndirs; i++ )
	    {
	      nfiles = pdlsdir((char *)dirs[i], &files);
	      for ( int j = 0; j < nfiles; j++ )
#ifndef _WIN32
		// UNIX filenames are case sensitive
		if ( pdstrcmp(getSuffix((const char *)files[j]), ldfInfo::ldfSuffix) == 0 )
#else
		// NT filenames are not case sensitive
		if ( pdstrcasecmp(getSuffix((const char *)files[j]), ldfInfo::ldfSuffix) == 0 )
#endif
		  if ( (fullname = pdepath((char *)dirs[i], (char *)files[j])) != 0 )
		    if ( filenames->insert((void *)fullname) < 0 )
		      FREE_MEMORY(fullname);
	      pdstring_delete(&files);
	    }
	  
	  pdstring_delete(&dirs);

	  if ( filenames->size() > 0 )
	    retval = 1;
	}
	  
      FREE_MEMORY(searchPath);
    }

  return retval;
}

int language::getLDFSearchPath(char **searchPath) const
{
  int         retval           = -1;
  char       *fullSearchPath   = 0;
  const char *userSearchPath   = 0;
  const char  pathSeperator[]  = { PATH_PATH_SEP_CH, '\0' }; // PATH_PATH_SEP_CH defined in pdupath.h

  if ( searchPath )
    {
      //// TEMP HACK!!!!
      //if ( (userSearchPath = customize_getStrPref(language::ldfSearchPref)) != 0 )
      //{
      //	  if ( (fullSearchPath = pdstrdup(userSearchPath)) != 0 )
      //	    if ( pdstrcat(fullSearchPath, pathSeperator) >= 0 )
      // if ( pdstrcat(fullSearchPath, language::ldfSearchPath) >= 0 )
      //retval = 1;
      //}
      //else // no pref specifed, just use default search path
	{
	  if ( (fullSearchPath = pdstrdup(language::ldfSearchPath)) != 0 )
	    retval = 1;
	}

      if ( retval )
	*searchPath = fullSearchPath;
      else
	FREE_MEMORY(fullSearchPath);
    }

  return retval;
}
      

