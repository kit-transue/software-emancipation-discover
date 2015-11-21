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

#ifndef _reportStreamText_h
#define _reportStreamText_h

#ifndef _reportStream_h
#include <reportStream.h>
#endif

class raFile;

class reportStreamText : public reportStream
{
public:

  // Constructor(s) / Destructor(s)
  reportStreamText(char const *);
  virtual ~reportStreamText(void);

  //
  // Functions to print "high level objects"
  //
  virtual int print_title_page(titlePageData *);
  virtual int start_report(reportHeader *);
  virtual int end_report(reportFooter *);
  virtual int start_section(sectionHeader *);
  virtual int end_section(sectionFooter *);
  virtual int print_inventory(inventory *);
  virtual int print_inputs(inputs *, int);
  virtual int print_quality(qualityItem *);
  virtual int graph_quality(qualityItem *, queryFolder *);
  virtual int print_quality_table(qualityItem *, queryFolder *);
  virtual int print_metrics(metricFolder *);

  //
  // Functions to print "low level objects"
  //
  virtual int hard_space(void);
  virtual int end_line(void);
  virtual int end_paragraph(void);
  virtual int end_page(void);

  //
  // Functions to do some character modification
  //
  virtual charOpts *getCharOpts(void);
  virtual charOpts *setCharOpts(charOpts *);

protected:

  // constants to define page size
  static int const page_length;
  static int const page_width;
  static int const left_margin;
  static int const right_margin;
  static int const top_margin;
  static int const bottom_margin;

protected:

  // these write directly to the file
  virtual int write(char const *);
  virtual int write(char);
  
  // these decide what to write and how
  virtual int handle_text(textmode, char const *);
  virtual int handle_whitespace(textmode, char const *);

private:
  
  // keeps track of current position
  int row;
  int column;
  int top;
  int dideol;

  // character stuff
  charOpts *copts;

private:

  // helpers for handle_text/handle_whitespace
  int         handle_code_text(char const *);
  int         handle_code_whitespace(char const *);
  int         handle_text_text(char const *);
  int         handle_text_whitespace(char const *);
  
  // row and column manipulation
  int         pass_left_margin(void);
  int         pass_top_margin(void);
  int         lines_remaining(void);
  int         columns_remaining(void);
  int         at_top_of_page(void);
  int         last_was_end_line(void);
  char const *padField(char const *, int, char, int);
  int         reset(void);

  // file opening
  int         open_outfile(char const *);

  // misc.
  char const *center_line(char const *);

  // the file to write to
  char const *filename;
  raFile     *outfile;

};

#endif 
// _reportStreamText_h

