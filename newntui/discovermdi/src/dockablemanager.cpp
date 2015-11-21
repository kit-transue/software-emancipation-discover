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
// DockableManager.cpp : implementation file
//

#include "stdafx.h"
#include "afxpriv.h"
#include "discovermdi.h"
#include "DockableManager.h"
#include "MSDEVIntegrator.h"
#include "MainFrm.h"
#include "PromptDialog.h"
#include "QuestionDlg.h"

#define HEADER 10
#define BORDER 5
#define SASH_SPACE 8
#define SPLITTER_SPACE 10
#define SELECTOR_HEIGHT 26

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int  browserSetType=0;
CString browserVarName;
static BOOL processing = FALSE;

#define IDC_SETBROWSER 29874

/////////////////////////////////////////////////////////////////////////////
// CDockableManager

CDockableManager::CDockableManager() {
	AfxEnableControlContainer();
	m_DockSide=-1;
	m_OnSize=FALSE;
//	m_cxLeftBorder=m_cxRightBorder=m_cyTopBorder=m_cyBottomBorder=0;
}

CDockableManager::~CDockableManager()
{
}


BEGIN_MESSAGE_MAP(CDockableManager, CDialogBar)
	//{{AFX_MSG_MAP(CDockableManager)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_MESSAGE(WM_SASHMOVED,OnSashMoved)
	ON_MESSAGE(WM_SPLITMOVE,OnSplitMoved)
	ON_COMMAND(IDC_SERVICECHANGED,OnServiceChanged)
	ON_COMMAND(IDC_SETBROWSER,SetBrowserVariable)
	ON_WM_PAINT()
	ON_NOTIFY(TCN_SELCHANGE, IDC_BROWSERSELECTOR, OnModeChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CDockableManager, CDialogBar)
    //{{AFX_EVENTSINK_MAP(CDockableManager)
	ON_EVENT(CDockableManager, IDC_TREE,     1 /*NodeChanged*/,          OnNodeChanged,           VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_MODULES,  1 /*ProjectChanged*/,       OnProjectChanged,        VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_MODULES,  4 /*ElementDoubleclicked*/, OnFileDoubleclicked,     VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_RIGHTPANE,2 /*CategoryChanged*/,      OnRightCategoryChanged,  VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_RIGHTPANE,4 /*SelectionChanged*/,     OnRightSelectionChanged, VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_RIGHTPANE,5 /*GroupChanged*/,         OnRightGroupChanged,     VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_RIGHTPANE,6 /*Elementdoubleclicked*/, OnElementdoubleclicked,  VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_LEFTPANE, 2 /*CategoryChanged*/,      OnLeftCategoryChanged,   VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_LEFTPANE, 4 /*SelectionChanged*/,     OnLeftSelectionChanged,  VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_LEFTPANE, 5 /*GroupChanged*/,         OnLeftGroupChanged,      VTS_BSTR)
    ON_EVENT(CDockableManager, IDC_LEFTPANE, 6 /*Elementdubleclicked*/,  OnElementdoubleclicked,  VTS_BSTR)
	ON_EVENT(CDockableManager, IDC_MSDEVINTEGRATOR,1 /* IntegratorCtrlClick */, IntegratorCtrlClick,  VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDockableManager, IDC_MSDEVINTEGRATOR,2 /* IntegratorQuery */,     IntegratorQuery,      VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDockableManager, IDC_MSDEVINTEGRATOR,3 /* IntegratorDefinition */,IntegratorDefinition, VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDockableManager, IDC_MSDEVINTEGRATOR,4 /* IntegratorActivate */,  IntegratorActivate,   VTS_NONE)
	ON_EVENT(CDockableManager, IDC_DATASOURCE,1 /* ServerNotification */,       OnServerNotification,   VTS_BSTR)
//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP() 

/////////////////////////////////////////////////////////////////////////////
// CDockableManager message handlers


//----------------------------------------------------------------------------
// This callback will run every time the user selected different tab 
// on the mode tab control. We need to make one project browser window visible
// and the other one - invisible. We will fill the project list here in the 
// case the project node changed after the last switch.
//----------------------------------------------------------------------------
void CDockableManager::OnModeChanged(NMHDR* pNMHDR, LRESULT* pResult)  {

	m_Selector   = (CTabCtrl *)        GetDlgItem(IDC_BROWSERSELECTOR);
	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);
	m_Tree       = (CProjectTree *)    GetDlgItem(IDC_TREE);
	m_Modules    = (CProjectModules *) GetDlgItem(IDC_MODULES);
	m_LeftPanel  = (CCBrowserFrame *) GetDlgItem(IDC_LEFTPANE);
	m_RightPanel = (CCBrowserFrame*) GetDlgItem(IDC_RIGHTPANE);
	switch(m_Selector->GetCurSel()) {
	   // The user selected tree presentation
	   case 0 : m_LeftPanel ->ShowWindow(SW_HIDE); 
		        m_RightPanel->ShowWindow(SW_HIDE); 
				m_Modules->ShowWindow(SW_SHOW); 
		        m_Tree->ShowWindow(SW_SHOW); 
		        break;
	   case 1 : m_Modules->ShowWindow(SW_HIDE); 
		        m_Tree->ShowWindow(SW_HIDE); 
				m_LeftPanel ->ShowWindow(SW_SHOW); 
		        m_RightPanel->ShowWindow(SW_SHOW); 
		        break;
	}
	// let the tab itself process change notifications
	*pResult = 0;
}
//----------------------------------------------------------------------------




void CDockableManager::OnSize(UINT nType, int cx, int cy) {
	 LayoutControls();
}


int CDockableManager::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_RightSash.Create("", WS_CHILD | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,10000);
	m_RightSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);
	m_LeftSash.Create("", WS_CHILD | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,10001);
	m_LeftSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);
	m_TopSash.Create("", WS_CHILD | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,10002);
	m_TopSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);
	m_BottomSash.Create("", WS_CHILD  | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,10003);
	m_BottomSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);

	m_DividerSash.Create("", WS_CHILD  | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,10004);
	m_DividerSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);
	return 0;
}

