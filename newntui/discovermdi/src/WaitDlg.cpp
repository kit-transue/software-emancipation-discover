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
// WaitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "WaitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaitDlg dialog

const int CWaitDlg::GRANULARITY = 100;

CWaitDlg::CWaitDlg(HANDLE hProcess,CWnd* pParent /*=NULL*/)
	: CDialog(CWaitDlg::IDD, pParent)
{
	m_pCompletionProvider = NULL;
	m_hProcess = hProcess;
	m_szMessage = "";
	//{{AFX_DATA_INIT(CWaitDlg)
	//}}AFX_DATA_INIT
}


void CWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaitDlg)
	DDX_Control(pDX, IDC_MESSAGE, m_MessageCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWaitDlg, CDialog)
	//{{AFX_MSG_MAP(CWaitDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaitDlg message handlers


int CWaitDlg::Wait(int nTimeout /*=0*/)
{
/*	Create(IDD);
	CWnd* pParent=GetParent();
	if(pParent==NULL) {
		pParent=AfxGetMainWnd();
		SetParent(pParent);
	}
	CenterWindow(pParent);
	UINT nTimer = SetTimer(1,100,0);
	// enter modal loop
	DWORD dwFlags = MLF_SHOWONIDLE | WF_CONTINUEMODAL | WF_MODALLOOP;
	if (GetStyle() & DS_NOIDLEMSG)
		dwFlags |= MLF_NOIDLEMSG;
	VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
*/
	m_nTimeout = nTimeout;
	m_bTimeoutSet = m_nTimeout>0;
	return DoModal();
//	KillTimer(nTimer);
//	return m_nModalResult;
}

void CWaitDlg::OnTimer(UINT nIDEvent) 
{
	// Wait until child process exits.
    if(WaitForSingleObject( m_hProcess, 0 )==WAIT_OBJECT_0)
		EndDialog(IDOK);
	else
		if(m_pCompletionProvider!=NULL && m_pCompletionProvider->IsWaitComplete())
			EndDialog(IDOK);
		else 
			if(m_bTimeoutSet) {
				m_nTimeout -= GRANULARITY;
				if(m_nTimeout<=0) EndDialog(IDOK);
			}
	
	CDialog::OnTimer(nIDEvent);
}

void CWaitDlg::OnOK()  {
	// do nothing - just wait for process to finish
}

void CWaitDlg::OnCancel()  {
	// do nothing - just wait for process to finish
}

CString CWaitDlg::GetTextWithEllipsis(CDC* pDC, CString& szString, CRect& Rect)
{
    SIZE    Size;
    int     nFit;
    int     nWidth = Rect.right-Rect.left;
    CString  EllipsisString;
    BOOL    fSuccess = FALSE;

    // Get how many chars will fit.
    fSuccess = GetTextExtentExPoint( pDC->GetSafeHdc(), 
        szString, 
        szString.GetLength(), 
        nWidth, 
        &nFit, 
        NULL, 
        &Size);
        
    // Get the dimensions of the full string.
    if(fSuccess)
        fSuccess = GetTextExtentExPoint( pDC->GetSafeHdc(), 
            szString, 
            szString.GetLength(), 
            nWidth, 
            NULL, 
            NULL, 
            &Size);
        
	if(fSuccess && nFit<szString.GetLength()) {
		// Copy to our working buffer.
		int nStart = nFit/2;
		int nRest = nFit-nStart;

        // If we need Ellipsis'.
        while(Size.cx > nWidth && fSuccess) {
			// Add them to what will fit and try again.
			EllipsisString = szString.Left(nStart);
			EllipsisString += "...";
			EllipsisString += szString.Right(nRest);
            fSuccess = GetTextExtentExPoint( pDC->GetSafeHdc(), 
					EllipsisString, 
                    EllipsisString.GetLength(), 
                    nWidth, 
                    NULL, 
                    NULL, 
                    &Size);
            nStart--; // Decrement in case we need another pass.
			nRest--;

            // Rectangle too small.
            if (nStart<=0 && nRest<=0)
				fSuccess = FALSE;
		}
		// if no success - return an empty string
		if(!fSuccess) EllipsisString = "";
        // Working buffer now contains the string with Ellipsis'.
	} else {  // Else no rectangle so return the same string we received as an input.
		EllipsisString = szString;
	}

    return EllipsisString;
}

int CWaitDlg::DoModal() 
{
	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_lpDialogTemplate != NULL);

	// load resource as necessary
	LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
	HGLOBAL hDialogTemplate = m_hDialogTemplate;
	HINSTANCE hInst = AfxGetResourceHandle();
	if (m_lpszTemplateName != NULL)
	{
		hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		hDialogTemplate = LoadResource(hInst, hResource);
	}
	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

	// return -1 in case of failure to load the dialog template resource
	if (lpDialogTemplate == NULL)
		return -1;

	// disable parent (before creating dialog)
	HWND hWndParent = PreModal();
	AfxUnhookWindowCreate();
	BOOL bEnableParent = FALSE;
	if (hWndParent != NULL && ::IsWindowEnabled(hWndParent))
	{
		::EnableWindow(hWndParent, FALSE);
		bEnableParent = TRUE;
	}

	TRY
	{
		// create modeless dialog
		AfxHookWindowCreate(this);
		if (CreateDlgIndirect(lpDialogTemplate,
						CWnd::FromHandle(hWndParent), hInst))
		{
			CDC*	pDC = m_MessageCtrl.GetWindowDC();
			CFont*	pFont = pDC->SelectObject(m_MessageCtrl.GetFont());
			CRect	compRect;
			m_MessageCtrl.GetClientRect(compRect);
			m_MessageCtrl.SetWindowText(GetTextWithEllipsis(pDC,m_szMessage,compRect));
			pDC->SelectObject(pFont);

			UINT nTimer = SetTimer(1,GRANULARITY,0);
			
			if (m_nFlags & WF_CONTINUEMODAL)
			{
				// enter modal loop
				DWORD dwFlags = MLF_SHOWONIDLE;
				if (GetStyle() & DS_NOIDLEMSG)
					dwFlags |= MLF_NOIDLEMSG;
				VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
			}

			// hide the window before enabling the parent, etc.
			if (m_hWnd != NULL)
				SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
					SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
			
			KillTimer(nTimer);
		}
	}
	CATCH_ALL(e)
	{
		e->Delete();
		m_nModalResult = -1;
	}
	END_CATCH_ALL

	if (bEnableParent)
		::EnableWindow(hWndParent, TRUE);
	if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(hWndParent);

	// destroy modal window
	DestroyWindow();
	PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	return m_nModalResult;
}

void CWaitDlg::SetCompletionProvider(CCompletionProvider* pProvider)
{
	m_pCompletionProvider = pProvider;
}

void CWaitDlg::SetMessage(CString &szMsg)
{
	m_szMessage = szMsg;
}
