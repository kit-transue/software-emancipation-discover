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
// QueriesComboCtl.cpp : Implementation of the CQueriesComboCtrl OLE control class.

#include "stdafx.h"
#include <afxtempl.h>
#include "QueriesCombo.h"
#include "QueriesComboCtl.h"
#include "QueriesComboPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CQueriesComboCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CQueriesComboCtrl, COleControl)
	//{{AFX_MSG_MAP(CQueriesComboCtrl)
	ON_WM_CREATE()
	ON_NOTIFY(LVN_ITEMCHANGED,IDC_QUERIESLIST,OnQueryChanged) 
	ON_WM_SIZE()
	ON_MESSAGE(WM_USERSELECTION,OnUserSelection)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CQueriesComboCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CQueriesComboCtrl)
	DISP_PROPERTY_EX(CQueriesComboCtrl, "DataSource", GetDataSource, SetDataSource, VT_DISPATCH)
	DISP_PROPERTY_EX(CQueriesComboCtrl, "Selection", GetSelection, SetSelection, VT_BSTR)
	DISP_PROPERTY_EX(CQueriesComboCtrl, "Categories", GetCategories, SetCategories, VT_BSTR)
	DISP_FUNCTION(CQueriesComboCtrl, "AccessFromName", AccessFromName, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CQueriesComboCtrl, "Update", Update, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CQueriesComboCtrl, "GetQueriesList", GetQueriesList, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CQueriesComboCtrl, "Clear", Clear, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CQueriesComboCtrl, "GetSelectedQueries", GetSelectedQueries, VT_BSTR, VTS_NONE)
	DISP_STOCKPROP_CAPTION()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CQueriesComboCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CQueriesComboCtrl, COleControl)
	//{{AFX_EVENT_MAP(CQueriesComboCtrl)
	EVENT_CUSTOM("QueryChanged", FireQueryChanged, VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CQueriesComboCtrl, 1)
	PROPPAGEID(CQueriesComboPropPage::guid)
