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
// ImpactCtl.cpp : Implementation of the CImpactCtrl ActiveX Control class.

#include "stdafx.h"
#include "Impact.h"
#include "ImpactCtl.h"
#include "ImpactPpg.h"
#include "Entity.h"
#include "ImpactItem.h"

#include "ClassActions.h"
#include "InterfaceActions.h"
#include "UnionActions.h"
#include "EnumActions.h"
#include "FieldActions.h"
#include "FunctionActions.h"
#include "TypedefActions.h"
#include "MacroActions.h"
#include "TemplateActions.h"
#include "VariableActions.h"
#include "ModuleActions.h"
#include "PackageActions.h"
#include "LocalVariableActions.h"

#include "custommsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CImpactCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CImpactCtrl, COleControl)
        //{{AFX_MSG_MAP(CImpactCtrl)
        ON_WM_CREATE()
        ON_WM_SIZE()
		ON_MESSAGE( WM_TREEITEMSTATECHANGED, OnItemStateChanged )
		ON_MESSAGE( WM_OPENINSTANCE, OnOpenInstance )
        //}}AFX_MSG_MAP
        ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CImpactCtrl, COleControl)
    //{{AFX_DISPATCH_MAP(CImpactCtrl)
	DISP_PROPERTY_EX(CImpactCtrl, "DataSource", GetDataSource, SetDataSource, VT_DISPATCH)
	DISP_PROPERTY_EX(CImpactCtrl, "EditorCtrl", GetEditorCtrl, SetEditorCtrl, VT_DISPATCH)
	DISP_FUNCTION(CImpactCtrl, "Update", Update, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CImpactCtrl, "SaveReportTo", SaveReportTo, VT_BOOL, VTS_BSTR)
	DISP_FUNCTION(CImpactCtrl, "IsReportView", IsReportView, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CImpactCtrl, "GetReportName", GetReportName, VT_BSTR, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CImpactCtrl, COleControl)
        //{{AFX_EVENT_MAP(CImpactCtrl)
        // NOTE - ClassWizard will add and remove event map entries
        //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CImpactCtrl, 1)
        PROPPAGEID(CImpactPropPage::guid)
END_PROPPAGEIDS(CImpactCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CImpactCtrl, "IMPACT.ImpactCtrl.1",
        0x2047b32d, 0x6d1b, 0x4538, 0x9a, 0xf5, 0x41, 0xf7, 0xb8, 0xd0, 0xbd, 0xbe)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CImpactCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DImpact =
                { 0xbc2bd6a2, 0x420b, 0x4721, { 0x9e, 0xed, 0xd5, 0xad, 0x9c, 0xff, 0xd8, 0x85 } };
const IID BASED_CODE IID_DImpactEvents =
                { 0xd12f7a11, 0x987, 0x43fd, { 0x8a, 0x8b, 0xfb, 0x2a, 0x6b, 0xc1, 0x78, 0xcd } };

        
const int CImpactCtrl::IconsIdxs[] = {IDB_FILES,IDB_FUNCTIONS,IDB_VARIABLES,IDB_CLASSES,
                        IDB_STRUCTURES,IDB_UNIONS,IDB_ENUMS,IDB_TYPEDEFS,IDB_MACROS,IDB_TEMPLATES,
                        IDB_STRINGS,IDB_LOCALVARS,IDB_FIELD,IDB_SUBSYSTEMS,IDB_INSTANCES,IDB_AST,
                        IDB_PROJECTS,IDB_DEFECT,IDB_CONST,IDB_SEMTYPE,IDB_EVAL,IDB_LABEL,IDB_TABLE,
                        IDB_CURSOR,IDB_XREF,IDB_ACTION,IDB_IMPACTPROJECT,IDB_CHANGES,IDB_PACKAGES,
						IDB_INTERFACES,IDB_WARNING};

/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwImpactOleMisc =
        OLEMISC_SIMPLEFRAME |
        OLEMISC_ACTIVATEWHENVISIBLE |
        OLEMISC_SETCLIENTSITEFIRST |
        OLEMISC_INSIDEOUT |
        OLEMISC_CANTLINKINSIDE |
        OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CImpactCtrl, IDS_IMPACT, _dwImpactOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CImpactCtrl::CImpactCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CImpactCtrl

