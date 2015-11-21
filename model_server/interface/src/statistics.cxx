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

#include <statistics.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

StatInfo* StatisticsList[NUM_OF_STATS];

int statistics_list_init() 
{
  static int once = 0;
  if (!once) {
    once ++;
    StatisticsList[STAT_SCOPE] = new StatInfo("STAT_SCOPE", TEXT_STAT, NULL);
    StatisticsList[STAT_CRITERIA] = new StatInfo("STAT_CRITERIA", TEXT_STAT, NULL);
    StatisticsList[STAT_REBUILT_FILES] = new StatInfo("STAT_REBUILT_FILES", BOOL_STAT, NULL);
    StatisticsList[STAT_REBUILT_PROJECTS] = new StatInfo("STAT_REBUILT_PROJECTS", BOOL_STAT, NULL);
    StatisticsList[STAT_STATS_INFO] = new StatInfo("STAT_STATS_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_METRICS_INFO] = new StatInfo("STAT_METRICS_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_CC_THRESH] = new StatInfo("STAT_CC_THRESH", INT_STAT, NULL);
    StatisticsList[STAT_SWITCHES_THRESH] = new StatInfo("STAT_SWITCHES_THRESH", INT_STAT, NULL);
    StatisticsList[STAT_SHALLOW] = new StatInfo("STAT_SHALLOW", BOOL_STAT, NULL);
    StatisticsList[STAT_STANDARD_DEV] = new StatInfo("STAT_STANDARD_DEV",BOOL_STAT, NULL);
    StatisticsList[STAT_PROJECT_INFO] = new StatInfo("STAT_PROJECT_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_VAR_INFO] = new StatInfo("STAT_VAR_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_FUNCT_INFO] = new StatInfo("STAT_FUNCT_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_TYPEDEF_INFO] = new StatInfo("STAT_TYPEDEF_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_FILE_INFO] = new StatInfo("STAT_FILE_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_CLASS_INFO] = new StatInfo("STAT_CLASS_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_TEMPLATE_INFO] = new StatInfo("STAT_TEMPLATE_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_MACRO_INFO] = new StatInfo("STAT_MACRO_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_ENUM_INFO] = new StatInfo("STAT_ENUM_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_UNION_INFO] = new StatInfo("STAT_UNION_INFO", BOOL_STAT, NULL);
    StatisticsList[STAT_OUTPUT_FILE] = new StatInfo("STAT_OUTPUT_FILE", TEXT_STAT, NULL);
    StatisticsList[STAT_OUTPUT_FORMAT] = new StatInfo("STAT_OUTPUT_FORMAT", OPTION_STAT, NULL);
  }
  return 1;
}

char* PrefsFileName = "/.DISCOVER.summary.prefs";
