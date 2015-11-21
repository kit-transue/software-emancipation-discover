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
#include <pdufile.h>
#include <pdustring.h>
#include <pdumem.h>
//--OSPORT

//++ADS
#include <parray.h>
#include <dirFSet.h>
//--ADS

//++TSET
#include <settings.h>
//--TSET

//++ELS
#include <language.h>
//--ELS


////////////////////////////////////////////////////////////////////////////////
//
// Static initializations
//
////////////////////////////////////////////////////////////////////////////////

const char * const language::ldfInfo::ldfSuffix               = ".ldf";
const char * const language::ldfInfo::suffixPref              = "suffixes";
const char * const language::ldfInfo::defaultSuffixPref       = "defaultSuffix";
const char * const language::ldfInfo::headerSuffixPref        = "headerSuffixes";
const char * const language::ldfInfo::defaultHeaderSuffixPref = "defaultHeaderSuffix";
const char * const language::ldfInfo::langNamePref            = "languageName";
const char * const language::ldfInfo::parserFlagsPref         = "parserFlags";
const char * const language::ldfInfo::makefileStubPref        = "makefileStub";
const char * const language::ldfInfo::compilerPref            = "compiler";
const char * const language::ldfInfo::flagTranslationPref     = "flagFilter";
const char * const language::ldfInfo::parserPref              = "parser";
const char * const language::ldfInfo::gmakePref               = "gmake";
const char * const language::ldfInfo::sorterPref              = "sorter";

////////////////////////////////////////////////////////////////////////////////
//
// Constructor(s) / Destructor(s)
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: language::ldfInfo::ldfInfo
//
// Non-modified inputs: 
//      const char *file_name: the filename of the LDF to read its information from
//
// Modified inputs: 
//      none
//
// Returns: 
//      void (constructor)
//
// Description:
// Not much needed in the way of a description.  This sets all of the
// member data to the appropriate values.  If there is any error along the
// way, it cleans itself up and resets into an error condition.  It will
// also (assuming everying goes well), break the suffixes and
// headerSuffixes prefs into parrays for easier handling.

language::ldfInfo::ldfInfo(const char *file_name)
{
  int retval = -1;

  filename       = 0;
  discName       = 0;
  suffixes       = 0;
  headerSuffixes = 0;
  prefs          = 0;
  
  const char *value = 0;

  if ( file_name && pdisfile(file_name) )
    {
      // store the filename
      if ( (filename = pdstrdup(file_name)) != 0 )
	{
	  // the discover name is the filename without the suffix and without the directories preceding it
	  dirFSet fullPath((char *)file_name);
	  if ((discName = pdstridup(fullPath.filename(), pdstrlen(fullPath.filename()) - pdstrlen(ldfSuffix))) != 0 )
	    // load the LDF file into the settings
	    if ( (prefs = new Settings(0)) != 0 )
	      if ( prefs->open(filename) >= 0 )
		{
		  // load the parray of suffixes
		  Settings::Item sfxsValue(suffixPref);
		  if ( prefs->read(sfxsValue) >= 0 )
		    if ( sfxsValue.read(&value) >= 0 )
		      if ( (suffixes = new parray(0)) != 0 )
			if ( parseSuffixString(value, suffixes) >= 0 )
			  {
			    // load the parray of headerSuffixes
			    Settings::Item hdrValue(headerSuffixPref);
			    if ( prefs->read(hdrValue) >= 0 )
			      if ( hdrValue.read(&value) >= 0 )
				if ( (headerSuffixes = new parray(0)) != 0 )
				  if ( parseSuffixString(value, headerSuffixes) >= 0 )
				    retval = 1;
			  }
		}
	}
    }
  
  if ( retval == -1 )
    {
      FREE_MEMORY(filename);
      FREE_MEMORY(discName);
      
      pdstring_delete(suffixes);
      FREE_OBJECT(suffixes);
      
      pdstring_delete(headerSuffixes);
      FREE_OBJECT(headerSuffixes);
      
      FREE_OBJECT(prefs);
    }
}

//
// Function: language::ldfInfo::~ldfInfo
//
// Non-modified inputs:
//
// Modified inputs:
//
// Returns:
//
// Description: 
// Destructor.  Cleans up all allocated memory.
//
language::ldfInfo::~ldfInfo(void)
{
  FREE_MEMORY(filename);
  FREE_MEMORY(discName);
  
  pdstring_delete(suffixes);
  FREE_OBJECT(suffixes);

  pdstring_delete(headerSuffixes);
  FREE_OBJECT(headerSuffixes);

  FREE_OBJECT(prefs);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: language::ldfInfo::handlesSuffix
//
// Non-modified inputs:
//      const char *suffix: the suffix that we are asking about
//
// Modified inputs:
//
// Returns:
//      boolean result
//
// Description: 
// This function is a boolean test to ask the LDF whether or not it can
// deal with files with suffix "X"
//
int language::ldfInfo::handlesSuffix(const char *suffix) const
{
  int retval = 0;
  
  if ( suffix )
    {
      if ( suffixes )
	for ( int i = suffixes->size() - 1; i >= 0 && retval == 0; i-- )
#ifndef _WIN32
	  // UNIX filenames are case sensitive
	  if ( pdstrcmp((const char *)(*suffixes)[i], suffix) == 0 )
#else
	  // NT filenames are not case sensitive
	  if ( pdstrcasecmp((const char *)(*suffixes)[i], suffix) == 0 )
#endif
	    retval = 1;
    }

  return retval;
}

const char *language::ldfInfo::discoverName(void) const
{
  const char *retval = "";
  
  if ( discName )
    retval = discName;

  return pdstrdup(discName);
}
 
////////////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: language::ldfInfo::parseSuffixString
//
// Non-modified inputs:
//      const char *str: the string to break up into the parray
//
// Modified inputs:
//      parray *ar: the parray to bread the string into
//
// Returns:
//      -1: error, >= 0: the number of suffixes found
//
// Description: 
// This function will take a string that has a list of suffixes, seperated
// by white space, and will put each of them as a seperate element in the
// parray
int language::ldfInfo::parseSuffixString(const char *str, parray *ar) const
{
  int retval = -1;

  if ( str && ar )
    {
      const int WHITESPACE = 0;
      const int SUFFIX     = 1;
      const int DONE       = 2;

      int state = WHITESPACE;
      const char *suffixStart = 0;
      int count = 0;

      for ( const char *i = str; state != DONE; i++ )
	{
	  switch ( *i )
	    {
	    case ' ':
	    case '\t':
	    case '\n':
	      if ( state == SUFFIX )
		{
		  ar->insert((void *)pdstridup(suffixStart, i - suffixStart));
		  count++;
		  state = WHITESPACE;
		}
	      break;
	      
	    case '\0':
	      if ( state == SUFFIX )
		{
		  ar->insert((void *)pdstridup(suffixStart, i - suffixStart));
		  count++;
		}
	      state = DONE;
	      break;

	    default:
	      if ( state == WHITESPACE )
		{
		  suffixStart = i;
		  state = SUFFIX;
		}
	      break;
	    }
	}

      retval = count;
    }

  return retval;
}

const char *language::ldfInfo::getPref(const char *prefName) const
{
  const char *retval = "";

  if ( prefName )
    {
      if ( prefs )
	{
	  const char *strValue = 0;
	  Settings::Item itemValue(prefName);
	  if ( prefs->read(itemValue) >= 0 )
	    if ( itemValue.read(&strValue) >= 0 )
	      retval = strValue;
	}
    }

  return pdstrdup(retval);
}
