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
// DockableBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "afxpriv.h"
#include "discovermdi.h"
#include "DockableBrowser.h"
#include "CodeRoverIntegrator.h"
#include "MainFrm.h"
#include "PromptDialog.h"
#include "QuestionDlg.h"
#include "ListSelDlg.h"
#include "EditorSelectionDialog.h"
#include "CMSelectionDialog.h"
#include "CMAttributesDlg.h"

#include "resource.h"
#ifndef __ATTRIBUTESDIALOG__
   #define __ATTRIBUTESDIALOG__
   #include "AttributesDialog.h"
#endif
#ifndef __FILTERSHEET__
   #define __FILTERSHEET__
   #include "FilterSheet.h"
#endif
#ifndef __COMPLEXSORT__
   #define __COMPLEXSORT__
   #include "ComplexSort.h"
#endif
#ifndef __ADDSELECTOR__
   #define __ADDSELECTOR__
   #include "AddSelectorDlg.h"
#endif

#define HEADER 10
#define BORDER 2
#define SASH_SPACE 4
#define SPLITTER_SPACE 4
#define SELECTOR_HEIGHT 26

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int  browserSetType=0;
static CString browserVarName;

#define IDC_SETBROWSER 29874

/////////////////////////////////////////////////////////////////////////////
// CDockableBrowser

CDockableBrowser::CDockableBrowser() {
	AfxEnableControlContainer();
	m_DockSide=-1;
	m_OnSize=FALSE;
	m_QueriesEnabled=TRUE;
	m_Updated = FALSE;
	m_CurrentHistoryEntry=NULL;
	m_VarIndex=0;
	m_ResultWasEmpty=FALSE;
	m_Closure=FALSE;
	m_pCMMenu = NULL;
	m_pCMCommands = NULL;
	m_pCMAttributes = NULL;
//	m_cxLeftBorder=m_cxRightBorder=m_cyTopBorder=m_cyBottomBorder=0;
}

CDockableBrowser::~CDockableBrowser()
{
	if(m_pCMMenu) delete m_pCMMenu;
	if(m_pCMCommands) delete m_pCMCommands;
	if(m_pCMAttributes) delete m_pCMAttributes;
}


BEGIN_MESSAGE_MAP(CDockableBrowser, CDialogBar)
	//{{AFX_MSG_MAP(CDockableBrowser)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_MODESELECTOR, OnQueryModeChanged)
	ON_WM_CREATE()
	ON_MESSAGE(WM_SASHMOVED,OnSashMoved)
	ON_MESSAGE(WM_PROJECTSPLITMOVE,OnProjectSplitMoved)
	ON_MESSAGE(UM_REFRESH_FILE_LIST,OnRefreshFileList)
	ON_WM_PAINT()
	ON_MESSAGE(WM_SERVICE_DISCONNECT,OnServiceDisconnect)
	ON_COMMAND(IDC_SERVICECHANGED,OnServiceChanged)
	ON_COMMAND(IDC_ENABLEQUERIES, OnQueriesEnable)
	ON_UPDATE_COMMAND_UI(IDC_ENABLEQUERIES,OnUpdateQueriesBtn)
	ON_COMMAND(IDC_PREV, PrevHistory)
	ON_COMMAND(IDC_NEXT, NextHistory)
	ON_UPDATE_COMMAND_UI(IDC_PREV,OnUpdatePrev)
	ON_UPDATE_COMMAND_UI(IDC_NEXT,OnUpdateNext)
	ON_UPDATE_COMMAND_UI(IDC_FILTER,OnUpdateFilter)
	ON_COMMAND(IDC_SELECTSORT,   OnSelectSort)
	ON_COMMAND(IDC_SELATTRIBUTES,OnSetAttributes)
	ON_COMMAND(IDC_SELECTFILTER, OnSetFilter)
	ON_COMMAND(IDC_RESETFILTER,  OnResetFilter)
	ON_COMMAND(IDC_ADDELEMENTS,  OnAddElements)
	ON_COMMAND(IDC_SAVEELEMENTS,  OnSaveElements)
	ON_COMMAND(IDC_SETBROWSER,SetBrowserVariable)
	ON_COMMAND(IDC_FILTER,       OnSetFilter)
	ON_UPDATE_COMMAND_UI(IDC_CLOSURE, OnUpdateClosure)
	ON_COMMAND(IDC_CLOSURE,      OnClosure)
	ON_UPDATE_COMMAND_UI(IDC_SHORTNAMES, OnUpdateShortNames)
	ON_COMMAND(IDC_SHORTNAMES,      OnShortNames)
	ON_COMMAND(ID_CM_SETUP, OnCMSetup)
	ON_UPDATE_COMMAND_UI(ID_CM_SETUP, OnUpdateCMSetup)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_CM_ACTIONS,ID_CM_ACTIONS+50,OnCMAction)
	ON_COMMAND_RANGE(IDC_QUERIES_START,IDC_QUERIES_START+200,OnMenuQuery)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CDockableBrowser, CDialogBar)
    //{{AFX_EVENTSINK_MAP(CDockableBrowser)
	ON_EVENT(CDockableBrowser, IDC_TREE,     1 /* NodeChanged */,          OnNodeChanged,           VTS_BSTR)
	ON_EVENT(CDockableBrowser, IDC_CMINTEGRATORCTRL,1 /* CMEvent */, OnCMEvent,  VTS_BSTR )
	ON_EVENT(CDockableBrowser, IDC_CODEROVERINTEGRATORCTRL,1 /* IntegratorCtrlClick */, IntegratorCtrlClick,  VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDockableBrowser, IDC_CODEROVERINTEGRATORCTRL,2 /* IntegratorQuery */,     IntegratorQuery,      VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDockableBrowser, IDC_CODEROVERINTEGRATORCTRL,3 /* IntegratorDefinition */,IntegratorDefinition, VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDockableBrowser, IDC_CODEROVERINTEGRATORCTRL,4 /* IntegratorActivate */,  IntegratorActivate,   VTS_NONE)
	ON_EVENT(CDockableBrowser, IDC_DATASOURCE,1 /* ServerNotification */,       OnServerNotification,   VTS_BSTR)
	ON_EVENT(CDockableBrowser, IDC_CATEGORIES,1 /* CategoryChanged */,  OnCategoryChanged,     VTS_BSTR)
	ON_EVENT(CDockableBrowser, IDC_RESULTS,  4 /* SelectionChanged */,     OnSelectionChanged,    VTS_BSTR)
	ON_EVENT(CDockableBrowser, IDC_RESULTS,  3 /* ElementDoubleclicked */, OnElementDoubleclicked,VTS_BSTR)
	ON_EVENT(CDockableBrowser, IDC_RESULTS,  2 /* RButtonEvent */, OnRButtonEvent,        VTS_I2 VTS_I2)
	ON_EVENT(CDockableBrowser, IDC_QUERIES,  1 /* QueryChanged */,     OnQueryChanged,     VTS_BSTR)

	ON_EVENT(CDockableBrowser, IDC_GROUPS,     1 /* GroupChanged */,     OnGroupChanged,     VTS_BSTR)
	ON_EVENT(CDockableBrowser, IDC_GROUPS,     2 /* GroupCreated */,     OnGroupCreated,     VTS_BSTR)
	ON_EVENT(CDockableBrowser, IDC_GROUPS,     3 /* GroupRemoved */,     OnGroupRemoved,     VTS_I2)
	ON_EVENT(CDockableBrowser, IDC_GROUPS,     5 /* SelectionInserted */,OnGroupInsSel, 0)
	ON_EVENT(CDockableBrowser, IDC_GROUPS,     6 /* SelectionRemover */, OnGroupDelSel, 0)
//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP() 

/////////////////////////////////////////////////////////////////////////////
// CDockableBrowser message handlers






void CDockableBrowser::OnSize(UINT nType, int cx, int cy) {
	if(m_Updated==FALSE) {
	    CTabCtrl*           ModeSelector;
        CProjectTree*       Tree;
	    CCategoriesCombo*   Category;
	    CQueriesCombo*      Query;
	    CGroupsCombo*       Group;
	    ModeSelector=(CTabCtrl*)           GetDlgItem(IDC_MODESELECTOR);
		if(ModeSelector==NULL) return;
	    Tree       = (CProjectTree *)      GetDlgItem(IDC_TREE);
	    Category   = (CCategoriesCombo *)  GetDlgItem(IDC_CATEGORIES);
	    Query      = (CQueriesCombo *)     GetDlgItem(IDC_QUERIES);
	    Group      = (CGroupsCombo *)      GetDlgItem(IDC_GROUPS);
		if(ModeSelector) {

			CBitmap projectBmp;
			CBitmap categoryBmp;
			CBitmap queryBmp;
			CBitmap groupBmp;

			projectBmp.LoadMappedBitmap(IDB_BROWSER_PROJECTS);
			categoryBmp.LoadMappedBitmap(IDB_BROWSER_CATEGORIES);
			queryBmp.LoadMappedBitmap(IDB_BROWSER_QUERIES);
			groupBmp.LoadMappedBitmap(IDB_BROWSER_GROUPS);

		    m_ModeImages.Create(15,15,TRUE,4,1);
	        m_ModeImages.Add(&projectBmp,   GetSysColor(COLOR_BTNFACE));
	        m_ModeImages.Add(&categoryBmp,  GetSysColor(COLOR_BTNFACE));
	        m_ModeImages.Add(&queryBmp,     GetSysColor(COLOR_BTNFACE));
	        m_ModeImages.Add(&groupBmp,     GetSysColor(COLOR_BTNFACE));

            ModeSelector->SetImageList(&m_ModeImages);


		    TC_ITEM item;
	        item.mask         = TCIF_TEXT | TCIF_IMAGE;
            item.pszText      = "Projects";     
            item.cchTextMax   = 15;     
            item.iImage       = 0;     
	        ModeSelector->InsertItem(0,&item);

	        item.mask         = TCIF_TEXT | TCIF_IMAGE;
            item.pszText      = "Categories";     
            item.cchTextMax   = 15;     
            item.iImage       = 1;     
	        ModeSelector->InsertItem(1,&item);

	        item.mask         = TCIF_TEXT | TCIF_IMAGE;
            item.pszText      = "Queries";     
            item.cchTextMax   = 15;     
            item.iImage       = 2;     
	        ModeSelector->InsertItem(2,&item);

	        item.mask         = TCIF_TEXT | TCIF_IMAGE;
            item.pszText      = "Groups";     
            item.cchTextMax   = 15;     
            item.iImage       = 3;     
	        ModeSelector->InsertItem(3,&item);


            m_ProjectDividerSash.ShowWindow(SW_HIDE);
            ModeSelector -> ShowWindow(SW_HIDE);
            Tree         -> ShowWindow(SW_HIDE);
	        Category     -> ShowWindow(SW_HIDE);
	        Query        -> ShowWindow(SW_HIDE);
	        Group        -> ShowWindow(SW_HIDE);
	        if(m_QueriesEnabled==TRUE) {
	        m_ProjectDividerSash.ShowWindow(SW_SHOW);
	            ModeSelector -> ShowWindow(SW_SHOW);
	            switch(ModeSelector->GetCurSel()) {
	                case 0 : Tree->ShowWindow(SW_SHOW);
				             break;
	                case 1 : Category->ShowWindow(SW_SHOW);
				             break;
	                case 2 : Query->ShowWindow(SW_SHOW);
				             break;
	                case 3 : Group->ShowWindow(SW_SHOW);
				             break;
				}
			}
			m_SplitBmp.LoadMappedBitmap(IDB_BROWSER_SPLIT);
			m_PrevBmp.LoadMappedBitmap(IDB_BROWSER_PREV);
			m_NextBmp.LoadMappedBitmap(IDB_BROWSER_NEXT);
	        CButton* QueriesBtn;
	        CButton* PrevBtn;
	        CButton* NextBtn;
	        QueriesBtn = (CButton *)GetDlgItem(IDC_ENABLEQUERIES);
	        PrevBtn    = (CButton *)GetDlgItem(IDC_PREV);
	        NextBtn    = (CButton *)GetDlgItem(IDC_NEXT);
	        QueriesBtn->SetBitmap(m_SplitBmp);
	        PrevBtn->SetBitmap(m_PrevBmp);
	        NextBtn->SetBitmap(m_NextBmp);
			m_Updated=TRUE;
		}
	}
	LayoutControls();
}


int CDockableBrowser::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
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

	m_ProjectDividerSash.Create("", WS_CHILD  | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,10004);
	m_ProjectDividerSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);
	return 0;
}

