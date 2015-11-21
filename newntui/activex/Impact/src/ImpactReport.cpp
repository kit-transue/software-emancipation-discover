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
// ImpactReport.cpp: implementation of the CImpactReport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "impact.h"
#include "ImpactReport.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


TReportEntities::~TReportEntities() {
	POSITION pos = GetHeadPosition();
	while(pos!=NULL)
		delete GetNext(pos);
	RemoveAll();
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

    *m_pStream << "<H1>" << endl;
    *m_pStream << "<CENTER>" << endl;
    *m_pStream << " DIScover" << endl;
    *m_pStream << "<br>" << endl;
    *m_pStream << " Impact Analysis Report" << endl;
    *m_pStream << "</CENTER>" << endl;
    *m_pStream << "</H1>" << endl;
    *m_pStream << "<H3>" << endl;
    *m_pStream << "<CENTER>" << endl;
    *m_pStream << " Created on " << asctime(newtime) << endl;
    *m_pStream << "</CENTER>" << endl;
    *m_pStream << "</H3>" << endl;
    *m_pStream << "<HR>" << endl;
    *m_pStream << "<P>" << endl;
    *m_pStream << " Impact analysis calculates the overall impact of the user-specified change to the selected symbols." << endl;
    *m_pStream << "</P>" << endl;
}

void CImpactReport::recursivePrint(CResultsTree* pRoot,CString& openTag, CString& closeTag) {
	if(pRoot != NULL) {
		do{
			CResultEntity* pEntity = pRoot->getEntity();
			CString* szMessage = pEntity->getMessage();
			if(!szMessage->IsEmpty()) {
				*m_pStream << (char*)(LPCTSTR)openTag << endl;
				*m_pStream << "<I>" << (char*)(LPCTSTR)*szMessage << "</I>" << endl;
				*m_pStream << (char*)(LPCTSTR)closeTag << endl;
			}
	        *m_pStream << "<OL>" << endl;
			recursivePrint(pRoot->children(),CString(""),CString(""));
	        *m_pStream << "</OL>" << endl;
		} while((pRoot=pRoot->getNext())!=NULL); 
	}
}

void CImpactReport::Descriptions() {
	*m_pStream << "<P>" << endl;
    *m_pStream << "<H2>" << endl;
    *m_pStream << "<CENTER>" << endl;
    *m_pStream << "Impact description" << endl;
    *m_pStream << "</CENTER>" << endl;
    *m_pStream << "</H2>" << endl;
    *m_pStream << "</P>" << endl;	
	POSITION iter;
	for(iter = m_Entries.GetHeadPosition(); iter != NULL;) {
		CReportEntry* pEntry = m_Entries.GetNext(iter);
		CString& szEntityDesc = pEntry->getMessage();
	    *m_pStream << "<LI>" << endl;
		*m_pStream << "<B>" << (char*)(LPCTSTR)szEntityDesc << "</B>" << endl;
	    *m_pStream << "<OL>" << endl;
		TReportEntities* pChildren = pEntry->GetChildren();
		if(pChildren!=NULL) {
			POSITION childIter;
			for(childIter = pChildren->GetHeadPosition(); childIter != NULL;) {
				CReportEntry* pChild = pChildren->GetNext(childIter);
				CString& szMessage = pChild->getMessage();
				*m_pStream << "<LI>" << endl;
				*m_pStream << "<B><I>" << (char*)(LPCTSTR)szMessage << "</I></B>" << endl;
				*m_pStream << "</LI>" << endl;
				recursivePrint(pChild->getResults(),CString("<LI>"),CString("</LI>"));
			}
		}
	    *m_pStream << "</OL>" << endl;
	    *m_pStream << "</LI>" << endl;
	}
}

void CImpactReport::Generate() {
	*m_pStream << "<HTML>" << endl;
    *m_pStream << "<HEAD>" << endl;
    *m_pStream << "<TITLE>" << endl << "Impact analysis report" << endl << "</TITLE>" << endl;
    *m_pStream << "</HEAD>" << endl;     
	*m_pStream << "<BODY>" << endl;
	Header();
	Descriptions();
	Footer();
	*m_pStream << "</BODY>" << endl;
	*m_pStream << "</HTML>" <<endl;
}

void CImpactReport::Footer()
{
}
