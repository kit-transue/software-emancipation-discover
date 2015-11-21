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
// MiniBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "MiniBrowser.h"
#include "CntrItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define HEADER 3
#define BORDER 5
#define SASH_SPACE 0
#define SPLITTER_SPACE 10
#define SELECTOR_HEIGHT 26

/////////////////////////////////////////////////////////////////////////////
// CMiniBrowser

IMPLEMENT_DYNCREATE(CMiniBrowser, CFormView)

CMiniBrowser::CMiniBrowser()
	: CFormView(CMiniBrowser::IDD) {
	EnableAutomation();
	//{{AFX_DATA_INIT(CMiniBrowser)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CMiniBrowser::~CMiniBrowser() {

}

void CMiniBrowser::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CFormView::OnFinalRelease();
}

void CMiniBrowser::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMiniBrowser)
	DDX_Control(pDX, IDC_MODESELECTOR, m_ModeSelector);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	DDX_Control(pDX, IDC_LEFTPANE, m_Left);
	DDX_Control(pDX, IDC_RIGHTPANE, m_Right);
	DDX_Control(pDX, IDC_PROJECTTREE, m_Tree);
	DDX_Control(pDX, IDC_PROJECTMODULES, m_Modules);
	DDX_Control(pDX, IDC_MSDEVINTEGRATOR, m_MSDEVIntegrator);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMiniBrowser, CFormView)
	//{{AFX_MSG_MAP(CMiniBrowser)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_OLE_INSERT_NEW, OnInsertObject)
	ON_COMMAND(ID_CANCEL_EDIT_CNTR, OnCancelEditCntr)
	ON_COMMAND(ID_CANCEL_EDIT_SRVR, OnCancelEditSrvr)
	ON_WM_CREATE()
	ON_MESSAGE(WM_SPLITMOVE,OnSplitMoved)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MODESELECTOR, OnModeChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMiniBrowser, CFormView)
	//{{AFX_DISPATCH_MAP(CMiniBrowser)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IMiniBrowser to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {220505F2-593F-11D2-AF31-00A0C9B71DC4}
static const IID IID_IMiniBrowser =
{ 0x220505f2, 0x593f, 0x11d2, { 0xaf, 0x31, 0x0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };

BEGIN_INTERFACE_MAP(CMiniBrowser, CFormView)
	INTERFACE_PART(CMiniBrowser, IID_IMiniBrowser, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiniBrowser diagnostics


/////////////////////////////////////////////////////////////////////////////
// CMiniBrowser message handlers

void CMiniBrowser::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// TODO: remove this code when final selection model code is written
	m_pSelection = NULL;    // initialize selection
	if(m_Updated==TRUE) return;
	m_Updated=TRUE;
	TC_ITEM projTreeItem;
	projTreeItem.mask         = TCIF_TEXT;
    projTreeItem.pszText      = "Project Browser";     
    projTreeItem.cchTextMax   = 20;     
	m_ModeSelector.InsertItem(0,&projTreeItem);
	TC_ITEM projFilesItem;
	projFilesItem.mask         = TCIF_TEXT;
    projFilesItem.pszText      = "Model Browser";     
    projFilesItem.cchTextMax   = 20;     
	m_ModeSelector.InsertItem(1,&projFilesItem);
	LPDISPATCH lpDispatch;
	m_DataSource.GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);
	m_Left.SetDataSource(lpDispatch);
	m_Right.SetDataSource(lpDispatch);
	m_Tree.SetDataSource(lpDispatch);
	m_Modules.SetDataSource(lpDispatch);
	// Browser ActiveX properties initialization.
	// Any category request will but it's results into "CategoryResultsLeft" server variable
	m_Left.SetResultsFromCategory("CategoryResultsLeft");
	// Any query request will but it's results into "QueryResultsLeft" server variable
	m_Left.SetResultsFromQuery("QueryResultsLeft");
	// Any group request will but it's results into "GroupResultsLeft" server variable
	m_Left.SetResultsFromGroup("GroupResultsLeft");
    // Any selection in categories mode will be placed into "SelectionLeft" server variable
	m_Left.SetSelectionFromCategory("SelectionLeft");
    // Any selection in queries mode will be placed into "SelectionLeft" server variable
	m_Left.SetSelectionFromQuery("SelectionLeft");
    // Any selection in groups mode will be placed into "SelectionLeft" server variable
	m_Left.SetSelectionFromGroup("SelectionLeft");
	// We will use other browser window selection as arguments for our queries
	m_Left.SetArguments("SelectionRight");
	// Initial mode for this window will be "Categories"
	m_Left.SetMode(0);
	// Browser ActiveX properties initialization.
	// Any category request will but it's results into "CategoryResultsRight" server variable
	m_Right.SetResultsFromCategory("CategoryResultsRight");
	// Any query request will but it's results into "QueryResultsRight" server variable
	m_Right.SetResultsFromQuery("QueryResultsRight");
	// Any group request will but it's results into "GroupResultsRight" server variable
	m_Right.SetResultsFromGroup("GroupResultsRight");
    // Any selection in categories mode will be placed into "SelectionRight" server variable
	m_Right.SetSelectionFromCategory("SelectionRight");
    // Any selection in queries mode will be placed into "SelectionRight" server variable
	m_Right.SetSelectionFromQuery("SelectionRight");
    // Any selection in groups mode will be placed into "SelectionRight" server variable
	m_Right.SetSelectionFromGroup("SelectionRight");
	// We will use other browser window selection as arguments for our queries
	m_Right.SetArguments("SelectionLeft");
	// Initial mode for this window will be "Categories"
	m_Right.SetMode(0);
	m_DividerSash.SetContextWindow(this);
	m_DividerSash.SetNotificationMessage(WM_SPLITMOVE);
	CRect rect;
	GetClientRect(rect);
    m_SplitterPos=rect.Width()/2-2;

}


