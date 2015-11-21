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

//++OSPORT
#include <pdumem.h>
#include <pdustring.h>
//--OSPORT

//++SEV
#include <rootFolder.h>
#include <reportTemplate.h>
#include <cstring.h>
#include <descParser.yacc.h>
#include <reportStream.h>
#include <titlePageData.h>
#include <inventory.h>
#include <inputs.h>
//--SEV

rootFolder::rootFolder(char const *_desc, char const *_score, char const *_weight) 
    : qualityItem(pdstrdup("Root Folder"), _desc, _score, _weight, 0, 0, 0)
{
  the_inventory = 0;
  the_inputs    = 0;
  queries       = 0;
  metrics       = 0;
}

rootFolder::~rootFolder(void)
{
  FREE_OBJECT(the_inventory);
  FREE_OBJECT(the_inputs);
}

//
// serializing the root will print the whole report
//
int rootFolder::serialize(reportStream *stream, reportTemplate *tagfile)
{
  int retval = -1;

  if ( stream )
    {
      // start the report
      stream->start_report(0);

      // send title page
      titlePageData tpd;
      tpd.title = "Quality Assessment Report";
      stream->print_title_page(&tpd);
      
      // send introduction
      parseDesc(tagfile->getDesc(desc()), stream);

      // send query folders
      if ( queries )
	{
	  int n = queries->numItems();
	  for ( int i = 0; i < n; i++ )
	    {
	      reportItem *child = queries->getItem(i);
	      if ( child )
		{
		  child->serialize(stream, tagfile);
		}
	    }
	}
      
      // send metric folder
      if ( metrics )
	metrics->serialize(stream, tagfile);
      
      // end the report
      stream->end_report(0);

      retval = 0;
    }

  return retval;
}

int rootFolder::onTag(int tag)
{
  int retval = -1;

  if ( current_stream )
    {
      switch ( tag )
	{
	case INPUTS:
	  retval = current_stream->print_inputs(the_inputs, 0);
	  break;
	  
	case INVENTORY:
	  retval = current_stream->print_inventory(the_inventory);
	  break;

	case OVERALL_QUALITY:
	  retval = current_stream->add_string(score());
	  break;

	case QUALITY_TABLE:
 	  retval = current_stream->print_quality_table(queries, queries);
 	  break;

 	case QUALITY_GRAPH:
	  retval = current_stream->graph_quality(queries, queries);
 	  break;

	default:
	  // pass the rest to base class
	  retval = qualityItem::onTag(tag);
	  break;
	}
    }
  
  return retval;
}

int rootFolder::store_inventory(inventory *_inventory)
{
  int retval = -1;

  if ( _inventory )
    {
      FREE_OBJECT(the_inventory);
      the_inventory = _inventory;
      retval = 0;
    }

  return retval;
}

int rootFolder::store_inputs(inputs *_inputs)
{
  int retval = -1;

  if ( _inputs )
    {
      FREE_OBJECT(the_inputs);
      the_inputs = _inputs;
      retval = 0;
    }

  return retval;
}
  

int rootFolder::add_metric_folder(metricFolder *m)
{
  int retval = -1;

  if ( m )
    {
      FREE_OBJECT(metrics);
      metrics = m;
      retval = 0;
    }

  return retval;
}

int rootFolder::add_query_folder(queryFolder *q)
{
  int retval = -1;

  if ( q )
    {
      if ( ! queries )
	queries = new queryFolder(pdstrdup("Overall Quality"), desc(), score(), weight());

      if ( queries )
	retval = queries->insertItem(q);
    }

  return retval;
}
