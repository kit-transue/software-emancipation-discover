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
// ProjectModulesCtl.cpp : Implementation of the CProjectModulesCtrl OLE control class.

#include "stdafx.h"
#include "ProjectModules.h"
#include "ProjectModulesCtl.h"
#include "ProjectModulesPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CProjectModulesCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CProjectModulesCtrl, COleControl)
	//{{AFX_MSG_MAP(CProjectModulesCtrl)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_NOTIFY(LVN_ITEMCHANGED,IDC_PROJECTMODULES, OnSelectElement) 
	ON_NOTIFY(NM_DBLCLK,IDC_PROJECTMODULES,       OnExpandProject) 
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CProjectModulesCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CProjectModulesCtrl)
	DISP_PROPERTY_EX(CProjectModulesCtrl, "DataSource", GetDataSource, SetDataSource, VT_DISPATCH)
	DISP_PROPERTY_EX(CProjectModulesCtrl, "Project", GetProject, SetProject, VT_BSTR)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CProjectModulesCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CProjectModulesCtrl, COleControl)
	//{{AFX_EVENT_MAP(CProjectModulesCtrl)
	EVENT_CUSTOM("ProjectChanged", FireProjectChanged, VTS_BSTR)
	EVENT_CUSTOM("ProjectDoubleclicked", FireProjectDoubleclicked, VTS_BSTR)
	EVENT_CUSTOM("Elementchanged", FireElementchanged, VTS_BSTR)
	EVENT_CUSTOM("ElementDoubleclicked", FireElementDoubleclicked, VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CProjectModulesCtrl, 1)
	PROPPAGEID(CProjectModulesPropPage::guid)
