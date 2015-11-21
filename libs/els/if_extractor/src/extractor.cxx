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
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

//++C++
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
#include <pdustring.h>
#include <pdufile.h>
#include <pdumem.h>
#include <dirFSet.h>
//--OSPORT

//++ADS
#include <argParam.h>
#include <parray.h>
//--ADS

//++IF_PARSER
#include <ifFileCache.h>
#include <elsMapper.h>
//--IF_PARSER

//++EXTRACTOR
#include <extractor.h>
#include <extractorArgs.h>
#include <ifExtractor.h>
//--EXTRACTOR

int main ( int argc, char ** argv )
{
  int retval = -1;

  if ( argc > 1 )
    {
      // collect command-line arguments
      parray arguments(8);
      for ( int i = 0; i < argc; i++ )
	arguments.insert((void *)argv[i]);

      if ( arguments.size() > 0 )
	{
	  // create argument object out of argument texts
	  argParam *arg_param = setup_argPrmAssign(extractorArgID_xname, extractorArg_ID_LIST_SIZE);
	  if ( arg_param )
	    {
	      if ( arg_param->extract(&arguments) > 0 )
		{
		  // do the "meat" of the processing
		  retval = processArguments(arg_param);
		  FREE_OBJECT(arg_param);
		}
	      else
		cerr << "Error: " << argv[0] << ": could not access command-line arguments." << '\n';
	    }
	  else
	    cerr << "Error: " << argv[0] << ": unable to build command-line argument structures." << '\n';
	}
      else
	cerr << "Error: " << argv[0] << ": unable to read to command-line arguments." << '\n';
    }
  else
    cerr << "Usage: " << argv[0] << " if-file=<if-filename> split-file=<split-filename>" << '\n';
  
  return retval;
}

int processArguments(argParam *args)
{
  int retval = -1;

  if ( args )
    {
      char *showbadlocs = args->value(extractorArg_SHOW_BAD_LOCS, 0);
      if ( showbadlocs )
	if ( showbadlocs[0] != 'n' && showbadlocs[0] != 'N' && showbadlocs[0] != '0' )
	  elsMapper::showBadLocs(1);
	else
	  elsMapper::showBadLocs(0);

      char *showmaptable = args->value(extractorArg_SHOW_MAP_TABLE, 0);
      if ( showmaptable )
	if ( showmaptable[0] != 'n' && showmaptable[0] != 'N' && showmaptable[0] != '0' )
	  ifExtractor::showMapTable(1);
	else
	  ifExtractor::showMapTable(0);

      char *showprogress = args->value(extractorArg_SHOW_PROGRESS, 0);
      if ( showprogress )
	if ( showprogress[0] != 'n' && showprogress[0] != 'N' && showprogress[0] != '0' )
	  ifExtractor::showProgress(1);
	else
	  ifExtractor::showProgress(0);

      char *showallunmappings = args->value(extractorArg_SHOW_ALL_UNMAPPINGS, 0);
      if ( showallunmappings )
	if ( showallunmappings[0] != 'n' && showallunmappings[0] != 'N' && showallunmappings[0] != '0' )
	  elsMapper::showAllUnmappings(1);
	else
	  elsMapper::showAllUnmappings(0);


      dirFSet  ifInput(args->value(extractorArg_IF_FILE, 0));
      dirFSet  splitInput(args->value(extractorArg_SPLIT_FILE, 0));

      // make sure the files specifed int that arguments actually exist
      char *ifInputName    = ifInput.rldrlf(0);
      char *splitInputName = splitInput.rldrlf(0);
      if ( ifInputName )
	if ( splitInputName )
	  {
	    parray sources(32);
	    parray ifs(32);
	    if ( readSplitInfo(splitInputName, &sources, &ifs) >= 0 )
	      {
		// make sure we have a IF for each source
		if ( sources.size() == ifs.size() )
		  {
		    ifExtractor extr(ifInputName);
		    if ( extr.extract(&sources, &ifs) >= 0 )
		      {
			retval = 0;
		      }
		    else
		      cerr << "Error: extraction was unsuccessful." << '\n';
		  }
		else
		  cerr << "Error: Check the split file: Odd number of entries not allowed." << '\n';
	      }
	    else
	      cerr << "Error: could not successfully parse the split-file." << '\n';
	    //pdstring_delete(&sources);
	    //pdstring_delete(&ifs);
	  }
	else
	  cerr << "Error: the split file \"" << args->value(extractorArg_SPLIT_FILE, 0) << "\" could not be found." << '\n';
      else
	cerr << "Error: the IF file \"" << args->value(extractorArg_IF_FILE, 0) << "\" could not be found." << '\n';
      FREE_MEMORY(ifInputName);
      FREE_MEMORY(splitInputName);
    }
  
  return retval;
}

#define SPLIT_BUF_SIZE 32768
#define SEPCHARS "\t\n\r"

int readSplitInfo(const char *filename, parray *sources, parray *ifs)
{
  int retval = -1;
  
  if ( filename && sources && ifs )
    {
      char *fileinfo = pdloadfile(filename);
      if ( fileinfo )
	{
	  int   state    = 0;
	  int   len      = pdstrlen(fileinfo);
	  char *filename = fileinfo;
	  for ( int i = 0; i < len; i++ )
	    {
	      if ( pdccontained(fileinfo[i], SEPCHARS) || ((i+1)==len))
		{
		  fileinfo[i] = 0;
		  char *absfile = 0;
		  char *relfile = 0;
		  ifFileCache_insertFile(filename, &relfile, &absfile);
		  if ( state == 0 )
		    sources->insert((void *)absfile);
		  else
		    ifs->insert((void *)absfile);
		  for (i++; i<len && pdccontained(fileinfo[i], SEPCHARS); i++) ;
		  filename = &fileinfo[i];
		  if ( state == 0 )
		    state = 1;
		  else
		    state = 0;
		}
	    }
	  
	  retval = 1;
	}
      FREE_MEMORY(fileinfo);
    }

  return retval;
}
