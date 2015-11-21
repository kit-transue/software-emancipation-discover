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
// CBrowserFrameCtl.cpp : Implementation of the CBrowserFrameCtrl OLE control class.

#include "stdafx.h"

#include "CBrowserFrameCtl.h"

#ifndef __BROWSERFRAME__
   #define __BROWSERFRAME__
   #include "CBrowserFrame.h"
#endif


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

#include "CBrowserFramePpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CBrowserFrameCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CBrowserFrameCtrl, COleControl)
	//{{AFX_MSG_MAP(CBrowserFrameCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(IDC_SELECTSORT,   OnSelectSort)
	ON_COMMAND(IDC_SELATTRIBUTES,OnSetAttributes)
	ON_COMMAND(IDC_SELECTFILTER, OnSetFilter)
	ON_COMMAND(IDC_RESETFILTER,  OnResetFilter)
	ON_COMMAND(IDC_ADDELEMENTS,  OnAddElements)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Child events  map
BEGIN_EVENTSINK_MAP(CBrowserFrameCtrl, COleControl)
	ON_EVENT(CBrowserFrameCtrl, IDC_MODESELECTOR, 1 /* ModeChanged     */,  OnModeChanged,      VTS_I2)
	ON_EVENT(CBrowserFrameCtrl, IDC_MODESELECTOR, 2 /* CategoryChanged */,  OnCategoryChanged,  VTS_BSTR)
	ON_EVENT(CBrowserFrameCtrl, IDC_MODESELECTOR, 3 /* QueryChanged    */,  OnQueryChanged,     VTS_BSTR)
	ON_EVENT(CBrowserFrameCtrl, IDC_MODESELECTOR, 4 /* GroupChanged    */,  OnGroupChanged,     VTS_BSTR)

	ON_EVENT(CBrowserFrameCtrl, IDC_RESULTS,  4 /* SelectionChanged */,     OnSelectionChanged,    VTS_BSTR)
	ON_EVENT(CBrowserFrameCtrl, IDC_RESULTS,  3 /* ElementDoubleclicked */, OnElementDoubleclicked,VTS_BSTR)
	ON_EVENT(CBrowserFrameCtrl, IDC_RESULTS,  2 /* RButtonEvent */, OnRButtonEvent,        VTS_I2 VTS_I2)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CBrowserFrameCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CBrowserFrameCtrl)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "Arguments", GetArguments, SetArguments, VT_BSTR)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "ResultsFromCategory", GetResultsFromCategory, SetResultsFromCategory, VT_BSTR)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "ResultsFromQuery", GetResultsFromQuery, SetResultsFromQuery, VT_BSTR)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "ResultsFromGroup", GetResultsFromGroup, SetResultsFromGroup, VT_BSTR)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "SelectionFromQuery", GetSelectionFromQuery, SetSelectionFromQuery, VT_BSTR)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "SelectionFromCategory", GetSelectionFromCategory, SetSelectionFromCategory, VT_BSTR)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "SelectionFromGroup", GetSelectionFromGroup, SetSelectionFromGroup, VT_BSTR)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "Projects", GetProjects, SetProjects, VT_BSTR)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "Mode", GetMode, SetMode, VT_I2)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "DataSource", GetDataSource, SetDataSource, VT_DISPATCH)
	DISP_PROPERTY_EX(CBrowserFrameCtrl, "SelectedCategories", GetSelectedCategories, SetSelectedCategories, VT_BSTR)
	DISP_FUNCTION(CBrowserFrameCtrl,    "Update", Update, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CBrowserFrameCtrl, "SetSet", SetSet, VT_EMPTY, VTS_BSTR)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CBrowserFrameCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CBrowserFrameCtrl, COleControl)
	//{{AFX_EVENT_MAP(CBrowserFrameCtrl)
	EVENT_CUSTOM("ModeChanged", FireModeChanged, VTS_I2)
	EVENT_CUSTOM("CategoryChanged", FireCategoryChanged, VTS_BSTR)
	EVENT_CUSTOM("QueryChanged", FireQueryChanged, VTS_BSTR)
	EVENT_CUSTOM("GroupChanged", FireGroupChanged, VTS_BSTR)
	EVENT_CUSTOM("ElementDoubleclicked", FireElementDoubleclicked, VTS_BSTR)
	EVENT_CUSTOM("SelectionChanged", FireSelectionChanged, VTS_BSTR)
	EVENT_CUSTOM("SubsystemsNeedUpdate", FireSubsystemsNeedUpdate, VTS_NONE)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CBrowserFrameCtrl, 1)
	PROPPAGEID(CBrowserFramePropPage::guid)
