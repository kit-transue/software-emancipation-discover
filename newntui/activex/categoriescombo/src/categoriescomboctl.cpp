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
// CategoriesComboCtl.cpp : Implementation of the CCategoriesComboCtrl OLE control class.

#include "stdafx.h"
#include "CategoriesCombo.h"
#include "CategoriesComboCtl.h"
#include "CategoriesComboPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCategoriesComboCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CCategoriesComboCtrl, COleControl)
	//{{AFX_MSG_MAP(CCategoriesComboCtrl)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_NOTIFY(LVN_ITEMCHANGED,IDC_CATEGORIESLIST,OnCategoryChanged) 
	ON_MESSAGE(WM_USERSELECTION,OnUserSelection)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT,       OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CCategoriesComboCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CCategoriesComboCtrl)
	DISP_PROPERTY_EX(CCategoriesComboCtrl, "DataSource", GetDataSource, SetDataSource, VT_DISPATCH)
	DISP_FUNCTION(CCategoriesComboCtrl, "AccessFromName", AccessFromName, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CCategoriesComboCtrl, "GetCategoriesList", GetCategoriesList, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CCategoriesComboCtrl, "Update", Update, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CCategoriesComboCtrl, "Unselect", Unselect, VT_EMPTY, VTS_NONE)
	DISP_STOCKPROP_CAPTION()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CCategoriesComboCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CCategoriesComboCtrl, COleControl)
	//{{AFX_EVENT_MAP(CCategoriesComboCtrl)
	EVENT_CUSTOM("CategoryChanged", FireCategoryChanged, VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CCategoriesComboCtrl, 1)
	PROPPAGEID(CCategoriesComboPropPage::guid)
