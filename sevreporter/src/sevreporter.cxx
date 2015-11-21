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
////////////////////////////////////////////////////////////////////////////////

//++C++
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C++

//++ADS
#include <argParam.h>
#include <argPrmAssign.h>
//--ADS

//++OSPORT
#include <pdumem.h>
#include <pdustring.h>
#include <pdupath.h>
//--OSPORT

//++SEV
#include <reportStream.h>
#include <reportStreamText.h>
#include <reportStreamHTML.h>
#include <reportTemplate.h>
#include <reportDataParser.h>
#include <reporterArgs.h>
#include <rootFolder.h>

//--SEV

//
// Forward declarations of file-static functions
//

static int             processCmdLineArgs(int, char **);
static int             generateReport(rootFolder *, char const *);
static reportStream   *selectReportStream(char const *, char const *);
static reportTemplate *selectReportTemplate(char const *);

//
// MAIN
//

int main ( int argc, char **argv )
{
  int     retval = -1;

  rootFolder *root_folder = 0;
  inventory  *inven       = 0;
  inputs     *inp         = 0;
  
  if ( processCmdLineArgs(argc, argv) >= 0 )
    {
      if ( reportDataParse((char *)rptgenArg_value(rptgenArg_REPORT_DATA), &root_folder, &inven, &inp) >= 0 )
	{
	  if ( root_folder && root_folder->store_inputs(inp) >= 0 && root_folder->store_inventory(inven) >= 0 )
	    {
	      if ( generateReport(root_folder, rptgenArg_value(rptgenArg_REPORT_OUTPUT)) >= 0 )
		{
		  retval = 0;
		}
	      else
		cerr << argv[0] << ": error during report generation.\n";
	    }
	  else
	    cerr << argv[0] << ":error during report construction.\n";
	}
      else
	cerr << argv[0] << ": error during parsing of report data.\n";
    }
  else
    cerr << argv[0] << ": error processing the command-line.\n";
  
  FREE_OBJECT(root_folder);

  return retval;
}


//
// ProcessCmdLineArgs:
// returns: -1 if missing or invalid arguments, 0 otherwise
//

static int processCmdLineArgs(int argc, char **argv)
{
  int retval = -1;

  if ( argc > 1 && argv )
    {
      // get the args from the command-line
      parray args(16);
      for (int i = 1; i < argc; i++)
	args.insert((void *)argv[i]);
      
      // build an argParam out of them
      argParam *cmd_line_opts = setup_argPrmAssign(rptgenArg_xname, rptgenArg_ID_LIST_SIZE);
      if ( cmd_line_opts && cmd_line_opts->extract(&args) > 0)
	{
	  // fill the arg table with defaults, or values from command-line
	  if ( rptgenArg_init_values(cmd_line_opts) >= 0 )
	    {
	      retval = 0;
	    }
	}
    }

  return retval;
}
		  
//
// This function generates the report from the report data
//
static int generateReport(rootFolder *root, char const *output)
{
  int             retval    = -1;
  reportStream   *outstream = 0;
  reportTemplate *templ     = 0;
  
  if ( root && output )
    {
      // get a report stream of the appropriate type
      if ( outstream = selectReportStream(rptgenArg_value(rptgenArg_REPORT_FORMAT), output) )
	{
	  
	  // get the proper report template
	  if ( templ = selectReportTemplate(rptgenArg_value(rptgenArg_REPORT_TEMPLATE)) )
	    {
	      // generate the report
	      retval = root->serialize(outstream, templ);
	    }
	}
    }

  // free resources
  FREE_OBJECT(outstream);
  FREE_OBJECT(templ);

  return retval;
}

//
// This function will create the proper reportStream object
//
static reportStream *selectReportStream(char const *type, char const *filename)
{
  reportStream *retval = 0;

  if ( type && filename )
    {
      if ( pdstrcasecmp(type, "html") == 0 )
	{
	  retval = new reportStreamHTML(filename);
	}
      else if ( pdstrcasecmp(type, "text") == 0 )
	{
	  retval = new reportStreamText(filename);
	}
    }

  return retval;
}

//
// This function will create the proper reportTemplate object
//
static reportTemplate *selectReportTemplate(char const *file)
{
  reportTemplate *retval    = new reportTemplate;
  char const     *realfile  = 0;

  if ( retval && file )
    {
      if ( realfile = pdrealpath(file) )
	{
	  retval->read_file(realfile);
	}
    }

  FREE_MEMORY(realfile);

  return retval;
}

