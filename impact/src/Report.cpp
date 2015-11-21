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
#include "Report.h"

TReportEntities::~TReportEntities() {
	TReportIterator iter;
	for(iter = begin(); iter != end(); iter++) delete *iter;
	clear();
}

CImpactReport::CImpactReport() {
	m_pStream = &cout;
}

CImpactReport::CImpactReport(ostream* pOS) {
	m_pStream = pOS;
}

CImpactReport::~CImpactReport() {
	m_pStream->flush();
}

void CImpactReport::Header() {
	struct tm *newtime;
	time_t aclock;
	time( &aclock );
   	newtime = localtime( &aclock );

	*m_pStream << "\t\t\t\tDIScover"<<endl<<"\t\t\t      Impact Analysis Report"<<endl<<endl<<"\t\t\tCreated on ";
   	*m_pStream << asctime(newtime);
	*m_pStream << "-----------------------------------------------------------------------------"<<endl<<endl; 
	*m_pStream << "Impact analysis calculates the overall impact of the user-specified change to the selected symbols.";
	*m_pStream << endl << endl << endl;
}

void CImpactReport::recursivePrint(CResultsTree* pRoot,string prefix) {
	if(pRoot != NULL) {
		do{
			CResultEntity* pEntity = pRoot->getEntity();
			*m_pStream << prefix << pEntity->getMessage()->c_str()<<endl;
			recursivePrint(pRoot->children(),prefix + "    ");
		} while((pRoot=pRoot->getNext())!=NULL); 
	}
}

void CImpactReport::Descriptions() {
	*m_pStream << "Impact description"<<endl<<endl;
	TReportIterator iter;
	string prefix = "    ";
	for(iter = m_Entries.begin(); iter != m_Entries.end(); iter++) {
		CReportEntry* pEntry = *iter;
		*m_pStream << " " <<pEntry->getMessage() << endl;
		TReportEntities* pChildren = pEntry->GetChildren();
		if(pChildren!=NULL) {
			TReportIterator ChildrenIter;		
			int nIdx = 1;
			for(ChildrenIter = pChildren->begin(); ChildrenIter != pChildren->end(); ChildrenIter++,nIdx++) {
				CReportEntry* pChildEntry = *ChildrenIter;
				*m_pStream << prefix << nIdx << "." << pChildEntry->getMessage()<<endl;
				recursivePrint(pChildEntry->getResults(),prefix);
			}
		}
	}
}

void CImpactReport::Generate() {
	Header();
	Descriptions();
}