//---------------------------------------------------------------------------------
// Callback runs every time  sash changed it's position.
// We need to resize our dockable window, position all controls in it and update
// our sash position.
//---------------------------------------------------------------------------------
LRESULT CDockableBrowser::OnSashMoved(WPARAM wparam, LPARAM lparam) {
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
	if(m_ProjectSplitterX>=m_sizeDefault.cx-15) {
		m_ProjectSplitterX=m_sizeDefault.cx-15;
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
LRESULT CDockableBrowser::OnProjectSplitMoved(WPARAM wparam, LPARAM lparam) {
CRect winRect;
CRect sashRect;

    GetWindowRect(&winRect);
	m_ProjectDividerSash.GetWindowRect(&sashRect);
    m_ProjectSplitterX=(int)wparam+(sashRect.left-winRect.left);
    SetSplitteredWindowsPos();
    return TRUE;
}
//---------------------------------------------------------------------------------

LRESULT CDockableBrowser::OnRefreshFileList(WPARAM wparam, LPARAM lparam) {
	BrowseGivenSet(CString(""));
    return TRUE;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// Callback runs every time  sash changed it's position.
// We need to resize our dockable window, position all controls in it and update
// our sash position.
//---------------------------------------------------------------------------------
LRESULT CDockableBrowser::OnQuerySplitMoved(WPARAM wparam, LPARAM lparam) {
CRect winRect;
CRect sashRect;

    GetWindowRect(&winRect);
	m_QueryDividerSash.GetWindowRect(&sashRect);
    m_QuerySplitterX=(int)wparam+(sashRect.left-winRect.left);
    SetSplitteredWindowsPos();
    return TRUE;
}
//---------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
// This function returns the current side of the frame window to which this 
// dockable bar docked. The values are DOCK_LEFT, DOCK_RIGHT, DOCK_TOP, DOCK_BOTTOM.
//--------------------------------------------------------------------------------
int CDockableBrowser::GetDockSide() {
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
void CDockableBrowser::UpdateSashes(BOOL initialize) {
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
void CDockableBrowser::OnPaint()  {
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
void CDockableBrowser::LayoutControls() {
static BOOL sourced=FALSE;

    if(!::IsWindow(*this)) return;
    CDataSource* DataSource = (CDataSource *) GetDlgItem(IDC_DATASOURCE);
	if(DataSource==NULL) return;
	m_RightSash.SetContextWindow(GetParent()->GetParent());
	m_LeftSash.SetContextWindow(GetParent()->GetParent());
	m_TopSash.SetContextWindow(GetParent()->GetParent());
	m_BottomSash.SetContextWindow(GetParent()->GetParent());
	m_ProjectDividerSash.SetContextWindow(GetParent());
	m_ProjectDividerSash.SetNotificationMessage(WM_PROJECTSPLITMOVE);

    SetSplitteredWindowsPos();


}
//--------------------------------------------------------------------------------



//---------------------------------------------------------------------------------
// This function will calculate variable size of the dockable window.
// (This dimention can be changed by shifting a splitter)
//---------------------------------------------------------------------------------
void CDockableBrowser::CalculateVariableSize() {
CRect frameRect;
      if(!::IsWindow(*this)) return;
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
void CDockableBrowser::CalculateFixedSize() {
CRect frameRect;
      
       GetParent()->GetWindowRect(&frameRect); 
	   int xlen = 1024;
	   int ylen = 768;
	   CDC* dc = GetWindowDC();
	   if(dc) {
	       int xlen = dc->GetDeviceCaps(HORZRES);
	       int ylen = dc->GetDeviceCaps(HORZRES);
	   }
	   switch(GetDockSide()) {
	       case DOCK_LEFT : m_sizeDefault.cx=xlen/3;
			                break;
	       case DOCK_RIGHT: m_sizeDefault.cx=xlen/3;
			                break;
	       case DOCK_TOP :  m_sizeDefault.cy=ylen/3;
			                break;
	       case DOCK_BOTTOM:m_sizeDefault.cy=ylen/3;
			                break;
	   }
	   m_ProjectSplitterX=m_sizeDefault.cx/2;
	   m_QuerySplitterX=m_sizeDefault.cx/2;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This function will position dockable window sash bar to the sizable edge
// according to the window sizes and docking side.
//---------------------------------------------------------------------------------
void CDockableBrowser::PositionSashBars() {
       
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

void CDockableBrowser::SetSplitteredWindowsPos()  {

	CTabCtrl*          ModeSelector;
    CProjectTree*      Tree;
	CCategoriesCombo*  Category;
	CQueriesCombo*     Query;
	CGroupsCombo*      Group;
	CQueryResults*     Result;


	CButton* QueriesBtn;
	CButton* PrevBtn;
	CButton* FilterBtn;
	CButton* NextBtn;


	ModeSelector=(CTabCtrl*)           GetDlgItem(IDC_MODESELECTOR);
	Tree       = (CProjectTree *)      GetDlgItem(IDC_TREE);
	Category   = (CCategoriesCombo *)  GetDlgItem(IDC_CATEGORIES);
	Query      = (CQueriesCombo *)     GetDlgItem(IDC_QUERIES);
	Group      = (CGroupsCombo *)      GetDlgItem(IDC_GROUPS);

	Result     = (CQueryResults *)     GetDlgItem(IDC_RESULTS);

	QueriesBtn = (CButton *)GetDlgItem(IDC_ENABLEQUERIES);
	FilterBtn  = (CButton *)GetDlgItem(IDC_FILTER);
	PrevBtn    = (CButton *)GetDlgItem(IDC_PREV);
	NextBtn    = (CButton *)GetDlgItem(IDC_NEXT);

	m_sizeDefault.cx-=3;
	m_sizeDefault.cy-=3;

	int resultsStart;
	int resultsEnd;
	if(m_QueriesEnabled==TRUE) {
		resultsStart = m_ProjectSplitterX;
		resultsEnd   = m_sizeDefault.cx;
	} else {
		resultsStart = 0;
		resultsEnd   = m_sizeDefault.cx;
	}

	CRect rect;
	ModeSelector->GetItemRect(0,&rect);
	int tabHeight = rect.Height()+2;


    switch(GetDockSide()) {
	   case DOCK_LEFT : m_ProjectDividerSash.MoveWindow(m_ProjectSplitterX-3,
							                     HEADER+BORDER,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER)-BORDER);

						// Project tree window layout
                        Tree      -> MoveWindow(BORDER+2,
		                                            HEADER+BORDER+2+tabHeight,
		                                            m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                    m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);
                        Category      -> MoveWindow(BORDER+2,
		                                            HEADER+BORDER+2+tabHeight,
		                                            m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                    m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);
                        Query      -> MoveWindow(BORDER+2,
		                                            HEADER+BORDER+2+tabHeight,
		                                            m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                    m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);

                        Group      -> MoveWindow(BORDER+2,
		                                            HEADER+BORDER+2+tabHeight,
		                                            m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                    m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);

                        ModeSelector -> MoveWindow(BORDER,
		                                           HEADER+BORDER,
		                                           m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2,
								                   m_sizeDefault.cy-(HEADER+BORDER)-BORDER);



						// Results window layout
                        Result-> MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
		                                    HEADER+BORDER,
								            (resultsEnd-resultsStart)-SASH_SPACE-2*BORDER-SPLITTER_SPACE/2,
								            m_sizeDefault.cy-(HEADER+BORDER)-BORDER-20
											);

						// Buttons layout
	                    QueriesBtn->MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
							                   m_sizeDefault.cy-BORDER-20,
											   20,
											   20);
	                    PrevBtn->MoveWindow   (resultsStart+SPLITTER_SPACE/2+BORDER+20,
							                   m_sizeDefault.cy-BORDER-20,
											   20,
											   20);
						FilterBtn->MoveWindow(resultsStart+BORDER+SPLITTER_SPACE/2+20+20,
							                  m_sizeDefault.cy-BORDER-20,
											  (resultsEnd-resultsStart)-60-(BORDER+SPLITTER_SPACE/2)-SASH_SPACE-BORDER,
											  20);
						NextBtn->MoveWindow(resultsEnd-SASH_SPACE-20-BORDER,
							                 m_sizeDefault.cy-BORDER-20,
											 20,
											 20);
						break;

	   case DOCK_RIGHT: m_ProjectDividerSash.MoveWindow(m_ProjectSplitterX-3,
							                     HEADER+BORDER,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER)-BORDER);


						// Project tree window layout
                        Tree      -> MoveWindow(BORDER+SASH_SPACE+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-SASH_SPACE-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);
						// Query window layout
                        Category      -> MoveWindow(BORDER+SASH_SPACE+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-SASH_SPACE-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);
                        Query      -> MoveWindow(BORDER+SASH_SPACE+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-SASH_SPACE-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);

                        Group      -> MoveWindow(BORDER+SASH_SPACE+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-SASH_SPACE-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);
                        ModeSelector -> MoveWindow(BORDER+SASH_SPACE,
		                                           HEADER+BORDER,
		                                           m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-SASH_SPACE,
								                   m_sizeDefault.cy-(HEADER+BORDER)-BORDER);


						// Results window layout
                        Result-> MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
		                                    HEADER+BORDER,
								            (resultsEnd-resultsStart)-2*BORDER-SPLITTER_SPACE/2,
								            m_sizeDefault.cy-(HEADER+BORDER)-BORDER-20
											);

						// Buttons layout
	                    QueriesBtn->MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
							                   m_sizeDefault.cy-BORDER-20,
											   20,
											   20);
	                    PrevBtn->MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER+20,
							                   m_sizeDefault.cy-BORDER-20,
											   20,
											   20);
						FilterBtn->MoveWindow(resultsStart+BORDER+SPLITTER_SPACE/2+20+20,
							                  m_sizeDefault.cy-BORDER-20,
											  (resultsEnd-resultsStart)-60-(BORDER+SPLITTER_SPACE/2)-BORDER,
											  20);
						NextBtn->MoveWindow(resultsEnd-20-BORDER,
							                 m_sizeDefault.cy-BORDER-20,
											 20,
											 20);
						break;

	   case DOCK_TOP:  m_ProjectDividerSash.MoveWindow(m_ProjectSplitterX-3,
							                     HEADER+BORDER,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE);


						// Project tree window layout
                        Tree      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4-SASH_SPACE);

						// Query window layout
                        Category      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4-SASH_SPACE);
                        Query      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4-SASH_SPACE);
                        Group      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4-SASH_SPACE);

                        ModeSelector -> MoveWindow(BORDER,
		                                           HEADER+BORDER,
		                                           m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2,
								                   m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE);

						// Results window layout
                        Result-> MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
		                                    HEADER+BORDER,
								            (resultsEnd-resultsStart)-2*BORDER-SPLITTER_SPACE/2,
								            m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE-20
											);

						// Buttons layout
	                    QueriesBtn->MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
							                   m_sizeDefault.cy-BORDER-SASH_SPACE-20,
											   20,
											   20);
	                    PrevBtn->MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER+20,
							                   m_sizeDefault.cy-BORDER-SASH_SPACE-20,
											   20,
											   20);
						FilterBtn->MoveWindow(resultsStart+BORDER+SPLITTER_SPACE/2+20+20,
							                  m_sizeDefault.cy-BORDER-SASH_SPACE-20,
											  (resultsEnd-resultsStart)-60-(BORDER+SPLITTER_SPACE/2)-BORDER,
											  20);
						NextBtn->MoveWindow(resultsEnd-20-BORDER,
							                 m_sizeDefault.cy-BORDER-SASH_SPACE-20,
											 20,
											 20);
						break;
	   case DOCK_BOTTOM:  m_ProjectDividerSash.MoveWindow(m_ProjectSplitterX-3,
							                     HEADER+BORDER+SASH_SPACE,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE);


						// Project tree window layout
                        Tree      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+SASH_SPACE+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE-tabHeight-4);


						// Query window layout
                        Category      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+SASH_SPACE+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE-tabHeight-4);
                        Query      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+SASH_SPACE+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE-tabHeight-4);
                        Group      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+SASH_SPACE+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE-tabHeight-4);

                        ModeSelector -> MoveWindow(BORDER,
		                                           HEADER+BORDER+SASH_SPACE,
		                                           m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2,
								                   m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE);


						// Results window layout
                        Result-> MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
		                                    HEADER+BORDER+SASH_SPACE,
								            (resultsEnd-resultsStart)-2*BORDER-SPLITTER_SPACE/2,
								            m_sizeDefault.cy-(HEADER+BORDER)-BORDER-SASH_SPACE-20
											);

						// Buttons layout
	                    QueriesBtn->MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
							                   m_sizeDefault.cy-BORDER-20,
											   20,
											   20);
	                    PrevBtn->MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER+20,
							                   m_sizeDefault.cy-BORDER-20,
											   20,
											   20);
						FilterBtn->MoveWindow(resultsStart+BORDER+SPLITTER_SPACE/2+40,
							                  m_sizeDefault.cy-BORDER-20,
											  (resultsEnd-resultsStart)-60-(BORDER+SPLITTER_SPACE/2)-BORDER,
											  20);
						NextBtn->MoveWindow(resultsEnd-20-BORDER,
							                 m_sizeDefault.cy-BORDER-20,
											 20,
										     20);
						break;

	   default: 
	                    m_ProjectDividerSash.MoveWindow(m_ProjectSplitterX-3,
							                     HEADER+BORDER,
												 6,
												 m_sizeDefault.cy-(HEADER+BORDER)-BORDER);


						// Project tree window layout
                        Tree      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);


                        Category      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);
						// Query window layout
                        Query      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);
                        Group      -> MoveWindow(BORDER+2,
		                                        HEADER+BORDER+2+tabHeight,
		                                        m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2-4,
								                m_sizeDefault.cy-(HEADER+BORDER)-BORDER-tabHeight-4);

                        ModeSelector -> MoveWindow(BORDER,
		                                           HEADER+BORDER,
		                                           m_ProjectSplitterX-2*BORDER-SPLITTER_SPACE/2,
								                   m_sizeDefault.cy-(HEADER+BORDER)-BORDER);



						// Results window layout
                        Result-> MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
		                                    HEADER+BORDER,
								            (resultsEnd-resultsStart)-2*BORDER-SPLITTER_SPACE/2,
								            m_sizeDefault.cy-(HEADER+BORDER)-BORDER-20
											);

						// Buttons layout
	                    QueriesBtn->MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER,
							                   m_sizeDefault.cy-BORDER-20,
											   20,
											   20);
	                    PrevBtn->MoveWindow(resultsStart+SPLITTER_SPACE/2+BORDER+20,
							                   m_sizeDefault.cy-BORDER-20,
											   20,
											   20);
						FilterBtn->MoveWindow(resultsStart+BORDER+SPLITTER_SPACE/2+40,
							                  m_sizeDefault.cy-BORDER-20,
											  (resultsEnd-resultsStart)-60-(BORDER+SPLITTER_SPACE/2)-BORDER,
											  20);
					
						NextBtn->MoveWindow(resultsEnd-20-BORDER,
							                 m_sizeDefault.cy-BORDER-20,
											 20,
											 20);
						break;
	}
	QueriesBtn->Invalidate(TRUE);
	PrevBtn->Invalidate(TRUE);
	FilterBtn->Invalidate(TRUE);
	NextBtn->Invalidate(TRUE);
	m_sizeDefault.cx+=3;
	m_sizeDefault.cy+=3;
}