void CMiniBrowser::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);

	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL)
		pActiveItem->SetItemRects();

	if(::IsWindow(m_ModeSelector)) LayoutWindows(cx,cy);
	
}

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.  Here,
//  the container (not the server) causes the deactivation.
void CMiniBrowser::OnCancelEditCntr()
{
	// Close any in-place active item on this view.
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL)
	{
		pActiveItem->Close();
	}
	ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
}

// Special handling of OnSetFocus and OnSize are required for a container
//  when an object is being edited in-place.
void CMiniBrowser::OnSetFocus(CWnd* pOldWnd)
{
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL &&
		pActiveItem->GetItemState() == COleClientItem::activeUIState)
	{
		// need to set focus to this item if it is in the same view
		CWnd* pWnd = pActiveItem->GetInPlaceWindow();
		if (pWnd != NULL)
		{
			pWnd->SetFocus();   // don't call the base class
			return;
		}
	}

	CFormView::OnSetFocus(pOldWnd);
}

BOOL CMiniBrowser::IsSelected(const CObject* pDocItem) const
{
	// The implementation below is adequate if your selection consists of
	//  only CDiscoverMDICntrItem objects.  To handle different selection
	//  mechanisms, the implementation here should be replaced.

	// TODO: implement this function that tests for a selected OLE client item

	return pDocItem == m_pSelection;
}



void CMiniBrowser::OnInsertObject()
{
	// Invoke the standard Insert Object dialog box to obtain information
	//  for new CDiscoverMDICntrItem object.
	COleInsertDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;

	BeginWaitCursor();

	CDiscoverMDICntrItem* pItem = NULL;
	TRY
	{
		// Create new item connected to this document.
		CDiscoverMDIDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		pItem = new CDiscoverMDICntrItem(pDoc);
		ASSERT_VALID(pItem);

		// Initialize the item from the dialog data.
		if (!dlg.CreateItem(pItem))
			AfxThrowMemoryException();  // any exception will do
		ASSERT_VALID(pItem);

		// If item created from class list (not from file) then launch
		//  the server to edit the item.
		if (dlg.GetSelectionType() == COleInsertDialog::createNewItem)
			pItem->DoVerb(OLEIVERB_SHOW, this);

		ASSERT_VALID(pItem);

		// As an arbitrary user interface design, this sets the selection
		//  to the last item inserted.

		// TODO: reimplement selection as appropriate for your application

		m_pSelection = pItem;   // set selection to last inserted item
		pDoc->UpdateAllViews(NULL);
	}
	CATCH(CException, e)
	{
		if (pItem != NULL)
		{
			ASSERT_VALID(pItem);
			pItem->Delete();
		}
		AfxMessageBox(IDP_FAILED_TO_CREATE);
	}
	END_CATCH

	EndWaitCursor();
}
/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIView diagnostics

#ifdef _DEBUG
void CMiniBrowser::AssertValid() const
{
	CFormView::AssertValid();
}