END_PROPPAGEIDS(CProjectModulesCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CProjectModulesCtrl, "PROJECTMODULES.ProjectModulesCtrl.1",
	0x5743ffdd, 0xa20, 0x11d2, 0xae, 0xe7, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CProjectModulesCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DProjectModules =
		{ 0x5743ffdb, 0xa20, 0x11d2, { 0xae, 0xe7, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DProjectModulesEvents =
		{ 0x5743ffdc, 0xa20, 0x11d2, { 0xae, 0xe7, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwProjectModulesOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CProjectModulesCtrl, IDS_PROJECTMODULES, _dwProjectModulesOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CProjectModulesCtrl::CProjectModulesCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CProjectModulesCtrl

BOOL CProjectModulesCtrl::CProjectModulesCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_PROJECTMODULES,
			IDB_PROJECTMODULES,
			afxRegInsertable | afxRegApartmentThreading,
			_dwProjectModulesOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CProjectModulesCtrl::CProjectModulesCtrl - Constructor

CProjectModulesCtrl::CProjectModulesCtrl()
{
	InitializeIIDs(&IID_DProjectModules, &IID_DProjectModulesEvents);

	// TODO: Initialize your control's instance data here.
	m_DataSource=NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CProjectModulesCtrl::~CProjectModulesCtrl - Destructor

CProjectModulesCtrl::~CProjectModulesCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CProjectModulesCtrl::OnDraw - Drawing function

void CProjectModulesCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	/*if(::IsWindow(m_FileTable)) {
		m_FileTable.Invalidate();
	    m_FileTable.UpdateWindow();
	}*/

}


/////////////////////////////////////////////////////////////////////////////
// CProjectModulesCtrl::DoPropExchange - Persistence support

void CProjectModulesCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CProjectModulesCtrl::OnResetState - Reset control to default state

void CProjectModulesCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CProjectModulesCtrl::AboutBox - Display an "About" box to the user

void CProjectModulesCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_PROJECTMODULES);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CProjectModulesCtrl message handlers

LPDISPATCH CProjectModulesCtrl::GetDataSource()  {
	
	return m_DataSource;
}

void CProjectModulesCtrl::SetDataSource(LPDISPATCH newValue)  {

    // Control will use new data source to query model
	m_FullNameList.RemoveAll();
	m_FileTable.DeleteAllItems();
	m_DataSource=newValue;
	FillFileTable("/");
	SetModifiedFlag();
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
int CProjectModulesCtrl::EvaluateSync(CString& command,CString* results) {
CString MethodName("AccessSync"); // We will run AccessSync method of the DataSource
DISPID accessID;                  // IDispatch id for the AccessSync command
HRESULT hresult;

    // if there is no data source assigned to this control, we will be unable to
    // evaluate this function.
    if(m_DataSource==NULL) return ERROR_DATA_SOURCE;


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


void CProjectModulesCtrl::FillFileTable(const char* projname) {
unsigned register i;
CString serverCall;
CString res;
unsigned int elements;
CString sizeString;


    // Quering the directory contents
    serverCall.Format("set all [contents %s]",projname!=NULL ? projname : "");
	EvaluateSync(serverCall,&res);


	// Inserting project elements
    serverCall.Format("set projs [filter project $all]");
	EvaluateSync(serverCall,&res);
    EvaluateSync(CString("size $projs"),&sizeString);
	elements=atoi(sizeString);
	for(i=1;i<=elements;i++) {
		CString nameRequest;
		CString nameReturned;
		nameRequest.Format("name $projs %d",i);
		EvaluateSync(nameRequest,&nameReturned);
		CListEntry projEntry;
		projEntry.Name=nameReturned;
		projEntry.Type = TYPE_PROJECT;
		int index      = m_FullNameList.Add(projEntry);

		//------- Inserting item into the file list  ------------
        LV_ITEM lvItem;
	    lvItem.mask           = LVIF_TEXT | TVIF_IMAGE | LVIF_PARAM;
	    lvItem.pszText        = (char *)(LPCSTR)nameReturned;
	    lvItem.cchTextMax     = strlen(nameReturned);
	    lvItem.iImage         = 0;
	    lvItem.iItem          = i-1;
	    lvItem.iSubItem       = 0;
	    lvItem.lParam         = index;
		m_FileTable.InsertItem(&lvItem);
		m_FileTable.SetItemText(i-1,2,"File Folder");

	   //---------------------------------------------------------

	}
	int last=elements;
	EvaluateSync(CString("unset projs"));

    // Creating file elements
    serverCall.Format("set files [filter module $all]");
	EvaluateSync(serverCall,&res);
    EvaluateSync(CString("size $files"),&sizeString);
	elements=atoi(sizeString);

	for(i=1;i<=elements;i++) {
		CString nameRequest;
		CString nameReturned;
		nameRequest.Format("name $files %d",i);
		EvaluateSync(nameRequest,&nameReturned);
		CListEntry projEntry;
		projEntry.Name=nameReturned;
		projEntry.Type = TYPE_C;
		int index      = m_FullNameList.Add(projEntry);
		//------- Inserting item into the file list  ------------
        LV_ITEM lvItem;
	    lvItem.mask           = LVIF_TEXT | TVIF_IMAGE | LVIF_PARAM;
		int slashPos=nameReturned.ReverseFind('/');
		if(slashPos==-1)
	       lvItem.pszText        = (char *)(LPCSTR)nameReturned;
	    else
	       lvItem.pszText        = (char *)((LPCSTR)nameReturned+slashPos+1);
	    lvItem.cchTextMax     = strlen(nameReturned);

		int typeIndex=5;
		int dotPos=nameReturned.ReverseFind('.');
		if(dotPos!=-1)  {
			CString extention = (char *)nameReturned.GetBuffer(strlen(nameReturned))+dotPos+1;
			extention.MakeUpper();
			if(extention=="C")                       typeIndex=1;
			if(extention=="CPP" || extention=="CPP") typeIndex=2;
			if(extention=="H")                       typeIndex=3;
			if(extention=="DOC")                     typeIndex=4;
		}
	    lvItem.iImage         = typeIndex;
	    lvItem.iItem          = last+i-1;
	    lvItem.iSubItem       = 0;
	    lvItem.lParam         = index;
		m_FileTable.InsertItem(&lvItem);
		switch(typeIndex) {
		   case 0 : m_FileTable.SetItemText(last+i-1,2,"File Folder");   break;
		   case 1 : m_FileTable.SetItemText(last+i-1,2,"C Source");      break;
		   case 2 : m_FileTable.SetItemText(last+i-1,2,"C++ Source");    break;
		   case 3 : m_FileTable.SetItemText(last+i-1,2,"C Header");      break;
		   case 4 : m_FileTable.SetItemText(last+i-1,2,"Word Document"); break;
		   case 5 : 
		   default: m_FileTable.SetItemText(last+i-1,2,"Unknown");       break;

		}
	   //---------------------------------------------------------

	}
	EvaluateSync(CString("unset files"));
    EvaluateSync(CString("unset all"));
}

void CProjectModulesCtrl::OnSize(UINT nType, int cx, int cy) 
{
	COleControl::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(::IsWindow(m_FileTable)) m_FileTable.MoveWindow(0,0,cx,cy,TRUE);
	
}

int CProjectModulesCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (COleControl::OnCreate(lpCreateStruct) == -1) 
		return -1;
	
	// TODO: Add your specialized creation code here
	m_FileTable.Create( LVS_SHAREIMAGELISTS | LVS_REPORT | WS_CHILD |WS_VISIBLE,
		           CRect(lpCreateStruct->x, 
		                 lpCreateStruct->y,
			             lpCreateStruct->x+lpCreateStruct->cx,
					     lpCreateStruct->y+lpCreateStruct->cy),
			       this,
				   IDC_PROJECTMODULES);
	ModifyStyleEx(0,WS_EX_CLIENTEDGE);
	// Creating our image list and connecting it to the tree control
	m_ImageList.Create(16,16,FALSE,2,0);
	CBitmap project;
	project.LoadBitmap(IDB_PROJECT);
	CBitmap cmodule;
	cmodule.LoadBitmap(IDB_C);
	CBitmap cppmodule;
	cppmodule.LoadBitmap(IDB_CPLUSPLUS);
	CBitmap chmodule;
	chmodule.LoadBitmap(IDB_CHEADER);
	CBitmap word;
	word.LoadBitmap(IDB_WORDDOC);
	CBitmap unknown;
	unknown.LoadBitmap(IDB_UNKNOWN);


	m_ImageList.Add(&project, RGB(0,0,0));
	m_ImageList.Add(&cmodule,RGB(0,0,0));
	m_ImageList.Add(&cppmodule,RGB(0,0,0));
	m_ImageList.Add(&chmodule,RGB(0,0,0));
	m_ImageList.Add(&word,RGB(0,0,0));
	m_ImageList.Add(&unknown,RGB(0,0,0));
    m_FileTable.SetImageList(&m_ImageList,LVSIL_SMALL);


    LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt=LVCFMT_CENTER;

	lvColumn.cx=70;
	lvColumn.iSubItem=4;
	lvColumn.pszText="Attribute";
    m_FileTable.InsertColumn(0,&lvColumn);

	lvColumn.cx=80;
	lvColumn.iSubItem=3;
	lvColumn.pszText="Modified";
    m_FileTable.InsertColumn(0,&lvColumn);

	lvColumn.cx=70;
	lvColumn.iSubItem=2;
	lvColumn.pszText="Type";
    m_FileTable.InsertColumn(0,&lvColumn);

	lvColumn.cx=50;
	lvColumn.iSubItem=1;
	lvColumn.pszText="Size";
    m_FileTable.InsertColumn(0,&lvColumn);

	lvColumn.cx=120;
	lvColumn.iSubItem=0;
	lvColumn.pszText="Name";
    m_FileTable.InsertColumn(0,&lvColumn);
	return 0;
}

BSTR CProjectModulesCtrl::GetProject()  {
	return m_ProjectName.AllocSysString();
}

void CProjectModulesCtrl::SetProject(LPCTSTR lpszNewValue)  {
	// TODO: Add your property handler here
	if(m_ProjectName == CString(lpszNewValue)) return;
	m_ProjectName = lpszNewValue;
	ClearList();
	FillFileTable(lpszNewValue);
	SetModifiedFlag();
}

void CProjectModulesCtrl::ClearList(void)
{
   m_FileTable.DeleteAllItems();
}

void CProjectModulesCtrl::OnSelectElement(NMHDR* pNMHDR, LRESULT* pResult) {
NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// TODO: Add your control notification handler code here
	int index = pNMListView->lParam;
	m_Element.Name = m_FullNameList[index].Name;
	m_Element.Type = m_FullNameList[index].Type;
	if(m_Element.Type==TYPE_PROJECT) FireProjectChanged(m_FullNameList[index].Name);
	else                             FireElementChanged(m_FullNameList[index].Name);
	*pResult  = 0;

}


void CProjectModulesCtrl::OnExpandProject(NMHDR* pNMHDR, LRESULT* pResult) {

	// TODO: Add your control notification handler code here
	if(m_Element.Type==TYPE_PROJECT) SetProject(m_Element.Name);
	if(m_Element.Type==TYPE_PROJECT) FireProjectDoubleclicked(m_Element.Name);
	else                             FireElementDoubleclicked(m_Element.Name);
	*pResult = 0;

}

BOOL CProjectModulesCtrl::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class

	
	return COleControl::PreCreateWindow(cs);
}
