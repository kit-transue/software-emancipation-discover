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
// This code is a -*- C++ -*- header file

#ifndef _reporterArgs_h
#define _reporterArgs_h

enum rptgenArg_ID 
{
  rptgenArg_ID_NULL = 0,
  
  rptgenArg_REPORT_TEMPLATE,
  rptgenArg_REPORT_FORMAT,
  rptgenArg_REPORT_DATA,
  rptgenArg_REPORT_OUTPUT,

  rptgenArg_DEBUG_DUMP,

  rptgenArg_GIF_TO_PNG_EXEC,
  rptgenArg_PERL_EXEC,
  rptgenArg_PERL_INCL,
  rptgenArg_PERL_TEMPLATE,

  rptgenArg_GRAPH_WIDTH,
  rptgenArg_GRAPH_HEIGHT,
  rptgenArg_GRAPH_FG_COLOR,
  rptgenArg_GRAPH_TEXT_COLOR,
  rptgenArg_GRAPH_BAR_COLOR,
  rptgenArg_GRAPH_TITLE,
  rptgenArg_GRAPH_MAX_Y_VAL,
  rptgenArg_GRAPH_NUM_Y_VALS,

  rptgenArg_SUPPRESS_QUERY_TEXT,

  rptgenArg_ID_LIST_SIZE
};

// -- interface routines for argument information

class argParam;

extern int         rptgenArg_xname(int, char **);
extern int         rptgenArg_init_values(argParam *);
extern int         rptgenArg_set_value(int, char const *);
extern char const *rptgenArg_default(int);
extern char const *rptgenArg_value(int);

#endif
// _reporterArgs_h

