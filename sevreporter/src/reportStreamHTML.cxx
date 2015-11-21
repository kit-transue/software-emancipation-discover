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
// This is the implementation of the report stream that will output
// the report in HTML format.
//
////////////////////////////////////////////////////////////////////////////////

//++C
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C

//++C++
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
#include <pdumem.h>
#include <raFile.h>
#include <pdustring.h>
#include <pduio.h>
#include <dirFSet.h>
//--OSPORT

//++SEV
#include <reportStreamHTML.h>
#include <titlePageData.h>
#include <sectionHeader.h>
#include <qualityItem.h>
#include <queryFolder.h>
#include <reporterArgs.h>
#include <inputs.h>
#include <metricFolder.h>
#include <metricQuery.h>
#include <charOpts.h>
#include <inventory.h>
#include <inventoryItem.h>
#include <chartImage.h>
//--SEV


/////////////////////////////////////////////////////////////////////////////////
//
// Constructors/Destructors
//
////////////////////////////////////////////////////////////////////////////////

reportStreamHTML::reportStreamHTML(char const *_filename)
{
  lastmode = TEXT;
  filename = _filename;
  outfile  = 0;
  copts    = new charOpts;
}

reportStreamHTML::~reportStreamHTML(void)
{
  FREE_OBJECT(outfile);
  FREE_MEMORY(filename);
  FREE_OBJECT(copts);
}

////////////////////////////////////////////////////////////////////////////////

//
// prints the title page
//
int reportStreamHTML::print_title_page(titlePageData *tpd)
{
  int retval = -1;

  if ( tpd )
    {
      if ( outfile )
	{
	  write("<H1><CENTER>");
	  write(tpd->title);
	  write("</CENTER></H1>\n");
	  write("<H3><CENTER>");
	  write(tpd->date());
	  write("</CENTER></H3>\n");
	  write("<HR>\n\n");
	}
    }

  return retval;
}

//
// start a report (open file)
//
int reportStreamHTML::start_report(reportHeader *)
{
  int retval = -1;

  if ( outfile || open_outfile(filename) >= 0 )
    {
      write("<HTML>\n");
      write("<HEAD>\n");
      write("<TITLE>QAR</TITLE>\n");
      write("</HEAD>\n");
      write("<BODY>\n");
      retval = 1;
    }

  return retval;
}

//
// end a report (close file)
//
int reportStreamHTML::end_report(reportFooter *)
{
  int retval = -1;
  
  if ( outfile )
    {
      write("</BODY>\n");
      write("</HTML>\n");
      outfile->close(0);
      retval = 1;
    }

  FREE_OBJECT(outfile);

  return retval;
}

//
// start a new section
//
int reportStreamHTML::start_section(sectionHeader *secthead)
{
  int retval = -1;
  
  if ( secthead )
    {
      // return to text mode, if not in it
      if ( get_mode() == CODE )
	set_mode(TEXT);

      // increment counter
      start_of_section();

      // if start of major section, start new page
      if ( get_section_depth() == 0 )
	write("<HR>\n");

      // put section tag
      char *stag = pdmpack("d", "<H%d>", get_section_depth()+1);
      char *etag = pdmpack("d", "</H%d>\n", get_section_depth()+1);

      // print the section number X.Y.Z
      write(stag);
      for ( int i = 0; i <= get_section_depth(); i++ )
	{
	  char const *a = pdinttoa(get_section_number(i));
	  write(a);
	  if ( i != get_section_depth() ) write(".");
	  FREE_MEMORY(a);
	}
      
      hard_space();
      write("<A NAME=\"");
      write(cdatafy(secthead->title));
      write("\">");
      write(secthead->title);
      write("</A>");
      write(etag);

      FREE_MEMORY(stag);
      FREE_MEMORY(etag);

      retval = 1;
    }
  
  return retval;
}

//
// end a section
//
int reportStreamHTML::end_section(sectionFooter *)
{
  end_of_section();
  return 1;
}

