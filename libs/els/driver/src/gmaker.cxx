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
#include <fstream.h>
//--C++

#ifndef __STAND_ALONE_IF_PARSER
//++MACHINE
#include <customize_extern.h>
//--MACHINE
#endif

//++OSPORT
#include <pdumem.h>
#include <pdustring.h>
#include <pdufile.h>
#include <pdupath.h>
#include <pduproc.h>
#include <raFile.h>
//--OSPORT

//++ADS
#include <parray.h>
//--ADS

//++ELS
#include <langIffProducer.h>
#include <language.h>
#include <gmaker.h>
#include <flagGenerator.h>
#include <fileSorter.h>
//--ELS

const char * const gmaker::debugGmakerPrefKey         = "DIS_model.Debug.DebugGmaker";
const char * const gmaker::gmakeExecPrefKey           = "DIS_model.gmakeExec";
const char * const gmaker::parserExecPrefKey          = "DIS_model.parserExec";
const char * const gmaker::makefileStubPrefKey        = "DIS_model.makefileStub";
const char * const gmaker::gmakeFlagsPrefKey          = "DIS_model.gmakeFlags";

#ifndef _WIN32
const char * const gmaker::gmakeExecPrefDefaultVal    = "$PSETHOME/bin/els/gmake";
const char * const gmaker::parserExecPrefDefaultVal   = "$PSETHOME/bin/els/aset_CCcc";
const char * const gmaker::makefileStubPrefDefaultVal = "$PSETHOME/lib/els/Make.C.stub";
const char * const gmaker::gmakeFlagsPrefDefaultVal   = "-f %m";
#else
const char * const gmaker::gmakeExecPrefDefaultVal    = "%PSETHOME%\\bin\\els\\gmake.exe";
const char * const gmaker::parserExecPrefDefaultVal   = "%PSETHOME%\\bin\\els\\aset_CCcc.exe";
const char * const gmaker::makefileStubPrefDefaultVal = "%PSETHOME%\\lib\\els\\Make.C.stub";
const char * const gmaker::gmakeFlagsPrefDefaultVal   = "-f %m";
#endif

const char * const gmaker::makefileMarker             = "%m";  // note to change above, if you're changing this

gmaker::gmaker(const language *forLanguage) : lang(forLanguage)
{
#ifndef __STAND_ALONE_IF_PARSER
  debuggingGmaker = customize_getIntPref(gmaker::debugGmakerPrefKey);
#else
  debuggingGmaker = 1;
#endif
}

gmaker::~gmaker(void)
{
}

int gmaker::execute(const char *source, const char *iffile) const
{
  int retval = -1;

  if ( source && lang && iffile )
    {
      char       *commandline           = 0;
      const char *makefilename          = 0;
      const char *intermediatesFilename = 0;

      if ( buildMakeFile(source, iffile, &makefilename, &intermediatesFilename) >= 0 )
	if ( buildCommandLine(makefilename, &commandline) >= 0 )
	  if ( execute(commandline) >= 0 )
	    if ( buildIntermediates(intermediatesFilename, iffile) >= 0 )
	      retval = 1;

      if ( ! debuggingGmaker )
	{
	  pdrmfile(makefilename);
	  pdrmfile(intermediatesFilename);
	}

      FREE_MEMORY(makefilename);
      FREE_MEMORY(commandline);
      FREE_MEMORY(intermediatesFilename);
    }

  return retval;
}

int gmaker::buildMakeFile(const char *source, const char *iffile, const char **makefile, const char **intermediatesFilename) const
{
  int retval = -1;

  if ( source && lang && iffile && makefile && intermediatesFilename)
    {
      const char    *parserPref   = 0;
      const char    *gmakePref    = 0;
      const char    *parser       = 0;
      const char    *gmake        = 0;
      const char    *stub         = 0;
      const char    *stubContents = 0;
      const char    *fullSource   = 0;
      const char    *statFlags    = 0;
      const char    *dynFlags     = 0;
      
      *makefile                   = pdtempname(0);
      *intermediatesFilename      = pdtempname(0);

      if ( *makefile && *intermediatesFilename )
	{
	  flagGenerator generator(lang);

	  if ( (parserPref = lang->getPref(gmaker::parserExecPrefKey)) == 0 )
	    parserPref = pdstrdup(gmaker::parserExecPrefDefaultVal);

	  if ( (gmakePref = lang->getPref(gmaker::gmakeExecPrefKey)) == 0 )
	    gmakePref = pdstrdup(gmaker::gmakeExecPrefDefaultVal);

	  if ( (stub = lang->getPref(gmaker::makefileStubPrefKey)) == 0 )
	    stub = pdstrdup(gmaker::makefileStubPrefDefaultVal);

	  parser       = pdexecpath(parserPref);
	  gmake        = pdexecpath(gmakePref);
	  fullSource   = resolveFilename(source);
	  stubContents = pdloadfile(stub);
	  statFlags    = generator.getStaticFlags();
	  dynFlags     = generator.getDynamicFlags(fullSource);
      
	  if ( parser && gmake && fullSource && stubContents && statFlags && dynFlags )
	    {
	      ofstream Makefile(*makefile);
	      if ( Makefile )
		{
		  Makefile << "#\n";
		  Makefile << "# This makefile generated automatically by DISCOVER\n";
		  Makefile << "# Do not edit this file!\n";
		  Makefile << "#\n\n";
		  Makefile << "PARSER=" << parser << '\n';
		  Makefile << "STATIC_FLAGS=" << statFlags << '\n';
		  Makefile << "DYNAMIC_FLAGS=" << dynFlags << '\n';
		  Makefile << "SOURCE_FILE=" << fullSource << '\n';
		  Makefile << "IF_FILE=" << iffile << '\n';
		  Makefile << "INTERMEDIATES=" << intermediatesFilename << '\n';
		  Makefile << "GMAKE=" << gmake << '\n';
		  Makefile << "THIS_MAKEFILE=" << makefile << '\n';
		  Makefile << "\n\n";
		  Makefile << stubContents << '\n';
		  Makefile.close();
		  retval = 1;
		}
	    }
	}

      FREE_MEMORY(parserPref);
      FREE_MEMORY(gmakePref);
      FREE_MEMORY(stub);
      FREE_MEMORY(parser);
      FREE_MEMORY(gmake);
      FREE_MEMORY(fullSource);
      FREE_MEMORY(stubContents);
      FREE_MEMORY(statFlags);
      FREE_MEMORY(dynFlags);

      if ( retval < 0 )
	{
	  FREE_MEMORY(*makefile);
	  FREE_MEMORY(*intermediatesFilename);
	}
    }

  return retval;
}



