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
#ifndef _idfSymbol_h
#define _idfSymbol_h

#include <idf.h>
#include <locationStruct.h>
#include <idfDict.h>

class idfSymbol
{
public:
  idfSymbol(int num, const idfModel &);
  idfSymbol(int num, const char *name, const char *kind, const struct location *, boolean generated, boolean, const idfModel &);
  ~idfSymbol();

  idfRelationDictionary &getRelations();
  idfAttributeDictionary &getAttributes();
  const idfModel &getModel() const;
  idfSymbol  *getMatch() const;
  void        setMatch(idfSymbol *);
  boolean     getDiscrepancyReported() const;
  void        setDiscrepancyReported(boolean);
  boolean     set(const char *name, const char *kind, const struct location *, boolean generated, boolean);
  boolean     isGenerated() const;
  boolean     isRefBeforeDef() const;
  const char *getName() const;
  boolean     getLocalVariableName(const char *&name,
				   const char *&fn_name,
				   const unsigned int *&block_id) const;
  const char *getKind() const;
  const struct location *getLocation() const;
  int         getIdentNum() const;
private:
  void        setLocalName();

  idfSymbol  *match;
  boolean     discrepancyReportedFlag;
  idfRelationDictionary *relations;
  idfAttributeDictionary *attributes;
  const idfModel *model;
  locationStruct *location;
  const char *name;
  const char *kind;
  boolean     generated;
  boolean     refBeforeDef;
  int         ident_num;
  const char *localName;
  const char *definingFunction;
  boolean     hasBlockId;
  unsigned int blockId;
};

//============================================================================
//============================================================================
inline idfRelationDictionary &idfSymbol::getRelations()
{
  return *relations;
}

//============================================================================
//============================================================================
inline idfAttributeDictionary &idfSymbol::getAttributes()
{
  return *attributes;
}

//============================================================================
//============================================================================
inline const idfModel &idfSymbol::getModel() const
{
  return *model;
}

//============================================================================
//============================================================================
inline idfSymbol *idfSymbol::getMatch() const
{
  return match;
}

//============================================================================
//============================================================================
inline void idfSymbol::setMatch(idfSymbol *m)
{
  match = m;
}

//============================================================================
//============================================================================
inline boolean idfSymbol::getDiscrepancyReported() const
{
  return discrepancyReportedFlag;
}

//============================================================================
//============================================================================
inline void idfSymbol::setDiscrepancyReported(boolean b)
{
  discrepancyReportedFlag = b;
}

//============================================================================
//============================================================================
inline boolean idfSymbol::isGenerated() const
{
  return generated;
}

//============================================================================
//============================================================================
inline boolean idfSymbol::isRefBeforeDef() const
{
  return refBeforeDef;
}

//============================================================================
//============================================================================
inline const char *idfSymbol::getName() const
{
  return name;
}

//============================================================================
//============================================================================
inline const char *idfSymbol::getKind() const
{
  return kind;
}

//============================================================================
//============================================================================
inline const struct location *idfSymbol::getLocation() const
{
  return location;
}

//============================================================================
//============================================================================
inline int idfSymbol::getIdentNum() const
{
  return ident_num;
}

#endif
