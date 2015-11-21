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
// TabSelectorCtl.cpp : Implementation of the CTabSelectorCtrl OLE control class.

#include "stdafx.h"
#include "TabSelector.h"
#include "TabSelectorCtl.h"
#include "TabSelectorPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CTabSelectorCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CTabSelectorCtrl, COleControl)
	//{{AFX_MSG_MAP(CTabSelectorCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnSelchangeTab)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CTabSelectorCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CTabSelectorCtrl)
	DISP_PROPERTY_EX(CTabSelectorCtrl, "DataSource", GetDataSource, SetDataSource, VT_DISPATCH)
	DISP_PROPERTY_EX(CTabSelectorCtrl, "OutputVariable", GetOutputVariable, SetOutputVariable, VT_BSTR)
	DISP_PROPERTY_EX(CTabSelectorCtrl, "SelectionVariable", GetSelectionVariable, SetSelectionVariable, VT_BSTR)
	DISP_PROPERTY_EX(CTabSelectorCtrl, "SelectedCategories", GetSelectedCategories, SetSelectedCategories, VT_BSTR)
	DISP_FUNCTION(CTabSelectorCtrl, "AccessFromName", AccessFromName, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CTabSelectorCtrl, "Update", Update, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CTabSelectorCtrl, "AccessFromCategory", AccessFromCategory, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CTabSelectorCtrl, "AccessFromQuery", AccessFromQuery, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CTabSelectorCtrl, "Unselect", Unselect, VT_EMPTY, VTS_NONE)
	DISP_STOCKPROP_CAPTION()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CTabSelectorCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CTabSelectorCtrl, COleControl)
	//{{AFX_EVENT_MAP(CTabSelectorCtrl)
	EVENT_CUSTOM("ModeChanged", FireModeChanged, VTS_I2)
	EVENT_CUSTOM("CategoryChanged", FireCategoryChanged, VTS_BSTR)
	EVENT_CUSTOM("QueryChanged", FireQueryChanged, VTS_BSTR)
	EVENT_CUSTOM("GroupChanged", FireGroupChanged, VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event callbacks
BEGIN_EVENTSINK_MAP(CTabSelectorCtrl, COleControl)
    //{{AFX_EVENTSINK_MAP(CTabSelectorCtrl)
	ON_EVENT(CTabSelectorCtrl, IDC_CATEGORIES, 1 /* CategoryChanged */,  OnCategoryChanged,  VTS_BSTR)
	ON_EVENT(CTabSelectorCtrl, IDC_GROUPS,     1 /* GroupChanged */,     OnGroupChanged,     VTS_BSTR)
	ON_EVENT(CTabSelectorCtrl, IDC_GROUPS,     2 /* GroupCreated */,     OnGroupCreated,     VTS_BSTR)
	ON_EVENT(CTabSelectorCtrl, IDC_GROUPS,     3 /* GroupRemoved */,     OnGroupRemoved,     VTS_I2)
	ON_EVENT(CTabSelectorCtrl, IDC_GROUPS,     5 /* SelectionInserted */,OnGroupInsSel, 0)
	ON_EVENT(CTabSelectorCtrl, IDC_GROUPS,     6 /* SelectionRemover */, OnGroupDelSel, 0)
	ON_EVENT(CTabSelectorCtrl, IDC_QUERIES,    1 /* QueryChanged */,     OnQueryChanged,     VTS_BSTR)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CTabSelectorCtrl, 1)
	PROPPAGEID(CTabSelectorPropPage::guid)