//----------------------------------------------------------------------------
// This callback runs every time the user doubleclicked the file in the project
// modules window
//----------------------------------------------------------------------------
void CDockableBrowser::OnFileDoubleclicked(LPCTSTR element)  {
CString serverQuery;
CString filename;
CString viewable;
CString kind;
BSTR query;


    CDataSource* DataSource;
	DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);
	if(DataSource==NULL) return;
	CCodeRoverIntegrator* integrator=(CCodeRoverIntegrator*) GetDlgItem(IDC_CODEROVERINTEGRATORCTRL);
	if(integrator==NULL) return;

    serverQuery.Format("viewable %s", element);
    query=serverQuery.AllocSysString();
    viewable=DataSourceAccessSync(DataSource, &query);
    ::SysFreeString(query);
	viewable.TrimLeft();
	viewable.TrimRight();

    serverQuery.Format("kind %s", element);
    query=serverQuery.AllocSysString();
    kind=DataSourceAccessSync(DataSource, &query);
    ::SysFreeString(query);

	kind.MakeUpper();
	kind.TrimLeft();
	kind.TrimRight();


    if(!(viewable=="0") || kind=="AST" || kind=="INSTANCE") {
        serverQuery.Format("fname %s",element);
	    query=serverQuery.AllocSysString();
	    filename=DataSourceAccessSync(DataSource, &query);
        ::SysFreeString(query);
        integrator->MakeSelection(filename,1,1,0);
    }
    else {
        ((CMainFrame *)AfxGetMainWnd())->MessageBox(
            _T("File cannot be viewed as text"), _T("File View Error"),MB_OK|MB_ICONERROR);
    }
}
//----------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// This callback runs every time the user runs cm command.
//-------------------------------------------------------------------------------------------------
void CDockableBrowser::OnCMEvent(LPCTSTR lfile){
}
//----------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// This function describes selected symbol
//-------------------------------------------------------------------------------------------------
static CString getDescription(CString&  name,
                              CString&  kind,
                              CString&	type,
                              CString&  lang,
                              bool isPublic,
                              bool isProtected,
                              bool isPrivate) {
    CString description="";

	kind.MakeLower();
	lang.MakeUpper();
    
    if(kind.Compare("module")==0) // Module description
         description +=  "[ "+lang+" "+kind+" ]  "+ name;
    else if(kind.Compare("funct")==0) { // function description
        CString descr = "funct";
        if(lang.Compare("ELS")==0 || lang.Compare("JAVA")==0) {
            descr = "method";
        }
        CString access = "";
        if(isPublic) access = "public";
        else if(isProtected) access = "protected";
        else if(isPrivate)   access = "private";
        description += "[ "+access+ " " + descr+ " " +type+" ]  "+ name;
    } else if(kind.Compare("field")==0) { // field description
        CString descr = "field";
        CString access = "";
        if(isPublic) access = "public";
        else if(isProtected) access = "protected";
        else if(isPrivate)   access = "private";
        description += "[ "+access+ " " + descr+ " " +type+" ]  "+ name;
    } else if(kind.Compare("struct")==0) { // struct/class description
        CString descr = "struct";
        if(lang.Compare("CPP")==0 ||
           lang.Compare("ELS")==0 ||
           lang.Compare("JAVA")==0) {
            descr = "class";
        }
        description +=  "[ "+descr+" ]  "+ name;
    } else if(kind.Compare("var")==0 || kind.Compare("local")==0) { // global/local variables
       if((lang.Compare("ELS")==0 || lang.Compare("JAVA")==0) && 
		   kind.Compare("var")==0) {
           description +=  "[static field "+type+"] "+ name;
       } else {
           description +=  "["+kind+ " " +type+"] "+ name;
       }
    } else { // default
       description +=  "[ "+kind+" ]  "+ name;
    }
    return description;
}
//--------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// This callback runs every time the user make Ctrl+Click in the Developer Studio.
// The MSDEV Integrator fire the CtrlClick event to run this callback.
//-------------------------------------------------------------------------------------------------
void CDockableBrowser::IntegratorCtrlClick(LPCTSTR lfile, long line,long col, long size){
CCodeRoverIntegrator* integrator=(CCodeRoverIntegrator*) GetDlgItem(IDC_CODEROVERINTEGRATORCTRL);
CString serverQuery;
CString infoString;
CString param;
CString ffile="";

BSTR q;

	if(integrator==NULL) return;

	CDataSource* DataSource;
	DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);

	// Asking Developer Studio for it's current tab size.
	//int tabsize = integrator->GetTabSize();


	// Getting info string
	unsigned register i;
	if(lfile!=NULL) {
		for(i=0;i<strlen(lfile);i++) {
		    if(lfile[i]=='\\') 
			    ffile+="/";
		    else
			    ffile+=lfile[i];
		}
	}
//	line = integrator->GetModelLine(ffile,ffile,line);
	serverQuery.Format("printformat \"%%s\" etag;set SYMBOL_INSTANCE [get_instance_symbol [set_copy -e %ld,%ld:[lname {%s}]]]",line,col,ffile);
	q=serverQuery.AllocSysString();
	param=DataSourceAccessSync(DataSource, &q);
    ::SysFreeString(q);
	serverQuery = "printformat \"\t%s\t%s\t%s\t%s\t%s\t%s\" name kind type language public protected private;print $SYMBOL_INSTANCE";
	q=serverQuery.AllocSysString();
	param=DataSource->AccessOutput(&q);
    ::SysFreeString(q);
    serverQuery="unset SYMBOL_INSTANCE";
	q=serverQuery.AllocSysString();
	DataSourceAccessSync(DataSource, &q);
    ::SysFreeString(q);

	CString reply;
	if(param[0] == _T('\t')) { // if reply is valid
		param = param.Mid(1);
		// Extracting parameters
		CString name,kind,type,lang;
		bool bPublic,bProtected,bPrivate;
		int nLen = param.GetLength();
		int nStart = 0;
		int nEnd = 0;
		int nField = 0;
		while(nStart < nLen) {
			CString szField;
			nEnd = param.Find(_T('\t'),nStart);
			if(nEnd == -1)
				szField = param.Mid(nStart);
			else
				szField = param.Mid(nStart,nEnd-nStart);
			switch(nField) {
				case 0 : name = szField; break;
				case 1 : kind = szField; break;
				case 2 : type = szField; break;
				case 3 : lang = szField; break;
				case 4 : bPublic = szField.Compare("1")==0; break;
				case 5 : bProtected = szField.Compare("1")==0; break;
				case 6 : bPrivate = szField.Compare("1")==0; break;
			}
			nField++;
			if(nEnd!=-1)
				nStart = nEnd + 1;
			else
				nStart = nLen + 1;
		}
		reply = getDescription(name,kind,type,lang,bPublic,bProtected,bPrivate);
	}
	if(reply.GetLength()>0)  integrator->SetInfo(reply);
	else                     integrator->SetInfo("Can't describe selected symbol");
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Callback runs every time the user clicked "Query" button in the Developer
// Studio. We are extracting symbol from the developer studio into Query list.
//-------------------------------------------------------------------------------------------------
void CDockableBrowser::IntegratorQuery(LPCTSTR lfile, long line,long col, long size){
CCodeRoverIntegrator* integrator=(CCodeRoverIntegrator*) GetDlgItem(IDC_CODEROVERINTEGRATORCTRL);
CString serverQuery;
CString infoString;
CString ffile;

BSTR q;

	if(integrator==NULL) return;
	CDataSource* DataSource;
	DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);

	// Asking Developer Studio for it's current tab size.
//	int tabsize = integrator->GetTabSize();

	unsigned register i;
	if(lfile!=NULL) {
		for(i=0;i<strlen(lfile);i++) {
		    if(lfile[i]=='\\') 
			    ffile+="/";
		    else
			    ffile+=lfile[i];
		}
	}
	// Getting symbol
//	line = integrator->GetModelLine(ffile,ffile,line);
	serverQuery.Format("printformat \"%%s\" etag;set tmp [get_instance_symbol [set_copy -e %ld,%ld:[lname {%s}]]]",line,col,ffile);
//    serverQuery.Format("set tmp [msdev2sym \"%s\" %ld %ld %ld]",ffile, line, col, 1);
	q=serverQuery.AllocSysString();
	CString test;
	test=DataSourceAccessSync(DataSource, &q);
    ::SysFreeString(q);

	BrowseGivenSet(CString("tmp"));

    serverQuery="unset tmp";
	q=serverQuery.AllocSysString();
	DataSourceAccessSync(DataSource, &q);
    ::SysFreeString(q);

    CMainFrame* main;
    main = (CMainFrame *)AfxGetMainWnd();
	main->SetForegroundWindow();
	main->ActivateFrame();
}
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Callback runs every time the user clicked "Definition" button in the Developer
// Studio. We are opening definition of the selected symbol.
//-------------------------------------------------------------------------------------------------
void CDockableBrowser::IntegratorDefinition(LPCTSTR lfile, long line,long col, long size){
register i;
CCodeRoverIntegrator* integrator=(CCodeRoverIntegrator*) GetDlgItem(IDC_CODEROVERINTEGRATORCTRL);
CString serverQuery;
CString ffile;
CString param;
CString filestr;
CString tokenstr;
CString linestr;
CString colstr;

BSTR q;

	if(integrator==NULL) return;
	CDataSource* DataSource;

	DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);

	// Asking Developer Studio for it's current tab size.
	//int tabsize = integrator->GetTabSize();

	if(lfile!=NULL) {
		for(i=0;i<(int)strlen(lfile);i++) {
		    if(lfile[i]=='\\') 
			    ffile+="/";
		    else
			    ffile+=lfile[i];
		}
	}
	// Getting symbol
