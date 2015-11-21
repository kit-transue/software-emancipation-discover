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
#ifndef _idfSmt_h
#define _idfSmt_h

#include <idf.h>
#include <locationList.h>
#include <general.h>

class idfSmt
{
public:
                idfSmt(const char *kw, locationList *, boolean,
		       const idfModel &);
                idfSmt(unsigned int id, boolean implicit,
		       locationList *, boolean,
		       const idfModel &);
                ~idfSmt();
  
  const idfModel &getModel() const { return *model; }
  locationList *getLocation() { return location; }
  const char   *getName();
  unsigned int  getId() { return id; }
  boolean       isKeyword() { return !id_flag; }
  boolean       isId() { return id_flag; }
  boolean       isImplicit() { return implicit; }

  void          setNext(idfSmt *n) { next = n; }
  void          setPrev(idfSmt *p) { prev = p; }
  idfSmt       *getNext() { return next; }
  idfSmt       *getPrev() { return prev; }

  void          setMatch(idfSmt *m) { match = m; }
  idfSmt       *getMatch() { return match; }

  boolean       getDiscrepancyReported() const { return discrepancyReportedFlag; }
  void          setDiscrepancyReported(boolean r) { discrepancyReportedFlag = r; }
  
private:
  const idfModel *model;
  unsigned int  id;
  boolean       implicit;
  boolean       id_flag;
  boolean       discrepancyReportedFlag;
  const char   *keywordName;
  locationList *location;

  idfSmt       *next;
  idfSmt       *prev;
  idfSmt       *match;
};
#endif
