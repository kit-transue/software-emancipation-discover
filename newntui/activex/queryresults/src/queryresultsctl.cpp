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
//----------------------------------------------------------------------------------------
//
// QueryResultsCtl.cpp : Implementation of the CQueryResultsCtrl ActiveX control class.
//
// Designed by Y. Kulemin
//
// CQueryResultsctrl class implements ActiveX component which we are using to display
// any server SET variable or any query results. There are three names of server 
// variables inside this component:
//
//  m_FullListName -      the server variable which is used to store the complete query
//                        results (if query string is not empty) or we will display the 
//                        contents of this variable + filter/sort(if query string is empty)
//  m_FilteredListName -  the server variable which will be used to store the filter
//                        results of filter and sort. We are displaing this variable in the
//                        component window
//  m_SelectionName    -  the server variable reflects the current selection in the 
//                        component window.
//
// Three strings inside this class are important:
//  
//  m_Query            - contains the query we will run to fill m_FullListName variable
//                       or, if empty, we assume that this variable has something, maybe
//                       nill_set, inside and we will use it to filter->sort->display.
//  m_Filter           - contains the query we will apply to the m_FullListName variable
//                       to filter it and put the results in the m_FilteredListName var.
//  m_Sort             - sort query we will use to sort m_FilteredListName variable.
//----------------------------------------------------------------------------------------


#include "stdafx.h"

#include <fstream.h>

#include "QueryResults.h"
#include "QueryResultsCtl.h"
#include "QueryResultsPpg.h"
#include "..\..\..\discovermdi\include\trace.h"
#define WM_SOCKET_NOTIFY    0x0373
#define WM_SOCKET_DEAD      0x0374


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAP_MAXIMUM_ELEMENTS 1000
#define USER_INCREMENTAL_SEARCH 100

IMPLEMENT_DYNCREATE(CQueryResultsCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CQueryResultsCtrl, COleControl)
	//{{AFX_MSG_MAP(CQueryResultsCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED,IDC_ELEMENTSLIST,OnItemChanged)
	ON_MESSAGE(WM_USERSELECTION,OnUserSelection)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
	ON_MESSAGE(WM_FILTERKEY,OnListKeyDown)
	ON_WM_KEYDOWN()
	ON_NOTIFY(LVN_GETDISPINFO,IDC_ELEMENTSLIST,FillElementInfo)
	ON_NOTIFY(LVN_COLUMNCLICK,IDC_ELEMENTSLIST,OnColumnClick)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CQueryResultsCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CQueryResultsCtrl)
	DISP_PROPERTY_NOTIFY(CQueryResultsCtrl, "FullListName", m_FullListName, OnFullListNameChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CQueryResultsCtrl, "FilteredListName", m_FilteredListName, OnFilteredListNameChanged, VT_BSTR)
	DISP_PROPERTY_EX(CQueryResultsCtrl, "DataSource", GetDataSource, SetDataSource, VT_DISPATCH)
	DISP_PROPERTY_EX(CQueryResultsCtrl, "Query", GetQuery, SetQuery, VT_BSTR)
	DISP_PROPERTY_EX(CQueryResultsCtrl, "Filter", GetFilter, SetFilter, VT_BSTR)
	DISP_PROPERTY_EX(CQueryResultsCtrl, "Sort", GetSort, SetSort, VT_BSTR)
	DISP_PROPERTY_EX(CQueryResultsCtrl, "Element", GetElement, SetElement, VT_I4)
	DISP_PROPERTY_EX(CQueryResultsCtrl, "SelectionName", GetSelectionName, SetSelectionName, VT_BSTR)
	DISP_PROPERTY_EX(CQueryResultsCtrl, "ShortNames", GetShortNames, SetShortNames, VT_BOOL)
	DISP_FUNCTION(CQueryResultsCtrl, "Push", Push, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "Pop", Pop, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "Update", Update, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "AmountSelected", AmountSelected, VT_I4, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "Clear", Clear, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "RowAmount", RowAmount, VT_I4, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "FilteredAmount", FilteredAmount, VT_I4, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "ApplyAttributeChanges", ApplyAttributeChanges, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "CancelAttributeChanges", CancelAttributeChanges, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "AddAttribute", AddAttribute, VT_EMPTY, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CQueryResultsCtrl, "RemoveAttribute", RemoveAttribute, VT_EMPTY, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CQueryResultsCtrl, "CheckAttribute", CheckAttribute, VT_I2, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CQueryResultsCtrl, "Calculate", Calculate, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CQueryResultsCtrl, "SaveSelectionTo", SaveSelectionTo, VT_BOOL, VTS_BSTR VTS_I2)
	DISP_FUNCTION(CQueryResultsCtrl, "GetPrintformat", GetPrintformat, VT_BSTR, VTS_NONE)
	DISP_STOCKPROP_CAPTION()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CQueryResultsCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CQueryResultsCtrl, COleControl)
	//{{AFX_EVENT_MAP(CQueryResultsCtrl)
	EVENT_CUSTOM("ElementsSorted", FireElementsSorted, VTS_I2)
	EVENT_CUSTOM("RButtonClicked", FireRButtonClicked, VTS_I2  VTS_I2)
	EVENT_CUSTOM("ElementDoubleclicked", FireElementDoubleclicked, VTS_BSTR)
	EVENT_CUSTOM("SelectionChanged", FireSelectionChanged, VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CQueryResultsCtrl, 1)
	PROPPAGEID(CQueryResultsPropPage::guid)