//---------------------------------------------------------------------------------
// Callback runs every time  sash changed it's position.
// We need to resize our dockable window, position all controls in it and update
// our sash position.
//---------------------------------------------------------------------------------
LRESULT CDockableManager::OnSashMoved(WPARAM wparam, LPARAM lparam) {
CRect winRect;
CRect sashRect;
int x = wparam;
int y= lparam;

    // Calculating new window sizes from the nes sash position
    GetWindowRect(&winRect);
    switch(GetDockSide()) {
	    case DOCK_LEFT : m_RightSash.GetWindowRect(&sashRect);
			             m_sizeDefault.cx=x+(sashRect.left-winRect.left)+6;
			             break;
	    case DOCK_RIGHT: m_LeftSash.GetWindowRect(&sashRect);
			             m_sizeDefault.cx=(winRect.right-sashRect.right)-x+6;
			             break;
	    case DOCK_TOP :  m_BottomSash.GetWindowRect(&sashRect);
			             m_sizeDefault.cy=y+(sashRect.top-winRect.top)+6;
			             break;
	    case DOCK_BOTTOM:m_TopSash.GetWindowRect(&sashRect);
			             m_sizeDefault.cy=(winRect.bottom-sashRect.bottom)-y+6;
			             break;
	}
	if(m_SplitterX>=m_sizeDefault.cx-15) {
		m_SplitterX=m_sizeDefault.cx-15;
        SetSplitteredWindowsPos();
	}

    // Setting new position for the sash bar
	PositionSashBars();
	// Re-calculating frame window dockables layout.
    GetDockingFrame()->RecalcLayout(); 
	Invalidate();
	return TRUE;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// Callback runs every time  sash changed it's position.
// We need to resize our dockable window, position all controls in it and update
// our sash position.
//---------------------------------------------------------------------------------
LRESULT CDockableManager::OnSplitMoved(WPARAM wparam, LPARAM lparam) {
CRect winRect;
CRect sashRect;

    GetWindowRect(&winRect);
	m_DividerSash.GetWindowRect(&sashRect);

    m_SplitterX=(int)wparam+(sashRect.left-winRect.left);


    SetSplitteredWindowsPos();
    return TRUE;
}
//---------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// This function returns the current side of the frame window to which this 
// dockable bar docked. The values are DOCK_LEFT, DOCK_RIGHT, DOCK_TOP, DOCK_BOTTOM.
//--------------------------------------------------------------------------------
int CDockableManager::GetDockSide() {
	if(IsFloating()) return DOCK_FLOAT;
    if(m_pDockBar==GetDockingFrame()->GetControlBar(AFX_IDW_DOCKBAR_TOP) )    
		return DOCK_TOP;
    if(m_pDockBar==GetDockingFrame()->GetControlBar(AFX_IDW_DOCKBAR_BOTTOM) ) 
		return DOCK_BOTTOM;
    if(m_pDockBar==GetDockingFrame()->GetControlBar(AFX_IDW_DOCKBAR_LEFT) )   
		return DOCK_LEFT;
    if(m_pDockBar==GetDockingFrame()->GetControlBar(AFX_IDW_DOCKBAR_RIGHT) )  
		return DOCK_RIGHT;
	return -1;
}
//--------------------------------------------------------------------------------



//--------------------------------------------------------------------------------
// This function will position all sash bars. If initialize flag set the fixed size
// of the dockablle window will be calculated
//--------------------------------------------------------------------------------
void CDockableManager::UpdateSashes(BOOL initialize) {
    CalculateVariableSize();
	if(initialize) {
		CalculateFixedSize();
	}
    GetDockingFrame()->RecalcLayout(); 
	PositionSashBars();
	LayoutControls();
}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// Paint will aloso update sashes positions if the docking stater changed.
//--------------------------------------------------------------------------------
void CDockableManager::OnPaint()  {
     CPaintDC dc(this);  

					 
	 if(m_DockSide!=GetDockSide()) {
		 UpdateSashes(TRUE);
		 m_DockSide=GetDockSide();
	 }

	 /*CBrush bkgnd( GetSysColor(COLOR_3DLIGHT));
	 dc.FillRect(CRect(0,0,m_sizeDefault.cx,HEADER),&bkgnd);*/

	 switch(GetDockSide()) {
	       case DOCK_LEFT :  
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2-2, 
					                      m_sizeDefault.cx-2*BORDER-SASH_SPACE, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				                        );
					 
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2+2, 
					                      m_sizeDefault.cx-2*BORDER-SASH_SPACE, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				            );

			                break;
	       case DOCK_RIGHT :  
	                      dc.Draw3dRect(  BORDER+SASH_SPACE, 
		                                  HEADER/2-2, 
					                      m_sizeDefault.cx-2*BORDER-SASH_SPACE, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				                        );
					 
	                      dc.Draw3dRect(  BORDER+SASH_SPACE, 
		                                  HEADER/2+2, 
					                      m_sizeDefault.cx-2*BORDER-SASH_SPACE, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				            );

			                break;
	       case DOCK_TOP :  
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2-2, 
					                      m_sizeDefault.cx-2*BORDER, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				                        );
					 
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2+2, 
					                      m_sizeDefault.cx-2*BORDER, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				            );

			                break;
	       case DOCK_BOTTOM :  
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2-2+SASH_SPACE, 
					                      m_sizeDefault.cx-2*BORDER, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				                        );
					 
	                      dc.Draw3dRect(  BORDER, 
		                                  HEADER/2+2+SASH_SPACE, 
					                      m_sizeDefault.cx-2*BORDER, 
		                                  3, 
					                      GetSysColor(COLOR_3DHILIGHT),
					                      GetSysColor(COLOR_3DSHADOW)
				            );

			                break;
	   }

     m_OnSize=TRUE;

}
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// This function will position all controls in the dockable window according to 
// the  current window size.
//--------------------------------------------------------------------------------
void CDockableManager::LayoutControls() {
static BOOL sourced=FALSE;

	m_Selector   = (CTabCtrl *)        GetDlgItem(IDC_BROWSERSELECTOR);
    if(!(m_Selector && ::IsWindow(*m_Selector))) return;
	if(sourced==FALSE) {
		ExecuteCommand(CString("source \"c:/source.dis\""));
		sourced=TRUE;
	}

	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);
	m_Tree       = (CProjectTree *)    GetDlgItem(IDC_TREE);
	m_Modules    = (CProjectModules *) GetDlgItem(IDC_MODULES);
	m_LeftPanel  = (CCBrowserFrame *)  GetDlgItem(IDC_LEFTPANE);
	m_RightPanel = (CCBrowserFrame*)   GetDlgItem(IDC_RIGHTPANE);

	m_RightSash.SetContextWindow(GetParent()->GetParent());
	m_LeftSash.SetContextWindow(GetParent()->GetParent());
	m_TopSash.SetContextWindow(GetParent()->GetParent());
	m_BottomSash.SetContextWindow(GetParent()->GetParent());
	m_DividerSash.SetContextWindow(GetParent());
	m_DividerSash.SetNotificationMessage(WM_SPLITMOVE);


	SetSplitteredWindowsPos();


	if(m_Selector->GetItemCount()!=0) return;
	TC_ITEM projTreeItem;
	projTreeItem.mask         = TCIF_TEXT;
    projTreeItem.pszText      = "Project Browser";     
    projTreeItem.cchTextMax   = 20;     
	m_Selector->InsertItem(0,&projTreeItem);
	TC_ITEM projFilesItem;
	projFilesItem.mask         = TCIF_TEXT;
    projFilesItem.pszText      = "Model Browser";     
    projFilesItem.cchTextMax   = 20;     
	m_Selector->InsertItem(1,&projFilesItem);

	// We need to supply all our ActiveX controls with IDispatch interface to the DataSource 
	// object
	LPDISPATCH lpDispatch;
	m_DataSource->GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);
	m_Tree->SetDataSource(lpDispatch);
	m_Modules->SetDataSource(lpDispatch);
	m_LeftPanel->SetDataSource(lpDispatch);
	m_RightPanel->SetDataSource(lpDispatch);

	// Browser ActiveX properties initialization.
	// Any category request will but it's results into "CategoryResultsLeft" server variable
	m_LeftPanel->SetResultsFromCategory("CategoryResultsLeft");
	// Any query request will but it's results into "QueryResultsLeft" server variable
	m_LeftPanel->SetResultsFromQuery("QueryResultsLeft");
	// Any group request will but it's results into "GroupResultsLeft" server variable
	m_LeftPanel->SetResultsFromGroup("GroupResultsLeft");
    // Any selection in categories mode will be placed into "SelectionLeft" server variable
	m_LeftPanel->SetSelectionFromCategory("SelectionLeft");
    // Any selection in queries mode will be placed into "SelectionLeft" server variable
	m_LeftPanel->SetSelectionFromQuery("SelectionLeft");
    // Any selection in groups mode will be placed into "SelectionLeft" server variable
	m_LeftPanel->SetSelectionFromGroup("SelectionLeft");
	// We will use other browser window selection as arguments for our queries
	m_LeftPanel->SetArguments("SelectionRight");
	// Initial mode for this window will be "Categories"
	m_LeftPanel->SetMode(0);


	// Browser ActiveX properties initialization.
	// Any category request will but it's results into "CategoryResultsRight" server variable
	m_RightPanel->SetResultsFromCategory("CategoryResultsRight");
	// Any query request will but it's results into "QueryResultsRight" server variable
	m_RightPanel->SetResultsFromQuery("QueryResultsRight");
	// Any group request will but it's results into "GroupResultsRight" server variable
	m_RightPanel->SetResultsFromGroup("GroupResultsRight");
    // Any selection in categories mode will be placed into "SelectionRight" server variable
	m_RightPanel->SetSelectionFromCategory("SelectionRight");
    // Any selection in queries mode will be placed into "SelectionRight" server variable
	m_RightPanel->SetSelectionFromQuery("SelectionRight");
    // Any selection in groups mode will be placed into "SelectionRight" server variable
	m_RightPanel->SetSelectionFromGroup("SelectionRight");
	// We will use other browser window selection as arguments for our queries
	m_RightPanel->SetArguments("SelectionLeft");
	// Initial mode for this window will be "Categories"
	m_RightPanel->SetMode(0);
}
//--------------------------------------------------------------------------------



