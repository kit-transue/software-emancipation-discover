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
#ifndef __IMPACTREPORT_HEADER_FILE
#define __IMPACTREPORT_HEADER_FILE

#if defined(hp10) || defined(irix6)
   #include <iostream.h>
#else
   #include <ostream>
   #include <iostream>
#endif

#include "stl.h"
#include "ResultsTree.h"

class CReportEntry;
class TReportEntities : public list<CReportEntry*> {
 public:
  ~TReportEntities();
};

typedef TReportEntities::iterator TReportIterator;

class CReportEntry {
 public:
  CReportEntry(string message,CResultsTree* pResults) { m_pChildren = NULL;m_message = message;m_pResults=pResults;};
  ~CReportEntry() {
    if(m_pChildren != NULL) delete m_pChildren;
  };
  string& getMessage() { return m_message; };
  CResultsTree* getResults() { return m_pResults; };
  void Add(CReportEntry* pChild) { 
    if(m_pChildren == NULL) m_pChildren = new TReportEntities();
    m_pChildren->push_back(pChild);
  };
  TReportEntities* GetChildren(){return m_pChildren;};
 private:
  string m_message;
  CResultsTree* m_pResults;
  TReportEntities* m_pChildren;
};

class CImpactReport {
 public:
   CImpactReport();
   CImpactReport(ostream* pOS);
   ~CImpactReport();
   void Header();
   void Descriptions();
   void recursivePrint(CResultsTree* pRoot,string prefix);
   void Generate();
   void Add(CReportEntry* entity) { m_Entries.push_back(entity); };
 private:
   ostream* m_pStream;
   TReportEntities m_Entries;
};

#endif //__IMPACTREPORT_HEADER_FILE
