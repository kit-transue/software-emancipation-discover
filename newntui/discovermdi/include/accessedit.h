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
#if !defined(AFX_ACCESSEDIT_H__FC3C4CAF_5EF4_11D2_AF3B_00A0C9B71DC4__INCLUDED_)
#define AFX_ACCESSEDIT_H__FC3C4CAF_5EF4_11D2_AF3B_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AccessEdit.h : header file
//

#define ACCESS_CAN_PASTE   1
#define ACCESS_CAN_COPY    2
#define ACCESS_HAS_PREV    4
#define ACCESS_HAS_NEXT    8
#define ACCESS_CAN_STOP    16

/////////////////////////////////////////////////////////////////////////////
// CAccessEdit window

class CVisibleBuffer {
  public:
	  CVisibleBuffer(int maxLines=2000);
	  void AppendBlock(CString& data);
	  CString& GetVisibleBuffer();
  protected:
	  CString m_VisibleBuffer;
	  int     m_LinesInBuffer;
	  int     m_MaxLines;
};

class CAccessEdit : public CEdit
{
// Construction
public:
	CAccessEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAccessEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	void UseFont(LOGFONT* fontdescr);
	unsigned int GetEditorStatus(void);
	void GoNextCommand();
	void GoPrevCommand();
	void ToClipboard(void);
	void FromClipboard(void);
	void InsertResults(CString& results);
	CString& GetCommand(void);
	virtual ~CAccessEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAccessEdit)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	void GetPrevCommand(void);
	void GetNextCommand(void);
    void ReplaceCommand(CString& newCmd);
private:
	CFont* m_UserFont;
	BOOL m_MouseLock;
	BOOL m_ShiftPressed;
	BOOL m_CtrlPressed;
	BOOL m_InSelectionMode;
	BOOL m_Lock;
	CString m_ExecLineStr;
	CString m_Command;
	int     m_InHistory;
	int     m_HistoryCount;
	CString m_History[50];
    int m_CommandPos;
	CVisibleBuffer* m_Visible;
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACCESSEDIT_H__FC3C4CAF_5EF4_11D2_AF3B_00A0C9B71DC4__INCLUDED_)
