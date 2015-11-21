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
#ifndef _statistics_h
#define _statistics_h


enum StatName {STAT_SCOPE, 
		 STAT_CRITERIA, 
		 STAT_REBUILT_FILES, 
		 STAT_REBUILT_PROJECTS,
		 STAT_STATS_INFO,
		 STAT_METRICS_INFO,
		 STAT_CC_THRESH,
		 STAT_SWITCHES_THRESH,
		 STAT_SHALLOW,
		 STAT_STANDARD_DEV,
		 STAT_FILE_INFO, 
		 STAT_PROJECT_INFO,
		 STAT_VAR_INFO,
		 STAT_TYPEDEF_INFO,
		 STAT_FUNCT_INFO,
		 STAT_CLASS_INFO,
		 STAT_TEMPLATE_INFO,
		 STAT_MACRO_INFO,
		 STAT_ENUM_INFO,
		 STAT_UNION_INFO,
		 STAT_OUTPUT_FILE,
		 STAT_OUTPUT_FORMAT,
		 NUM_OF_STATS};

enum StatType {BOOL_STAT, 
		 TEXT_STAT, 
		 INT_STAT,
		 OPTION_STAT};
    

class gtPrimitive;

struct StatInfo {
  char* string;
  StatType type;
  gtPrimitive* widget;
  StatInfo(char* s, StatType t, gtPrimitive* w)
    :string(s), type(t), widget(w) {}
};

int statistics_list_init();

#endif // _statistics_h