END_PROPPAGEIDS(CQueriesComboCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CQueriesComboCtrl, "QUERIESCOMBO.QueriesComboCtrl.1",
	0x6e672c25, 0x1543, 0x11d2, 0xae, 0xed, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CQueriesComboCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DQueriesCombo =
		{ 0x6e672c23, 0x1543, 0x11d2, { 0xae, 0xed, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DQueriesComboEvents =
		{ 0x6e672c24, 0x1543, 0x11d2, { 0xae, 0xed, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwQueriesComboOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CQueriesComboCtrl, IDS_QUERIESCOMBO, _dwQueriesComboOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CQueriesComboCtrl::CQueriesComboCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CQueriesComboCtrl

BOOL CQueriesComboCtrl::CQueriesComboCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_QUERIESCOMBO,
			IDB_QUERIESCOMBO,
			afxRegInsertable | afxRegApartmentThreading,
			_dwQueriesComboOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CQueriesComboCtrl::CQueriesComboCtrl - Constructor

CQueriesComboCtrl::CQueriesComboCtrl()
{
	InitializeIIDs(&IID_DQueriesCombo, &IID_DQueriesComboEvents);
	m_Items=0;
	m_InSelection      = FALSE;
	m_DataSource       = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CQueriesComboCtrl::~CQueriesComboCtrl - Destructor

CQueriesComboCtrl::~CQueriesComboCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CQueriesComboCtrl::OnDraw - Drawing function

void CQueriesComboCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcInvalid,&CBrush(GetSysColor(COLOR_BTNFACE)));

//	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
//	pdc->Ellipse(rcBounds);
}


/////////////////////////////////////////////////////////////////////////////
// CQueriesComboCtrl::DoPropExchange - Persistence support

void CQueriesComboCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CQueriesComboCtrl::OnResetState - Reset control to default state

void CQueriesComboCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CQueriesComboCtrl::AboutBox - Display an "About" box to the user

void CQueriesComboCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_QUERIESCOMBO);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CQueriesComboCtrl message handlers

//------------------------------------------------------------------------------------------
// This method will return the access command from the category name. Will return empty
// string if no access command associated with the given name.
//------------------------------------------------------------------------------------------
BSTR CQueriesComboCtrl::AccessFromName(LPCTSTR name)  {
CString serverCall;
CString serverResults;

    // Queries the server for available categories for the spesified selection.
    serverCall.Format("get_query_command \"%s\"",name);
	EvaluateSync(serverCall,&serverResults);
	return serverResults.AllocSysString();
}
//------------------------------------------------------------------------------------------


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
int CQueriesComboCtrl::EvaluateSync(CString& command, CString* results) {
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


//-----------------------------------------------------------------------------------
// Query server for the queries list. Fills the list with returned queries.
//----------------------------------------------------------------------------------- 
void CQueriesComboCtrl::FillList(void) {
register i;

    m_ListView.DeleteAllItems();
	// Now we will fill the list with queries names, extracting them from the "serverResults"
	int     line   = 0;
	BOOL    inside = FALSE;
	CString name   ="";
	for(i=0;i<m_QueriesList.GetLength();i++) {
		// Entering query substring
		if(m_QueriesList[i]=='{') {
			inside=TRUE;
			name="";
			continue;
		}
		// Exiting query substring - need to insert results into the list
		if(m_QueriesList[i]=='}') {
			inside=FALSE;
			// Inserting new item into the list
	        m_ListView.InsertItem(i,name,0);
			continue;
		}
		if(inside==TRUE) {
		    name+=m_QueriesList[i];
		}
	}
}
//-----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Returns the DataSource IDispatch interface. This component uses the DataSource to
// communicate with the server.
//----------------------------------------------------------------------------------------
LPDISPATCH CQueriesComboCtrl::GetDataSource()  {
	return m_DataSource;
}
//-----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Sets the DataSource IDispatch interface. This component uses the DataSource to
// communicate with the server.
//----------------------------------------------------------------------------------------
void CQueriesComboCtrl::SetDataSource(LPDISPATCH newValue)  {
	// TODO: Add your property handler here
    m_DataSource=newValue;
	FormQueriesList();
	Update();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Callback runs every time the container wants to create component window. Creates the
// reflector window and the ListView window.
//-----------------------------------------------------------------------------------------
int CQueriesComboCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	//----- We are going to create list view with only one column.
	m_ListView.Create( LVS_SHAREIMAGELISTS | 
					   WS_CHILD | 
					   WS_VISIBLE | 
					   LVS_NOCOLUMNHEADER | 
					   LVS_LIST           |
					   LVS_AUTOARRANGE |
					   LVS_SHOWSELALWAYS,
		               CRect(lpCreateStruct->x, 
		                     lpCreateStruct->y,
			                 lpCreateStruct->x+lpCreateStruct->cx,
					         lpCreateStruct->y+lpCreateStruct->cy),
			       this,
				   IDC_QUERIESLIST);
	m_ListView.ModifyStyleEx( 0, WS_EX_CLIENTEDGE);

    LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvColumn.fmt=LVCFMT_LEFT;
	lvColumn.cx=150;
	lvColumn.iSubItem=0;
	lvColumn.pszText="Categories";
    m_ListView.InsertColumn(0,&lvColumn);

	m_ImageList.Create(16,16,TRUE,1,0);
	CBitmap category;
	category.LoadBitmap(IDB_QUERYMAP);
	m_ImageList.Add(&category, RGB(255,255,255));
	m_ListView.SetImageList(&m_ImageList,LVSIL_SMALL); 
	//-----

	
	return 0;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Callback runs every time WM_SIZE message comes. Make ListView fit in the window with 
// only 3D border extent.
//-----------------------------------------------------------------------------------------
void CQueriesComboCtrl::OnSize(UINT nType, int cx, int cy)  {
	COleControl::OnSize(nType, cx, cy);
	if(::IsWindow(m_ListView)) {
		m_ListView.MoveWindow(::GetSystemMetrics(SM_CYEDGE),
			                  ::GetSystemMetrics(SM_CYEDGE),
							   cx-2*::GetSystemMetrics(SM_CYEDGE),
							   cy-2*::GetSystemMetrics(SM_CYEDGE),
							   TRUE);
	}

}
//-----------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// Fire every time the selection changed or added. Pass the string with categories
// readable names separated by {}
//-----------------------------------------------------------------------------------------
void CQueriesComboCtrl::OnQueryChanged(NMHDR* pNMHDR, LRESULT* pResult) {
NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// Item status changed
    if( (pNMListView->uOldState&LVIS_SELECTED)!=
		(pNMListView->uNewState&LVIS_SELECTED) ) {
		   m_InSelection=TRUE;
	}
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Returns the server variable name which is used by this component to obtain selection
// list.
//----------------------------------------------------------------------------------------
BSTR CQueriesComboCtrl::GetSelection() {
	return m_Selection.AllocSysString();
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Set the new name for the server variable which must contain selection with which this
// component works.
//----------------------------------------------------------------------------------------
void CQueriesComboCtrl::SetSelection(LPCTSTR lpszNewValue)  {
CString oldList=m_QueriesList;
	m_Selection=lpszNewValue;
    FormQueriesList();
	if(oldList!=m_QueriesList) Update();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Returns the list of the categories from which this component form queries if nothing
// selected. Categories separated by {}
//-----------------------------------------------------------------------------------------
BSTR CQueriesComboCtrl::GetCategories()  {
	return m_Categories.AllocSysString();
}
//-----------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// Sets the new categories list. The component will use this list to form queries in the
// case selection is not set.Categories separated by {} in this list.
//-----------------------------------------------------------------------------------------
void CQueriesComboCtrl::SetCategories(LPCTSTR lpszNewValue) {
	if(m_Categories!=CString(lpszNewValue)) {
	   m_Categories=lpszNewValue;
       FormQueriesList();
	   Update();
	}
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This method re-fills complete queries list trying to re-set old selections.
//-----------------------------------------------------------------------------------------
void CQueriesComboCtrl::Update()  {
CList<CString,CString&> selection;

	// Collecting all selected items names in the selection array
	for(register i=0;i<m_ListView.GetItemCount();i++) {
		if(m_ListView.GetItemState(i,LVIS_SELECTED)!=0) {
			CString lookFor;
			lookFor=m_ListView.GetItemText(i,0);
			selection.AddTail(lookFor);
		}
	}

	// Filling list with the new values
	FillList();

	// Restoring selections where possible
	POSITION pos;
    LV_FINDINFO findInfo;
	findInfo.flags=LVFI_STRING;
	pos=selection.GetHeadPosition();
	while(pos!=NULL) {
	   findInfo.psz=selection.GetAt(pos); 
	   int index = m_ListView.FindItem(&findInfo);
	   // And if it is present
	   if(index!=-1) {
	       // Selecting this item
	       m_ListView.SetItemState(index,LVIS_SELECTED,LVIS_SELECTED);
	   }
	   selection.GetNext(pos);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// This method returns the complete list of the selected queries 
// separated by {}
//---------------------------------------------------------------------------
BSTR CQueriesComboCtrl::GetQueriesList()  {
    return m_QueriesList.AllocSysString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Form complete list of queries asking server for them. queries are 
// separated by {}
//---------------------------------------------------------------------------
void CQueriesComboCtrl::FormQueriesList(void) {
int selElements;
CString serverCall;
CString serverResults;
CString rowList;

    if(m_Selection.GetLength()==0) selElements=0;
	else {
       serverCall.Format("size $%s",m_Selection);
	   EvaluateSync(serverCall,&serverResults);
	   selElements=atoi(serverResults);
	}
    if(selElements!=0) {
		 // Queries the server for available queries for the specified selection.
        if(m_Selection.GetLength()!=0) {
            serverCall.Format("get_queries -selection $%s",m_Selection);
	        EvaluateSync(serverCall,&rowList);
		} else  m_QueriesList="";
	} else {
		 // Queries the server for all available queries for the specified categories
         serverCall.Format("get_queries -categories %s",m_Categories);
	     EvaluateSync(serverCall,&rowList);
	}

	// Removes unnessesary spaces
	m_QueriesList="";
	BOOL inside;
	CString name;
	CString spaces;
	for(register i=0;i<rowList.GetLength();i++) {
		// Entering query substring
		if(rowList[i]=='{') {
			inside=TRUE;
			name="";
	        spaces="";
			// removes spaces at the beginning
			while(i<rowList.GetLength()-1 && rowList[i+1]==' ') i++;
			continue;
		}
		// Exiting query substring - need to insert results into the list
		if(rowList[i]=='}') {
			inside=FALSE;
			// Inserting new item into the list
	        m_QueriesList+='{';
	        m_QueriesList+=name;
	        m_QueriesList+='}';
			continue;
		}
		if(inside==TRUE) {
			if(rowList[i]!=' ') {
				name+=spaces;
			    name+=rowList[i];
				spaces="";
			} else spaces+=" ";
		}
	}
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Will remove all selections from the query list.
//---------------------------------------------------------------------------
void CQueriesComboCtrl::Clear()  {
   for(register i=0; i<m_ListView.GetItemCount();i++) 
     if(m_ListView.GetItemState(i,LVIS_SELECTED)!=0)
	    m_ListView.SetItemState(i,0,LVIS_SELECTED);

}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Returns the query list with only selected queries in.
//---------------------------------------------------------------------------
BSTR CQueriesComboCtrl::GetSelectedQueries() {

	return m_SelectedQueriesList.AllocSysString();
}
//---------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
LRESULT CQueriesComboCtrl::OnUserSelection(WPARAM,LPARAM) {
	if(m_InSelection==TRUE) {
		// Forming string with all selected categories
		m_SelectedQueriesList="";
	    for(register i=0; i<m_ListView.GetItemCount();i++) {
		   if(m_ListView.GetItemState(i,LVIS_SELECTED)!=0) {
			  m_SelectedQueriesList+=" {";
			  m_SelectedQueriesList+=m_ListView.GetItemText(i,0);
			  m_SelectedQueriesList+='}';
		   }
		}
	    FireQueryChanged(m_SelectedQueriesList);
	    m_InSelection=FALSE;
	}
	return 0;
}
//------------------------------------------------------------------------------------------
