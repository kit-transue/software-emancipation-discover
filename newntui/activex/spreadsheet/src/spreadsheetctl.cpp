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
// SpreadsheetCtl.cpp : Implementation of the CSpreadsheetCtrl ActiveX Control class.

#include "stdafx.h"
#include "Spreadsheet.h"
#include "SpreadsheetCtl.h"
#include "SpreadsheetPpg.h"

#define START_MENU_ID 100


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_GRID 10200
#define IDC_FLOATEDITOR 10201

int CCellInfo::m_MenuCommand=START_MENU_ID;

typedef LRESULT (CALLBACK* WindowProcType) (HWND, UINT, WPARAM, LPARAM);

static LRESULT CALLBACK NewWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static WindowProcType OldWindowProc=0;
static BOOL in_add=FALSE;



//======================= CCellInfo class implementation ==================================


//-----------------------------------------------------------------------------------------
// Constructor: all pointers will be NULL - nothing assigned to the cell at the beginning.
// Initial cell type - read-only text string.
//-----------------------------------------------------------------------------------------
CCellInfo::CCellInfo() {
    m_Type=0;
	m_Checked     = NULL;
	m_Unchecked   = NULL;
	m_IChecked    = NULL;
	m_IUnchecked  = NULL;
	m_Popup       = NULL;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Destructor: will remove images as well as popup menu if assigned to the cell.
//-----------------------------------------------------------------------------------------
CCellInfo::~CCellInfo() {
   if(m_Checked !=NULL)   delete m_Checked;
   if(m_Unchecked!=NULL)  delete m_Unchecked;
   if(m_Popup!=NULL)      {
		m_Popup->DestroyMenu();
		delete m_Popup;
	}
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function defines the cell type. Available ftypes are:
// 0 - read-only text string
// 1 - editable text string
// 2 - boolean cell with a checkbox insisde
// 3 - set cell with popup menu attached.
// Old cell type will be returned.
//-----------------------------------------------------------------------------------------
short CCellInfo::SetType(short type) { 
short old = m_Type;

   m_Type=type;
   return old;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function returns the cell type. Available ftypes are:
// 0 - read-only text string
// 1 - editable text string
// 2 - boolean cell with a checkbox insisde
// 3 - set cell with popup menu attached.
//-----------------------------------------------------------------------------------------
short CCellInfo::GetType(void) { 
   return m_Type;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Sets the row this cell represent
//-----------------------------------------------------------------------------------------
void CCellInfo::SetRow(int row) {
    m_Row=row;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the row represented by this cell
//-----------------------------------------------------------------------------------------
int CCellInfo::GetRow(void) {
    return m_Row;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Sets the column this cell represent.
//-----------------------------------------------------------------------------------------
void CCellInfo::SetColumn(int column) {
	m_Column=column;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the column represented by this cell.
//-----------------------------------------------------------------------------------------
int CCellInfo::GetColumn(void) {
	return m_Column;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Creates the new check image from the bitmap resource ID. Makes IDispatch image interface 
// available.
//-----------------------------------------------------------------------------------------
void CCellInfo::SetCheckedImage(int resID) {
PICTDESC   description;
LPDISPATCH iPictDisp;

    if(m_Checked!=NULL) {
	    delete m_Checked;
		m_Checked=NULL;
	}
	if(resID==0) {
       m_IChecked = NULL;
	   return;
	}
	m_Checked = new CBitmap;
    m_Checked->LoadBitmap(resID);

	description.cbSizeofstruct=sizeof(description);
    description.picType=PICTYPE_BITMAP;
    description.bmp.hbitmap=*m_Checked;

    ::OleCreatePictureIndirect(&description, IID_IDispatch,TRUE,(void **)&iPictDisp);
	m_IChecked=iPictDisp;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Creates the new uneck image from the bitmap resource ID. Makes IDispatch image interface 
// available.
//-----------------------------------------------------------------------------------------
void CCellInfo::SetUncheckedImage(int resID) {
PICTDESC   description;
LPDISPATCH iPictDisp;

    if(m_Unchecked!=NULL) {
	    delete m_Unchecked;
		m_Unchecked=NULL;
	}
	if(resID==0) {
       m_IChecked = NULL;
	   return;
	}
	m_Unchecked = new CBitmap;
	m_Unchecked->LoadBitmap(resID);

	description.cbSizeofstruct=sizeof(description);
    description.picType=PICTYPE_BITMAP;
    description.bmp.hbitmap=*m_Unchecked;

    ::OleCreatePictureIndirect(&description, IID_IDispatch,TRUE,(void **)&iPictDisp);
	 m_IUnchecked=iPictDisp;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the pointer to the checked image bitmap or NULL if no bitmap defined.
//-----------------------------------------------------------------------------------------
CBitmap* CCellInfo::GetCheckedImage(void) {
	return m_Checked;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the pointer to the unchecked image bitmap or NULL if no bitmap defined.
//-----------------------------------------------------------------------------------------
CBitmap* CCellInfo::GetUncheckedImage(void) {
	return m_Unchecked;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Returns the checked image IDISPATCH interface or NULL if no image defined.
//-----------------------------------------------------------------------------------------
LPDISPATCH CCellInfo::GetCheckedImageInterface(void) {
   return m_IChecked;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the unchecked image IDISPATCH interface or NULL if no image defined.
//-----------------------------------------------------------------------------------------
LPDISPATCH CCellInfo::GetUncheckedImageInterface(void) {
	return m_IUnchecked;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Sets the cell state. Can be TRUE or FALSE.
//-----------------------------------------------------------------------------------------
void CCellInfo::SetCheckState(BOOL state) {
	m_CheckState=state;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the cell check state.
//-----------------------------------------------------------------------------------------
BOOL CCellInfo::GetCheckState(void) {
	return m_CheckState;
}
//-----------------------------------------------------------------------------------------

           
//-----------------------------------------------------------------------------------------
// Creates popup menu assosiated with this cell. Old one if any will be removed.
//-----------------------------------------------------------------------------------------
void CCellInfo::SetMenu(void) {
	if(m_Popup!=NULL) {
		m_Popup->DestroyMenu();
		delete m_Popup;
	}
	m_Popup = new CMenu;
	m_Popup->CreatePopupMenu();
}
//-----------------------------------------------------------------------------------------

		   
//-----------------------------------------------------------------------------------------
// Retuns the popup menu pointer or NULL if no mwnu defined.
//-----------------------------------------------------------------------------------------
CMenu* CCellInfo::GetMenu() {
	return m_Popup;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the text assosiated with this cell.
//-----------------------------------------------------------------------------------------
CString& CCellInfo::GetText(void) {
	return m_Text;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Sets the text assosiated with this cell.
//-----------------------------------------------------------------------------------------
void CCellInfo::SetText(CString& text) {
	m_Text=text;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the menu item ID which you can use for your new pop-up menu item.
//-----------------------------------------------------------------------------------------
int CCellInfo::NewMenuCommand(void) {
	return m_MenuCommand++;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Bring menu item ID counter to it's initial state.
//-----------------------------------------------------------------------------------------
void CCellInfo::ZeroMenuCommand(void) {
	m_MenuCommand=START_MENU_ID;
}
//-----------------------------------------------------------------------------------------



//=============== CCellInfo class implementation ends =====================================









IMPLEMENT_DYNCREATE(CSpreadsheetCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CSpreadsheetCtrl, COleControl)
	//{{AFX_MSG_MAP(CSpreadsheetCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CSpreadsheetCtrl, COleControl)
    //{{AFX_EVENTSINK_MAP(CAboutDlg)
    ON_EVENT(CSpreadsheetCtrl, IDC_GRID, -602 /* KeyDown */,      OnKeyDownEvent, VTS_PI2 VTS_I2)
	ON_EVENT(CSpreadsheetCtrl, IDC_GRID, -605 /* MouseDown */,    OnMouseDown,    VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CSpreadsheetCtrl, IDC_GRID,   71 /* EnterCell */,    OnEnterCell,    VTS_NONE)
	ON_EVENT(CSpreadsheetCtrl, IDC_GRID,   72 /* LeaveCell */,    OnLeaveCell,    VTS_NONE)
    ON_EVENT(CSpreadsheetCtrl, IDC_GRID,   73 /* Scroll */,       OnScroll,       VTS_NONE)
	ON_EVENT(CSpreadsheetCtrl, IDC_GRID, -601 /* DblClick */,     OnDblClick,     VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CSpreadsheetCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CSpreadsheetCtrl)
	DISP_PROPERTY_EX(CSpreadsheetCtrl, "Rows", GetRows, SetRows, VT_I4)
	DISP_PROPERTY_EX(CSpreadsheetCtrl, "Cols", GetCols, SetCols, VT_I4)
	DISP_PROPERTY_EX(CSpreadsheetCtrl, "FixedRows", GetFixedRows, SetFixedRows, VT_I4)
	DISP_PROPERTY_EX(CSpreadsheetCtrl, "FixedCols", GetFixedCols, SetFixedCols, VT_I4)
	DISP_PROPERTY_EX(CSpreadsheetCtrl, "UserSelection", GetUserSelection, SetUserSelection, VT_I2)
	DISP_PROPERTY_EX(CSpreadsheetCtrl, "CurrentRow", GetCurrentRow, SetCurrentRow, VT_I4)
	DISP_PROPERTY_EX(CSpreadsheetCtrl, "CurrentColumn", GetCurrentColumn, SetCurrentColumn, VT_I4)
	DISP_PROPERTY_EX(CSpreadsheetCtrl, "EnableDraw", GetEnableDraw, SetEnableDraw, VT_BOOL)
	DISP_FUNCTION(CSpreadsheetCtrl, "AddMenuItem", AddMenuItem, VT_EMPTY, VTS_I4 VTS_I4 VTS_BSTR)
	DISP_FUNCTION(CSpreadsheetCtrl, "AddRow", AddRow, VT_EMPTY, VTS_I4)
	DISP_FUNCTION(CSpreadsheetCtrl, "RemoveRow", RemoveRow, VT_EMPTY, VTS_I4)
	DISP_FUNCTION(CSpreadsheetCtrl, "Clear", Clear, VT_EMPTY, VTS_NONE)
	DISP_PROPERTY_PARAM(CSpreadsheetCtrl, "CellType", GetCellType, SetCellType, VT_I2, VTS_I4 VTS_I4)
	DISP_PROPERTY_PARAM(CSpreadsheetCtrl, "CellText", GetCellText, SetCellText, VT_BSTR, VTS_I4 VTS_I4)
	DISP_PROPERTY_PARAM(CSpreadsheetCtrl, "CellCheck", GetCellCheck, SetCellCheck, VT_BOOL, VTS_I4 VTS_I4)
	DISP_PROPERTY_PARAM(CSpreadsheetCtrl, "ColWidth", GetColWidth, SetColWidth, VT_I4, VTS_I4)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CSpreadsheetCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CSpreadsheetCtrl, COleControl)
	//{{AFX_EVENT_MAP(CSpreadsheetCtrl)
	EVENT_CUSTOM("BoolChanged", FireBoolChanged, VTS_I4  VTS_I4  VTS_BSTR)
	EVENT_CUSTOM("StringChanged", FireStringChanged, VTS_I4  VTS_I4  VTS_BSTR)
	EVENT_CUSTOM("SetChanged", FireSetChanged, VTS_I4  VTS_I4  VTS_BSTR)
	EVENT_CUSTOM("TextDoubleclicked", FireTextDoubleclicked, VTS_I4  VTS_I4  VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CSpreadsheetCtrl, 1)
	PROPPAGEID(CSpreadsheetPropPage::guid)
END_PROPPAGEIDS(CSpreadsheetCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CSpreadsheetCtrl, "SPREADSHEET.SpreadsheetCtrl.1",
	0xfc470417, 0x2bf8, 0x11d2, 0xaf, 0x3, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CSpreadsheetCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DSpreadsheet =
		{ 0xfc470415, 0x2bf8, 0x11d2, { 0xaf, 0x3, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DSpreadsheetEvents =
		{ 0xfc470416, 0x2bf8, 0x11d2, { 0xaf, 0x3, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwSpreadsheetOleMisc =
	OLEMISC_SIMPLEFRAME |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CSpreadsheetCtrl, IDS_SPREADSHEET, _dwSpreadsheetOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CSpreadsheetCtrl::CSpreadsheetCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CSpreadsheetCtrl

BOOL CSpreadsheetCtrl::CSpreadsheetCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegInsertable | afxRegApartmentThreading to afxRegInsertable.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_SPREADSHEET,
			IDB_SPREADSHEET,
			afxRegInsertable | afxRegApartmentThreading,
			_dwSpreadsheetOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CSpreadsheetCtrl::CSpreadsheetCtrl - Constructor

CSpreadsheetCtrl::CSpreadsheetCtrl()
{
	InitializeIIDs(&IID_DSpreadsheet, &IID_DSpreadsheetEvents);
	AfxEnableControlContainer();
	m_CurrentRow    = 1;
	m_CurrentColumn = 1;
	m_InEditMode    = FALSE;
	m_InFocusSet    = FALSE;
	m_SelectionMode = 3;
	m_IgnoreCellChange=FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CSpreadsheetCtrl::~CSpreadsheetCtrl - Destructor

CSpreadsheetCtrl::~CSpreadsheetCtrl()
{
	// TODO: Cleanup your control's instance data here.
}



//--------------------------------------------------------------------------------
// Trying to find a cell with the specified row and column in a list. Returns a 
// pointer to such cell or NULL if no cell defined in a list.
//--------------------------------------------------------------------------------
CCellInfo* CSpreadsheetCtrl::FindCell(int row, int col) {
POSITION at = m_CellsList.GetHeadPosition();
   while(at) {
	   if(m_CellsList.GetAt(at).GetRow()==row && m_CellsList.GetAt(at).GetColumn()==col) 
		   return &m_CellsList.GetAt(at);
       m_CellsList.GetNext(at);
   }
   return NULL;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This local function designed to move and open floating editor.
// Current cell will be used for editing, text from the cell will be inserted 
// into the editor if the update flag is == TRUE;
//--------------------------------------------------------------------------------
void CSpreadsheetCtrl::OpenCellEditor(BOOL update) {
CRect editorPos;

    if(m_InEditMode==TRUE) return;
	m_InEditMode=TRUE;
    editorPos.left   = m_GridWindow.GetCellLeft()/15-3;
    editorPos.top    = m_GridWindow.GetCellTop()/15-3;
    editorPos.right  = editorPos.left+m_GridWindow.GetCellWidth()/15;
    editorPos.bottom = editorPos.top +m_GridWindow.GetCellHeight()/15;

	m_FloatingEditor.MoveWindow( editorPos);
	m_FloatingEditor.EnableWindow(TRUE);
	m_FloatingEditor.ShowWindow(SW_SHOW);
	if(update==TRUE)
       m_FloatingEditor.SetWindowText(m_GridWindow.GetText());
    else
       m_FloatingEditor.SetWindowText("");

}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// This local function designed to close the floating editor. The spreadsheet
// cell text will be updated if the update flag == TRUE (default)
//--------------------------------------------------------------------------------
void CSpreadsheetCtrl::CloseCellEditor(BOOL update) {
CString newText;

    if(m_InEditMode==FALSE) return;
	m_InEditMode=FALSE;

    if(update==TRUE) {
       m_FloatingEditor.GetWindowText(newText);
	   CCellInfo* pCell;
	   pCell=FindCell(m_GridWindow.GetRow(),m_GridWindow.GetCol());
	   if(pCell!=NULL && pCell->GetType()==1) {
		   pCell->SetText(newText);
           m_GridWindow.SetText(newText);
		   FireStringChanged(m_GridWindow.GetRow(),m_GridWindow.GetCol(),newText);
	   }
	}
	m_FloatingEditor.EnableWindow(FALSE);
	m_FloatingEditor.ShowWindow(SW_HIDE);

}
//--------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Moves the focused cell according to the key code.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::MoveCell(short keyCode) {
int rowY;
int windowY;
int colX;
int windowX;
int pgSize;
int oldRow=m_CurrentRow;

    CRect winRect;
    m_GridWindow.GetClientRect(winRect);
	windowY=winRect.Height();
	windowX=winRect.Width();
	pgSize=windowY/(m_GridWindow.GetRowHeight(0)/15);
	switch(keyCode) {
       // Up arrow pressed
	   case 38 : if(m_CurrentRow>m_GridWindow.GetFixedRows()) {
		             m_CurrentRow--;
					 if(m_GridWindow.GetRowIsVisible(m_CurrentRow)==FALSE) {
						 if(m_GridWindow.GetTopRow()>0) 
	                         m_GridWindow.SetTopRow(m_GridWindow.GetTopRow()-1);
					 }
	                 m_GridWindow.SetRow(m_CurrentRow);

				 }
		         break;
       // Down arrow pressed
	   case 40 : if(m_CurrentRow<m_GridWindow.GetRows()-1) {
		             m_CurrentRow++;
	                 rowY   = (m_GridWindow.GetRowPos(m_CurrentRow) + m_GridWindow.GetRowHeight(m_CurrentRow))/15;
					 if((m_GridWindow.GetRowIsVisible(m_CurrentRow)==FALSE) || (rowY > windowY)) {
						 if(m_GridWindow.GetTopRow()<m_GridWindow.GetRows()-1) {
	                         m_GridWindow.SetTopRow(m_GridWindow.GetTopRow()+1);
	                         m_GridWindow.SetRow(m_CurrentRow);
						 }

					 }
	                 m_GridWindow.SetRow(m_CurrentRow);
				 }
		         break;
       // Left arrow pressed
	   case 37 : if(m_CurrentColumn>m_GridWindow.GetFixedCols()) {
		             m_CurrentColumn--;
					 if(m_GridWindow.GetColIsVisible(m_CurrentColumn)==FALSE) {
						 if(m_GridWindow.GetLeftCol()>0) 
	                         m_GridWindow.SetLeftCol(m_GridWindow.GetLeftCol()-1);
					 }
	                 m_GridWindow.SetCol(m_CurrentColumn);
				 }
		         break;
       // Right arrow pressed
	   case 39 : if(m_CurrentColumn<m_GridWindow.GetCols()-1) {
		             m_CurrentColumn++;
	                 colX   = (m_GridWindow.GetColPos(m_CurrentColumn) + m_GridWindow.GetColWidth(m_CurrentColumn))/15;
					 if((m_GridWindow.GetColIsVisible(m_CurrentColumn)==FALSE) || (colX>windowX)) {
						 if(m_GridWindow.GetLeftCol()<m_GridWindow.GetCols()-1) 
	                         m_GridWindow.SetLeftCol(m_GridWindow.GetLeftCol()+1);
					 }
	                 m_GridWindow.SetCol(m_CurrentColumn);
				 }
		         break;
       // PgUp pressed
	   case 33 : if(m_CurrentRow==m_GridWindow.GetFixedRows()) break;		   
				 if((m_CurrentRow-pgSize)<m_GridWindow.GetFixedRows()) {
					 m_CurrentRow=m_GridWindow.GetFixedRows();
				 } else {
					 m_CurrentRow-=pgSize;
				 }
				 while(m_GridWindow.GetRowIsVisible(m_CurrentRow)==FALSE) {
	                  m_GridWindow.SetTopRow(m_GridWindow.GetTopRow()-1);
				 }
	             m_GridWindow.SetRow(m_CurrentRow);
		         break;
       // PgDn  pressed
	   case 34 : if(m_CurrentRow==(m_GridWindow.GetRows()-1)) break;		   
				 if((m_CurrentRow+pgSize)>m_GridWindow.GetRows()) {
					 m_CurrentRow=m_GridWindow.GetRows()-1;
				 } else {
		             m_CurrentRow+=pgSize;
				 }

	             rowY   = (m_GridWindow.GetRowPos(m_CurrentRow) + m_GridWindow.GetRowHeight(m_CurrentRow))/15;
			     while((m_GridWindow.GetRowIsVisible(m_CurrentRow)==FALSE) || (rowY > windowY)) {
	                 m_GridWindow.SetTopRow(m_GridWindow.GetTopRow()+1);
	                 rowY   = (m_GridWindow.GetRowPos(m_CurrentRow) + m_GridWindow.GetRowHeight(m_CurrentRow))/15;
				 }
	             m_GridWindow.SetRow(m_CurrentRow);
		         break;
	   default :  return;
	}
   if((m_SelectionMode==3) && (oldRow!=m_CurrentRow)) {
      m_GridWindow.SetCol(0);
      m_GridWindow.SetRow(oldRow);
      m_GridWindow.SetRefCellPicture(NULL);

      m_GridWindow.SetCol(0);
      m_GridWindow.SetRow(m_CurrentRow);
      m_GridWindow.SetRefCellPicture(m_IMarker);
      m_GridWindow.SetRow(m_CurrentRow);
      m_GridWindow.SetCol(m_CurrentColumn);
   }
}
//-----------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// CSpreadsheetCtrl::OnDraw - Drawing function

void CSpreadsheetCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	pdc->Ellipse(rcBounds);
}


/////////////////////////////////////////////////////////////////////////////
// CSpreadsheetCtrl::DoPropExchange - Persistence support

void CSpreadsheetCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CSpreadsheetCtrl::OnResetState - Reset control to default state

void CSpreadsheetCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CSpreadsheetCtrl::AboutBox - Display an "About" box to the user

void CSpreadsheetCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_SPREADSHEET);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CSpreadsheetCtrl message handlers

int CSpreadsheetCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if(!m_GridWindow.Create("Grid", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CRect(lpCreateStruct->x,
		      lpCreateStruct->y,
			  lpCreateStruct->cx,
			  lpCreateStruct->cy),
		this, 
		IDC_GRID))
		return -1;

	OldWindowProc=(WindowProcType)::SetWindowLong(m_GridWindow,GWL_WNDPROC,(long)NewWindowProc);

	m_FloatingEditor.Create(WS_CHILD | ES_AUTOHSCROLL,
				            CRect(lpCreateStruct->x,
		                          lpCreateStruct->y,
			                      lpCreateStruct->cx,
			                      lpCreateStruct->cy),
							&m_GridWindow,
							IDC_FLOATEDITOR);
	m_InfoFont.CreateFont(14,0,0,0,FW_BOLD,0,0,0,0,0,0,0,0,"Arial");
	m_FloatingEditor.SetFont(&m_InfoFont,FALSE);
	m_FloatingEditor.Destination=this;
	// Default amount of rows/columns
	m_GridWindow.SetRows(10);
	m_GridWindow.SetCols(10);
	// The first selected row/column will be the first non-fixed cell
	m_CurrentRow    = m_GridWindow.GetFixedRows();
	m_CurrentColumn = m_GridWindow.GetFixedCols();
	m_GridWindow.SetRow(m_CurrentRow);
	m_GridWindow.SetCol(m_CurrentColumn);

	// By default, we will allow user to resize columns, we will limit
	// row height to completely contain checkbox image and we will alow only
	// complete row selection.
    m_GridWindow.SetSelectionMode(1);
    m_GridWindow.SetAllowUserResizing(1);
    m_GridWindow.SetFixedCols(1);

    PICTDESC    description;
    LPDISPATCH  iPictDisp;
	m_Marker.LoadMappedBitmap(IDB_MARKER);
	description.cbSizeofstruct=sizeof(description);
    description.picType=PICTYPE_BITMAP;
    description.bmp.hbitmap=m_Marker;
    ::OleCreatePictureIndirect(&description, IID_IDispatch,TRUE,(void **)&iPictDisp);
	 m_IMarker=iPictDisp;

    m_GridWindow.SetRow(1);
    m_GridWindow.SetCol(0);
    m_GridWindow.SetRefCellPicture(m_IMarker);
    m_GridWindow.SetCol(1);

	m_FloatingEditor.EnableWindow(FALSE);
	m_FloatingEditor.ShowWindow(SW_HIDE);

	return 0;
}

void CSpreadsheetCtrl::OnSize(UINT nType, int cx, int cy)  {
	COleControl::OnSize(nType, cx, cy);
    if(::IsWindow(m_GridWindow)) {
		CloseCellEditor();
		m_GridWindow.MoveWindow(0,0,cx,cy);	
	}
}

//-------------------------------------------------------------------------------------------
// Callback runs every time the user press the key in the spreadsheet (grid generates KeyDown
// event). We need this callback to perform spreadsheet cell positioning and spreadsheet
// scrolling.
//-------------------------------------------------------------------------------------------
void CSpreadsheetCtrl::OnKeyDownEvent(short FAR* KeyCode, short Shift) {
CCellInfo* pCell;


	if(*KeyCode==13) {
        pCell=FindCell(m_GridWindow.GetRow(),m_GridWindow.GetCol());
        if(pCell==NULL) return;
        switch(pCell->GetType()) {
	       case 1 : OpenCellEditor();
		            m_FloatingEditor.SetFocus();
				    break;
	       case 2 : if(pCell->GetCheckState()==TRUE) {
		               pCell->SetCheckState(FALSE);
	                   LPDISPATCH pDispatch = pCell->GetUncheckedImageInterface();
	                   m_GridWindow.SetRefCellPicture(pDispatch);
					} else {
		               pCell->SetCheckState(TRUE);
	                   LPDISPATCH pDispatch = pCell->GetCheckedImageInterface();
	                   m_GridWindow.SetRefCellPicture(pDispatch);
					}
				    break;
	       case 3 : POINT menuBase;
				    menuBase.x=m_GridWindow.GetCellLeft()/15;
				    menuBase.y=m_GridWindow.GetCellTop()/15;
				    m_GridWindow.ClientToScreen(&menuBase);
				    int index = ::TrackPopupMenu(*(pCell->GetMenu()),
					                          TPM_LEFTALIGN | 
										      TPM_RETURNCMD | 
										      TPM_LEFTBUTTON, 
					                          menuBase.x, 
										      menuBase.y, 
											  0,
										      m_GridWindow,
											  NULL);
				    CString selection;
			        pCell->GetMenu()->GetMenuString(index, selection, MF_BYCOMMAND);
	                SetCellText(m_CurrentRow,m_CurrentColumn,selection);
		            break;
		}
	} else MoveCell(*KeyCode);
}
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// Callback runs every time the user press mouse button on the spreadsheet. We need this
// callback to position the row/column because in the case of only row or only column
// selection mouse click position the active cell to the first one.
//-------------------------------------------------------------------------------------------
void CSpreadsheetCtrl::OnMouseDown(short Button, short Shift, long x, long y) {
CCellInfo* pCell;
int row = m_GridWindow.GetMouseRow();
int col = m_GridWindow.GetMouseCol();


   if(m_InFocusSet==TRUE) return;

   if(row<m_GridWindow.GetFixedRows()) row=m_GridWindow.GetFixedRows();

   if((m_SelectionMode==3) && (row!=m_CurrentRow)) {
      m_GridWindow.SetCol(0);
      m_GridWindow.SetRow(m_CurrentRow);
      m_GridWindow.SetRefCellPicture(NULL);
   

      m_GridWindow.SetRow(row);
      m_GridWindow.SetRefCellPicture(m_IMarker);
      m_GridWindow.SetRow(m_CurrentRow);
      m_GridWindow.SetCol(m_CurrentColumn);
   }


   
   if(row>=m_GridWindow.GetFixedRows()) m_CurrentRow = row;
   else                                 m_CurrentRow = m_GridWindow.GetFixedRows();

   if(col>=m_GridWindow.GetFixedCols()) m_CurrentColumn = col;
   else {
      m_GridWindow.SetCol(m_CurrentColumn);
      m_GridWindow.SetRow(m_CurrentRow);
	  return;
   }


   m_GridWindow.SetCol(m_CurrentColumn);
   m_GridWindow.SetRow(m_CurrentRow);

   pCell=FindCell(m_CurrentRow,m_CurrentColumn);

   if(pCell==NULL) return;
   switch(pCell->GetType()) {
	   case 0 : break;
	   case 1 : break;
	   case 2 : if(pCell->GetCheckState()==TRUE) {
		             pCell->SetCheckState(FALSE);
	                 LPDISPATCH pDispatch = pCell->GetUncheckedImageInterface();
	                 m_GridWindow.SetRefCellPicture(pDispatch);
				     FireBoolChanged(m_GridWindow.GetRow(),m_GridWindow.GetCol(),"0");
				} else {
		             pCell->SetCheckState(TRUE);
	                 LPDISPATCH pDispatch = pCell->GetCheckedImageInterface();
	                 m_GridWindow.SetRefCellPicture(pDispatch);
				     FireBoolChanged(m_GridWindow.GetRow(),m_GridWindow.GetCol(),"1");
				}
				break;
	   case 3 : POINT menuBase;
				menuBase.x=m_GridWindow.GetCellLeft()/15;
				menuBase.y=m_GridWindow.GetCellTop()/15;
				m_GridWindow.ClientToScreen(&menuBase);
				int index = ::TrackPopupMenu(*(pCell->GetMenu()),
					                          TPM_LEFTALIGN | 
										      TPM_RETURNCMD | 
										      TPM_LEFTBUTTON, 
					                         menuBase.x, 
										     menuBase.y, 
											 0,
										     m_GridWindow,
											 NULL);
				if(index!=0) {
				   CString selection;
			       pCell->GetMenu()->GetMenuString(index, selection, MF_BYCOMMAND);
	               SetCellText(m_CurrentRow,m_CurrentColumn,selection);
				   FireSetChanged(m_GridWindow.GetRow(),m_GridWindow.GetCol(),selection);
				}
		        break;

	}
}
//-------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------
// This property Get method retuns the  cell type or -2 if cell does not exist.
// For the empty cells we will return -1 as a type.
// The cell is specified by row and col parameters.
//-------------------------------------------------------------------------------------------
short CSpreadsheetCtrl::GetCellType(long row, long col)  {
CCellInfo* pCell;

    // If the row/col outside the range we will return -2 as a cell type    
    if(row>=m_GridWindow.GetRows() || col>=m_GridWindow.GetCols()) return -2;

	pCell=FindCell(row,col);

    // For the empty cells we will return -1 as a type.
	if(pCell==NULL) return -1;

	return pCell->GetType();

}
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// This property Set method sets the cell type.
// The cellis spesified by row and col parameters.
// A new CCellInfo class instance will be created if no one exists.
//-------------------------------------------------------------------------------------------
void CSpreadsheetCtrl::SetCellType(long row, long col, short nCellType) {
CCellInfo* pCell;
LPDISPATCH pDispatch;


     // If the row/col outside the range we will do nothing
    if(row>=m_GridWindow.GetRows() || col>=m_GridWindow.GetCols()) return;

    if(m_CurrentRow==row && m_CurrentColumn==col) CloseCellEditor();

    m_IgnoreCellChange=TRUE;
    m_InFocusSet=TRUE;


	pCell=FindCell(row,col);
	if(pCell!=NULL) pCell->SetType(nCellType);
	else { // If the cell was empty we will create a new one.
	   CCellInfo newCell;
       newCell.SetType(nCellType);
	   newCell.SetRow(row);
	   newCell.SetColumn(col);
	   m_CellsList.AddHead(newCell);
	   pCell=FindCell(row,col);
	}

	// Changing the cell type will result in new menu and images creation if
	// the cell is of boolean or set type, as well as creation of the new cell.

	switch(nCellType) {
	   // Read-only text
	   case 0 : pCell->SetText(CString(""));
                pCell->SetCheckedImage   (0);
                pCell->SetUncheckedImage (0);
		        m_GridWindow.SetRow(row);
                m_GridWindow.SetCol(col);
		        m_GridWindow.SetRefCellPicture(NULL);
				break;
	   // Editable text
	   case 1 : pCell->SetText(CString(""));
                pCell->SetCheckedImage   (0);
                pCell->SetUncheckedImage (0);
		        m_GridWindow.SetRow(row);
                m_GridWindow.SetCol(col);
		        m_GridWindow.SetRefCellPicture(NULL); 
		        // If we changed the type of the current cell into editable,
		        // we need to open cell editor.
		        //if(m_CurrentRow==row && m_CurrentColumn==col) OpenCellEditor();
				break;
	   // Boolean cell with checkbox in it
	   case 2 :  // We need to enter the cell first - the base MSFlexGrid ActiveX
		         // can operate only with the current cell.
				 m_CurrentRow    = row;
				 m_CurrentColumn = col;
		         m_GridWindow.SetRow(m_CurrentRow);
                 m_GridWindow.SetCol(m_CurrentColumn);
				 // We will use the default images for the checked/unchecked status.
			     // One can change them using CheckedImage and UncheckedImage properties.
				 pCell=FindCell(row,col);
                 pCell->SetCheckedImage   (IDB_CHECKED);
                 pCell->SetUncheckedImage (IDB_UNCHECKED);
				 // No text will be displayed in the boolean cell
				 m_GridWindow.SetText("");
				 // The default cell status is UNCHECHED. You can change it by using
				 // CellStatus property.
                 pCell->SetCheckState(FALSE);
	             pDispatch = pCell->GetUncheckedImageInterface();
	             m_GridWindow.SetRefCellPicture(pDispatch);
				 break;
	   // Set type - popup menu must be attached
	   case 3 :  // The initial menu is empty. You can use AddMenuItem method to add
		         // new items to the menu or ClearMenu method to remove all items from
		         // the menu. AddMenu method will return you the menu item ID which
		         // you can use in the responce table.
		         pCell->SetMenu();
		         m_GridWindow.SetRow(row);
                 m_GridWindow.SetCol(col);
		         m_GridWindow.SetRefCellPicture(NULL); 
	}

   m_IgnoreCellChange=FALSE;
   m_InFocusSet=FALSE;
}
//-------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This callback will run every time we are entereing the cell by mouse click or by 
// keyboard actions. We will open the editor in the case we are entering the editable cell.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::OnEnterCell(void)  {
CCellInfo* pCell;

   if(m_IgnoreCellChange==TRUE) return;

   pCell=FindCell(m_GridWindow.GetRow(),m_GridWindow.GetCol());
   if(pCell==NULL) return;
   switch(pCell->GetType()) {
	   case 1 : OpenCellEditor();
		        m_FloatingEditor.SetFocus();
				break;
   }
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This callback will run every time we are quitting the cell by mouse click or by 
// keyboard actions. We will open the editor in the case we are entering the editable cell.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::OnLeaveCell(void)  {
   if(m_IgnoreCellChange==TRUE) return;
   CloseCellEditor();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This Get method of the CellText property will return the text assigned to the cell or
// empty string if you are trying to get text from the empty cell,
//-----------------------------------------------------------------------------------------
BSTR CSpreadsheetCtrl::GetCellText(long row, long col)  {
CCellInfo* pCell;
	CString strResult;

   //  Looking for the cell with spesified row and column
   pCell=FindCell(row,col);
   // If no cell defined we will return empty string
   if(pCell==NULL) strResult="";
   else            strResult=pCell->GetText();

   return strResult.AllocSysString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This Set method of the CellText property will assign the text to the cell and will
// create a new cSellInfo object if the cell was empty.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::SetCellText(long row, long col, LPCTSTR lpstrText)  {
CCellInfo* pCell;
   m_IgnoreCellChange=TRUE;
   m_InFocusSet=TRUE;
   CloseCellEditor();
   //  Looking for the cell with spesified row and column
   pCell=FindCell(row,col);
   // If no cell defined we will create a new one
   if(pCell==NULL) {
      CCellInfo info;
      info.SetType(0);
	  info.SetRow(row);
	  info.SetColumn(col);
	  pCell=&m_CellsList.GetAt(m_CellsList.AddHead(info));
   }
   pCell->SetText(CString(lpstrText));
   m_CurrentRow   =row;
   m_CurrentColumn=col;
   m_GridWindow.SetRow(m_CurrentRow);
   m_GridWindow.SetCol(m_CurrentColumn);
   m_GridWindow.SetText(lpstrText);
   m_IgnoreCellChange=FALSE;
   m_InFocusSet=FALSE;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Call this method to add menu item to the set- type spreadsheet cell. If no menu assigned
// to this cell this function will do nothing. 
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::AddMenuItem(long row, long col, LPCTSTR text)  {
CCellInfo* pCell;
CMenu*     pMenu;

   //  Looking for the cell with spesified row and column
   pCell=FindCell(row,col);
   // If not found
   if(pCell==NULL)       return;
   pMenu=pCell->GetMenu();
   // If no menu assigned to the cell
   if(pMenu==NULL) return;

   pMenu->AppendMenu(MF_STRING,pCell->NewMenuCommand() ,text);

}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Runs every time the spreadsheet scrolls.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::OnScroll()  {
  CloseCellEditor();

}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Runs every time the user double-click the cell
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::OnDblClick()  {
  FireTextDoubleclicked(m_GridWindow.GetRow(),m_GridWindow.GetCol(),m_GridWindow.GetText());
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// We can use this property method to inspect boolean cell status.
//-----------------------------------------------------------------------------------------
BOOL CSpreadsheetCtrl::GetCellCheck(long row, long col)  {
CCellInfo* pCell;

   //  Looking for the cell with spesified row and column
   pCell=FindCell(row,col);
   // If not found
   if(pCell==NULL) return FALSE;
   return pCell->GetCheckState();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// We can use this property Pit method to set the cell check status.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::SetCellCheck(long row, long col, BOOL bNewValue)  {
CCellInfo* pCell;
   //  Looking for the cell with spesified row and column
   pCell=FindCell(row,col);
   // If not found doing nothing
   if(pCell==NULL) return;

   CloseCellEditor();


   // This status was already set.
   if(pCell->GetCheckState()==bNewValue) return;
	
   m_IgnoreCellChange=TRUE;
   m_InFocusSet=TRUE;

   pCell->SetCheckState(bNewValue);

   m_CurrentRow=row;
   m_CurrentColumn=col;
   m_GridWindow.SetRow(m_CurrentRow);
   m_GridWindow.SetCol(m_CurrentColumn);


   if(bNewValue==FALSE) {
	    LPDISPATCH pDispatch = pCell->GetUncheckedImageInterface();
	    m_GridWindow.SetRefCellPicture(pDispatch);
   } else {
	    LPDISPATCH pDispatch = pCell->GetCheckedImageInterface();
	    m_GridWindow.SetRefCellPicture(pDispatch);
   }
   m_IgnoreCellChange=FALSE;
   m_InFocusSet=FALSE;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method adds the new empty row after the one passed as parameter.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::AddRow(long after)  {
POSITION at = m_CellsList.GetHeadPosition();
CCellInfo *p;

   CloseCellEditor();
   m_IgnoreCellChange=TRUE;
   if(after>m_GridWindow.GetRows()-m_GridWindow.GetFixedRows()) 
	   after = m_GridWindow.GetRows()-m_GridWindow.GetFixedRows();
   m_GridWindow.SetRows(m_GridWindow.GetRows()+1);
   while(at) {
	   p=&m_CellsList.GetAt(at);
	   if(p->GetRow()>after) {
		   // Cleaning new table row
		   if(p->GetRow()==after+1) {
                m_GridWindow.SetRow(p->GetRow());
                m_GridWindow.SetCol(p->GetColumn());
                m_GridWindow.SetText("");        
		        m_GridWindow.SetRefCellPicture(NULL);
		   }
	       p->SetRow(p->GetRow()+1);
           m_GridWindow.SetRow(p->GetRow());
           m_GridWindow.SetCol(p->GetColumn());
           m_GridWindow.SetText(p->GetText());        
	       if(p->GetType()==2) {
	          if(p->GetCheckState()==TRUE) {
	             m_GridWindow.SetRefCellPicture(p->GetCheckedImageInterface());
			  } else {
	             m_GridWindow.SetRefCellPicture(p->GetUncheckedImageInterface());
			  }
		   } else m_GridWindow.SetRefCellPicture(NULL);
	   }
       m_CellsList.GetNext(at);
   }
   m_GridWindow.Invalidate();


   if((m_SelectionMode==3)) {
       m_GridWindow.SetCol(0);
       m_GridWindow.SetRow(m_CurrentRow);
       m_GridWindow.SetRefCellPicture(NULL);
   }
   m_CurrentRow++;
   if((m_SelectionMode==3)) {
       m_GridWindow.SetCol(0);
       m_GridWindow.SetRow(m_CurrentRow);
       m_GridWindow.SetRefCellPicture(m_IMarker);
   }
   SetFocus();
   m_IgnoreCellChange=FALSE;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method removes the row passed as a parameter.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::RemoveRow(long row)  {
POSITION at = m_CellsList.GetHeadPosition();
CCellInfo *p;

   if( row<m_GridWindow.GetFixedRows() || 
	   row>m_GridWindow.GetRows()-1) return;


   CloseCellEditor();

   m_GridWindow.SetRows(m_GridWindow.GetRows()-1);
   m_IgnoreCellChange=TRUE;
   while(at) {
	   p=&m_CellsList.GetAt(at);

	   if(p->GetRow()==row) {
		    POSITION pos = at;
            m_CellsList.GetNext(at);
            m_CellsList.RemoveAt(pos);
		    continue;
	   }

	   if(p->GetRow()>row) {
	       p->SetRow(p->GetRow()-1);
           m_GridWindow.SetRow(p->GetRow());
           m_GridWindow.SetCol(p->GetColumn());
           m_GridWindow.SetText(p->GetText());        
	       if(p->GetType()==2) {
	           if(p->GetCheckState()==TRUE) {
	               m_GridWindow.SetRefCellPicture(p->GetCheckedImageInterface());
			   } else {
	               m_GridWindow.SetRefCellPicture(p->GetUncheckedImageInterface());
			   }
		   } else m_GridWindow.SetRefCellPicture(NULL);
	   }
       m_CellsList.GetNext(at);
   }

   if(m_CurrentRow>=m_GridWindow.GetRows()) {
	   m_CurrentRow--;
   }

   if((m_SelectionMode==3) && row>m_GridWindow.GetFixedRows()) {
      m_GridWindow.SetCol(0);
      m_GridWindow.SetRow(m_CurrentRow);
      m_GridWindow.SetRefCellPicture(m_IMarker);
   }

   m_GridWindow.Invalidate();

   SetFocus();
   m_IgnoreCellChange=FALSE;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method gets the number of rows in the spreadsheet by deligating the call to the
// base MS Flex Ggid component.
//-----------------------------------------------------------------------------------------
long CSpreadsheetCtrl::GetRows()  {

	return m_GridWindow.GetRows();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method sets the number of rows in the spreadsheet by deligating the call to the
// base MS Flex Ggid component.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::SetRows(long nNewValue)  {

	m_GridWindow.SetRows(nNewValue);

}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method gets the number of columns in the spreadsheet by deligating the call to the
// base MS Flex Ggid component.
//-----------------------------------------------------------------------------------------
long CSpreadsheetCtrl::GetCols()  {
	return m_GridWindow.GetCols();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method sets the number of columns in the spreadsheet by deligating the call to the
// base MS Flex Ggid component.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::SetCols(long nNewValue)  {
	m_GridWindow.SetCols(nNewValue);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method gets the number of fixed rows in the spreadsheet by deligating the call to the
// base MS Flex Ggid component.
//-----------------------------------------------------------------------------------------
long CSpreadsheetCtrl::GetFixedRows()  {
	return m_GridWindow.GetFixedRows();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method sets the number of fixed rows in the spreadsheet by deligating the call to the
// base MS Flex Ggid component.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::SetFixedRows(long nNewValue)  {
	m_GridWindow.SetFixedRows(nNewValue);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This method gets the number of fixed Columns in the spreadsheet by deligating the call to the
// base MS Flex Ggid component.
//-----------------------------------------------------------------------------------------
long CSpreadsheetCtrl::GetFixedCols()  {
	return m_GridWindow.GetFixedCols();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method sets the number of fixed columns in the spreadsheet by deligating the call to the
// base MS Flex Ggid component.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::SetFixedCols(long nNewValue)  {
	m_GridWindow.SetFixedCols(nNewValue);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This Get method of the UserSelection property returns the current selection mode for the
// spreadsheet. The possible selections modes are:
//   0 - Free 
//   1 - By row
//   2 - By column
//   3 - Row marker 
//------------------------------------------------------------------------------------------
short CSpreadsheetCtrl::GetUserSelection()  {

	return m_SelectionMode;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CSpreadsheetCtrl::SetUserSelection(short nNewValue)  {
	if(nNewValue<3){
		if(m_SelectionMode==3) {
		   m_IgnoreCellChange=TRUE;
           m_GridWindow.SetCol(0);
           m_GridWindow.SetRefCellPicture(NULL);
           m_GridWindow.SetCol(m_CurrentColumn);
		   m_IgnoreCellChange=FALSE;
		}
		m_SelectionMode=nNewValue;
		m_GridWindow.SetSelectionMode(m_SelectionMode);
	} else {
		m_SelectionMode=3;
		m_GridWindow.SetSelectionMode(1);
	}

}
//------------------------------------------------------------------------------------------



LRESULT CALLBACK NewWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if(message==WM_GETDLGCODE) {
	   return DLGC_WANTALLKEYS;
	}
	if(message==WM_PAINT && in_add==TRUE) {
	     return 0;
	}
	return CallWindowProc(OldWindowProc,hWnd,message,wParam,lParam); 
}

UINT CSpreadsheetCtrl::OnGetDlgCode()  {
	return DLGC_WANTALLKEYS;
}


//-----------------------------------------------------------------------------------------
// Need this callback to manualy set focus to the old cell (when switching between dialog)
// items by TAB key.
//-----------------------------------------------------------------------------------------
void CSpreadsheetCtrl::OnSetFocus(CWnd* pOldWnd)  {
int x,y;

	COleControl::OnSetFocus(pOldWnd);
   if(m_GridWindow.GetRows()<=m_GridWindow.GetFixedRows()) return;

    m_InFocusSet=TRUE;

	m_GridWindow.SetRow(m_CurrentRow);
	m_GridWindow.SetCol(m_CurrentColumn);
	x=m_GridWindow.GetCellLeft();
	y=m_GridWindow.GetCellTop();
    m_GridWindow.SendMessage(WM_LBUTTONDOWN,MK_LBUTTON,(y<<16)+x);

	m_GridWindow.SetRow(m_CurrentRow);
	m_GridWindow.SetCol(m_CurrentColumn);
	x=m_GridWindow.GetCellLeft();
	y=m_GridWindow.GetCellTop();
    m_GridWindow.SendMessage(WM_LBUTTONUP,MK_LBUTTON,(y<<16)+x);

	m_InFocusSet=FALSE;
	
}
//-----------------------------------------------------------------------------------------


long CSpreadsheetCtrl::GetColWidth(long col)  {
	return m_GridWindow.GetColWidth(col)/15;
}

void CSpreadsheetCtrl::SetColWidth(long col, long nNewValue)  {
	m_GridWindow.SetColWidth(col,nNewValue*15);
}

long CSpreadsheetCtrl::GetCurrentRow()  {
	return m_CurrentRow;
}

void CSpreadsheetCtrl::SetCurrentRow(long nNewValue)  {
   if(m_GridWindow.GetRows()<=m_GridWindow.GetFixedRows()) return;
   if((m_SelectionMode==3)) {
       m_GridWindow.SetCol(0);
       m_GridWindow.SetRow(m_CurrentRow);
       m_GridWindow.SetRefCellPicture(NULL);
   }
   m_CurrentRow=nNewValue;
   if((m_SelectionMode==3)) {
       m_GridWindow.SetCol(0);
       m_GridWindow.SetRow(m_CurrentRow);
       m_GridWindow.SetRefCellPicture(m_IMarker);
   }
    m_GridWindow.SetCol(m_CurrentColumn);
}

long CSpreadsheetCtrl::GetCurrentColumn()  {
	return m_CurrentColumn;
}

void CSpreadsheetCtrl::SetCurrentColumn(long nNewValue)  {
	m_CurrentColumn=nNewValue;
	m_GridWindow.SetCol(m_CurrentColumn);
}

BOOL CSpreadsheetCtrl::GetEnableDraw()  {
	return !in_add;
}

void CSpreadsheetCtrl::SetEnableDraw(BOOL bNewValue)  {
   in_add=!bNewValue;
   if(in_add==TRUE) Invalidate();
}

void CSpreadsheetCtrl::Clear()  {
   CloseCellEditor();
   m_CellsList.RemoveAll();
   m_CurrentRow=0;
   m_GridWindow.SetRows(m_GridWindow.GetFixedRows());
}