//
// Dislplay a quality entity
//
int reportStreamHTML::print_quality(qualityItem *qi)
{
  int retval = -1;

  if ( qi )
    {
      write("<EM>");
      write("Quality: ");
      write(qi->score());
      write(", Weight: ");
      write(qi->weight());
      
      if ( pdstrcmp(qi->found(), "N/A") != 0 )
	{
	  write(", Found: ");
	  write(qi->found());
	  write(" / ");
	  write(qi->outof());
	  write(" (");
	  write(qi->units());
	  write(")");
	}
      write("</EM>\n");

      retval = 0;
    }

  return retval;
}

//
// display a quality "graph"
//
int reportStreamHTML::graph_quality(qualityItem *head, queryFolder *qdata)
{
  int retval = -1;

  chartImage *image          = 0;
  char const *imagename_base = 0;
  char const *imagename      = 0;
  static int last_number     = 0;

  // get the image name
  if ( imagename = pdmpack("sd", "%s.chart%d.gif", rptgenArg_value(rptgenArg_REPORT_OUTPUT), last_number++) )
    {
      // get the image
      if ( image = new chartImage )
	{
	  // plot the image
	  if ( image->plot_data(head, qdata, imagename, chartImage::gif) >= 0 )
	    {
	      // get the report base name
	      dirFSet imagefilename((char *)imagename);
	      if ( imagename_base = imagefilename.filename() )
		{
		  // write the html
		  write("<P><CENTER><IMG SRC=\"");
		  write(imagename_base);
		  write("\"></CENTER></P>\n");
		  
		  retval = 0;
		}
	    }
	}
    }

  FREE_MEMORY(imagename);
  FREE_OBJECT(image);

  return retval;
}

//
// print the quality table
//
int reportStreamHTML::print_quality_table(qualityItem *head, queryFolder *qdata)
{
  int retval = -1;

  if ( head && qdata )
    {
      write("<P><CENTER>\n");
      write("<TABLE WIDTH=\"90%\" BORDER=1 CELLPADDING=3>\n");
      write("<TR>");
      write("<TH>Category Name</TH>\n");
      write("<TH>Quality</TH>\n");
      write("<TH>Weight</TH>\n");
      write("<TH>Found</TH>\n");
      write("<TH>Out Of</TH>\n");
      write("<TH>Units</TH>\n");

      // for each entry in qdata, print the line
      for ( int i = 0; i <= qdata->numItems(); i++ )
	{
	  qualityItem *qi = 0;
	  
	  // first time through, use head, else use qdata
	  if ( i == 0 )
	    qi = head;
	  else
	    qi = (qualityItem *) qdata->getItem(i-1);

	  if ( qi )
	    {
	      // start row
	      write("<TR>");

	      // write category name column (with a link to appropriate section)
	      write("<TD>");
	      if ( i != 0 ) // for all but first, print a number
		{
		  write(pdinttoa(i));
		  write(". ");
		}
	      write("<A HREF=\"#");
	      write(cdatafy(qi->name()));
	      write("\">");
	      write(qi->name());
	      write("</A>");

	      // write quality column
	      write("<TD ALIGN=RIGHT>");
	      write(qi->score());

	      // write weight column
	      write("<TD ALIGN=RIGHT>");
	      write(qi->weight());

	      // write found column
	      write("<TD ALIGN=RIGHT>");
	      write(qi->found());

	      // write out of column
	      write("<TD ALIGN=RIGHT>");
	      write(qi->outof());

	      // write units column
	      write("<TD ALIGN=RIGHT>");
	      write(qi->units());
	    }
	}

      // end the table
      write("</TABLE>\n");
      write("</CENTER>\n");

      retval = 0;
    }

  return retval;
}

//
// print the module table
//
int reportStreamHTML::print_inputs(inputs *inp, int can_defer)
{
  int retval = -1;

  if ( inp )
    {
      // get the maximum length a module name
      int maxlen = 0;
      int i;
      for ( i = 0; i < inp->numItems(); i++ )
	{
	  reportItem *mi = inp->getItem(i);
	  if ( mi )
	    if ( maxlen < pdstrlen(mi->name()) )
	      maxlen = pdstrlen(mi->name());
	}

      // compute number of allowable columns
      int maxcols = 120 / maxlen;
      if ( maxcols == 0 )
	maxcols = 1;

      // compute the number of rows in the table
      int maxrows = inp->numItems() / maxcols;
      if ( maxrows == 0 )
	maxrows = 1;

      // start table
      write("<P><CENTER><TABLE WIDTH=\"75%\" BORDER=0 CELLPADDING=3>\n");

      // print the rows
      for ( i = 0; i < maxrows; i++ )
	{
	  write("<TR>");
	  for ( int j = 0; j < maxcols; j++ )
	    {
	      reportItem *mi = inp->getItem(i*maxcols+j);
	      if ( mi )
		{
		  write("<TD>");
		  write(mi->name());
		}
	    }
	}

      write("</TABLE>\n");
      write("</CENTER>\n");
      retval = 0;
    }

  return retval;
}

