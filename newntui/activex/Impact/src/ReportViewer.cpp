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
// ReportViewer.cpp : implementation file
//

#include "stdafx.h"
#include "impact.h"
#include "ReportViewer.h"
#include "ImpactItem.h"

#include <io.h>
#include <fstream.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReportViewer

CReportViewer::CReportViewer()
{
	m_pImpactTree = NULL;
	m_szGeneratedReportPath.Empty();
}

CReportViewer::~CReportViewer()
{
	Clean();
}


BEGIN_MESSAGE_MAP(CReportViewer, CHtmlCtrl)
	//{{AFX_MSG_MAP(CReportViewer)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReportViewer message handlers

void CReportViewer::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CHtmlCtrl::OnShowWindow(bShow, nStatus);
	
	if(bShow && (m_pImpactTree!=NULL)) {
		m_szGeneratedReportPath = GenerateReportName();
		
		SaveTo(m_szGeneratedReportPath);

		Navigate2(CString("file://")+m_szGeneratedReportPath);
	} else {
		Clean();
   	}
}

void CReportViewer::SetImpactTree(CCheckedTree* pTree)
{
	m_pImpactTree = pTree;
}

BOOL CReportViewer::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= ES_MULTILINE | ES_READONLY;
	return CHtmlCtrl::PreCreateWindow(cs);
}

CString CReportViewer::GenerateReportName()
{
	// generating report name....
	struct tm *newtime;
	time_t aclock;
	time( &aclock );
   	newtime = localtime( &aclock );
	char homePath[100];
	char reportName[1024];
	strcpy(homePath,getenv("PSETHOME_LOCAL"));
	strcpy(reportName,"ImpactReport_");
	strcat(reportName,asctime(newtime));
	reportName[strlen(reportName)-1]=0;
	strcat(reportName,".htm");
	//replacing all spaces to '_'
	int nLen = strlen(reportName);
	for(int i=0;i<nLen;i++) {
		char ch = reportName[i];
		if(ch==' ' || ch==':' || ch=='\n')	ch = '_';
		if(ch=='/') ch = '\\';
		reportName[i]=ch;
	}
	
	char fullPath[2048];
	strcpy(fullPath,homePath);
	if(fullPath[strlen(fullPath)-1]!='\\' && fullPath[strlen(fullPath)-1]!='/')
		strcat(fullPath,"\\");
	strcat(fullPath,"Reports\\");
	strcat(fullPath,reportName);
	printf("Report file name is %s\n",fullPath);

	return CString(fullPath);
}

void CReportViewer::CreateReportStructure(CImpactReport &report)
{
	HTREEITEM hRoot = m_pImpactTree->GetRootItem();
	HTREEITEM hChild = m_pImpactTree->GetChildItem(hRoot);
	while(hChild!=NULL) {
		CImpactItem* pItem = (CImpactItem*)m_pImpactTree->GetItemData(hChild);
		if(pItem!=NULL) {
			CImpactActionsSet* pSet = pItem->GetActions();
			TActionsList& Actions = pSet->GetActions();
			CString szEntityDescr = "Changes for ";
			szEntityDescr += pItem->GetEntity()->GetKind();
			szEntityDescr += " " + pItem->GetEntity()->GetName();
			CReportEntry* pSymbolEntry = new CReportEntry(szEntityDescr,NULL);
			int nCount = 0;
			POSITION pos = Actions.GetHeadPosition();
			while(pos!=NULL) {
				CImpactAction* action = (CImpactAction*)Actions.GetNext(pos);
				CResultsTree* pResults = action->GetResults();
				if(pResults!=NULL) {
					pSymbolEntry->Add(new CReportEntry(action->GetDescription(),pResults));
					nCount++;
				}
			}
			if(nCount > 0 )
				report.Add(pSymbolEntry);
			else
				delete pSymbolEntry;
		}
		hChild = m_pImpactTree->GetNextSiblingItem(hChild);
	}
}

void CReportViewer::Clean()
{
	if(!m_szGeneratedReportPath.IsEmpty() && (_access(m_szGeneratedReportPath, 0) != -1)) {
		CFile::Remove(m_szGeneratedReportPath);
		m_szGeneratedReportPath.Empty();
	}
}

BOOL CReportViewer::SaveTo(CString &name)
{
	ofstream os(name);
	CImpactReport report(&os);

	CreateReportStructure(report);

	report.Generate();
	os.close();

	return true;
}

CString CReportViewer::GetGeneratedReportName()
{
	return m_szGeneratedReportPath;
}
