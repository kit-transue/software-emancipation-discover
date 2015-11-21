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
#ifndef _rtlFilter_h
#define _rtlFilter_h

// rtlFilter.h
//------------------------------------------
// synopsis:
//------------------------------------------

#include <gtRTL_cfg.h>
#include <genString.h>

struct rtlFilterSortInfo {
    genString code;
    char direction;
};

struct rtlFilterFormatInfo {
    genString code;
    int  width;
};

class rtlFilter {

private:
  gtRTL_cfg def_table;

public:
  int getFilterCount ();
  char *getFilterName (int index);

  void genFilterInfo (	char **argv, 
			rtlFilterSortInfo **, 
			rtlFilterFormatInfo **, 
			char **formatExpression,
			char **filter, 
			char **hide,
			char **CLIExpression);

  static char *compute_sort_spec (struct rtlFilterSortInfo *s_info);
  static char *compute_format_spec (struct rtlFilterFormatInfo *f_info, char *formatExpression);
  static char *compute_filter_spec (char *filter, char *CLIExpression);

  static char *compute_sort_titles (struct rtlFilterSortInfo *s_info);
  static char *compute_format_titles (struct rtlFilterFormatInfo *f_info);

  void Create (char *newName, char *sort, char *format, char *filter, char *hide);
  void Rename (int index, const char *newName);
  void Save (int index);
  void Delete (int index);

  static void RTL_parse_specs (	rtlFilterFormatInfo **, 
				rtlFilterSortInfo **, 
				const char * format_spec,
				const char * sort_spec,
				const char * show_spec,
				genString &show_filt,
				genString &cli_filt,
				genString &fexpr);
};

#endif // _rtlFilter_h
