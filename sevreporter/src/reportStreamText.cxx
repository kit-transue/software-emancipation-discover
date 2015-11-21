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
// the report in plain text format.
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

//++OSPORT
#include <pdumem.h>
#include <raFile.h>
#include <pdustring.h>
#include <pduio.h>
//--OSPORT

//++SEV
#include <reportStreamText.h>
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
//--SEV

//
// Assumptions:
//
int const reportStreamText::page_length   = 66;
int const reportStreamText::page_width    = 80;

int const reportStreamText::left_margin   = 6;
int const reportStreamText::right_margin  = 75;
int const reportStreamText::top_margin    = 6;
int const reportStreamText::bottom_margin = 60;


/////////////////////////////////////////////////////////////////////////////////
//
// Constructors/Destructors
//
////////////////////////////////////////////////////////////////////////////////

reportStreamText::reportStreamText(char const *_filename)
{
  reset();
  filename = _filename;
  outfile  = 0;
  copts    = new charOpts;
}

reportStreamText::~reportStreamText(void)
{
  FREE_OBJECT(outfile);
  FREE_MEMORY(filename);
  FREE_OBJECT(copts);
}

////////////////////////////////////////////////////////////////////////////////

//
// prints the title page
//
int reportStreamText::print_title_page(titlePageData *tpd)
{
  int retval = -1;

  char const *temp1 = 0;
  char const *temp2 = 0;
  char const *temp3 = 0;

  if ( tpd )
    {
      if ( outfile )
	{
	  set_mode(CODE);
	  pass_top_margin();
      	  int quarter = (bottom_margin - top_margin) / 4;
	  for ( int i = 0; i < quarter; i++ )
	    end_line();
      
	  if ( temp1 = center_line(tpd->title) )
	    {
	      add_string(temp1);

	      for ( int i = 0; i < 2 * quarter; i++ )
		end_line();
	      temp2 = tpd->date();
	      temp3 = center_line(temp2);
	      add_string(temp3);
	      
	      end_page();
	      retval = 1;
	    }
	  set_mode(TEXT);
	}
    }

  FREE_MEMORY(temp1);
  FREE_MEMORY(temp2);
  FREE_MEMORY(temp3);

  return retval;
}

//
// start a report (open file)
//
int reportStreamText::start_report(reportHeader *)
{
  int retval = -1;

  if ( reset() )
    {
      if ( outfile || open_outfile(filename) )
	retval = 1;
    }

  return retval;
}

//
// end a report (close file)
//
int reportStreamText::end_report(reportFooter *)
{
  int retval = -1;
  
  if ( outfile )
    {
      end_page();
      outfile->close(0);
      retval = 1;
    }

  FREE_OBJECT(outfile);

  return retval;
}

//
// start a new section
//
int reportStreamText::start_section(sectionHeader *secthead)
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
	if ( ! at_top_of_page() )
	  end_page();

      // if not at least 4 lines left, then start new page
      if ( lines_remaining() < 4 )
	end_page();

      // print the section number X.Y.Z
      for ( int i = 0; i <= get_section_depth(); i++ )
	{
	  char const *a = pdinttoa(get_section_number(i));
	  add_string(a);
	  if ( i != get_section_depth() ) add_string(".");
	  FREE_MEMORY(a);
	}
      
      hard_space();
      add_string(secthead->title);
      end_paragraph();

      retval = 1;
    }
  
  return retval;
}

//
// end a section
//
int reportStreamText::end_section(sectionFooter *)
{
  end_of_section();
  return 1;
}

//
// Dislplay a quality entity
//
int reportStreamText::print_quality(qualityItem *qi)
{
  int retval = -1;

  if ( qi )
    {
      add_string("Quality: ");
      add_string(qi->score());
      add_string(", Weight: ");
      add_string(qi->weight());
      
      if ( pdstrcmp(qi->found(), "N/A") != 0 )
	{
	  add_string(", Found: ");
	  add_string(qi->found());
	  add_string(" / ");
	  add_string(qi->outof());
	  add_string(" (");
	  add_string(qi->units());
	  add_string(")");
	}

      retval = 0;
    }

  return retval;
}

