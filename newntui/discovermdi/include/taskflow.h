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
//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif
#if !defined(AFX_TASKFLOW_H__77D99632_3132_11D2_AF08_00A0C9B71DC4__INCLUDED_)
#define AFX_TASKFLOW_H__77D99632_3132_11D2_AF08_00A0C9B71DC4__INCLUDED_
#ifndef __SPREADSHEET_H
   #define __SPREADSHEET_H
   #include "spreadsheet.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TaskFlow.h : header file
//
#define OP_INSERT    0
#define OP_MOVE      1
#define OP_REPLACE   2

#include "FlowTab.h"

/////////////////////////////////////////////////////////////////////////////
// CTaskFlow form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


//------------------------------------------------------------------------------------------
// This class provides the drop target functionality. It will run the drop process in the
// case the dropping element is "DiscoverTaskFlowNode" type. In this case it willreceve
// the following integer array:
//     0 - operation code which must be performed with elements:
//         available opcodes are:
//         OP_INSERT    - element will be inserted
//         OP_MOVE      - element will be moved
//         OP_REPLACE   - element will be replaced
//     1 - amount of elements involved into operation
//     2,3 ... elements indexes,
//------------------------------------------------------------------------------------------ 
class CTaskFlow;
class CTaskDropTarget : public COleDropTarget {
public:
	CTaskFlow* m_Parent;
public:
	// This function will be called when the mouse enter the window while in drag-and
	// drop mode
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd,COleDataObject* pDataObject,DWORD dwKeyState,CPoint point );
	// This function will be called when the mouse move in the window while in drag-and
	// drop mode
	virtual DROPEFFECT OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
	// This function will be called when the user release mouse the window while in 
	// drag-and drop mode
	virtual BOOL OnDrop( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point );
};




class CTaskFlow : public CFormView
{
protected:
	CTaskFlow();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTaskFlow)

// Form Data
public:
	//{{AFX_DATA(CTaskFlow)
	enum { IDD = IDD_TPMFLOW };
	CFlowTab	m_TaskFlow;
	CComboBox	m_SequenceSelector;
	CStatic	m_SequenceCaption;
	CButton	m_NewBtn;
	CSpreadsheet	m_TaskSpreadsheet;
	CDataSource	m_DataSource;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void Update(void);
	void ContinueFlow(void);
	void StepFlow(void);
	void StopFlow(void);
	BOOL m_StopQuery;
	void RunFlow(void);
	void MoveItems(int size,   int* elements, CPoint& point);
	void InsertItems(int size, int* elements, CPoint& point);
	void RemoveNode(int pos);
	void InsertNewNode(int atPos, int id);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskFlow)
	public:
	virtual void OnFinalRelease();
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTaskFlow();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CTaskFlow)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeSequenceSelector();
	afx_msg void OnSelchangeNode(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeydownTaskFlow(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewTaskFlow();
	afx_msg void OnBoolChanged(long row, long col, LPCTSTR val);
	afx_msg void OnStringChanged(long row, long col, LPCTSTR val);
	afx_msg void OnSetChanged(long row, long col, LPCTSTR val);
	afx_msg void OnTextDoubleclicked(long row, long col, LPCTSTR text);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CTaskFlow)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
private:
	BOOL m_Running;
	BOOL m_Updated;
	void ServerMoveNode(int from, int to);
	void ServerRemoveNode(int pos);
	int  ServerAddNode(CString name, int pos);
	CTaskDropTarget m_DropTarget;
	COleDropSource m_DropSource;
	void FillTaskFlow(void);
	void FillSequencesSelector(void);
	void FillProperties(void);
	CImageList m_FlowImages;
	CImageList m_TasksImages;
	void LayoutTaskPage(int cx, int cy);
	CString DataSourceAccessSync(BSTR* command);
};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
#endif // !defined(AFX_TASKFLOW_H__77D99632_3132_11D2_AF08_00A0C9B71DC4__INCLUDED_)