//
// print a table of metric data
//
int reportStreamHTML::print_metrics(metricFolder *mdata)
{
  int retval = -1;

  if ( mdata )
    {
      write("<P><CENTER>\n");
      write("<TABLE WIDTH=\"90%\" BORDER=1 CELLPADDING=3>\n");
      write("<TR>");
      write("<TH>Metric Name</TH>\n");
      write("<TH>High Value</TH>\n");
      write("<TH>Low Value</TH>\n");
      write("<TH>Average</TH>\n");
      write("<TH>Std. Deviation</TH>\n");

      // print each line
      for ( int i = 0; i < mdata->numItems(); i++ )
	{
	  metric *mi = (metric *) mdata->getItem(i);
	  if ( mi )
	    {
	      // start row
	      write("<TR>");

	      // write name column
	      write("<TD>");
	      write(mi->name());

	      // write high value
	      write("<TD ALIGN=RIGHT>");
	      write(mi->high());

	      // write low value
	      write("<TD ALIGN=RIGHT>");
	      write(mi->low());

	      // write average value
	      write("<TD ALIGN=RIGHT>");
	      write(mi->average());

	      // write standard deviation
	      write("<TD ALIGN=RIGHT>");
	      write(mi->stddev());
	    }
	}

      // end the table
      write("</TABLE>\n");
      write("</CENTER>\n");
      
      retval = 0;
    }
  
  return retval;
}

int reportStreamHTML::print_inventory(inventory *inv)
{
  int retval = -1;

  if ( inv )
    {
      write("<P><CENTER>\n");
      write("<TABLE WIDTH=\"75%\" BORDER=0 CELLPADDING=3>\n");

      int n = inv->numItems();
      for ( int i = 0; i < n; i++ )
	{
	  inventoryItem *ii = (inventoryItem *) inv->getItem(i);
	  if ( ii )
	    {
	      // start row
	      write("<TR>");

	      // write name column
	      write("<TD>");
	      write(ii->name());

	      // write value column
	      write("<TD ALIGN=RIGHT>");
	      write(ii->value());
	    }
	}

      // end the table
      write("</TABLE>\n");
      write("</CENTER>\n");
      
      retval = 0;
    }
  
  return retval;
}

////////////////////////////////////////////////////////////////////////////////
//
// Text-handling functions
//
////////////////////////////////////////////////////////////////////////////////

//
// Forces a space into the report stream
//

int reportStreamHTML::hard_space(void)
{
  write(' ');
  return 1;
}

//
// Forces a newline into the report stream
//

int reportStreamHTML::end_line(void)
{
  if ( get_mode() == CODE )
    write('\n');
  else
    write("<BR>\n");
  return 1;
}

//
// Ends a paragraph 
//
int reportStreamHTML::end_paragraph(void)
{
  write("<P>\n");
  return 1;
}

//
// Forces a form-feed into the report stream
//
int reportStreamHTML::end_page(void)
{
  write("<HR>\n");
  return 1;
}

//
// how to handle text
//
int reportStreamHTML::handle_text(textmode m, char const *str)
{
  int retval = -1;

  if ( str )
    {
      // handle mode switching
      if ( lastmode == TEXT && m == CODE )
	write("<PRE>");
      else if ( lastmode == CODE && m == TEXT )
	write("</PRE>");
      lastmode = m;

      if ( m == CODE )
	retval = handle_code_text(str);
      else
	retval = handle_text_text(str);
    }

  return retval;
}

//
// how to handle whitespace
//