//
// display a quality "graph"
//
int reportStreamText::graph_quality(qualityItem *head, queryFolder *qdata)
{
  int retval = -1;

  if ( qdata )
    {
      // make sure that we are in CODE mode
      textmode old = set_mode(CODE);

      // output header
      char const *temp = center_line(rptgenArg_value(rptgenArg_GRAPH_TITLE));
      if ( temp )
	{
	  add_string(temp);
	  FREE_MEMORY(temp);
	  end_paragraph();
	}

      //
      // create "pseudo graph" from data
      //

      // print graph header
      add_string("|Category       0%        25%        50%        75%       100%");
      end_line();
      add_string("|===============|==========|==========|==========|==========|");
      end_line();
      
      // for each line of data, write the tag and graph the score
      for ( int i = 0; i <= qdata->numItems(); i++ )
	{
	  qualityItem *qi = 0;
	  
	  // first time through, use head, else use qdata
	  if ( i == 0 )
	    qi = head;
	  else
	    qi = (qualityItem *) qdata->getItem(i-1); // this should be safe!
	  
	  if ( qi )
	    {
	      // write the description of the quality item
	      add_char('|');
	      add_string(padField(qi->name(), 15, ' ', 0));
	      add_char('|');

	      // write the quality score
	      char chars[45];
	      pdmemset(chars, ' ', 45);
	      int  numchars = (((pdatoint(qi->score())*10)/25)*11)/10; 
	      pdmemset(chars, '-', numchars);
	      chars[10] = '|';
	      chars[21] = '|';
	      chars[32] = '|';
	      chars[43] = '|';
	      chars[44] = 0;
	      add_string(chars);
	      end_line();
	    }
	}

      add_string("|===============|==========|==========|==========|==========|");
      end_line();

      // return to previous mode
      set_mode(old);

      retval = 0;
    }
  
  return retval;
}

//
// print the quality table
//
int reportStreamText::print_quality_table(qualityItem *head, queryFolder *qdata)
{
  int retval = -1;

  if ( head && qdata )
    {
      // if header will be a widow, end the page
      if ( lines_remaining() < 3 )
	end_page();

      // set to CODE mode so that formatting is preserved
      textmode oldmode = set_mode(CODE);

      // print table header
      add_string("Category Name           Quality Weight Found   Out Of  Units");
      end_line();
      add_string("======================= ======= ====== ======= ======= =====");
      end_line();

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
	      // write category name column
	      add_string(padField(qi->name(), 23, ' ', 0));
	      add_char(' ');

	      // write quality column
	      add_string(padField(qi->score(), 7, ' ', 1));
	      add_char(' ');

	      // write weight column
	      add_string(padField(qi->weight(), 6, ' ', 1));
	      add_char(' ');

	      // write found column
	      add_string(padField(qi->found(), 7, ' ', 1));
	      add_char(' ');

	      // write out of column
	      add_string(padField(qi->outof(), 7, ' ', 1));
	      add_char(' ');

	      // write units column
	      add_string(padField(qi->units(), 5, ' ', 1));

	      // end the line
	      end_line();
	    }
	}

      // return to old mode
      set_mode(oldmode);
      
      retval = 0;
    }

  return retval;
}

//
// print the module table
//
int reportStreamText::print_inputs(inputs *inp, int can_defer)
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
      int maxcols = 60 / maxlen;
      if ( maxcols == 0 )
	maxcols = 1;

      // compute the number of rows in the table
      int maxrows = inp->numItems() / maxcols;
      if ( maxrows == 0 )
	maxrows = 1;

      // if at least 3 lines won't fit, start new page
      if ( lines_remaining() < 3 )
	end_page();
      
      // compute the maximum column width
      int maxwidth = (60 - maxcols + 1) / maxcols;
      
      // set to CODE mode so that formatting is preserved
      textmode oldmode = set_mode(CODE);
      
      for ( i = 0; i < maxrows; i++ )
	{
	  for ( int j = 0; j < maxcols; j++ )
	    {
	      reportItem *mi = inp->getItem(i*maxcols+j);
	      if ( mi )
		{
		  if ( j )
		    add_char(' ');
		  add_string(padField(mi->name(), maxwidth, ' ', 0));
		}
	    }
	  end_line();
	}
      
      
      // return to old mode
      set_mode(oldmode);

      retval = 0;
    }

  return retval;
}

