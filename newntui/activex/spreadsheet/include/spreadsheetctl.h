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
#if !defined(AFX_SPREADSHEETCTL_H__FC470425_2BF8_11D2_AF03_00A0C9B71DC4__INCLUDED_)
#define AFX_SPREADSHEETCTL_H__FC470425_2BF8_11D2_AF03_00A0C9B71DC4__INCLUDED_

#include "msflexgrid.h"	// Added by ClassView
#include "editwithquit.h"
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxtempl.h>

//-----------------------------------------------------------------------------------------
// This class contains all information about a cell in a spreadsheet. Any cell can be
// one of the following type:
//    0 - read-only text  cell 
//    1 - editable  text  cell
//    2 - boolean cell with checkbox in it
//    3 - set cell with the pop-up menu with all available cell items.
//  Empty cell have no CCellInfo object connected to it.
//-----------------------------------------------------------------------------------------
class CCellInfo {
   public: 
	       CCellInfo();
		   ~CCellInfo();

		   // Defines the cell type. Can be 0,1,2,or 3. (future extentions possible)
		   short SetType(short type);
		   // Returns the cell type.  Described in the header.
		   short GetType(void);
           // Sets the row described by this object
		   void SetRow(int row);
           // Returns the row accosiated with this object.
		   int  GetRow(void);
           // Sets the column described by this object
		   void SetColumn(int column);
           // Returns the column accosiated with this object.
		   int GetColumn(void);
		   // Sets the image resource ID for the checked status for the boolean cells.
		   // Calling this function will make the IImageDisp image interface available
		   // (can be obtained by calling GetCheckedImageInterface).
		   void SetCheckedImage(int resID);
		   // Sets the image resource ID for the unchecked status for the boolean cells.
		   // Calling this function will make the IImageDisp image interface available
		   // (can be obtained by calling GetUncheckedImageInterface).
		   void SetUncheckedImage(int resID);
           // Returns the pointer to the checked status image or NULL if no image
		   // in this object.
		   CBitmap* GetCheckedImage(void);
           // Returns the pointer to the unchecked status image or NULL if no image
		   // in this object.
		   CBitmap* GetUncheckedImage(void);
		   // Returns the dispatch interface to the checked status image
		   LPDISPATCH GetCheckedImageInterface(void);
		   // Returns the dispatch interface to the unchecked status image
		   LPDISPATCH GetUncheckedImageInterface(void);
		   // Sets the new status of the boolean cell
           void SetCheckState(BOOL state);
		   // Returns the status of the boolean cell.
           BOOL GetCheckState(void);
		   // Sets the popup menu which will be used by the cell. Old menu will be
		   // destroyed
           void SetMenu(void);
		   // Returns the pointer to the popup menu used by this cell or NULL if no
		   // menu assigned to the cell
		   CMenu* GetMenu();
		   // Returns the text string connected to the cell.
		   CString& GetText(void);
		   // Sets the text string wich will be displayed in a cell
		   void SetText(CString& text);
		   // Returns a menu ID you can use for the new mwnu item
		   int NewMenuCommand(void);
		   // Zeroz the menu ID counter,
		   void ZeroMenuCommand(void);
   protected:
	       static int  m_MenuCommand;
	       short       m_Type;
		   int         m_Row;
		   int         m_Column;
		   CBitmap*    m_Checked;
		   LPDISPATCH  m_IChecked;
		   CBitmap*    m_Unchecked;
		   LPDISPATCH  m_IUnchecked;
		   BOOL        m_CheckState;
		   CMenu*      m_Popup;
		   CString     m_Text;
};
//-----------------------------------------------------------------------------------------


// SpreadsheetCtl.h : Declaration of the CSpreadsheetCtrl ActiveX Control class.


/////////////////////////////////////////////////////////////////////////////
// CSpreadsheetCtrl : See SpreadsheetCtl.cpp for implementation.

class CSpreadsheetCtrl : public COleControl
{
	DECLARE_DYNCREATE(CSpreadsheetCtrl)

// Constructor
public:
	CSpreadsheetCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpreadsheetCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CSpreadsheetCtrl();

	DECLARE_OLECREATE_EX(CSpreadsheetCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CSpreadsheetCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CSpreadsheetCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CSpreadsheetCtrl)		// Type name and misc status