//	line = integrator->GetModelLine(ffile,ffile,line);
	serverQuery.Format("printformat \"%%s\" etag;set SYMBOL_INSTANCE [get_instance_symbol [set_copy -e %ld,%ld:[lname {%s}]]]",line,col,ffile);
	q=serverQuery.AllocSysString();
	param=DataSourceAccessSync(DataSource, &q);
    ::SysFreeString(q);
	serverQuery = "printformat \"%s\t%s\" fname line;print $SYMBOL_INSTANCE";
	q=serverQuery.AllocSysString();
	param=DataSource->AccessOutput(&q);
    ::SysFreeString(q);

	// Extracting parameters
	int nFileDelim = param.Find(_T('\t'));
	filestr = param.Left(nFileDelim);
	linestr = param.Mid(nFileDelim+1);
	colstr = "-1";
	tokenstr = "";

	// Selecting
	if(filestr.GetLength()!=0) {
       integrator->MakeSelection(filestr,atoi(linestr),atoi(colstr),tokenstr.GetLength());
	} else
		integrator->SetInfo("Can't find selected symbol");

    serverQuery="unset SYMBOL_INSTANCE";
	q=serverQuery.AllocSysString();
	DataSourceAccessSync(DataSource, &q);
    ::SysFreeString(q);
}
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void CDockableBrowser::IntegratorActivate(void){
CMainFrame* main;

    main = (CMainFrame *)AfxGetMainWnd();
	main->SetForegroundWindow();
	main->ActivateFrame();
}
//-------------------------------------------------------------------------------------------------

CString CDockableBrowser::ExecuteCommand(CString & command) {
BSTR q;
CString result;

	CDataSource* DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);
	q=command.AllocSysString();
	result = DataSourceAccessSync(DataSource, &q);
	::SysFreeString(q);
	return result;
}

//-----------------------------------------------------------------------------------------------
// Export elements from the given set into the DiscoverAll and show this set in the browser
//-----------------------------------------------------------------------------------------------
void CDockableBrowser::BrowseGivenSet(CString & name, int col) {
CString query;

   if(name.GetLength()==0) {
	    if(m_CurrentHistoryEntry) {
		   CHistoryEntry entry;
		   entry=m_History.GetAt(m_CurrentHistoryEntry);
		   if(entry.m_Command.GetLength()>0) {
               query.Format("set DiscoverAll [%s]",entry.m_Command);
               CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
               Results->SetQuery(query);
               Results->Calculate();
               Results->SetQuery("");
               Results->Update();
               CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
               Category->Unselect();
               CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
               CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
               main->m_SelectionSize=atoi(sizeStr);
               DisplayInfoString();
		   }
		}
		return;
   }


   if(strnicmp(name,"_DI_",4)==0)
      query.Format("set DiscoverAll [set_copy %s]",name);
   else
      query.Format("set DiscoverAll $%s",name);
   CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);

   Results->SetQuery(query);
   Results->Calculate();
   Results->SetQuery("");
   Results->Update();

   SaveHistory();

   CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
   Category->Unselect();

   CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
   CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
   main->m_SelectionSize=atoi(sizeStr);
   DisplayInfoString();
}
//-----------------------------------------------------------------------------------------------

LRESULT CDockableBrowser::OnServiceDisconnect(WPARAM wparam, LPARAM lparam) {
	CDataSource* DataSource = (CDataSource *) GetDlgItem(IDC_DATASOURCE);
    
	ClearHistory();

	CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
	app->m_ServiceName = "";
	DataSource->SetSource(app->m_ServiceName,TRUE);
	DataSource->SetSource(app->m_ServiceName,FALSE);

	LPDISPATCH lpDispatch;
	DataSource->GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);

	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	Results->SetDataSource(lpDispatch);
	CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
	Category->SetDataSource(lpDispatch);
	CQueriesCombo*    Query = (CQueriesCombo *) GetDlgItem(IDC_QUERIES);
	Query->SetDataSource(lpDispatch);
	CGroupsCombo*    Group = (CGroupsCombo *) GetDlgItem(IDC_GROUPS);
	Group->SetDataSource(lpDispatch);
	CProjectTree* Tree       = (CProjectTree *)    GetDlgItem(IDC_TREE);
	Tree->SetDataSource(lpDispatch);
	return TRUE;
}