END_PROPPAGEIDS(CBrowserFrameCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CBrowserFrameCtrl, "CBROWSERFRAME.CBrowserFrameCtrl.1",
	0x1de4f494, 0x201b, 0x11d2, 0xae, 0xf3, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CBrowserFrameCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DCBrowserFrame =
		{ 0x1de4f492, 0x201b, 0x11d2, { 0xae, 0xf3, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DCBrowserFrameEvents =
		{ 0x1de4f493, 0x201b, 0x11d2, { 0xae, 0xf3, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwCBrowserFrameOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CBrowserFrameCtrl, IDS_CBROWSERFRAME, _dwCBrowserFrameOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CBrowserFrameCtrl::CBrowserFrameCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CBrowserFrameCtrl

BOOL CBrowserFrameCtrl::CBrowserFrameCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_BROWSERFRAME,
			IDB_BROWSERFRAME,
			afxRegInsertable | afxRegApartmentThreading,
			_dwCBrowserFrameOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CBrowserFrameCtrl::CBrowserFrameCtrl - Constructor

CBrowserFrameCtrl::CBrowserFrameCtrl(){
	AfxEnableControlContainer();
	InitializeIIDs(&IID_DCBrowserFrame, &IID_DCBrowserFrameEvents);
	m_DataSource = NULL;
	m_ModeID     = CATEGORIES_MODE;
	m_DurtyFlag=FALSE;

} 


/////////////////////////////////////////////////////////////////////////////
// CBrowserFrameCtrl::~CBrowserFrameCtrl - Destructor

CBrowserFrameCtrl::~CBrowserFrameCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CBrowserFrameCtrl::OnDraw - Drawing function

void CBrowserFrameCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
}


/////////////////////////////////////////////////////////////////////////////
// CBrowserFrameCtrl::DoPropExchange - Persistence support

void CBrowserFrameCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CBrowserFrameCtrl::OnResetState - Reset control to default state

void CBrowserFrameCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CBrowserFrameCtrl::AboutBox - Display an "About" box to the user

void CBrowserFrameCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_CBROWSERFRAME);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CBrowserFrameCtrl message handlers


//---------------------------------------------------------------------------------------
// This server variable name will be used to add/remove selections from the groups and
// to form queries from the selection. It will also determine availablequeries.
//---------------------------------------------------------------------------------------
BSTR CBrowserFrameCtrl::GetArguments()  {
	return m_Arguments.AllocSysString();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This server variable name will be used to add/remove selections from the groups and
// to form queries from the selection. It will also determine availablequeries.
//---------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetArguments(LPCTSTR lpszNewValue)  {
	m_Arguments=lpszNewValue;
	if(m_ModeID!=QUERIES_MODE) {
		m_DurtyFlag=TRUE;
		return;
	}


	// Tell ActiveX Tab control from where are we going to take arguments.

	// Need to re-fill query results - new arguments are used.
	m_QueryResults.SetFullListName(m_ResultsFromQuery);
	m_ModeSelector.SetSelectionVariable(m_Arguments);
	m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F");
	m_QueryResults.SetQuery(m_SavedQueryRequest);
	m_QueryResults.Calculate();
	m_QueryResults.SetQuery("");

	// We need to update the results window only if we are in queries mode.
	// The arguments change will not influence to the results window if we are
	// in categories or groups modes.
	switch(m_ModeID) {
	   case CATEGORIES_MODE : m_QueryResults.SetFullListName(m_ResultsFromCategory);
		                      m_QueryResults.SetFilterListName(m_ResultsFromCategory+"_F");
		                      break;
	   case QUERIES_MODE    : m_QueryResults.Update(); 
		                      DisplayInfoString();
		                      break;
	   case GROUPS_MODE     : m_QueryResults.SetFullListName(m_ResultsFromGroup);
		                      m_QueryResults.SetFilterListName(m_ResultsFromGroup+"_F");
		                      break;
	}
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Returns the name of the server variable which this control is using to store the
// results of any category request.
//---------------------------------------------------------------------------------------
BSTR CBrowserFrameCtrl::GetResultsFromCategory()  {
	return m_ResultsFromCategory.AllocSysString();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Sets the name of the server variable which this control will use to store the
// results of any category request.
//---------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetResultsFromCategory(LPCTSTR lpszNewValue)  {
	m_ResultsFromCategory=lpszNewValue;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Returns the name of the server variable which this control is using to store the
// results of any query request.
//---------------------------------------------------------------------------------------
BSTR CBrowserFrameCtrl::GetResultsFromQuery()  {
	return m_ResultsFromQuery.AllocSysString();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Sets the name of the server variable which this control will use to store the
// results of any query request.
//---------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetResultsFromQuery(LPCTSTR lpszNewValue)  {
	m_ResultsFromQuery=lpszNewValue;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Returns the name of the server variable which this control is using to store the
// results of any group request.
//---------------------------------------------------------------------------------------
BSTR CBrowserFrameCtrl::GetResultsFromGroup()  {
	return m_ResultsFromGroup.AllocSysString();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Sets the name of the server variable which this control will use to store the
// results of any group request.
//---------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetResultsFromGroup(LPCTSTR lpszNewValue)  {
	m_ResultsFromGroup=lpszNewValue;
	m_ModeSelector.SetOutputVariable(m_ResultsFromGroup);
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Returns the name of the server variable which this control is using to store any
// selected elements in the query request results.
//---------------------------------------------------------------------------------------
BSTR CBrowserFrameCtrl::GetSelectionFromQuery()  {
	return m_SelectionFromQuery.AllocSysString();
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Sets the name of the server variable which this control will use to store any
// selected elements in the query request results.
//---------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetSelectionFromQuery(LPCTSTR lpszNewValue)  {
	m_SelectionFromQuery=lpszNewValue;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Returns the name of the server variable which this control is using to store any
// selected elements in the category request results.
//---------------------------------------------------------------------------------------
BSTR CBrowserFrameCtrl::GetSelectionFromCategory()  {
	return m_SelectionFromCategory.AllocSysString();
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Sets the name of the server variable which this control will use to store any
// selected elements in the category request results.
//---------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetSelectionFromCategory(LPCTSTR lpszNewValue)  {
	m_SelectionFromCategory=lpszNewValue;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Returns the name of the server variable which this control is using to store any
// selected elements in the group request results.
//---------------------------------------------------------------------------------------
BSTR CBrowserFrameCtrl::GetSelectionFromGroup()  {
	return m_SelectionFromGroup.AllocSysString();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Sets the name of the server variable which this control will use to store any
// selected elements in the group request results.
//---------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetSelectionFromGroup(LPCTSTR lpszNewValue)  {
	m_SelectionFromGroup=lpszNewValue;
	m_ModeSelector.SetSelectionVariable(m_SelectionFromGroup);
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Returns the complete list of the projects that this control is using to run any 
// category request and, if the argument is not set, to run any query request,
// The projects in the list are separated by {}.
//---------------------------------------------------------------------------------------
BSTR CBrowserFrameCtrl::GetProjects()  {
	return m_Projects.AllocSysString();
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Sets the complete list of the projects that this control will use to run any 
// category request and, if the argument is not set, to run any query request,
// The projects in the list must be separated by {}.
//---------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetProjects(LPCTSTR lpszNewValue)  {
	m_Projects=lpszNewValue;
	if(m_SavedCategoryRequest.GetLength()>0) {
        CString query;
		// Filling categories page - depends only from selected projects.
		m_QueryResults.SetFullListName(m_ResultsFromCategory);
		m_QueryResults.SetFilterListName(m_ResultsFromCategory+"_F");
        query.Format("%s %s",m_SavedCategoryRequest, m_Projects);
	    m_QueryResults.SetQuery(query);
	    m_QueryResults.Calculate();
	    m_QueryResults.SetQuery("");
	    // We need to update the results window only if we are in categories or queries
	    // without argument modes.
	    switch(m_ModeID) {
	       case CATEGORIES_MODE : m_QueryResults.SetFullListName(m_ResultsFromCategory);
			                      m_QueryResults.SetFilterListName(m_ResultsFromCategory+"_F");
			                      m_QueryResults.Update();
		                          break;
		   case QUERIES_MODE    : m_QueryResults.SetFullListName(m_ResultsFromQuery);
			                      m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F");
			                      if(m_Arguments.GetLength()==0) {
									  m_QueryResults.Update(); 
		                              DisplayInfoString();
								  }
		                          break;
	       case GROUPS_MODE     : m_QueryResults.SetFullListName(m_ResultsFromGroup);
			                      m_QueryResults.SetFilterListName(m_ResultsFromGroup+"_F");
		                          break;
	    }
    }

}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Returns the current mode ID of the control.
//---------------------------------------------------------------------------------------
short CBrowserFrameCtrl::GetMode()  {
	return m_ModeID;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Sets the current mode ID of the control.
//---------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetMode(short nNewValue)  {
    if( nNewValue!=CATEGORIES_MODE && nNewValue!=QUERIES_MODE && nNewValue!=GROUPS_MODE)
       return;
	m_ModeID=nNewValue;
	switch(m_ModeID) {
	   case CATEGORIES_MODE : m_QueryResults.SetFullListName(m_ResultsFromCategory);
		                      m_QueryResults.SetFilterListName(m_ResultsFromCategory+"_F"); 
			                  m_QueryResults.SetSelectionName(m_SelectionFromCategory); 
		                      break;
	   case QUERIES_MODE    : if(m_DurtyFlag==TRUE) {
	                                // Need to re-fill query results - new arguments are used.
	                                m_QueryResults.SetFullListName(m_ResultsFromQuery);
	                                m_ModeSelector.SetSelectionVariable(m_Arguments);
	                                m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F");
	                                m_QueryResults.SetQuery(m_SavedQueryRequest);
	                                m_QueryResults.Calculate();
	                                m_QueryResults.SetQuery("");
									m_DurtyFlag=FALSE;
							  }
		                      m_QueryResults.SetFullListName(m_ResultsFromQuery);
		                      m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F"); 
			                  m_QueryResults.SetSelectionName(m_SelectionFromQuery);
		                      break;
	   case GROUPS_MODE     : m_QueryResults.SetFullListName(m_ResultsFromGroup);
		                      m_QueryResults.SetFilterListName(m_ResultsFromGroup+"_F");
			                  m_QueryResults.SetSelectionName(m_SelectionFromGroup);
		                      break;
	}
    m_QueryResults.Update();
   DisplayInfoString();
}
//---------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the IDispatch interface to the DataSource object which this component will use
// to communicate with server.
//-----------------------------------------------------------------------------------------
LPDISPATCH CBrowserFrameCtrl::GetDataSource()  {

	return m_DataSource;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Sets the IDispatch interface to the DataSource object which this component will use
// to communicate with server.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetDataSource(LPDISPATCH newValue)  {
	m_DataSource=newValue; 
	m_ModeSelector.SetDataSource(m_DataSource);
	m_QueryResults.SetDataSource(m_DataSource);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method will re-fill and re-draw the results window. 
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::Update()  {
	m_QueryResults.SetQuery("");
	m_QueryResults.Update();

}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Runs every time the control want its own window. Will create mode selector with
// categories, queries and groups windows (CTabSelector class), results window to
// show results of the category, query or group request (CQueryResults class) and
// statusbar window (CStatic class).
//-----------------------------------------------------------------------------------------
int CBrowserFrameCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Creating mode selector ActiveX component
    m_ModeSelector.Create("Mode Selector",
		                     WS_CHILD | WS_VISIBLE,
		                     CRect(lpCreateStruct->x, 
		                           lpCreateStruct->y,
			                       lpCreateStruct->x+lpCreateStruct->cx,
					               lpCreateStruct->y+lpCreateStruct->cy
							 ),
 							 this, 
							 IDC_MODESELECTOR);


	// Creating result window ActiveX component
    m_QueryResults.Create("Results Window",
		                    WS_CHILD | WS_VISIBLE,
		                    CRect(lpCreateStruct->x, 
		                          lpCreateStruct->y,
			                      lpCreateStruct->x+lpCreateStruct->cx,
					              lpCreateStruct->y+lpCreateStruct->cy
						    ),
 						    this, 
						    IDC_RESULTS);

	// Creating information statusbar
    m_InfoBar.Create(       "",
		                    WS_CHILD | WS_VISIBLE,
		                    CRect(lpCreateStruct->x, 
		                          lpCreateStruct->y,
			                      lpCreateStruct->x+lpCreateStruct->cx,
					              lpCreateStruct->y+lpCreateStruct->cy
						    ),
 						    this, 
						    IDC_STATUSBAR);
	m_InfoBar.ModifyStyleEx(0,WS_EX_CLIENTEDGE);
	m_InfoFont.CreateFont(16,0,0,0,0,0,0,0,0,0,0,0,0,"Arial");
	m_InfoBar.SetFont(&m_InfoFont,FALSE);

	// Group window doesn't have it's own query, instead it will use given
	// server variable name to store the group operation results.
	m_ModeSelector.SetOutputVariable(m_ResultsFromGroup);

	switch(m_ModeID) {
	    case CATEGORIES_MODE : m_QueryResults.SetFullListName(m_ResultsFromCategory);
			                   m_QueryResults.SetFilterListName(m_ResultsFromCategory+"_F");
			                   m_QueryResults.SetSelectionName(m_SelectionFromCategory); 
			                   break;
	    case QUERIES_MODE    : m_QueryResults.SetFullListName(m_ResultsFromQuery);
			                   m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F");
			                   m_QueryResults.SetSelectionName(m_SelectionFromQuery);
			                   break;
	    case GROUPS_MODE     : m_QueryResults.SetFullListName(m_ResultsFromGroup);
			                   m_QueryResults.SetFilterListName(m_ResultsFromGroup+"_F");
			                   m_QueryResults.SetSelectionName(m_SelectionFromGroup);
			                   break;
	}
	return 0;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This is callback which runs every time the window change it's size. On NT, it will run
// also after windows creation but as I can remember it will not run after windows creation
// on Windows 95.
// We are making layout procedure for all child windows here.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnSize(UINT nType, int cx, int cy)  {
	COleControl::OnSize(nType, cx, cy);
	
	// set the child windows positions
	if(::IsWindow(m_ModeSelector))   
		m_ModeSelector.MoveWindow(0,0,cx,(int)(0.3*cy),FALSE);
	if(::IsWindow(m_QueryResults))    
		m_QueryResults.MoveWindow(0,(int)(0.3*cy),cx,cy-(int)(0.3*cy)-20,FALSE);
	if(::IsWindow(m_InfoBar))         
		m_InfoBar.MoveWindow(0,cy-20,cx,20,TRUE);
	
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// The mode selector ActiveX component fire "ModeChanged" event every time the user 
// selected some tab in the tab control. We need to catch this event here to change the
// view of the results window to the appropriate mode variable and re-fill the results
// window with this variable elements.  We will pass this event to the control
//  container after our work.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnModeChanged(short mode) {
	m_ModeID=mode;
	switch(mode) {
	    case CATEGORIES_MODE : m_QueryResults.SetFullListName(m_ResultsFromCategory);
			                   m_QueryResults.SetFilterListName(m_ResultsFromCategory+"_F"); 
			                   m_QueryResults.SetSelectionName(m_SelectionFromCategory); 
			                   break;
	    case QUERIES_MODE    : if(m_DurtyFlag==TRUE) {
	                                // Need to re-fill query results - new arguments are used.
	                                m_QueryResults.SetFullListName(m_ResultsFromQuery);
	                                m_ModeSelector.SetSelectionVariable(m_Arguments);
	                                m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F");
	                                m_QueryResults.SetQuery(m_SavedQueryRequest);
	                                m_QueryResults.Calculate();
	                                m_QueryResults.SetQuery("");
									m_DurtyFlag=FALSE;
							   }
			                   m_QueryResults.SetFullListName(m_ResultsFromQuery);
			                   m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F");
			                   m_QueryResults.SetSelectionName(m_SelectionFromQuery);
			                   break;
	    case GROUPS_MODE     : m_QueryResults.SetFullListName(m_ResultsFromGroup);
			                   m_QueryResults.SetFilterListName(m_ResultsFromGroup+"_F");
			                   m_QueryResults.SetSelectionName(m_SelectionFromGroup);
			                   break;
	}
	m_QueryResults.Update();
	DisplayInfoString();
	FireModeChanged(m_ModeID);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will analyse the categories list with {} and will form category reuest
// command saving it in m_SavedCategoryRequest member string.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::FormCategoryRequest(CString& categoriesList) {
register i;
CString oneQuery;
CString category;

   if(categoriesList.GetLength()==0) {
       m_SavedCategoryRequest="";
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
	     oneQuery = m_ModeSelector.AccessFromCategory(category);
		 if(first==TRUE) {
		    m_SavedCategoryRequest=oneQuery;
			first=FALSE;
		 } else {
			int pos = oneQuery.Find('-');
			if(pos!=-1) {
				m_SavedCategoryRequest+=' ';
				m_SavedCategoryRequest+=oneQuery.GetBuffer(2)+pos;
			}
		 }
		 continue;
	  }
	  category+=categoriesList[i];
   } 
 }
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// The mode selector ActiveX component fire  "CategoryChanged" event every time the user
// changed the selections in the categories list. It sends all selected categories as a
// string with {} separators. We need to catch this event to re-fill the results window
// with the results of the new category request and then send this event to the control
// container.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnCategoryChanged(LPCTSTR categoriesList) {
CString query;

   FormCategoryRequest(CString(categoriesList));
   query.Format("%s %s",m_SavedCategoryRequest, m_Projects);
   m_QueryResults.SetFullListName(m_ResultsFromCategory);
   m_QueryResults.SetFilterListName(m_ResultsFromCategory+"_F");
   m_QueryResults.SetQuery(query);
   m_QueryResults.Calculate();
   m_QueryResults.SetQuery("");
   // We need to update the results window only if we are in categories mode.
   // The category change will not influence to the results window if we are
   // in queries or groups modes.
   switch(m_ModeID) {
      case CATEGORIES_MODE : m_QueryResults.SetFullListName(m_ResultsFromCategory); 
		                     m_QueryResults.SetFilterListName(m_ResultsFromCategory+"_F"); 
		                     m_QueryResults.Update();  
							 DisplayInfoString();
		                     break;
      case QUERIES_MODE    : m_QueryResults.SetFullListName(m_ResultsFromQuery); 
		                     m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F"); 
		                     break;
      case GROUPS_MODE     : m_QueryResults.SetFullListName(m_ResultsFromGroup);
		                     m_QueryResults.SetFilterListName(m_ResultsFromGroup+"_F"); 
		                     break;
   }
   FireCategoryChanged(categoriesList);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Forms the complete query from the selected queries and selected project lists or 
// selection variable.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::FormQueryRequest(CString& queriesList) {
register  i;
CString query;

    // If there are no queries selected we will form a command to clear
    // the result window.
    if(queriesList.GetLength()==0) {
	   m_SavedQueryRequest.Format("set $%s [nil_set]",m_ResultsFromQuery);
	   return;
	}

    // Using temporary variable to store the command sequence 
    m_SavedQueryRequest="set tmp [nil_set];";
    // Forming the complex command with multiple queries
	i=0;
	while(queriesList[i]==' ') i++;
	for(;i<(int)strlen(queriesList);i++) {
	   if(queriesList[i]=='{') {
		  query="";
		  continue;
	   }
	   // Query end. Forming the results command and adding it to the
	   // summarized query.
	   if(queriesList[i]=='}'){
	      // Access command from the query name
		  CString oneQuery;
		  oneQuery = m_ModeSelector.AccessFromQuery(query);

          // Access appending command with the arguments
		  CString merge;

		  // If argument variable is not set we will use projects list as an argument.
		  if(m_Arguments.GetLength()!=0)
		      merge.Format("sappend $tmp [%s $%s];",oneQuery,m_Arguments);
		  else
		      merge.Format("sappend $tmp [%s %s];",oneQuery,m_Projects);
		  // Results command
		  m_SavedQueryRequest+=merge;
		  continue;			 
	   }
	   query+=queriesList[i];
	}
	query.Format("list $tmp");
	m_SavedQueryRequest+=query;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// The ModeSelector ActiveX component generates the "QueryChanged" event every time the
// user changed the queries selection and pass the queries string with single queries
// separated by {}. We need to catch this event to re-fill the results window and only
// after our work we will forward this event to the control container.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnQueryChanged(LPCTSTR queriesList) {
	FormQueryRequest(CString(queriesList));
    m_QueryResults.SetFullListName(m_ResultsFromQuery);
    m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F");
	m_QueryResults.SetQuery(m_SavedQueryRequest);
	m_QueryResults.Calculate();
	m_QueryResults.SetQuery("");
    // We need to update the results window only if we are in queries mode.
    // The query change will not influence to the results window if we are
    // in queries or groups modes.
    switch(m_ModeID) {
      case QUERIES_MODE    : m_QueryResults.SetFullListName(m_ResultsFromQuery);
		                     m_QueryResults.SetFilterListName(m_ResultsFromQuery+"_F");
		                     m_QueryResults.Update();  
							 DisplayInfoString();
		                     break;
      case CATEGORIES_MODE : m_QueryResults.SetFullListName(m_ResultsFromCategory);
		                     m_QueryResults.SetFilterListName(m_ResultsFromCategory+"_F"); 
		                     break;
      case GROUPS_MODE     : m_QueryResults.SetFullListName(m_ResultsFromGroup);
		                     m_QueryResults.SetFilterListName(m_ResultsFromGroup+"_F");
		                     break;
    }
    FireQueryChanged(queriesList);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// The ModeSelector ActiveX component generates the "GroupChanged" event every time the
// user changed the group selection or group operation and pass the groups string with 
// single groups separated by {}. We need to catch this event to re-fill the results 
// window and only after our work we will forward this event to the control container.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnGroupChanged(LPCTSTR groupsList) {
	// The group operation results variable is already filled by the ModeSelector 
	// control. We need to re-display the results window only if we are in the
	// groups mode.
    m_QueryResults.SetQuery("");  
    m_QueryResults.Update();  
	DisplayInfoString();
    FireGroupChanged(groupsList);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// The QueryResults ActiveX component generates "SelectionChanged" event every time the
// user selected something in the results window. We need to catch this event and fill
// the selection variable according to the current mode and only after this we will send
// the same message to the control container.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnSelectionChanged(LPCTSTR pstrParams)  {
	// The selection variable will be filled automatically by the QueryResults control.
	// The only one thing we need to do is to set the appropriate selection variable 
	// when changing the mode
	DisplayInfoString();
	FireSelectionChanged(pstrParams);
}
//-----------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// This callback runs at the reply to the ELEMENT_DOUBLECLICKED event from CQueryResults
// ActiveX control and just translate parameter string to the parent. Parameter string
// looks like this: "<filename>\n<elementname>\n<line>\n<column>" The only one 
// integration ActiveX can present in the system and it is lockated in this component 
// parent.
//-------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnElementDoubleclicked(LPCTSTR pstrParams)  {
	FireElementDoubleclicked(pstrParams);
}
//-------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Queries amount information from the results list and show it in the miniframe statusbar.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::DisplayInfoString(void) {
CString templ;
CString info;
     
    templ.LoadString(IDS_INFO);
	info.Format(templ,m_QueryResults.FilteredAmount(),
		              m_QueryResults.RowAmount(),
					  m_QueryResults.AmountSelected());
	m_InfoBar.SetWindowText(info);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// The QueryResults ActiveX component generates the "RButtonDown" event every time the 
// user press mouse right button. We are processing this event to show pop-up menu
// to select or clear filter, to change the sorting mode or to change the visible 
// attributes
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnRButtonEvent(short x,short y) {
CPoint point(x,y);

	CMenu popup;
	popup.CreatePopupMenu();
	popup.AppendMenu(MF_STRING,IDC_SELECTFILTER,"Select Filter ...");
	popup.AppendMenu(MF_STRING,IDC_RESETFILTER, "Clear Filter");
	popup.AppendMenu(MF_SEPARATOR);
	popup.AppendMenu(MF_STRING,IDC_SELATTRIBUTES,"Select Attributes ...");
	popup.AppendMenu(MF_SEPARATOR);
	popup.AppendMenu(MF_STRING,IDC_SELECTSORT, "Select sorting ...");
	popup.AppendMenu(MF_SEPARATOR);
	popup.AppendMenu(MF_STRING,IDC_ADDELEMENTS, "Export to ...");
	CRect main;
	CRect child;
	GetWindowRect(main);
	m_QueryResults.GetWindowRect(child);
	point.y+=child.top-main.top;
	point.x+=child.left-main.left;
	ClientToScreen(&point);
    popup.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this);
	popup.DestroyMenu();
}
//-----------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Select Attributes..." item in the 
// pop-up menu. This function will show the attributes dialog.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnSetAttributes(void) {
	CAttributesDialog AttributesDialog(NULL);
	AttributesDialog.SetApplyTaget(&m_QueryResults);
	// I will kill the everybody who will try to comment the followind three lines.
	HWND hWndCapture = ::GetCapture();
	if (hWndCapture != NULL) ::SendMessage(hWndCapture, WM_CANCELMODE, 0, 0);
	PreModalDialog();
	AttributesDialog.DoModal();
	PostModalDialog();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Select Filter..." item in the 
// pop-up menu. This function will show the filter wizard.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnSetFilter(void) {
FilterSheet filter("Filter Setup",this);

	filter.SetApplyTaget(&m_QueryResults); 
	// I will kill the everybody who will try to comment the followind three lines.
	HWND hWndCapture = ::GetCapture();
	if (hWndCapture != NULL) ::SendMessage(hWndCapture, WM_CANCELMODE, 0, 0);
	PreModalDialog(); 
	filter.DoModal();
	PostModalDialog();
	DisplayInfoString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Select Filter..." item in the 
// pop-up menu. This function will show the filter wizard.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnAddElements(void) {
CAddSelectorDlg add("Elements Insertion",this);

	PreModalDialog(); 
    ModifyStyle(WS_CHILD,WS_OVERLAPPED|WS_BORDER);// Floating bug will appear if this line will be commented
	add.DoModal();
    ModifyStyle(WS_OVERLAPPED|WS_BORDER,WS_CHILD);// Floating bug will appear if this line will be commented
	PostModalDialog();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Reset Filter" item in the 
// pop-up menu. This function will remove any applied filter.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnResetFilter(void)  {
   m_QueryResults.SetFilter("");
   m_QueryResults.Update();
   DisplayInfoString();	
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will run when the user select the "Select Sort..." item in the 
// pop-up menu. This function will show the Sort Selector dialog.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::OnSelectSort(void) {
	CComplexSort sort(this);
	sort.SetApplyTaget(&m_QueryResults);
	// I will kill the everybody who will try to comment the followind three lines.
	HWND hWndCapture = ::GetCapture();
	if (hWndCapture != NULL)::SendMessage(hWndCapture, WM_CANCELMODE, 0, 0);
	PreModalDialog();
	sort.DoModal();
	PostModalDialog();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the list of the categories to use for query definition in the case there are
// no selection to determine query set.
//-----------------------------------------------------------------------------------------
BSTR CBrowserFrameCtrl::GetSelectedCategories()  {
	return m_SelectedCategories.AllocSysString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Sets the list of the categories separated by {} to use for query set definition in 
// the case there are no selection to determine query set.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetSelectedCategories(LPCTSTR lpszNewValue)  {
	m_SelectedCategories=lpszNewValue;
	m_ModeSelector.SetSelectedCategories(m_SelectedCategories);
	m_ModeSelector.Update();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This function fires event to the parent - need to update subsystems.
// The parent, in our case CdockableManager, must find all windows containing subsystems
// list and ask them to re-fill this list.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::UpdateSubsystems() {
	FireSubsystemsNeedUpdate();

}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Call this method if you want to browse a set defined by you (maybe from the other
// window), Pass the name of the server variable containing the set. All selections
// in categories, queries or groups tab windows will be removed, the symbols
// from the passed server variable will be shown on the list.
//-----------------------------------------------------------------------------------------
void CBrowserFrameCtrl::SetSet(LPCTSTR varname)  {
CString query;

   m_ModeSelector.Unselect();
   if(strnicmp(varname,"_DI_",4)==0)
      query.Format("set %s %s",m_ResultsFromQuery,varname);
   else
      query.Format("set %s $%s",m_ResultsFromQuery,varname);
   m_QueryResults.SetQuery(query);
   m_QueryResults.Calculate();
   m_QueryResults.SetQuery("");
   m_QueryResults.Update();
   DisplayInfoString();
}
//-----------------------------------------------------------------------------------------