END_PROPPAGEIDS(CQueryResultsCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CQueryResultsCtrl, "QUERYRESULTS.QueryResultsCtrl.1",
	0x26c03863, 0xd33, 0x11d2, 0xae, 0xe9, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CQueryResultsCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DQueryResults =
		{ 0x26c03861, 0xd33, 0x11d2, { 0xae, 0xe9, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DQueryResultsEvents =
		{ 0x26c03862, 0xd33, 0x11d2, { 0xae, 0xe9, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwQueryResultsOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CQueryResultsCtrl, IDS_QUERYRESULTS, _dwQueryResultsOleMisc)




//------------------------------------------------------------------------------------------
// This class was designed to map all attributes of the list view instead of server re-query
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Construct new elements map with the maximum size  = size;
//------------------------------------------------------------------------------------------
CElementsMap::CElementsMap(int size) {
	ElementsMap = new Element[size];
	Size=size;
	EntryTime=0;
	ClearMap();
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Removes everything from the element map.
//------------------------------------------------------------------------------------------
CElementsMap::~CElementsMap() {
	delete [] ElementsMap;
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Search in the attribute list for the element with spesified row and
// column. Returns the attribute string pointer if successfull or NULL
// if no such element.
//------------------------------------------------------------------------------------------
CString** CElementsMap::SearchInMap(int row) {
register i;

   for(i=0;i<Size;i++) {
	   if(row==ElementsMap[i].row) {
		   return ElementsMap[i].value;
	   }
   }
   return NULL;
}
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// Adds the attribute value at the row, column position.
//-------------------------------------------------------------------------------------------
void CElementsMap::AddToMap(int row, CString** value, int nSize) {
int index = FILO();

	ElementsMap[index].row    = row;
	ElementsMap[index].size   = nSize;
    ElementsMap[index].value  = value;
    ElementsMap[index].usage  = EntryTime++;
}
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// Removes all elements from the element map.
//-------------------------------------------------------------------------------------------
void CElementsMap::ClearMap(void) {
register i;

	for(i=0;i<Size;i++) {
		if(ElementsMap[i].value!=NULL) {
			for(int j=0;j<ElementsMap[i].size;j++) {
				if(ElementsMap[i].value[j]!=NULL) {
					delete ElementsMap[i].value[j];
					ElementsMap[i].value[j] = NULL; 
				}
			}
			delete[] ElementsMap[i].value;
		}
		ElementsMap[i].row    = -1;
		ElementsMap[i].size   = 0;
		ElementsMap[i].value  = NULL;
		ElementsMap[i].usage  = 0;
	}
}
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// Calculates the index of the last recently used element
//-------------------------------------------------------------------------------------------
int CElementsMap::FILO(void) {
register i;
int FILOindex;
unsigned FILOusage =0xFFFFFFFF;
   
   for(i=0;i<Size;i++) {
	   if(ElementsMap[i].row==-1) return i;
	   if(ElementsMap[i].usage<FILOusage) {
           FILOindex = i;
		   FILOusage = ElementsMap[i].usage;
	   }
   }
   return FILOindex;
}  
//-------------------------------------------------------------------------------------------








/////////////////////////////////////////////////////////////////////////////
// CQueryResultsCtrl::CQueryResultsCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CQueryResultsCtrl

BOOL CQueryResultsCtrl::CQueryResultsCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_QUERYRESULTS,
			IDB_QUERYRESULTS,
			afxRegInsertable | afxRegApartmentThreading,
			_dwQueryResultsOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


//-----------------------------------------------------------------------------------------
// CQueryResultsCtrl::CQueryResultsCtrl - Constructor
// CElementsMap is allocated dynamically here
//-----------------------------------------------------------------------------------------
CQueryResultsCtrl::CQueryResultsCtrl() {
	InitializeIIDs(&IID_DQueryResults, &IID_DQueryResultsEvents);

	m_DataSource       = NULL;
	m_Query            = "";
	m_Filter           = "";
	m_Sort             = "";
	m_FullListName     = "";
	m_FilteredListName = "";
	m_OldFilter        = "";    
	m_AttributeMap     = new CElementsMap(MAP_MAXIMUM_ELEMENTS);
	m_CurrentColumn    = 0;
	m_CurrentRow       = 0;
	m_IncSearchString  = "";
	m_SelectionName    = "";
	m_QueryDurty       = FALSE;
	m_FilterDurty      = FALSE;
	m_SortDurty        = FALSE;
	m_InSelection      = FALSE;
	m_RowElementsAmount=0;
	m_NameSortOrder    = DECENDING;
    m_ShortNames       = FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CQueryResultsCtrl::~CQueryResultsCtrl - Destructor

CQueryResultsCtrl::~CQueryResultsCtrl() {
CString serverCall;
    if(m_FullListName.GetLength()>0) {
        serverCall.Format("unset %s",m_FullListName);
	    EvaluateSync(serverCall);
	}
    if(m_FilteredListName.GetLength()>0) {
        serverCall.Format("unset %s",m_FilteredListName);
	    EvaluateSync(serverCall);
	}
    if(m_SelectionName.GetLength()>0) {
        serverCall.Format("unset %s",m_SelectionName);
	    EvaluateSync(serverCall);
	}
	m_AttributeMap->ClearMap();
	delete m_AttributeMap;
}


/////////////////////////////////////////////////////////////////////////////
// CQueryResultsCtrl::OnDraw - Drawing function

void CQueryResultsCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
}


/////////////////////////////////////////////////////////////////////////////
// CQueryResultsCtrl::DoPropExchange - Persistence support

void CQueryResultsCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CQueryResultsCtrl::OnResetState - Reset control to default state

void CQueryResultsCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange
	m_DataSource=NULL;
    m_Query  = "defines files /mfc";
    m_Filter = "";
    m_Sort   = "";
    m_FullListName="allEntities";
    m_FilteredListName="filteredEntities";
	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CQueryResultsCtrl::AboutBox - Display an "About" box to the user

void CQueryResultsCtrl::AboutBox()
{ 
	CDialog dlgAbout(IDD_ABOUTBOX_QUERYRESULTS);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CQueryResultsCtrl message handlers

//-----------------------------------------------------------------------------------------
// Returns the currently selected data source LPDISPATCH interface.
//-----------------------------------------------------------------------------------------
LPDISPATCH CQueryResultsCtrl::GetDataSource()  {
   return m_DataSource;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This function will be called every time the container tried to set new data source.
// 
void CQueryResultsCtrl::SetDataSource(LPDISPATCH lpszNewValue)  {
CString serverCall;

	// Removing all lists from old server
    if(m_FullListName.GetLength()>0) {
        serverCall.Format("unset %s",m_FullListName);
	    EvaluateSync(serverCall);
	}
    if(m_FilteredListName.GetLength()>0) {
        serverCall.Format("unset %s",m_FilteredListName);
	    EvaluateSync(serverCall);
	}
    if(m_SelectionName.GetLength()>0) {
        serverCall.Format("unset %s",m_SelectionName);
	    EvaluateSync(serverCall);
	}

	m_ElementsList.DeleteAllItems();
    m_AttributeMap->ClearMap();


    m_DataSource=lpszNewValue;


    if(m_SelectionName.GetLength()!=0) {
	   serverCall.Format("set %s [nil_set]",m_SelectionName);
	   EvaluateSync(serverCall);
	}
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// We will clear new query variable
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::OnFullListNameChanged()  {
CString serverCall;
CString serverResults;

    serverCall.Format("info exist %s",m_FullListName);
	EvaluateSync(serverCall,&serverResults);
	if(serverResults=="0") {
        serverCall.Format("set %s [nil_set]",m_FullListName);
	    EvaluateSync(serverCall);
	}
    if(m_SelectionName.GetLength()!=0) {
	   serverCall.Format("set %s [nil_set]",m_SelectionName);
	   EvaluateSync(serverCall);
	}

}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Changing the name of the filtered list variable will result cleaning new variable
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::OnFilteredListNameChanged() {
CString serverCall;
CString serverResults;

    serverCall.Format("info exist %s",m_FilteredListName);
	EvaluateSync(serverCall,&serverResults);
	if(serverResults=="0") {
        serverCall.Format("set %s [nil_set]",m_FilteredListName);
	    EvaluateSync(serverCall);
	}
    if(m_SelectionName.GetLength()!=0) {
	   serverCall.Format("set %s [nil_set]",m_SelectionName);
	   EvaluateSync(serverCall);
	}

}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This property get function will return current query.
//-----------------------------------------------------------------------------------------
BSTR CQueryResultsCtrl::GetQuery()  {

	return m_Query.AllocSysString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This property set function will set new query and will fill complete list of elements.
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::SetQuery(LPCTSTR lpszNewValue)  {
	if(m_DataSource!=NULL) {
	   // New query will be used to fill elements list
	   m_Query=lpszNewValue;
	   m_QueryDurty=TRUE;
	}
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the currently selected filter.
//-----------------------------------------------------------------------------------------
BSTR CQueryResultsCtrl::GetFilter()  {

	return m_Filter.AllocSysString();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Set the new value to the filter. It will not check the filter command syntax, so you
// must always supply the right one.
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::SetFilter(LPCTSTR lpszNewValue)  {
    m_Filter = lpszNewValue;
	m_FilterDurty=TRUE;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Returns the currently selected SORT command.
//-----------------------------------------------------------------------------------------
BSTR CQueryResultsCtrl::GetSort()  {
	return m_Sort.AllocSysString();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Set the new value to the sort string. It will not check the sort command syntax, so you
// must always supply the right one.
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::SetSort(LPCTSTR lpszNewValue)  {
    m_Sort=lpszNewValue;
	m_SortDurty=TRUE;
}
//-----------------------------------------------------------------------------------------




//-----------------------------------------------------------------------------------------
// Returns the first selected element from the element list.
//-----------------------------------------------------------------------------------------
long CQueryResultsCtrl::GetElement()  {
    return m_CurrentRow;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Sets the selection to the element which index mentioned as a parameter.
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::SetElement(long nNewValue)  {
    m_ElementsList.SetItemState(m_CurrentRow,0,LVIS_FOCUSED | LVIS_SELECTED);
    m_ElementsList.SetItemState(nNewValue,LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED);
	m_ElementsList.EnsureVisible(nNewValue,FALSE);
	m_CurrentRow=nNewValue;
	SetModifiedFlag();
}
//-----------------------------------------------------------------------------------------

void CQueryResultsCtrl::Push() 
{
	// TODO: Add your dispatch handler code here

}

BOOL CQueryResultsCtrl::Pop() 
{
	// TODO: Add your dispatch handler code here

	return TRUE;
}



//-----------------------------------------------------------------------------------------
// Process WM_CREATE message. Will create child list view window, image list attached to it
// and will create all columns listed as 1s in m_PropertyMask.
//-----------------------------------------------------------------------------------------
int CQueryResultsCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_ElementsList.Create( LVS_SHAREIMAGELISTS | LVS_REPORT | WS_CHILD | WS_VISIBLE | LVS_SHOWSELALWAYS,
		           CRect(lpCreateStruct->x, 
		                 lpCreateStruct->y,
			             lpCreateStruct->x+lpCreateStruct->cx,
					     lpCreateStruct->y+lpCreateStruct->cy),
			       this,
				   IDC_ELEMENTSLIST);
	m_ElementsList.ModifyStyleEx(0,WS_EX_CLIENTEDGE);
	// Creating our image list and connecting it to the tree control
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

	// Members
	CBitmap b12;
	b12.LoadBitmap(IDB_FIELD);
	m_ImageList.Add(&b12, RGB(255,255,255));

	// Subsystems
	CBitmap b13;
	b13.LoadBitmap(IDB_SUBSYSTEMS);
	m_ImageList.Add(&b13, RGB(255,255,255));

	CBitmap b14;
	b14.LoadBitmap(IDB_INSTANCES);
	m_ImageList.Add(&b14, RGB(255,255,255));

	CBitmap b15;
	b15.LoadBitmap(IDB_AST);
	m_ImageList.Add(&b15, RGB(255,255,255));

	CBitmap b16;
	b16.LoadBitmap(IDB_PROJECTS);
	m_ImageList.Add(&b16, RGB(255,255,255));

	CBitmap b17;
	b17.LoadBitmap(IDB_DEFECT);
	m_ImageList.Add(&b17, RGB(255,255,255));

	CBitmap b18;
	b18.LoadBitmap(IDB_CONST);
	m_ImageList.Add(&b18, RGB(255,255,255));

	CBitmap b19;
	b19.LoadBitmap(IDB_SEMTYPE);
	m_ImageList.Add(&b19, RGB(255,255,255));

	CBitmap b20;
	b20.LoadBitmap(IDB_EVAL);
	m_ImageList.Add(&b20, RGB(255,255,255));

	CBitmap b21;
	b21.LoadBitmap(IDB_LABEL);
	m_ImageList.Add(&b21, RGB(255,255,255));

	CBitmap b22;
	b22.LoadBitmap(IDB_TABLE);
	m_ImageList.Add(&b22, RGB(255,255,255));

	CBitmap b23;
	b23.LoadBitmap(IDB_CURSOR);
	m_ImageList.Add(&b23, RGB(255,255,255));

	CBitmap b24;
	b24.LoadBitmap(IDB_XREF);
	m_ImageList.Add(&b24, RGB(255,255,255));

	CBitmap b25;
	b25.LoadBitmap(IDB_PACKAGES);
	m_ImageList.Add(&b25, RGB(255,255,255));

    m_ElementsList.SetImageList(&m_ImageList,LVSIL_SMALL);
	// Fills the attribute table with attribute command, attrribute name and attribute 
	// status.
	// Forms the list view header by se
	CreateListHeader();
	return 0;
}
//-----------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Keep the list view window the same size as it's reflector window.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::OnSize(UINT nType, int cx, int cy)  {
	COleControl::OnSize(nType, cx, cy);
	if(::IsWindow(m_ElementsList)) m_ElementsList.MoveWindow(0,0,cx,cy,TRUE);
}
//-----------------------------------------------------------------------------------------

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
int CQueryResultsCtrl::EvaluateSync(CString& command, CString* results) {
CString MethodName("AccessSync"); // We will run AccessSync method of the DataSource
CString ServerStatus("IsServerBusy"); 
DISPID accessID;                  // IDispatch id for the AccessSync command
HRESULT hresult;
CString cmd=command;


    // if there is no data source assigned to this control, we will be unable to
    // evaluate this function.
    if(m_DataSource==NULL) return ERROR_DATA_SOURCE;
	if(results == NULL) cmd+=";set tmp 1";


    // Trying to receve method ID from it's name.
	BSTR statusBSTR=ServerStatus.AllocSysString();
    hresult = m_DataSource->GetIDsOfNames(IID_NULL,&statusBSTR, 
		                                  1,LOCALE_SYSTEM_DEFAULT,&accessID);
	VARIANT busyBOOL;
	unsigned int errors_in_arg;
    DISPPARAMS Param;
	Param.cArgs     = 0;
	Param.cNamedArgs=0;
    hresult = m_DataSource->Invoke(accessID,
                                   IID_NULL,
							       LOCALE_SYSTEM_DEFAULT,
							       DISPATCH_METHOD,
							       &Param,
							       &busyBOOL,
							       NULL,
							       &errors_in_arg);
	if (busyBOOL.intVal!=0) {
		 return ERROR_ACCESS_SERVER;
	}


    // Trying to receve method ID from it's name.
	BSTR methodBSTR=MethodName.AllocSysString();
    hresult = m_DataSource->GetIDsOfNames(IID_NULL,&methodBSTR, 
		                                  1,LOCALE_SYSTEM_DEFAULT,&accessID);

	// Interface doesn't support AccessSync method
	if(hresult!=S_OK) return ERROR_ACCESS_SUPPORT;

	// Converting command into new OEM string wich will be passed into OLE automation.
	// We will free when pushing into the stack
	BSTR access_call = cmd.AllocSysString();

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
	::SysFreeString(access_call);
	return EVAL_OK;
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// This function will fill complete table of elements. In the nearest future it will only 
// fill one page and additional reading will take place when scrolling.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::FillElementsList() {
unsigned register i;
unsigned int elements;
CString serverCall;
CString serverResults;
   
    m_AttributeMap->ClearMap();
	m_ElementsList.DeleteAllItems();
	m_SelectionIndexes.RemoveAll();
	// Quering the size of the resulted list of elements
    serverCall.Format("size $%s",m_FilteredListName);
	EvaluateSync(serverCall,&serverResults);
	elements=atoi(serverResults);
	m_ElementsList.SetItemCount(elements);
	for(i=0;i<elements;i++) InsertElementRow(i);

}
//------------------------------------------------------------------------------------------




//------------------------------------------------------------------------------------------
// This function process attribute list to form the view header.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::CreateListHeader(void) {
POSITION at = m_AttributeList.GetHeadPosition();
LV_COLUMN lvColumn;


	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt=LVCFMT_LEFT;
	while(m_ElementsList.DeleteColumn(0));

	// Creating element name column
	CString elementName;
	elementName.LoadString(IDS_NAME_COLUMN);
	lvColumn.iSubItem=0;
	lvColumn.pszText=(char *)(LPCSTR)elementName;
	lvColumn.cx=180;
    m_ElementsList.InsertColumn(0,&lvColumn);


	// No attributes if the attribute table is not filled.
	if(at==NULL) return;

    // Inserting element attributes columns
	int itemIndex=0;
	while(at) {
	   lvColumn.iSubItem=itemIndex;
	   lvColumn.pszText=(char *)(LPCSTR)m_AttributeList.GetAt(at).GetReadableName();
	   lvColumn.cx=50; 
       m_ElementsList.InsertColumn(++itemIndex,&lvColumn);
	   m_AttributeList.GetNext(at);
	} 
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Inserts new line into the elements view with element name and optional element attributes
// which this function inserts only if the status filed in the attribute table is set.
// num is zero-based.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::InsertElementRow(int num) {
LV_ITEM lvItem;

   // Inserting item into the table
   lvItem.mask           = LVIF_TEXT | TVIF_IMAGE;
   lvItem.pszText        = LPSTR_TEXTCALLBACK;
   lvItem.cchTextMax     = 0;
   lvItem.iImage         = I_IMAGECALLBACK;
   lvItem.iItem          = num;
   lvItem.iSubItem       = 0;
   m_ElementsList.InsertItem(&lvItem);

}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Executes the query filling the variable with the m_FullListName on the client side.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::RunQuery(void) {
CString serverCall;
CString serverResults;
    // Filling the complete query results list
	if(m_Query.GetLength()!=0)  {
       serverCall.Format("set %s [%s]",m_FullListName,m_Query);
	   EvaluateSync(serverCall);
	}
	// Asking the size of the list - even if no query set and we are looking
	// at the variable itself instead of looking on the query results
    serverCall.Format("size $%s",m_FullListName);
	EvaluateSync(serverCall,&serverResults);
    m_RowElementsAmount=atoi(serverResults);
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Executes the filter command on the server taking the data from the server variable 
// called m_FullListName and storing filter results in the server variable named
// m_FilteredListName.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::RunFilter(void) {
CString serverCall;
    // Filling the filtered list
	if(m_Filter.GetLength()!=0)  {
       serverCall.Format("set %s [%s $%s]",m_FilteredListName,m_Filter,m_FullListName);
	   EvaluateSync(serverCall);
	} else {
       serverCall.Format("set %s $%s",m_FilteredListName,m_FullListName);
	   EvaluateSync(serverCall);
	}
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Executes the sort command on the server taking data from the server variable named 
// m_FilteredListName and storing the results in the same variable.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::RunSort(void) {
CString serverCall;

    // Filling the filtered list
	if(m_Sort.GetLength()!=0)  {
       serverCall.Format("set %s [%s $%s]",m_FilteredListName,m_Sort,m_FilteredListName);
	   EvaluateSync(serverCall);
	}

}
//------------------------------------------------------------------------------------------

CString** CQueryResultsCtrl::GetItemDescription(int nIdx,bool bApplyFormat) {
	CString**	pCachedValues = NULL;
	pCachedValues=m_AttributeMap->SearchInMap(nIdx);
	if(pCachedValues==NULL) {
		if(bApplyFormat) {
			if(EvaluateSync(FormPrintformat(),NULL)!=1) {
				return NULL;
			}
		}
			
		CString serverRequest;
		CString	serverResults;
		serverRequest.Format("print $%s %d",m_FilteredListName,nIdx+1);
		if(EvaluateOutput(serverRequest,&serverResults)!=1) {
			return NULL;
		}

		int nAttrCount = GetAttributesCount() + 5; // +kind+name+cname+lname+line = 5
		pCachedValues = new CString*[nAttrCount];
		memset(pCachedValues,0,nAttrCount*sizeof(CString*));
		//-------- Extracting attribute from the query result string -------
		int nStart = 0;
		int nFieldIdx = 0;
		int nResultsLen = serverResults.GetLength();
		// searching attribute start position
		for(int i=0;i<nResultsLen;i++) {
			if(serverResults[i]=='\10') {
				CString* pszAttributeText = new CString(serverResults.Mid(nStart,i-nStart));
				pCachedValues[nFieldIdx] = pszAttributeText;
				nStart = i + 1;
				nFieldIdx++;
			}
		}

		m_AttributeMap->AddToMap(nIdx,pCachedValues,nAttrCount);

		if(bApplyFormat) {
			serverRequest="printformat \"%s \t %s\" name kind";
			if(EvaluateSync(serverRequest,NULL)!=1) {
				return NULL;
			}
		}
	}
	return pCachedValues;
}
//------------------------------------------------------------------------------------------
// This callback will run every time the list view needs information to display items.
// It will nor run second time for the items and icons, but still will run to obtain 
// information about sub-items (attributes). The LVIF_DI_SETITEM doesn't influence on the
// attributes.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::FillElementInfo(NMHDR* pNMHDR, LRESULT* pResult) {
	LV_DISPINFO *pInfo = (LV_DISPINFO *)pNMHDR;
	CString		szItemText;
	CString**	pItemDescr = NULL;

	// Will give information only for visible items
	if( (pInfo->item.iItem >= m_ElementsList.GetTopIndex()-2) && 
		(pInfo->item.iItem <= m_ElementsList.GetTopIndex()+m_ElementsList.GetCountPerPage()+2)) {

		pItemDescr = GetItemDescription(pInfo->item.iItem);
		if(pItemDescr == NULL) {
			RECT rect;
			m_ElementsList.GetItemRect(pInfo->item.iItem,&rect,LVIR_BOUNDS);
			m_ElementsList.InvalidateRect(&rect,FALSE);
			pInfo->item.pszText=LPSTR_TEXTCALLBACK;
			*pResult=-1;
			return;
		}

		CString kind = *pItemDescr[0]; // kind

		pInfo->item.mask|=LVIF_DI_SETITEM;
		if(pInfo->item.mask & LVIF_TEXT) {
			// Runs query to get the element name and all mentioned element attributes
			switch(pInfo->item.iSubItem) {
				case 0 : 
					if(kind!="ast") {
						if(m_ShortNames==FALSE) {
							szItemText = *pItemDescr[1]; // name
						} else {
							szItemText = *pItemDescr[2]; // cname
						}
					} else {
						CString lname = *pItemDescr[3]; // lname
						lname.Replace("\\","/");
						int nNameStart = lname.ReverseFind('/');
						// even if ReverseFine will return -1
						// we'll have a correct result - the whole 
						// lname string (because -1 + 1 = 0)
						szItemText=lname.Mid(nNameStart+1)+" : "+(*pItemDescr[4]);
					}
					break;
				default: // Attributes
					szItemText = *pItemDescr[pInfo->item.iSubItem+5-1];
					break;
			}
			::lstrcpyn(pInfo->item.pszText,(LPCTSTR)szItemText,pInfo->item.cchTextMax);
		}
		//---------------------------------------------------------------------------------
		
		// Setting the element image from category
		if(pInfo->item.mask & LVIF_IMAGE) {
			if(kind=="module")      pInfo->item.iImage = 0;
			if(kind=="funct")       pInfo->item.iImage = 1;
			if(kind=="var")         pInfo->item.iImage = 2;
			if(kind=="struct")      pInfo->item.iImage = 3;
			if(kind=="union")       pInfo->item.iImage = 5;
			if(kind=="enum")        pInfo->item.iImage = 6;
			if(kind=="typedef")     pInfo->item.iImage = 7;
			if(kind=="macro")       pInfo->item.iImage = 8;
			if(kind=="templ")       pInfo->item.iImage = 9;
			if(kind=="string")      pInfo->item.iImage = 10;
			if(kind=="local")       pInfo->item.iImage = 11;
			if(kind=="field")       pInfo->item.iImage = 12;
			if(kind=="group")       pInfo->item.iImage = 13;
			if(kind=="instance")    pInfo->item.iImage = 14;
			if(kind=="ast")         pInfo->item.iImage = 15;
			if(kind=="proj")        pInfo->item.iImage = 16;
			if(kind=="defect")      pInfo->item.iImage = 17;
			if(kind=="defect")      pInfo->item.iImage = 17;
			if(kind=="constant")    pInfo->item.iImage = 18;
			if(kind=="semtype")     pInfo->item.iImage = 19;
			if(kind=="evalue")      pInfo->item.iImage = 20;
			if(kind=="label")       pInfo->item.iImage = 21;
			if(kind=="table")       pInfo->item.iImage = 22;
			if(kind=="cursor")      pInfo->item.iImage = 23;
			if(kind=="externref")   pInfo->item.iImage = 24;
			if(kind=="package")     pInfo->item.iImage = 25;
		}
	} else {
		pInfo->item.pszText = LPSTR_TEXTCALLBACK;
		pInfo->item.iImage=-1;
	}
   *pResult=0;
}
//------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------
// Queries the server to send the results in the specified format.
// valid for "print" query.
//-------------------------------------------------------------------------------------
CString& CQueryResultsCtrl::FormPrintformat(void) {
static CString formattedLine;
CString queryLine;
POSITION at;

     formattedLine="printformat \""; 
     formattedLine += CString("%s\10"); // kind 
	 queryLine+=" kind";
     formattedLine += CString("%s\10"); // name
	 queryLine+=" name";
     formattedLine += CString("%s\10"); // cname
	 queryLine+=" cname";
     formattedLine += CString("%s\10"); // lname
	 queryLine+=" lname";
     formattedLine += CString("%s\10"); // line
	 queryLine+=" line";
     at = m_AttributeList.GetHeadPosition();
     while(at) {
	    formattedLine+=CString("%s\10"); 
		queryLine+=" ";
        queryLine+= m_AttributeList.GetAt(at).GetAccessOption();
	    m_AttributeList.GetNext(at); 
	 } 
	 formattedLine+="\" ";
	 formattedLine+=queryLine;
	 return formattedLine;
}
//----------------------------------------------------------------------------------------

int CQueryResultsCtrl::GetAttributesCount() {
	return m_AttributeList.GetCount();
}

bool CQueryResultsCtrl::HasAttributes() {
	return !m_AttributeList.IsEmpty();
}

//----------------------------------------------------------------------------------------
// This stupid function, as well as AccessOutput in the data source we need to catch 
// print output. It is different from any command output! Hope in future we will remove
// this stuff as well as AccessOutput function from the data source.
//----------------------------------------------------------------------------------------
int CQueryResultsCtrl::EvaluateOutput(CString& command, CString* results) {
CString MethodName("AccessOutput"); // We will run AccessSync method of the DataSource
CString ServerStatus("IsServerBusy"); 
DISPID accessID;                  // IDispatch id for the AccessSync command
HRESULT hresult;
CString cmd = command;

    // if there is no data source assigned to this control, we will be unable to
    // evaluate this function.
    if(m_DataSource==NULL) return ERROR_DATA_SOURCE;


    // Trying to receve method ID from it's name.
	BSTR statusBSTR=ServerStatus.AllocSysString();
    hresult = m_DataSource->GetIDsOfNames(IID_NULL,&statusBSTR, 
		                                  1,LOCALE_SYSTEM_DEFAULT,&accessID);
	VARIANT busyBOOL;
	unsigned int errors_in_arg;
    DISPPARAMS Param;
	Param.cArgs     = 0;
	Param.cNamedArgs=0;
    hresult = m_DataSource->Invoke(accessID,
                                   IID_NULL,
							       LOCALE_SYSTEM_DEFAULT,
							       DISPATCH_METHOD,
							       &Param,
							       &busyBOOL,
							       NULL,
							       &errors_in_arg);
	if (busyBOOL.intVal!=0) {
		return ERROR_ACCESS_SERVER;
	}

    // Trying to receve method ID from it's name.
	BSTR methodBSTR=MethodName.AllocSysString();
    hresult = m_DataSource->GetIDsOfNames(IID_NULL,&methodBSTR, 
		                                  1,LOCALE_SYSTEM_DEFAULT,&accessID);

	// Interface doesn't support AccessSync method
	if(hresult!=S_OK) return ERROR_ACCESS_SUPPORT;

	// Converting command into new OEM string wich will be passed into OLE automation.
	// We will free when pushing into the stack
	BSTR access_call = cmd.AllocSysString();

	// Creating parameter structure - only one parameter will be passed as argument - command string
    DISPPARAMS cmdParamStruct;
	cmdParamStruct.rgvarg              = new VARIANT[1];
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
	::SysFreeString(access_call);

	if(hresult!=S_OK) return ERROR_ACCESS_COMMAND;
	if(results!=NULL) *results=resBSTR.bstrVal;
	return EVAL_OK;
}
//-----------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// This function will run every time user clicked some column. It will modify the
// sort property to sort the selected column and call RunSort function.
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) {
NM_LISTVIEW *pInfo = (NM_LISTVIEW *)pNMHDR;
int column = pInfo->iSubItem;
CString parameter;
int direction;

    // This column will be marked as selected,
    m_CurrentColumn=column;
    if(column==0) {
		parameter = "name";
	    direction=m_NameSortOrder;
	    m_NameSortOrder = (direction==ASCENDING) ? DECENDING : ASCENDING;
	} else {
       //------- Extracting element info for this column ---------------
	   POSITION at = m_AttributeList.GetHeadPosition(); 
	   int count = 1;
	   while(at) {
		   if(count++ == column) break;
		   m_AttributeList.GetNext(at); 
	   };
	   if(at==NULL) return;
	   parameter = m_AttributeList.GetAt(at).GetAccessOption();
	   direction = m_AttributeList.GetAt(at).GetSortDirection();
	   m_AttributeList.GetAt(at).SetSortDirection(direction==ASCENDING ? DECENDING : ASCENDING);
	}
	//---------------------------------------------------------------
	CString sort;
	if(direction==ASCENDING) sort.Format("sort -a %s",parameter);
	else                     sort.Format("sort -d %s",parameter);
	SetSort(sort);
	Update();
	m_IncSearchString  = "";
	*pResult=1;
}
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// Will run incremental search using the currently selected column.
//--------------------------------------------------------------------------------------
LRESULT CQueryResultsCtrl::OnListKeyDown(WPARAM vKey,LPARAM flags) {

   KillTimer(USER_INCREMENTAL_SEARCH);   
   SetTimer(USER_INCREMENTAL_SEARCH,1000,NULL);
   if(vKey==8) {
	   m_IncSearchString="";
   } else {
      //converting virtual key into upper-case character.
      char character =::MapVirtualKey(vKey,2);
      if(character == 0) return 0;
      m_IncSearchString+=character;
   }
   return 1;
}
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Running this function every time item data changed. We will select only 
// unselected->selected jumps to modify current row variable.
//--------------------------------------------------------------------------------------
void CQueryResultsCtrl::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult) {
NM_LISTVIEW *pInfo = (NM_LISTVIEW *)pNMHDR;

    // selection changed from unselected to selected.
    if(((pInfo->uOldState & LVIS_SELECTED)==0) && ((pInfo->uNewState & LVIS_SELECTED) !=0)) {
		m_CurrentRow=pInfo->iItem;
		// Adding selected variable to the list
		m_SelectionIndexes.AddTail(pInfo->iItem);
		m_InSelection=TRUE;
	}

	// Selection changed from selected to unselected.
    if(((pInfo->uOldState & LVIS_SELECTED)!=0) && ((pInfo->uNewState & LVIS_SELECTED) ==0)) {
		// Removes unselected element from the list
		POSITION p; 
		int index;
		p=m_SelectionIndexes.GetHeadPosition();
		if(p!=NULL) {
		   do {
		       index=m_SelectionIndexes.GetAt(p);
			   if(index==pInfo->iItem) {
                   m_SelectionIndexes.RemoveAt(p);
				   break;
			   }
			   m_SelectionIndexes.GetNext(p);
		   } while(p!=NULL);
		}
	    m_InSelection=TRUE;
	}
}
//----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Callback will run every time the user finished with the selection in the list. We need
// this + CListCtrl-derived class to fire the selection changed event only at the end of
// the selection. Unfortunately windows list control sends the selection changed event 
// every time element selected/unselected, even in the case we are using mouse Ctrl+arrow
// key for multiple selections. We do not really need to run 50 queries when using 
// multiple selection for 50 elements!
//------------------------------------------------------------------------------------------
LRESULT CQueryResultsCtrl::OnUserSelection(WPARAM,LPARAM) {
CString serverCall;
CString serverResults;

	if(m_InSelection==TRUE) {
		// Syncronizing the server selection variable with the list selection.
		FillSelectionVariable();

	    serverCall.Format("sym_info $%s %ld ",m_FilteredListName,m_CurrentRow+1);
	    EvaluateSync(serverCall,&serverResults);
		FireSelectionChanged(serverResults);
	    m_InSelection=FALSE;
	}
	return 0;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// m_SelectionName member variable contains the server variable name which contains 
// the list of the currently selected entities. Use this function to obtain the server
// variable name.
//------------------------------------------------------------------------------------------
BSTR CQueryResultsCtrl::GetSelectionName()  {
	return m_SelectionName.AllocSysString();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// m_SelectionName member variable contains the server variable name which contains 
// the list of the currently selected entities. Use this function to set new server variable
// name.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::SetSelectionName(LPCTSTR lpszNewName)  {
CString query;
    // unset the old selection variable
    if(m_DataSource!=NULL && m_SelectionName.GetLength()>0) {
        query.Format("unset %s",m_SelectionName);	
		EvaluateSync(query);
	}
    // set new selection variable	
	m_SelectionName=lpszNewName;

	// fill the variable with the current selection.
    if(m_DataSource!=NULL && m_SelectionName.GetLength()>0) {
	   FillSelectionVariable();
	}
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// This function will fill the server variable named m_SelectionName with the currently
// selected items.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::FillSelectionVariable() {
CString query;
CString add;

    if(m_SelectionName.GetLength()==0) return;
	query.Format("set %s [nil_set]",m_SelectionName);
    EvaluateSync(query); 
    if(m_DataSource!=NULL && m_SelectionName.GetLength()>0) {
		POSITION p; 
		int index;
		p=m_SelectionIndexes.GetHeadPosition();
		if(p!=NULL) {
		   CString indexes;
		   int count=0;
		   do {
		       index=m_SelectionIndexes.GetAt(p);
			   add.Format("%d ",index+1);
			   count++;
			   indexes+=add;
			   if(count>=500) {
			       add.Format("sappend $%s [list $%s %s]",m_SelectionName,m_FilteredListName,indexes);
			       EvaluateSync(add);
				   indexes="";
				   count=0;
			   }
			   m_SelectionIndexes.GetNext(p);
		   } while(p!=NULL);
		   if(count) {
		      add.Format("sappend $%s [list $%s %s]",m_SelectionName,m_FilteredListName,indexes);
	          EvaluateSync(add);
		   }
		}
    
    }
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// We will use this method to fill all results list. This will take some time.
//------------------------------------------------------------------------------------------
void CQueryResultsCtrl::Update()  {
	if(m_DataSource!=NULL) {
	   Calculate();
       FillElementsList();
	   FillSelectionVariable();
	   SetModifiedFlag();
	}
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This method returns the amount of the elements selected in the list.
//-----------------------------------------------------------------------------------------
long CQueryResultsCtrl::AmountSelected()  {
	long amount=m_SelectionIndexes.GetCount();
	return amount;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Will set the empty query string and zero all storage server variables.
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::Clear()  {
    CString query;
	m_Query="";
	query.Format("set %s [nil_set]",m_SelectionName);
	EvaluateSync(query);
	query.Format("set %s [nil_set]",m_FullListName);
	EvaluateSync(query);
	query.Format("set %s [nil_set]",m_FilteredListName);
	EvaluateSync(query);
}
//---------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Returns the amount of the elements afrer query command. If filter is not
// applied, this amout will be equal to the amount of the elements in 
// m_FilteredListName variable.
//-----------------------------------------------------------------------------------------
long CQueryResultsCtrl::RowAmount()  {
	return m_RowElementsAmount;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Returns the amount of the elements afrer filter command.
//-----------------------------------------------------------------------------------------
long CQueryResultsCtrl::FilteredAmount()  {
   return m_ElementsList.GetItemCount();
}
//-----------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// This method is designed to modify and re-fill attribute list after changed
// in attribute order or amount/kind need to be reflected in the list.
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::ApplyAttributeChanges()  {
	m_AttributeList.RemoveAll();
	m_AttributeList.AddHead(&m_ModifiedAttributes);
	CreateListHeader();
	FillElementsList();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Cancels all changes applied by  AddAttribute and RemoveAttribute functions.
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::CancelAttributeChanges()  {
	m_ModifiedAttributes.RemoveAll();
	m_ModifiedAttributes.AddHead(&m_AttributeList);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This method adds new attribute to the visible attributes list.
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::AddAttribute(LPCTSTR logname, LPCTSTR option)  {
CAttributeInfo attr;

    attr.SetReadableName(logname);
    attr.SetAccessOption(option);
	attr.SetSortDirection(ASCENDING);
	m_ModifiedAttributes.AddTail(attr);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method will remove attribute from the visible attributes list. It will try to find
// the attribute by it's logical name, if it is not empty. If logname is empty, it will
// try to find attribute using it's access options. 
//-----------------------------------------------------------------------------------------
void CQueryResultsCtrl::RemoveAttribute(LPCTSTR logname, LPCTSTR option)  {
POSITION at;

	if(strlen(logname)==0) {
		if(strlen(option)==0) return;
		// searching by access option
	    at = m_ModifiedAttributes.GetHeadPosition(); 
	    while(at!=NULL) {
		   if ( m_ModifiedAttributes.GetAt(at).GetAccessOption() == CString(option) ) break;
		   m_ModifiedAttributes.GetNext(at); 
	    }
	    if(at!=NULL) m_ModifiedAttributes.RemoveAt(at);
	} else { // searching by logical name
	    at = m_ModifiedAttributes.GetHeadPosition(); 
	    while(at!=NULL) {
		   if ( m_ModifiedAttributes.GetAt(at).GetReadableName() == CString(logname) ) break;
		   m_ModifiedAttributes.GetNext(at); 
	    }
	    if(at!=NULL) m_ModifiedAttributes.RemoveAt(at);
	}
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method will check if attribute is in the visible attributes list. 
// It will try to find  the attribute by it's logical name, if it is not empty. 
// If logname is empty, it will try to find attribute using it's access options. 
// Returns attribute index starting from 0 or -1 if no attribute present.
//-----------------------------------------------------------------------------------------
short CQueryResultsCtrl::CheckAttribute(LPCTSTR logname, LPCTSTR option)  {
POSITION at;
int index=0;

	if(strlen(logname)==0) {
		if(strlen(option)==0) return FALSE;
		// searching by access option
	    at = m_ModifiedAttributes.GetHeadPosition(); 
	    while(at!=NULL) {
		   if ( m_ModifiedAttributes.GetAt(at).GetAccessOption() == CString(option) ) break;
		   index++;
		   m_ModifiedAttributes.GetNext(at); 
	    }
	    if(at!=NULL) return index;
	} else { // searching by logical name
	    at = m_ModifiedAttributes.GetHeadPosition(); 
	    while(at!=NULL) {
		   if ( m_ModifiedAttributes.GetAt(at).GetReadableName() == CString(logname) ) break;
		   index++;
		   m_ModifiedAttributes.GetNext(at); 
	    }
	    if(at!=NULL) return index;
	}
	return -1;
}
//-----------------------------------------------------------------------------------------



void CQueryResultsCtrl::OnRButtonDown(UINT nFlags, CPoint point)  {
    FireRButtonClicked((short)point.x,(short)point.y);
}

void CQueryResultsCtrl::Calculate()  {
CString serverCall;

   if(m_QueryDurty==TRUE) {
	   RunQuery();
	   RunFilter();
	   RunSort();
	   m_QueryDurty =FALSE;
	   m_FilterDurty=FALSE;
	   m_SortDurty  =FALSE;
       if(m_SelectionName.GetLength()!=0) {
	      serverCall.Format("set %s [nil_set]",m_SelectionName);
	      EvaluateSync(serverCall);
	   }
   }
   if(m_FilterDurty==TRUE) {
	   RunFilter();
	   RunSort();
	   m_FilterDurty=FALSE;
	   m_SortDurty  =FALSE;
       if(m_SelectionName.GetLength()!=0) {
	      serverCall.Format("set %s [nil_set]",m_SelectionName);
	      EvaluateSync(serverCall);
	   }
    }
	if(m_SortDurty==TRUE) {
	   RunSort();
	   m_SortDurty=FALSE;
       if(m_SelectionName.GetLength()!=0) {
	      serverCall.Format("set %s [nil_set]",m_SelectionName);
	      EvaluateSync(serverCall);
	   }
	}
}

void CQueryResultsCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)  {
CString res;
    res.Format("%s %ld",m_FilteredListName,m_CurrentRow);
	FireElementDoubleclicked(res);
}

void CQueryResultsCtrl::OnTimer(UINT nIDEvent)  {
	if(nIDEvent==USER_INCREMENTAL_SEARCH) {
        CString filterCommand;
		if(m_IncSearchString.GetLength()>0) {
           filterCommand.Format("filter strstr(strupr(name),\"%s\")!=\"\"",m_IncSearchString);
		} else {
		   filterCommand="";
		}
        SetFilter(filterCommand);
        Update();
	    KillTimer(USER_INCREMENTAL_SEARCH);
		m_IncSearchString="";
	}
	CString serverCall;
	CString serverResults;
    serverCall.Format("sym_info $%s %ld ",m_FilteredListName,m_CurrentRow+1);
    EvaluateSync(serverCall,&serverResults);
	FireSelectionChanged(serverResults);
	COleControl::OnTimer(nIDEvent);
}



BOOL CQueryResultsCtrl::GetShortNames()  {
	return m_ShortNames;
}

void CQueryResultsCtrl::SetShortNames(BOOL bNewValue)  {
    m_ShortNames = bNewValue;
	int nCount=m_ElementsList.GetItemCount();
	for(int i=0;i<nCount;i++) {
		m_ElementsList.SetItemText(i,0,LPSTR_TEXTCALLBACK);
	}
}

BOOL CQueryResultsCtrl::SaveSelectionTo(LPCTSTR szFileName, short cDelimiter) 
{
	CString szAttribute;
	CString serverRequest,serverResults;
	int nColumns = m_ElementsList.GetHeaderCtrl()->GetItemCount();
	CString** pItemDescr = NULL;

	ofstream fileStream(szFileName);

	POSITION p; 
	p=m_SelectionIndexes.GetHeadPosition();
	bool bFirstItem = true;
	while(p!=NULL) {
		int iItem=m_SelectionIndexes.GetAt(p);
		m_SelectionIndexes.GetNext(p); // get next element

		// Getting the element information
		pItemDescr = GetItemDescription(iItem,(bFirstItem || p==NULL));
		if(pItemDescr == NULL) return FALSE;

		// getting the item name
		CString kind = *pItemDescr[0]; // kind
		if(kind!="ast") {
			if(m_ShortNames==FALSE) {
				szAttribute = *pItemDescr[1]; // name
			} else {
				szAttribute = *pItemDescr[2]; // cname
			}
		} else {
			CString lname = *pItemDescr[3]; // lname
			lname.Replace("\\","/");
			int nNameStart = lname.ReverseFind('/');
			// even if ReverseFine will return -1
			// we'll have a correct result - the whole 
			// lname string (because -1 + 1 = 0)
			szAttribute=lname.Mid(nNameStart+1)+" : "+(*pItemDescr[4]);
		}
		fileStream << szAttribute;
		
		// ... and all mentioned element attributes
		if(HasAttributes())   {
			int iAttr=1; //we've already saved the first attribute - name
			while(iAttr<nColumns) {
				fileStream << (char)cDelimiter; // add the delimiter after a name
				szAttribute=*pItemDescr[iAttr+5-1];
				fileStream << szAttribute;
				iAttr++;
			}
		}
		fileStream << endl;
		bFirstItem = false;
	}
	fileStream.close();

	return TRUE;
}

BSTR CQueryResultsCtrl::GetPrintformat() 
{
	CString formattedLine;
	CString queryLine;
	POSITION at;

	if(m_ShortNames==FALSE)
		queryLine = "name";
	else
		queryLine = "cname";

	formattedLine="printformat \"%s"; // name is the first element
	at = m_AttributeList.GetHeadPosition();
	while(at) {
		formattedLine+='\t';
		formattedLine+=CString("%s"); 
		queryLine+=" ";
		queryLine+= m_AttributeList.GetAt(at).GetAccessOption();
		m_AttributeList.GetNext(at); 
	} 
	formattedLine+="\" ";
	formattedLine+=queryLine;

	return formattedLine.AllocSysString();
}