//---------------------------------------------------------------------------------
// This function will calculate variable size of the dockable window.
// (This dimention can be changed by shifting a splitter)
//---------------------------------------------------------------------------------
void CDockableManager::CalculateVariableSize() {
CRect frameRect;
       GetParent()->GetWindowRect(&frameRect); 
	   switch(GetDockSide()) {
	       case DOCK_LEFT : m_sizeDefault.cy=frameRect.Height()+3;
			                break;
	       case DOCK_RIGHT: m_sizeDefault.cy=frameRect.Height()+3;
			                break;
	       case DOCK_TOP :  m_sizeDefault.cx=frameRect.Width()+3;						
			                break;
	       case DOCK_BOTTOM:m_sizeDefault.cx=frameRect.Width()+3;
			                break;
	   }
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This function will calculate fixed size of the dockable window.
// (This dimention depends only from the frame size)
//---------------------------------------------------------------------------------
void CDockableManager::CalculateFixedSize() {
CRect frameRect;
      
       GetParent()->GetWindowRect(&frameRect); 
	   switch(GetDockSide()) {
	       case DOCK_LEFT : m_sizeDefault.cx=400;
			                break;
	       case DOCK_RIGHT: m_sizeDefault.cx=400;
			                break;
	       case DOCK_TOP :  m_sizeDefault.cy=350;
			                break;
	       case DOCK_BOTTOM:m_sizeDefault.cy=350;
			                break;
	   }
	   m_SplitterX=m_sizeDefault.cx/2;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This function will position dockable window sash bar to the sizable edge
// according to the window sizes and docking side.
//---------------------------------------------------------------------------------
void CDockableManager::PositionSashBars() {
       
	m_sizeDefault.cx-=3;
	m_sizeDefault.cy-=3;
	   switch(GetDockSide()) {
	       case DOCK_LEFT : m_LeftSash.ShowWindow(SW_HIDE);
			                m_RightSash.ShowWindow(SW_SHOW);
						    m_TopSash.ShowWindow(SW_HIDE);
						    m_BottomSash.ShowWindow(SW_HIDE);
	                        m_RightSash.MoveWindow(m_sizeDefault.cx-5, 0, 6, m_sizeDefault.cy);
							m_RightSash.Invalidate();
							break;
	       case DOCK_RIGHT: m_LeftSash.ShowWindow(SW_SHOW);
			                m_RightSash.ShowWindow(SW_HIDE);
						    m_TopSash.ShowWindow(SW_HIDE);
						    m_BottomSash.ShowWindow(SW_HIDE);
	                        m_LeftSash.MoveWindow(1, 0, 6, m_sizeDefault.cy);
							m_LeftSash.Invalidate();
							break;
	       case DOCK_TOP :  m_LeftSash.ShowWindow(SW_HIDE);
			                m_RightSash.ShowWindow(SW_HIDE);
						    m_TopSash.ShowWindow(SW_HIDE);
						    m_BottomSash.ShowWindow(SW_SHOW);
	                        m_BottomSash.MoveWindow(0, m_sizeDefault.cy-5, m_sizeDefault.cx,6);
							m_BottomSash.Invalidate();
							break;
	       case DOCK_BOTTOM:m_LeftSash.ShowWindow(SW_HIDE);
			                m_RightSash.ShowWindow(SW_HIDE);
						    m_TopSash.ShowWindow(SW_SHOW);
						    m_BottomSash.ShowWindow(SW_HIDE);
	                        m_TopSash.MoveWindow(0, 1, m_sizeDefault.cx,6);
							m_TopSash.Invalidate();
							break;
	       case DOCK_FLOAT:m_LeftSash.ShowWindow(SW_HIDE);
			                m_RightSash.ShowWindow(SW_HIDE);
						    m_TopSash.ShowWindow(SW_HIDE);
						    m_BottomSash.ShowWindow(SW_HIDE);
							break;
	   }
	m_sizeDefault.cx+=3;
	m_sizeDefault.cy+=3;

}
//---------------------------------------------------------------------------------

void CDockableManager::SetSplitteredWindowsPos()  {


	m_Selector   = (CTabCtrl *)        GetDlgItem(IDC_BROWSERSELECTOR);
	m_Tree       = (CProjectTree *)    GetDlgItem(IDC_TREE);
	m_Modules    = (CProjectModules *) GetDlgItem(IDC_MODULES);
	m_LeftPanel  = (CCBrowserFrame *) GetDlgItem(IDC_LEFTPANE);
	m_RightPanel = (CCBrowserFrame*) GetDlgItem(IDC_RIGHTPANE);
	m_sizeDefault.cx-=3;
	m_sizeDefault.cy-=3;
    switch(GetDockSide()) {
	   case DOCK_LEFT : m_DividerSash.MoveWindow(m_SplitterX-3,
							                     HEADER+BORDER+SELECTOR_HEIGHT,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

		                m_Selector  -> MoveWindow(BORDER,
							                      HEADER+BORDER,
											      m_sizeDefault.cx-2*BORDER-SASH_SPACE,
											      SELECTOR_HEIGHT);

                        m_Tree      -> MoveWindow(BORDER,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

                        m_Modules   -> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SASH_SPACE-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

                        m_LeftPanel -> MoveWindow(BORDER,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

                        m_RightPanel-> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SASH_SPACE-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

						break;

	   case DOCK_RIGHT: m_DividerSash.MoveWindow(m_SplitterX-3,
							                     HEADER+BORDER+SELECTOR_HEIGHT,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

		                m_Selector  -> MoveWindow(BORDER+SASH_SPACE,
							                      HEADER+BORDER,
											      m_sizeDefault.cx-2*BORDER-SASH_SPACE,
											      SELECTOR_HEIGHT);

                        m_Tree      -> MoveWindow(BORDER+SASH_SPACE,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2-SASH_SPACE,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

                        m_Modules   -> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);
                        m_LeftPanel -> MoveWindow(BORDER+SASH_SPACE,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2-SASH_SPACE,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

                        m_RightPanel-> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

						break;



	   case DOCK_TOP:   
	                    m_DividerSash.MoveWindow(m_SplitterX-3,
							                     HEADER+BORDER+SELECTOR_HEIGHT,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);

		                m_Selector  -> MoveWindow(BORDER,
							                      HEADER+BORDER,
											      m_sizeDefault.cx-2*BORDER,
											      SELECTOR_HEIGHT);

                        m_Tree      -> MoveWindow(BORDER,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);

                        m_Modules   -> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);
                        m_LeftPanel -> MoveWindow(BORDER,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);

                        m_RightPanel-> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);		   
						break;
	   case DOCK_BOTTOM:
	                    m_DividerSash.MoveWindow(m_SplitterX-3,
							                     HEADER+BORDER+SELECTOR_HEIGHT+SASH_SPACE,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);

		                m_Selector  -> MoveWindow(BORDER,
							                      HEADER+BORDER+SASH_SPACE,
											      m_sizeDefault.cx-2*BORDER,
											      SELECTOR_HEIGHT);

                        m_Tree      -> MoveWindow(BORDER,
		                                          HEADER+BORDER+SELECTOR_HEIGHT+SASH_SPACE,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);

                        m_Modules   -> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT+SASH_SPACE,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);
                        m_LeftPanel -> MoveWindow(BORDER,
		                                          HEADER+BORDER+SELECTOR_HEIGHT+SASH_SPACE,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);

                        m_RightPanel-> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT+SASH_SPACE,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER-SASH_SPACE);
		 			    break;
	   default: 
	                    m_DividerSash.MoveWindow(m_SplitterX-3,
							                     HEADER+BORDER+SELECTOR_HEIGHT,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

		                m_Selector  -> MoveWindow(BORDER,
							                      HEADER+BORDER,
											      m_sizeDefault.cx-2*BORDER,
											      SELECTOR_HEIGHT);

                        m_Tree      -> MoveWindow(BORDER,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

                        m_Modules   -> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);
                        m_LeftPanel -> MoveWindow(BORDER,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
		                                          m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

                        m_RightPanel-> MoveWindow(m_SplitterX+SPLITTER_SPACE/2,
		                                          HEADER+BORDER+SELECTOR_HEIGHT,
								                  m_sizeDefault.cx-m_SplitterX-BORDER-SPLITTER_SPACE/2,
								                  m_sizeDefault.cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

						break;
	}
	m_sizeDefault.cx+=3;
	m_sizeDefault.cy+=3;
}


//----------------------------------------------------------------------------
// This callbac will run every time the user changed the node in the project 
// tree. We need to inform our browsers about this.
//----------------------------------------------------------------------------
void CDockableManager::OnNodeChanged(LPCTSTR projectName)  {
	m_CurrentProject=projectName;
	m_LeftPanel  = (CCBrowserFrame *) GetDlgItem(IDC_LEFTPANE);
	m_RightPanel = (CCBrowserFrame*) GetDlgItem(IDC_RIGHTPANE);
	m_Modules    = (CProjectModules *) GetDlgItem(IDC_MODULES);

    if(m_LeftPanel && ::IsWindow(*m_LeftPanel)) {
		m_LeftPanel->SetProjects(m_CurrentProject);
		m_LeftPanel->Update();
	}

    if(m_RightPanel && ::IsWindow(*m_RightPanel)) {
		m_RightPanel->SetProjects(m_CurrentProject);
		m_RightPanel->Update();
	}


    if(m_CurrentProject!=m_Modules->GetProject()) 
		m_Modules->SetProject(m_CurrentProject);
}
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// This callback runs every time the user changed selection in the project 
// list view. The only thing we need to do is to inform tree window in the 
// project browser about this event, it will change it's selection and it will
// fire "Node Changed" event.
//----------------------------------------------------------------------------
void CDockableManager::OnProjectChanged(LPCTSTR Project)  {
	m_CurrentProject=Project;
	m_Tree       = (CProjectTree *)    GetDlgItem(IDC_TREE);
    m_Tree->SetSelectedNode(m_CurrentProject);
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// This callback runs every time the user doubleclicked the file in the project
// modules window
//----------------------------------------------------------------------------
void CDockableManager::OnFileDoubleclicked(LPCTSTR element)  {
CString serverQuery;
CString filename;
BSTR query;


	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);
	if(m_DataSource==NULL) return;
	CMSDEVIntegrator* integrator=(CMSDEVIntegrator*) GetDlgItem(IDC_MSDEVINTEGRATOR);
	if(integrator==NULL) return;

    serverQuery.Format("fname %s",element);
	query=serverQuery.AllocSysString();
	filename=DataSourceAccessSync(m_DataSource, &query);
    ::SysFreeString(query);
    integrator->MakeSelection(filename,1,1,"");

}
//----------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Event "Category Changed" generated by ActiveX "Browser Frame" control will be processed 
// here. We need to set new category for the opposite browser to re-display queries list if
// no selectopn presents in this browser.
//-------------------------------------------------------------------------------------------------
void CDockableManager::OnLeftCategoryChanged(LPCTSTR list)  {

    m_RightPanel = (CCBrowserFrame*)  GetDlgItem(IDC_RIGHTPANE);
    m_RightPanel->SetSelectedCategories(list);
	
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Event "Category Changed" generated by ActiveX "Browser Frame" control will be processed 
// here. We need to set new category for the opposite browser to re-display queries list if
// no selectopn presents in this browser.
//-------------------------------------------------------------------------------------------------
void CDockableManager::OnRightCategoryChanged(LPCTSTR list)  {
   m_LeftPanel = (CCBrowserFrame*)  GetDlgItem(IDC_LEFTPANE);
   m_LeftPanel->SetSelectedCategories(list);
}
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Event "Selection Changed" generated by ActiveX "Browser Frame" control will be processed 
// here. We need to set new selection for the opposite browser to re-display queries list  and
// re-fill results window.
//-------------------------------------------------------------------------------------------------
void CDockableManager::OnLeftSelectionChanged(LPCTSTR info)  {
   m_RightPanel = (CCBrowserFrame*)  GetDlgItem(IDC_RIGHTPANE);
   // This will update ActiveX CView
   m_RightPanel->SetArguments(m_RightPanel->GetArguments());
   CWnd* main = GetParent()->GetParent();
   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,info);


	// We will use global variable called "Selection" to store the selection
	// for all other modes wich will need the sDiscoverelection.
    CString query;
	CString sizeStr;
	BSTR q;
	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);


	query="global DiscoverSelection; set DiscoverSelection $SelectionLeft; set tmp 1";
    ExecuteCommand(query);

	query="size $DiscoverSelection";
	q=query.AllocSysString();
	sizeStr=DataSourceAccessSync(m_DataSource, &q);
    ::SysFreeString(q);
	((CMainFrame *)main)->m_SelectionSize=atoi(sizeStr);


}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Event "Selection Changed" generated by ActiveX "Browser Frame" control will be processed 
// here. We need to set new selection for the opposite browser to re-display queries list  and
// re-fill results window.
//-------------------------------------------------------------------------------------------------
void CDockableManager::OnRightSelectionChanged(LPCTSTR info)  {
   m_LeftPanel = (CCBrowserFrame*)  GetDlgItem(IDC_LEFTPANE);
   // This will update ActiveX
   m_LeftPanel->SetArguments(m_LeftPanel->GetArguments());
   CWnd* main = GetParent()->GetParent();
   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,info);

	// We will use global variable called "Selection" to store the selection
	// for all other modes wich will need the sDiscoverelection.
    CString query;
	CString sizeStr;
	BSTR q;

	query="global DiscoverSelection; set DiscoverSelection $SelectionRight; set tmp 1";
    ExecuteCommand(query);

	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);
	query="size $DiscoverSelection";
	q=query.AllocSysString();
	sizeStr=DataSourceAccessSync(m_DataSource, &q);
    ::SysFreeString(q);
	((CMainFrame *)main)->m_SelectionSize=atoi(sizeStr);


}
//-------------------------------------------------------------------------------------------------


void CDockableManager::OnRightGroupChanged(LPCTSTR gList)  {
   m_LeftPanel = (CCBrowserFrame*)  GetDlgItem(IDC_LEFTPANE);
   // This will update ActiveX
   m_LeftPanel->SetArguments(m_LeftPanel->GetArguments());
}
//-------------------------------------------------------------------------------------------------

void CDockableManager::OnLeftGroupChanged(LPCTSTR gList)  {
   m_RightPanel = (CCBrowserFrame*)  GetDlgItem(IDC_RIGHTPANE);
   // This will update ActiveX
   m_RightPanel->SetArguments(m_RightPanel->GetArguments());
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// This callback runs every time we ActiveX element list fires the "Element Doubleclicked"
// event. We will form the element description string and we will send this string to the
// MSDEVIntegractor ActiveX, it will show this string in the Developer Studio Discover output
// window.
//-------------------------------------------------------------------------------------------------
void CDockableManager::OnElementdoubleclicked(LPCTSTR parmlist)  {
register i;
CString serverQuery;
CString serverResults;
CString param;
CString filestr;
CString tokenstr;
CString linestr;
CString colstr;
BSTR q;

CString filteredSetName;
CString elementN;
int elNumber;
int count=0;

	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);
	CMSDEVIntegrator* integrator=(CMSDEVIntegrator*) GetDlgItem(IDC_MSDEVINTEGRATOR);
	if(integrator==NULL) return;

    for(i=0;i<strlen(parmlist);i++) {
	   if(parmlist[i]==' ') {
		   count++;
		   continue;
	   }
	   if(count==0) filteredSetName+=parmlist[i];
	   if(count==1) elementN  +=parmlist[i];
    }
    elNumber=atoi(elementN);

	// Asking Developer Studio for it's current tab size.
	int tabsize = integrator->GetTabSize();

	// Queryng server to obtain Developer Studio filename, line and position.
	// The "sym2msdev <tabsize> <element#> <set>" command will return the following :
	// logfilename \n
	// line\n
	// column\n
	// token
    serverQuery.Format("sym2msdev %d %ld $%s",tabsize, elNumber,filteredSetName);
	q=serverQuery.AllocSysString();
	param=DataSourceAccessSync(m_DataSource, &q);
    ::SysFreeString(q);
	// Extracting parameters
	count=0;
	for(i=0;i<param.GetLength();i++) {
		if(param[i]=='\n') {
			count++;
            continue;
		}
		switch(count) {
		   case 0 : filestr += param[i]; break;
		   case 1 : linestr += param[i]; break;
		   case 2 : colstr  += param[i]; break;
		   case 3 : tokenstr+= param[i]; break;
		}
	}
	// Selecting
	if(filestr.GetLength()!=0)
       integrator->MakeSelection(filestr,atoi(linestr),atoi(colstr)-1,tokenstr);

}
//----------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// This callback runs every time the user make Ctrl+Click in the Developer Studio.
// The MSDEV Integrator fire the CtrlClick event to run this callback.
//-------------------------------------------------------------------------------------------------
void CDockableManager::IntegratorCtrlClick(LPCTSTR lfile, long line,long col, long size){
CMSDEVIntegrator* integrator=(CMSDEVIntegrator*) GetDlgItem(IDC_MSDEVINTEGRATOR);
CString serverQuery;
CString infoString;
CString ffile="";

BSTR q;

	if(integrator==NULL) return;
	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);

	// Asking Developer Studio for it's current tab size.
	int tabsize = integrator->GetTabSize();


	// Getting info string
	register i;
	if(lfile!=NULL) {
		for(i=0;i<strlen(lfile);i++) {
		    if(lfile[i]=='\\') 
			    ffile+="/";
		    else
			    ffile+=lfile[i];
		}
	}
    serverQuery.Format("msdev_info \"%s\" %ld %ld %ld",ffile, line, col, tabsize);
	q=serverQuery.AllocSysString();
	infoString=DataSourceAccessSync(m_DataSource, &q);
    ::SysFreeString(q);
	integrator->SetInfo(infoString);
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Callback runs every time the user clicked "Query" button in the Developer
// Studio. We are extracting symbol from the developer studio into Query list.
//-------------------------------------------------------------------------------------------------
void CDockableManager::IntegratorQuery(LPCTSTR lfile, long line,long col, long size){
CMSDEVIntegrator* integrator=(CMSDEVIntegrator*) GetDlgItem(IDC_MSDEVINTEGRATOR);
CString serverQuery;
CString infoString;
CString ffile;

BSTR q;

	if(integrator==NULL) return;
	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);

	// Asking Developer Studio for it's current tab size.
	int tabsize = integrator->GetTabSize();

	register i;
	if(lfile!=NULL) {
		for(i=0;i<strlen(lfile);i++) {
		    if(lfile[i]=='\\') 
			    ffile+="/";
		    else
			    ffile+=lfile[i];
		}
	}
	// Getting symbol
    serverQuery.Format("set tmp [msdev2sym \"%s\" %ld %ld %ld]",ffile, line, col-1, tabsize);
	q=serverQuery.AllocSysString();
	CString test;
	test=DataSourceAccessSync(m_DataSource, &q);
    ::SysFreeString(q);

	// Setting symbol
	m_LeftPanel = (CCBrowserFrame*) GetDlgItem(IDC_LEFTPANE);
	m_LeftPanel->SetSet("tmp");

    serverQuery="unset tmp";
	q=serverQuery.AllocSysString();
	DataSourceAccessSync(m_DataSource, &q);
    ::SysFreeString(q);
}
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Callback runs every time the user clicked "Definition" button in the Developer
// Studio. We are opening definition of the selected symbol.
//-------------------------------------------------------------------------------------------------
void CDockableManager::IntegratorDefinition(LPCTSTR lfile, long line,long col, long size){
register i;
CMSDEVIntegrator* integrator=(CMSDEVIntegrator*) GetDlgItem(IDC_MSDEVINTEGRATOR);
CString serverQuery;
CString ffile;
CString param;
int     count;
CString filestr;
CString tokenstr;
CString linestr;
CString colstr;

BSTR q;

	if(integrator==NULL) return;
	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);

	// Asking Developer Studio for it's current tab size.
	int tabsize = integrator->GetTabSize();

	if(lfile!=NULL) {
		for(i=0;i<strlen(lfile);i++) {
		    if(lfile[i]=='\\') 
			    ffile+="/";
		    else
			    ffile+=lfile[i];
		}
	}
	// Getting symbol
    serverQuery.Format("set __dis__tmp [msdev2sym \"%s\" %ld %ld %ld]",ffile, line, col-1, tabsize);
	q=serverQuery.AllocSysString();
	CString test;
	test=DataSourceAccessSync(m_DataSource, &q);
    ::SysFreeString(q);

	// Queryng server to obtain Developer Studio filename, line and position.
	// The "sym2msdev <tabsize> <element#> <set>" command will return the following :
	// filename \n
	// line\n
	// column\n
	// token
    serverQuery.Format("sym2msdev %d 0 $__dis__tmp",tabsize);
	q=serverQuery.AllocSysString();
	param=DataSourceAccessSync(m_DataSource, &q);
    ::SysFreeString(q);
	// Extracting parameters
	count=0;
	for(i=0;i<param.GetLength();i++) {
		if(param[i]=='\n') {
			count++;
            continue;
		}
		switch(count) {
		   case 0 : filestr += param[i]; break;
		   case 1 : linestr += param[i]; break;
		   case 2 : colstr  += param[i]; break;
		   case 3 : tokenstr+= param[i]; break;
		}
	}
	// Converting logical name into NT name
	// Selecting
	if(filestr.GetLength()!=0)
       integrator->MakeSelection(filestr,atoi(linestr),atoi(colstr)-1,tokenstr);

    serverQuery="unset tmp";
	q=serverQuery.AllocSysString();
	DataSourceAccessSync(m_DataSource, &q);
    ::SysFreeString(q);
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void CDockableManager::IntegratorActivate(void){
CMainFrame* main;

    main = (CMainFrame *)AfxGetMainWnd();
	main->SetForegroundWindow();
	main->ActivateFrame();
}
//-------------------------------------------------------------------------------------------------

CString CDockableManager::ExecuteCommand(CString & command) {
BSTR q;
CString result;

	m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);
	q=command.AllocSysString();
	result = DataSourceAccessSync(m_DataSource, &q);
	::SysFreeString(q);
	return result;
}


void CDockableManager::BrowseGivenSet(CString & name, int col) {
static int lastCol=0;

    if(col==0) {
	    m_LeftPanel = (CCBrowserFrame*) GetDlgItem(IDC_LEFTPANE);
	    m_LeftPanel->SetSet(name);
		lastCol=0;
	} else {
        if(col==1) {
	        m_RightPanel = (CCBrowserFrame*) GetDlgItem(IDC_RIGHTPANE);
	        m_RightPanel->SetSet(name);
			lastCol=1;
		} else {
			lastCol=++lastCol%2;
			if(lastCol==0) {
	             m_LeftPanel = (CCBrowserFrame*) GetDlgItem(IDC_LEFTPANE);
	             m_LeftPanel->SetSet(name);
			}
			if(lastCol==1) {
	             m_RightPanel = (CCBrowserFrame*) GetDlgItem(IDC_RIGHTPANE);
	             m_RightPanel->SetSet(name);
			}
		}
	}
}

void CDockableManager::OnServiceChanged() {


    CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
	m_DataSource = (CDataSource *)GetDlgItem(IDC_DATASOURCE);
	m_DataSource->SetSource(app->m_ServiceName,FALSE);
	LPDISPATCH lpDispatch;
	m_DataSource->GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);
	m_Tree       = (CProjectTree *)    GetDlgItem(IDC_TREE);
	m_Tree->SetDataSource(lpDispatch);
	m_Modules    = (CProjectModules *) GetDlgItem(IDC_MODULES);
	m_Modules->SetDataSource(lpDispatch);
	m_LeftPanel  = (CCBrowserFrame *) GetDlgItem(IDC_LEFTPANE);
	m_LeftPanel->SetDataSource(lpDispatch);
	m_RightPanel = (CCBrowserFrame*) GetDlgItem(IDC_RIGHTPANE);
	m_RightPanel->SetDataSource(lpDispatch);

    ExecuteCommand(CString("set_ui_type 1"));
    ExecuteCommand(CString("source_dis \"source.dis\""));
    ExecuteCommand(CString("set DiscoverSelection [nil_set]"));
    ExecuteCommand(CString("set DiscoverScope /"));

    CWnd* main = GetParent()->GetParent();
	((CMainFrame *)main)->m_SelectionSize=0;
}

