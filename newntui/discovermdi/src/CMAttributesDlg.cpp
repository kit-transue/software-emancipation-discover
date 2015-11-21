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
// CMAttributesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "CMAttributesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCMAttributesDlg dialog


CCMAttributesDlg::CCMAttributesDlg(CCMAttributesList* pAttrList,CWnd* pParent /*=NULL*/)
	: CDialog(CCMAttributesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCMAttributesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pWndList = NULL;
	m_pAttrsList = pAttrList;
}

CCMAttributesDlg::~CCMAttributesDlg() {
	if(m_pWndList!=NULL) {
		POSITION posHead = m_pWndList->GetHeadPosition();
		while(posHead!=NULL) {
			CWnd* pWnd = m_pWndList->GetNext(posHead); 
			delete pWnd;
		}
		delete m_pWndList;
	}
}

void CCMAttributesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCMAttributesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCMAttributesDlg, CDialog)
	//{{AFX_MSG_MAP(CCMAttributesDlg)
	ON_BN_CLICKED(IDC_OKALL, OnOkAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCMAttributesDlg message handlers

void CCMAttributesDlg::OnOkAll() 
{
	if (!UpdateData(TRUE))
	{
		TRACE0("UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}
	EndDialog(IDC_OKALL);
}

BOOL CCMAttributesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetWindowText("CM command parameters");
	
	int nYPos = 12;
	const int nLineShift = 5;
	const int nColumnShift = 5;
	const int nXGap = 10;
	CRect rcDialog;
	GetWindowRect(&rcDialog);
	if(m_pAttrsList!=NULL) {
		POSITION pos = m_pAttrsList->GetHeadPosition();
		while(pos!=NULL) {
			CCMAttribute* pAttr = m_pAttrsList->GetNext(pos);
			int nHeight;
			if(pAttr!=NULL) {
				if(m_pWndList == NULL) m_pWndList=new CWindowsList();
				int nType = pAttr->GetType();
				CWnd* pWnd = NULL;
				CSize szWnd;
				int nXPos = 5;
				if(!pAttr->IsBoolean()) {
					pWnd = new CStatic();
					if(((CStatic*)pWnd)->Create(pAttr->GetName(),WS_CHILD|WS_VISIBLE|SS_LEFT,CRect(0,0,0,0),this)) {
						CDC* pDC=pWnd->GetDC();
						szWnd =  pDC->GetTextExtent(pAttr->GetName());
						pWnd->ReleaseDC(pDC);
						pWnd->MoveWindow(nXPos,nYPos,szWnd.cx,szWnd.cy);
						m_pWndList->AddTail(pWnd);
					}
					nHeight = szWnd.cy;
					nXPos = nXPos + szWnd.cx + nColumnShift;
					int nWidth= rcDialog.Width()-nXPos-nXGap;
					pWnd = NULL;
					if(nType==CCMAttribute::TEXT) {
						nHeight = (nHeight*12)/10;
						pWnd = new CEdit();
						((CEdit*)pWnd)->Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,CRect(0,0,0,0),this,(UINT)pos);
						pWnd->ModifyStyleEx(0, WS_EX_CLIENTEDGE);   
						pWnd->MoveWindow(nXPos, nYPos, nWidth-3, nHeight);
						((CEdit*)pWnd)->SetWindowText(pAttr->GetValue());
					} else {
						if(nType==CCMAttribute::MULTILINE_TEXT) {
							pWnd = new CEdit();
							nHeight = (nHeight*18)/5;
							((CEdit*)pWnd)->Create(ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,CRect(0,0,0,0),this,(UINT)pos);
							pWnd->MoveWindow(nXPos, nYPos, nWidth, nHeight);
							pWnd->ModifyStyleEx(0, WS_EX_CLIENTEDGE);   
							((CEdit*)pWnd)->SetWindowText(pAttr->GetValue());
						} else {
							if(nType==CCMAttribute::LIST) {
								pWnd = new CComboBox();
								((CComboBox*)pWnd)->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,CRect(0,0,0,0),this,(UINT)pos);
								pWnd->MoveWindow(nXPos, nYPos, nWidth, nHeight*5);
								int nCount = pAttr->GetListElementsCount();
								for(int i=0;i<nCount;i++) 
									((CComboBox*)pWnd)->AddString(pAttr->GetListElementValue(i));
								CString szValue = pAttr->GetListElementValue(pAttr->GetValue());
								((CComboBox*)pWnd)->SelectString(0,szValue);
							}
						}
					}
				} else {
					pWnd = new CButton();
					if(((CButton*)pWnd)->Create(pAttr->GetName(), WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,CRect(0,0,0,0), this,(UINT)pos)) {
						CDC* pDC=pWnd->GetDC();
						szWnd =  pDC->GetTextExtent(pAttr->GetName());
						pWnd->ReleaseDC(pDC);
						pWnd->MoveWindow(nXPos,nYPos,rcDialog.Width()-nXPos-nXGap,szWnd.cy);
						nHeight = szWnd.cy;
						CString szValue = pAttr->GetListElementValue(pAttr->GetValue());
						((CButton*)pWnd)->SetCheck(szValue.CompareNoCase("true")==0);
					}
				}
				pWnd->SetFont(GetFont(),false);
				m_pWndList->AddTail(pWnd);
			}
			nYPos += nHeight;
			if(pos!=NULL)
				nYPos += nLineShift;
		}
	}
	CWnd* pWnd = GetDlgItem(IDOK);
	CRect rcButton;
	pWnd->GetWindowRect(&rcButton);
	ScreenToClient(&rcButton);
	rcButton.top += nYPos;
	rcButton.bottom += nYPos;
	pWnd->MoveWindow(rcButton);
	
	pWnd = GetDlgItem(IDC_BORDER);
	pWnd->GetWindowRect(&rcButton);
	ScreenToClient(&rcButton);
	rcButton.bottom += nYPos;
	pWnd->MoveWindow(rcButton);

	pWnd = GetDlgItem(IDCANCEL);
	pWnd->GetWindowRect(&rcButton);
	ScreenToClient(&rcButton);
	rcButton.top += nYPos;
	rcButton.bottom += nYPos;
	pWnd->MoveWindow(rcButton);

	pWnd = GetDlgItem(IDC_OKALL);
	pWnd->GetWindowRect(&rcButton);
	ScreenToClient(&rcButton);
	rcButton.top += nYPos;
	rcButton.bottom += nYPos;
	pWnd->MoveWindow(rcButton);

	MoveWindow(rcDialog.left,rcDialog.top,rcDialog.Width(),rcDialog.Height()+nYPos);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CCMAttributesDlg::UpdateData(BOOL bSaveAndValidate) {
	BOOL bRet = CDialog::UpdateData(bSaveAndValidate);

	if(m_pAttrsList!=NULL) {
		POSITION pos = m_pAttrsList->GetHeadPosition();
		while(pos!=NULL) {
			CCMAttribute* pAttr = m_pAttrsList->GetNext(pos);
			CWnd* pWnd = GetDlgItem((UINT)pos);
			if(pWnd!=NULL) {
				int nType = pAttr->GetType();
				if(nType == CCMAttribute::TEXT || nType == CCMAttribute::MULTILINE_TEXT) {
					CString szValue;
					((CEdit*)pWnd)->GetWindowText(szValue);
					pAttr->SetValue(szValue);
				} else {
					if(pAttr->IsBoolean()) {
						int nCheck = ((CButton*)pWnd)->GetCheck();
						CString szValue = (nCheck?"true":"false");
						CString szKey = pAttr->GetListElementKey(szValue);
						pAttr->SetValue(szKey);
					} else {
						int nIdx = ((CComboBox*)pWnd)->GetCurSel();
						CString szValue;
						((CComboBox*)pWnd)->GetLBText(nIdx,szValue);
						pAttr->SetValue(szValue);
					}
				}
			}
		}
	}
	return bRet;
}

void CCMAttributesDlg::OnOK() 
{
	if (!UpdateData(TRUE))
	{
		TRACE0("UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}
	EndDialog(IDOK);
}