	afx_msg void OnKeyDownEvent(short FAR* KeyCode, short Shift);
	afx_msg void OnEnterCell();
    afx_msg void OnLeaveCell();
	afx_msg void OnMouseDown(short Button, short Shift, long x, long y);
    afx_msg void OnScroll();
    afx_msg void OnDblClick();


// Message maps
	//{{AFX_MSG(CSpreadsheetCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CSpreadsheetCtrl)
	afx_msg long GetRows();
	afx_msg void SetRows(long nNewValue);
	afx_msg long GetCols();
	afx_msg void SetCols(long nNewValue);
	afx_msg long GetFixedRows();
	afx_msg void SetFixedRows(long nNewValue);
	afx_msg long GetFixedCols();
	afx_msg void SetFixedCols(long nNewValue);
	afx_msg short GetUserSelection();
	afx_msg void SetUserSelection(short nNewValue);
	afx_msg long GetCurrentRow();
	afx_msg void SetCurrentRow(long nNewValue);
	afx_msg long GetCurrentColumn();
	afx_msg void SetCurrentColumn(long nNewValue);
	afx_msg BOOL GetEnableDraw();
	afx_msg void SetEnableDraw(BOOL bNewValue);
	afx_msg void AddMenuItem(long row, long col, LPCTSTR text);
	afx_msg void AddRow(long after);
	afx_msg void RemoveRow(long row);
	afx_msg void Clear();
	afx_msg short GetCellType(long row, long col);
	afx_msg void SetCellType(long row, long col, short nNewValue);
	afx_msg BSTR GetCellText(long row, long col);
	afx_msg void SetCellText(long row, long col, LPCTSTR lpszNewValue);
	afx_msg BOOL GetCellCheck(long row, long col);
	afx_msg void SetCellCheck(long row, long col, BOOL bNewValue);
	afx_msg long GetColWidth(long col);
	afx_msg void SetColWidth(long col, long nNewValue);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CSpreadsheetCtrl)
	void FireBoolChanged(long row, long col, LPCTSTR val)
		{FireEvent(eventidBoolChanged,EVENT_PARAM(VTS_I4  VTS_I4  VTS_BSTR), row, col, val);}
	void FireStringChanged(long row, long col, LPCTSTR val)
		{FireEvent(eventidStringChanged,EVENT_PARAM(VTS_I4  VTS_I4  VTS_BSTR), row, col, val);}
	void FireSetChanged(long row, long col, LPCTSTR val)
		{FireEvent(eventidSetChanged,EVENT_PARAM(VTS_I4  VTS_I4  VTS_BSTR), row, col, val);}
	void FireTextDoubleclicked(long row, long col, LPCTSTR text)
		{FireEvent(eventidTextDoubleclicked,EVENT_PARAM(VTS_I4  VTS_I4  VTS_BSTR), row, col, text);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	CEditWithQuit m_FloatingEditor;
	CMSFlexGrid m_GridWindow;
	enum {
	//{{AFX_DISP_ID(CSpreadsheetCtrl)
	dispidRows = 1L,
	dispidCols = 2L,
	dispidFixedRows = 3L,
	dispidFixedCols = 4L,
	dispidUserSelection = 5L,
	dispidCurrentRow = 6L,
	dispidCurrentColumn = 7L,
	dispidEnableDraw = 8L,
	dispidCellType = 13L,
	dispidCellText = 14L,
	dispidAddMenuItem = 9L,
	dispidCellCheck = 15L,
	dispidAddRow = 10L,
	dispidRemoveRow = 11L,
	dispidColWidth = 16L,
	dispidClear = 12L,
	eventidBoolChanged = 1L,
	eventidStringChanged = 2L,
	eventidSetChanged = 3L,
	eventidTextDoubleclicked = 4L,
	//}}AFX_DISP_ID
	};
public:
	void MoveCell(short keyCode);
	void CloseCellEditor(BOOL update=TRUE);
	void OpenCellEditor(BOOL update=TRUE);
public:
	long                         m_CurrentColumn;
	long                         m_CurrentRow;
	BOOL                         m_IgnoreCellChange;
	BOOL                         m_InFocusSet;
private:
    CCellInfo* FindCell(int row, int col);
private:
	CFont m_InfoFont;
	short                        m_SelectionMode;
	BOOL                         m_InEditMode;
	CList <CCellInfo,CCellInfo&> m_CellsList;
    CBitmap                      m_Marker;
	LPDISPATCH                   m_IMarker;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPREADSHEETCTL_H__FC470425_2BF8_11D2_AF03_00A0C9B71DC4__INCLUDED)
