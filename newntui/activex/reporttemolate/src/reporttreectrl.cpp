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
// ReportTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "reporttemolate.h"
#include "ReportTreeCtrl.h"
#include "ReportTemolateCtl.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReportTreeCtrl

CReportTreeCtrl::CReportTreeCtrl()
{
}

CReportTreeCtrl::~CReportTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CReportTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CReportTreeCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReportTreeCtrl message handlers

void CReportTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)  {
UINT flags;
HTREEITEM node;
CString attributesFileName;
CString treeFileName;
char buf[1024];

	CTreeCtrl::OnLButtonDown(nFlags, point);
    node=HitTest(point,&flags);
	if(flags==TVHT_ONITEMLABEL) {
        // Register or get clipboard format for the task flow node
	    UINT formatID=RegisterClipboardFormat("DiscoverReportNode");
        if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
            treeFileName=buf;
            treeFileName+="\\Reports\\Templates\\";
		} else {
            treeFileName="C:\\Discover\\Reports\\Templates\\";
		}
		attributesFileName=treeFileName;
		treeFileName+="__tmp__.xxx";
		attributesFileName+="__tmp__.atr";
		((CReportTemplateCtrl *)m_Parent)->MakeSubTreeFile(node,treeFileName);
        STGMEDIUM dataStruct;
		dataStruct.tymed=TYMED_FILE;

		HGLOBAL dataID= GlobalAlloc(GMEM_FIXED,(treeFileName.GetLength()+1)*sizeof(short));
	    dataStruct.lpszFileName = (BSTR)::GlobalLock(dataID);
		BSTR sysString;
		sysString = treeFileName.AllocSysString();
        memmove(dataStruct.lpszFileName,sysString,treeFileName.GetLength()*sizeof(short));
		SysFreeString(sysString);
	    GlobalUnlock(dataID);

		dataStruct.pUnkForRelease=NULL;
		CRect dragRect;
		GetItemRect(node,&dragRect,FALSE);
        m_DropSource.CacheData(formatID, &dataStruct);
		((CReportTemplateCtrl *)m_Parent)->MakeAttributesFile(node,attributesFileName);
		if(m_DropSource.DoDragDrop(DROPEFFECT_COPY|DROPEFFECT_MOVE,&dragRect)==DROPEFFECT_MOVE) {
		      ((CReportTemplateCtrl *)m_Parent)->DeleteItem(node);
		}
		m_DropSource.Empty();
		DeleteFile(treeFileName); 
		DeleteFile(attributesFileName);
	}

}

void CReportTreeCtrl::OnClose()  {
	m_DropTarget->Revoke();
	CTreeCtrl::OnClose();
}
