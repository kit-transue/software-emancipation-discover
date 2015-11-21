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
#include <iostream.h>
//--C++

//++OSPORT
#include <pdufile.h>
#include <pdumem.h>
#include <pdupath.h>
#include <pduproc.h>
#include <pdustring.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <language.h>
#include <fileSorter.h>
//--ELS

const char * const fileSorter::sorterExecPrefKey          = "DIS_model.sorterExec";
const char * const fileSorter::sorterExecFlagsPrefKey     = "DIS_model.sorterExecFlags";
const char * const fileSorter::debuggingFileSorterPrefKey = "DIS_model.debugFileSorter";

#ifndef _WIN32
const char * const fileSorter::sorterExecPrefDefaultVal  = "sort";
const char * const fileSorter::sorterExecFlagsPrefDefaultVal = "%i -o %o";
#else
const char * const fileSorter::sorterExecPrefDefaultVal  = "%WINHOME%\\system32\\sort.exe";
const char * const fileSorter::sorterExecFlagsPrefDefaultVal = "< %i > %o";
#endif

const char * const fileSorter::inputFileMarker  = "%i";
const char * const fileSorter::outputFileMarker = "%o";

fileSorter::fileSorter(const language *forLang)
{
  lang = forLang;
  debuggingFileSorter = (pdstrcmp(lang->getPref(fileSorter::debuggingFileSorterPrefKey), "1") == 0);
}

fileSorter::~fileSorter(void)
{
}

int fileSorter::sortIntoLines(const char *input, parray *lines) const
{
  int retval = -1;

        char *shellCmd = 0;
  const char *output   = 0;
  const char *exec     = 0;
  const char *flags    = 0;
  const char *sorted   = 0;

  if ( input && lines )
    {
      if ( (output = pdtempname(0)) != 0 )
	if ( (exec = getSorterExec()) != 0 )
	  if ( (flags = getSorterExecFlags(input, output)) != 0 )
	    if ( (shellCmd = GET_MEMORY(char, pdstrlen(exec) + 1 + pdstrlen(flags) + 1)) != 0 )
	      if ( pdstrcpy(shellCmd, exec) != 0 )
		if ( pdstrcat(shellCmd, " ") != 0 )
		  if ( pdstrcat(shellCmd, flags) != 0 )
		    if ( pdsystem(shellCmd) >= 0 )
		      {
			if ( debuggingFileSorter )
			  {
			    cerr << "fileSorter: executing command \"" << shellCmd << "\"\n";
			  }
			if ( (sorted = pdloadfile(output)) != 0 )
			  if ( splitLines(sorted, lines) >= 0 )
			    {
			      if ( ! debuggingFileSorter )
				pdrmfile(output);
			      retval = 1;
			    }
		      }
    }

  return retval;
}

const char *fileSorter::getSorterExec(void) const
{
  const char *retval = 0;
  const char *cmd    = 0;

  if ( lang )
    {
      cmd = lang->getPref(fileSorter::sorterExecPrefKey);
      if ( cmd == 0 || pdstrcmp(cmd, "") == 0 )
	cmd = pdstrdup(fileSorter::sorterExecPrefDefaultVal);
      
      retval = pdexecpath(cmd);
    }
  
  FREE_MEMORY(cmd);
  return retval;
}

const char *fileSorter::getSorterExecFlags(const char *input, const char *output) const
{
  const char *retval = 0;

  char       *temp   = 0;
  const char *flags  = 0;

  if ( lang )
    {
      flags = lang->getPref(fileSorter::sorterExecFlagsPrefKey);
      if ( flags == 0 || pdstrcmp(flags, "") == 0 )
	flags = pdstrdup(fileSorter::sorterExecFlagsPrefDefaultVal);
      
      if ( pdstrgsub(flags, fileSorter::inputFileMarker, input, &temp) >= 0 )
	{
	  FREE_MEMORY(flags);
	  flags = temp;

	  if ( pdstrgsub(flags, fileSorter::outputFileMarker, output, &temp) >= 0 )
	    {
	      FREE_MEMORY(flags);
	      retval = temp;
	    }
	}
    }
  
  return retval;
}

int fileSorter::splitLines(const char *line, parray *lines) const
{
  int retval = -1;

  if ( line && lines )
    {
      pdstring_delete(lines);
      char *temp = GET_MEMORY(char, pdstrlen(line)+1);
      if ( temp )
	{
	  int t_indx = 0;
	  int l_indx = 0;
	  do 
	    {
	      t_indx = 0;
	      temp[0] = 0;
	      while ( line[l_indx] != '\0' && line[l_indx] != '\n')
		temp[t_indx++] = line[l_indx++];
	      temp[t_indx] = '\0';
	      lines->insert((void *)pdstrdup(temp));
	      if ( line[l_indx] == '\n' )
		l_indx++;
	    }
	  while ( line[l_indx] != '\0');
	  
	  retval = 1;
	}
      FREE_MEMORY(temp);
    }

  return retval;
}

const char *fileSorter::stripNewLines(const char *input) const
{
  char *retval = 0;

  if ( input )
    {
      if ( (retval = pdstrdup(input)) != 0 )
	{
	  int n = pdstrlen(retval);
	  for ( int i = 0; i < n; i++ )
	    if ( retval[i] == '\n' )
	      retval[i] = ' ';
	}
    }

  return retval;
}

