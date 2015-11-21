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
#ifndef _idfAst_h
#define _idfAst_h

#include <idf.h>
#include <locationList.h>
#include <astnodeStruct.h>
#include <general.h>

class idfAstSiblings
{
public:
                idfAstSiblings(astnodeList *children, idfAst *parent,
                               const idfModel &);
                ~idfAstSiblings();
  
  int           getNumber() const;
  idfAst       *getSibling(int) const; // 0-indexed
  const idfModel &getModel() const;

  boolean       getSubtreeLocation(locationStruct &loc) const;

private:
  void          determineSubtreeLocation();

  idfAst       *first;

  locationStruct subtreeLocation;
  boolean       hasSubtreeLocation;
  const idfModel *model;
};

class idfAst
{
public:
                idfAst(astnodeStruct *, idfAst *parent,
		       const idfModel &);
                ~idfAst();
  
  const idfModel &getModel() const;

  locationList *getLocation() const { return location; }
  boolean       getSubtreeLocation(locationStruct &loc) const;

  idfAst       *getMatch() const { return match; }
  void          setMatch(idfAst *m) { match = m; }
  boolean       getDiscrepancyReported() const { return discrepancyReportedFlag; }
  void          setDiscrepancyReported(boolean r) { discrepancyReportedFlag = r; }
  boolean       getContainsDiscrepancy() const { return containsDiscrepancyFlag; }
  void          setContainsDiscrepancy(boolean r) { containsDiscrepancyFlag = r; }
  int           getNumChildren() const { return children.getNumber(); }
  idfAst       *getChild(int n) const { return children.getSibling(n); } // 0-indexed
  const idfAstSiblings &getChildren() const { return children; }
  const char   *getKind() const;
  unsigned int  getIdentifier() const;
  const char   *getValue() const;
  boolean       isGenerated() const;
  idfAst       *getParent() const;
  idfAst       *getNextSib() const { return nextSib; }
  idfAst       *getPrevSib() const { return prevSib; }

  locationStruct *getComparableLocation() const;

private:
  void          determineSubtreeLocation();

  char         *kind;
  unsigned int  identifier;
  char         *value;
  boolean       isGen;

  locationList *location;
  locationStruct subtreeLocation;
  boolean       hasSubtreeLocation;

  boolean       discrepancyReportedFlag;
  boolean       containsDiscrepancyFlag;

  idfAst       *parent;
  idfAstSiblings children;
  idfAst       *nextSib;
  idfAst       *prevSib;
  idfAst       *match;

  friend class  idfAstSiblings;
};

//============================================================================
inline const idfModel &idfAstSiblings::getModel() const
{
  return *model;
}

//============================================================================
inline const idfModel &idfAst::getModel() const
{
  return children.getModel();
}

#endif