void CMiniBrowser::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CDiscoverMDIDoc* CMiniBrowser::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDiscoverMDIDoc)));
	return (CDiscoverMDIDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// OLE Server support

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.  Here,
//  the server (not the container) causes the deactivation.
void CMiniBrowser::OnCancelEditSrvr()
{
	GetDocument()->OnDeactivateUI(FALSE);
}
void CMiniBrowser::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMiniBrowser::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CMiniBrowser::OnDestroy()
{
	// Deactivate the item on destruction; this is important
	// when a splitter view is being used.
   CFormView::OnDestroy();
   COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
   if (pActiveItem != NULL && pActiveItem->GetActiveView() == this)
   {
      pActiveItem->Deactivate();
      ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
   }
}

BOOL CMiniBrowser::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}


//---------------------------------------------------------------------------------------
// Callback runs when Windows ask to create a window. We will use it to create splitter
// between project tree/project modeules and between two brojsers.
//---------------------------------------------------------------------------------------
int CMiniBrowser::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_DividerSash.Create("", WS_CHILD  | WS_VISIBLE | SS_NOTIFY, CRect(0,0,0,0),this,10004);
	m_DividerSash.ModifyStyleEx(0,WS_EX_DLGMODALFRAME);	
	m_Updated=FALSE;
	return 0;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// Callback runs every time  sash changed it's position.
// We need to reposition our project and model browser windows.
//---------------------------------------------------------------------------------
LRESULT CMiniBrowser::OnSplitMoved(WPARAM wparam, LPARAM lparam) {
CRect winRect;
CRect sashRect;

    GetWindowRect(&winRect);
	m_DividerSash.GetWindowRect(&sashRect);
    m_SplitterPos=(int)wparam+(sashRect.left-winRect.left);
	LayoutWindows(winRect.Width(),winRect.Height());
    return TRUE;
}
//---------------------------------------------------------------------------------

