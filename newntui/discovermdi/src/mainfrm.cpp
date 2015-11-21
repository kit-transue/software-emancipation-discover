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
// MainFrm.cpp : implementation of the CMainFrame class
//

#define ID_USER_ACTION 20000

#include "stdafx.h"
#include "DiscoverMDI.h"
#include "ServiceManagerDlg.h"
#include "CheckinPromptDlg.h"
#include "CheckoutPromptDlg.h"
#include "UnCheckoutPromptDlg.h"
#include "LockPromptDlg.h"
#include "UnlockPromptDlg.h"
#include "CallGraphDlg.h"
#include "cmintegrator.h"
#include "ServiceChooser.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BOOL CAccessMenu::m_IDsInit=FALSE;
BOOL CAccessMenu::m_UsedIDs[100];

/////////////////////////////////////////////////////////////////////////////////////////
// CAccessMenuItem
/////////////////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------------------
// This function will set the menu item name
//---------------------------------------------------------------------------------------
void CAccessMenuItem::SetItemName(CString& name) {
	m_Name=name;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This function will return the menu item name
//---------------------------------------------------------------------------------------
CString& CAccessMenuItem::GetItemName(void) {
	return m_Name;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This function will set the menu item action
//---------------------------------------------------------------------------------------
void CAccessMenuItem::SetItemAction(CString& action) {
	m_Action=action;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This function will return the menu item name
//---------------------------------------------------------------------------------------
CString& CAccessMenuItem::GetItemAction(void) {
	return m_Action;
}
//---------------------------------------------------------------------------------------

void CAccessMenuItem::SetItemID(int id) {
	m_ID=id;
}

int CAccessMenuItem::GetItemID(void) {
	return m_ID;
}


//////////////////////////////////////////////////////////////////////////////////////////







/////////////////////////////////////////////////////////////////////////////////////////
// CAccessMenu
/////////////////////////////////////////////////////////////////////////////////////////
CAccessMenu::CAccessMenu() {
	if(m_IDsInit==FALSE) {
		for(register i=0;i<100;i++) m_UsedIDs[i]=FALSE;
		m_IDsInit=TRUE;
	}
	m_Name="";
}

CAccessMenu::~CAccessMenu() {
POSITION pos;
CAccessMenuItem* item;

   pos=m_ItemsList.GetHeadPosition();
   while(pos!=NULL) {
		item = m_ItemsList.GetAt(pos);
		int id = item->GetItemID();
        m_UsedIDs[id]=FALSE;
		delete m_ItemsList.GetAt(pos);
		m_ItemsList.GetNext(pos);
   }
   m_ItemsList.RemoveAll();
   pos=m_MenusList.GetHeadPosition();
   while(pos!=NULL) {
		delete m_MenusList.GetAt(pos);
		m_MenusList.GetNext(pos);
   }
   m_MenusList.RemoveAll();

}

//---------------------------------------------------------------------------------------
// This function will set the Access menu name.
//---------------------------------------------------------------------------------------
void CAccessMenu::SetMenuName(CString& name) {
	m_Name=name;
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This function will return the Access menu name.
//---------------------------------------------------------------------------------------
CString& CAccessMenu::GetMenuName(void) {
   return m_Name;
}
//---------------------------------------------------------------------------------------


CString& CAccessMenu::GetDefSelection(void) {
   return m_DefSelection;
}


void     CAccessMenu::SetDefSelection(CString& selection) {
   m_DefSelection=selection;
}



//---------------------------------------------------------------------------------------
// This function will add new pop-up menu to the access submenu.
// Will return TRUE if the menu was successfully added or FALSE if the menu with this 
// name was already in the menus list;
//---------------------------------------------------------------------------------------
BOOL CAccessMenu::AddMenu(CString& menuName,CString& defSelection) {
POSITION pos;
CAccessMenu* to;
CString title;
CString s;

   // starting with this menu
   to=this;

   // Searching for the "." separators
   for(int i=0;i<menuName.GetLength();i++) {
	   if(menuName[i]=='.') {
		   to=FindMenu(s);
		   if(to==NULL) return FALSE;
		   s="";
		   continue;
	   }
	   s+=menuName[i];
   }
   title=s;


   pos=to->m_MenusList.GetHeadPosition();

   while(pos!=NULL) {
	   if(to->m_MenusList.GetAt(pos)->GetMenuName()==title) {
			return FALSE;
	   }
	   to->m_MenusList.GetNext(pos);
   }

   CAccessMenu* popupMenu;
   popupMenu=new CAccessMenu;
   popupMenu->SetMenuName(title);
//   popupMenu->SetDefSelection(defSelection);
   to->m_MenusList.AddTail(popupMenu);
   return TRUE;
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This function will try to remove the pop-up menu from the Access sub-menu. Will return
// TRUE if the pop-up menu was successfully removed or FALSE if no such menu in the menus
// list.
//---------------------------------------------------------------------------------------
BOOL CAccessMenu::RemoveMenu(CString& menuName) {
POSITION pos;
CAccessMenu* toRemove;
CAccessMenu* to;
CString title;
CString s;


   // starting with this menu
   to=this;

   // Searching for the "." separators
   for(int i=0;i<menuName.GetLength();i++) {
	   if(menuName[i]=='.') {
		   to=FindMenu(s);
		   if(to==NULL) return FALSE;
		   s="";
		   continue;
	   }
	   s+=menuName[i];
   }
   title=s;

   pos=to->m_MenusList.GetHeadPosition();

   while(pos!=NULL) {
		if(to->m_MenusList.GetAt(pos)->GetMenuName()==title) {
		   toRemove=to->m_MenusList.GetAt(pos);
		   delete toRemove;
		   to->m_MenusList.RemoveAt(pos);
		   return TRUE;
		}
		to->m_MenusList.GetNext(pos);
   }
   return FALSE;
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This function will add new pop-up menu to the access submenu.
// Will return TRUE if the menu was successfully added or FALSE if the menu with this 
// name was already in the menus list;
//---------------------------------------------------------------------------------------
BOOL CAccessMenu::AddMenuItem(CString& itemName,CString& action) {
POSITION pos;
int freeMenuID;
CAccessMenu* to;
CString title;
CString s;

   // starting with this menu
   to=this;

   // Searching for the "." separators
   for(int i=0;i<itemName.GetLength();i++) {
	   if(itemName[i]=='.') {
		   to=FindMenu(s);
		   if(to==NULL) return FALSE;
		   s="";
		   continue;
	   }
	   s+=itemName[i];
   }
   title=s;


   for(freeMenuID=0;freeMenuID<100;freeMenuID++) 
	  if(m_UsedIDs[freeMenuID]==FALSE) break;
   if(freeMenuID==100) return FALSE;


   pos=to->m_ItemsList.GetHeadPosition();

   while(pos!=NULL) {
		if(to->m_ItemsList.GetAt(pos)->GetItemName()==title) {
			return FALSE;
		}
		to->m_ItemsList.GetNext(pos);
   }
   CAccessMenuItem* menuItem;
   menuItem=new CAccessMenuItem;
   menuItem->SetItemName(itemName);
   menuItem->SetItemAction(action);
   menuItem->SetItemID(freeMenuID);
   m_UsedIDs[freeMenuID]=TRUE;
   to->m_ItemsList.AddTail(menuItem);
   return TRUE;
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This function will try to remove the pop-up menu from the Access sub-menu. Will return
// TRUE if the pop-up menu was successfully removed or FALSE if no such menu in the menus
// list.
//---------------------------------------------------------------------------------------
BOOL CAccessMenu::RemoveMenuItem(CString& itemName) {
POSITION pos;
CAccessMenu* to;
CString title;
CString s;


   // starting with this menu
   to=this;

   // Searching for the "." separators
   for(int i=0;i<itemName.GetLength();i++) {
	   if(itemName[i]=='.') {
		   to=FindMenu(s);
		   if(to==NULL) return FALSE;;
		   s="";
		   continue;
	   }
	   s+=itemName[i];

   }
   title=s;

   pos=to->m_ItemsList.GetHeadPosition();

   while(pos!=NULL) {
		if(to->m_ItemsList.GetAt(pos)->GetItemName()==title) {
            m_UsedIDs[to->m_ItemsList.GetAt(pos)->GetItemID()]=FALSE;
			delete to->m_ItemsList.GetAt(pos);
			to->m_ItemsList.RemoveAt(pos);
			return TRUE;
		}
		to->m_ItemsList.GetNext(pos);
   }
   return FALSE;
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This function will try to find the popup menu with the "menuName" name. Will return
// it's position or NULL if no menu with this name.
//---------------------------------------------------------------------------------------
CAccessMenu* CAccessMenu::FindMenu(CString& menuName) {
POSITION pos;
CAccessMenu* to;
CString title;
CString s;


   // starting with this menu
   to=this;

   // Searching for the "." separators
   for(int i=0;i<menuName.GetLength();i++) {
	   if(menuName[i]=='.') {
		   to=FindMenu(s);
		   if(to==NULL) return NULL;
		   s="";
		   continue;
	   }
	   s+=menuName[i];

   }
   title=s;


   pos=to->m_MenusList.GetHeadPosition();

   while(pos!=NULL) {
		if(to->m_MenusList.GetAt(pos)->GetMenuName()==title) {
			return to->m_MenusList.GetAt(pos);
		}
		to->m_MenusList.GetNext(pos);
   }
   return NULL;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This function will fill command from the menu item ID. Will return TRUE if successfull
// or FALSE if no command with this ID was found.
//---------------------------------------------------------------------------------------
BOOL CAccessMenu::CommandFromID(int id, CString& command) {
POSITION pos;

   pos=m_ItemsList.GetHeadPosition();
   while(pos!=NULL) {
		if(m_ItemsList.GetAt(pos)->GetItemID()==id) {
			command=m_ItemsList.GetAt(pos)->GetItemAction();
			return TRUE;
		}
		m_ItemsList.GetNext(pos);
   }

   pos=m_MenusList.GetHeadPosition();
   while(pos!=NULL) {
	   if(m_MenusList.GetAt(pos)->CommandFromID(id,command) == TRUE) {
		   return TRUE;
	   }
	   m_MenusList.GetNext(pos);
   }
   command="";
   return FALSE;
}
//---------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////////////////
// CMainFrame
/////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_MESSAGE(WM_UPDATEGROUPS,OnUpdateGroups)
	ON_MESSAGE(WM_UPDATE_ERRORS,OnUpdateErrors)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_COMMAND(ID_DORMANT, OnDormant)
	ON_COMMAND(ID_FLOWCHART, OnFlowchart)
	ON_COMMAND(ID_DATACHART, OnDatachart)
	ON_COMMAND(ID_ERD, OnErd)
	ON_COMMAND(ID_INHERITANCE, OnInheritance)
	ON_COMMAND(ID_OUTLINE, OnOutline)
	ON_COMMAND(ID_TREEDIAGRAM, OnTreediagram)
	ON_UPDATE_COMMAND_UI(ID_DORMANT, OnUpdateDormant)
	ON_UPDATE_COMMAND_UI(ID_ERD, OnUpdateErd)
	ON_UPDATE_COMMAND_UI(ID_DATACHART, OnUpdateDatachart)
	ON_UPDATE_COMMAND_UI(ID_FLOWCHART, OnUpdateFlowchart)
	ON_UPDATE_COMMAND_UI(ID_INHERITANCE, OnUpdateInheritance)
	ON_UPDATE_COMMAND_UI(ID_OUTLINE, OnUpdateOutline)
	ON_UPDATE_COMMAND_UI(ID_TREEDIAGRAM, OnUpdateTreediagram)
	ON_COMMAND(ID_QA, OnQa)
	ON_COMMAND(IDC_SERVICECHANGED,OnServiceChanged)
	ON_MESSAGE(WM_SERVICE_DISCONNECT,OnServiceDisconnect)
	ON_COMMAND(ID_SERVICESELECT, OnServiceSelect)
	ON_COMMAND(ID_ACCESS, OnAccess)
	ON_COMMAND(ID_OPENDATACHART, OnOpenDataChart)
	ON_COMMAND(ID_OPENERD, OnOpenERD)
	ON_COMMAND(ID_OPENFLOWCHART, OnOpenFlowChart)
	ON_COMMAND(ID_OPENINHERITANCE, OnOpenInheritance)
	ON_COMMAND(ID_OPENOUTLINE, OnOpenOutline)
	ON_COMMAND(ID_OPENTREEDIAGRAM, OnOpenTree)
	ON_WM_CLOSE()
	ON_COMMAND(ID_UPDATE, OnUpdate)
	ON_UPDATE_COMMAND_UI(ID_UPDATE, OnUpdateUpdate)
	ON_WM_SETCURSOR()
	ON_COMMAND(IDC_SIMPLIFY, OnSimplify)
	ON_UPDATE_COMMAND_UI(ID_SIMPLIFY, OnUpdateSimplify)
	ON_COMMAND(IDC_IMPACT, OnImpact)
	ON_UPDATE_COMMAND_UI(IDC_IMPACT, OnUpdateImpact)
	ON_COMMAND(ID_VIEW_BROWSER, OnViewBrowser)
	ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BROWSER, OnUpdateViewBrowser)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateViewToolbar)
	ON_UPDATE_COMMAND_UI(ID_QA, OnUpdateQa)
	ON_UPDATE_COMMAND_UI(ID_ACCESS, OnUpdateAccess)
	ON_UPDATE_COMMAND_UI(ID_XMLREPORT, OnUpdateXmlreport)
	ON_COMMAND(ID_CALLGRAPH, OnCallgraph)
	ON_UPDATE_COMMAND_UI(ID_CALLGRAPH, OnUpdateCallgraph)
	ON_COMMAND(IDC_VIEW_ERRORS, OnViewErrors)
	ON_UPDATE_COMMAND_UI(IDC_VIEW_ERRORS, OnUpdateViewErrors)
	ON_UPDATE_COMMAND_UI(ID_EDITOR_SETUP, OnUpdateEditorSetup)
	ON_COMMAND(ID_EDITOR_SETUP, OnEditorSetup)
	ON_UPDATE_COMMAND_UI(ID_CM_SETUP, OnUpdateCMSetup)
	ON_COMMAND(ID_CM_SETUP, OnCMSetup)
	ON_UPDATE_COMMAND_UI(IDC_SIMPLIFY, OnUpdateSimplify)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND_RANGE(ID_USER_ACTION,ID_USER_ACTION+100,OnUserAction)
	ON_COMMAND_RANGE(ID_CM_ACTIONS,ID_CM_ACTIONS+50,OnCMAction)
	//ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	// ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelpIndex)
	//ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	// ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpIndex)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_wndDormant=NULL;
	m_wndQA=NULL;
	m_wndAccess=NULL;
	m_wndSimplify=NULL;
	m_wndImpact=NULL;
	m_SelectionSize=0;
	m_ServerConnected=FALSE;
	m_Updating=FALSE;
	m_Waiting=FALSE;
    //initGLT();

}

CMainFrame::~CMainFrame() {
	//cleanupGLT();
	DestroyCMMenu();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	/*if (!m_wndViewsToolbar.Create(this) ||
		!m_wndViewsToolbar.LoadToolBar(IDR_MAINFRAMEVIEWS))
	{
		TRACE0("Failed to create views toolbar\n");
		return -1;      // fail to create
	}*/

	if (!m_wndBaseToolbar.Create(this) ||
		!m_wndBaseToolbar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create main toolbar\n");
		return -1;      // fail to create
	}
	m_wndBaseToolbar.SetWindowText("DIScover Tools");


	if (!m_wndBrowser.Create(this, IDD_DOCKABLEBROWSER,
		CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC, IDD_DOCKABLEBROWSER))
	{
		TRACE0("Failed to create dockable browser\n");
		return -1;      // fail to create
	}
	m_wndBrowser.SetWindowText("Information Model Browser");


	
	if (!m_wndErrors.Create(this, IDD_ERRORSBROWSER,
		CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC, IDD_ERRORSBROWSER))
	{
		TRACE0("Failed to create errors browser\n");
		return -1;      // fail to create
	}
	m_wndErrors.SetWindowText("Errors Browser");



	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndBaseToolbar.SetBarStyle(m_wndBaseToolbar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	/*m_wndViewsToolbar.SetBarStyle(m_wndViewsToolbar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);*/

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	EnableDocking(CBRS_ALIGN_ANY);
	m_wndBrowser.EnableDocking(CBRS_ALIGN_ANY);
	m_wndErrors.EnableDocking(CBRS_ALIGN_ANY);
	m_wndBaseToolbar.EnableDocking(CBRS_ALIGN_ANY);

	DockControlBar(&m_wndBrowser,AFX_IDW_DOCKBAR_LEFT);
	DockControlBar(&m_wndErrors,AFX_IDW_DOCKBAR_BOTTOM);
	DockControlBar(&m_wndBaseToolbar);


	LoadBarState("DiscoverControlBarState");

	m_wndBrowser.EnableDocking(CBRS_ALIGN_ANY);
	m_wndErrors.EnableDocking(CBRS_ALIGN_ANY);
	m_wndBaseToolbar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);

    Preferences* pPrefs = ((CDiscoverMDIApp *)AfxGetApp())->GetPreferences();
	CString szSetting;
	if(pPrefs->get("CM",szSetting)) m_wndBrowser.SetCM(szSetting);
	if(pPrefs->get("Editor",szSetting)) m_wndBrowser.SetEditor(szSetting);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnSize(UINT nType, int cx, int cy)  {
	CMDIFrameWnd::OnSize(nType, cx, cy);
}

void CMainFrame::RecalcLayout(BOOL bNotify)  {
	CMDIFrameWnd::RecalcLayout(bNotify);
	m_wndBrowser.CalculateVariableSize();
	m_wndBrowser.LayoutControls();
	m_wndErrors.CalculateVariableSize();
	m_wndErrors.LayoutControls();
   SetDiscoverName();
}


//---------------------------------------------------------------------------------------
// This function will add new pop-up menu to the Access sub-menu.
//---------------------------------------------------------------------------------------
BOOL CMainFrame::AddAccessMenu(CString& menuName, CString& defSelection) {

   return m_AccessMenu.AddMenu(menuName,defSelection);

}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This function will try to remove all menu items and pop-up menu itself from the acceess
// sub-menu.
//---------------------------------------------------------------------------------------
BOOL CMainFrame::RemoveAccessMenu(CString& menuName) {
	return m_AccessMenu.RemoveMenu(menuName);
}
//---------------------------------------------------------------------------------------




//---------------------------------------------------------------------------------------
// This function will add new menu item to the Access sub-menu. The "menuName"
// parameter specifies the popup menu to which we are goint to add new item or,
// if empty, specifies that we are going to add the item to the Access menu itself.
// "itemName" parameter specifies the name of the item, "action" parameter 
// specifies the Access procedure call when menu item selected.
// Will return TRUE  if menu item was added successfully or FALSE if no menu
// with the name passed was detected.
//---------------------------------------------------------------------------------------
BOOL CMainFrame::AddAccessMenuItem(CString& menuName,CString& itemName, CString& action) {

	if(menuName.GetLength()==0) 
		return m_AccessMenu.AddMenuItem(itemName,action);
	CAccessMenu* menu=m_AccessMenu.FindMenu(menuName);
	if(menu==NULL) return FALSE;
	return menu->AddMenuItem(itemName,action);
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This function will try to remove menu item from the "menuName" menu. It will try to
// remove the item with the name "itemName".
// Will return TRUE if the menu item was successfully removed or FALSE if no menu or 
// menu item was detected.
//---------------------------------------------------------------------------------------
BOOL CMainFrame::RemoveAccessMenuItem(CString& menuName,CString& itemName) {
	if(menuName.GetLength()==0) 
		return m_AccessMenu.RemoveMenuItem(itemName);
	CAccessMenu* menu=m_AccessMenu.FindMenu(menuName);
	if(menu==NULL) return FALSE;
	return menu->RemoveMenuItem(itemName);
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This recursively called function will add all menus to the given menu on all levels;
//---------------------------------------------------------------------------------------
void CMainFrame::RecursiveMenuAdd(CAccessMenu& from, CMenu& to) {
CMenu local;
POSITION menuPos,itemPos;

   // Adding all sub-menus
   menuPos=from.m_MenusList.GetHeadPosition();
   while(menuPos!=NULL) {
	    // Creating pop-up menu
	    CMenu local;
        HMENU hm = ::CreatePopupMenu();
        local.Attach(hm);
        if(hm!=NULL) {
            // Adding it to the Access menu
	        to.AppendMenu( MF_POPUP|MF_STRING,
			               (unsigned int)hm,
						   m_AccessMenu.m_MenusList.GetAt(menuPos)->GetMenuName());
        }
		// The same for all sub-menus
		RecursiveMenuAdd(*from.m_MenusList.GetAt(menuPos),local);
		local.Detach();
	    from.m_MenusList.GetNext(menuPos);
   }

   // Adding all menu items    
   itemPos=from.m_ItemsList.GetHeadPosition();
   while(itemPos!=NULL) {
	   to.AppendMenu(MF_STRING,
		             ID_USER_ACTION+from.m_ItemsList.GetAt(itemPos)->GetItemID(),
					 from.m_ItemsList.GetAt(itemPos)->GetItemName());
	   from.m_ItemsList.GetNext(itemPos);
   }
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This function will destroy the menu for CM oparations
//---------------------------------------------------------------------------------------
void CMainFrame::DestroyCMMenu() {
	int iPos;
	if(m_MainMenu.m_hMenu!=0) {
		for(iPos = m_MainMenu.GetMenuItemCount()-1; iPos >= 0; iPos--) {
			CString strMenuItem;
			m_MainMenu.GetMenuString(iPos, strMenuItem, MF_BYPOSITION);
			if(strMenuItem.CompareNoCase("CM")==0) {
				CMenu* pMenu = m_MainMenu.GetSubMenu(iPos);
				m_MainMenu.RemoveMenu(iPos,MF_BYPOSITION);
				delete pMenu;
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This function will create the new menu for CM operations
//---------------------------------------------------------------------------------------
void CMainFrame::CreateCMMenu() {
	if(m_MainMenu.m_hMenu!=0) {
		CMenu* pMenu = NULL;
		m_wndBrowser.CreateCMMenu(&pMenu);
		if(pMenu!=NULL)
			m_MainMenu.InsertMenu(1, MF_BYPOSITION | MF_POPUP,(UINT)pMenu->GetSafeHmenu(), "CM");
	}
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This function will set the new menu to the main window. The difference between
// this function and MDISetMenu function is that this function will add to the
// mentioned menu all menu items added by Access commands grouped in the Access
// sub-menu (lockated before "Windows" and "Help" sub-menus).
//---------------------------------------------------------------------------------------
void CMainFrame::SetNewMenu(UINT nIdResource) {
CMenu access;
   DestroyCMMenu();
   m_MainMenu.DestroyMenu();
   m_BaseMenuResource=nIdResource;
   m_MainMenu.LoadMenu(nIdResource);
   CreateCMMenu();
   HMENU hm = ::CreatePopupMenu();
   access.Attach(hm);
   if(hm!=NULL) {
       RecursiveMenuAdd(m_AccessMenu,access);
       UINT pos=m_MainMenu.GetMenuItemCount();
       if(pos>=2) pos-=2;
       m_MainMenu.InsertMenu(pos,MF_BYPOSITION|MF_POPUP|MF_STRING,(unsigned int)hm,"Access");
   }
   access.Detach();

   int count=0;
   if(m_wndDormant!=NULL)  {
	   m_MainMenu.InsertMenu(ID_WINDOW_CASCADE,MF_BYCOMMAND|MF_STRING,ID_DORMANT,"Dormant");
	   count++;
   }
   if(m_wndQA!=NULL) {
	   m_MainMenu.InsertMenu(ID_WINDOW_CASCADE,MF_BYCOMMAND|MF_STRING,ID_QA,"Caliper");
	   count++;
   }
   if(m_wndAccess!=NULL)   {
	   m_MainMenu.InsertMenu(ID_WINDOW_CASCADE,MF_BYCOMMAND|MF_STRING,ID_ACCESS,"Access terminal");
	   count++;
   }
   if(m_wndSimplify!=NULL) {
	   m_MainMenu.InsertMenu(ID_WINDOW_CASCADE,MF_BYCOMMAND|MF_STRING,IDC_SIMPLIFY,"Simplify");
	   count++;
   }
   if(m_wndImpact!=NULL) {
	   m_MainMenu.InsertMenu(ID_WINDOW_CASCADE,MF_BYCOMMAND|MF_STRING,IDC_IMPACT,"Impact");
	   count++;
   }
   if(count>0) m_MainMenu.InsertMenu(ID_WINDOW_CASCADE,MF_SEPARATOR,0,"");



   MDISetMenu(&m_MainMenu,NULL);
//   access.DestroyMenu();
   DrawMenuBar();

}
//---------------------------------------------------------------------------------------


UINT CMainFrame::GetBaseMenuResource(void)  {
	return m_BaseMenuResource;
}


//---------------------------------------------------------------------------------------
// This callback will process all user-defined menu items.
//---------------------------------------------------------------------------------------
void CMainFrame::OnUserAction(int id) {
CString command;
CString result;
    m_AccessMenu.CommandFromID(id-ID_USER_ACTION,command);
    result=m_wndBrowser.ExecuteCommand(command);
	if(result.Find("_DI_")==-1) {
		// If this string is not a set name we will put this string into status line
		m_wndErrors.SetErrorLog(result);
        m_wndStatusBar.SetPaneText(0,result);
	} else {
		// If  this string is a set name we will put it's elements into results window
		m_wndBrowser.BrowseGivenSet(result,1);
	}
}
//---------------------------------------------------------------------------------------


void CMainFrame::OnPaint() {
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	if(::IsWindow(m_wndBrowser))  {
		if(m_wndBrowser.m_OnSize==TRUE)
		   m_wndBrowser.UpdateSashes(FALSE);
		if(m_wndErrors.m_OnSize==TRUE)
		   m_wndErrors.UpdateSashes(FALSE);
	}
	
	
	// Do not call CMDIFrameWnd::OnPaint() for painting messages
}


//----------------------------------------------------------------------
void CMainFrame::OnDormant()  {
   if(m_wndDormant==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CDormantChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CDormantChildFrame)));
      m_wndDormant=(CDormantChildFrame *) pObject;
      m_wndDormant->LoadFrame(IDR_DORMANT, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,NULL,NULL);
      m_wndDormant->ShowWindow(SW_SHOW);
   } else {
	   m_wndDormant->Update();
   }
   MDIActivate(m_wndDormant);
   SetDiscoverName();
}

void CMainFrame::OnFlowchart() {
	m_wndBrowser.ExecuteCommand(CString("open_view -flowchart $DiscoverSelection"));
	
}

void CMainFrame::OnDatachart()  {
	m_wndBrowser.ExecuteCommand(CString("open_view -datachart $DiscoverSelection"));
	
}

void CMainFrame::OnErd()  {
	m_wndBrowser.ExecuteCommand(CString("open_view -relations $DiscoverSelection"));
	
}

void CMainFrame::OnInheritance()  {
	m_wndBrowser.ExecuteCommand(CString("open_view -inheritance $DiscoverSelection"));
	
}

void CMainFrame::OnOutline()  {
	m_wndBrowser.ExecuteCommand(CString("open_view -outline $DiscoverSelection"));
	
}

void CMainFrame::OnTreediagram() 
{
	m_wndBrowser.ExecuteCommand(CString("open_view -dependency $DiscoverSelection"));
	
}

void CMainFrame::OnUpdateDormant(CCmdUI* pCmdUI)  {
   pCmdUI->Enable(m_SelectionSize>0 && m_Lic[LIC_DORMANT]);
	
}

void CMainFrame::OnUpdateErd(CCmdUI* pCmdUI)  {
   pCmdUI->Enable((m_SelectionSize>0) && m_wndBrowser.SelectionHasKind("struct"));
}

void CMainFrame::OnUpdateDatachart(CCmdUI* pCmdUI) {
   pCmdUI->Enable(m_SelectionSize>0);
}

void CMainFrame::OnUpdateFlowchart(CCmdUI* pCmdUI) {
   pCmdUI->Enable((m_SelectionSize>0) && m_wndBrowser.SelectionHasKind("funct"));
}

void CMainFrame::OnUpdateInheritance(CCmdUI* pCmdUI) {
   pCmdUI->Enable((m_SelectionSize>0) && m_wndBrowser.SelectionHasKind("struct"));
}

void CMainFrame::OnUpdateOutline(CCmdUI* pCmdUI) {
   pCmdUI->Enable((m_SelectionSize>0) && m_wndBrowser.SelectionHasKind("funct"));
}

void CMainFrame::OnUpdateTreediagram(CCmdUI* pCmdUI)  {
   pCmdUI->Enable(m_SelectionSize>0);
	
}

void CMainFrame::OnQa()  {
	if(m_wndQA==NULL) {
        CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CQAChildFrame);
        CObject* pObject = pRuntimeClass->CreateObject();
        ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CQAChildFrame)));
        m_wndQA=(CQAChildFrame *) pObject;
        m_wndQA->LoadFrame(IDR_QA, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,NULL,NULL);
        m_wndQA->ShowWindow(SW_SHOW);
	}
    MDIActivate(m_wndQA);
    SetDiscoverName();
}

LRESULT CMainFrame::OnServiceDisconnect(WPARAM wparam, LPARAM lparam) {
	// Notify all child windows that service have changed
	if(m_wndDormant!=NULL)	m_wndDormant->SendMessage(WM_SERVICE_DISCONNECT);
	if(m_wndQA!=NULL)		m_wndQA->SendMessage(WM_SERVICE_DISCONNECT);
	if(m_wndAccess!=NULL)	m_wndAccess->SendMessage(WM_SERVICE_DISCONNECT);
	if(m_wndSimplify!=NULL)	m_wndSimplify->SendMessage(WM_SERVICE_DISCONNECT);
	if(m_wndImpact!=NULL)	m_wndImpact->SendMessage(WM_SERVICE_DISCONNECT);

	// Notify browser that service changed
	m_wndBrowser.SendMessage(WM_SERVICE_DISCONNECT);

	SetDiscoverName();
	m_ServerConnected=FALSE;
	return TRUE;
}

void CMainFrame::OnServiceChanged()  {

   // Notify browser that service changed
   m_wndBrowser.PostMessage(WM_COMMAND,IDC_SERVICECHANGED,0);

   // Notify all child windows that service have changed
   if(m_wndDormant!=NULL)          m_wndDormant->PostMessage(WM_COMMAND,IDC_SERVICECHANGED,0);
   if(m_wndQA!=NULL)               m_wndQA->PostMessage(WM_COMMAND,IDC_SERVICECHANGED,0);
   if(m_wndAccess!=NULL)		   m_wndAccess->PostMessage(WM_COMMAND,IDC_SERVICECHANGED,0);
   if(m_wndSimplify!=NULL)		   m_wndSimplify->PostMessage(WM_COMMAND,IDC_SERVICECHANGED,0);
   if(m_wndImpact!=NULL)		   m_wndImpact->PostMessage(WM_COMMAND,IDC_SERVICECHANGED,0);
   SetDiscoverName();
   m_ServerConnected=TRUE;
}

void CMainFrame::OnServiceSelect()  {
    CDiscoverMDIApp* app =  (CDiscoverMDIApp *) AfxGetApp();

	// get current selected repository
	Preferences* pPrefs = app->GetPreferences();
	CString szRepository;
	pPrefs->get("DISIM",szRepository);

	CServiceManagerDlg dlg(this,&szRepository);
	if(dlg.DoModal()==IDOK) {
		CString szServiceName = dlg.GetServiceName();
		if(m_ServerConnected)	{
			SendMessage(WM_SERVICE_DISCONNECT);
		}
	    app->m_ServiceName=szServiceName;
		OnServiceChanged();
	}
	// save current selected repository
	pPrefs->set("DISIM",dlg.GetRepositoryPath());
}

void CMainFrame::OnAccess()  {
   if(m_wndAccess==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CAccessChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CAccessChildFrame)));
      m_wndAccess=(CAccessChildFrame *) pObject;
      m_wndAccess->LoadFrame(IDR_ACCESS, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,NULL,NULL);
      m_wndAccess->ShowWindow(SW_SHOW);
   }
   MDIActivate(m_wndAccess);
   SetDiscoverName();
}

//=============================== GRAPHICAL VIEWS ========================================

//----------------------------------------------------------------------------------------
// This callback will run when user opens Data Chart graphical view
//----------------------------------------------------------------------------------------
void CMainFrame::OnOpenDataChart()  {
/*   if(m_wndDataChartView==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CDataChartViewChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CDataChartViewChildFrame)));
      m_wndDataChartView=(CDataChartViewChildFrame *) pObject;
      m_wndDataChartView->LoadFrame(IDR_DATACHARTVIEW, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,NULL,NULL);
      m_wndDataChartView->ShowWindow(SW_SHOW);
   }
   MDIActivate(m_wndDataChartView);*/
	
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This callback will run when user opens ERD graphical view
//----------------------------------------------------------------------------------------
void CMainFrame::OnOpenERD()  {
   /*if(m_wndERDView==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CERDViewChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CERDViewChildFrame)));
      m_wndERDView=(CERDViewChildFrame *) pObject;
      m_wndERDView->LoadFrame(IDR_ERDVIEW, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,NULL,NULL);
      m_wndERDView->ShowWindow(SW_SHOW);
   }
   MDIActivate(m_wndERDView);*/
	
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This callback will run when user opens Flow Chart graphical view
//----------------------------------------------------------------------------------------
void CMainFrame::OnOpenFlowChart()  {
   /*if(m_wndFlowChartView==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CFlowChartViewChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CFlowChartViewChildFrame)));
      m_wndFlowChartView=(CFlowChartViewChildFrame *) pObject;
      m_wndFlowChartView->LoadFrame(IDR_FLOWCHARTVIEW, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,NULL,NULL);
      m_wndFlowChartView->ShowWindow(SW_SHOW);
   }
   MDIActivate(m_wndFlowChartView);*/
	
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This callback will run when user opens Inheritance graphical view
//----------------------------------------------------------------------------------------
void CMainFrame::OnOpenInheritance()  {
   /*if(m_wndInheritanceView==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CInheritanceViewChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CInheritanceViewChildFrame)));
      m_wndInheritanceView=(CInheritanceViewChildFrame *) pObject;
      m_wndInheritanceView->LoadFrame(IDR_INHERITANCEVIEW, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,NULL,NULL);
      m_wndInheritanceView->ShowWindow(SW_SHOW);
   }
   MDIActivate(m_wndInheritanceView);*/

}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This callback will run when user opens Outline graphical view
//----------------------------------------------------------------------------------------
void CMainFrame::OnOpenOutline()  {
   /*if(m_wndOutlineView==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(COutlineViewChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(COutlineViewChildFrame)));
      m_wndOutlineView=(COutlineViewChildFrame *) pObject;
      m_wndOutlineView->LoadFrame(IDR_OUTLINEVIEW, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,NULL,NULL);
      m_wndOutlineView->ShowWindow(SW_SHOW);
   }
   MDIActivate(m_wndOutlineView);*/
	
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This callback will run when user opens Call Tree graphical view
//----------------------------------------------------------------------------------------
void CMainFrame::OnOpenTree()  {
   /*if(m_wndTreeView==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CTreeViewChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CTreeViewChildFrame)));
      m_wndTreeView=(CTreeViewChildFrame *) pObject;
      m_wndTreeView->LoadFrame(IDR_TREEVIEW, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,NULL,NULL);
      m_wndTreeView->ShowWindow(SW_SHOW);
   }
   MDIActivate(m_wndTreeView);*/
	
}
//----------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// This callback will be called every time the user want to update selected symbols.
// it's task is the followind:
//    1. It will extract all currently selected symbols or project scope files.
//    2. It will create a list of files we need to update.
//    3. It will call "update_module" command giving the list of modules we need to update.
//------------------------------------------------------------------------------------------
void CMainFrame::OnUpdate()  {
CString parserLogFile;

    m_wndBrowser.ExecuteCommand(CString("set dis_build_list [filter {outdated==1} [apply lname [where defined $DiscoverSelection]]]"));
	CString sz = m_wndBrowser.ExecuteCommand(CString("size $dis_build_list"));
	int amount = atoi(sz);
	if(amount>0) {
        if(m_UpdateDlg.DoModal()==IDOK) {
		    HCURSOR save = SetCursor(::LoadCursor(NULL, IDC_WAIT));
		    parserLogFile=m_wndBrowser.ExecuteCommand(CString("els_tmpname [fname [filter header==0  [apply lname [where defined $DiscoverSelection]]] 1]"));
		    parserLogFile+=".err";
            ShowControlBar( &m_wndErrors, TRUE, FALSE);	
            m_wndErrors.StartFileLog(parserLogFile);
	        m_wndBrowser.ExecuteCommand(CString("update_module $dis_build_list"));
            m_wndErrors.StopFileLog();
		    SetCursor(save);
		}
	} else MessageBox("No outdated files in the current selection.","Information Model Update",MB_ICONINFORMATION|MB_OK);
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// When we are closing our application we need to save tge status and 
// position of all toolbars.
//------------------------------------------------------------------------------
void CMainFrame::OnClose()  {
	if(m_wndDormant    != NULL) m_wndDormant->SendMessage(WM_CLOSE,0,0);
	if(m_wndQA         != NULL) m_wndQA->SendMessage(WM_CLOSE,0,0);
	if(m_wndAccess     != NULL) m_wndAccess->SendMessage(WM_CLOSE,0,0);
	if(m_wndSimplify   != NULL) m_wndSimplify->SendMessage(WM_CLOSE,0,0);
	if(m_wndImpact     != NULL) m_wndImpact->SendMessage(WM_CLOSE,0,0);
	SaveBarState("DiscoverControlBarState");
	CMDIFrameWnd::OnClose();
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// There is nothing to update if nothing selected.
//------------------------------------------------------------------------------------------
void CMainFrame::OnUpdateUpdate(CCmdUI* pCmdUI)  {
   pCmdUI->Enable(m_SelectionSize>0);
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// This command will evaluate "dis_interactive_simplify" which will call Javs client
// to show simplify UI. This is not the best solution - Java client will run on the
// server side. Hope we will change this in future.
//------------------------------------------------------------------------------------------
void CMainFrame::OnSimplify()  {
   if(m_wndSimplify==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CSimplifyChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CSimplifyChildFrame)));
      m_wndSimplify=(CSimplifyChildFrame *) pObject;
      if(m_wndSimplify->LoadFrame(IDR_SIMPLIFY, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, this,NULL)) 
			m_wndSimplify->ShowWindow(SW_SHOW);
	  else 
		  m_wndSimplify = NULL;
   }
   if(m_wndSimplify != NULL) {
		MDIActivate(m_wndSimplify);
		SetDiscoverName();
   }
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// This command enabler will disable simplify if no selection
//------------------------------------------------------------------------------------------
void CMainFrame::OnUpdateSimplify(CCmdUI* pCmdUI)  {
   pCmdUI->Enable((m_SelectionSize>0) && m_wndBrowser.SelectionHasKind("module") && m_Lic[LIC_SIMPLIFY]);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// This command will evaluate "dis_interactive_simplify" which will call Javs client
// to show simplify UI. This is not the best solution - Java client will run on the
// server side. Hope we will change this in future.
//------------------------------------------------------------------------------------------
void CMainFrame::OnImpact()  {
   if(m_wndImpact==NULL) {
      CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CImpactChildFrame);
      CObject* pObject = pRuntimeClass->CreateObject();
      ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CImpactChildFrame)));
      m_wndImpact=(CImpactChildFrame *) pObject;
      m_wndImpact->LoadFrame(IDR_IMPACT, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE | WS_VISIBLE, this,NULL); 
	  m_wndImpact->SetEditorCtrl((CCodeRoverIntegrator*)m_wndBrowser.GetDlgItem(IDC_CODEROVERINTEGRATORCTRL));
   } else 
	  m_wndImpact->Update();
   MDIActivate(m_wndImpact);
   SetDiscoverName();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// This command enabler will disable simplify if no selection
//------------------------------------------------------------------------------------------
void CMainFrame::OnUpdateImpact(CCmdUI* pCmdUI)  {
   pCmdUI->Enable(m_SelectionSize>0);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Will change the cursor shape when some huge operation runs.
//------------------------------------------------------------------------------------------
BOOL CMainFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)  {
	if(m_Waiting==TRUE) {
		SetCursor(::LoadCursor(NULL, IDC_WAIT));
	    return TRUE;
	} else return CMDIFrameWnd::OnSetCursor(pWnd, nHitTest, message);
}
//------------------------------------------------------------------------------------------

//========================================================================================







//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CMainFrame::OnViewBrowser()  {
	ShowControlBar( &m_wndBrowser, !m_wndBrowser.IsWindowVisible(), FALSE);
}
//------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CMainFrame::OnUpdateViewBrowser(CCmdUI* pCmdUI)  {
	pCmdUI->SetCheck(m_wndBrowser.IsWindowVisible());
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CMainFrame::OnViewToolbar()  {
	ShowControlBar( &m_wndBaseToolbar, !m_wndBaseToolbar.IsWindowVisible(), FALSE);

}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CMainFrame::OnUpdateViewToolbar(CCmdUI* pCmdUI)  {
	pCmdUI->SetCheck(m_wndBaseToolbar.IsWindowVisible());
	
}
//------------------------------------------------------------------------------------------


/*
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CMainFrame::OnViewGraphbar()  {
	ShowControlBar(&m_wndViewsToolbar,!m_wndViewsToolbar.IsWindowVisible(),FALSE);
	
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CMainFrame::OnUpdateViewGraphbar(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(m_wndViewsToolbar.IsWindowVisible());
	
}
//------------------------------------------------------------------------------------------
*/

void CMainFrame::OnUpdateQa(CCmdUI* pCmdUI)  {
   pCmdUI->Enable(m_ServerConnected  && m_Lic[LIC_QA]>0);

}

void CMainFrame::OnUpdateAccess(CCmdUI* pCmdUI)  {
   pCmdUI->Enable(m_ServerConnected && m_Lic[LIC_ACCESS]>0);	
}

void CMainFrame::OnUpdateXmlreport(CCmdUI* pCmdUI) {
   pCmdUI->Enable(m_ServerConnected  && m_Lic[LIC_AUTODOC]>0);	
	
}


void CMainFrame::DockControlBarLeftOf(CToolBar* Bar,CToolBar* LeftOf)
{
	CRect rect;
	DWORD dw;
	UINT n;

	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout();
	LeftOf->GetWindowRect(&rect);
	rect.OffsetRect(1,0);
	dw=LeftOf->GetBarStyle();
	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;

	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line.  By calculating a rectangle, we in effect
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar,n,&rect);
}

void CMainFrame::OnCallgraph()  {
CCallGraphDlg dlg;

    dlg.DoModal();
	
}

void CMainFrame::OnUpdateCallgraph(CCmdUI* pCmdUI) 
{
   pCmdUI->Enable(m_ServerConnected && (m_SelectionSize>0) && (m_wndBrowser.SelectionHasKind("funct")));	
	
}

void CMainFrame::OnViewErrors()  {
   ShowControlBar( &m_wndErrors, !m_wndErrors.IsWindowVisible(), FALSE);	
}

void CMainFrame::OnUpdateViewErrors(CCmdUI* pCmdUI)  {
	pCmdUI->SetCheck(m_wndErrors.IsWindowVisible());
	
}

void CMainFrame::SetDiscoverName() {
    CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
	CString base;
	base.LoadString(IDR_MAINFRAME);
	int i;
	int c = 0;
	CString name=app->m_ServiceName;
	if(name.GetLength()>0) {
	   base+="-";
	   for(i=0;i<name.GetLength();i++) {
		   if(name[i]==':') {
			   if(c == 1) break;
			   else c++;
		   }
	   }
	}
	int sz = name.GetLength()-i-1;
	if(sz>0)   name=name.Right(name.GetLength()-i-1);
	base+=name;
	SetWindowText(base);
}

void CMainFrame::OnUpdateEditorSetup(CCmdUI* pCmdUI) 
{
   pCmdUI->Enable(true);	
	
}

void CMainFrame::OnEditorSetup()  {
   m_wndBrowser.OnEditorSetup();
	
}

void CMainFrame::OnUpdateCMSetup(CCmdUI* pCmdUI) 
{
	m_wndBrowser.OnUpdateCMSetup(pCmdUI);
}

void CMainFrame::OnCMSetup()  {
	m_wndBrowser.OnCMSetup();
}

void CMainFrame::OnCMAction(int id) {
	m_wndBrowser.OnCMAction(id);
}

LRESULT CMainFrame::OnUpdateGroups(WPARAM wParam, LPARAM lParam) {
	CGroupsCombo* pGroups = (CGroupsCombo *)m_wndBrowser.GetDlgItem(IDC_GROUPS);
	if(pGroups!=NULL) 
		pGroups->Update();
	return 0;
}

LRESULT CMainFrame::OnUpdateErrors(WPARAM wParam, LPARAM lParam) {
	m_wndErrors.UpdateFile();
	return 0;
}

void CMainFrame::OnDestroy() 
{
    Preferences* pPrefs = ((CDiscoverMDIApp *)AfxGetApp())->GetPreferences();
	pPrefs->set("CM",m_wndBrowser.GetCM()); 
	pPrefs->set("Editor",m_wndBrowser.GetEditor()); 
	
	CMDIFrameWnd::OnDestroy();	
}
