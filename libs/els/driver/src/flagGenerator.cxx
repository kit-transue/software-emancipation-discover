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
#include <pdustring.h>
#include <raFile.h>
#include <pdufile.h>
#include <pdumem.h>
#include <pduproc.h>
#include <pdupath.h>
//--OSPORT

//++ELS
#include <flagGenerator.h>
#include <language.h>
//--ELS

const char * const flagGenerator::filterInputFileMarker         = "%i";
const char * const flagGenerator::filterOutputFileMarker        = "%o";   

const char * const flagGenerator::staticFlagsPrefKey            = "DIS_model.staticFlags";
const char * const flagGenerator::staticFlagsPrefDefaultVal     = " ";

const char * const flagGenerator::filterExecFlagsPrefKey        = "DIS_model.dynFlagsFilterFlags";
const char * const flagGenerator::filterExecFlagsPrefDefaultVal = "< %i > %o";

const char * const flagGenerator::filterExecPrefKey             = "DIS_model.dynFlagsFilter";
#ifndef _WIN32
const char * const flagGenerator::filterExecPrefDefaultVal      = "$PSETHOME/bin/els/unixflagfilter";
#else
const char * const flagGenerator::filterExecPrefDefaultVal      = "%PSETHOME%\\bin\\els\\ntflagfilter.exe";
#endif

flagGenerator::flagGenerator(const language *forLanguage)
{
  lang = forLanguage;
}

flagGenerator::~flagGenerator(void)
{
}

const char *flagGenerator::getStaticFlags(void) const
{
  const char *retval = 0;

  if ( lang )
    {
      retval = lang->getPref(flagGenerator::staticFlagsPrefKey);
    }

  return retval;
}


const char *flagGenerator::getDynamicFlags(const char *srcFile) const 
{
  const char *retval = 0;
  
  const char *flagsIn  = 0;
  const char *flagsOut = 0;

  if ( lang && srcFile )
    {
      if ( (flagsIn = getFlagsFromPDF(srcFile)) !=0 )
	if ( (flagsOut = filterFlags(flagsIn)) != 0 )
	  retval = flagsOut;
    }

  FREE_MEMORY(flagsIn);
      
  return retval;
}

const char *flagGenerator::getFlagsFromPDF(const char *filename) const
{
  // need to figure out how to do this for real!!!!
  return pdstrdup("CC -g -O2 --all-warnings -I. -I../.. -DTHIS_IS_A_TEST -DFLAG=1234\n");
}

const char *flagGenerator::filterFlags(const char *flagsToFilter) const
{
  const char *retval = 0;

  if ( flagsToFilter )
    {
      char       *cmd        = 0;
      char       *tmpflags   = 0;
      const char *flags      = 0;
      const char *filterExec = 0;
      const char *input      = pdtempname(0);
      const char *output     = pdtempname(0);

      if ( input && output )
	{
	  if ( (filterExec = lang->getPref(flagGenerator::filterExecPrefKey)) == 0 )
	    filterExec = pdstrdup(flagGenerator::filterExecPrefDefaultVal);

	  if ( filterExec )
	    {
	      if ( (flags = lang->getPref(flagGenerator::filterExecFlagsPrefKey)) == 0 )
		flags = pdstrdup(flagGenerator::filterExecFlagsPrefDefaultVal);

	      if ( flags )
		{
		  if ( pdstrgsub(flags, flagGenerator::filterInputFileMarker, input, &tmpflags) >= 0 )
		    {
		      FREE_MEMORY(flags);
		      flags = tmpflags;
		      if ( pdstrgsub(flags, flagGenerator::filterOutputFileMarker, output, &tmpflags) >= 0 )
			{
			  FREE_MEMORY(flags);
			  flags = tmpflags;
			  if ( (cmd = GET_MEMORY(char, pdstrlen(filterExec) + pdstrlen(flags) + 2)) != 0 )
			    {
			      pdstrcpy(cmd, filterExec);
			      pdstrcat(cmd, " ");
			      pdstrcat(cmd, flags);

			      raFile file(input, 0);
			      if ( file.open(raFile_ACCESS_WRITE, raFile_OPEN_CREATE | raFile_OPEN_TRUNC) >= 0 )
				if ( (file.write((char *)flagsToFilter, pdstrlen(flagsToFilter))) >= 0 )
				  if ( (file.write("\n", 1)) >= 0 )
				    if ( (file.close(0)) >= 0 )
				      if ( pdsystem(cmd) >= 0 )
					if ( pdisfile(output) )
					  {
					    const char *temp  = pdloadfile(output);
					    char       *temp2 = 0;

					    // for NT make sure we get rid of carriage-returns as well
					    pdstrgsub(temp, "\r", "", &temp2);
					    FREE_MEMORY(temp);
					    temp = temp2;

					    pdstrgsub(temp, "\n", " ", &temp2);
					    FREE_MEMORY(temp);
					    retval = temp2;
					  }
					else
					  retval = pdstrdup("");
			    }
			}
		    }
		}
	    }

	  pdrmfile(input);
	  pdrmfile(output);
	  FREE_MEMORY(input);
	  FREE_MEMORY(output);
	  FREE_MEMORY(filterExec);
	  FREE_MEMORY(flags);
	  FREE_MEMORY(cmd);
	}
    }
  
  return retval;
}

