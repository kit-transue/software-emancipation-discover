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
// SimplifyText.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "SimplifyText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimplifyText

IMPLEMENT_DYNCREATE(CSimplifyText, CFormView)

CSimplifyText::CSimplifyText()
	: CFormView(CSimplifyText::IDD)
{
	//{{AFX_DATA_INIT(CSimplifyText)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CSimplifyText::~CSimplifyText()
{
}

void CSimplifyText::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSimplifyText)
	DDX_Control(pDX, IDC_RICHEDIT1, m_Editor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSimplifyText, CFormView)
	//{{AFX_MSG_MAP(CSimplifyText)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimplifyText diagnostics

#ifdef _DEBUG
void CSimplifyText::AssertValid() const
{
	CFormView::AssertValid();
}

void CSimplifyText::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimplifyText message handlers


DWORD CALLBACK EditStreamReadCallback(DWORD dwCookie,LPBYTE pbBuff,LONG cb,LONG *pcb) {

   CFile* pFile = (CFile*) dwCookie;

   *pcb = pFile->Read(pbBuff, cb);

   return 0;
}

void CSimplifyText::LoadFile(const CString &name) {
CFile file;

      if(file.Open(name, CFile::modeRead) ) {
          CHARFORMAT cf;
	      cf.cbSize=sizeof(CHARFORMAT);
		  cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;
		  cf.dwEffects=0;
		  cf.yHeight=200;
		  cf.crTextColor=RGB(0,0,0);
		  strcpy(cf.szFaceName,"Arial");

	      m_Editor.SetDefaultCharFormat(cf);
		  m_Editor.SetSel(0,-1);
	      m_Editor.SetSelectionCharFormat(cf);

		  EDITSTREAM es;
		  es.dwCookie=(DWORD) &file;
		  es.pfnCallback=&EditStreamReadCallback;
		  m_Editor.StreamIn(SF_TEXT,es);

		  m_Editor.SetSel(-1,-1);
	  }
	  m_Editor.SetReadOnly();

}

void CSimplifyText::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
    if(::IsWindow(m_Editor)) m_Editor.MoveWindow(0,0,cx,cy);	
	
}

void CSimplifyText::MarkLine(int line, int type) {
	if(line<0) return;
    m_Editor.SetSel(m_Editor.LineIndex(line),m_Editor.LineIndex(line+1));
    CHARFORMAT cf;
	cf.cbSize=sizeof(CHARFORMAT);
	m_Editor.GetSelectionCharFormat(cf);
    switch(type) {
	     case 0 : cf.crTextColor=RGB(0,0,0);
	              cf.dwEffects = CFE_BOLD;
		          break;
	     case 1 : cf.crTextColor=RGB(0,190,0);
	              cf.dwEffects = CFE_BOLD;
			      break;
	     case 2 : cf.crTextColor=RGB(255,0,0);
	              cf.dwEffects = CFE_STRIKEOUT;
			      break;
	}
	m_Editor.SetSelectionCharFormat(cf);
	m_Editor.SetSel(-1,-1);
}


void CSimplifyText::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
    SIZE sizeTotal;
	sizeTotal.cx=0;
	sizeTotal.cy=0;
	SetScrollSizes( MM_TEXT, sizeTotal);	
}