void CMiniBrowser::LayoutWindows(int cx, int cy) {

	if(cx>8 && (m_SplitterPos>(cx-8))) m_SplitterPos=cx-8;
    m_DividerSash.MoveWindow(m_SplitterPos-3,
							 HEADER+BORDER+SELECTOR_HEIGHT,
							 6,
							 cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

    m_ModeSelector.MoveWindow(BORDER,HEADER+BORDER,cx-2*BORDER-SASH_SPACE,SELECTOR_HEIGHT);

    m_Tree.MoveWindow(BORDER,HEADER+BORDER+SELECTOR_HEIGHT,
		              m_SplitterPos-BORDER-SPLITTER_SPACE/2,
					  cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

    m_Modules.MoveWindow(m_SplitterPos+SPLITTER_SPACE/2, HEADER+BORDER+SELECTOR_HEIGHT,
						 cx-m_SplitterPos-BORDER-SASH_SPACE-SPLITTER_SPACE/2,
						 cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

    m_Left.MoveWindow(BORDER,HEADER+BORDER+SELECTOR_HEIGHT,
		              m_SplitterPos-BORDER-SPLITTER_SPACE/2,
					  cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);

    m_Right.MoveWindow(m_SplitterPos+SPLITTER_SPACE/2,
		               HEADER+BORDER+SELECTOR_HEIGHT,
				       cx-m_SplitterPos-BORDER-SASH_SPACE-SPLITTER_SPACE/2,
					   cy-(HEADER+BORDER+SELECTOR_HEIGHT)-BORDER);


}

BEGIN_EVENTSINK_MAP(CMiniBrowser, CFormView)
    //{{AFX_EVENTSINK_MAP(CMiniBrowser)
	ON_EVENT(CMiniBrowser, IDC_PROJECTTREE, 1 /* NodeChanged */, OnNodeChanged, VTS_BSTR)
	ON_EVENT(CMiniBrowser, IDC_LEFTPANE, 1 /* ModeChanged */, OnLeftModeChanged, VTS_I2)
	ON_EVENT(CMiniBrowser, IDC_LEFTPANE, 2 /* CategoryChanged */, OnLeftCategoryChanged, VTS_BSTR)
	ON_EVENT(CMiniBrowser, IDC_LEFTPANE, 4 /* SelectionChanged */, OnLeftSelectionChanged, VTS_BSTR)
	ON_EVENT(CMiniBrowser, IDC_RIGHTPANE, 2 /* CategoryChanged */, OnRightCategoryChanged, VTS_BSTR)
	ON_EVENT(CMiniBrowser, IDC_RIGHTPANE, 4 /* SelectionChanged */, OnRightSelectionChanged, VTS_BSTR)
	ON_EVENT(CMiniBrowser, IDC_MSDEVINTEGRATOR, 1 /* CtrlClick */, IntegratorCtrlClick, VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMiniBrowser, IDC_MSDEVINTEGRATOR, 2 /* Query */, IntegratorQuery, VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMiniBrowser, IDC_MSDEVINTEGRATOR, 3 /* Definition */, IntegratorDefinition, VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMiniBrowser, IDC_MSDEVINTEGRATOR, 4 /* Activate */, IntegratorActivate, VTS_NONE)
	ON_EVENT(CMiniBrowser, IDC_LEFTPANE, 6 /* ElementDoubleclicked */, OnDoubleclick, VTS_BSTR)
	ON_EVENT(CMiniBrowser, IDC_RIGHTPANE, 6 /* ElementDoubleclicked */, OnDoubleclick, VTS_BSTR)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMiniBrowser::OnNodeChanged(LPCTSTR projectName)  {

	m_CurrentProject=projectName;
    if(::IsWindow(m_Left)) {
		m_Left.SetProjects(m_CurrentProject);
		m_Left.Update();
	}

    if(IsWindow(m_Right)) {
		m_Right.SetProjects(m_CurrentProject);
		m_Right.Update();
	}
    if(m_CurrentProject!=m_Modules.GetProject()) 
		m_Modules.SetProject(m_CurrentProject);	
}

void CMiniBrowser::OnModeChanged(NMHDR* pNMHDR, LRESULT* pResult)  {
	switch(m_ModeSelector.GetCurSel()) {
	   // The user selected tree presentation
	   case 0 : m_Left.ShowWindow(SW_HIDE); 
		        m_Right.ShowWindow(SW_HIDE); 
				m_Modules.ShowWindow(SW_SHOW); 
		        m_Tree.ShowWindow(SW_SHOW); 
		        break;
	   case 1 : m_Modules.ShowWindow(SW_HIDE); 
		        m_Tree.ShowWindow(SW_HIDE); 
				m_Left.ShowWindow(SW_SHOW); 
		        m_Right.ShowWindow(SW_SHOW); 
		        break;
	}
	*pResult = 0;
}

void CMiniBrowser::OnLeftModeChanged(short newMode) 
{
	// TODO: Add your control notification handler code here
	
}

void CMiniBrowser::OnLeftCategoryChanged(LPCTSTR categoriesList)  {
   m_Right.SetSelectedCategories(categoriesList);	
}

void CMiniBrowser::OnLeftSelectionChanged(LPCTSTR varName)  {
   m_Right.SetArguments(m_Right.GetArguments());	
}

void CMiniBrowser::OnRightCategoryChanged(LPCTSTR categoriesList)  {
   m_Left.SetSelectedCategories(categoriesList);
	
}

void CMiniBrowser::OnRightSelectionChanged(LPCTSTR varName)  {
   m_Left.SetArguments(m_Left.GetArguments());	
}

void CMiniBrowser::IntegratorCtrlClick(LPCTSTR lfile, long line, long col, long token)  {
CString serverQuery;
CString infoString;
CString ffile="";
BSTR q;

	// Asking Developer Studio for it's current tab size.
	int tabsize = m_MSDEVIntegrator.GetTabSize();
	// Getting info string
	register i;
	if(lfile!=NULL) {
		for(i=0;i<strlen(lfile);i++) {
		    if(lfile[i]=='\\') 
			    ffile+="/";
		    else
			    ffile+=lfile[i];
		}
	}
    serverQuery.Format("msdev_info \"%s\" %ld %ld %ld",ffile, line, col, tabsize);
	q=serverQuery.AllocSysString();
	infoString=DataSourceAccessSync(&q);
    ::SysFreeString(q);
	m_MSDEVIntegrator.SetInfo(infoString);
}



void CMiniBrowser::IntegratorQuery(LPCTSTR lfile, long line, long col, long len)  {
CString serverQuery;
CString infoString;
CString ffile;
BSTR q;


	// Asking Developer Studio for it's current tab size.
	int tabsize = m_MSDEVIntegrator.GetTabSize();

	register i;
	if(lfile!=NULL) {
		for(i=0;i<strlen(lfile);i++) {
		    if(lfile[i]=='\\') 
			    ffile+="/";
		    else
			    ffile+=lfile[i];
		}
	}
	// Getting symbol
    serverQuery.Format("set tmp [msdev2sym \"%s\" %ld %ld %ld]",ffile, line, col-1, tabsize);
	q=serverQuery.AllocSysString();
	CString test;
	test=DataSourceAccessSync(&q);
    ::SysFreeString(q);

	// Setting symbol
	m_Left.SetSet("tmp");

    serverQuery="unset tmp";
	q=serverQuery.AllocSysString();
	DataSourceAccessSync(&q);
    ::SysFreeString(q);
}

void CMiniBrowser::IntegratorDefinition(LPCTSTR lfile, long line, long col, long len)  {
register i;
CString serverQuery;
CString ffile;
CString param;
int     count;
CString filestr;
CString tokenstr;
CString linestr;
CString colstr;
BSTR q;

	// Asking Developer Studio for it's current tab size.
	int tabsize = m_MSDEVIntegrator.GetTabSize();

	if(lfile!=NULL) {
		for(i=0;i<strlen(lfile);i++) {
		    if(lfile[i]=='\\') 
			    ffile+="/";
		    else
			    ffile+=lfile[i];
		}
	}
	// Getting symbol
    serverQuery.Format("set __dis__tmp [msdev2sym \"%s\" %ld %ld %ld]",ffile, line, col-1, tabsize);
	q=serverQuery.AllocSysString();
	CString test;
	test=DataSourceAccessSync(&q);
    ::SysFreeString(q);

	// Queryng server to obtain Developer Studio filename, line and position.
	// The "sym2msdev <tabsize> <element#> <set>" command will return the following :
	// filename \n
	// line\n
	// column\n
	// token
    serverQuery.Format("sym2msdev %d 0 $__dis__tmp",tabsize);
	q=serverQuery.AllocSysString();
	param=DataSourceAccessSync(&q);
    ::SysFreeString(q);
	// Extracting parameters
	count=0;
	for(i=0;i<param.GetLength();i++) {
		if(param[i]=='\n') {
			count++;
            continue;
		}
		switch(count) {
		   case 0 : filestr += param[i]; break;
		   case 1 : linestr += param[i]; break;
		   case 2 : colstr  += param[i]; break;
		   case 3 : tokenstr+= param[i]; break;
		}
	}
	// Converting logical name into NT name
	// Selecting
	if(filestr.GetLength()!=0)
       m_MSDEVIntegrator.MakeSelection(filestr,atoi(linestr),atoi(colstr)-1,tokenstr);

    serverQuery="unset tmp";
	q=serverQuery.AllocSysString();
	DataSourceAccessSync(&q);
    ::SysFreeString(q);
}

void CMiniBrowser::IntegratorActivate()  {
  m_MSDEVIntegrator.MakeSelection("C:\\Discov1.prefs",0,0,"");
}

void CMiniBrowser::OnDoubleclick(LPCTSTR parmlist)  {
register i;
CString serverQuery;
CString serverResults;
CString param;
CString filestr;
CString tokenstr;
CString linestr;
CString colstr;
BSTR q;

CString filteredSetName;
CString elementN;
int elNumber;
int count=0;

    for(i=0;i<strlen(parmlist);i++) {
	   if(parmlist[i]==' ') {
		   count++;
		   continue;
	   }
	   if(count==0) filteredSetName+=parmlist[i];
	   if(count==1) elementN  +=parmlist[i];
    }
    elNumber=atoi(elementN);

	// Asking Developer Studio for it's current tab size.
	int tabsize = m_MSDEVIntegrator.GetTabSize();

	// Queryng server to obtain Developer Studio filename, line and position.
	// The "sym2msdev <tabsize> <element#> <set>" command will return the following :
	// logfilename \n
	// line\n
	// column\n
	// token
    serverQuery.Format("sym2msdev %d %ld $%s",tabsize, elNumber,filteredSetName);
	q=serverQuery.AllocSysString();
	param=DataSourceAccessSync(&q);
    ::SysFreeString(q);
	// Extracting parameters
	count=0;
	for(i=0;i<param.GetLength();i++) {
		if(param[i]=='\n') {
			count++;
            continue;
		}
		switch(count) {
		   case 0 : filestr += param[i]; break;
		   case 1 : linestr += param[i]; break;
		   case 2 : colstr  += param[i]; break;
		   case 3 : tokenstr+= param[i]; break;
		}
	}
	// Selecting
	if(filestr.GetLength()!=0)
       m_MSDEVIntegrator.MakeSelection(filestr,atoi(linestr),atoi(colstr)-1,tokenstr);

	
}

CString CMiniBrowser::DataSourceAccessSync(BSTR* command) {
	CString res;
	static BOOL bCommDlgShown = FALSE;
	res = m_DataSource.AccessSync(command);
	if (m_DataSource.IsConnectionLost() == TRUE &&
		bCommDlgShown != TRUE ) {
		::MessageBox(m_hWnd, 
			   _T("Connection with server is lost."
			   " Make sure the server is running."),
			   _T("Server Communication Error."), 
			   MB_OK | MB_ICONINFORMATION); 
		bCommDlgShown = TRUE;
	} else if (m_DataSource.IsConnectionLost() != TRUE) {
		bCommDlgShown = FALSE;
	}	
	return res;
}