//
// print a table of metric data
//
int reportStreamText::print_metrics(metricFolder *mdata)
{
  int retval = -1;

  if ( mdata )
    {
      // if header will be a widow, end the page
      if ( lines_remaining() < 3 )
	end_page();
	  
      // set to CODE mode so that formatting is preserved
      textmode oldmode = set_mode(CODE);
      
      // print table header
      add_string("Metric Name              High Val Low Val  Average  Std Dev ");
      end_line();
      add_string("======================== ======== ======== ======== ========");
      end_line();

      // print each line
      for ( int i = 0; i < mdata->numItems(); i++ )
	{
	  metric *mi = (metric *) mdata->getItem(i);
	  if ( mi )
	    {
	      // write name column
	      add_string(padField(mi->name(), 24, ' ', 0));
	      add_char(' ');

	      // write high value
	      add_string(padField(mi->high(), 8, ' ', 1));
	      add_char(' ');

	      // write low value
	      add_string(padField(mi->low(), 8, ' ', 1));
	      add_char(' ');

	      // write average value
	      add_string(padField(mi->average(), 8, ' ', 1));
	      add_char(' ');

	      // write standard deviation
	      add_string(padField(mi->stddev(), 8, ' ', 1));
	    }
	  end_line();
	}

      // return to old mode
      set_mode(oldmode);
  
      retval = 0;
    }
  
  return retval;
}