void CDockableManager::OnServerNotification(LPCTSTR str)  {

	processing=TRUE;
register i;
int words=0;
CString wordArray[100];
CString curStr;
CPromptDialog dlg;
CQuestionDlg question;
m_DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);

   CWnd* main = AfxGetMainWnd();


	for(i=0;i<strlen(str);i++) {
		if(str[i]=='\n') {
			wordArray[words++]=curStr;
			while(str[i]=='\n' && i<strlen(str)) i++;
			curStr="";
		}
		curStr+=str[i];
	}
	if(curStr.GetLength()>0) wordArray[words++]=curStr;


	//--------- "add_menu" notification processing 
	if(wordArray[0]==CString("add_menu")) {
         ((CMainFrame *)main)->AddAccessMenu(wordArray[1],wordArray[2]);
         ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
         m_DataSource->AcceptNotification("*");
		 processing = FALSE;
		 return;
	}
	//---------------------------------------------



	//--------- "browser" notification processing
	if(wordArray[0]==CString("browser")) {
		 // If adding menu item
         m_DataSource->AcceptNotification("*");
		 processing = FALSE;
		 if(wordArray[1]=="add") {
              ((CMainFrame *)main)->AddAccessMenuItem(wordArray[2],
				                                      wordArray[3],
													  wordArray[4]);
              ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
		 }
		 // If removing elements from menu
		 if(wordArray[1]=="remove") {
             ((CMainFrame *)main)->RemoveAccessMenuItem(wordArray[2],wordArray[3]);
             ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
		 }
		 // If setting results 
		 if(wordArray[1]=="set") {
			 browserVarName=wordArray[3];
			 browserSetType = 0;
			 if(wordArray[2]=="elements") browserSetType = 0;
			 if(wordArray[2]=="results")  browserSetType = 1;
			 if(wordArray[2]=="next")     browserSetType = -1;
			 PostMessage(WM_COMMAND,IDC_SETBROWSER,0);
		 }
		 return;
	}
	//--------------------------------------------


	//----------- "add_item" notification processing
	if(wordArray[0]==CString("add_item")) {
	     CString accessCmds;
	     if(wordArray[4]==CString("-scope")) {
			    accessCmds=wordArray[3];
				accessCmds+=" $DiscoverScope";
		 } else {
	         if(wordArray[4]==CString("-opt") || wordArray[3]==CString("-optional")) {
				   accessCmds="set tmp [nil_set}; sappend $tmp $DiscoverSelection;";
			       accessCmds+=wordArray[3];
				   accessCmds+=" $tmp; unset tmp";
			 } else {
	             if(wordArray[4]==CString("-none")) {
			         accessCmds=wordArray[3];
				 } else {
					 if(wordArray[4]==CString("-filter")) {
                         accessCmds=wordArray[3];
                         accessCmds+=" $DiscoverElements";
					 } else {
				         CAccessMenu* menu=((CMainFrame *)main)->m_AccessMenu.FindMenu(wordArray[1]);
					     if(menu!=NULL) {
						     CString sel = menu->GetDefSelection();
	                         if(sel==CString("-scope")) {
			                     accessCmds=wordArray[3];
				                 accessCmds+=" $DiscoverScope";
							 } else {
	                             if(sel==CString("-opt") || sel==CString("-optional")) {
				                     accessCmds="set tmp [nil_set]; sappend $tmp $DiscoverSelection;";
			                         accessCmds+=wordArray[3];
				                     accessCmds+=" $tmp; unset tmp";
								 } else {
					                 if(sel==CString("-filter")) {
                                         accessCmds=wordArray[3];
                                         accessCmds+=" $DiscoverElements";
									 } else {
	                                     if(sel==CString("-none")) {
			                                 accessCmds=wordArray[3];
										 } else {
			                                 accessCmds=wordArray[3];
				                             accessCmds+=" $DiscoverSelection";
										 }
									 }
								 }
							 }
						 } else {
			                accessCmds=wordArray[3];
				            accessCmds+=" $DiscoverSelection";
						 }
					 }



				 }
			 }
		 }
         ((CMainFrame *)main)->AddAccessMenuItem(wordArray[1],wordArray[2],accessCmds);
         ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
         m_DataSource->AcceptNotification("*");
		 processing = FALSE;
		 return;
	}
	//------------------------------------------------------------------



	//------"prompt" notification processing
	if((words==2 || words==4) && (wordArray[0]==CString("prompt"))) {
		 if(words==2) {
			 dlg.m_PromptText=wordArray[1];
		 } else {
		     if((words==4) && (wordArray[1]==CString("-title"))) {
			     dlg.SetPromptTitle(wordArray[2]);
			     dlg.m_PromptText=wordArray[3];
			 } else return;
		 }
		 dlg.m_UserText="";
	     if(dlg.DoModal()==IDOK) {
             m_DataSource->AcceptNotification(dlg.m_UserText);
		     processing = FALSE;
		 } else {
             m_DataSource->AcceptNotification(" ");
		     processing = FALSE;
		 }
		 return;
	}
	//------------------------------------------------------------------------



    //----- "remove menu" notification processing ------------------------------
	if((words==2) && (wordArray[0]==CString("remove_menu"))) {
       ((CMainFrame *)main)->RemoveAccessMenu(wordArray[1]);
       ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
       m_DataSource->AcceptNotification("*");
	   processing = FALSE;
	   return;
	}
	//---------------------------------------------------------------------------


	//----- "remove item" notification processing -------------------------------
	if((words==3) && (wordArray[0]==CString("remove_item"))) {
       ((CMainFrame *)main)->RemoveAccessMenuItem(wordArray[1],wordArray[2]);
       ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
       m_DataSource->AcceptNotification("*");
	   processing = FALSE;
	   return;
	}
	//-----------------------------------------------------------------------------

	//----- "dis_message" notification processing -------------------------------
	if((words>0) && (wordArray[0]==CString("error"))) {
	   main->MessageBox(wordArray[1],"Server error",MB_OK|MB_ICONERROR);
       m_DataSource->AcceptNotification("*");
       processing = FALSE;
	   return;
	}
	if((words>0) && (wordArray[0]==CString("stderror"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       m_DataSource->AcceptNotification("*");
	   processing = FALSE;
	   return;
	}
	if((words>0) && (wordArray[0]==CString("warning"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       m_DataSource->AcceptNotification("*");
	   processing = FALSE;
	   return;
	}
	if((words>0) && (wordArray[0]==CString("warning"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       m_DataSource->AcceptNotification("*");
	   processing = FALSE;
	   return;
	}
	if((words>0) && (wordArray[0]==CString("status"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       m_DataSource->AcceptNotification("*");
	   processing = FALSE;
	   return;
	}
	if((words>0) && (wordArray[0]==CString("info"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       m_DataSource->AcceptNotification("*");
	   processing = FALSE;
	   return;
	}
	if((words>0) && (wordArray[0]==CString("diag"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       m_DataSource->AcceptNotification("*");
	   processing = FALSE;
	   return;
	}
	if((words>0) && (wordArray[0]==CString("default"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       m_DataSource->AcceptNotification("*");
	   processing = FALSE;
	   return;
	}
	//-----------------------------------------------------------------------------

	if((words>1) && (wordArray[0]==CString("question"))) {
	   CString title;
	   CString message;
	   CString btn1;
	   CString btn2;
	   CString btn3;
	   if(wordArray[1]=="-title") {
		   title = wordArray[2];
		   if(words>3) message = wordArray[3];
		   if(words>4) btn1=wordArray[4];
		   if(words>5) btn2=wordArray[5];
		   if(words>6) btn3=wordArray[6];
	   } else {
		   if(words>1) message = wordArray[1];
		   if(words>2) btn1=wordArray[2];
		   if(words>3) btn2=wordArray[3];
		   if(words>4) btn3=wordArray[4];
	   }
	   question.m_Title   = title;
	   question.m_Message = message;
	   question.m_BtnTitle1 = btn1;
	   question.m_BtnTitle2 = btn2;
	   question.m_BtnTitle3 = btn3;
	   question.DoModal();
       m_DataSource->AcceptNotification( question.m_Result);
	   processing = FALSE;
	   return;
	}

	// accepting for all unknown notifications
    m_DataSource->AcceptNotification("*");
    processing = FALSE;
}


void CDockableManager::SetBrowserVariable(void) {
   if(processing) {
	   // will process this command later
	   PostMessage(WM_COMMAND,IDC_SETBROWSER,0);
   } else {
      BrowseGivenSet(CString("notification_var_lock"),browserSetType);
      ExecuteCommand(CString("unset notification_var_lock"));
   }
}

CString CDockableManager::DataSourceAccessSync(CDataSource* DataSource, BSTR* command) {
	CString res;
	static BOOL bCommDlgShown = FALSE;
	res = DataSource->AccessSync(command);
	if (DataSource->IsConnectionLost() == TRUE &&
		bCommDlgShown != TRUE ) {
		::MessageBox(m_hWnd, 
			   _T("Connection with server is lost."
			   " Make sure the server is running."),
			   _T("Server Communication Error."), 
			   MB_OK | MB_ICONINFORMATION); 
		bCommDlgShown = TRUE;
	} else if (DataSource->IsConnectionLost() != TRUE) {
		bCommDlgShown = FALSE;
	}	
	return res;
}
