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

#ifndef _reportStream_h
#define _reportStream_h

class inventory;
class inputs;
class qualityItem;
class queryFolder;
class metric;
class metricFolder;
class charOpts;

struct titlePageData;
struct reportHeader;
struct reportFooter;
struct sectionHeader;
struct sectionFooter;
struct qualityData;


class reportStream
{
public:

  // Constructor(s) / Destructor(s)
  reportStream(void);
  virtual ~reportStream(void);

public:

  enum textmode  { CODE, TEXT };

public:

  //
  // Functions to print "high level objects"
  //
  virtual int print_title_page(titlePageData *)                 = 0;
  virtual int start_report(reportHeader *)                      = 0;
  virtual int end_report(reportFooter *)                        = 0;
  virtual int start_section(sectionHeader *)                    = 0;
  virtual int end_section(sectionFooter *)                      = 0;
  virtual int print_inventory(inventory *)                      = 0;
  virtual int print_inputs(inputs *, int)                       = 0;
  virtual int print_quality(qualityItem *)                      = 0;
  virtual int graph_quality(qualityItem *, queryFolder *)       = 0;
  virtual int print_quality_table(qualityItem *, queryFolder *) = 0;
  virtual int print_metrics(metricFolder *)                     = 0;

  //
  // Functions to print "low level objects"
  //
          int add_string(char const *);
          int add_char(char);
  virtual int hard_space(void)                  = 0;
  virtual int end_line(void)                    = 0;
  virtual int end_paragraph(void)               = 0;
  virtual int end_page(void)                    = 0;

  //
  // Functions to do some character modification
  //
  virtual charOpts *getCharOpts(void)        = 0;
  virtual charOpts *setCharOpts(charOpts *)  = 0;

public:
  
  virtual textmode set_mode(textmode);
  inline  textmode get_mode(void);

protected:

  // functions to keep track of section nesting
  int start_of_section(void);
  int end_of_section(void);

  // retrieve section numbers
  int get_section_depth(void);
  int get_section_number(int);

  // these are callbacks so that text can be handled differently in different modes
  virtual int handle_text(textmode, char const *)       = 0;
  virtual int handle_whitespace(textmode, char const *) = 0;

private:
  
  static int const max_section_depth;

  int           *section_counters;
  int            section_depth;
  int            last_was_section_start;
  textmode       mode;

private:

  enum texttype { WHITESPACE, NONWHITESPACE };
  texttype identify_type(char); 
  
};

reportStream::textmode reportStream::get_mode(void)
{
  return mode;
}

#endif 
// _reportStream_h

