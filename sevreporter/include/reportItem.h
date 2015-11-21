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

#ifndef _reportItem_h
#define _reportItem_h

class descParser;
class reportStream;
class reportTemplate;

class reportItem
{
public:
  reportItem(char const *, char const *);
  virtual ~reportItem(void);
  
public:
  // must override to define how to serialize a node
  virtual int        serialize(reportStream *, reportTemplate *) = 0;
  inline char const *name(void);

public:
  virtual int         onString(char const *);
  virtual int         onTag(int);

protected:
  inline char const *desc(void);
  
protected:
  virtual descParser *getParser(void);
  virtual int         parseDesc(char const *, reportStream *);

protected:
  reportStream *current_stream;

private:
  char const *__desc_index;
  char const *__name;
};

////////////////////////////////////////////////////////////////////////////////

//
// Inline method declarations
//

char const *reportItem::desc(void)
{
  return __desc_index;
}

char const *reportItem::name(void)
{
  return __name;
}

#endif 
// _reportItem_h

