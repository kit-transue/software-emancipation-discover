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
#ifndef _reportDataParser_h
#define _reportDataParser_h

#ifndef __cplusplus

/* typedef the class definitions to void so that C will not gag on them */

typedef void inputs;
typedef void inventory;
typedef void metric;
typedef void metricFolder;
typedef void query;
typedef void queryFolder;
typedef void reportItem;
typedef void reportItemList;
typedef void rootFolder;

#else

/* forward reference the classes in C++ */

class inputs;
class inventory;
class metric;
class metricFolder;
class query;
class queryFolder;
class reportItem;
class reportItemList;
class rootFolder;

#endif

#ifdef __cplusplus
extern "C"
{
#endif

  int             reportDataParse(char *, rootFolder **, inventory **, inputs **);
  rootFolder     *create_rootFolder(char *, char *, char *);
  int             insertQueryFolders_rootFolder(rootFolder *, reportItemList *);
  int             insertMetricFolder_rootFolder(rootFolder *, metricFolder *);
  queryFolder    *create_queryFolder(char *, char *, char *, char *);
  int             insertItems_queryFolder(queryFolder *, reportItemList *);
  metricFolder   *create_metricFolder(char *, char *);
  int             insertItems_metricFolder(metricFolder *, reportItemList *);
  reportItemList *create_reportItemList(void);
  int             insertItem_reportItemList(reportItemList *, reportItem *);
  query          *create_query(char *, char *, char *, char *, char *, char *, char *);
  metric         *create_metric(char *, char *, char *, char *, char *, char *);
  inputs         *create_inputs(void);
  int             insert_inputs(inputs *, char *);
  inventory      *create_inventory(void);
  int             insert_inventory(inventory *, char *, char *);
  char           *merge_name_and_thresh(char *, char *);
  
#ifdef __cplusplus
}
#endif

#endif
