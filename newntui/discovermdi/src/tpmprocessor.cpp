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
// TpmProcessor.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "TpmProcessor.h"
#include "NewFilter.h"
#include "TPMEditor.h"

#define SPACE 5

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTpmProcessor

IMPLEMENT_DYNCREATE(CTpmProcessor, CFormView)

CTpmProcessor::CTpmProcessor()
	: CFormView(CTpmProcessor::IDD)
{
	EnableAutomation();
	m_Updated=FALSE;
	//{{AFX_DATA_INIT(CTpmProcessor)
	//}}AFX_DATA_INIT
}

CTpmProcessor::~CTpmProcessor() {

}

void CTpmProcessor::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CFormView::OnFinalRelease();
}

void CTpmProcessor::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTpmProcessor)
	DDX_Control(pDX, IDC_INCLUSIVE, m_Inclusive);
	DDX_Control(pDX, IDC_EDITROW, m_EditRow);
	DDX_Control(pDX, IDC_FILTERTYPE, m_FilterType);
	DDX_Control(pDX, IDC_MODESELECTOR, m_ModeSelector);
	DDX_Control(pDX, IDC_NEWFILTER, m_NewFilter);
	DDX_Control(pDX, IDC_FILTERSELECTOR, m_FilterSelector);
	DDX_Control(pDX, IDC_DELROW, m_DelRow);
	DDX_Control(pDX, IDC_ADDROW, m_AddRow);
	DDX_Control(pDX, IDC_TEXTSPREADSHEET, m_TextSpreadsheet);
	DDX_Control(pDX, IDC_TYPESPREADSHEET, m_TypeSpreadsheet);
	DDX_Control(pDX, IDC_TPMSPREADSHEET, m_TpmSpreadsheet);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	//}}AFX_DATA_MAP
	CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
	m_DataSource.SetSource(app->m_ServiceName,TRUE);
}


BEGIN_MESSAGE_MAP(CTpmProcessor, CFormView)
	//{{AFX_MSG_MAP(CTpmProcessor)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_FILTERTYPE, OnSelchangeFilterType)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MODESELECTOR, OnSelchangeMode)
	ON_BN_CLICKED(IDC_ADDROW, OnNewRow)
	ON_BN_CLICKED(IDC_DELROW, OnDelrow)
	ON_BN_CLICKED(IDC_NEWFILTER, OnNewFilter)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_FILTERSELECTOR, OnSelchangeFilter)
	ON_BN_CLICKED(IDC_EDITROW, OnEditRow)
	ON_NOTIFY(TCN_SELCHANGING, IDC_MODESELECTOR, OnSelchangingMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CTpmProcessor, CFormView)
	//{{AFX_DISPATCH_MAP(CTpmProcessor)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_ITpmProcessor to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {F44373E1-2D53-11D2-AF05-00A0C9B71DC4}