BOOL CImpactCtrl::CImpactCtrlFactory::UpdateRegistry(BOOL bRegister)
{
        // TODO: Verify that your control follows apartment-model threading rules.
        // Refer to MFC TechNote 64 for more information.
        // If your control does not conform to the apartment-model rules, then
        // you must modify the code below, changing the 6th parameter from
        // afxRegApartmentThreading to 0.

        if (bRegister)
                return AfxOleRegisterControlClass(
                        AfxGetInstanceHandle(),
                        m_clsid,
                        m_lpszProgID,
                        IDS_IMPACT,
                        IDB_IMPACT,
                        afxRegApartmentThreading,
                        _dwImpactOleMisc,
                        _tlid,
                        _wVerMajor,
                        _wVerMinor);
        else
                return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CImpactCtrl::CImpactCtrl - Constructor

CImpactCtrl::CImpactCtrl():m_cszError("Error:")
{
    InitializeIIDs(&IID_DImpact, &IID_DImpactEvents);

    EnableSimpleFrame();

    // TODO: Initialize your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CImpactCtrl::~CImpactCtrl - Destructor

CImpactCtrl::~CImpactCtrl()
{
}


/////////////////////////////////////////////////////////////////////////////
// CImpactCtrl::OnDraw - Drawing function

void CImpactCtrl::OnDraw(
                        CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
    if(m_ImpactTree.GetSafeHwnd()!=NULL) {
            m_ImpactTree.Invalidate();
    }
}


/////////////////////////////////////////////////////////////////////////////
// CImpactCtrl::DoPropExchange - Persistence support

void CImpactCtrl::DoPropExchange(CPropExchange* pPX)
{
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);

    // TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CImpactCtrl::OnResetState - Reset control to default state

void CImpactCtrl::OnResetState()
{
    COleControl::OnResetState();  // Resets defaults found in DoPropExchange

    // TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CImpactCtrl message handlers

int CImpactCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (COleControl::OnCreate(lpCreateStruct) == -1)
        return -1;
    
	// create impact tabs
	m_wndPages.Create(WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | TCS_FOCUSNEVER,
						CRect(lpCreateStruct->x,
                             lpCreateStruct->y,
                             lpCreateStruct->x+lpCreateStruct->cx,
                             lpCreateStruct->y+lpCreateStruct->cy),
						this,0);

	// initialize impact report component
    m_wndReport.Create(WS_CHILD,
						CRect(lpCreateStruct->x,
                             lpCreateStruct->y,
                             lpCreateStruct->x+lpCreateStruct->cx,
                             lpCreateStruct->y+lpCreateStruct->cy),
							&m_wndPages,
							IDC_IMPACTREPORT);
	// initialize impact tree component
    m_ImpactTree.Create(WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_LINESATROOT | TVS_DISABLEDRAGDROP,
                       CRect(lpCreateStruct->x,
                             lpCreateStruct->y,
                             lpCreateStruct->x+lpCreateStruct->cx,
                             lpCreateStruct->y+lpCreateStruct->cy),
							&m_wndPages,
							IDC_IMPACTTREE);     
	m_wndReport.SetImpactTree(&m_ImpactTree);

    m_TypeIcons.Create(16,16,TRUE,1,0);
    int nSize = sizeof(IconsIdxs)/sizeof(int);
    CBitmap icon;
	// load icons
    for(int i=0;i<nSize;i++) {
        icon.LoadBitmap(IconsIdxs[i]);
        m_TypeIcons.Add(&icon, RGB(255,255,255));
        icon.Detach();
    }
	// set icons
    m_ImpactTree.SetImageList(&m_TypeIcons,TVSIL_NORMAL);

	// loading pages icons
    m_PagesIcons.Create(16,16,TRUE,1,0);
    icon.LoadBitmap(IDB_IMPACTPROJECT);
    m_PagesIcons.Add(&icon, RGB(255,255,255));
    icon.Detach();
    icon.LoadBitmap(IDB_IMPACTREPORT);
    m_PagesIcons.Add(&icon, RGB(255,255,255));
    icon.Detach();
	// set pages images
	m_wndPages.SetImageList(&m_PagesIcons);
	
	// Add Impact tree and report tabs
	CString szTabName;
	szTabName.LoadString(IDS_IMPACTPROJECT);
    m_wndPages.AddPage(&m_ImpactTree,szTabName,0);
	szTabName.LoadString(IDS_IMPACTREPORT);
    m_wndPages.AddPage(&m_wndReport, szTabName,1);

	// show project screen first
	m_wndPages.SetCurSel(0);
	return 0;
}

void CImpactCtrl::OnSize(UINT nType, int cx, int cy) 
{
    COleControl::OnSize(nType, cx, cy);
    
    if(m_wndPages.GetSafeHwnd()!=NULL) m_wndPages.MoveWindow(0,0,cx,cy,true);
}

LPDISPATCH CImpactCtrl::GetDataSource() 
{
    return m_DataSource;
}

void CImpactCtrl::SetDataSource(LPDISPATCH newValue) 
{
    m_DataSource=newValue;
}


CString CImpactCtrl::ExecPrint(CString &command) 
{
    CString MethodName("AccessDish"); // We will run AccessSync method of the DataSource
    CString ServerStatus("IsServerBusy"); 
    DISPID accessID;                  // IDispatch id for the AccessSync command
    HRESULT hresult;
    CString cmd = command;

    // if there is no data source assigned to this control, we will be unable to
    // evaluate this function.
    if(m_DataSource==NULL) return m_cszError + CString(" No data source - can't evaluate a command");


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
    if (busyBOOL.intVal!=0) return m_cszError + CString(" server is busy.");
	::SysFreeString(statusBSTR);

    // Trying to receve method ID from it's name.
    BSTR methodBSTR=MethodName.AllocSysString();
    hresult = m_DataSource->GetIDsOfNames(IID_NULL,&methodBSTR, 
                                          1,LOCALE_SYSTEM_DEFAULT,&accessID);

    // Interface doesn't support AccessSync method
    if(hresult!=S_OK) return m_cszError + CString(" data source does not support AccessSync method");
	::SysFreeString(methodBSTR);

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

    CString szResult;
    if(hresult!=S_OK) szResult = m_cszError + " Command '" + command + "' failed.\n";
    szResult += resBSTR.bstrVal;
	
	int nLen = szResult.GetLength()-2; // remove % sign at the end
	szResult = szResult.Left(nLen);
    return szResult;
}

//---------------------------------------------------------------------------
// Executes Access command and returns a result:
//   If a result starts with 'Error:' - it means some error has occurred and 
//              rest of the strind is the error description
//   Otherwise its the command result
//---------------------------------------------------------------------------
CString CImpactCtrl::ExecCommand(CString &command)
{
    CString MethodName("AccessSync"); // We will run AccessSync method of the DataSource
    DISPID accessID;                  // IDispatch id for the AccessSync command
    HRESULT hresult;

    // if there is no data source assigned to this control, we will be unable to
    // evaluate this function.
	if(m_DataSource==NULL) return m_cszError + CString(" No data source - can't evaluate a command");

    // Trying to receve method ID from it's name.
    BSTR methodBSTR=MethodName.AllocSysString();
    hresult = m_DataSource->GetIDsOfNames(IID_NULL,&methodBSTR, 
                                              1,LOCALE_SYSTEM_DEFAULT,&accessID);

    // Interface doesn't support AccessSync method
    if(hresult!=S_OK) return m_cszError + CString(" data source does not support AccessSync method");
	::SysFreeString(methodBSTR);

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
	::SysFreeString(access_call);

    CString szResult;
    if(hresult!=S_OK) szResult = m_cszError + " Command '" + command + "' failed.\n";
    szResult += resBSTR.bstrVal;
    return szResult;
}

BOOL CImpactCtrl::Update() 
{
	CString result;
	CString query;
	int selectionSize;

	Clean();

	query = "set ImpactSelection $DiscoverSelection";
	result=ExecCommand(query);
	if(result.Left(m_cszError.GetLength()).CompareNoCase(m_cszError)==0)    
		return FALSE;

	// Calculationg selection size
	query = "size $ImpactSelection";
	result=ExecCommand(query);
	if(result.Left(m_cszError.GetLength()).CompareNoCase(m_cszError)==0)    
		return FALSE;
    
	selectionSize = atoi(result);
	if(selectionSize > 0) {
		query = "printformat \"%s\t%s\t%s\t%s\t%s\t%s\t%s\" name kind language etag private protected cname;print $ImpactSelection";
		result=ExecPrint(query);
		if(result.Left(m_cszError.GetLength()).CompareNoCase(m_cszError)==0)    
			return FALSE;

		CString szRootName;
		szRootName.LoadString(IDS_IMPACTROOTNAME);
		int nRootIconIdx = GetIconIdx(IDB_IMPACTPROJECT);
		HTREEITEM hRoot = m_ImpactTree.InsertItem(szRootName,nRootIconIdx,nRootIconIdx);

		TVINSERTSTRUCT tvInsert;
		tvInsert.hParent = hRoot;
		tvInsert.hInsertAfter = NULL;
		tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

		int nStartDelim=0,
			nEndDelim;
		while((nEndDelim=result.Find(_T('\n'),nStartDelim))!=-1) {
			CString szAttrs = result.Mid(nStartDelim,nEndDelim-nStartDelim);
			CEntity* pEntity = new CEntity(szAttrs,_T('\t'));
			// if current language is ELS - give it another try ask a language using another command
			if(!pEntity->GetLanguage().CompareNoCase("ELS")) {
				CString szEtag = pEntity->GetEtag();
				if(szEtag.Find(" ")>-1)	szEtag = "{" + szEtag + "}";
				CString queryLanguage = "language [ etag " + szEtag + " ]";
				pEntity->SetLanguage(ExecCommand(queryLanguage));
			}
			CImpactActionsSet* pSet = CreateActions(pEntity);
			if(pSet!=NULL) {  // if we have impact queries for such an entity
				CImpactItem* pImpactItem = new CImpactItem(pEntity,pSet);

				int nIconIdx = GetIconIdx(pEntity->GetKind());

				tvInsert.item.pszText = (char*)(LPCTSTR)pEntity->GetName();
				tvInsert.item.iImage = nIconIdx;
				tvInsert.item.iSelectedImage = nIconIdx;
				tvInsert.item.lParam = (LPARAM)pImpactItem;

				HTREEITEM hEntityRoot = m_ImpactTree.InsertItem(&tvInsert);
				AddActions(hEntityRoot,pImpactItem);
			} else {
				delete pEntity;
			}

			nStartDelim = nEndDelim+1;
		}
		m_ImpactTree.Expand(hRoot,TVE_EXPAND);
	}

	query = "unset ImpactSelection";
	result=ExecCommand(query);
	return result.Left(m_cszError.GetLength()).CompareNoCase(m_cszError)!=0;
}

int CImpactCtrl::GetIconIdx(CString kind)
{
	kind.MakeLower();
	int nType = 0;
	if(kind=="module")      nType = IDB_FILES;
	else if(kind=="funct")       nType = IDB_FUNCTIONS;
	else if(kind=="var")         nType = IDB_VARIABLES;
	else if(kind=="struct")      nType = IDB_CLASSES;
	else if(kind=="union")       nType = IDB_UNIONS;
	else if(kind=="enum")        nType = IDB_ENUMS;
	else if(kind=="typedef")     nType = IDB_TYPEDEFS;
	else if(kind=="macro")       nType = IDB_MACROS;
	else if(kind=="templ")       nType = IDB_TEMPLATES;
	else if(kind=="string")      nType = IDB_STRINGS;
	else if(kind=="local")       nType = IDB_LOCALVARS;
	else if(kind=="field")       nType = IDB_FIELD;
	else if(kind=="group")       nType = IDB_SUBSYSTEMS;
	else if(kind=="instance")    nType = IDB_INSTANCES;
	else if(kind=="ast")         nType = IDB_AST;
	else if(kind=="proj")        nType = IDB_PROJECTS;
	else if(kind=="defect")      nType = IDB_DEFECT;
	else if(kind=="constant")    nType = IDB_CONST;
	else if(kind=="semtype")     nType = IDB_SEMTYPE;
	else if(kind=="evalue")      nType = IDB_EVAL;
	else if(kind=="label")       nType = IDB_LABEL;
	else if(kind=="table")       nType = IDB_TABLE;
	else if(kind=="cursor")      nType = IDB_CURSOR;
	else if(kind=="externref")   nType = IDB_XREF;
	else if(kind=="package")     nType = IDB_PACKAGES;
	else if(kind=="interface")   nType = IDB_INTERFACES;
	else if(kind=="warning")	 nType = IDB_WARNING;
/* -- Debug Code
	else
		MessageBox(kind,"Warning",MB_OK|MB_ICONWARNING);
// */
	return GetIconIdx(nType);
}

void CImpactCtrl::Clean()
{
	HTREEITEM hRoot = m_ImpactTree.GetRootItem();
	if(m_ImpactTree.GetSafeHwnd()!=NULL && hRoot != NULL) {
		HTREEITEM hItem = m_ImpactTree.GetChildItem(hRoot);
		while(hItem!=NULL) {
			CImpactItem* pItem = (CImpactItem*)m_ImpactTree.GetItemData(hItem);
			delete pItem;
			hItem = m_ImpactTree.GetNextSiblingItem(hItem);
		}
		m_ImpactTree.DeleteAllItems();
	}
}

BOOL CImpactCtrl::DestroyWindow() 
{
	Clean();
	return COleControl::DestroyWindow();
}

CImpactActionsSet* CImpactCtrl::CreateActions(CEntity* pEntity)
{
	CString kind = pEntity->GetKind();
	kind.MakeLower();
	bool isMemberOfClass = false;

	// Check to see if the entity is static
	if(kind == "var") { 
		CString id = CImpactAction::PrepareQuery(pEntity->GetEtag());
		CString result = ExecCommand("oo_member " + id);
		if (result == "1") { 
			isMemberOfClass = true;
		} else {
			isMemberOfClass = false;
		}
	}

	if(kind=="module") {  // module
		// HACK, needed to change the constructor in order to have an Entity
		// so that we can do a language check in ModuleActions.  
		// all files in the change package 21019:6 should eventually be 
		// changed back after the language attribute is fixed
		return new CModuleActions(pEntity->GetLanguage(), pEntity, this);
	} else if(kind=="struct" && 
		(pEntity->GetLanguage().CompareNoCase("CPP")==0  || 
		 pEntity->GetLanguage().CompareNoCase("JAVA")==0 ||
		 pEntity->GetLanguage().CompareNoCase("C")==0 )) { // class or struct
		return new CClassActions(pEntity->GetLanguage(), this);
	} else if(kind=="interface") { // interface
		return new CInterfaceActions(pEntity->GetLanguage(), this);
	} else if(kind=="union") {
		return new CUnionActions(pEntity->GetLanguage(), this);
	} else if(kind=="enum") {
		return new CEnumActions(pEntity->GetLanguage(), this);
	} else if(kind=="field") {
		return new CFieldActions(pEntity->GetLanguage(), this);
	} else if(kind=="funct") {
		return new CFunctionActions(pEntity->GetLanguage(), this);
	} else if(kind=="typedef") {
		return new CTypedefActions(pEntity->GetLanguage(), this);
	} else if(kind=="macro") {
		return new CMacroActions(pEntity->GetLanguage(), this);
	} else if(kind=="templ") {
		return new CTemplateActions(pEntity->GetLanguage(), this);
	} else if(kind=="var") {
		return new CVariableActions(pEntity->GetLanguage(), this, isMemberOfClass);
	} else if(kind=="package") {
		return new CPackageActions(pEntity->GetLanguage(), this);
	} else if(kind=="local") {
		return new CLocalVariableActions(pEntity->GetLanguage(), this);
	}
	return NULL;// impact for this entity is not supported
}

void CImpactCtrl::AddActions(HTREEITEM item,CImpactItem* pItem)
{
	if(pItem!=NULL) {
		CImpactActionsSet* pSet = pItem->GetActions();
		int nSize = pSet->GetActions().GetCount();
		
		for(int i=0;i<nSize;i++) {
			CImpactAction* pAction = pSet->GetAction(i);
			AddAction(item,NULL,pAction);
		}

		CRect rcRect;
		m_ImpactTree.GetClientRect(&rcRect);
		m_ImpactTree.InvalidateRect(rcRect) ;
		m_ImpactTree.UpdateWindow();
	}
}

int CImpactCtrl::GetIconIdx(int iconID)
{
	int nSize = sizeof(IconsIdxs)/sizeof(int);
	for(int i=0;i<nSize && IconsIdxs[i]!=iconID;i++);
	return (i==nSize?-1:i);
}

LRESULT CImpactCtrl::OnOpenInstance(WPARAM wparm, LPARAM lparm) {
	CResultEntity* pEntity = (CResultEntity*)lparm;
	if(pEntity!=NULL) {
		CString* fName = pEntity->getFile();
		int		 nLine = pEntity->getLine();

		CString MethodName("OpenFile"); // We will run OpenFile method of the Editor integrator
		DISPID accessID;                // IDispatch id for the AccessSync command
		HRESULT hresult;

		// if there is no editor integrator assigned to this control, we will be unable to
		// evaluate this function.
		if(m_EditorCtrl==NULL) return 0;

		// Trying to receve method ID from it's name.
		BSTR methodBSTR=MethodName.AllocSysString();
		hresult = m_EditorCtrl->GetIDsOfNames(IID_NULL,&methodBSTR, 
                                              1,LOCALE_SYSTEM_DEFAULT,&accessID);

		// Interface doesn't support OpenFile method
		if(hresult!=S_OK) return 0;
		::SysFreeString(methodBSTR);

		// Converting command into new OEM string wich will be passed into OLE automation.
		// We will free when pushing into the stack
		BSTR file_name = fName->AllocSysString();

		// Creating parameter structure - only one parameter will be passed as 
		// argument - command string
		DISPPARAMS cmdParamStruct; 
		cmdParamStruct.rgvarg              = new VARIANT[2];
		cmdParamStruct.rgvarg[1].vt        = VT_BSTR;
		cmdParamStruct.rgvarg[1].bstrVal   = file_name;
		cmdParamStruct.rgvarg[0].vt        = VT_I4;			
		cmdParamStruct.rgvarg[0].lVal	   = nLine;
		cmdParamStruct.cArgs               = 2;
		cmdParamStruct.cNamedArgs		   = 0;

		// This will show the argument with an error
		unsigned int argWithError;
		// Remote command execution
		hresult = m_EditorCtrl->Invoke(accessID,
                                   IID_NULL,
                                   LOCALE_SYSTEM_DEFAULT,
                                   DISPATCH_METHOD,
                                   &cmdParamStruct,
                                   NULL,
                                   NULL,
                                   &argWithError);

		delete[] cmdParamStruct.rgvarg;
		::SysFreeString(file_name);
	}
	return 0;
}

LRESULT CImpactCtrl::OnItemStateChanged(WPARAM wparm, LPARAM lparm) 
{
	HTREEITEM hItem = (HTREEITEM)lparm;
	HTREEITEM hParent = m_ImpactTree.GetParentItem(hItem);
	CImpactAction* pAction = (CImpactAction*)m_ImpactTree.GetItemData(hItem);
	if(pAction != NULL) {
		if(!pAction->IsExecuted()) {  // if action has not been executed yet - run it
			CImpactItem* pItem = (CImpactItem*)m_ImpactTree.GetItemData(m_ImpactTree.GetParentItem(hItem));
			if(pAction->ActionPerformed(pItem->GetEntity())) {
				CResultsTree* pResults = pAction->GetResults();
				if(pResults!=NULL) {
					DisplayActionResults(hItem,pResults->children());
					// if we can repeat the action - add it to the tree again
					if(pAction->DoRepeat()) { 
						CImpactItem* pImpactItem = (CImpactItem*)m_ImpactTree.GetItemData(hParent);
						CImpactActionsSet* pSet = pImpactItem->GetActions();
						TActionsList& actionsList = pSet->GetActions();
						
						// duplicate the action
						CObject* pObject = pAction->GetRuntimeClass()->CreateObject();
						ASSERT( pObject->IsKindOf(pAction->GetRuntimeClass()) );
						CImpactAction* pNewAction = (CImpactAction*)pObject;
						pNewAction->SetCtrl(pAction->GetCtrl());
						if(pAction->HasHash()) pNewAction->SetHash(pAction->GetHash());
						pAction->SetDuplicated(true);
						
						actionsList.InsertAfter(actionsList.Find(pAction),pNewAction);
						AddAction(hParent,hItem,pNewAction);
					}
					m_ImpactTree.Expand(hItem,TVE_EXPAND);
				}
			} else {
				m_ImpactTree.SetItemState(hItem, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
			}
		} else { // otherwise - clean all results
			DeleteResults(m_ImpactTree.GetChildItem(hItem));
			pAction->SetExecuted(false);
			// if it's a duplicate - remove it
			if(pAction->IsDuplicated()) {
				CImpactItem* pImpactItem = (CImpactItem*)m_ImpactTree.GetItemData(hParent);
				CImpactActionsSet* pSet = pImpactItem->GetActions();
				TActionsList& actionsList = pSet->GetActions();				
				actionsList.RemoveAt(actionsList.Find(pAction));
				m_ImpactTree.DeleteItem(hItem);
				pAction->Undo();
				delete pAction;
			}
		}
	}
	CRect rcRect;
	m_ImpactTree.GetItemRect(hItem,rcRect,FALSE);
	m_ImpactTree.InvalidateRect(rcRect) ;
	m_ImpactTree.UpdateWindow();

	return 0;
}


void CImpactCtrl::DisplayActionResults(HTREEITEM hRoot, CResultsTree *pRoot)
{
	while(pRoot != NULL) {
		CResultEntity* pEntity = pRoot->getEntity();
		//MessageBox("Breakpoint","Warning",MB_OK|MB_ICONWARNING);
		int nIconIdx = GetIconIdx(*pEntity->getKind());
		if(nIconIdx<0) nIconIdx = GetIconIdx(*pEntity->getName());
		if(nIconIdx<0) nIconIdx = GetIconIdx(IDB_CHANGES);
		HTREEITEM hItem = m_ImpactTree.InsertItem(*pEntity->getMessage(),nIconIdx,nIconIdx,hRoot);
		m_ImpactTree.SetItem(hItem, TVIF_PARAM, NULL, 0, 0, 0, 0, (LPARAM)pEntity);
		DisplayActionResults(hItem,pRoot->children());
		pRoot=pRoot->getNext();
	}
}

void CImpactCtrl::DeleteResults(HTREEITEM hRoot)
{
	while(hRoot != NULL) {
		DeleteResults(m_ImpactTree.GetChildItem(hRoot));
		HTREEITEM hItem = m_ImpactTree.GetNextSiblingItem(hRoot);
		m_ImpactTree.DeleteItem(hRoot);
		hRoot = hItem;
	}
}

BOOL CImpactCtrl::AddAction(HTREEITEM parent, HTREEITEM after, CImpactAction *pAction)
{
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = parent;
	tvInsert.hInsertAfter = after;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	tvInsert.item.pszText = (char*)(LPCTSTR)pAction->GetName();
	tvInsert.item.lParam = (LPARAM)pAction;

	int nIcon = GetIconIdx(IDB_ACTION);
	tvInsert.item.iImage = nIcon;
	tvInsert.item.iSelectedImage = nIcon;

	return m_ImpactTree.SetItemState(m_ImpactTree.InsertItem(&tvInsert), INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
}

BOOL CImpactCtrl::SaveReportTo(LPCTSTR szFileName) 
{
	m_wndReport.SaveTo(CString(szFileName));
	return TRUE;
}

BOOL CImpactCtrl::IsReportView() 
{
	return 	m_wndPages.GetCurSel()==1;
}

BSTR CImpactCtrl::GetReportName() 
{
	CString strResult=m_wndReport.GetGeneratedReportName();
	return strResult.AllocSysString();
}


LPDISPATCH CImpactCtrl::GetEditorCtrl() 
{
	return m_EditorCtrl;
}

void CImpactCtrl::SetEditorCtrl(LPDISPATCH newValue) 
{
    m_EditorCtrl=newValue;
}