END_PROPPAGEIDS(CCategoriesComboCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CCategoriesComboCtrl, "CATEGORIESCOMBO.CategoriesComboCtrl.1",
	0x2451d66c, 0x1203, 0x11d2, 0xae, 0xed, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CCategoriesComboCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DCategoriesCombo =
		{ 0x2451d66a, 0x1203, 0x11d2, { 0xae, 0xed, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DCategoriesComboEvents =
		{ 0x2451d66b, 0x1203, 0x11d2, { 0xae, 0xed, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwCategoriesComboOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CCategoriesComboCtrl, IDS_CATEGORIESCOMBO, _dwCategoriesComboOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CCategoriesComboCtrl::CCategoriesComboCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CCategoriesComboCtrl

BOOL CCategoriesComboCtrl::CCategoriesComboCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_CATEGORIESCOMBO,
			IDB_CATEGORIESCOMBO,
			afxRegInsertable | afxRegApartmentThreading,
			_dwCategoriesComboOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CCategoriesComboCtrl::CCategoriesComboCtrl - Constructor

CCategoriesComboCtrl::CCategoriesComboCtrl()
{
	InitializeIIDs(&IID_DCategoriesCombo, &IID_DCategoriesComboEvents);
	m_InSelection      = FALSE;
	m_Items=0;
}


/////////////////////////////////////////////////////////////////////////////
// CCategoriesComboCtrl::~CCategoriesComboCtrl - Destructor

CCategoriesComboCtrl::~CCategoriesComboCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CCategoriesComboCtrl::OnDraw - Drawing function

void CCategoriesComboCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcInvalid,&CBrush(GetSysColor(COLOR_BTNFACE)));
}


/////////////////////////////////////////////////////////////////////////////
// CCategoriesComboCtrl::DoPropExchange - Persistence support

void CCategoriesComboCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CCategoriesComboCtrl::OnResetState - Reset control to default state

void CCategoriesComboCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CCategoriesComboCtrl::AboutBox - Display an "About" box to the user

void CCategoriesComboCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_CATEGORIESCOMBO);
	dlgAbout.DoModal();
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
int CCategoriesComboCtrl::EvaluateSync(CString& command, CString* results) {
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
// Query server for the categories list. Fills the list with returned categories.
//----------------------------------------------------------------------------------- 
void CCategoriesComboCtrl::FillList(void) {
register i;
CString serverCall;
CString serverResults;
CString spaces;

    // Removes all items from the list
    m_ListView.DeleteAllItems();


    // Queries the server for available categories for the spesified selection.
    serverCall.Format("get_categories");
	EvaluateSync(serverCall,&serverResults);

    // serverResults string now will contain the complete list of redable
	// categories names separated by { }.

	// Now we will fill the list with categories names, extracting them from 
	// the "serverResults". 

    //*** In future we will fill only floating frame and we will fill the list
	//*** from the storage. Elements in the list can appear only by drug-drop
	//*** or pressing the button in the floating frame.

	int     line   = 0;
	BOOL    inside = FALSE;
	CString name   ="";
	spaces="";
	int itemN=0;
	for(i=0;i<serverResults.GetLength();i++) {
		// Entering query substring
		if(serverResults[i]=='{') {
			inside=TRUE;
			name   ="";
            spaces ="";
		    // removes spaces at the beginning
			while(i<serverResults.GetLength()-1 && serverResults[i+1]==' ') i++;
			continue;
		}
		// Exiting query substring - need to insert results into the list
		if(serverResults[i]=='}') {
			inside=FALSE;
			// Inserting new item into the combobox
	        m_ListView.InsertItem(itemN++,name,IconIndex(name));
			continue;
		}
		if(inside==TRUE) {
			if(serverResults[i]!=' ') {
				name+=spaces;
			    name+=serverResults[i];
				spaces="";
			} else {
				spaces+=' ';
			}
		}
	}
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Returns the LPDISPATCH interface of the currently used DataSource.
//-----------------------------------------------------------------------------------
LPDISPATCH CCategoriesComboCtrl::GetDataSource()  {
	return m_DataSource;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Sets the new LPDISPATCH interface of the DataSource object.
// Fills the categories list and displays it.
//-----------------------------------------------------------------------------------
void CCategoriesComboCtrl::SetDataSource(LPDISPATCH newValue) {
    m_DataSource=newValue;
	FillList();
	SetModifiedFlag();
}
//-----------------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// CCategoriesComboCtrl message handlers


//------------------------------------------------------------------------------------------
// This method will return the access command from the category name. Will return empty
// string if no access command associated with the given name.
//------------------------------------------------------------------------------------------
BSTR CCategoriesComboCtrl::AccessFromName(LPCTSTR name)  {
CString serverCall;
CString serverResults;

    // Queries the server for available categories for the spesified selection.
    serverCall.Format("get_category_command \"%s\"",name);
	EvaluateSync(serverCall,&serverResults);
	return serverResults.AllocSysString();
}
//------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
// Fire every time the selection changed or added. Pass the string with categories
// readable names separated by {}
//----------------------------------------------------------------------------------
void CCategoriesComboCtrl::OnCategoryChanged(NMHDR* pNMHDR, LRESULT* pResult) {
NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// Item status changed from unselected to selected
    if( (pNMListView->uOldState&LVIS_SELECTED)!=
		(pNMListView->uNewState&LVIS_SELECTED) ) {
		   // Forming string with all selected categories
           CString categoriesList="";
	       for(register i=0; i<m_ListView.GetItemCount();i++) {
		      if(m_ListView.GetItemState(i,LVIS_SELECTED)!=0) {
			     categoriesList+=" {";
			     categoriesList+=m_ListView.GetItemText(i,0);
			     categoriesList+='}';
			  }
		   }
		   m_InSelection=TRUE;
	}
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Will run when WM_SIZE message comes. Perform layout for combobox. list view and
// clear button.
//-----------------------------------------------------------------------------------
void CCategoriesComboCtrl::OnSize(UINT nType, int cx, int cy) {
	COleControl::OnSize(nType, cx, cy);
	if(::IsWindow(m_ListView)) {
		m_ListView.MoveWindow(::GetSystemMetrics(SM_CYEDGE),::GetSystemMetrics(SM_CYEDGE),cx-2*::GetSystemMetrics(SM_CYEDGE),cy-2*::GetSystemMetrics(SM_CYEDGE),TRUE);
	}
	
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Creating this ActiveX component will result in creating all child windows -
// combobox, list view and clear button
//------------------------------------------------------------------------------------
int CCategoriesComboCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	//----- We are going to create list view with only one column.
	m_ListView.Create( LVS_SHAREIMAGELISTS | 
		               LVS_LIST | 
					   WS_CHILD | 
					   WS_VISIBLE | 
					   LVS_NOCOLUMNHEADER | 
					   LVS_AUTOARRANGE |
					   LVS_SHOWSELALWAYS,
		               CRect(lpCreateStruct->x, 
		                     lpCreateStruct->y,
			                 lpCreateStruct->x+lpCreateStruct->cx,
					         lpCreateStruct->y+lpCreateStruct->cy),
			       this,
				   IDC_CATEGORIESLIST);
	m_ListView.ModifyStyleEx( 0, WS_EX_CLIENTEDGE);

    LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvColumn.fmt=LVCFMT_LEFT;
	lvColumn.cx=150;
	lvColumn.iSubItem=0;
	lvColumn.pszText="Categories";
    m_ListView.InsertColumn(0,&lvColumn);

	m_ImageList.Create(16,16,TRUE,1,0);

	// Files
	CBitmap b0;
	b0.LoadBitmap(IDB_FILES);
	m_ImageList.Add(&b0, RGB(255,255,255));

	// Functions
	CBitmap b1;
	b1.LoadBitmap(IDB_FUNCTIONS);
	m_ImageList.Add(&b1,RGB(255,255,255));

	// Variables
	CBitmap b2;
	b2.LoadBitmap(IDB_VARIABLES);
	m_ImageList.Add(&b2, RGB(255,255,255));

	// Classes
	CBitmap b3;
	b3.LoadBitmap(IDB_CLASSES);
	m_ImageList.Add(&b3, RGB(255,255,255));

	// Structures
	CBitmap b4;
	b4.LoadBitmap(IDB_STRUCTURES);
	m_ImageList.Add(&b4, RGB(255,255,255));

	// Unions
	CBitmap b5;
	b5.LoadBitmap(IDB_UNIONS);
	m_ImageList.Add(&b5, RGB(255,255,255));

	// Enums
	CBitmap b6;
	b6.LoadBitmap(IDB_ENUMS);
	m_ImageList.Add(&b6, RGB(255,255,255));

	// Typedefs
	CBitmap b7;
	b7.LoadBitmap(IDB_TYPEDEFS);
	m_ImageList.Add(&b7, RGB(255,255,255));

	// Macros
	CBitmap b8;
	b8.LoadBitmap(IDB_MACROS);
	m_ImageList.Add(&b8, RGB(255,255,255));

	// Templates
	CBitmap b9;
	b9.LoadBitmap(IDB_TEMPLATES);
	m_ImageList.Add(&b9, RGB(255,255,255));

	// Strings
	CBitmap b10;
	b10.LoadBitmap(IDB_STRINGS);
	m_ImageList.Add(&b10, RGB(255,255,255));

	// Local variables
	CBitmap b11;
	b11.LoadBitmap(IDB_LOCALVARS);
	m_ImageList.Add(&b11, RGB(255,255,255));

	// Subsystems
	CBitmap b12;
	b12.LoadBitmap(IDB_SUBSYSTEMS);
	m_ImageList.Add(&b12, RGB(255,255,255));

	// Relations
	CBitmap b13;
	b13.LoadBitmap(IDB_RELATIONS);
	m_ImageList.Add(&b13, RGB(255,255,255));

	// Associations
	CBitmap b14;
	b14.LoadBitmap(IDB_ASSOCIATIONS);
	m_ImageList.Add(&b14, RGB(255,255,255));

	// Link types
	CBitmap b15;
	b15.LoadBitmap(IDB_LINKS);
	m_ImageList.Add(&b15, RGB(255,255,255));

	// Errors
	CBitmap b16;
	b16.LoadBitmap(IDB_ERRORS);
	m_ImageList.Add(&b16, RGB(255,255,255));

	// Tests
	CBitmap b17;
	b17.LoadBitmap(IDB_TESTS);
	m_ImageList.Add(&b17, RGB(255,255,255));

	// Defects
	CBitmap b18;
	b18.LoadBitmap(IDB_DEFECTS);
	m_ImageList.Add(&b18, RGB(255,255,255));
	//-----

	// Packages
	CBitmap b19;
	b19.LoadBitmap(IDB_PACKAGES);
	m_ImageList.Add(&b19, RGB(255,255,255));
	//-----

	// Methods
	CBitmap b20;
	b20.LoadBitmap(IDB_METHODS);
	m_ImageList.Add(&b20, RGB(255,255,255));
	//-----

	// Class variables
	CBitmap b21;
	b21.LoadBitmap(IDB_CLASSVARS);
	m_ImageList.Add(&b21, RGB(255,255,255));
	//-----

	// Interfaces
	CBitmap b22;
	b22.LoadBitmap(IDB_INTERFACES);
	m_ImageList.Add(&b22, RGB(255,255,255));
	//-----

	CBitmap b23;
	b23.LoadBitmap(IDB_TABLE);
	m_ImageList.Add(&b23, RGB(255,255,255));

	CBitmap b24;
	b24.LoadBitmap(IDB_CURSOR);
	m_ImageList.Add(&b24, RGB(255,255,255));

	CBitmap b25;
	b25.LoadBitmap(IDB_XREF);
	m_ImageList.Add(&b25, RGB(255,255,255));

	m_ListView.SetImageList(&m_ImageList,LVSIL_SMALL); 
	return 0;
}
//----------------------------------------------------------------------------------------





//-----------------------------------------------------------------------------------------
// This method can be used to obtain complete list of the selected categories separated by
// {}.
//-----------------------------------------------------------------------------------------
BSTR CCategoriesComboCtrl::GetCategoriesList() {
    CString categoriesList="";
	for(register i=0; i<m_ListView.GetItemCount();i++) {
       if(m_ListView.GetItemState(i,LVIS_SELECTED)!=0) {
	       categoriesList+=" {";
		   categoriesList+=m_ListView.GetItemText(i,0);
		   categoriesList+='}';
	   }
    }
	return categoriesList.AllocSysString();
}
//-----------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
LRESULT CCategoriesComboCtrl::OnUserSelection(WPARAM,LPARAM) {
	if(m_InSelection==TRUE) {
        CString categoriesList="";
	    for(register i=0; i<m_ListView.GetItemCount();i++) {
		   if(m_ListView.GetItemState(i,LVIS_SELECTED)!=0) {
		      categoriesList+=" {";
			  categoriesList+=m_ListView.GetItemText(i,0);
			  categoriesList+='}';
		   }
		}
		FireCategoryChanged(categoriesList);
	    m_InSelection=FALSE;
	}
	return 0;
}
//------------------------------------------------------------------------------------------

void CCategoriesComboCtrl::Update()  {
	FillList();
}

//------------------------------------------------------------------------------------------
// This METHOD will remove all selections from the categories list
//------------------------------------------------------------------------------------------
void CCategoriesComboCtrl::Unselect()  {
   for(register i=0; i<m_ListView.GetItemCount();i++)
		m_ListView.SetItemState(i,0,LVIS_SELECTED);

}
//------------------------------------------------------------------------------------------

int CCategoriesComboCtrl::IconIndex(const CString &name) {
	if(name == "Files")              return 0;
	if(name == "Functions")          return 1;
	if(name == "Variables")          return 2;
	if(name == "Classes")            return 3;
	if(name == "Structures")         return 4;
	if(name == "Unions")             return 5;
	if(name == "Enums")              return 6;
	if(name == "Typedefs")           return 7;
	if(name == "Macros")             return 8;
	if(name == "Templates")          return 9;
	if(name == "String Literals")    return 10;
	if(name == "Local Variables")    return 11;
	if(name == "Subsystems")         return 12;
	if(name == "Relations")          return 13;
	if(name == "Associations")       return 14;
	if(name == "Link Types")         return 15;
	if(name == "Errors")             return 16;
	if(name == "Tests")              return 17;
	if(name == "Defects")            return 18;
	if(name == "Packages")           return 19;
	if(name == "Methods")            return 20;
	if(name == "Class Variables")    return 21;
	if(name == "Interfaces")         return 22;
	if(name == "Tables")             return 23;
	if(name == "Cursors")            return 24;
	if(name == "Externrefs")         return 25;

	return 0;


}
