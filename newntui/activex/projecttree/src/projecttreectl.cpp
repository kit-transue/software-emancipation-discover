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
// ProjectTreeCtl.cpp : Implementation of the CProjectTreeCtrl OLE control class.

#include "stdafx.h"
#include "ProjectTree.h"
#include "ProjectTreeCtl.h"
#include "ProjectTreePpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CProjectTreeCtrl, COleControl)

/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CProjectTreeCtrl, COleControl)
	//{{AFX_MSG_MAP(CProjectTreeCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED,   IDC_PROJECTTREE, OnSelchangedProjectTree)
	ON_NOTIFY(NM_DBLCLK,        IDC_PROJECTTREE, OnDoubleclickProjectTree) 
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_PROJECTTREE, OnExpandProjectTree)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT,  OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CProjectTreeCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CProjectTreeCtrl)
	DISP_PROPERTY_EX(CProjectTreeCtrl, "DataSource", GetDataSource, SetDataSource, VT_DISPATCH)
	DISP_PROPERTY_EX(CProjectTreeCtrl, "SelectedNode", GetSelectedNode, SetSelectedNode, VT_BSTR)
	DISP_FUNCTION(CProjectTreeCtrl, "Query", Query, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CProjectTreeCtrl, "Clean", Clean, VT_EMPTY, VTS_NONE)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CProjectTreeCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CProjectTreeCtrl, COleControl)
	//{{AFX_EVENT_MAP(CProjectTreeCtrl)
	EVENT_CUSTOM("NodeChanged", FireNodeChanged, VTS_BSTR)
	EVENT_CUSTOM("NodeDoubleclicked", FireNodeDoubleclicked, VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CProjectTreeCtrl, 1)
	PROPPAGEID(CProjectTreePropPage::guid)
