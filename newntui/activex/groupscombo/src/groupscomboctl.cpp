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
// GroupsComboCtl.cpp : Implementation of the CGroupsComboCtrl OLE control class.

#include "stdafx.h"
#include "GroupsCombo.h"
#include "GroupsComboCtl.h"
#include "GroupsComboPpg.h"
#include "NewGroupDlg.h"
#include "GroupsSearch.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CString oldLabel;

IMPLEMENT_DYNCREATE(CGroupsComboCtrl, COleControl)

/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CGroupsComboCtrl, COleControl)
	//{{AFX_MSG_MAP(CGroupsComboCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED,IDC_GROUPSLIST,OnGroupChanged) 
	ON_MESSAGE(WM_USERSELECTION,OnUserSelection)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)


	// Change the way the taget will present the results. We will show all elements
	// from all selected groups.Will Fire the ChangeView event with GR_SHOWALL parm.
	ON_COMMAND(IDB_ALL,         OnShowAll)
	// Change the way the taget will present the results. We will show selected groups
	// intersection elements. Will Fire the ChangeView event with GR_SHOWINTERSECT param.
	ON_COMMAND(IDB_INTERSECT,   OnShowIntersection)
	// Change the way the taget will present the results. We will show elements not 
	// common for the selected groups. Will Fire the ChangeView event with 
	// GR_SHOWDIFF param.
	ON_COMMAND(IDB_DIFF,        OnShowDiff)

	// Open dialog to select new group name and new group type.
	// Creates new group if not in a list and if user doesn't cancel the creation
	ON_COMMAND(IDB_NEWGROUP,      OnNewGroup)

	// Removes all selected groups
	ON_COMMAND(IDB_REMOVEGROUP,   OnRemoveGroup)

	// Adds all elements in the server selection variable to all selected groups
	ON_COMMAND(IDB_INSERTSEL,     OnAddSel)
	// Removes all elements defined in the server selection variable from all selected
	// groups
	ON_COMMAND(IDB_REMOVESEL,     OnRemoveSel)

	ON_COMMAND(IDB_FINDFIRSTGROUP,    OnFindFirst)
	ON_COMMAND(IDB_FINDNEXTGROUP,     OnFindNext)

    // In-place group name editing...
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_GROUPSLIST,OnItemEditStart)
	ON_NOTIFY(LVN_ENDLABELEDIT,   IDC_GROUPSLIST,OnItemEditEnd)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CGroupsComboCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CGroupsComboCtrl)
	DISP_PROPERTY_EX(CGroupsComboCtrl, "SelectionVariable", GetSelectionVariable, SetSelectionVariable, VT_BSTR)
	DISP_PROPERTY_EX(CGroupsComboCtrl, "DataSource", GetDataSource, SetDataSource, VT_DISPATCH)
	DISP_PROPERTY_EX(CGroupsComboCtrl, "OutputVariable", GetOutputVariable, SetOutputVariable, VT_BSTR)
	DISP_PROPERTY_EX(CGroupsComboCtrl, "ControlsEnable", GetControlsEnable, SetControlsEnable, VT_BOOL)
	DISP_FUNCTION(CGroupsComboCtrl, "Update", Update, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CGroupsComboCtrl, "GetSelectedGroups", GetSelectedGroups, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CGroupsComboCtrl, "CreateGroup", CreateGroup, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CGroupsComboCtrl, "AddElements", AddElements, VT_EMPTY, VTS_NONE)
	DISP_STOCKPROP_CAPTION()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CGroupsComboCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CGroupsComboCtrl, COleControl)
	//{{AFX_EVENT_MAP(CGroupsComboCtrl)
	EVENT_CUSTOM("GroupChanged", FireGroupChanged, VTS_BSTR)
	EVENT_CUSTOM("GroupCreated", FireGroupCreated, VTS_BSTR)
	EVENT_CUSTOM("GroupRemoved", FireGroupRemoved, VTS_I2)
	EVENT_CUSTOM("GroupRenamed", FireGroupRenamed, VTS_BSTR)
	EVENT_CUSTOM("SelectionInserted", FireSelectionInserted, VTS_NONE)
	EVENT_CUSTOM("SelectionRemoved", FireSelectionRemoved, VTS_NONE)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CGroupsComboCtrl, 1)
	PROPPAGEID(CGroupsComboPropPage::guid)