int reportStreamHTML::handle_whitespace(textmode m, char const *str)
{
  int retval = -1;

  if ( str )
    {
      // handle mode switching
      if ( lastmode == TEXT && m == CODE )
	write("<PRE>");
      else if ( lastmode == CODE && m == TEXT )
	write("</PRE>");
      lastmode = m;

      if ( m == CODE )
	retval = handle_code_whitespace(str);
      else
	retval = handle_text_whitespace(str);
    }

  return retval;
}

//
// how to write writespace in CODE mode
//

int reportStreamHTML::handle_code_whitespace(char const *ws)
{
  int retval = -1;
  
  if ( ws )
    {
      for ( int i = 0; i < pdstrlen(ws); i++ )
	switch ( ws[i] )
	  {
	  case '\t':
	  case ' ':
	    hard_space();
	    break;
	    
	  case '\n':
	    end_line();
	    break;
	    
	  case 12:
	    end_page();
	    break;
	    
	  default:
	    break;
	  }
      retval = 1;
    }
  
  return retval;
}

//
// hot to write whitespace in TEXT mode
//

int reportStreamHTML::handle_text_whitespace(char const *ws)
{
  int retval = -1;

  if ( ws )
    {
      for ( int i = 0; i < pdstrlen(ws); i++ )
	switch ( ws[i] )
	  {
	  case '\t':
	  case ' ':
	    hard_space();
	    break;
	    
	  case 12:
	    end_page();
	    break;

	  case '\r':
	  case '\n':
	    handle_text_whitespace(" ");
	    break;
	    
	  default:
	    break;
	  }
      retval = 1;
    }
  
  return retval;
}

//
// how to handle text in CODE mode
//

int reportStreamHTML::handle_code_text(char const *str)
{
  int retval = -1;

  if ( str )
    {
      write(str);
      retval = 1;
    }

  return retval;
}

//
// how to handle text in TEXT mode
//

int reportStreamHTML::handle_text_text(char const *str)
{
  int retval = -1;

  if ( str )
    {
      // do HTML replacements
      char *temp1 = 0;
      char *temp2 = 0;
      char *temp3 = 0;
      char *temp4 = 0;

      pdstrgsub(str,   "&",  "&amp;",  &temp1);
      pdstrgsub(temp1, "\"", "&quot;", &temp2);
      pdstrgsub(temp2, "<",  "&lt;",   &temp3);
      pdstrgsub(temp3, ">",  "&gt;",   &temp4);
      
      write(temp4);

      FREE_MEMORY(temp1);
      FREE_MEMORY(temp2);
      FREE_MEMORY(temp3);
      FREE_MEMORY(temp4);

      retval = 1;
    }
  
  return retval;
}

//
// handles character change requests
//

charOpts *reportStreamHTML::getCharOpts(void)
{
  return copts;
}

charOpts *reportStreamHTML::setCharOpts(charOpts *opts)
{
  charOpts *retval = copts;
  copts = opts;
  return retval;
}

//
// function to write directly to file
//

int reportStreamHTML::write(char const *str)
{
  int retval = -1;

  if ( str && outfile )
    retval = outfile->write((char *)str, pdstrlen(str));
      
  return retval;
}

int reportStreamHTML::write(char c)
{
  int retval = -1;

  if ( outfile )
    retval = outfile->write(&c, 1);

  return retval;
}
 
int reportStreamHTML::open_outfile(char const *file)
{
  int retval = -1;

  outfile  = new raFile(file, 0);  
  if ( outfile )
    if ( outfile->open(raFile_ACCESS_WRITE, raFile_OPEN_CREATE | raFile_OPEN_TRUNC) >= 0 )
      retval = 0;
    else
      {
	cerr << "Error opening file " << file;
	cerr << ": " << pdstrerror(-1, 0) << '\n';
      }

  return retval;
}

char const *reportStreamHTML::cdatafy(char const *text)
{
  static char *retval = 0;

  if ( retval )
    FREE_MEMORY(retval);

  if ( text )
    {
      if ( retval = pdstrdup(text) )
	for ( char *i = retval; *i; i++ )
	  if ( ! isalnum(*i) )
	    *i = '_';
    }

  return retval;
}
      
