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
//////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////

//++OSPORT
#include <pduio.h>
//--OSPORT

//++SEV
#include <reportDataParser.h>
#include <rootFolder.h>
#include <queryFolder.h>
#include <metricFolder.h>
#include <query.h>
#include <metricQuery.h>
#include <inputs.h>
#include <inventory.h>
#include <inputItem.h>
#include <inventoryItem.h>
//--SEV

//////////////////////////////////////////////////////////////////////////////////
//
// Parsing callbacks
//
//////////////////////////////////////////////////////////////////////////////////

extern "C" rootFolder *create_rootFolder(char *weight, char *score, char *desc)
{
  rootFolder *retval = 0;

  if ( weight && score && desc )
    {
      retval = new rootFolder(desc, score, weight);
    }

  return retval;
}

extern "C" int insertQueryFolders_rootFolder(rootFolder *rf, reportItemList *ril)
{
  int retval = -1;

  if ( rf && ril )
    {
      int n = ril->numItems();
      int i, error;
      for ( i = 0, error = 0; i < n && ! error; i++ )
	{
	  if ( rf->add_query_folder((queryFolder *)ril->getItem(i)) < 0 )
	    error = 1;
	}
      retval = i;
    }

  return retval;
}

extern "C" int insertMetricFolder_rootFolder(rootFolder *rf, metricFolder *m)
{
  int retval = -1;

  if ( rf && m )
    retval = rf->add_metric_folder(m);

  return retval;
}

extern "C" queryFolder *create_queryFolder(char *name, char *weight, char *score, char *desc)
{
  queryFolder *retval = 0;

  if ( name && weight && score && desc )
    {
      retval = new queryFolder(name, desc, score, weight);
    }

  return retval;
}

extern "C" int insertItems_queryFolder(queryFolder *qf, reportItemList *ril)
{
  int retval = -1;

  if ( qf && ril )
    {
      int n = ril->numItems();
      int i, error;
      for ( i = 0, error = 0; i < n && ! error; i++ )
	{
	  if ( qf->insertItem(ril->getItem(i)) < 0 )
	    error = 1;
	}
      retval = i;
    }

  return retval;
}

extern "C" metricFolder *create_metricFolder(char *name, char *desc)
{
  metricFolder *retval = 0;

  if ( desc )
    {
      retval = new metricFolder(name, desc);
    }

  return retval;
}

extern "C" int insertItems_metricFolder(metricFolder *mf, reportItemList *ril)
{
  int retval = -1;

  if ( mf && ril )
    {
      int n = ril->numItems();
      int i, error;
      for ( i = 0, error = 0; i < n && ! error; i++ )
	{
	  if ( mf->insertItem(ril->getItem(i)) < 0 )
	    error = 1;
	}
      retval = i;
    }

  return retval;
}

extern "C" reportItemList *create_reportItemList(void)
{
  return new reportItemList(32);
}

extern "C" int insertItem_reportItemList(reportItemList *ril, reportItem *ri)
{
  int retval = -1;

  if ( ril && ri )
    {
      retval = ril->insertItem(ri);
    }

  return retval;
}

extern "C" query *create_query(char *name, char *desc, char *score, char *weight, char *found, char *outof, char *units)
{
  query *retval = 0;

  if ( name && desc && score && weight && found && outof && units )
    {
      retval = new query(name, desc, score, weight, found, outof, units);
    }

  return retval;
}

extern "C" metric *create_metric(char *name, char *desc, char *high, char *low, char *avg, char *stddev)
{
  metric *retval = 0;

  if ( name && desc && high && low && avg && stddev )
    {
      retval = new metric(name, desc, high, low, avg, stddev);
    }

  return retval;
}

extern "C" inputs *create_inputs(void)
{
  return new inputs();
}

extern "C" int insert_inputs(inputs *inp, char *module)
{
  int retval = -1;

  if ( inp && module )
    {
      inputItem *i = new inputItem(module);
      if ( module)
	retval = inp->insertItem(i);
    }

  return retval;
}

extern "C" inventory *create_inventory(void)
{
  return new inventory();
}

extern "C" int insert_inventory(inventory *inv, char *name, char *value)
{
  int retval = -1;

  if ( inv && name && value )
    {
      inventoryItem *i = new inventoryItem(name, value);
      if ( i )
	retval = inv->insertItem(i);
    }

  return retval;
}

extern "C" char *merge_name_and_thresh(char *name, char *thresh)
{
  char *retval = 0;

  if ( name && thresh )
    retval = pdmpack("ss", "%s > %s", name, thresh);

  return retval;
}