END_PROPPAGEIDS(CGroupsComboCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CGroupsComboCtrl, "GROUPSCOMBO.GroupsComboCtrl.1",
	0x1487dd54, 0x1cb6, 0x11d2, 0xae, 0xf0, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CGroupsComboCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DGroupsCombo =
		{ 0x1487dd52, 0x1cb6, 0x11d2, { 0xae, 0xf0, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DGroupsComboEvents =
		{ 0x1487dd53, 0x1cb6, 0x11d2, { 0xae, 0xf0, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwGroupsComboOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CGroupsComboCtrl, IDS_GROUPSCOMBO, _dwGroupsComboOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CGroupsComboCtrl::CGroupsComboCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CGroupsComboCtrl

BOOL CGroupsComboCtrl::CGroupsComboCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_GROUPSCOMBO,
			IDB_GROUPSCOMBO,
			afxRegInsertable | afxRegApartmentThreading,
			_dwGroupsComboOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CGroupsComboCtrl::CGroupsComboCtrl - Constructor

CGroupsComboCtrl::CGroupsComboCtrl()
{
	InitializeIIDs(&IID_DGroupsCombo, &IID_DGroupsComboEvents);

	// TODO: Initialize your control's instance data here.
	m_DataSource=NULL;
	m_ControlsEnable = TRUE;
	m_Search = "";
	m_FirstFound = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CGroupsComboCtrl::~CGroupsComboCtrl - Destructor

CGroupsComboCtrl::~CGroupsComboCtrl()
{
	// TODO: Cleanup your control's instance data here.
}

//-----------------------------------------------------------------------------------------
// This function will syncronously evaluate script stored in the string "command" on the
// server or mini-server. It will use DataSource IDispatch interface to run AccessSync
// command.
// Returns EVAL_OK if the command was evaluated successfully or :
//     ERROR_DATA_SOURCE    if we were trying to evaluate the command without IDispatch 
//                          interface
//     ERROR_ACCESS_SUPPORT if the interface doesn't support AccessSync method
//     ERROR_ACCESS_COMMAND if the wrong command was passed to evaluate     
//-----------------------------------------------------------------------------------------
int CGroupsComboCtrl::EvaluateSync(CString& command, CString* results) {
CString MethodName("AccessSync"); // We will run AccessSync method of the DataSource
DISPID accessID;                  // IDispatch id for the AccessSync command
HRESULT hresult;

    // if there is no data source assigned to this control, we will be unable to
    // evaluate this function.
    if(m_DataSource==NULL) return ERROR_DATA_SOURCE;
	if(results == NULL) command+=";set tmp 1";

    // Trying to receve method ID from it's name.
	BSTR methodBSTR=MethodName.AllocSysString();
    hresult = m_DataSource->GetIDsOfNames(IID_NULL,&methodBSTR, 
		                                  1,LOCALE_SYSTEM_DEFAULT,&accessID);

	// Interface doesn't support AccessSync method
	if(hresult!=S_OK) return ERROR_ACCESS_SUPPORT;

	// Converting command into new OEM string wich will be passed into OLE automation.
	// We will free when pushing into the stack
	BSTR access_call = command.AllocSysString();

	// Creating parameter structure - only one parameter will be passed as argument - command string
    DISPPARAMS cmdParamStruct;
	cmdParamStruct.rgvarg              = new VARIANT;
	cmdParamStruct.rgvarg[0].vt        = VT_BSTR | VT_BYREF; // we will pass it as BSTR reference
	cmdParamStruct.rgvarg[0].pbstrVal  = &access_call;

	cmdParamStruct.cArgs               = 1;
	cmdParamStruct.cNamedArgs=0;

    // Results will be inserted into VARIANT union
	VARIANT resBSTR;
	// This will show the argument with an error
	unsigned int argWithError;
    // Remote command execution
	hresult = m_DataSource->Invoke(accessID,
		                           IID_NULL,
								   LOCALE_SYSTEM_DEFAULT,
								   DISPATCH_METHOD,
								   &cmdParamStruct,
								   &resBSTR,
								   NULL,
								   &argWithError);
	delete cmdParamStruct.rgvarg;

	if(hresult!=S_OK) return ERROR_ACCESS_COMMAND;
	if(results!=NULL) *results=resBSTR.bstrVal;
	return EVAL_OK;
}
//------------------------------------------------------------------------------------------



/////////////////////////////////////////////////////////////////////////////
// CGroupsComboCtrl::OnDraw - Drawing function

void CGroupsComboCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid) {
	pdc->FillRect(rcInvalid,&CBrush(GetSysColor(COLOR_BTNFACE)));
}


/////////////////////////////////////////////////////////////////////////////
// CGroupsComboCtrl::DoPropExchange - Persistence support

void CGroupsComboCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CGroupsComboCtrl::OnResetState - Reset control to default state

void CGroupsComboCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CGroupsComboCtrl::AboutBox - Display an "About" box to the user

void CGroupsComboCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_GROUPSCOMBO);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CGroupsComboCtrl message handlers


//---------------------------------------------------------------------------------
// Creates NewSelectionList list vier to show complete set of groups and toolbar 
// control which will select the the mode to obtain elements.
//---------------------------------------------------------------------------------
int CGroupsComboCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// List view to show all available groups
	m_ElementsList.Create( LVS_SHAREIMAGELISTS | 
		               LVS_LIST | 
					   WS_CHILD | 
					   WS_VISIBLE | 
					   LVS_NOCOLUMNHEADER | 
					   LVS_AUTOARRANGE |
					   LVS_SHOWSELALWAYS |
					   LVS_SORTASCENDING |
					   LVS_EDITLABELS,
		               CRect(lpCreateStruct->x, 
		                     lpCreateStruct->y,
			                 lpCreateStruct->x+lpCreateStruct->cx,
					         lpCreateStruct->y+lpCreateStruct->cy-10),
			       this,
				   IDC_GROUPSLIST); 
	m_ElementsList.ModifyStyleEx( 0, WS_EX_CLIENTEDGE);

    // We will need only one column
    LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvColumn.fmt=LVCFMT_LEFT;
	lvColumn.cx=150;
	lvColumn.iSubItem=0;
	lvColumn.pszText="Groups";
    m_ElementsList.InsertColumn(0,&lvColumn);
    // And we will need image list with only one image which represents the group
	m_ListImageList.Create(16,16,TRUE,1,0);
	CBitmap group;
	group.LoadBitmap(IDB_GROUP);
	m_ListImageList.Add(&group, RGB(255,255,255));
	m_ElementsList.SetImageList(&m_ListImageList,LVSIL_SMALL); 

	
    // Toolbar creation
    m_OpTools.Create(WS_CHILD | WS_VISIBLE | CCS_BOTTOM,
		                CRect(lpCreateStruct->x, 
		                      lpCreateStruct->y+lpCreateStruct->cy-0,
			                  lpCreateStruct->x+lpCreateStruct->cx,
					          lpCreateStruct->y+lpCreateStruct->cy),
			          this,
				      IDC_GROUPTOOLS);
	m_OpTools.SetBitmapSize(CSize(16,10));
	m_OpTools.SetButtonSize(CSize(18,12));


    //Adding buttons to the toolbar
    TBBUTTON tbbutton;

    tbbutton.iBitmap  = 0;
    tbbutton.idCommand = IDB_ALL;
    tbbutton.fsState  = TBSTATE_ENABLED | TBSTATE_CHECKED;
    tbbutton.fsStyle  = TBSTYLE_CHECKGROUP;
    tbbutton.iString  = 0; 
	m_OpTools.AddButtons(1,&tbbutton);

    tbbutton.iBitmap  = 1;
    tbbutton.idCommand = IDB_INTERSECT;
    tbbutton.fsState  = TBSTATE_ENABLED;
    tbbutton.fsStyle  = TBSTYLE_CHECKGROUP;
    tbbutton.iString  = 1; 
	m_OpTools.AddButtons(1,&tbbutton);

    tbbutton.iBitmap  = 2;
    tbbutton.idCommand = IDB_DIFF;
    tbbutton.fsState  = TBSTATE_ENABLED;
    tbbutton.fsStyle  = TBSTYLE_CHECKGROUP;
    tbbutton.iString  = 2; 
	m_OpTools.AddButtons(1,&tbbutton);

    tbbutton.iBitmap  = -1;
    tbbutton.idCommand = -1;
    tbbutton.fsState  = 0;
    tbbutton.fsStyle  = TBSTYLE_SEP;
    tbbutton.iString  = -1; 
	m_OpTools.AddButtons(1,&tbbutton);

	tbbutton.iBitmap   = 3;
    tbbutton.idCommand = IDB_NEWGROUP;
    tbbutton.fsState   = TBSTATE_ENABLED;
    tbbutton.fsStyle   = TBSTYLE_BUTTON;
    tbbutton.iString   = 3; 
	m_OpTools.AddButtons(1,&tbbutton);

	tbbutton.iBitmap   = 4;
    tbbutton.idCommand = IDB_REMOVEGROUP;
    tbbutton.fsState   = TBSTATE_ENABLED;
    tbbutton.fsStyle   = TBSTYLE_BUTTON;
    tbbutton.iString   = 4; 
	m_OpTools.AddButtons(1,&tbbutton);

    tbbutton.iBitmap  = -1;
    tbbutton.idCommand = -1;
    tbbutton.fsState  = 0;
    tbbutton.fsStyle  = TBSTYLE_SEP;
    tbbutton.iString  = -1; 
	m_OpTools.AddButtons(1,&tbbutton);

	
	tbbutton.iBitmap   = 5;
    tbbutton.idCommand = IDB_REMOVESEL;
    tbbutton.fsState   = TBSTATE_ENABLED;
    tbbutton.fsStyle   = TBSTYLE_BUTTON;
    tbbutton.iString   = 5; 
	m_OpTools.AddButtons(1,&tbbutton);


    tbbutton.iBitmap  = -1;
    tbbutton.idCommand = -1;
    tbbutton.fsState  = 0;
    tbbutton.fsStyle  = TBSTYLE_SEP;
    tbbutton.iString  = -1; 
	m_OpTools.AddButtons(1,&tbbutton);

	
	tbbutton.iBitmap   = 6;
    tbbutton.idCommand = IDB_FINDFIRSTGROUP;
    tbbutton.fsState   = TBSTATE_ENABLED;
    tbbutton.fsStyle   = TBSTYLE_BUTTON;
    tbbutton.iString   = 6; 
	m_OpTools.AddButtons(1,&tbbutton);

	tbbutton.iBitmap   = 7;
    tbbutton.idCommand = IDB_FINDNEXTGROUP;
    tbbutton.fsState   = TBSTATE_ENABLED;
    tbbutton.fsStyle   = TBSTYLE_BUTTON;
    tbbutton.iString   = 7; 
	m_OpTools.AddButtons(1,&tbbutton);



	// Adding bitmaps to the toolbar
	m_OpTools.AddBitmap(1,IDB_ALL);
	m_OpTools.AddBitmap(1,IDB_INTERSECT);
	m_OpTools.AddBitmap(1,IDB_DIFF);
	m_OpTools.AddBitmap(1,IDB_NEWGROUP);
	m_OpTools.AddBitmap(1,IDB_REMOVEGROUP);
	m_OpTools.AddBitmap(1,IDB_REMOVESEL);
	m_OpTools.AddBitmap(1,IDB_FINDFIRSTGROUP);
	m_OpTools.AddBitmap(1,IDB_FINDNEXTGROUP);



    
   // Tool tips for the toolbar
   CRect buttonRect;
   m_ToolTips.Create(this);

   m_OpTools.GetItemRect(0,buttonRect);
   m_ToolTips.AddTool(&m_OpTools,IDS_ALLELEMETS,buttonRect,IDB_ALL);

   m_OpTools.GetItemRect(1,buttonRect);
   m_ToolTips.AddTool(&m_OpTools,IDS_DIFFELEMENTS,buttonRect,IDB_DIFF);

   m_OpTools.GetItemRect(2,buttonRect);
   m_ToolTips.AddTool(&m_OpTools,IDS_COMMONELEMENTS,buttonRect,IDB_INTERSECT);

   m_OpTools.GetItemRect(4,buttonRect);
   m_ToolTips.AddTool(&m_OpTools,IDS_NEWGROUP,buttonRect,IDB_NEWGROUP);

   m_OpTools.GetItemRect(5,buttonRect);
   m_ToolTips.AddTool(&m_OpTools,IDS_REMOVEGROUP,buttonRect,IDB_REMOVEGROUP);

   m_OpTools.GetItemRect(7,buttonRect);
   m_ToolTips.AddTool(&m_OpTools,IDS_REMOVESEL,buttonRect,IDB_REMOVESEL);

   m_OpTools.GetItemRect(9,buttonRect);
   m_ToolTips.AddTool(&m_OpTools,IDS_FINDFIRSTGROUP,buttonRect,IDB_FINDFIRSTGROUP);

   m_OpTools.GetItemRect(10,buttonRect);
   m_ToolTips.AddTool(&m_OpTools,IDS_FINDNEXTGROUP,buttonRect,IDB_FINDNEXTGROUP);


   m_OpTools.SetToolTips(&m_ToolTips);
 
   SetToolbarStatus();
   return 0;
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Callback runs every time WM_SIZE message comes to the control (When window 
// creating and when window changing it's size). Position ElementsList in the
// window. No need to position toolbar, it will be done automatically.
//----------------------------------------------------------------------------------
void CGroupsComboCtrl::OnSize(UINT nType, int cx, int cy)  {
CRect toolsRect;

	COleControl::OnSize(nType, cx, cy);
	if(m_ControlsEnable == TRUE) {
	   if(::IsWindow(m_OpTools)) {
		   m_OpTools.GetWindowRect(toolsRect);
		   m_OpTools.MoveWindow(0,cy-toolsRect.Height(),cx,toolsRect.Height());
	       if(::IsWindow(m_ElementsList)) {
		       m_ElementsList.MoveWindow(::GetSystemMetrics(SM_CYEDGE),::GetSystemMetrics(SM_CYEDGE),cx-2*::GetSystemMetrics(SM_CYEDGE),cy-2*::GetSystemMetrics(SM_CYEDGE)-toolsRect.Height(),TRUE);
		   }
	   } else {
	      if(::IsWindow(m_ElementsList)) {
		      m_ElementsList.MoveWindow(::GetSystemMetrics(SM_CYEDGE),::GetSystemMetrics(SM_CYEDGE),cx-2*::GetSystemMetrics(SM_CYEDGE),cy-2*::GetSystemMetrics(SM_CYEDGE),TRUE);
		  }
	   }
	} else {
	   m_ElementsList.MoveWindow(0,0,cx,cy,TRUE);
	}
}
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
// PROPERTY. Returns IDispatch interface to the DataSource object which this control
// is using to communicate with server.
//----------------------------------------------------------------------------------
LPDISPATCH CGroupsComboCtrl::GetDataSource()  {

	return m_DataSource;
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// PROPERTY. Sets IDispatch interface to the DataSource object which this control
// will use to communicate with server,
//----------------------------------------------------------------------------------
void CGroupsComboCtrl::SetDataSource(LPDISPATCH newValue)  {
	m_DataSource = newValue;
	FillList();
}
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// PROPERTY. Returns the name of the server variable which this control is using
// to add/remove selections.
//----------------------------------------------------------------------------------
BSTR CGroupsComboCtrl::GetSelectionVariable() {
	return m_SelectionVar.AllocSysString();
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// PROPERTY. Sets the name of the server variable which this control will use to
// add/remove selections.
//----------------------------------------------------------------------------------
void CGroupsComboCtrl::SetSelectionVariable(LPCTSTR lpszNewValue)  {
	m_SelectionVar = lpszNewValue;
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Callback which generates event. Runs every time the selection changed or added. 
// Pass the string with groups names separated by {}.
//----------------------------------------------------------------------------------
void CGroupsComboCtrl::OnGroupChanged(NMHDR* pNMHDR, LRESULT* pResult) {
NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// Item status changed from unselected to selected or from selected to unselected
    if( (pNMListView->uOldState&LVIS_SELECTED)!=
		(pNMListView->uNewState&LVIS_SELECTED) ) {
		   m_InSelection=TRUE;
           SetToolbarStatus();
	}
}
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// Callback runs when the editor send WM_USERSELECTION event. Forms the group names
// string with groups separated by {} and sends this string to the container.
//----------------------------------------------------------------------------------
LRESULT CGroupsComboCtrl::OnUserSelection(WPARAM,LPARAM) {

	if(m_InSelection==TRUE) {
        HCURSOR save = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
        m_GroupsList="";
	    for(register i=0; i<m_ElementsList.GetItemCount();i++) {
		   if(m_ElementsList.GetItemState(i,LVIS_SELECTED)!=0) {
		      m_GroupsList+="{";
			  m_GroupsList+=m_ElementsList.GetItemText(i,0);
			  m_GroupsList+='}';
		   }
		}
		FillElementsList();
		FireGroupChanged(m_GroupsList);
	    m_InSelection=FALSE;
		SetToolbarStatus();
		::SetCursor(save);
	}
	return 0;
}
//------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Query server for all available groups. Fills the list with returned groups name.
//----------------------------------------------------------------------------------- 
void CGroupsComboCtrl::FillList(void) {
register i;
CString serverCall;
CString serverResults;
CString spaces;


    // Removes all items from the list
    m_ElementsList.DeleteAllItems();


    // Queries the server for available categories for the spesified selection.
    serverCall.Format("group info");
	EvaluateSync(serverCall,&serverResults);

    // serverResults string now will contain the complete list of redable
	// group names separated by spaces. ***No spacves in the group name!

	// Now we will fill the list with groups names, extracting them from 
	// the "serverResults". 

	int     line   = 0;
	BOOL    inside = FALSE;
	CString name   ="";
	for(i=0;i<serverResults.GetLength();i++) {
		// Entering group name
		if(serverResults[i]!=' ') {
			inside=TRUE;
		}
		// Adding new symbol to the group
		if(inside==TRUE && serverResults[i]!=' ') name+=serverResults[i];

		// Exiting query substring - need to insert results into the list
		if((serverResults[i]==' ' || i==serverResults.GetLength()-1) && inside==TRUE) {
			inside=FALSE;
			// Inserting new item into the combobox
	        m_ElementsList.InsertItem(i,name,0);
			name = "";
		}
	}
	// Different group/groups selection will automatically re-calculate the 
	// selected elements.
    if(m_ElementsList.GetSelectedCount()>0) FillElementsList();
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Updates complete window re-quering data from the server.
//-----------------------------------------------------------------------------------
void CGroupsComboCtrl::Update()  {
	FillList();
    SetToolbarStatus();
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Callback runs when "Show All" button selected. Fills the output server variable 
// with all elements in all selected groups.
//------------------------------------------------------------------------------------
void CGroupsComboCtrl::OnShowAll() {
	FillElementsList();
	FireGroupChanged(m_GroupsList);
}
//-----------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Callback runs when "Intersect" button selected. Fills the output server variable
// with intersected elements in the all selected groups.
//------------------------------------------------------------------------------------
void CGroupsComboCtrl::OnShowIntersection() {
	FillElementsList();
	FireGroupChanged(m_GroupsList);
}
//-----------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Callback runs when "Difference" button selected. Fills the output server variable
// with all non-common elements in the all selected groups.
//------------------------------------------------------------------------------------
void CGroupsComboCtrl::OnShowDiff() {
	FillElementsList();
	FireGroupChanged(m_GroupsList);
}
//------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Callback will run when user pressed new group button on the toolbar.
// Will ask the user group name and will create new group if possible. The list
// will be automatically re-filled and the selection will be placed on the new group.
// Will fire NewGroup event to the container.
//------------------------------------------------------------------------------------
void CGroupsComboCtrl::OnNewGroup() {
CString serverCall;
CString serverResults;
CString serverName;
CNewGroupDlg name;
   name.m_Scratch = 0;
   if(name.DoModal()==IDOK) {
	   // Will change "{" "}" or space into "_"
	   serverName="";
	   for(register i=0;i<name.m_Name.GetLength();i++) { 
		   if( name.m_Name[i] != '{' && name.m_Name[i]!= '}' && name.m_Name[i]!= ' '  && 
			   name.m_Name[i] != '~' && name.m_Name[i]!= '@' && name.m_Name[i]!= '#'  && 
			   name.m_Name[i] != '$' && name.m_Name[i]!= '%' && name.m_Name[i]!= '^'  && 
			   name.m_Name[i] != '&' && name.m_Name[i]!= '*' && name.m_Name[i]!= '('  && 
			   name.m_Name[i] != ')' && name.m_Name[i]!= '+' && name.m_Name[i]!= '\\' && 
			   name.m_Name[i] != '|' && name.m_Name[i]!= '=' && name.m_Name[i]!= '`'  && 
			   name.m_Name[i] != '<' && name.m_Name[i]!= '>' && name.m_Name[i]!= '/') 
			   serverName+=name.m_Name[i];
		   else
			   serverName+='_';
	   }
       // Maybe we are trying to create existing group ?   
	   serverCall.Format("group exists %s",serverName);
	   EvaluateSync(serverCall,&serverResults);
	   if(atoi(serverResults)!=0) {
		   CString errStr;
		   errStr.Format("The group with the name \"%s\" already exists!",serverName);
		   MessageBox(errStr,"Group Manager Error",MB_ICONSTOP);
		   return;
	   }
	   // Creating new group in the database
	   if(name.m_Scratch!=0) {
	      serverCall.Format("group new %s",serverName);
	      EvaluateSync(serverCall);
	      serverCall.Format("group save %s",serverName);
	      EvaluateSync(serverCall);
	   } else {
	      serverCall.Format("group tmp %s",serverName);
	      EvaluateSync(serverCall);
	   }

	   m_ElementsList.InsertItem(0,serverName,0);


       // We will select newly created group
       LV_FINDINFO findWhat; 
       findWhat.flags = LVFI_STRING;
	   findWhat.psz   = serverName;
	   int thisGroup = m_ElementsList.FindItem(&findWhat);

	   // Removing all old selections
	   for(i=0; i<m_ElementsList.GetItemCount();i++) 
          m_ElementsList.SetItemState(i,~LVIS_SELECTED,LVIS_SELECTED);
	   // Selecting current item
	   if(thisGroup>=0)
	       m_ElementsList.SetItemState(thisGroup,LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED);

       m_GroupsList="";
	   for(i=0; i<m_ElementsList.GetItemCount();i++) {
          if(m_ElementsList.GetItemState(i,LVIS_SELECTED)!=0) {
		     m_GroupsList+="{";
		     m_GroupsList+=m_ElementsList.GetItemText(i,0);
		     m_GroupsList+='}';
	         m_ElementsList.SetItemState(thisGroup,0,LVIS_SELECTED);
		   }
	   }
	   SetToolbarStatus();
	   FillElementsList();
	   FireGroupCreated(serverName);
   }
   
}
//------------------------------------------------------------------------------------




//------------------------------------------------------------------------------------
// Will remove all selected groups. Will ask to confirm the removing if there are 
// multiple groups selected.
// Will fire RemoveGroups event with the removed groups counter as a  parameter.
//------------------------------------------------------------------------------------
void CGroupsComboCtrl::OnRemoveGroup() {
int selectionsAmount;

   selectionsAmount= m_ElementsList.GetSelectedCount();
    if(selectionsAmount==0) {
	   MessageBox("No groups selected to remove!","Group Manager Error",MB_ICONSTOP);
	   return;
	} 
	if(selectionsAmount>1) {
	   CString prompt;
	   prompt.Format("You are going to remove %d groups\nAre you shure?",selectionsAmount);
	   if(MessageBox(prompt,"Group Manager Warning",MB_YESNO | MB_ICONQUESTION)==IDNO)
		   return;
	}
	for(register i=m_ElementsList.GetItemCount()-1; i>=0; i--) {
		if(m_ElementsList.GetItemState(i,LVIS_SELECTED)!=0) {
			CString serverRequest;
			serverRequest.Format("group delete %s",m_ElementsList.GetItemText(i,0));
			m_ElementsList.DeleteItem(i);
			EvaluateSync(serverRequest);
		}
	}
	SetToolbarStatus();
	if(m_ElementsList.GetSelectedCount()>0) FillElementsList();

    m_GroupsList="";
	for(i=0; i<m_ElementsList.GetItemCount();i++) {
       if(m_ElementsList.GetItemState(i,LVIS_SELECTED)!=0) {
		  m_GroupsList+="{";
		  m_GroupsList+=m_ElementsList.GetItemText(i,0);
		  m_GroupsList+='}';
		}
	}
	FillElementsList();
	FireGroupRemoved(selectionsAmount);
}
//-----------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// If the server selection variable is set, takes all selected elements and adds them
// to all selected groups.
//------------------------------------------------------------------------------------
void CGroupsComboCtrl::OnAddSel() {
register i;
CString serverQuery;
   // No need to add something if selection variable is not available
   if(m_SelectionVar.GetLength()==0) return;

   // Searching all  selected groups
   for(i=0;i<m_ElementsList.GetItemCount();i++) {
      if(m_ElementsList.GetItemState(i,LVIS_SELECTED)!=0) {
	     serverQuery.Format("group add %s $%s",
				             m_ElementsList.GetItemText(i,0),m_SelectionVar);
		 EvaluateSync(serverQuery);
	  }
   }
   FillElementsList();
   FireSelectionInserted();
}
//-----------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// If the server selection variable is set, takes all selected elements and removes 
// them from all selected groups, if possible.
//------------------------------------------------------------------------------------
void CGroupsComboCtrl::OnRemoveSel() {
register i;
CString serverQuery;
CString serverResults;

   // No need to remove something if selection variable is not available
   if(m_SelectionVar.GetLength()==0) return;

   // Searching all  selected groups
   for(i=0;i<m_ElementsList.GetItemCount();i++) {
      if(m_ElementsList.GetItemState(i,LVIS_SELECTED)!=0) {
	     serverQuery.Format("group subtract %s $%s",
				             m_ElementsList.GetItemText(i,0),m_SelectionVar);
		 EvaluateSync(serverQuery);
         serverQuery.Format("group kind %s",m_ElementsList.GetItemText(i,0));
		 EvaluateSync(serverQuery,&serverResults);
		 if(serverResults.Find("persistant")>=0) {
	         serverQuery.Format("group save %s",m_ElementsList.GetItemText(i,0));
		     EvaluateSync(serverQuery,&serverResults);
		 }
	  }
   }
   FillElementsList();
   FireSelectionRemoved();
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
void CGroupsComboCtrl::OnFindFirst() {
int i,j;
CGroupsSearch search;
   search.m_Text = m_Search;
   if(search.DoModal()==IDOK) {
	   m_Search = search.m_Text;
   }
   for(i=0;i<m_ElementsList.GetItemCount();i++) {
	   CString text = m_ElementsList.GetItemText(i,0);
       if(text.Find(m_Search)>=0) {
   	       for(j=0; j<m_ElementsList.GetItemCount(); j++) {
		       if(m_ElementsList.GetItemState(j,LVIS_SELECTED)!=0) {
				    m_ElementsList.SetItemState(j,0,LVIS_SELECTED);
			   }
		   }
		   m_ElementsList.EnsureVisible(i,FALSE);
		   m_ElementsList.SetItemState(i,LVIS_SELECTED,LVIS_SELECTED);
		   m_FirstFound=i;
 	       OnUserSelection(0,0);
		   return;
	   }
   }
   MessageBox("No groups with the matching names.","Groups search",MB_ICONINFORMATION);
   m_Search="";
   SetToolbarStatus();


}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
void CGroupsComboCtrl::OnFindNext() {
int i,j;
   for(i=m_FirstFound+1;i<m_ElementsList.GetItemCount();i++) {
	   CString text = m_ElementsList.GetItemText(i,0);
       if(text.Find(m_Search)>=0) {
   	       for(j=0; j<m_ElementsList.GetItemCount(); j++) {
		       if(m_ElementsList.GetItemState(j,LVIS_SELECTED)!=0) {
				    m_ElementsList.SetItemState(j,0,LVIS_SELECTED);
			   }
		   }
		   m_ElementsList.EnsureVisible(i,FALSE);
		   m_ElementsList.SetItemState(i,LVIS_SELECTED,LVIS_SELECTED);
		   m_FirstFound = i;
 	       OnUserSelection(0,0);
		   return;
	   }
   }
   MessageBox("No more groups with the matching names.","Groups search",MB_ICONINFORMATION);
   m_FirstFound=0;

}
//-----------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------
// Callabck runs every time the element editing process is ended.
// will not allow to change the group name to the empty one or to use the existing 
// group name.
//-------------------------------------------------------------------------------------
void CGroupsComboCtrl::OnItemEditStart(NMHDR* pNMHDR, LRESULT* pResult) {
LV_DISPINFO* info = (LV_DISPINFO *) pNMHDR;
   oldLabel=m_ElementsList.GetItemText(info->item.iItem,info->item.iSubItem);
}
//-------------------------------------------------------------------------------------





//-------------------------------------------------------------------------------------
// Callabck runs every time the element editing process is ended.
// will not allow to change the group name to the empty one or to use the existing 
// group name.
//-------------------------------------------------------------------------------------
void CGroupsComboCtrl::OnItemEditEnd(NMHDR* pNMHDR, LRESULT* pResult) {
LV_DISPINFO* info = (LV_DISPINFO *) pNMHDR;
CString newName;
CString serverCall;
CString serverResults;

   // Operation cancelled. 
   if(info->item.pszText==NULL) return;

   // Removing "{" "}" and spaces from the new name;
   newName="";
   for(register unsigned i=0;i<strlen(info->item.pszText);i++) {
      if( info->item.pszText[i] != '{' && 
		  info->item.pszText[i]!= '}' && 
		  info->item.pszText[i]!= ' ')  {
	           newName+=info->item.pszText[i];
	  } else {
	           newName+='_';
	  }
   }   

   // Maybe we are trying to create existing group ?   
   serverCall.Format("group exists %s",newName);
   EvaluateSync(serverCall,&serverResults);
   if(atoi(serverResults)!=0) {
      CString errStr;
	  errStr.Format("The group with the name \"%s\" already exists!",newName);
	  MessageBox(errStr,"Group Manager Error",MB_ICONSTOP);
	  m_ElementsList.SetItemText(info->item.iItem,info->item.iSubItem,oldLabel);
	  return;
   }
   serverCall.Format("group rename %s %s",oldLabel,newName);
   EvaluateSync(serverCall,&serverResults);
   m_ElementsList.SetItemText(info->item.iItem,info->item.iSubItem,newName);
   FireGroupRenamed(newName);
}
//-------------------------------------------------------------------------------------





//-------------------------------------------------------------------------------------
// This callback runs every time the client ask the name of the output variable.
// Output variable contains the elements in the selected group, if only one is selected,
// or results of the current group operation - diff, merge , intersect ...
//-------------------------------------------------------------------------------------
BSTR CGroupsComboCtrl::GetOutputVariable() {
	return m_OutputVar.AllocSysString();
}
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// This callback runs every time the client going to change the name of the output 
// variable.
// Output variable contains the elements in the selected group, if only one is selected,
// or results of the current group operation - diff, merge , intersect ...
//-------------------------------------------------------------------------------------
void CGroupsComboCtrl::SetOutputVariable(LPCTSTR lpszNewValue)  {
	m_OutputVar = lpszNewValue;
	FillElementsList();
}
//-------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------
// This function will add all groups elements to the group with name "name"
// It will scan groups list to detect all selected groups.
//-------------------------------------------------------------------------------------
void CGroupsComboCtrl::MakeGroupsConcat(CString& name) {
register i;
CString serverQuery;

   // Searching all  selected groups
   for(i=0;i<m_ElementsList.GetItemCount();i++) {
      if(m_ElementsList.GetItemState(i,LVIS_SELECTED)!=0) {
	     serverQuery.Format("group add %s group@%s",
				             name, m_ElementsList.GetItemText(i,0));
		 EvaluateSync(serverQuery);
	  }
   }
}
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// This function will calculate all elements which are common to the selected groups
// and add them to the group named "name"
// It will scan groups list to detect all selected groups.
//-------------------------------------------------------------------------------------
void CGroupsComboCtrl::MakeGroupsIntersect(CString& name) {
register i;
CString serverQuery;

    for(i=0;i<m_ElementsList.GetItemCount();i++) {
       if(m_ElementsList.GetItemState(i,LVIS_SELECTED)!=0) {
			 serverQuery.Format("group intersect %s group@%s",
				                 name,
								 m_ElementsList.GetItemText(i,0));
		   EvaluateSync(serverQuery);
	   }
   }

}
//-------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------
// This private function will fill the output variable which name is given in the
// m_OutputVariable member, if any, with the results of group operation or with the
// elements in the current group if only one group selected.
//-------------------------------------------------------------------------------------
void CGroupsComboCtrl::FillElementsList(void) {
CString serverQuery;
    // No need to fill any output server variable if the variable name wasn't set.
    if(m_OutputVar.GetLength()==0) return;

	// creating new temporary group which will store all operationd 
	// results.
    EvaluateSync(CString("group tmp __distmp__"));
    // If all elements button selected ...
    if((m_ControlsEnable==FALSE) || (m_OpTools.IsButtonChecked(IDB_ALL)!=0)) {
		MakeGroupsConcat(CString("__distmp__"));
    } else { 
	    // If intersect button selected
        if(m_OpTools.IsButtonChecked(IDB_INTERSECT)!=0) {
		   MakeGroupsConcat(CString("__distmp__"));
		   MakeGroupsIntersect(CString("__distmp__"));
		} else { // intersect END
	       // If diff button selected
           if(m_OpTools.IsButtonChecked(IDB_DIFF)!=0) {
			   // pass 1 : collecting all elements in a single group
			   MakeGroupsConcat(CString("__distmp__"));
			   // pass 2 : calculating elements intersection 
               EvaluateSync(CString("group tmp __distmp1__"));
               EvaluateSync(CString("group set __distmp1__ __distmp__"));
			   MakeGroupsIntersect(CString("__distmp1__"));
			   // pass 3 : calculating difference 
               EvaluateSync(CString("group subtract __distmp__ __distmp1__"));
			   // Removing group for temporary intersect stirage
	           EvaluateSync(CString("group delete __distmp1__"));
		   } 
		}
	}
    // coping data to the output variable
	serverQuery.Format("set %s [group get __distmp__]",m_OutputVar);
	EvaluateSync(serverQuery);

    // removing temporary group - all results are in the output variable now
	EvaluateSync(CString("group delete __distmp__"));
}
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// Enables or disables toolbar buttons according to the current selection and current
// elements amount.
//-------------------------------------------------------------------------------------
void CGroupsComboCtrl::SetToolbarStatus(void) {
    if(m_ControlsEnable==FALSE) return;
	if(m_ElementsList.GetItemCount()==0) {
		m_OpTools.EnableButton(IDB_ALL,            FALSE);
		m_OpTools.EnableButton(IDB_DIFF,           FALSE);
		m_OpTools.EnableButton(IDB_INTERSECT,      FALSE);
		m_OpTools.EnableButton(IDB_REMOVEGROUP,    FALSE);
		m_OpTools.EnableButton(IDB_INSERTSEL,      FALSE);
		m_OpTools.EnableButton(IDB_REMOVESEL,      FALSE);
		m_OpTools.EnableButton(IDB_FINDFIRSTGROUP, FALSE);
		m_OpTools.EnableButton(IDB_FINDNEXTGROUP,  FALSE);
	} else {
		if(m_ElementsList.GetSelectedCount()>1) {
		   m_OpTools.EnableButton(IDB_DIFF,     TRUE);
		   m_OpTools.EnableButton(IDB_INTERSECT,TRUE);
		} else {
		   m_OpTools.EnableButton(IDB_DIFF,     FALSE);
		   m_OpTools.CheckButton(IDB_ALL);
		   if(m_ElementsList.GetSelectedCount()==0) {
		      m_OpTools.EnableButton(IDB_REMOVEGROUP, FALSE);
		      m_OpTools.EnableButton(IDB_REMOVESEL,   FALSE);
	          m_OpTools.EnableButton(IDB_INSERTSEL,   FALSE);
		   } else {
		      m_OpTools.EnableButton(IDB_REMOVEGROUP, TRUE);
		      m_OpTools.EnableButton(IDB_REMOVESEL,   TRUE);
	          m_OpTools.EnableButton(IDB_INSERTSEL,   TRUE);
		   }
		}
		m_OpTools.EnableButton(IDB_ALL,         TRUE);
		m_OpTools.EnableButton(IDB_INSERTSEL,   TRUE);
		m_OpTools.EnableButton(IDB_FINDFIRSTGROUP, TRUE);
		if(m_Search.GetLength()>0)
		    m_OpTools.EnableButton(IDB_FINDNEXTGROUP,  TRUE);
	    else
		    m_OpTools.EnableButton(IDB_FINDNEXTGROUP,  FALSE);
	}
	m_OpTools.EnableButton(IDB_NEWGROUP,TRUE);

}
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// Returns all selected groups names as a list with {} separators.
//-------------------------------------------------------------------------------------
BSTR CGroupsComboCtrl::GetSelectedGroups()  {
	return m_GroupsList.AllocSysString();
}
//-------------------------------------------------------------------------------------

BOOL CGroupsComboCtrl::GetControlsEnable()  {
	return m_ControlsEnable;
}

void CGroupsComboCtrl::SetControlsEnable(BOOL bNewValue)  {
	m_ControlsEnable=bNewValue;
	if(::IsWindow(*this)) {
		if(m_ControlsEnable==FALSE) {
		   m_OpTools.EnableWindow(FALSE);
		   m_OpTools.ShowWindow(SW_HIDE);
		   CRect winRect;
		   GetWindowRect(winRect);
	       m_ElementsList.MoveWindow(0,0,winRect.Width(),winRect.Height(),TRUE);
		}
		if(m_ControlsEnable==TRUE) {
		   m_OpTools.EnableWindow(TRUE);
		   m_OpTools.ShowWindow(SW_SHOW);
		   CRect winRect;
		   CRect toolsRect;
		   GetWindowRect(winRect);
		   m_OpTools.GetWindowRect(toolsRect);
		   m_OpTools.MoveWindow(0,winRect.Height()-toolsRect.Height(),winRect.Width(),toolsRect.Height());
	       if(::IsWindow(m_ElementsList)) {
		       m_ElementsList.MoveWindow(::GetSystemMetrics(SM_CYEDGE),::GetSystemMetrics(SM_CYEDGE),winRect.Width()-2*::GetSystemMetrics(SM_CYEDGE),winRect.Height()-2*::GetSystemMetrics(SM_CYEDGE)-toolsRect.Height(),TRUE);
		   }
		}
	}
			
}

void CGroupsComboCtrl::CreateGroup()  {
   OnNewGroup();
}

void CGroupsComboCtrl::AddElements()  {
register i;
BOOL in;
CString name;
CString serverQuery;
CString serverResults;
    in=FALSE;
    for(i=0;i<m_GroupsList.GetLength();i++) {
        if(m_GroupsList[i]=='{') {
			in=TRUE;
		    name="";
			continue;
		}
        if(m_GroupsList[i]=='}') {
	        serverQuery.Format("group add %s $%s",name,m_SelectionVar);
		    EvaluateSync(serverQuery,&serverResults);

	        serverQuery.Format("group kind %s",name);
		    EvaluateSync(serverQuery,&serverResults);
			if(serverResults.Find("persistant")>=0) {
	            serverQuery.Format("group save %s",name);
		        EvaluateSync(serverQuery,&serverResults);
			}
			in=FALSE;
		    name="";
			continue;
		}
		if(in==TRUE) name+=m_GroupsList[i];
	}

}