static const IID IID_ITpmProcessor =
{ 0xf44373e1, 0x2d53, 0x11d2, { 0xaf, 0x5, 0x0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };

BEGIN_INTERFACE_MAP(CTpmProcessor, CFormView)
	INTERFACE_PART(CTpmProcessor, IID_ITpmProcessor, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTpmProcessor diagnostics

#ifdef _DEBUG
void CTpmProcessor::AssertValid() const
{
	CFormView::AssertValid();
}

void CTpmProcessor::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTpmProcessor message handlers


//-----------------------------------------------------------------------------------------
// This callback runs every time the user changes size of the child window. We need our
// own processing to perform layout for all controls.
//-----------------------------------------------------------------------------------------
void CTpmProcessor::OnSize(UINT nType, int cx, int cy)  {
CFormView::OnSize(nType, cx, cy);

	   if(::IsWindow(m_ModeSelector)) {
	       if(cx<300) cx=300;
	       if(cy<200) cy=200;
           switch(m_ModeSelector.GetCurSel()) {
		      case 0 : LayoutFilterPage(cx,cy); break;
		      case 1 : LayoutTpmPage(cx,cy);    break;
		   }
	   }
}
//-----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// The framework will run this function when all window elements are already created.
// We will use this callback to initialize tab controls and scroll bars ranges.
//----------------------------------------------------------------------------------------
void CTpmProcessor::OnInitialUpdate()  {
	CFormView::OnInitialUpdate();
	m_TypeSpreadsheet.SetFocus();
	
    if(m_Updated==TRUE) return;
	m_Updated=TRUE;

	TC_ITEM item;
	item.mask         = TCIF_TEXT;
    item.pszText      = "Filter";     
    item.cchTextMax   = 20;     
	m_ModeSelector.InsertItem(0,&item);

	item.mask         = TCIF_TEXT;
    item.pszText      = "Pattern";     
    item.cchTextMax   = 20;     
	m_ModeSelector.InsertItem(1,&item);

    item.pszText      = "Type Matching";     
    item.cchTextMax   = 20;     
	m_FilterType.InsertItem(0,&item);

	item.mask         = TCIF_TEXT;
    item.pszText      = "Text Matching";     
    item.cchTextMax   = 20;     
	m_FilterType.InsertItem(1,&item);


	m_InfoFont.CreateFont(14,0,0,0,0,0,0,0,0,0,0,0,0,"Arial");
	m_ModeSelector.SetFont(&m_InfoFont,FALSE);

	CRect mainRect;
	GetClientRect(&mainRect);

    SIZE sizeTotal;
	sizeTotal.cx=300;
	sizeTotal.cy=200;
	SetScrollSizes( MM_TEXT, sizeTotal);

	LayoutFilterPage(mainRect.Width(),mainRect.Height());

    // Setting spreadsheets parameters.


	// Type spreadsheet initial state
	m_TypeSpreadsheet.SetFixedRows(1);
	m_TypeSpreadsheet.SetRows(1);
	m_TypeSpreadsheet.SetCols(4);
	m_TypeSpreadsheet.SetFixedCols(1);
	m_TypeSpreadsheet.SetCellText(0,0,"#");
	m_TypeSpreadsheet.SetColWidth(0,50);
	m_TypeSpreadsheet.SetCellText(0,1,"Include");
	m_TypeSpreadsheet.SetColWidth(1,50);
	m_TypeSpreadsheet.SetCellText(0,2,"C/C++ Type");
	m_TypeSpreadsheet.SetColWidth(2,200);
	m_TypeSpreadsheet.SetCellText(0,3,"Array Size");
	m_TypeSpreadsheet.SetColWidth(3,80);


	// Text spreadsheet initial state
	m_TextSpreadsheet.SetFixedRows(1);
	m_TextSpreadsheet.SetRows(1);
	m_TextSpreadsheet.SetCols(6);
	m_TextSpreadsheet.SetFixedCols(1);
	m_TextSpreadsheet.SetCellText(0,0,"#");
    m_TextSpreadsheet.SetColWidth(0,50);
    m_TextSpreadsheet.SetCellText(0,1,"Include");
	m_TextSpreadsheet.SetColWidth(1,50);
	m_TextSpreadsheet.SetCellText(0,2,"Base String");
	m_TextSpreadsheet.SetColWidth(2,200);
	m_TextSpreadsheet.SetCellText(0,3,"Mixed Case");
	m_TextSpreadsheet.SetColWidth(3,70);
	m_TextSpreadsheet.SetCellText(0,4,"Entity Name");
	m_TextSpreadsheet.SetColWidth(4,70);
	m_TextSpreadsheet.SetCellText(0,5,"Strings");
	m_TextSpreadsheet.SetColWidth(5,70);

	// TPM spreadsheet initial state
	m_TpmSpreadsheet.SetFixedRows(1);
	m_TpmSpreadsheet.SetRows(1);
	m_TpmSpreadsheet.SetCols(4);
	m_TpmSpreadsheet.SetFixedCols(1);
	m_TpmSpreadsheet.SetCellText(0,0,"#");
    m_TpmSpreadsheet.SetColWidth(0,50);
    m_TpmSpreadsheet.SetCellText(0,1,"Name");
	m_TpmSpreadsheet.SetColWidth(1,100);
	m_TpmSpreadsheet.SetCellText(0,2,"Comment");
	m_TpmSpreadsheet.SetColWidth(2,200);
	m_TpmSpreadsheet.SetCellText(0,3,"TPM expression");
	m_TpmSpreadsheet.SetColWidth(3,200);

	FillFilterSelector();
}
//----------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// This function will position all Filter Page controls on the filter page. 
// (Visible and hidden).
//-----------------------------------------------------------------------------------------
void CTpmProcessor::LayoutFilterPage(int cx, int cy) {
CRect rectNewFilter;
CRect rectAddRow;
CRect rectDelRow;
int spYStart=0;



	m_NewFilter.GetWindowRect(&rectNewFilter);

    m_ModeSelector.MoveWindow(0,spYStart,cx,cy);
	m_ModeSelector.ModifyStyle(0,WS_CLIPCHILDREN| WS_CLIPSIBLINGS);
	spYStart+=(25+SPACE);
    m_FilterSelector.MoveWindow(2*SPACE,
		                        spYStart,
		                        cx-SPACE-rectNewFilter.Width()-4*SPACE,
								rectNewFilter.Height());
	m_NewFilter.MoveWindow(cx-rectNewFilter.Width()-2*SPACE,
		                   spYStart,
						   rectNewFilter.Width(),
						   rectNewFilter.Height());
	spYStart+=(rectNewFilter.Height()+SPACE);
    m_FilterType.MoveWindow(SPACE,spYStart,cx-2*SPACE,cy-spYStart-SPACE);
	spYStart+=(25+SPACE);
	m_AddRow.GetWindowRect(&rectAddRow);
	m_DelRow.GetWindowRect(&rectDelRow);
	m_TypeSpreadsheet.MoveWindow(2*SPACE,
		                         spYStart,
						         cx-4*SPACE,
						         cy-spYStart-rectAddRow.Height()-2*SPACE);
	m_TextSpreadsheet.MoveWindow(2*SPACE,
		                         spYStart,
						         cx-4*SPACE,
						         cy-spYStart-rectAddRow.Height()-2*SPACE);
	spYStart=cy-rectAddRow.Height()-2*SPACE;
    m_AddRow.MoveWindow(2*SPACE,spYStart,rectAddRow.Width(),rectAddRow.Height());
	m_DelRow.MoveWindow(2*SPACE+rectAddRow.Width()+SPACE,spYStart,rectDelRow.Width(),rectAddRow.Height());
	m_AddRow.Invalidate(TRUE);
	m_DelRow.Invalidate(TRUE);
}
//-----------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// This function will position all controls on the TPM page. It will use the new window 
// sizes passed as "cx" and "cy" to do this job.
//-----------------------------------------------------------------------------------------
void CTpmProcessor::LayoutTpmPage(int cx, int cy) {
CRect rectNewFilter;
CRect rectAddRow;
CRect rectDelRow;
CRect rectEditRow;
CRect rectInclusive;
int spYStart=0;


	m_NewFilter.GetWindowRect(&rectNewFilter);
    m_ModeSelector.MoveWindow(0,spYStart,cx,cy);
	m_ModeSelector.ModifyStyle(0,WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	spYStart+=(25+SPACE);
    m_FilterSelector.MoveWindow(2*SPACE,
		                        spYStart,
		                        cx-SPACE-rectNewFilter.Width()-4*SPACE,
								rectNewFilter.Height());
	m_NewFilter.MoveWindow(cx-rectNewFilter.Width()-2*SPACE,
		                   spYStart,
						   rectNewFilter.Width(),
						   rectNewFilter.Height());
	spYStart+=(rectNewFilter.Height()+SPACE);
	m_AddRow.GetWindowRect(&rectAddRow);
	m_DelRow.GetWindowRect(&rectDelRow);
	m_EditRow.GetWindowRect(&rectEditRow);
	m_Inclusive.GetWindowRect(&rectInclusive);
	m_TpmSpreadsheet.MoveWindow(2*SPACE,
		                         spYStart,
						         cx-4*SPACE,
						         cy-spYStart-rectAddRow.Height()-2*SPACE);
	spYStart=cy-rectAddRow.Height()-2*SPACE;
    m_AddRow.MoveWindow(2*SPACE,spYStart,rectAddRow.Width(),rectAddRow.Height());
	m_DelRow.MoveWindow(2*SPACE+rectAddRow.Width()+SPACE,spYStart,rectDelRow.Width(),rectAddRow.Height());
	m_EditRow.MoveWindow(2*SPACE+rectAddRow.Width()+rectDelRow.Width()+2*SPACE,spYStart,rectEditRow.Width(),rectAddRow.Height());
	m_Inclusive.MoveWindow(2*SPACE+rectAddRow.Width()+rectDelRow.Width()+rectEditRow.Width()+3*SPACE,spYStart,rectInclusive.Width(),rectAddRow.Height());
	m_AddRow.Invalidate(TRUE);
	m_DelRow.Invalidate(TRUE);
	m_EditRow.Invalidate(TRUE);
}
//------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// This callback will run every time the user switches between "Type Matching" and "Text
// Matching" spreadsheets on the Filter page. It will never run if the current page is
// TPM page. We will use this callback to switch between visible/invisible states of the
// Type Matching and Text Matching spreadsheets.
//-----------------------------------------------------------------------------------------
void CTpmProcessor::OnSelchangeFilterType(NMHDR* pNMHDR, LRESULT* pResult)  {
	if(m_ModeSelector.GetCurSel()==0) {
	    switch(m_FilterType.GetCurSel()) {
	        case 0 : m_TextSpreadsheet.ShowWindow(SW_HIDE);
		             m_TypeSpreadsheet.ShowWindow(SW_SHOW);
					 break;
	        case 1 : m_TypeSpreadsheet.ShowWindow(SW_HIDE);
		             m_TextSpreadsheet.ShowWindow(SW_SHOW);
					 break;
		}
	}	
    SetButtonsStatus();
	*pResult = 0;
}
//-----------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// This callback runs every time the user switches between filter page and pattern page.
// We will re-use most controls and the only controls we will turn visible/invisible are
// spreadsheents. We will also re-calculate layout for the controls and we will rename the
// "New" button.
//-----------------------------------------------------------------------------------------
void CTpmProcessor::OnSelchangeMode(NMHDR* pNMHDR, LRESULT* pResult)  {
CRect rect;

    GetClientRect(&rect);
	switch(m_ModeSelector.GetCurSel()) {
	   case 0 :  FillFilterSelector();
		         m_TpmSpreadsheet.ShowWindow(SW_HIDE);
		         LayoutFilterPage(rect.Width(),rect.Height());
		         m_EditRow.ShowWindow(SW_HIDE);
			     m_Inclusive.ShowWindow(SW_HIDE);
 		         switch(m_FilterType.GetCurSel()) {
	                 case 0 : m_TextSpreadsheet.ShowWindow(SW_HIDE);
		                      m_TypeSpreadsheet.ShowWindow(SW_SHOW);
					          break;
	                 case 1 : m_TypeSpreadsheet.ShowWindow(SW_HIDE);
		                      m_TextSpreadsheet.ShowWindow(SW_SHOW);
					          break;
				 }
	             m_FilterType.ShowWindow(SW_SHOW);
				 m_NewFilter.SetWindowText("New Filter");
				 break;
	   case 1 :  FillFilterSelector();
		         m_TypeSpreadsheet.ShowWindow(SW_HIDE);
	             m_TextSpreadsheet.ShowWindow(SW_HIDE);
	             m_FilterType.ShowWindow(SW_HIDE);
				 LayoutTpmPage(rect.Width(),rect.Height());
		         m_TpmSpreadsheet.ShowWindow(SW_SHOW);
				 m_NewFilter.SetWindowText("New Pattern");
		         m_EditRow.ShowWindow(SW_SHOW);
			     m_Inclusive.ShowWindow(SW_SHOW);
				 break;
	}
    SetButtonsStatus();
	*pResult = 0; 
}
//-----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This callback runs every time the user press "New Row" button. The action depends 
// from the current page and, for the first page, from currently selected spreadsheet.
// This will add new row after current one for the text and type matching spreadsheet
// and will open TPM dialog for the TPM spreadsheet and will add TPM spreadsheet row
// with the information from this dialog.
//----------------------------------------------------------------------------------------
void CTpmProcessor::OnNewRow()  {
CString s;
register i;
int saveRow;
CTPMEditor tpm;
int curRow;

	switch(m_ModeSelector.GetCurSel()) {
        // Filter page - we need to determine currently open spreadsheet
	    case 0 :  switch(m_FilterType.GetCurSel()) {
			          // Type matching spreadsheet is current
		              case 0 : m_TypeSpreadsheet.SetEnableDraw(FALSE);
							   curRow=m_TypeSpreadsheet.GetCurrentRow();
                               m_TypeSpreadsheet.AddRow(curRow); 
							   curRow=m_TypeSpreadsheet.GetCurrentRow();
	                           m_TypeSpreadsheet.SetCellType(curRow,2,1);
	                           m_TypeSpreadsheet.SetCellType(curRow,3,1);
	                           m_TypeSpreadsheet.SetCellType(curRow,1,2);
							   saveRow=curRow;
							   for(i=m_TypeSpreadsheet.GetCurrentRow();i<m_TypeSpreadsheet.GetRows();i++) {
								   CString num;
								   num.Format("%ld",i);
	                               m_TypeSpreadsheet.SetCellText(i,0,num);
							   }
							   m_TypeSpreadsheet.SetEnableDraw(TRUE);
							   m_TypeSpreadsheet.SetCurrentRow(saveRow);
						       break;
					  // Text matching spreadsheet is now current
		              case 1 : m_TextSpreadsheet.SetEnableDraw(FALSE);
						       m_TextSpreadsheet.AddRow(m_TextSpreadsheet.GetCurrentRow()); 
	                           m_TextSpreadsheet.SetCellType(m_TextSpreadsheet.GetCurrentRow(),2,1);
	                           m_TextSpreadsheet.SetCellType(m_TextSpreadsheet.GetCurrentRow(),3,2);
	                           m_TextSpreadsheet.SetCellType(m_TextSpreadsheet.GetCurrentRow(),4,2);
	                           m_TextSpreadsheet.SetCellType(m_TextSpreadsheet.GetCurrentRow(),5,2);
						       m_TextSpreadsheet.SetCellType(m_TextSpreadsheet.GetCurrentRow(),1,2);
							   saveRow=m_TextSpreadsheet.GetCurrentRow();
							   for(i=m_TextSpreadsheet.GetCurrentRow();i<m_TextSpreadsheet.GetRows();i++) {
								   CString num;
								   num.Format("%ld",i);
	                               m_TextSpreadsheet.SetCellText(i,0,num);
							   }
							   m_TextSpreadsheet.SetEnableDraw(TRUE);
							   m_TextSpreadsheet.SetCurrentRow(saveRow);
						       break;
				  } 
			      SaveFiltersSpreadsheets();
			      break;
		// TPM page - we will oped dialog to fill new row information.
		case 1: // All edit fields initially empty
			    tpm.m_CurrentTPM="";
			    tpm.m_NewTPM="";
				tpm.m_NameEdit="";
				tpm.m_CommentEdit="";
				// Run the TPM editor dialog
			    if(tpm.DoModal()==IDOK) {
			       m_TpmSpreadsheet.SetEnableDraw(FALSE);
                   m_TpmSpreadsheet.AddRow(m_TpmSpreadsheet.GetCurrentRow());  
				   m_TpmSpreadsheet.SetCellText(m_TpmSpreadsheet.GetCurrentRow(),1,tpm.m_NameEdit);
	               m_TpmSpreadsheet.SetCellText(m_TpmSpreadsheet.GetCurrentRow(),2,tpm.m_CommentEdit);
	               m_TpmSpreadsheet.SetCellText(m_TpmSpreadsheet.GetCurrentRow(),3,tpm.m_CurrentTPM);
				   saveRow=m_TpmSpreadsheet.GetCurrentRow();
				   for(i=m_TpmSpreadsheet.GetCurrentRow();i<m_TpmSpreadsheet.GetRows();i++) {
					   CString num;
					   num.Format("%ld",i);
	                   m_TpmSpreadsheet.SetCellText(i,0,num);
				   }
				   m_TpmSpreadsheet.SetEnableDraw(TRUE);
				   m_TpmSpreadsheet.SetCurrentRow(saveRow);
				   SavePatternSpreadsheet();
				}
				break;
	}
    SetButtonsStatus();
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This callback runs every time the user press "Delete Row" button. The action depends 
// from the current page and, for the first page, from currently selected spreadsheet.
// This will remove current row from the current spreadsheet.
//----------------------------------------------------------------------------------------
void CTpmProcessor::OnDelrow()  {
int saveRow;
register i;
    // Analysing current page
	switch(m_ModeSelector.GetCurSel()) {
		// Page 0 is filter page, we need to determine current spreadsheet.
	    case 0 : switch(m_FilterType.GetCurSel()) {
			          // Spreadsheet 0 is the type matching spreadshhet.
		              case 0 : m_TypeSpreadsheet.SetEnableDraw(FALSE);
						       m_TypeSpreadsheet.RemoveRow(m_TypeSpreadsheet.GetCurrentRow()); 
							   if(m_TypeSpreadsheet.GetCurrentRow()!=0) {
							      saveRow=m_TypeSpreadsheet.GetCurrentRow();
							      for(i=m_TypeSpreadsheet.GetCurrentRow();i<m_TypeSpreadsheet.GetRows();i++) {
								      CString num;
								      num.Format("%ld",i);
	                                  m_TypeSpreadsheet.SetCellText(i,0,num);
								  }
							      m_TypeSpreadsheet.SetCurrentRow(saveRow);
							   }
							   m_TypeSpreadsheet.SetEnableDraw(TRUE);
						       break;
					  // Spreadsheet 1 is the text matching spreadsheet.
		              case 1 : m_TextSpreadsheet.SetEnableDraw(FALSE);
						       m_TextSpreadsheet.RemoveRow(m_TextSpreadsheet.GetCurrentRow()); 
							   if(m_TextSpreadsheet.GetCurrentRow()!=0) {
							      saveRow=m_TextSpreadsheet.GetCurrentRow();
							      for(i=m_TextSpreadsheet.GetCurrentRow();i<m_TextSpreadsheet.GetRows();i++) {
								      CString num;
								      num.Format("%ld",i);
	                                  m_TextSpreadsheet.SetCellText(i,0,num);
								  }
							      m_TextSpreadsheet.SetCurrentRow(saveRow);
							   }
							   m_TextSpreadsheet.SetEnableDraw(TRUE);
						       break;
				  } 
			      break;
		// Page 1 is TPM filter with only one spreadsheet
		case 1:  m_TpmSpreadsheet.SetEnableDraw(FALSE);
			     m_TpmSpreadsheet.RemoveRow(m_TpmSpreadsheet.GetCurrentRow()); 
				 if(m_TpmSpreadsheet.GetCurrentRow()!=0) {
				    saveRow=m_TpmSpreadsheet.GetCurrentRow();
				    for(i=m_TpmSpreadsheet.GetCurrentRow();i<m_TpmSpreadsheet.GetRows();i++) {
					   CString num;
					   num.Format("%ld",i);
	                   m_TpmSpreadsheet.SetCellText(i,0,num);
					}
				    m_TpmSpreadsheet.SetCurrentRow(saveRow);
				 }
				 m_TpmSpreadsheet.SetEnableDraw(TRUE);
				 break;
	}
	switch(m_ModeSelector.GetCurSel()) {
	   case 0 : SaveFiltersSpreadsheets(); break;
	   case 1 : SavePatternSpreadsheet();  break;
	}
    SetButtonsStatus();
}


//----------------------------------------------------------------------------------------
// Callback runs every time the user press "New Filter" button. We will open dialog prompt
// for the new filter name.
//----------------------------------------------------------------------------------------
void CTpmProcessor::OnNewFilter() {
register i;
CNewFilter dialog;
CString query;
CString result;
int index;
CString* fullName;
int pos;
BSTR sysQuery;

    // Save all changes in the old lists
	switch(m_ModeSelector.GetCurSel()) {
	   case 0 : SaveFiltersSpreadsheets(); break;
	   case 1 : SavePatternSpreadsheet();  break;
	}
    if(dialog.DoModal()==IDOK) {
	    switch(m_ModeSelector.GetCurSel()) {
		// Current page is filter page
	    case 0 : // Maybe the filter with this name is already in the list?
		         for(i=0;i<m_FilterSelector.GetCount();i++) {
			         CString existFilter;
					 m_FilterSelector.GetLBText(i,existFilter);
					 if(existFilter==dialog.m_Name) {
						 MessageBox("The filter with this name is already exists.","Filter Creation Error",MB_ICONEXCLAMATION);
						 return;
					 }
				 }
		         // The name is new one. we will create the filter.
		         query.Format("set __flist [extfile new filter \"%s\"]",dialog.m_Name);
		         sysQuery=query.AllocSysString();
		         DataSourceAccessSync(&sysQuery);
		         ::SysFreeString(sysQuery);
		         // Inserting name into the combobox
		         index=m_FilterSelector.InsertString(-1,dialog.m_Name);
		         // We need it's complete path
		         query="fname $__flist";
		         sysQuery=query.AllocSysString();
		         result=DataSourceAccessSync(&sysQuery);
		         ::SysFreeString(sysQuery);
		
		         fullName=new CString;
		         pos=0;
		         while(result[pos]==' ' || result[pos]=='/' || result[pos]=='\\') pos++;
		         *fullName=result.Mid(pos);
		         m_FilterSelector.SetItemData(index,(DWORD)fullName);
		         m_FilterSelector.SetCurSel(index);
				 // Save current filter name and path
                 m_FilterName=dialog.m_Name;
			     m_FilterPath=*fullName;
				 m_TypeSpreadsheet.Clear();
				 m_TextSpreadsheet.Clear();
				 SaveFiltersSpreadsheets();
				 // Fill filters spreadsheets
		         FillFiltersSpreadsheets();
		         break;
	   // Current page is pattern page
	   case 1:   // Maybe the filter with this name is already in the list?
		         for(i=0;i<m_FilterSelector.GetCount();i++) {
					 CString existFilter;
					 m_FilterSelector.GetLBText(i,existFilter);
					 if(existFilter==dialog.m_Name) {
						 MessageBox("The pattern with this name is already exists.","Filter Creation Error",MB_ICONEXCLAMATION);
						 return;
					 }
				 }
				 // The name is new one. we will create the filter.
				 query.Format("set __flist [extfile new pattern \"%s\"]",dialog.m_Name);
				 sysQuery=query.AllocSysString();
				 DataSourceAccessSync(&sysQuery);
				 ::SysFreeString(sysQuery);
				 // Inserting name into the combobox
				 index=m_FilterSelector.InsertString(-1,dialog.m_Name);
				 // We need it's complete path
				 query="fname $__flist";
				 sysQuery=query.AllocSysString();
				 result=DataSourceAccessSync(&sysQuery);
				 ::SysFreeString(sysQuery);
				 fullName=new CString;
		         pos=0;
		         while(result[pos]==' ' || result[pos]=='/' || result[pos]=='\\') pos++;
		         *fullName=result.Mid(pos);
		         m_FilterSelector.SetItemData(index,(DWORD)fullName);
		         m_FilterSelector.SetCurSel(index);
				 // Save current pattern name and path
                 m_PatternName=dialog.m_Name;
			     m_PatternPath=*fullName;
				 m_TpmSpreadsheet.Clear();
				 SavePatternSpreadsheet();
				 // Re-fill pattern spreadsheet
		         FillPatternSpreadsheet();
		         break;
		}
	}
    SetButtonsStatus();
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This function will query server for a list of the existing filters or TPM patterns and
// fill the filter selector combobox.
//----------------------------------------------------------------------------------------
void CTpmProcessor::FillFilterSelector() {
register i;
CString query;
CString result;
BSTR sysQuery;
int index;
int size;
CString* fullName;
int pos,pos1,len;

   // Clear all combobox
   for(i=0;i<m_FilterSelector.GetCount();i++) {
	   fullName=(CString *)m_FilterSelector.GetItemData(i);
	   delete fullName;
   }
   m_FilterSelector.ResetContent();

   switch(m_ModeSelector.GetCurSel()) {
	   // If current page is filter page
       case 0 : // Query server for the available filters
                query="set __flist [extfile list filter]";
                sysQuery=query.AllocSysString();
                DataSourceAccessSync(&sysQuery);
                ::SysFreeString(sysQuery);
                // We need to know quantity of the available filters
                query="size $__flist";
                sysQuery=query.AllocSysString();
                result=DataSourceAccessSync(&sysQuery);
                ::SysFreeString(sysQuery);
                size=atoi(result);

                // Default filter is always available in $PSETHOME/lib/default.flt
                query="set tmp $env(PSETHOME)";
                sysQuery=query.AllocSysString();
                result=DataSourceAccessSync(&sysQuery);
                ::SysFreeString(sysQuery);
                result+="/lib/default.flt";

                // Filling the combobox with the default filter, attaching it's full name as an
                // item data
                pos = result.ReverseFind('/')+1;
                len=0;
                while(result[pos+len]!=0 && result[pos+len]!='.') len++;
                index=m_FilterSelector.InsertString(-1,result.Mid(pos,len));
                fullName=new CString;
                *fullName=result;
                m_FilterSelector.SetItemData(index,(DWORD)fullName);

                // Filling the combobox with all available filters, attaching their full names as an
                // items data
                for(i=0;i<size;i++) {
                    query.Format("fname $__flist %d",i+1);
                    sysQuery=query.AllocSysString();
                    result=DataSourceAccessSync(&sysQuery);
                    ::SysFreeString(sysQuery);
	                pos = result.ReverseFind('\\')+1;
	                pos1 = result.ReverseFind('/')+1;
					if(pos1>pos) pos=pos1;
                    len=0;
	                while(result[pos+len]!=0 && result[pos+len]!='.') len++;
	                index=m_FilterSelector.InsertString(-1,result.Mid(pos,len));

                    fullName=new CString;
	                pos=0;
	                while(result[pos]==' ' || result[pos]=='/' || result[pos]=='\\') pos++;
                    *fullName=result.Mid(pos);
                    m_FilterSelector.SetItemData(index,(DWORD)fullName);
				}
                // First filter will be selected
                m_FilterSelector.SetCurSel(0);
                m_FilterSelector.GetLBText(0,m_FilterName);
			    m_FilterPath=*((CString *)m_FilterSelector.GetItemData(0));
                FillFiltersSpreadsheets();
				break;
	   // If current page is TPM page
       case 1 : // Query server for the available filters
                query="set __flist [extfile list pattern]";
                sysQuery=query.AllocSysString();
                DataSourceAccessSync(&sysQuery);
                ::SysFreeString(sysQuery);
                // We need to know quantity of the available filters
                query="size $__flist";
                sysQuery=query.AllocSysString();
                result=DataSourceAccessSync(&sysQuery);
                ::SysFreeString(sysQuery);
                size=atoi(result);

                // Default filter is always available in $PSETHOME/lib/default.flt
                query="set tmp $env(PSETHOME)";
                sysQuery=query.AllocSysString();
                result=DataSourceAccessSync(&sysQuery);
                ::SysFreeString(sysQuery);
                result+="/lib/default.ptn";

                // Filling the combobox with the default filter, attaching it's full name as an
                // item data
                pos = result.ReverseFind('\\')+1;
	            pos1 = result.ReverseFind('/')+1;
			    if(pos1>pos) pos=pos1;
                len=0;
                while(result[pos+len]!=0 && result[pos+len]!='.') len++;
                index=m_FilterSelector.InsertString(-1,result.Mid(pos,len));
                fullName=new CString;
                *fullName=result;
                m_FilterSelector.SetItemData(index,(DWORD)fullName);

                // Filling the combobox with all available filters, attaching their full names as an
                // items data
                for(i=0;i<size;i++) {
                    query.Format("fname $__flist %d",i+1);
                    sysQuery=query.AllocSysString();
                    result=DataSourceAccessSync(&sysQuery);
                    ::SysFreeString(sysQuery);
	                pos = result.ReverseFind('\\')+1;
	                pos1 = result.ReverseFind('/')+1;
			        if(pos1>pos) pos=pos1;
                    len=0;
	                while(result[pos+len]!=0 && result[pos+len]!='.') len++;
	                index=m_FilterSelector.InsertString(-1,result.Mid(pos,len));

                    fullName=new CString;
	                pos=0;
	                while(result[pos]==' ' || result[pos]=='/' || result[pos]=='\\') pos++;
                    *fullName=result.Mid(pos);
                    m_FilterSelector.SetItemData(index,(DWORD)fullName);
				}
                // First filter will be selected
                m_FilterSelector.SetCurSel(0);
                m_FilterSelector.GetLBText(0,m_PatternName);
			    m_PatternPath=*((CString *)m_FilterSelector.GetItemData(0));
                FillPatternSpreadsheet();
				break;
		}
	   SetButtonsStatus();
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// We need to free memory allocated for the full filter files names attached to the 
// combobox items when closing the window.
//----------------------------------------------------------------------------------------
void CTpmProcessor::OnClose()  {
register i;
CString* fullName;
   for(i=0;i<m_FilterSelector.GetCount();i++) {
	   fullName=(CString *)m_FilterSelector.GetItemData(i);
	   if(fullName!=NULL) delete fullName;
   }
   // We need to save all changes
   switch(m_ModeSelector.GetCurSel()) {
      case 0 : SaveFiltersSpreadsheets(); break;
	  case 1 : SavePatternSpreadsheet();  break;
   }
   CFormView::OnClose();
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This function will fill the text matching and type matching spreadsheets reading data
// from the filter file currently selected in the filter selector combobox.
//----------------------------------------------------------------------------------------
void CTpmProcessor::FillFiltersSpreadsheets() {
register i,j;
CString *fileName;
CFile   f;
int     types;
int     texts;
int     curToken;
CString tokens[10];
int     brCount;
CString curString;
int     include;
CFileException e;


TRY {
   m_TypeSpreadsheet.Clear();
   m_TextSpreadsheet.Clear();
   fileName=(CString *)m_FilterSelector.GetItemData(m_FilterSelector.GetCurSel());
   if( !f.Open( *fileName, CFile::modeRead, &e) ) {
	   CString filter;
	   filter.Format("Unable to open filter:\n %s\n Error code is : %d",*fileName, e.m_cause);
	   AfxMessageBox(filter);
   } else {
       CArchive ar(&f,CArchive::load);
	   ar.ReadString(curString);
	   if(curString!="@@FLT01") {
	        AfxMessageBox("Wrong filter file format.");
			return;
	   }
	   ar.ReadString(curString);
	   types=atoi(curString);
	   ar.ReadString(curString);
	   texts=atoi(curString);
	   // reading the type matching spreadsheet
	   m_TypeSpreadsheet.SetRows(types+1);
       m_TypeSpreadsheet.SetEnableDraw(FALSE);
	   for(i=0;i<types;i++) {
	       ar.ReadString(curString);
		   brCount=0;
		   curToken=0;
		   tokens[curToken]="";
		   for(j=0;j<curString.GetLength();j++) {
			   if(curString[j]=='{' && brCount==0) {
				   brCount++;
				   continue;
			   }
			   if(curString[j]=='}' && brCount==1) {
				   brCount--;
				   continue;
			   }
			   if(curString[j]==' ' && brCount==0) {
				   curToken++;
				   tokens[curToken]="";
				   continue;
			   }
			   tokens[curToken]+=curString[j];
		   }
		   include=atoi(tokens[0]);

	       m_TypeSpreadsheet.SetCellType(i+1,1,2);
	       m_TypeSpreadsheet.SetCellType(i+1,2,1);
	       m_TypeSpreadsheet.SetCellType(i+1,3,1);


           CString num;
		   num.Format("%ld",i+1);
           m_TypeSpreadsheet.SetCellText(i+1,0,num);

		   m_TypeSpreadsheet.SetCellCheck(i+1,1,include!=0);
		   m_TypeSpreadsheet.SetCellText(i+1,2,tokens[1]);
		   m_TypeSpreadsheet.SetCellText(i+1,3,tokens[2]);

	   }
       m_TypeSpreadsheet.SetEnableDraw(TRUE);
	   m_TypeSpreadsheet.SetCurrentColumn(1);
	   m_TypeSpreadsheet.SetCurrentRow(1);

	   // reading the text matching spreadsheet
	   m_TextSpreadsheet.SetRows(texts+1);
       m_TextSpreadsheet.SetEnableDraw(FALSE);
	   for(i=0;i<texts;i++) {
	       ar.ReadString(curString);
		   brCount=0;
		   curToken=0;
		   tokens[curToken]="";
		   for(j=0;j<curString.GetLength();j++) {
			   if(curString[j]=='{' && brCount==0) {
				   brCount++;
				   continue;
			   }
			   if(curString[j]=='}' && brCount==1) {
				   brCount--;
				   continue;
			   }
			   if(curString[j]==' ' && brCount==0) {
				   curToken++;
				   tokens[curToken]="";
				   continue;
			   }
			   tokens[curToken]+=curString[j];
		   }
	       m_TextSpreadsheet.SetCellType(i+1,1,2);
	       m_TextSpreadsheet.SetCellType(i+1,2,1);
	       m_TextSpreadsheet.SetCellType(i+1,3,2);
	       m_TextSpreadsheet.SetCellType(i+1,4,2);
	       m_TextSpreadsheet.SetCellType(i+1,5,2);

           CString num;
		   num.Format("%ld",i+1);
           m_TextSpreadsheet.SetCellText(i+1,0,num);

		   include=atoi(tokens[0]);
		   m_TextSpreadsheet.SetCellCheck(i+1,1,include!=0);

		   m_TextSpreadsheet.SetCellText(i+1,2,tokens[1]);

		   include=atoi(tokens[2]);
		   m_TextSpreadsheet.SetCellCheck(i+1,3,include!=0);

		   include=atoi(tokens[3]);
		   m_TextSpreadsheet.SetCellCheck(i+1,4,include!=0);

		   include=atoi(tokens[4]);
		   m_TextSpreadsheet.SetCellCheck(i+1,5,include!=0);
	   }
       m_TextSpreadsheet.SetEnableDraw(TRUE);
	   m_TextSpreadsheet.SetCurrentColumn(1);
	   m_TextSpreadsheet.SetCurrentRow(1);
	   ar.Close();
	   f.Close();
   }
}
CATCH(CArchiveException,ae) {
	return;
}
END_CATCH
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This function will fill the pattern  spreadsheet reading data
// from the pattern file currently selected in the filter selector combobox.
//----------------------------------------------------------------------------------------
void CTpmProcessor::FillPatternSpreadsheet() {
register i,j;
CString *fileName;
CFile	f;
int 	curToken;
CString tokens[10];
int 	brCount;
CString curString;

TRY {
	m_TpmSpreadsheet.Clear();
	fileName=(CString *)m_FilterSelector.GetItemData(m_FilterSelector.GetCurSel());
	if( !f.Open( *fileName, CFile::modeRead) ) {
		CString filter;
		filter.Format("Unable to open TPM file:\n %s",*fileName);
		AfxMessageBox(filter);
	} else {
		CArchive ar(&f,CArchive::load);
		if(ar.ReadString(curString)==FALSE) {
		   CString filter;
		   filter.Format("Unexpected end of file:\n %s",*fileName);
		   AfxMessageBox(filter);
		}
		if(curString=="InclusiveBtn") m_Inclusive.SetCheck(1);
		else m_Inclusive.SetCheck(0);
		i=0;
        m_TpmSpreadsheet.SetEnableDraw(FALSE);
		while(1) {
			if(ar.ReadString(curString)==FALSE) break;
			brCount=0;
			curToken=0;
			tokens[curToken]="";
			for(j=0;j<curString.GetLength();j++) {
				if(curString[j]=='{' && brCount==0) {
					brCount++;
					continue;
				}
				if(curString[j]=='}' && brCount==1) {
					brCount--;
					continue;
				}
				if(curString[j]==' ' && brCount==0) {
					curToken++;
					tokens[curToken]="";
					continue;
				}
				tokens[curToken]+=curString[j];
			}
			int rows=m_TpmSpreadsheet.GetRows();
			m_TpmSpreadsheet.SetRows(rows+1);
			
			CString num;
			num.Format("%ld",i+1);
			m_TpmSpreadsheet.SetCellText(i+1,0,num);
			m_TpmSpreadsheet.SetCellText(i+1,1,tokens[0]);
			m_TpmSpreadsheet.SetCellText(i+1,2,tokens[1]);
			m_TpmSpreadsheet.SetCellText(i+1,3,tokens[2]);
			i++;
			//if(i>7) break;
		}
        m_TpmSpreadsheet.SetEnableDraw(TRUE);
		m_TpmSpreadsheet.SetCurrentColumn(1);
		m_TpmSpreadsheet.SetCurrentRow(1);
		ar.Close();
		f.Close();
	}
}
CATCH(CArchiveException,ae) {
	return;
}
END_CATCH
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// This callback will run every time the combobox selection changed (new filter selected)
// The action depends on the current page, while in filter page we will select new filter,
// and while on TPM page we will select new pattern.
//----------------------------------------------------------------------------------------
void CTpmProcessor::OnSelchangeFilter()  {
int index;

    // saving old spreadsheets
    switch(m_ModeSelector.GetCurSel()) {
       case 0 : SaveFiltersSpreadsheets(); break;
 	   case 1 : SavePatternSpreadsheet();  break;
    }
     
    index=m_FilterSelector.GetCurSel();
	// Fill with new values
	switch(m_ModeSelector.GetCurSel()) {
		// Page 0 is filter page.
	    case 0 :  m_FilterSelector.GetLBText(index,m_FilterName);
			      m_FilterPath=*((CString *)m_FilterSelector.GetItemData(index));
			      FillFiltersSpreadsheets(); 
				  break;
		// Page 1 is a TPM page
		case 1 :  m_FilterSelector.GetLBText(index,m_PatternName);
			      m_PatternPath=*((CString *)m_FilterSelector.GetItemData(index));
                  FillPatternSpreadsheet(); 
				  break;
	}
	SetButtonsStatus();
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This function will examine all filters spreadsheets (type matching and text matching)
// and will save them in the file given by the server.
//----------------------------------------------------------------------------------------
void CTpmProcessor::SaveFiltersSpreadsheets() {
register i;
CFile f;
int types;
int texts;
CString curString;

   if(m_FilterName=="default") return;
   if( !f.Open( m_FilterPath,  CFile::modeCreate | CFile::modeWrite) ) {
	   CString filter;
	   filter.Format("Unable to open filter file:\n %s",m_FilterPath);
	   AfxMessageBox(filter);
   } else {
       CArchive ar(&f,CArchive::store);
	   // Writing file ID
	   curString="@@FLT01\n";
	   ar.WriteString(curString);
	   // Writing amount of elements in the spreadsheets
	   types=m_TypeSpreadsheet.GetRows()-1;
	   texts=m_TextSpreadsheet.GetRows()-1;
	   curString.Format("%d\n",types);
	   ar.WriteString(curString);
	   curString.Format("%d\n",texts);
	   ar.WriteString(curString);
	   // Writing type matching spreadsheet
	   for(i=0;i<types;i++) {
		   if(m_TypeSpreadsheet.GetCellCheck(i+1,1)==TRUE) curString="1";
		   else                                            curString="0";
		   curString+=" {";
		   curString+=m_TypeSpreadsheet.GetCellText(i+1,2);
		   curString+="} {";
		   curString+=m_TypeSpreadsheet.GetCellText(i+1,3);
		   curString+="}";
		   curString+="\n";
	       ar.WriteString(curString);
	   }
	   // Writing text matching spreadsheet
	   for(i=0;i<texts;i++) {
		   if(m_TextSpreadsheet.GetCellCheck(i+1,1)==TRUE) curString="1";
		   else                                            curString="0";
		   curString+=" {";
		   curString+=m_TextSpreadsheet.GetCellText(i+1,2);
		   curString+="} ";
		   if(m_TextSpreadsheet.GetCellCheck(i+1,3)==TRUE) curString+="1 ";
		   else                                            curString+="0 ";
		   if(m_TextSpreadsheet.GetCellCheck(i+1,4)==TRUE) curString+="1 ";
		   else                                            curString+="0 ";
		   if(m_TextSpreadsheet.GetCellCheck(i+1,5)==TRUE) curString+="1";
		   else                                            curString+="0";
		   curString+="\n";
	       ar.WriteString(curString);
	   }
	   ar.Close();
       f.Close();
   }

}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This callback will run every time the user clicked "Edit Row" button on the TPM pattern
// page. This button is only available on the TPM pattern page, so there is no need to 
// check currently active page.
//----------------------------------------------------------------------------------------
void CTpmProcessor::OnEditRow()  {
CTPMEditor tpm;

	tpm.m_NameEdit    = m_TpmSpreadsheet.GetCellText(m_TpmSpreadsheet.GetCurrentRow(),1);
	tpm.m_CommentEdit = m_TpmSpreadsheet.GetCellText(m_TpmSpreadsheet.GetCurrentRow(),2);
	tpm.m_NewTPM      = m_TpmSpreadsheet.GetCellText(m_TpmSpreadsheet.GetCurrentRow(),3);
	tpm.m_CurrentTPM  = m_TpmSpreadsheet.GetCellText(m_TpmSpreadsheet.GetCurrentRow(),3);
	// Run the TPM editor dialog
	if(tpm.DoModal()==IDOK) {
		m_TpmSpreadsheet.SetEnableDraw(FALSE);
		m_TpmSpreadsheet.SetCellText(m_TpmSpreadsheet.GetCurrentRow(),1,tpm.m_NameEdit);
	    m_TpmSpreadsheet.SetCellText(m_TpmSpreadsheet.GetCurrentRow(),2,tpm.m_CommentEdit);
	    m_TpmSpreadsheet.SetCellText(m_TpmSpreadsheet.GetCurrentRow(),3,tpm.m_CurrentTPM);
		m_TpmSpreadsheet.SetEnableDraw(TRUE);
	    SavePatternSpreadsheet();
	}
	
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This private function will check the amount of rows in the currently selected 
// spreadsheet and if 0, will disable "Delete Row" and "Edit Row" buttons.
//----------------------------------------------------------------------------------------
void CTpmProcessor::SetButtonsStatus() {
	switch(m_ModeSelector.GetCurSel()) {
		// Filter page
	    case 0 : switch(m_FilterType.GetCurSel()) {
			         // Type matching spreadsheet
	                 case 0 : if(m_TypeSpreadsheet.GetRows()>1) m_DelRow.EnableWindow(TRUE);
						      else                              m_DelRow.EnableWindow(FALSE);
					          break;
			         // Text matching spreadsheet
	                 case 1 : if(m_TextSpreadsheet.GetRows()>1) m_DelRow.EnableWindow(TRUE);
						      else                              m_DelRow.EnableWindow(FALSE);
					          break;
				 }
			     break;
		// TPM patterns page
		case 1: if(m_TpmSpreadsheet.GetRows()>1) {
			         m_DelRow.EnableWindow(TRUE);
					 m_EditRow.EnableWindow(TRUE);
				} else {
			         m_DelRow.EnableWindow(FALSE);
					 m_EditRow.EnableWindow(FALSE);
				}
			    break;

	}	
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This function will examine TPM pattern spreadsheet and will save it in the file.
//----------------------------------------------------------------------------------------
void CTpmProcessor::SavePatternSpreadsheet() {
register i;
CFile f;
int elements;
CString curString;

   if(m_PatternName=="default") return;
   if( !f.Open( m_PatternPath, CFile::modeCreate | CFile::modeWrite) ) {
	   CString filter;
	   filter.Format("Unable to write TPM pattern file:\n %s",m_PatternPath);
	   AfxMessageBox(filter);
   } else {
       CArchive ar(&f,CArchive::store);

	   // Writing inclusive flag
	   if(m_Inclusive.GetCheck()==1) curString = "InclusiveBtn\n";
       else                          curString = "ExclusiveBtn\n";
	   ar.WriteString(curString);

	   // Writing TPM spreadsheet
	   elements=m_TpmSpreadsheet.GetRows()-1;
	   for(i=0;i<elements;i++) {
		   curString="{";
		   curString+=m_TpmSpreadsheet.GetCellText(i+1,1);
		   curString+="} {";
		   curString+=m_TpmSpreadsheet.GetCellText(i+1,2);
		   curString+="} {";
		   curString+=m_TpmSpreadsheet.GetCellText(i+1,3);
		   curString+="}";
		   curString+="\n";
	       ar.WriteString(curString);
	   }
	   ar.Close();
       f.Close();
   }
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// We need to save all changes when changing current mode
//----------------------------------------------------------------------------------------
void CTpmProcessor::OnSelchangingMode(NMHDR* pNMHDR, LRESULT* pResult)  {
	switch(m_ModeSelector.GetCurFocus()) {
	   case 0 : SaveFiltersSpreadsheets(); break;
	   case 1 : SavePatternSpreadsheet();  break;
	}
	*pResult = 0;
}
//----------------------------------------------------------------------------------------

void CTpmProcessor::Update() {
	FillFilterSelector();

}


BEGIN_EVENTSINK_MAP(CTpmProcessor, CFormView)
    //{{AFX_EVENTSINK_MAP(CTpmProcessor)
	ON_EVENT(CTpmProcessor, IDC_TEXTSPREADSHEET, 1 /* BoolChanged */, OnBoolChanged, VTS_I4 VTS_I4 VTS_BSTR)
	ON_EVENT(CTpmProcessor, IDC_TEXTSPREADSHEET, 2 /* StringChanged */, OnStringChanged, VTS_I4 VTS_I4 VTS_BSTR)
	ON_EVENT(CTpmProcessor, IDC_TYPESPREADSHEET, 1 /* BoolChanged */, OnBoolChangedTypespreadsheet, VTS_I4 VTS_I4 VTS_BSTR)
	ON_EVENT(CTpmProcessor, IDC_TYPESPREADSHEET, 2 /* StringChanged */, OnStringChangedTypespreadsheet, VTS_I4 VTS_I4 VTS_BSTR)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CTpmProcessor::OnBoolChanged(long row, long col, LPCTSTR val)  {
SaveFiltersSpreadsheets();	
}

void CTpmProcessor::OnStringChanged(long row, long col, LPCTSTR val)  {
SaveFiltersSpreadsheets();	
}

void CTpmProcessor::OnBoolChangedTypespreadsheet(long row, long col, LPCTSTR val)  {
SaveFiltersSpreadsheets();	
}

void CTpmProcessor::OnStringChangedTypespreadsheet(long row, long col, LPCTSTR val)  {
SaveFiltersSpreadsheets();	
}

//-------------------------------------------------------------------------------------------------------
// This function check communications status after access datasource.
//------------------------------------------------------------------------------------------
CString CTpmProcessor::DataSourceAccessSync(BSTR* command) {
	CString res;
	static BOOL bCommDlgShown = FALSE;
	res = m_DataSource.AccessSync(command);
	if (m_DataSource.IsConnectionLost() == TRUE &&
		bCommDlgShown != TRUE ) {
		::MessageBox(m_hWnd, 
			   _T("Connection with server is lost."
			   " Make sure the server is running."),
			   _T("Server Communication Error."), 
			   MB_OK | MB_ICONINFORMATION); 
		bCommDlgShown = TRUE;
	} else if (m_DataSource.IsConnectionLost() != TRUE) {
		bCommDlgShown = FALSE;
	}	
	return res;
}