int gmaker::buildCommandLine(const char *makefile, char **commandline) const
{
  int retval = -1;
  
  if ( makefile && commandline )
    {
      const char *gmakeExecPrefVal  = 0;
      const char *gmakeExec         = 0;
      const char *gmakeFlagsPrefVal = 0;
      char       *gmakeFlags        = 0;

      *commandline = 0;

      if ( (gmakeExecPrefVal = lang->getPref(gmaker::gmakeExecPrefKey)) == 0 )
	gmakeExecPrefVal = pdstrdup(gmaker::gmakeExecPrefDefaultVal);
      
      if ( gmakeExecPrefVal )
	{
	  if ( (gmakeExec = pdexecpath(gmakeExecPrefVal)) != 0 )
	    {
	      if ( (gmakeFlagsPrefVal = lang->getPref(gmaker::gmakeFlagsPrefKey)) == 0 )
		gmakeFlagsPrefVal = pdstrdup(gmaker::gmakeFlagsPrefDefaultVal);

	      if ( gmakeFlagsPrefVal )
		  if ( pdstrgsub(gmakeFlagsPrefVal, gmaker::makefileMarker, makefile, &gmakeFlags) >= 0 )
		    if ( (*commandline = GET_MEMORY(char, pdstrlen(gmakeExec) + 1 + pdstrlen(gmakeFlags) + 1)) != 0 )
		      if ( pdstrcpy(*commandline, gmakeExec) != 0 )
			if ( pdstrcat(*commandline, " ") != 0 )
			  if ( pdstrcat(*commandline, gmakeFlags) != 0 )
			    retval = 1;
	    }
	}

      if ( retval < 0 )
	{
	  FREE_MEMORY(*commandline);
	}

      FREE_MEMORY(gmakeExecPrefVal);
      FREE_MEMORY(gmakeExec);
      FREE_MEMORY(gmakeFlagsPrefVal);
      FREE_MEMORY(gmakeFlags);
    }

  return retval;
}

int gmaker::execute(const char *commandline) const
{
  int retval = -1;

 if ( commandline )
   if ( pdsystem(commandline) >= 0 )
     retval = 1;

  return retval;
}

int gmaker::buildIntermediates(const char *intermediatesFilename, const char *iffile) const
{
  int retval = -1;
  int error  = 0;

  if ( intermediatesFilename && iffile )
    {
      if ( pdisfile(intermediatesFilename) )
	{
	  parray     filenames(0);
	  fileSorter sorter(lang);
	  if ( sorter.sortIntoLines(intermediatesFilename, &filenames) > 0 )
	    {
	      langIffProducer producer;
	      int n = filenames.size();
	      for ( int i = 0; i < n && ! error; i++ )
		{
		  error = 1;
		  const char *nextIfFile = 0;
		  if ( producer.produceIff((const char *)filenames[i], &nextIfFile) >= 0 )
		    {
		      if ( catFiles(iffile, nextIfFile) >= 0 )
			{
			  pdrmfile(nextIfFile);
			  error = 0;
			}
		    }
		}
	      if ( ! error )
		{
		  retval = 1;
		}
	    }
	  pdstring_delete(&filenames);
	}
      else
	{
	  retval = 1; // no error if file was never created
	}
    }

  return retval;
}

char *gmaker::resolveFilename(const char *filename) const
{
  char *retval = 0;

  // could not auto initialize sep[] due to HP compiler limitations
  char sep[2]; sep[0] = PATH_FILE_SEP_CH; sep[1] = '\0';
  char temp[4096];

  if ( filename )
    {
      if ( ! pdabspath(filename) )
	{
	  if ( pdgetcwd(temp, 4096) != 0 )
	    if ( pdstrcat(temp, sep) != 0 )
	      if ( pdstrcat(temp, filename) != 0 )
		retval = pdrealpath(temp);
	}
      else
	{
	  retval = pdrealpath(filename);
	}
    }

  return retval;
}


int gmaker::catFiles(const char *file1, const char *file2) const
{
  int retval = -1;

  if ( file1 && file2 )
    {
      int error  = 0;

      raFile raf1(file1, 0);
      raFile raf2(file2, 0);
      if ( raf1.open(raFile_ACCESS_WRITE, raFile_WRITE_APPEND | raFile_OPEN_CREATE) >= 0 )
	if ( raf2.open(raFile_ACCESS_READ, raFile_OPEN_CREATE) >= 0 )
	  {
	    const int bufsize = 256 * 1024;
	    char buffer[bufsize];
	    int nbytes = 0;
	    while ( ! error && ((nbytes = raf2.read(buffer, bufsize)) > 0) )
	      {
		if ( raf1.write(buffer, nbytes) < 0 )
		  {
		    error = 1;
		  }
	      }
	  }

      if ( ! error )
	retval = 1;
    }

  return retval;
}