END_PROPPAGEIDS(CProjectTreeCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CProjectTreeCtrl, "PROJECTTREE.ProjectTreeCtrl.1",
	0xb888ed83, 0x911, 0x11d2, 0xae, 0xe6, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CProjectTreeCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DProjectTree =
		{ 0x77b20d5, 0x934, 0x11d2, { 0xae, 0xe6, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DProjectTreeEvents =
		{ 0x77b20d6, 0x934, 0x11d2, { 0xae, 0xe6, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwProjectTreeOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CProjectTreeCtrl, IDS_PROJECTTREE, _dwProjectTreeOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CProjectTreeCtrl::CProjectTreeCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CProjectTreeCtrl

BOOL CProjectTreeCtrl::CProjectTreeCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_PROJECTTREE,
			IDB_PROJECTTREE,
			afxRegInsertable | afxRegApartmentThreading,
			_dwProjectTreeOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CProjectTreeCtrl::CProjectTreeCtrl - Constructor

CProjectTreeCtrl::CProjectTreeCtrl()
{
	InitializeIIDs(&IID_DProjectTree, &IID_DProjectTreeEvents);

	// TODO: Initialize your control's instance data here.
	m_DataSource=NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CProjectTreeCtrl::~CProjectTreeCtrl - Destructor

CProjectTreeCtrl::~CProjectTreeCtrl()
{
	// TODO: Cleanup your control's instance data here.
	m_FullNameList.RemoveAll();
}


/////////////////////////////////////////////////////////////////////////////
// CProjectTreeCtrl::OnDraw - Drawing function

void CProjectTreeCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	if(::IsWindow(m_Tree)) {
		m_Tree.Invalidate();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CProjectTreeCtrl::DoPropExchange - Persistence support

void CProjectTreeCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CProjectTreeCtrl::OnResetState - Reset control to default state

void CProjectTreeCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CProjectTreeCtrl::AboutBox - Display an "About" box to the user

void CProjectTreeCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_PROJECTTREE);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CProjectTreeCtrl message handlers

LPDISPATCH CProjectTreeCtrl::GetDataSource() 
{
	// TODO: Add your property handler here
    return m_DataSource;
}


//--------------------------------------------------------------------------------------------------------
// This property accessed by Get() method is designed to recive The IDispatch interface to the DataSource
// object.
//--------------------------------------------------------------------------------------------------------
void CProjectTreeCtrl::SetDataSource(LPDISPATCH newValue)  {

    // Control will use new data source to query model
	Clean();
	m_DataSource=newValue;
	FillProjectTree();
	SetSelectedNode("/");
}
//-------------------------------------------------------------------------------------------------------

int CProjectTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_Tree.Create(WS_VISIBLE |  TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_LINESATROOT | TVS_DISABLEDRAGDROP,
		           CRect(lpCreateStruct->x,
		                 lpCreateStruct->y,
			             lpCreateStruct->x+lpCreateStruct->cx,
					     lpCreateStruct->y+lpCreateStruct->cy),
			       this,
				   IDC_PROJECTTREE);
	m_Tree.ModifyStyleEx( 0, WS_EX_CLIENTEDGE);
	// Creating our image list and connecting it to the tree control
	m_ImageList.Create(16,16,FALSE,2,0);
	CBitmap nodeClosed;
	nodeClosed.LoadBitmap(IDB_NODECLOSED);
	CBitmap nodeOpened;
	nodeOpened.LoadBitmap(IDB_NODEOPENED);
	m_ImageList.Add(&nodeClosed, RGB(0,0,0));
	m_ImageList.Add(&nodeOpened,RGB(0,0,0));
	m_Tree.SetImageList(&m_ImageList,TVSIL_NORMAL); 
	return 0;
}

void CProjectTreeCtrl::OnSize(UINT nType, int cx, int cy) 
{
	COleControl::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(::IsWindow(m_Tree)) m_Tree.MoveWindow(0,0,cx,cy,TRUE);
	
}


//--------------------------------------------------------------------------------------------------------
// This function will fill all project tree
//--------------------------------------------------------------------------------------------------------
void CProjectTreeCtrl::FillProjectTree() {
	m_Tree.DeleteAllItems();
	FillChilds("",0,TVI_ROOT);
	m_Tree.Expand(m_Tree.GetRootItem(),TVE_EXPAND);
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
int CProjectTreeCtrl::EvaluateSync(CString& command,CString* results) {
CString MethodName("AccessSync"); // We will run AccessSync method of the DataSource
DISPID accessID;                  // IDispatch id for the AccessSync command
HRESULT hresult;

    // if there is no data source assigned to this control, we will be unable to
    // evaluate this function.
    if(m_DataSource==NULL) return ERROR_DATA_SOURCE;

	if(results==NULL) command+=";set tmp 1";


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
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
// This recursivly called function will attach to the attach_to node all projname childs and for every 
// child it will call itself.
//---------------------------------------------------------------------------------------------------------
void CProjectTreeCtrl::FillChilds(CString varname, int index, HTREEITEM attach_to) {
CString sizeString;
static int level=0;

	if(index==0) {  // root projects requested
		EvaluateSync(CString("set projlist [roots]"));
		EvaluateSync(CString("size $projlist"),&sizeString);
		unsigned int n=atoi(sizeString);
		if(n>0) {
		   //------- Inserting item into the project tree ------------
           TV_ITEM tvItem;

	       tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

           CProjectPath path;
		   path.Name = "/";
		   path.Id   = 0;

		   POSITION at = m_FullNameList.AddTail(path);
	       tvItem.lParam         = (LPARAM)&m_FullNameList.GetAt(at);


           tvItem.pszText        = "/";
	       tvItem.cchTextMax     = 1;
	       tvItem.iImage         = 0;
	       tvItem.iSelectedImage = 1;

 	       TV_INSERTSTRUCT tvInsert;
	       tvInsert.hParent      = TVI_ROOT;
	       tvInsert.hInsertAfter = TVI_LAST;
	       tvInsert.item         = tvItem;

	       HTREEITEM root = m_Tree.InsertItem(&tvInsert);
	       m_FullNameList.GetAt(at).Id=root;
		   //---------------------------------------------------------
		   attach_to=root;
		}
	} else {              // childs projects requested
		CString serverCall;
        serverCall.Format("set projlist%u [subprojects %s %d]",attach_to,varname,index);
		EvaluateSync(serverCall);
	}

	if(index==0) {
       EvaluateSync(CString("size $projlist"),&sizeString);
	} else {
	   CString num;
	   num.Format("size $projlist%u",attach_to);
	   EvaluateSync(num,&sizeString);
	}
	unsigned int elements=atoi(sizeString);

	for(unsigned int i=1;i<=elements;i++) {
		CString nameRequest;
		CString nameReturned;
		if(index==0) {
		    nameRequest.Format("name $projlist %u",i);
		} else {
		    nameRequest.Format("name $projlist%u %u",attach_to,i);
		}
		EvaluateSync(nameRequest,&nameReturned);
	    int pos=nameReturned.ReverseFind('/');
	    pos++;
		if(nameReturned[pos]!='.') {
            CProjectPath path;
		    path.Name = nameReturned;
		    path.Id   = 0;

		    //------- Inserting item into the project tree ------------
            TV_ITEM tvItem;

	        tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

		    POSITION at = m_FullNameList.AddTail(path);
	        tvItem.lParam         = (LPARAM)&m_FullNameList.GetAt(at);

            tvItem.pszText        = ((char *)(LPCSTR)nameReturned)+pos;
	        tvItem.cchTextMax     = sizeof(nameReturned);
	        tvItem.iImage         = 0;
	        tvItem.iSelectedImage = 1;

 	        TV_INSERTSTRUCT tvInsert;
	        tvInsert.hParent      = attach_to;
	        tvInsert.hInsertAfter = TVI_LAST;
	        tvInsert.item         = tvItem;

	        HTREEITEM root = m_Tree.InsertItem(&tvInsert);
	        m_FullNameList.GetAt(at).Id=root;
		    //---------------------------------------------------------

		    // Recursive call to fill this child children
		    if(level<50) {
		       level++;
		       CString var;
		       if(index==0) {
			       var="$projlist";
			   } else {
		          var.Format("$projlist%u",attach_to);
			   }
               FillChilds(var,i,root);
		       level--;
			}
		}

	}

	// Server will remove Set for current subproject storage
	if(index==NULL) EvaluateSync(CString("unset projlist"));
	else {
		CString unset;
		unset.Format("unset projlist%u",attach_to);
		EvaluateSync(unset);
	}
}
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// This message processing function will be called every time the selected tree node changed. It will fire
// NodeChanged event with full project nams specified as parameter.
//---------------------------------------------------------------------------------------------------------
void CProjectTreeCtrl::OnSelchangedProjectTree(NMHDR* pNMHDR, LRESULT* pResult)  {
CString query;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	CProjectPath* fullPath = (CProjectPath*) pNMTreeView->itemNew.lParam;

	// We will set the current project name in the variable called DiscoverScope
	// This variable is global anc can be used by any processing mode;
	query.Format("global DiscoverScope; set DiscoverScope \"%s\"",fullPath->Name);
    EvaluateSync(query);

    FireNodeChanged(fullPath->Name);
	*pResult = 0;
}
//----------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// This message processing function will be called every time the selected tree node changed. It will fire
// NodeChanged event with full project nams specified as parameter.
//---------------------------------------------------------------------------------------------------------
void CProjectTreeCtrl::OnExpandProjectTree(NMHDR* pNMHDR, LRESULT* pResult)  {
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	CProjectPath* fullPath = (CProjectPath*) pNMTreeView->itemNew.lParam;
	if(pNMTreeView->action==TVE_EXPAND)    FireNodeExpanded(fullPath->Name);
	if(pNMTreeView->action==TVE_COLLAPSE)  FireNodeCollapsed(fullPath->Name);
	*pResult = 0;
}
//----------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// This message processing function will be called every time the tree node doublecliched. It will fire
// NodeChanged event with full project nams specified as parameter.
//---------------------------------------------------------------------------------------------------------
void CProjectTreeCtrl::OnDoubleclickProjectTree(NMHDR* pNMHDR, LRESULT* pResult)  {
	// TODO: Add your control notification handler code here
	CProjectPath* fullPath = (CProjectPath*) m_Tree.GetItemData(m_Tree.GetSelectedItem()); 
    FireNodeDoubleclicked(fullPath->Name);
	*pResult = 0;
}
//----------------------------------------------------------------------------------------------------------

BSTR CProjectTreeCtrl::GetSelectedNode()  {
	CString strResult;
	// TODO: Add your property handler here
	return m_SelectedNode.AllocSysString();
}

void CProjectTreeCtrl::SetSelectedNode(LPCTSTR lpszNewValue)  {

	if(m_FullNameList.IsEmpty()) return;

	// searching for the spesified project name
	POSITION at = m_FullNameList.GetHeadPosition(); 
	do {
		if ( m_FullNameList.GetAt(at).Name == CString(lpszNewValue) ) break;
		m_FullNameList.GetNext(at); 
	} while(at);


	// If not found, do nothing
	if(at==NULL) return;
	// Setting new selected project
	m_SelectedNode=lpszNewValue;
	// Selecting new project in the tree
	m_Tree.SelectItem(m_FullNameList.GetAt(at).Id);

}

void CProjectTreeCtrl::Query()  {
	// TODO: Add your dispatch handler code here
	Clean();
	FillProjectTree();

}

void CProjectTreeCtrl::Clean()  {
	// TODO: Add your dispatch handler code here
	m_Tree.DeleteAllItems();
	m_FullNameList.RemoveAll();
}





BOOL CProjectTreeCtrl::OnDoVerb(LONG iVerb, LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect) 
{
	// TODO: Add your specialized code here and/or call the base class
	return COleControl::OnDoVerb(iVerb, lpMsg, hWndParent, lpRect);
}
