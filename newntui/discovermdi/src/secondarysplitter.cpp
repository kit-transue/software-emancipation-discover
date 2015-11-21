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
// SecondarySplitter.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "SubmissionProperties.h"
#include "SubmitInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSecondarySplitter

IMPLEMENT_DYNCREATE(CSecondarySplitter, CSplitterWnd)

CSecondarySplitter::CSecondarySplitter()
{
}

CSecondarySplitter::~CSecondarySplitter()
{
}


BEGIN_MESSAGE_MAP(CSecondarySplitter, CSplitterWnd)
	//{{AFX_MSG_MAP(CSecondarySplitter)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSecondarySplitter message handlers


BOOL CSecondarySplitter::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
   //CRect winRect;
   //GetWindowRect(winRect);
   CSize size(100,100);

   if(CreateStatic(this,2,1)) {
      TRACE0("Failed to create split bar ");
      return FALSE; // CSplitterWnd
   } 
   return 1;
	
}

int CSecondarySplitter::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CSplitterWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
   CSize size(100,100);
	
	//CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
   if(CreateView(0,0,RUNTIME_CLASS(CSubmissionProperties),size,pContext)==FALSE) return 0;
   if(CreateView(1,0,RUNTIME_CLASS(CSubmitInfo),size, pContext)==FALSE)          return 0;

	return 0;
}