END_PROPPAGEIDS(CTabSelectorCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CTabSelectorCtrl, "TABSELECTOR.TabSelectorCtrl.1",
	0xadc2dfd4, 0x1e61, 0x11d2, 0xae, 0xf3, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CTabSelectorCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DTabSelector =
		{ 0xadc2dfd2, 0x1e61, 0x11d2, { 0xae, 0xf3, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DTabSelectorEvents =
		{ 0xadc2dfd3, 0x1e61, 0x11d2, { 0xae, 0xf3, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwTabSelectorOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CTabSelectorCtrl, IDS_TABSELECTOR, _dwTabSelectorOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CTabSelectorCtrl::CTabSelectorCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CTabSelectorCtrl

BOOL CTabSelectorCtrl::CTabSelectorCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_TABSELECTOR,
			IDB_TABSELECTOR,
			afxRegInsertable | afxRegApartmentThreading,
			_dwTabSelectorOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CTabSelectorCtrl::CTabSelectorCtrl - Constructor

CTabSelectorCtrl::CTabSelectorCtrl() {
	AfxEnableControlContainer();
	InitializeIIDs(&IID_DTabSelector, &IID_DTabSelectorEvents);
    m_DataSource = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CTabSelectorCtrl::~CTabSelectorCtrl - Destructor

CTabSelectorCtrl::~CTabSelectorCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CTabSelectorCtrl::OnDraw - Drawing function

void CTabSelectorCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
}


/////////////////////////////////////////////////////////////////////////////
// CTabSelectorCtrl::DoPropExchange - Persistence support

void CTabSelectorCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CTabSelectorCtrl::OnResetState - Reset control to default state

void CTabSelectorCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CTabSelectorCtrl::AboutBox - Display an "About" box to the user

void CTabSelectorCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_TABSELECTOR);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CTabSelectorCtrl message handlers


//---------------------------------------------------------------------------------------
// Returns the IDispatch interface of the DataSource object which this control currently
// using to communicate with server.
//--------------------------------------------------------------------------------------- 
LPDISPATCH CTabSelectorCtrl::GetDataSource()  {
	return m_DataSource;
}
//--------------------------------------------------------------------------------------- 

//---------------------------------------------------------------------------------------
// Sets the IDispatch interface of the DataSource object which this control will use
// to communicate with server.
//--------------------------------------------------------------------------------------- 
void CTabSelectorCtrl::SetDataSource(LPDISPATCH newValue)  {
	m_DataSource = newValue;
	m_Categories.SetDataSource(m_DataSource);
	m_Queries.SetDataSource(m_DataSource);
	m_Groups.SetDataSource(m_DataSource);
}
//--------------------------------------------------------------------------------------- 



//--------------------------------------------------------------------------------------- 
// Creating this window will result in creating all child windows - tab control and three
// lists: Categories selector, Queries selector and Groups selector.
//--------------------------------------------------------------------------------------- 
int CTabSelectorCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Creating tab selector to select active component
    m_Tab.Create(WS_CHILD | WS_VISIBLE, 
				 CRect(lpCreateStruct->x, 
		               lpCreateStruct->y,
			           lpCreateStruct->x+lpCreateStruct->cx,
					   lpCreateStruct->y+lpCreateStruct->cy
				),
                this, 
				IDC_TAB);
    // Adding tabs - components names
	TC_ITEM item;

	item.mask         = TCIF_TEXT;
    item.pszText      = "Categories";     
    item.cchTextMax   = 20;     
	m_Tab.InsertItem(0,&item);

	item.mask         = TCIF_TEXT;
    item.pszText      = "Queries";     
    item.cchTextMax   = 20;     
	m_Tab.InsertItem(1,&item);
	item.mask         = TCIF_TEXT;
    item.pszText      = "Groups";     
    item.cchTextMax   = 20;     
	m_Tab.InsertItem(2,&item);

	infoFont.CreateFont(14,0,0,0,0,0,0,0,0,0,0,0,0,"Arial");
	m_Tab.SetFont(&infoFont,FALSE);

	
	// Creating categories component
    m_Categories.Create("Categories Selector",
		                 WS_CHILD | WS_VISIBLE,
		                 CRect(lpCreateStruct->x, 
		                       lpCreateStruct->y,
			                   lpCreateStruct->x+lpCreateStruct->cx,
					           lpCreateStruct->y+lpCreateStruct->cy
						 ),
 					     this, 
					     IDC_CATEGORIES);
	// Creatind queries component
    m_Queries.Create("Queries Selector",
		              WS_CHILD | WS_VISIBLE,
		              CRect(lpCreateStruct->x, 
		                    lpCreateStruct->y,
			                lpCreateStruct->x+lpCreateStruct->cx,
					        lpCreateStruct->y+lpCreateStruct->cy
					   ),
 					   this, 
					   IDC_QUERIES);
	// Creating groups component
    m_Groups.Create("Groups Selector",
		            WS_CHILD | WS_VISIBLE,
		            CRect(lpCreateStruct->x, 
		                  lpCreateStruct->y,
			              lpCreateStruct->x+lpCreateStruct->cx,
					      lpCreateStruct->y+lpCreateStruct->cy
					),
 					this, 
					IDC_GROUPS);
	return 0;
}
//--------------------------------------------------------------------------------------- 


//---------------------------------------------------------------------------------------
// Changing the window size will result in size changing of all it's childs - tab control
// will occupy the topmost row and all ActiveX controls will occupy the bottom.
//---------------------------------------------------------------------------------------
void CTabSelectorCtrl::OnSize(UINT nType, int cx, int cy) {
	COleControl::OnSize(nType, cx, cy);
	if(::IsWindow(m_Tab)) m_Tab.MoveWindow(0,0,cx,25);
	if(::IsWindow(m_Categories)) m_Categories.MoveWindow(0,25,cx,cy-25);
	if(::IsWindow(m_Queries))    m_Queries.MoveWindow(0,25,cx,cy-25);
	if(::IsWindow(m_Groups))     m_Groups.MoveWindow(0,25,cx,cy-25);
	
	
}
//---------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// This method will return the access command from the category name. Will return empty
// string if no access command associated with the given name.
//------------------------------------------------------------------------------------------
BSTR CTabSelectorCtrl::AccessFromName(LPCTSTR name)  {
CString access;

	switch(m_Tab.GetCurSel()) {
	   case 0 : access=m_Categories.AccessFromName(name);
		        break;
	   case 1 : access=m_Queries.AccessFromName(name);
				break;
	   case 2 : access=m_Queries.AccessFromName(name);
				break;
	}
   return access.AllocSysString();
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Enables or disables elements according tab selection.
//------------------------------------------------------------------------------------------
void CTabSelectorCtrl::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)  {
int sel = m_Tab.GetCurSel();
	switch(sel) {
	   case 0 : m_Queries.ShowWindow(SW_HIDE); 
		        m_Groups.ShowWindow(SW_HIDE); 
				m_Categories.ShowWindow(SW_SHOW);
				FireModeChanged(0);
		        break;
	   case 1 : m_Categories.ShowWindow(SW_HIDE); 
		        m_Groups.ShowWindow(SW_HIDE); 
		        m_Queries.ShowWindow(SW_SHOW); 
				FireModeChanged(1);
				break;
	   case 2 : m_Categories.ShowWindow(SW_HIDE); 
		        m_Queries.ShowWindow(SW_HIDE); 
		        m_Groups.ShowWindow(SW_SHOW); 
				FireModeChanged(2);
				break;
	}
	*pResult = 0;
}
//------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Calling this function every time the category changed.
//-----------------------------------------------------------------------------------------
void CTabSelectorCtrl::OnCategoryChanged(LPCTSTR categoriesList) {
	FireCategoryChanged(categoriesList);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Calling this function every time the category changed.
//-----------------------------------------------------------------------------------------
void CTabSelectorCtrl::OnGroupChanged(LPCTSTR list) {
	FireGroupChanged(list);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Calling this function every time the new group created.
//-----------------------------------------------------------------------------------------
void CTabSelectorCtrl::OnGroupCreated(LPCTSTR list) {
	FireGroupChanged(m_Groups.GetSelectedGroups());
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Calling this function every time the category changed.
//-----------------------------------------------------------------------------------------
void CTabSelectorCtrl::OnGroupRemoved(short amount) {
	FireGroupChanged(m_Groups.GetSelectedGroups());
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Calling this function every time the category changed.
//-----------------------------------------------------------------------------------------
void CTabSelectorCtrl::OnGroupInsSel(void) {
	FireGroupChanged(m_Groups.GetSelectedGroups());
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Calling this function every time the category changed.
//-----------------------------------------------------------------------------------------
void CTabSelectorCtrl::OnGroupDelSel(void) {
	FireGroupChanged(m_Groups.GetSelectedGroups());
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the server variable name in which group ActiveX component is storing groups
// elements.
//-----------------------------------------------------------------------------------------
BSTR CTabSelectorCtrl::GetOutputVariable()  {
	CString strResult=m_Groups.GetOutputVariable();
	return strResult.AllocSysString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Sets the server variable name in which group ActiveX component will store groups
// elements.
//-----------------------------------------------------------------------------------------
void CTabSelectorCtrl::SetOutputVariable(LPCTSTR lpszNewValue)  {
	m_Groups.SetOutputVariable(lpszNewValue);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the name of the server variable which will be used by groups and queries
// ActiveX components as a variable which store selection.
//-----------------------------------------------------------------------------------------
BSTR CTabSelectorCtrl::GetSelectionVariable()  {
	return m_SelectionVar.AllocSysString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Sets the server variable name which will be used as arguments for the groups and
// queries ActiveX controls,
//-----------------------------------------------------------------------------------------
void CTabSelectorCtrl::SetSelectionVariable(LPCTSTR lpszNewValue)  {
	m_SelectionVar=lpszNewValue;
	m_Groups.SetSelectionVariable(m_SelectionVar);
	m_Queries.SetSelection(m_SelectionVar);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Updates all elements in the control.
//-----------------------------------------------------------------------------------------
void CTabSelectorCtrl::Update()  {
	m_Categories.Update();
	m_Queries.Update();
	m_Groups.Update();

}
//-----------------------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Calling this function every time the user selected query.
//---------------------------------------------------------------------------
void CTabSelectorCtrl::OnQueryChanged(LPCTSTR queriesList) {
	FireQueryChanged(queriesList);
}
//------------------------------------------------------------------------

BSTR CTabSelectorCtrl::AccessFromCategory(LPCTSTR category)  {
CString access;

	access=m_Categories.AccessFromName(category);
    return access.AllocSysString();
}

BSTR CTabSelectorCtrl::AccessFromQuery(LPCTSTR query)  {
CString access;

   access=m_Queries.AccessFromName(query);
   return access.AllocSysString();
}


//--------------------------------------------------------------------------------------
// Returns the list of the categories which will be used to form available quueries in 
// the case there are no selection to use.
//--------------------------------------------------------------------------------------
BSTR CTabSelectorCtrl::GetSelectedCategories()  {
	return m_SelectedCategories.AllocSysString();
}
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Sets the list of the categories which will be used to form available quueries in 
// the case there are no selection to use.
//--------------------------------------------------------------------------------------
void CTabSelectorCtrl::SetSelectedCategories(LPCTSTR lpszNewValue)  {
    m_SelectedCategories=lpszNewValue;
	m_Queries.SetCategories(m_SelectedCategories);
}
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// This method will remove any selection from the queries list.
//--------------------------------------------------------------------------------------
void CTabSelectorCtrl::Unselect() {
  m_Categories.Unselect();
  m_Tab.SetCurSel(0);
  m_Queries.ShowWindow(SW_HIDE); 
  m_Groups.ShowWindow(SW_HIDE); 
  m_Categories.ShowWindow(SW_SHOW);
  FireModeChanged(0);
}
//--------------------------------------------------------------------------------------

