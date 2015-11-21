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
#include <argParam.h>
//--OSPORT

//++SEV
#include <reporterArgs.h>
//--SEV

static char *rptgenArg_info_recs[rptgenArg_ID_LIST_SIZE] = 
{
  0,

  "template",
  "type",
  "in",
  "out",

  "debugdump",

  "gif2pngexec",
  "perlexec", 
  "perlinc",
  "perl-script",

  "graph-width",
  "graph-height",
  "graph-fg-color",
  "graph-text-color",
  "graph-bar-color",
  "graph-title",
  "graph-max-y-val",
  "graph-num-y-vals",

  "suppress-queries",
};

static char const *rptgenArg_defaults[rptgenArg_ID_LIST_SIZE] = 
{
  0, //  rptgenArg_ID_NULL

  "$PSETHOME/lib/sevreport.tmpl",  // rptgenArg_REPORT_TEMPLATE
  "html",                          // rptgenArg_REPORT_FORMAT
  0,                               // rptgenArg_REPORT_DATA
  0,                               // rptgenArg_REPORT_OUTPUT

  0,                               // rptgenArg_DEBUG_DUMP
  
  "$PSETHOME/bin/gif2png",         // rptgenArg_GIF_TO_PNG_EXEC
  "$PSETHOME/bin/disperl",         // rptgenArg_PERL_EXEC
  "$PSETHOME/lib",                 // rptgenArg_PERL_INCL
  "$PSETHOME/lib/sevgraph.pl",     // rptgenArg_PERL_SCRIPT

  "540",                           // rptgenArg_GRAPH_WIDTH
  "225",                           // rptgenArg_GRAPH_HEIGHT
  "0x000000",                      // rptgenArg_GRAPH_FG_COLOR
  "0x000000",                      // rptgenArg_GRAPH_TEXT_COLOR
  "0x0088FF",                      // rptgenArg_GRAPH_BAR_COLOR
  "Quality Breakdown",             // rptgenArg_GRAPH_TITLE
  "100",                           // rptgenArg_GRAPH_MAX_Y_VAL
  "10",                            // rptgenArg_GRAPH_NUM_Y_VALS

  "0",                             // rptgenArg_SUPPRESS_QUERY_TEXT
};

static char const *rptgenArg_values[rptgenArg_ID_LIST_SIZE] = 
{
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};

//
// returns: -1: cannot return name for the specified argument, 0: the name is available
//
int rptgenArg_xname(int id_val, char **id_name)
{
  int retval = -1;

  if ( id_val > rptgenArg_ID_NULL && id_val < rptgenArg_ID_LIST_SIZE) 
    {
      if ( id_name )
	{
	  *id_name = rptgenArg_info_recs[id_val];
	}
      retval = 0;
    }

  return retval;
}

//
// returns: -1: cannot return default for the specified argument, 0: the default is available
//
char const *rptgenArg_default(int id_val)
{
  char const *retval = 0;

  if ( id_val > rptgenArg_ID_NULL && id_val < rptgenArg_ID_LIST_SIZE) 
    {
      retval = rptgenArg_defaults[id_val];
    }

  return retval;
}

//
// returns: 0 cannot return value for the specified argument, !0: the value
//
char const *rptgenArg_value(int id_val)
{
  char const *retval = 0;

  if ( id_val > rptgenArg_ID_NULL && id_val < rptgenArg_ID_LIST_SIZE) 
    {
      retval = rptgenArg_values[id_val];
    }

  return retval;
}

//
// returns: -1 could not set value, 0: value set
//
int rptgenArg_set_value(int id_val, char const *value)
{
  int retval = -1;

  if ( id_val > rptgenArg_ID_NULL && id_val < rptgenArg_ID_LIST_SIZE) 
    {
      rptgenArg_values[id_val] = value;
      retval = 1;
    }

  return retval;
}


//
// returns -1: values initialized, 0: values initialized
//
int rptgenArg_init_values(argParam *cmd_line_opts)
{
  int retval = -1;

  if ( cmd_line_opts )
    {
      for ( int i = rptgenArg_ID_NULL; i < rptgenArg_ID_LIST_SIZE; i++ )
	{
	  char const *from_cmd_line = cmd_line_opts->value(i, 0);
	  if ( from_cmd_line )
	    rptgenArg_values[i] = from_cmd_line;
	  else
	    rptgenArg_values[i] = rptgenArg_default(i);
	}
      retval = 0;
    }

  return retval;
}

	   