//-----------------------------------------------------------------------------------------------
// Callback runs every time the user connect to the different service.
// It will clear old history and will remove all server history variables.
//-----------------------------------------------------------------------------------------------
void CDockableBrowser::OnServiceChanged() {
CMainFrame* main=(CMainFrame*) AfxGetMainWnd();

    HCURSOR save = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
    ClearHistory();

    CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
	CDataSource* DataSource;

	DataSource = (CDataSource *)GetDlgItem(IDC_DATASOURCE);
	DataSource->SetSource(app->m_ServiceName,FALSE);
	LPDISPATCH lpDispatch;
	DataSource->GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);

	main->m_Lic[LIC_ACCESS]         = atoi(ExecuteCommand(CString("license r feature")));
	main->m_Lic[LIC_AUTODOC]        = atoi(ExecuteCommand(CString("license d feature")));
	main->m_Lic[LIC_DORMANT]        = atoi(ExecuteCommand(CString("license v feature")));
	main->m_Lic[LIC_QA]             = atoi(ExecuteCommand(CString("license VI1 feature")));
	main->m_Lic[LIC_QABROWSE]       = atoi(ExecuteCommand(CString("license VI4 feature")));
	main->m_Lic[LIC_QAREPORT]       = atoi(ExecuteCommand(CString("license VI7 feature")));
	main->m_Lic[LIC_SIMPLIFY]       = atoi(ExecuteCommand(CString("license n feature")));
	main->m_Lic[LIC_TASKFLOW]       = atoi(ExecuteCommand(CString("license wa feature")));
	main->m_Lic[LIC_TPM]            = atoi(ExecuteCommand(CString("license wb feature")));
	main->m_Lic[LIC_PACKAGE]        = atoi(ExecuteCommand(CString("license l feature")));

	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	Results->SetFullListName("DiscoverAll");
	Results->SetFilterListName("DiscoverElements");
	Results->SetSelectionName("DiscoverSelection");
	Results->SetDataSource(lpDispatch);

    ExecuteCommand(CString("set_ui_type 1"));
    CString szRes = ExecuteCommand(CString("source_dis \"source.dis\""));
	bool bLicenseError = szRes.IsEmpty() || !szRes.Compare("No licenses available.");
    ExecuteCommand(CString("dis_DL_init"));
    ExecuteCommand(CString("set DiscoverGroups [nil_set]"));
    ExecuteCommand(CString("set DiscoverElements [nil_set]"));
    ExecuteCommand(CString("set DiscoverAll [nil_set]"));
    ExecuteCommand(CString("set DiscoverScope /"));


	CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
	Category->SetDataSource(lpDispatch);

	CQueriesCombo*    Query = (CQueriesCombo *) GetDlgItem(IDC_QUERIES);
	Query->SetDataSource(lpDispatch);

	CGroupsCombo*    Group = (CGroupsCombo *) GetDlgItem(IDC_GROUPS);
	Group->SetDataSource(lpDispatch);
	Group->SetSelectionVariable("DiscoverSelection");
	Group->SetOutputVariable("DiscoverAll");

	main->m_SelectionSize=0;

	CProjectTree* Tree       = (CProjectTree *)    GetDlgItem(IDC_TREE);
	Tree->SetDataSource(lpDispatch);
    DisplayInfoString();


    ::SetCursor(save);
	if(bLicenseError) {
		MessageBox("DIScover was unable to verify your \"Developer License\".\nPlease check your license server, license file\nand the value of LM_LICENSE_FILE, or call customer support","License Error",MB_OK|MB_ICONERROR);
		main->PostMessage(WM_SERVICE_DISCONNECT);
	}
}
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
// This callback runs every time the server sends notification to the client.
// The client must reply or server will wait
//-----------------------------------------------------------------------------------------------
void CDockableBrowser::OnServerNotification(LPCTSTR str)  {
unsigned register i;
int words=0;
CString wordArray[100];
CString curStr;
CPromptDialog dlg;
CQuestionDlg question;
CDataSource* DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);

    CWnd* main = AfxGetMainWnd();


    // Parsing the input string
	for(i=0;i<strlen(str);i++) {
		if(str[i]=='\3') {
			wordArray[words++]=curStr;
			curStr="";
			continue;
		}
		curStr+=str[i];
	}
	if(curStr.GetLength()>0) wordArray[words++]=curStr;


	//--------- "add_menu" notification processing 
	if(wordArray[0]==CString("add_menu")) {
         ((CMainFrame *)main)->AddAccessMenu(wordArray[1],wordArray[2]);
         ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
         DataSource->AcceptNotification("*");
		 return;
	}
	//---------------------------------------------



	//--------- "browser" notification processing
	if(wordArray[0]==CString("browser")) {
		 // If adding menu item
         DataSource->AcceptNotification("*");
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
	         if(wordArray[4]==CString("-opt") || wordArray[4]==CString("-optional")) {
				   accessCmds="set tmp [nil_set]; sappend $tmp $DiscoverSelection;";
			       accessCmds+=wordArray[3];
				   accessCmds+=" $tmp;";
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
				                     accessCmds+=" $tmp;";
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
		 if(wordArray[1].GetLength()>0) {
			 CString visibleItem;
			 visibleItem="";
			 for(int i=0;i<wordArray[2].GetLength();i++) {
				 if(wordArray[2][i]=='&') visibleItem+='&';
				 visibleItem+=wordArray[2][i];
			 }
             ((CMainFrame *)main)->AddAccessMenuItem(wordArray[1],visibleItem,accessCmds);
             ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
		 }
         DataSource->AcceptNotification("*");
		 return;
	}
	//------------------------------------------------------------------

	//------"prompt" notification processing
	if(wordArray[0]==CString("prompt")) {
		 if(words==2 && (wordArray[1]!=CString("-title"))) {
			if(wordArray[1]==CString("printformat")) {
				CQueryResults* pResults =(CQueryResults *)GetDlgItem(IDC_RESULTS);
				CString szPrintformat = pResults->GetPrintformat();
				DataSource->AcceptNotification(szPrintformat);
				return;
			} else
				dlg.m_PromptText=wordArray[1];
		 } else {
		     if((words>=4) && (wordArray[1]==CString("-title"))) {
			     dlg.SetPromptTitle(wordArray[2]);
			     dlg.m_PromptText=wordArray[3];
			 } else {
					DataSource->AcceptNotification("");
					return;
			 }
		 }
		 dlg.m_UserText="";
		 dlg.m_DataSource=DataSource;
	     dlg.DoModal();
		 return;
	}
	//------------------------------------------------------------------------



    //----- "remove menu" notification processing ------------------------------
	if((words==2) && (wordArray[0]==CString("remove_menu"))) {
       ((CMainFrame *)main)->RemoveAccessMenu(wordArray[1]);
       ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
       DataSource->AcceptNotification("*");
	   return;
	}
	//---------------------------------------------------------------------------


	//----- "remove item" notification processing -------------------------------
	if((words==3) && (wordArray[0]==CString("remove_item"))) {
       ((CMainFrame *)main)->RemoveAccessMenuItem(wordArray[1],wordArray[2]);
       ((CMainFrame *)main)->SetNewMenu(((CMainFrame *)main)->GetBaseMenuResource());
       DataSource->AcceptNotification("*");
	   return;
	}
	//-----------------------------------------------------------------------------

	//----- "dis_message" notification processing -------------------------------
	if((words>0) && (wordArray[0]==CString("error"))) {
	   ((CMainFrame *)main)->MessageBox(wordArray[1],"Server error",MB_OK|MB_ICONERROR);
       DataSource->AcceptNotification("*");
	   return;
	}
	if((words>0) && (wordArray[0]==CString("stderror"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       DataSource->AcceptNotification("*");
	   return;
	}
	if((words>0) && (wordArray[0]==CString("warning"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       DataSource->AcceptNotification("*");
	   return;
	}
	if((words>0) && (wordArray[0]==CString("warning"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       DataSource->AcceptNotification("*");
	   return;
	}
	if((words>0) && (wordArray[0]==CString("status"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       DataSource->AcceptNotification("*");
	   return;
	}
	if((words>0) && (wordArray[0]==CString("info"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       DataSource->AcceptNotification("*");
	   return;
	}
	if((words>0) && (wordArray[0]==CString("diag"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       DataSource->AcceptNotification("*");
	   return;
	}
	if((words>0) && (wordArray[0]==CString("default"))) {
	   ((CMainFrame *)main)->m_wndStatusBar.SetPaneText(0,wordArray[1]);
       DataSource->AcceptNotification("*");
	   return;
	}
	//-----------------------------------------------------------------------------


	//-----------------------------------------------------------------------------
	// "question" notification processing
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
	   question.m_DataSource=DataSource;
	   question.DoModal();
	   return;
	}
	//-----------------------------------------------------------------------------

	if(wordArray[0]==CString("stop")) {
        DataSource->AcceptNotification("*");

		AfxGetMainWnd()->SendMessage(WM_SERVICE_DISCONNECT);
		return;
	}

	if(wordArray[0]==CString("open_window")) {
         DataSource->AcceptNotification("*");
	 return;
	}

	if(wordArray[0]==CString("list_selection") && words >2) {
		 CListSelDlg dlg;
		 dlg.SetTitle(wordArray[1]);
		 dlg.SetList(wordArray[2]);
		 if(dlg.DoModal()==IDOK)
              DataSource->AcceptNotification(dlg.m_Selection);
		 else
              DataSource->AcceptNotification("");
		 return;
	}

	if(wordArray[0] == CString("edit_file") && words == 2) {
		ViewTextFile(wordArray[1]);
		DataSource->AcceptNotification("");
		return;
	}

	if(words >= 2 && wordArray[0] == CString("cm_cmd")) {
		DoCMAction(wordArray[1], words - 2, (words > 2 ? wordArray + 2 : NULL));
		DataSource->AcceptNotification("");
		return;
	}

	// accepting for all unknown notifications
    DataSource->AcceptNotification("*");
}
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
// This callback will run when "browser" notification comes. It is not in the notification
// callback to avoid deadlock.
//-----------------------------------------------------------------------------------------------
void CDockableBrowser::SetBrowserVariable(void) {
   BrowseGivenSet(CString("notification_var_lock"),browserSetType);
   ExecuteCommand(CString("unset notification_var_lock"));
}
//-----------------------------------------------------------------------------------------------


void CDockableBrowser::OnQueriesEnable(void) {
	CTabCtrl*           ModeSelector;
    CProjectTree*       Tree;
	CCategoriesCombo*   Category;
	CQueriesCombo*      Query;
	CGroupsCombo*       Group;

	ModeSelector=(CTabCtrl*)           GetDlgItem(IDC_MODESELECTOR);
	Tree       = (CProjectTree *)      GetDlgItem(IDC_TREE);
	Category   = (CCategoriesCombo *)  GetDlgItem(IDC_CATEGORIES);
	Query      = (CQueriesCombo *)     GetDlgItem(IDC_QUERIES);
	Group      = (CGroupsCombo *)      GetDlgItem(IDC_GROUPS);

	if(m_QueriesEnabled==TRUE) {
		m_QueriesEnabled=FALSE;
	    m_ProjectDividerSash.ShowWindow(SW_HIDE);
	    ModeSelector -> ShowWindow(SW_HIDE);
	    Tree         -> ShowWindow(SW_HIDE);
	    Category     -> ShowWindow(SW_HIDE);
	    Query        -> ShowWindow(SW_HIDE);
	    Group        -> ShowWindow(SW_HIDE);
	} else {
		m_QueriesEnabled=TRUE;
	    m_ProjectDividerSash.ShowWindow(SW_SHOW);
	    ModeSelector -> ShowWindow(SW_SHOW);
	    switch(ModeSelector->GetCurSel()) {
	        case 0 : Tree->ShowWindow(SW_SHOW);
				     break;
	        case 1 : Category->ShowWindow(SW_SHOW);
				     break;
	        case 2 : Query->ShowWindow(SW_SHOW);
				     break;
	        case 3 : Group->ShowWindow(SW_SHOW);
				     break;
		}
	}
    SetSplitteredWindowsPos();
}

void CDockableBrowser::OnUpdateQueriesBtn(CCmdUI* pCmdUI)  {
   pCmdUI->Enable(TRUE);	
}

void CDockableBrowser::OnQueryModeChanged(NMHDR* pNMHDR, LRESULT* pResult)  {
	CTabCtrl*          ModeSelector;
    CProjectTree*      Tree;
	CCategoriesCombo*  Category;
	CQueriesCombo*     Query;
	CGroupsCombo*      Group;

	ModeSelector=(CTabCtrl*)           GetDlgItem(IDC_MODESELECTOR);
	Tree       = (CProjectTree *)      GetDlgItem(IDC_TREE);
	Category   = (CCategoriesCombo *)  GetDlgItem(IDC_CATEGORIES);
	Query      = (CQueriesCombo *)     GetDlgItem(IDC_QUERIES);
	Group      = (CGroupsCombo *)      GetDlgItem(IDC_GROUPS);

	m_CurrentMode=ModeSelector->GetCurSel();
	switch(m_CurrentMode) {
	    case 0 : Tree->ShowWindow(SW_SHOW);
			     Category->ShowWindow(SW_HIDE);
			     Query->ShowWindow(SW_HIDE);
			     Group->ShowWindow(SW_HIDE);
				 break;
	    case 1 : Tree->ShowWindow(SW_HIDE);
			     Category->ShowWindow(SW_SHOW);
			     Query->ShowWindow(SW_HIDE);
			     Group->ShowWindow(SW_HIDE);
				 break;
	    case 2 : Tree->ShowWindow(SW_HIDE);
			     Category->ShowWindow(SW_HIDE);
			     Query->ShowWindow(SW_SHOW);
			     Group->ShowWindow(SW_HIDE);
				 break;
	    case 3 : Tree->ShowWindow(SW_HIDE);
			     Category->ShowWindow(SW_HIDE);
			     Query->ShowWindow(SW_HIDE);
				 Group->Update();
			     Group->ShowWindow(SW_SHOW);
				 break;
	}
}



//-----------------------------------------------------------------------------------------
// This private function will analyse the categories list with {} and will form 
// category reuest command saving it in m_CategoryRequest member string.
//-----------------------------------------------------------------------------------------
void CDockableBrowser::FormCategoryRequest(CString& categoriesList) {
register i;
CString oneQuery;
CString category;

   CCategoriesCombo*      Category;
   Category=(CCategoriesCombo*) GetDlgItem(IDC_CATEGORIES);
   if(categoriesList.GetLength()==0) {
       m_CategoryRequest="";
	   return;
   }

   // Forming the complex command with multiple categories
   // from the {} separated categories list.
   BOOL first = TRUE;
   i=0;
   while(categoriesList[i]==' ') i++;
   for(;i<categoriesList.GetLength();i++) {
      if(categoriesList[i]=='{') {
	     category="";
		 continue;
	  }
	  if(categoriesList[i]=='}'){
		 // will return the Access equivalent to the readable category name
	     oneQuery = Category->AccessFromName(category);
		 if(first==TRUE) {
		    m_CategoryRequest=oneQuery;
			first=FALSE;
		 } else {
			int pos = oneQuery.Find('-');
			if(pos!=-1) {
				m_CategoryRequest+=' ';
				m_CategoryRequest+=oneQuery.GetBuffer(2)+pos;
			}
		 }
		 continue;
	  }
	  category+=categoriesList[i];
   } 
 }
//-----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This callback will run every time the user changed the selected categories list. This 
// callback will convert the readable categories list receved from the categories
// ActiveX into the access commands (this is done by FormCategoryRequest function), will
// fill the results window withthe results and will pass the categories list to the 
// queries Active X control to let it form a query subset based on the selected 
// categories.
//----------------------------------------------------------------------------------------
void CDockableBrowser::OnCategoryChanged(LPCTSTR categoriesList) {
CString query;

   HCURSOR save = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
   m_CurrentCategory = categoriesList;
   FormCategoryRequest(CString(categoriesList));
   query.Format("%s %s",m_CategoryRequest, m_CurrentProject);
   CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
   Results->SetQuery(query);
   Results->Update();
   CQueriesCombo*      Query;
   Query=(CQueriesCombo*) GetDlgItem(IDC_QUERIES);
   Query->SetSelection("");
   Query->SetCategories(categoriesList);
   DisplayInfoString();
   CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
   CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
   main->m_SelectionSize=atoi(sizeStr);
   SaveHistory();
   ::SetCursor(save);
}
//----------------------------------------------------------------------------------------




//----------------------------------------------------------------------------
// This callbac will run every time the user changed the node in the project 
// tree. We need to inform our browsers about this.
//----------------------------------------------------------------------------
void CDockableBrowser::OnNodeChanged(LPCTSTR projectName)  {
    HCURSOR save = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
    CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
	Category->Unselect();
	m_CurrentProject=projectName;
	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	CString query;
	query.Format("modules %s",m_CurrentProject);
	Results->SetQuery(query);
	Results->Update();
    DisplayInfoString();
    CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
    CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
    main->m_SelectionSize=atoi(sizeStr);
    SaveHistory(query);
	::SetCursor(save);
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// This callback will run every time the user selects something in the
// results list. All user selection will go to the "DiscoverSelection"
// server variable. This callback will set m_SelectionAvailable flag to TRUE
// and all queries will use current selection instead of current project.
//----------------------------------------------------------------------------
void CDockableBrowser::OnSelectionChanged(LPCTSTR pstrParams) {
   CQueriesCombo*      Query;
   Query=(CQueriesCombo*) GetDlgItem(IDC_QUERIES);
   Query->Clear();
   Query->SetSelection("DiscoverSelection");
   CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
   CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
   main->m_SelectionSize=atoi(sizeStr);
   DisplayInfoString();
   main->m_wndStatusBar.SetPaneText(0,pstrParams);
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void CDockableBrowser::OnElementDoubleclicked(LPCTSTR parmsList) {

	ViewFile(CString(parmsList));
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// This function will create menu map from the current query string.
//----------------------------------------------------------------------------
void CDockableBrowser::MakeMenuMap() {
CString queryName;
register i;

	m_MenuMap.RemoveAll();
    CQueriesCombo*      Query;
    Query=(CQueriesCombo*) GetDlgItem(IDC_QUERIES);
	CString list = Query->GetQueriesList();
	int count=IDC_QUERIES_START;
	BOOL in=FALSE;
	for(i=0;i<list.GetLength();i++) {
		if(list[i]=='{') {
			in=TRUE;
			continue;
		}
		if(list[i]=='}') {
			in=FALSE;
			queryName.TrimLeft();
			queryName.TrimRight();
			CMenuMapEntry entry;
			entry.m_ID=count;
			entry.m_Query=queryName;
			m_MenuMap.AddTail(entry);
			queryName="";
			count++;
			continue;
		}
		if(in) queryName+=list[i];
	}
}
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
// Pop-up menu will appear on right mouse button click
//----------------------------------------------------------------------------
void CDockableBrowser::OnRButtonEvent(short x,short y) {
CPoint point(x,y);
CMenu popup;
CMenu query;
CMenu uses;
CMenu where;
CMenu show;
CMenu defines;
int   usesCount=0;
int   whereCount=0;
int   showCount=0;
int   definesCount=0;
CMainFrame* mainFrm=(CMainFrame*) AfxGetMainWnd();

    // This function will make menu map - assosiations between command IDs and
    // queries names.
    MakeMenuMap();

	// Base menu
	popup.CreatePopupMenu();

    CCMIntegrator* pIntegrator=(CCMIntegrator*) GetDlgItem(IDC_CMINTEGRATORCTRL);
	if(pIntegrator->IsConnected()) {
		CreateCMMenu(&m_pCMMenu);
		if(m_pCMMenu!=NULL) {
			popup.AppendMenu(MF_POPUP,(unsigned int)m_pCMMenu->m_hMenu,"Source Control");
			popup.AppendMenu(MF_SEPARATOR);
		}
	}

    // Query pop-up menu
	query.CreatePopupMenu();
	popup.AppendMenu(MF_POPUP,(unsigned int)query.m_hMenu,"Query");


	// Query sub-menus
	uses.CreatePopupMenu();
	where.CreatePopupMenu();
	show.CreatePopupMenu();
	defines.CreatePopupMenu();


	// Filling query sub-menus
	POSITION pos;
	pos=m_MenuMap.GetHeadPosition();
	while(pos) {
		CString name;
		int id;
		name = m_MenuMap.GetAt(pos).m_Query;
		id   = m_MenuMap.GetAt(pos).m_ID;
		if(name.Find("Uses")==0) {
	        uses.AppendMenu(MF_STRING,id,name);
			usesCount++;
		    m_MenuMap.GetNext(pos);
			continue;
		}
		if(name.Find("Where")==0) {
	        where.AppendMenu(MF_STRING,id,name);
			whereCount++;
		    m_MenuMap.GetNext(pos);
			continue;
		}
		if(name.Find("Defines")==0) {
	        defines.AppendMenu(MF_STRING,id,name);
			definesCount++;
		    m_MenuMap.GetNext(pos);
			continue;
		}
		if(name.Find("Show")==0) {
	        show.AppendMenu(MF_STRING,id,name);
			showCount++;
		    m_MenuMap.GetNext(pos);
			continue;
		}
		m_MenuMap.GetNext(pos);
    }
	// Attaching query sub-menus if not empty
	if(usesCount>0)    query.AppendMenu(MF_POPUP,(unsigned int)uses.m_hMenu,"Uses");
	if(whereCount>0)   query.AppendMenu(MF_POPUP,(unsigned int)where.m_hMenu,"Where");
	if(definesCount>0) query.AppendMenu(MF_POPUP,(unsigned int)defines.m_hMenu,"Defines");
	if(showCount>0)    query.AppendMenu(MF_POPUP,(unsigned int)show.m_hMenu,"Show");


	// Adding all queries not in the submenus
	pos=m_MenuMap.GetHeadPosition();
	while(pos) {
		CString name;
		int id;
		name = m_MenuMap.GetAt(pos).m_Query;
		id   = m_MenuMap.GetAt(pos).m_ID;
		if(name.Find("Uses")==0) {
		    m_MenuMap.GetNext(pos);
			continue;
		}
		if(name.Find("Where")==0) {
		    m_MenuMap.GetNext(pos);
			continue;
		}
		if(name.Find("Defines")==0) {
		    m_MenuMap.GetNext(pos);
			continue;
		}
		if(name.Find("Show")==0) {
		    m_MenuMap.GetNext(pos);
			continue;
		}
	    query.AppendMenu(MF_STRING,id,name);
		m_MenuMap.GetNext(pos);
    }
	popup.AppendMenu(MF_STRING,IDC_CLOSURE,"Closure");
	if(m_Closure==TRUE)
	    popup.CheckMenuItem(IDC_CLOSURE,MF_BYCOMMAND|MF_CHECKED);
    else
	    popup.CheckMenuItem(IDC_CLOSURE,MF_BYCOMMAND|MF_UNCHECKED);

	popup.AppendMenu(MF_STRING,IDC_SHORTNAMES,"Short names");
	CQueryResults* list  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	if(list->GetShortNames()==TRUE)
	    popup.CheckMenuItem(IDC_SHORTNAMES,MF_BYCOMMAND|MF_CHECKED);
    else
	    popup.CheckMenuItem(IDC_SHORTNAMES,MF_BYCOMMAND|MF_UNCHECKED);
	popup.AppendMenu(MF_SEPARATOR);

	// Filter items
	popup.AppendMenu(MF_STRING,IDC_SELECTFILTER,"Select Filter ...");
    CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
    mainFrm->m_SelectionSize=atoi(sizeStr);
	popup.AppendMenu(MF_STRING,IDC_RESETFILTER, "Clear Filter");
	popup.AppendMenu(MF_SEPARATOR);

	// Attribute and sorting items
	popup.AppendMenu(MF_STRING,IDC_SELATTRIBUTES,"Select Attributes ...");
	popup.AppendMenu(MF_STRING,IDC_SELECTSORT, "Select sorting ...");
	popup.AppendMenu(MF_SEPARATOR);

	// Export item
	CMenu exportPopup;
    // Export pop-up menu
	exportPopup.CreatePopupMenu();
	popup.AppendMenu(MF_POPUP,(unsigned int)exportPopup.m_hMenu,"Export to ...");
	
	exportPopup.AppendMenu(MF_STRING,IDC_ADDELEMENTS, "Group...");
	exportPopup.AppendMenu(MF_STRING,IDC_SAVEELEMENTS, "Text file...");

	// coordinates calculation to position menu window properly
	CRect main;
	CRect child;
	GetWindowRect(main);
	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	Results->GetWindowRect(child);
	point.y+=child.top-main.top;
	point.x+=child.left-main.left;
	ClientToScreen(&point);

	// selection loop
    popup.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this);


	// destroy all menus and free handles
	popup.DestroyMenu();
	query.DestroyMenu();
	where.DestroyMenu();
	uses.DestroyMenu();
	defines.DestroyMenu();
	show.DestroyMenu();
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// This callback will run if command id is inside queries range.
// It will search in the menu map to find the query string with this ID.
//----------------------------------------------------------------------------
void CDockableBrowser::OnMenuQuery(int id) {
CString query;
POSITION pos;

	// Searching for the query in the menu map
	pos=m_MenuMap.GetHeadPosition();
	while(pos) {
		if(m_MenuMap.GetAt(pos).m_ID==id) break;
		m_MenuMap.GetNext(pos);
    }
	// if in the menu map
	if(pos) {
	   CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
	   Category->Unselect();
	   query="{";
	   query+=m_MenuMap.GetAt(pos).m_Query;
	   query+="}";
       FormQueryRequest(query);
	   CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	   Results->SetQuery(m_QueryRequest);
	   Results->Update();
       CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
       CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
       main->m_SelectionSize=atoi(sizeStr);
       DisplayInfoString();
       SaveHistory();
	}
}
//----------------------------------------------------------------------------




//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Select Attributes..." item in the 
// pop-up menu. This function will show the attributes dialog.
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnSetAttributes(void) {
	CAttributesDialog AttributesDialog(NULL);
	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	AttributesDialog.SetApplyTaget(Results);
	AttributesDialog.DoModal();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Select Filter..." item in the 
// pop-up menu. This function will show the filter wizard.
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnSetFilter(void) {
FilterSheet filter("Filter Setup",this);

	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	filter.SetApplyTaget(Results); 
	filter.DoModal();
    CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
    CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
    main->m_SelectionSize=atoi(sizeStr);
    DisplayInfoString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Select Filter..." item in the 
// pop-up menu. This function will show the filter wizard.
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnAddElements(void) {
CAddSelectorDlg add("Elements Export",this);

	add.DoModal();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Export to..."/"Text File..." item in the 
// pop-up menu. This function will show the "Save as..." dialog.
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnSaveElements(void) {
	CFileDialog saveDlg(FALSE,"htm","Result",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					"Text files (*.txt) |*.txt |Comma delimited (*.csv) |*.csv ||",
					this);
	if(saveDlg.DoModal()==IDOK) {
		CString to;
		to = saveDlg.GetPathName();
		CQueryResults* pResults = (CQueryResults *)GetDlgItem(IDC_RESULTS);
		if(pResults!=NULL) {
			char cDelim;
			if(to.Right(4) == ".txt") {
				cDelim='\t';
			} else {
				cDelim=',';
			}
			pResults->SaveSelectionTo(to,cDelim);
		}
	}	
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Reset Filter" item in the 
// pop-up menu. This function will remove any applied filter.
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnResetFilter(void)  {
	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
    Results->SetFilter("");
    Results->Update();
    CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
    CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
    main->m_SelectionSize=atoi(sizeStr);
    DisplayInfoString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Select Sort..." item in the 
// pop-up menu. This function will show the Sort Selector dialog.
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnSelectSort(void) {
	CComplexSort sort(this);
	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	sort.SetApplyTaget(Results);
	sort.DoModal();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Forms the complete query from the selected queries and selected project lists or 
// selection variable.
//-----------------------------------------------------------------------------------------
void  CDockableBrowser::FormQueryRequest(CString& queriesList) {
register  i;
CString query;
CMainFrame* main=(CMainFrame*) AfxGetMainWnd();

    CQueriesCombo*      Query;
    Query=(CQueriesCombo*) GetDlgItem(IDC_QUERIES);

	m_QueryRequest="set tmp [nil_set];";
    // Forming the complex command with multiple queries
	i=0;
	queriesList.TrimLeft();
	for(;i<queriesList.GetLength();i++) {
	   if(queriesList[i]=='{') {
		  query="";
		  continue;
	   }
	   // Query end. Forming the results command and adding it to the
	   // summarized query.
	   if(queriesList[i]=='}'){
	      // Access command from the query name
		  CString oneQuery;
		  oneQuery = Query->AccessFromName(query);

          // Access appending command with the arguments
		  CString merge;

		  // If argument variable is not set we will use projects list as an argument.
		  if(main->m_SelectionSize>0) {
			  if(m_Closure==TRUE) {
		          merge.Format("sappend $tmp [query_closure 0 {%s} $DiscoverSelection];",oneQuery);
			  } else {
		          merge.Format("sappend $tmp [%s $DiscoverSelection];",oneQuery);
			  }
		  } else {
		      merge.Format("sappend $tmp [%s %s];",oneQuery,m_CurrentProject);
		  }
		  // Results command
		  m_QueryRequest+=merge;
		  continue;			 
	   }
	   query+=queriesList[i];
	}
	query.Format("list $tmp");
	m_QueryRequest+=query;
}
//-----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This callback will run every time the user select some query in the browser. It will
// use DiscoverSelection server variable as an query argument if something selected
// and m_SelectionAvailable flag is set to TRUE or current project if no selection 
// available. It will use "FormQueryRequest" function to form complex query from the
// given queries names.
//----------------------------------------------------------------------------------------
void CDockableBrowser::OnQueryChanged(LPCTSTR parmsList) {
CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
CQueriesCombo* Query = (CQueriesCombo *) GetDlgItem(IDC_QUERIES);
CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);

    if(main->m_SelectionSize>0) {
        HCURSOR save = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
        Category->Unselect();
	    m_CurrentQuery=parmsList;
	    FormQueryRequest(CString(parmsList));
	    Results->SetQuery(m_QueryRequest);
	    Results->Update();
        CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
        main->m_SelectionSize=atoi(sizeStr);
        DisplayInfoString();
        SaveHistory();
		::SetCursor(save);
	} else Query->Clear();
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// Enables or disables "Previous" button in the history panel
//----------------------------------------------------------------------------------------
void CDockableBrowser::OnUpdatePrev(CCmdUI* pCmdUI)  {
POSITION pos = m_CurrentHistoryEntry;
    if(pos)  {
		if(m_ResultWasEmpty==FALSE) m_History.GetPrev(pos);
		if(pos) {
		    pCmdUI->Enable(TRUE);	
		} else pCmdUI->Enable(FALSE);
	} else pCmdUI->Enable(FALSE);
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Enables or disables "Next" button in the history panel
//----------------------------------------------------------------------------------------
void CDockableBrowser::OnUpdateNext(CCmdUI* pCmdUI)  {
POSITION pos = m_CurrentHistoryEntry;
    if(pos)  {
		m_History.GetNext(pos);
		if(pos) {
		    pCmdUI->Enable(TRUE);	
		} else pCmdUI->Enable(FALSE);
	} else pCmdUI->Enable(FALSE);
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// Enables or disables "Previous" button in the history panel
//----------------------------------------------------------------------------------------
void CDockableBrowser::OnUpdateFilter(CCmdUI* pCmdUI)  {
	pCmdUI->Enable(TRUE);
}
//----------------------------------------------------------------------------------------


	
//----------------------------------------------------------------------------------------
// Saves current results and browser status in the history
//----------------------------------------------------------------------------------------
void CDockableBrowser::SaveHistory(CString command) {
CString serverVarName;

	CString result;
	CString cmd;

	cmd="size $DiscoverAll";
	result=ExecuteCommand(cmd);
	int size = atoi(result);
	m_ResultWasEmpty=FALSE;
	if(size==0) {
		m_ResultWasEmpty=TRUE;
		return;
	}

	if(m_InHistory>=30) m_History.RemoveHead();
	else                m_InHistory++;

	// Setting server history variable
	serverVarName.Format("__discover_history_%d__",m_VarIndex);
	if(command.GetLength()==0)
	    cmd.Format("set %s $DiscoverAll",serverVarName);
	else
	    cmd.Format("set %s [nil_set]",serverVarName);

	ExecuteCommand(cmd);

	// Filling client history entry
	CHistoryEntry entry;
	entry.m_Mode           = m_CurrentMode;
	entry.m_ServerVariable = serverVarName;
	entry.m_ProjectName    = m_CurrentProject;
	entry.m_CategoryName   = m_CurrentCategory;
	entry.m_QueryName      = m_CurrentQuery;
	entry.m_GroupName      = m_CurrentGroup;
	entry.m_Command        = command;

	int count = 0;
	POSITION pos;
    pos=m_CurrentHistoryEntry;
	if(pos) m_History.GetNext(pos);
	while(pos) {
		CHistoryEntry entry;
		entry=m_History.GetAt(pos);
		cmd.Format("unset %s",entry.m_ServerVariable);
		ExecuteCommand(cmd);
		count++;
		m_History.GetNext(pos);
	}
	for(int i=0;i<count;i++) m_History.RemoveTail();
	m_CurrentHistoryEntry  = m_History.AddTail(entry);
    m_VarIndex=++m_VarIndex%30;
};
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This function will remove all elements from the history list and will unset all server
// variables
//----------------------------------------------------------------------------------------
void CDockableBrowser::ClearHistory(void) {
POSITION pos;
    pos=m_History.GetHeadPosition();
	while(pos) {
		CHistoryEntry entry;
		entry=m_History.GetAt(pos);
		CString command;
		command.Format("unset %s",entry.m_ServerVariable);
		ExecuteCommand(command);
		m_History.GetNext(pos);
	}
	m_History.RemoveAll();
	m_CurrentHistoryEntry=NULL;
	m_InHistory=0;
	m_VarIndex=0;
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Sets the current results and browser set from history - previous history element
//----------------------------------------------------------------------------------------
void CDockableBrowser::PrevHistory(void) {
POSITION pos = m_CurrentHistoryEntry;

    if(m_ResultWasEmpty==FALSE) m_History.GetPrev(pos);
	m_ResultWasEmpty=FALSE;
	if(pos) {

        CQueriesCombo* Query=(CQueriesCombo*) GetDlgItem(IDC_QUERIES);
        Query->Clear();
	    CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
	    Category->Unselect();

		m_CurrentHistoryEntry=pos;
		CHistoryEntry entry;
		entry=m_History.GetAt(pos);

	    CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
        CString command;
		if(entry.m_Command.GetLength()==0) {
		    command.Format("set DiscoverAll $%s",entry.m_ServerVariable);
		} else {
			command = entry.m_Command;
		}
        Results->SetQuery(command);
        Results->Calculate();
        Results->SetQuery("");
        Results->SetFilter("");
        Results->Update();
        ExecuteCommand(CString("set DiscoverSelection [nil_set]"));
        CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
        CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
        main->m_SelectionSize=atoi(sizeStr);
        DisplayInfoString();
	}
};
//----------------------------------------------------------------------------------------




//----------------------------------------------------------------------------------------
// Sets the current results and browser set from history - next history element
//----------------------------------------------------------------------------------------
void CDockableBrowser::NextHistory(void) {
POSITION pos = m_CurrentHistoryEntry;

    m_History.GetNext(pos);
	if(pos) {
		m_CurrentHistoryEntry=pos;
		CHistoryEntry entry;
		entry=m_History.GetAt(pos);

	    CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
	    Category->Unselect();
        CQueriesCombo* Query=(CQueriesCombo*) GetDlgItem(IDC_QUERIES);
        Query->Clear();

	    CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
		CString command;
		if(entry.m_Command.GetLength()==0) {
		    command.Format("set DiscoverAll $%s",entry.m_ServerVariable);
		} else {
			command = entry.m_Command;
		}

        Results->SetQuery(command);
        Results->Calculate();
        Results->SetQuery("");
        Results->SetFilter("");
        Results->Update();

        ExecuteCommand(CString("set DiscoverSelection [nil_set]"));
        CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
        CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
        main->m_SelectionSize=atoi(sizeStr);
        DisplayInfoString();
	}
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Set information string as a filter button title
//----------------------------------------------------------------------------------------
void CDockableBrowser::DisplayInfoString(void) {
CString info;
    
    FillKindList();
	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	info.Format("%d from %d, %d selected",Results->FilteredAmount(),
		                                  Results->RowAmount(),
					                      Results->AmountSelected());
	CButton* FilterBtn  = (CButton *)GetDlgItem(IDC_FILTER);
	FilterBtn->SetWindowText(info);

}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This callback runs every time the user changes the selection in the groups window.
// It will fill the results window with the groups elements.
//-----------------------------------------------------------------------------------------
void  CDockableBrowser::OnGroupChanged(LPCTSTR groupsList) {
   CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
   Category->Unselect();
   CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
   Results->SetQuery("");
   Results->Update();
   ExecuteCommand(CString("set DiscoverGroups $DiscoverAll"));
   CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
   CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
   main->m_SelectionSize=atoi(sizeStr);
   SaveHistory();
   DisplayInfoString();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This callback runs every time the user create new group
//-----------------------------------------------------------------------------------------
void  CDockableBrowser::OnGroupCreated(LPCTSTR groupsList) {
   CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
   Results->SetQuery("");
   Results->Update();
   ExecuteCommand(CString("set DiscoverGroups $DiscoverAll"));
   SaveHistory();
   CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
   CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
   main->m_SelectionSize=atoi(sizeStr);
   DisplayInfoString();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This callback will run every time the user removed group(s). Passes the amount of groups
// removed.
//-----------------------------------------------------------------------------------------
void  CDockableBrowser::OnGroupRemoved(short amount) {
   CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
   Results->SetQuery("");
   Results->Update();
   ExecuteCommand(CString("set DiscoverGroups $DiscoverAll"));
   SaveHistory();
   CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
   CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
   main->m_SelectionSize=atoi(sizeStr);
   DisplayInfoString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This callback runs every time the user insert elements into the group
//-----------------------------------------------------------------------------------------
void  CDockableBrowser::OnGroupInsSel(void) {
   CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
   Results->SetQuery("");
   Results->Update();
   SaveHistory();
   ExecuteCommand(CString("set DiscoverGroups $DiscoverAll"));
   CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
   CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
   main->m_SelectionSize=atoi(sizeStr);
   DisplayInfoString();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This callback runs every time the user removes elements from the group
//-----------------------------------------------------------------------------------------
void  CDockableBrowser::OnGroupDelSel(void) {
   CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
   Results->SetQuery("");
   Results->Update();
   ExecuteCommand(CString("set DiscoverGroups $DiscoverAll"));
   SaveHistory();
   CString sizeStr = ExecuteCommand(CString("size $DiscoverSelection"));
   CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
   main->m_SelectionSize=atoi(sizeStr);
   DisplayInfoString();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This function will inspect current selection and will form the kinds list.
//-----------------------------------------------------------------------------------------
void CDockableBrowser::FillKindList(void) {
CString kinds;
CString kindName;

       m_KindList.RemoveAll();
       kinds = ExecuteCommand(CString("apply kind $DiscoverSelection"));
	   BOOL in=FALSE;
	   for(register i=0;i<kinds.GetLength();i++) {
		   if(kinds[i]=='{') {
			   in=TRUE;
			   continue;
		   }
		   if( (kinds[i]=='}' && in==TRUE) || (kinds[i]==' ' && in==FALSE)) {
			   kindName.TrimLeft();
			   kindName.TrimRight();
			   if(kindName.GetLength()>0) {
			      m_KindList.AddTail(kindName);
			      kindName="";
			   }
			   in=FALSE;
			   continue;
		   }
		   kindName+=kinds[i];
	   }
	   kindName.TrimLeft();
	   kindName.TrimRight();
	   if(kindName.GetLength()>0)  m_KindList.AddTail(kindName);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This function will search current kind list to determine if the mentioned kind is in 
// the list
//-----------------------------------------------------------------------------------------
BOOL CDockableBrowser::SelectionHasKind(CString kind) {
    if(m_KindList.Find(kind)==NULL) return FALSE;
	return TRUE;
}

//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Pdf structure changed because of submissin. Need to re-fill.
//-----------------------------------------------------------------------------------------
void CDockableBrowser::PdfChanged() {
    ClearHistory();

    CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
	CDataSource* DataSource;

	DataSource = (CDataSource *)GetDlgItem(IDC_DATASOURCE);
	LPDISPATCH lpDispatch;
	DataSource->GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);

	CQueryResults* Results  = (CQueryResults *) GetDlgItem(IDC_RESULTS);
	Results->SetFullListName("DiscoverAll");
	Results->SetFilterListName("DiscoverElements");
	Results->SetSelectionName("DiscoverSelection");
	Results->SetDataSource(lpDispatch);

    ExecuteCommand(CString("set DiscoverGroups [nil_set]"));
    ExecuteCommand(CString("set DiscoverElements [nil_set]"));
    ExecuteCommand(CString("set DiscoverAll [nil_set]"));
    ExecuteCommand(CString("set DiscoverScope /"));


	CCategoriesCombo* Category = (CCategoriesCombo *) GetDlgItem(IDC_CATEGORIES);
	Category->SetDataSource(lpDispatch);

	CQueriesCombo*    Query = (CQueriesCombo *) GetDlgItem(IDC_QUERIES);
	Query->SetDataSource(lpDispatch);

	CGroupsCombo*    Group = (CGroupsCombo *) GetDlgItem(IDC_GROUPS);
	Group->SetDataSource(lpDispatch);
	Group->SetSelectionVariable("DiscoverSelection");
	Group->SetOutputVariable("DiscoverAll");

    CMainFrame* main=(CMainFrame*) AfxGetMainWnd();
	main->m_SelectionSize=0;

	CProjectTree* Tree       = (CProjectTree *)    GetDlgItem(IDC_TREE);
	Tree->SetDataSource(lpDispatch);
    DisplayInfoString();

}
//-----------------------------------------------------------------------------------------
	
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnClosure(void) {
	if(m_Closure==TRUE) m_Closure=FALSE;
	else                m_Closure=TRUE;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnUpdateClosure(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_Closure);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnShortNames(void) {
	CQueryResults* list = (CQueryResults *)GetDlgItem(IDC_RESULTS);
	
	if(list->GetShortNames()==TRUE) list->SetShortNames(FALSE);
	else                            list->SetShortNames(TRUE);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void CDockableBrowser::OnUpdateShortNames(CCmdUI* pCmdUI) {
	CQueryResults* list = (CQueryResults *)GetDlgItem(IDC_RESULTS);
	pCmdUI->SetCheck(list->GetShortNames());
}
//-----------------------------------------------------------------------------------------


void CDockableBrowser::ViewFile(CString &params) {
register i;
CString serverQuery;
CString serverResults;
CString param;
CString filestr;
CString tokenstr;
CString linestr;
CString colstr;
CString viewable;
CString kind;
BSTR q;

CString filteredSetName;
CString elementN;
int elNumber;
int count=0;

    HCURSOR save = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	CDataSource* DataSource = (CDataSource *)     GetDlgItem(IDC_DATASOURCE);
	CCodeRoverIntegrator* Integrator=(CCodeRoverIntegrator*) GetDlgItem(IDC_CODEROVERINTEGRATORCTRL);
	if(Integrator==NULL || DataSource==NULL) return;

    for(i=0;i<params.GetLength();i++) {
	   if(params[i]==' ') {
		   count++;
		   continue;
	   }
	   if(count==0) filteredSetName+=params[i];
	   if(count==1) elementN  +=params[i];
    }
    elNumber=atoi(elementN);

    serverQuery.Format("set CurSet $%s", filteredSetName);
    q=serverQuery.AllocSysString();
    viewable=DataSourceAccessSync(DataSource, &q);
    ::SysFreeString(q);

    serverQuery.Format("viewable $%s %ld", filteredSetName, elNumber+1 );
    q=serverQuery.AllocSysString();
    viewable=DataSourceAccessSync(DataSource, &q);
    ::SysFreeString(q);
	viewable.TrimLeft();
	viewable.TrimRight();

    serverQuery.Format("kind $%s %ld", filteredSetName, elNumber+1 );
    q=serverQuery.AllocSysString();
    kind=DataSourceAccessSync(DataSource, &q);
    ::SysFreeString(q);
	kind.MakeUpper();
	kind.TrimLeft();
	kind.TrimRight();


    if(!(viewable=="0") || kind=="AST" || kind=="INSTANCE") {
       serverQuery.Format("printformat \"%%s\t%%s\" fname line;print $%s %d",filteredSetName,elNumber+1);
	   q=serverQuery.AllocSysString();
	   param=DataSource->AccessOutput(&q);
       ::SysFreeString(q);
	   // Extracting parameters
	   int nFileDelim = param.Find(_T('\t'));
	   filestr = param.Left(nFileDelim);
	   linestr = param.Mid(nFileDelim+1);
	   colstr = "-1";
	   tokenstr = "";
	   // Selecting
	   if(filestr.GetLength()!=0) {
		   if(tokenstr.GetLength()>0) {
               Integrator->MakeSelection(filestr,atoi(linestr),atoi(colstr),tokenstr.GetLength());
		   } else {
			   Integrator->OpenFile(filestr,atoi(linestr));
		   }
       }
       ::SetCursor(save);
    }
    else {
        ((CMainFrame *)AfxGetMainWnd())->MessageBox(
            _T("File cannot be viewed as text"), _T("File View Error"),MB_OK|MB_ICONERROR);
    }
}

void CDockableBrowser::ViewTextFile(CString &filename) {
    CWaitCursor wait;
    CCodeRoverIntegrator* Integrator =
               (CCodeRoverIntegrator*) GetDlgItem(IDC_CODEROVERINTEGRATORCTRL);
    if(Integrator != NULL) {
        Integrator->OpenFile(filename, -1);
    }
}

void CDockableBrowser::CreateCMMenu(CMenu** pCMMenu) {
	int id = ID_CM_ACTIONS;

	if(*pCMMenu) {
		delete *pCMMenu;
		*pCMMenu = NULL;
	}

	if(m_pCMCommands!=NULL) {
		if(!*pCMMenu) *pCMMenu = new CMenu();
		(*pCMMenu)->CreatePopupMenu();
		POSITION pos = m_pCMCommands->GetHeadPosition();
		while(pos!=NULL) {
			CCMCommand* pCommand = m_pCMCommands->GetNext(pos);
			CString name = pCommand->GetName();
			if(!name.IsEmpty())
				(*pCMMenu)->AppendMenu(MF_STRING,id,pCommand->GetName());
			id++;
		}
	}
}


void CDockableBrowser::OnEditorSetup()  {
	CEditorSelectionDialog dlg;
    CCodeRoverIntegrator* integrator=(CCodeRoverIntegrator*) GetDlgItem(IDC_CODEROVERINTEGRATORCTRL);
	if(integrator==NULL) return;
	dlg.m_AllEditorsStr=integrator->GetEditorsList();
	dlg.m_EditorName=integrator->GetEditorName();
    if(dlg.DoModal()==IDOK) {
		SetEditor(dlg.m_EditorName);
	}	
}

//---------------------------------------------------------------------------------------
// This callback will process all CM menu items.
//---------------------------------------------------------------------------------------
void CDockableBrowser::OnCMAction(int id) {
    CCMIntegrator* integrator=(CCMIntegrator*) GetDlgItem(IDC_CMINTEGRATORCTRL);
	if(integrator==NULL) return;

	int nCmdIdx = id-ID_CM_ACTIONS;
	CCMCommand* pCommand = m_pCMCommands->GetCommand(nCmdIdx);
	if(pCommand) {
		CStringList filesList;

		if(pCommand->HasAttribute(CString("file")) || 
		   pCommand->HasAttribute(CString("files"))) {
			// Forming file list  for submission
			bool bCheckWritable = false; // if you do not want the user be able
										 // to perform CM commands against not writable 
										 // projects set this variable to true
			CString res;
			CString cmd;
			BSTR sysCmd;
			int elements;
			CDataSource* pDataSource = (CDataSource *) GetDlgItem(IDC_DATASOURCE);

			if(bCheckWritable)
				cmd="set __cm_files__ [filter {kind==\"module\" && writable==1} [where defined $DiscoverSelection]]";
			else
				cmd="set __cm_files__ [filter {kind==\"module\"} [where defined $DiscoverSelection]]";

			sysCmd=cmd.AllocSysString();
			DataSourceAccessSync(pDataSource, &sysCmd);
			::SysFreeString(sysCmd);

			cmd="size $__cm_files__";
			sysCmd=cmd.AllocSysString();
			res=DataSourceAccessSync(pDataSource, &sysCmd);
			elements=atoi(res);
			::SysFreeString(sysCmd);
    
			
			cmd = "get_pref_value SubmissionCheck.Enabled";
			sysCmd=cmd.AllocSysString();
			res=DataSourceAccessSync(pDataSource, &sysCmd);
			::SysFreeString(sysCmd);
			
			for(int i=0;i<elements;i++) {
				// Extracting file name
				cmd.Format("fname $__cm_files__ %d",i+1);
				sysCmd=cmd.AllocSysString();
				res=DataSourceAccessSync(pDataSource, &sysCmd);
				::SysFreeString(sysCmd);
				filesList.AddHead(res);
			}
			cmd="unset __cm_files__";
			sysCmd=cmd.AllocSysString();
			res=DataSourceAccessSync(pDataSource, &sysCmd);
			elements=atoi(res);
			::SysFreeString(sysCmd);
			//---------------------------------------------------	
			if(filesList.GetCount()==0) {
				MessageBox("No writable files found to perform the CM command.","Warning",MB_OK|MB_ICONINFORMATION);
				return;
			}
		} else {
			CDataSource* pDataSource = (CDataSource *) GetDlgItem(IDC_DATASOURCE);

			CString cmd="fname " + m_CurrentProject;
			BSTR sysCmd=cmd.AllocSysString();
			CString szResult = DataSourceAccessSync(pDataSource, &sysCmd);
			::SysFreeString(sysCmd);
			filesList.AddHead(szResult);
		}
        DoCMAction(pCommand->GetKey(), filesList);
    }
}

void CDockableBrowser::DoCMAction(CString &cmdKey, CStringList &filesList) {
    CCMIntegrator* integrator=(CCMIntegrator*) GetDlgItem(IDC_CMINTEGRATORCTRL);
	if(integrator==NULL) return;

	CString szCMCmd;
	CCMCommand* pCommand = m_pCMCommands->GetCommandByKey(cmdKey);
    if(pCommand) {

		CCMAttributesList cmdAttrs;

		int nAttrCount = pCommand->GetAttributesCount();
		if(nAttrCount>0) {
			for(int i=0;i<nAttrCount;i++) {
				CString szCmdAttr = pCommand->GetAttribute(i);
				CCMAttribute* pAttr = NULL; 
				if(m_pCMAttributes->Lookup(szCmdAttr,pAttr)) {
					if(pAttr->AskFor()) {
						CString szOut = integrator->ExecCommand("get_attr_value\t"+pAttr->GetKey());
						if(pAttr->GetType() == CCMAttribute::LIST)
							pAttr->SetType(szOut);
						else
							pAttr->SetValue(szOut);
					}
					cmdAttrs.AddTail(pAttr);
				}
			}
		}
		
		if(filesList.GetCount()>0) {
			POSITION filePos = filesList.GetHeadPosition();
			while(filePos!=NULL) {
				int nRet = IDC_OKALL;
				
				if(cmdAttrs.GetCount() != 0) {
					CCMAttributesDlg wndDlg(&cmdAttrs,this);
					nRet = wndDlg.DoModal();
				}
			
				if(nRet==IDCANCEL) break;

				if(nRet==IDOK || nRet==IDC_OKALL) {
					szCMCmd = pCommand->GetKey();
					POSITION posHead = cmdAttrs.GetHeadPosition();
					while(posHead!=NULL) {
						CCMAttribute* pAttr = cmdAttrs.GetNext(posHead); 
						szCMCmd += "\t" + pAttr->GetKey() + "=\"" + pAttr->GetValue() + '"';
					}
						
					szCMCmd += "\t" + filesList.GetNext(filePos);
					if(nRet==IDC_OKALL) {
						while(filePos!=NULL) szCMCmd += "\t" + filesList.GetNext(filePos);
					}

					CString szReply = integrator->ExecCommand(szCMCmd);
					CString szError = "ERROR:";
					CString szPrefix = szReply.Left(szError.GetLength());
					if(szPrefix.CompareNoCase(szError)==0) {
						MessageBox(szReply.Mid(szError.GetLength()),"Source Management Error",MB_OK|MB_ICONERROR);
					} else {
						if(pCommand->Returns()) {
							MessageBox(szReply,"Result of a Source Management Command",MB_OK|MB_ICONINFORMATION);
						}
					}
				}
			}
		} else {
			szCMCmd = pCommand->GetKey();
			POSITION posHead = cmdAttrs.GetHeadPosition();
			while(posHead!=NULL) {
				CCMAttribute* pAttr = cmdAttrs.GetNext(posHead); 
				szCMCmd += "\t" + pAttr->GetKey() + "=\"" + pAttr->GetValue() + '"';
			}
				
			CString szReply = integrator->ExecCommand(szCMCmd);
			CString szError = "ERROR:";
			CString szPrefix = szReply.Left(szError.GetLength());
			if(szPrefix.CompareNoCase(szError)==0) {
				MessageBox(szReply.Mid(szError.GetLength()),"Source Management Error",MB_OK|MB_ICONERROR);
			} else {
				if(pCommand->Returns()) {
					MessageBox(szReply,"Result of a Source Management Command",MB_OK|MB_ICONINFORMATION);
				}
			}
		}
	}
}

void CDockableBrowser::DoCMAction(CString &cmdKey, int argNum, CString *argVal) {
    CCMIntegrator* integrator=(CCMIntegrator*) GetDlgItem(IDC_CMINTEGRATORCTRL);
    if(integrator != NULL && integrator->IsConnected()) {
	    CCMCommand* pCommand = m_pCMCommands->GetCommandByKey(cmdKey);
        if(pCommand != NULL) {
            // Process all passed parameters: extract CM arguments
            int i = 0;
            if(m_pCMAttributes != NULL) {
                for(i = 0; i < argNum; i++) {
                    int len = argVal[i].GetLength();
                    int pos = argVal[i].Find('=');
                    if(pos == -1)
                        break;
                    CString attrKey = argVal[i].Left(pos);
                    CCMAttribute* pAttr = NULL; 
	                if(m_pCMAttributes->Lookup(attrKey, pAttr)) {
                        if(argVal[i].GetAt(pos + 1) == '\"'
                            && argVal[i].GetAt(len - 1) == '\"') {
                            CString attrVal = argVal[i].Mid(pos + 2, len - pos - 3);
                            pAttr->SetValue(attrVal);
                        } else {
                            CString attrVal = argVal[i].Mid(pos + 1, len - pos - 1);
                            pAttr->SetValue(attrVal);
                        }
                    } else {
                        break;
                    }
                }
            }

            // The rest parameters are file names
            CStringList filesList;
            for(; i < argNum; i++) { // continue with the same index
                filesList.AddTail(argVal[i]);
            }

            DoCMAction(cmdKey, filesList);
        }
    }
}

void CDockableBrowser::OnCMSetup()  {
    CCMIntegrator* integrator=(CCMIntegrator*) GetDlgItem(IDC_CMINTEGRATORCTRL);
	if(integrator==NULL) return;

	CCMSelectionDialog dlg(integrator);
    if(dlg.DoModal()==IDOK) {
		CString szName = dlg.GetCMName();
		SetCM(szName);
	}	
}

void CDockableBrowser::OnUpdateCMSetup(CCmdUI* pCmdUI) 
{
	CCMIntegrator* pIntegrator=(CCMIntegrator*) GetDlgItem(IDC_CMINTEGRATORCTRL);
	if(pIntegrator) {
		if(!pIntegrator->IsAvailable()) pIntegrator->Reconnect();
		pCmdUI->Enable(pIntegrator->IsAvailable());	
	} else
		pCmdUI->Enable(false);
}

CString CDockableBrowser::DataSourceAccessSync(CDataSource* DataSource, BSTR* command) {
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

BOOL CDockableBrowser::SetCM(CString &szName)
{
    CCMIntegrator* integrator=(CCMIntegrator*) GetDlgItem(IDC_CMINTEGRATORCTRL);
	if(integrator==NULL) return FALSE;

	BOOL bRet = FALSE;
	if(szName.GetLength()!=0) {
		if(integrator->Connect(szName)) {
			// retriving CM attributes...
			m_pCMAttributes = new CCMAttributes(integrator->GetAttributes());

			// retriving CM commands...
			m_pCMCommands = new CCMCommandsList(integrator->GetCommands());

			CMainFrame* main = (CMainFrame *)AfxGetMainWnd();
			main->DestroyCMMenu();
			main->CreateCMMenu();
			main->DrawMenuBar();
			bRet = TRUE;
		} else {
			if(m_pCMCommands) delete m_pCMCommands;
			m_pCMCommands = NULL;
			if(m_pCMAttributes) delete m_pCMAttributes;
			m_pCMAttributes = NULL;
		}
	}
	return bRet;
}

CString CDockableBrowser::GetCM()
{
	CString szCMName;
    CCMIntegrator* integrator=(CCMIntegrator*) GetDlgItem(IDC_CMINTEGRATORCTRL);
	if(integrator!=NULL) 
		szCMName = integrator->GetCMName();
	return szCMName;
}

void CDockableBrowser::SetEditor(CString& szName)
{
    CCodeRoverIntegrator* integrator=(CCodeRoverIntegrator*) GetDlgItem(IDC_CODEROVERINTEGRATORCTRL);
	if(integrator==NULL) return;
	integrator->SetEditorName(szName);
	integrator->Reconnect();
}

CString CDockableBrowser::GetEditor()
{
	CString szEditor;
    CCodeRoverIntegrator* integrator=(CCodeRoverIntegrator*) GetDlgItem(IDC_CODEROVERINTEGRATORCTRL);
	if(integrator!=NULL)
		szEditor = integrator->GetEditorName();
	return szEditor;
}