int reportStreamText::print_inventory(inventory *inv)
{
  int retval = -1;

  if ( inv )
    {
      // must have > 3 lines
      if ( lines_remaining() < 3 )
	end_page();

      int n = inv->numItems();
      for ( int i = 0; i < n; i++ )
	{
	  inventoryItem *ii = (inventoryItem *) inv->getItem(i);
	  if ( ii )
	    {
	      add_string(padField(ii->name(), 48, '.', 0));
	      add_string(padField(ii->value(), 17, '.', 1));
	    }
	  end_line();
	}
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

int reportStreamText::hard_space(void)
{
  write(' ');
  column++;
  if ( column > right_margin )
    end_line();
  return 1;
}

//
// Forces a newline into the report stream
//

int reportStreamText::end_line(void)
{
  // mark that last operation was EOL
  dideol = 1;

  // if we are already at top of form, then dont add blank lines to report
  write('\n');
  row++;
  column = 1;
  if ( row >= bottom_margin )
    end_page();

  return 1;
}

//
// Ends a paragraph 
//
int reportStreamText::end_paragraph(void)
{
  // if we just did an end line, the only do 1 more, else do 2
  if ( ! last_was_end_line() )
    end_line();
  end_line();

  return 1;
}

//
// Forces a form-feed into the report stream
//
int reportStreamText::end_page(void)
{
  write((char)12);
  column = 1;
  row    = 1;
  top    = 1;
  dideol = 1;
  return 1;
}

//
// how to handle text
//
int reportStreamText::handle_text(textmode m, char const *str)
{
  int retval = -1;

  if ( str )
    {
      // writing text onto the stream will always mean not top of form and not eol
      top    = 0;

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

int reportStreamText::handle_whitespace(textmode m, char const *str)
{
  int retval = -1;

  if ( str )
    {
      // just to differentiate from above, writing whitespace will not necessarily
      // mean clearing the top of form (wholly whitespace lines at top of forms
      // will be discarded.)
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

int reportStreamText::handle_code_whitespace(char const *ws)
{
  int retval = -1;

  if ( ws )
    {
      dideol = 0;

      for ( int i = 0; i < pdstrlen(ws); i++ )
	switch ( ws[i] )
	  {
	  case '\t':
	  case ' ':
	    if ( row <= top_margin )
	      pass_top_margin();
	    if ( column <= left_margin )
	      pass_left_margin();
	    hard_space();
	    break;
	    
	  case '\n':
	    if ( ! at_top_of_page() )
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

int reportStreamText::handle_text_whitespace(char const *ws)
{
  int retval = -1;

  if ( ws )
    {
      dideol = 0;

      // 1+ spaces become 1 space
      int space_printed = 0;
      
      for ( int i = 0; i < pdstrlen(ws); i++ )
	switch ( ws[i] )
	  {
	  case '\t':
	  case ' ':
	    if ( ! space_printed )
	      {
		space_printed = 1;
		if ( row <= top_margin )
		  pass_top_margin();
		if ( column <= left_margin )
		  pass_left_margin();
		if ( column <= right_margin - 1 )
		  {
		    if ( column > left_margin + 1 )
		      hard_space();
		  }
		else
		  end_line();
	      }
	    break;
	    
	  case 12:
	    end_page();
	    break;

	  case '\r':
	  case '\n':
	    if ( ! space_printed )
	      {
		handle_text_whitespace(" ");
		space_printed = 1;
	      }
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

int reportStreamText::handle_code_text(char const *str)
{
  int retval = -1;

  if ( str )
    {
      dideol = 0;

      if ( row <= top_margin )
	pass_top_margin();
      if ( column <= left_margin )
	pass_left_margin();

      write(str);

      column += pdstrlen(str);
      if ( column >= right_margin )
	end_line();

      retval = 1;
    }

  return retval;
}

//
// how to handle text in TEXT mode
//

int reportStreamText::handle_text_text(char const *str)
{
  int retval = -1;

  if ( str )
    {
      dideol = 0;

      if ( row <= top_margin )
	pass_top_margin();
      if ( column <= left_margin )
	pass_left_margin();

      // compute room left on line
      int room;
      if ( column <= left_margin )
	room = right_margin - left_margin - 1;
      else
	room = right_margin - column;

      // can it fit?
      if ( room >= pdstrlen(str) )
	{
	  // yes, put in on the line
	  write(str);
	  column += pdstrlen(str);
	  if ( column >= right_margin )
	    end_line();
	}
      else
	{
	  // no, goto next line and write
	  end_line();
	  if ( row <= top_margin )
	    pass_top_margin();
	  if ( column <= left_margin )
	    pass_left_margin();
	  write(str);
	  column += pdstrlen(str);
	  if ( column >= right_margin )
	    end_line();
	}
      retval = 1;
    }
  
  return retval;
}

//
// handles character change requests
//

charOpts *reportStreamText::getCharOpts(void)
{
  return copts;
}

charOpts *reportStreamText::setCharOpts(charOpts *opts)
{
  charOpts *retval = copts;
  copts = opts;
  return retval;
}

//
// function to write directly to file
//

int reportStreamText::write(char const *str)
{
  int retval = -1;

  if ( str && outfile )
    retval = outfile->write((char *)str, pdstrlen(str));
      
  return retval;
}

int reportStreamText::write(char c)
{
  int retval = -1;

  if ( outfile )
    retval = outfile->write(&c, 1);

  return retval;
}

 
int reportStreamText::pass_left_margin(void)
{
  if ( column <= left_margin )
    for ( int i = column; i <= left_margin; i++ )
      hard_space();
  return 1;
}

int reportStreamText::pass_top_margin(void)
{
  if ( row <= top_margin )
    for ( int i = column; i <= top_margin; i++ )
      end_line();
  return 1;
}

int reportStreamText::lines_remaining(void)
{
  return bottom_margin - row;
}

int reportStreamText::columns_remaining(void)
{
  return right_margin - column;
}

int reportStreamText::at_top_of_page(void)
{
  return (row <= top_margin);
}

int reportStreamText::last_was_end_line(void)
{
  return (dideol == 1);
}

////////////////////////////////////////////////////////////////////////////////
//
// Private functions
//
////////////////////////////////////////////////////////////////////////////////

//
// The function returns a string that represents the passed string
// centered on a line of the report. Memory is allocated that must
// be freed by the caller.
//

char const *reportStreamText::center_line(char const *text)
{
  char *retval = 0;

  if ( text )
    {
      int width   = right_margin - left_margin;
      int padding = ( width - pdstrlen(text) ) / 2;
      retval      = GET_MEMORY(char, width + 1);
      if ( retval )
	{
	  for ( int i = 0; i < padding; i++ )
	    retval[i] = ' ';
	  retval[padding] = 0;
	  pdstrcat(retval, text);
	}
    }
  
  return retval;
}

//
// This function resets an instance of the object
//

int reportStreamText::reset(void)
{
  column = 1;
  row    = 1;
  top    = 1;
  dideol = 1;
  return 1;
}

int reportStreamText::open_outfile(char const *file)
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

//
// Function to pad a string to an approprite size and justification
// Does the following:
//    s is the string you wish padded
//    size is the size you want it padded to
//    pad is the character you want the field padded with
//    rightjust in a boolean value:  0 == left justified, != 0 == right-justified
//
//    If s is too large it will truncate it and put ... at the end to indicate that
//    
//  the return value is memory that is only good until the next call to padField!!!
//  it should *NOT* be stored or relied upon to be around later.
//
char const *reportStreamText::padField(char const *s, int size, char pad, int rightjust)
{
  static char *retval = 0;

  FREE_MEMORY(retval);

  if ( s )
    {
      retval = GET_MEMORY(char, size+1);
      if ( retval )
	{
	  int slen = pdstrlen(s);

	  if ( slen > size )
	    {
	      pdstrncpy(retval, s, size);
	      if ( size > 4 )
		{
		  retval[size-3] = '.';
		  retval[size-2] = '.';
		  retval[size-1] = '.';
		}
	      retval[size] = 0;
	    }
	  else 
	    {
	      pdmemset(retval, pad, size);
	      pdstrncpy(retval + (size - slen) * (rightjust ? 1 : 0), s, slen);
	      retval[size] = 0;
	    }
	}
    }

  return retval;
}
