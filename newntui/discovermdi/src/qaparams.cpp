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
// QAParams.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "QAParams.h"
#include "QATree.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SPACE 5

/////////////////////////////////////////////////////////////////////////////
// CQAParams

IMPLEMENT_DYNCREATE(CQAParams, CFormView)

CQAParams::CQAParams()
	: CFormView(CQAParams::IDD)
{
	//{{AFX_DATA_INIT(CQAParams)
	//}}AFX_DATA_INIT
}

CQAParams::~CQAParams()
{
}

void CQAParams::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQAParams)
	DDX_Control(pDX, IDC_SCOPETITLE, m_ScopeTitle);
	DDX_Control(pDX, IDC_PROPERTIESTITLE, m_PropertiesTitle);
	DDX_Control(pDX, IDC_MODESELECTOR, m_ModeSelector);
	DDX_Control(pDX, IDC_PROPERTIES, m_Properties);
	DDX_Control(pDX, IDC_REPORTER, m_Reporter);
	DDX_Control(pDX, IDC_SCOPE, m_ScopeCombo);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQAParams, CFormView)
	//{{AFX_MSG_MAP(CQAParams)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_MODESELECTOR, OnModeChanged)
	ON_CBN_SELCHANGE(IDC_SCOPE, OnSelchangeScope)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQAParams diagnostics

#ifdef _DEBUG
void CQAParams::AssertValid() const
{
	CFormView::AssertValid();
}

void CQAParams::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CQAParams message handlers


//-----------------------------------------------------------------------
// This callback will run every time the window created/changed it's size.
// We will use it to position all controls on our page.
//-----------------------------------------------------------------------
void CQAParams::OnSize(UINT nType, int cx, int cy)  {
	CFormView::OnSize(nType, cx, cy);
	LayoutControls(cx,cy);
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// This private function designed to reposition all controls according the
// given window size. It is called from the OnSize (WM_SIZE callback).
//-----------------------------------------------------------------------
void CQAParams::LayoutControls(int cx, int cy) {
int ypos=0;
int yend=cy;
CRect scopeTitleRect;
CRect propTitleRect;
CRect scopeComboRect;

	if(!::IsWindow(m_ModeSelector)) return;
	m_ScopeTitle.GetWindowRect(scopeTitleRect);
	m_PropertiesTitle.GetWindowRect(propTitleRect);
	m_ScopeCombo.GetWindowRect(scopeComboRect);
	m_ModeSelector.MoveWindow(0,ypos,cx,25);
	ypos+=25+SPACE;
	m_ScopeTitle.MoveWindow(0,ypos,cx,scopeTitleRect.Height());
	ypos+=scopeTitleRect.Height()+SPACE;
	m_ScopeCombo.MoveWindow(SPACE,ypos,cx-2*SPACE,scopeComboRect.Height());
	ypos+=scopeComboRect.Height()+SPACE;
	m_PropertiesTitle.MoveWindow(0,ypos,cx,propTitleRect.Height());
	ypos+=propTitleRect.Height()+SPACE;

	yend-=SPACE;

	m_Properties.MoveWindow(SPACE,ypos,cx-2*SPACE,yend-ypos);

	if(m_ShowReport==TRUE) { 
		m_Reporter.SetTop(25);
		m_Reporter.SetLeft(SPACE);
		m_Reporter.SetWidth(cx-2*SPACE);
		m_Reporter.SetHeight(cy-25-SPACE);
	}
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// This callback will run when all windows are created but still invisible.
// We will use it to fill our mode selector tab control.
//-----------------------------------------------------------------------
void CQAParams::OnInitialUpdate()  {
TC_ITEM item;
CString query;
CString types;
BSTR sysQuery;

	CFormView::OnInitialUpdate();
	if(m_Updated==FALSE)  {
        SIZE sizeTotal;
        sizeTotal.cx=0;
        sizeTotal.cy=0;
        SetScrollSizes( MM_TEXT, sizeTotal);

		m_Updated=TRUE;
	    item.mask         = TCIF_TEXT;
        item.pszText      = "Settings";     
        item.cchTextMax   = 20;     
	    m_ModeSelector.InsertItem(0,&item);

		EnableReport(false);

	    m_Properties.SetUserSelection(1);
	    m_Properties.SetFixedRows(1);
	    m_Properties.SetFixedCols(0);
	    m_Properties.SetCols(2);
	    m_Properties.SetRows(1);
        m_Properties.SetColWidth(0,200);
        m_Properties.SetColWidth(1,100);
	    m_Properties.SetCellText(0,0,"Property name");
	    m_Properties.SetCellText(0,1,"Property value");

	    ShowReportPage(FALSE);

        query="sev_get_input_types";
        sysQuery=query.AllocSysString();
        types=DataSourceAccessSync(&sysQuery);
	    SysFreeString(sysQuery); 

		BOOL in=FALSE;
		CString name;
		int line=0;
		for(int i=0;i<types.GetLength();i++) {
			if(types[i]=='{') {
				in=TRUE;
				continue;
			}
			if( (types[i]=='}') || (types[i]==' ' && in==FALSE) ) {
				if(name.GetLength()>0)
                     m_ScopeCombo.InsertString(line++,name);
				name="";
                in=FALSE;
				continue;
			}
			name+=types[i];
		}
		m_ScopeCombo.SetCurSel(0);
	}
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// This callback will run when the main window is created (childs still
// unavailable). We will use it to indicate that we need to fill our
// once-time needed information in OnInitialUpdate.
//-----------------------------------------------------------------------
int CQAParams::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Updated=FALSE;
	
	return 0;
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// This private function will show or hide all controls on the settings
// page depending on the show flag.
//-----------------------------------------------------------------------
void CQAParams::ShowSettingsPage(BOOL show) {
	int nCmd = show?SW_SHOW:SW_HIDE;
	m_ScopeTitle.ShowWindow(nCmd);
	m_ScopeCombo.ShowWindow(nCmd);
	m_PropertiesTitle.ShowWindow(nCmd);
	m_Properties.ShowWindow(nCmd);
	m_Properties.EnableWindow(false);
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// This private function will show or hide all controls on the report
// page depending on the show flag.
//-----------------------------------------------------------------------
void CQAParams::ShowReportPage(BOOL show) {
	if(::IsWindow(m_Reporter)) {
		CRect winRect;
        GetClientRect(&winRect);
		if(show==FALSE) {
			m_Reporter.Stop();
	        
			m_Reporter.SetTop(25);
	        m_Reporter.SetLeft(SPACE);
	        m_Reporter.SetWidth(0);
	        m_Reporter.SetHeight(0);
			m_ShowReport=FALSE;
		} else {
	        m_Reporter.SetTop(25);
	        m_Reporter.SetLeft(SPACE);
	        m_Reporter.SetWidth(winRect.Width()-2*SPACE);
	        m_Reporter.SetHeight(winRect.Height()-25-SPACE);
			m_ShowReport=TRUE;
		}
	}
} 
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// This callback will run every time the user switched between parameters
// and report page. We will use it to show/hide our pages.
//-----------------------------------------------------------------------
void CQAParams::OnModeChanged(NMHDR* pNMHDR, LRESULT* pResult) {

	switch(m_ModeSelector.GetCurSel()) {
		// This is settings page.
	    case 0 : ShowReportPage(FALSE);
			     ShowSettingsPage(TRUE);
				 break;
		// This is report page.
	    case 1 :
			if(m_ModeSelector.GetItemCount()) {
				ShowSettingsPage(FALSE);
				UpdateReport();
			    ShowReportPage(TRUE);
			} else {
				m_ModeSelector.SetCurSel(0);
			}
			break;
	}
	*pResult = 0;
}
//-----------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This public function will query the server to fill all properties of the node 
// with the nodename mentioned.
//---------------------------------------------------------------------------------------
void CQAParams::FillPropertiesOfNode(CString & node) {
CString query;
CString type;
CString status;
CString weight;
CString thresh;
CString score;
CString total;
CString hits;
BSTR sysQuery;
int i=1;

    m_Properties.Clear();
    if(node.GetLength()==0) {
         m_Properties.SetCols(2);
		 return;
	}
    query.Format("sev_get {%s} type",node);
    sysQuery=query.AllocSysString();
    type=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
	if(type=="query_1") m_Properties.SetRows(7);
	else                m_Properties.SetRows(6);

    // Getting the node status
    query.Format("sev_get {%s} on",node);
    sysQuery=query.AllocSysString();
    status=m_DataSource.AccessSync(&sysQuery);
	SysFreeString(sysQuery); 
	m_Properties.SetCellType(i,1,3);
	m_Properties.AddMenuItem(i,1,"Enabled");
	m_Properties.AddMenuItem(i,1,"Disabled");
    m_Properties.SetCellText(i,0,"Status");
	if(status=="1")
        m_Properties.SetCellText(i,1,"Enabled");
    else
        m_Properties.SetCellText(i,1,"Disabled");
	i++;

    // Getting the node weight
    query.Format("sev_get {%s} weight",node);
    sysQuery=query.AllocSysString();
    weight=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
    m_Properties.SetCellText(i,0,"Weight");
	m_Properties.SetCellType(i,1,1);
    m_Properties.SetCellText(i,1,weight);
	i++;

    // Getting the type of the node
	if(type=="query_1") {
           m_Properties.SetCellText(i,0,"Threshold");
           // Getting the node threshold
           query.Format("sev_get {%s} threshold",node);
           sysQuery=query.AllocSysString();
           thresh=DataSourceAccessSync(&sysQuery);
	       SysFreeString(sysQuery); 
	       m_Properties.SetCellType(i,1,1);
           m_Properties.SetCellText(i,1,thresh);
	       i++;
	}
    
    // Getting the node  score
    query.Format("sev_get {%s} score",node);
    sysQuery=query.AllocSysString();
    score=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
    m_Properties.SetCellText(i,0,"Score");
    m_Properties.SetCellText(i,1,score);
	i++;
	
    // Getting the node set
    query.Format("sev_get {%s} hitno",node);
    sysQuery=query.AllocSysString();
    hits=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
    m_Properties.SetCellText(i,0,"Hits");
    m_Properties.SetCellText(i,1,hits);
	i++;

    // Getting the node total score
    query.Format("sev_get {/} score",node);
    sysQuery=query.AllocSysString();
    total=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
    m_Properties.SetCellText(i,0,"Total score");
    m_Properties.SetCellText(i,1,total);
	i++;
}
//------------------------------------------------------------------------------------

BEGIN_EVENTSINK_MAP(CQAParams, CFormView)
    //{{AFX_EVENTSINK_MAP(CQAParams)
	ON_EVENT(CQAParams, IDC_PROPERTIES, 3 /* SetChanged */, OnSetChangedProperties, VTS_I4 VTS_I4 VTS_BSTR)
	ON_EVENT(CQAParams, IDC_PROPERTIES, 2 /* StringChanged */, OnStringChanged, VTS_I4 VTS_I4 VTS_BSTR)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


//--------------------------------------------------------------------------------------------
// Callback runs when any set-type cell changed it's value.
//--------------------------------------------------------------------------------------------
void CQAParams::OnSetChangedProperties(long row, long col, LPCTSTR val)  {
CQATree* view;
      view=(CQATree* )(((CSplitterWnd *)GetParent())->GetPane(0,0));
      view->SetCurrentItemStatus(CString(val)=="Enabled");
}
//--------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This private function will check if the string specified is a digit.
//---------------------------------------------------------------------------------------
BOOL CQAParams::IsDigit(CString& token) {
	for(int i=0;i<token.GetLength();i++) {
		if(token[i]!='0' && token[i]!='1' && token[i]!='2' && token[i]!='3'&&
           token[i]!='4' && token[i]!='5' && token[i]!='6' && token[i]!='7' &&
		   token[i]!='8' && token[i]!='9' && token[i]!='.') return FALSE;
	}
	int dotCount=0;
	for(i=0;i<token.GetLength();i++) {
		if(token[i]=='.')  {
			if(dotCount>0)  return FALSE;
			dotCount++;
		}
	}
	return TRUE;
}
//---------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// This callback will run every time the user changed the value in the edit cell.
//--------------------------------------------------------------------------------------------
void CQAParams::OnStringChanged(long row, long col, LPCTSTR val)  {
CString query;
CString weight;
BSTR sysQuery;
CQATree* view;
CString node;

    view=(CQATree* )(((CSplitterWnd *)GetParent())->GetPane(0,0));
    node=(*view->GetCurrentNodeName());
    if(row==2) {
        if(IsDigit(CString(val))) {
            query.Format("sev_set {%s} weight %s",node,val);
            sysQuery=query.AllocSysString();
            DataSourceAccessSync(&sysQuery);
	        SysFreeString(sysQuery); 
            query.Format("sev_get {%s} weight",node);
            sysQuery=query.AllocSysString();
            weight=DataSourceAccessSync(&sysQuery);
            m_Properties.SetCellText(2,1,weight);
	        SysFreeString(sysQuery); 
		} else {
		    MessageBox("The value you'v entered is not digital.","Properties Entry Error",MB_ICONEXCLAMATION);
            query.Format("sev_get {%s} weight",node);
            sysQuery=query.AllocSysString();
            weight=DataSourceAccessSync(&sysQuery);
            m_Properties.SetCellText(2,1,weight);
	        SysFreeString(sysQuery); 
		}
	}
    if(row==3) {
        if(IsDigit(CString(val))) {
            query.Format("sev_set {%s} threshold %s",node,val);
            sysQuery=query.AllocSysString();
            CString rr = DataSourceAccessSync(&sysQuery);
	        SysFreeString(sysQuery); 
            query.Format("sev_get {%s} threshold",node);
            sysQuery=query.AllocSysString();
            weight=DataSourceAccessSync(&sysQuery);
            m_Properties.SetCellText(3,1,weight);
	        SysFreeString(sysQuery); 
		} else {
		    MessageBox("The value you'v entered is not digital.","Properties Entry Error",MB_ICONEXCLAMATION);
            query.Format("sev_get {%s} threshold",node);
            sysQuery=query.AllocSysString();
            weight=DataSourceAccessSync(&sysQuery);
            m_Properties.SetCellText(3,1,weight);
	        SysFreeString(sysQuery); 
		}
	}
	
}
//--------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This public function will return the currently selected scope to run the query.
//---------------------------------------------------------------------------------------
void CQAParams::GetScope(CString & scope) {
	int sel = m_ScopeCombo.GetCurSel();
	if(sel<0) {
		scope="";
		return;
	}
	m_ScopeCombo.GetLBText(sel,scope); 

}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This public function will re-fill both parameters and browser pages. We need it to
// reflect changes after running our queries.
//---------------------------------------------------------------------------------------
void CQAParams::Refresh(CString& node) {
	FillPropertiesOfNode(node);
	if(m_ModeSelector.GetCurSel()==1) UpdateReport();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This public function will inspect the currently selected item trying to obtain the
// current result set. If the result set is available it will return TRUE, if not - FALSE.
// Don't try to use it directly in the command enabler - it runs server query and will
// fill communication channel with the  durt data.
//---------------------------------------------------------------------------------------
BOOL CQAParams::GetCurrentSetEnable(void) {
CString query;
CString hits;
BSTR sysQuery;
CQATree* view;
CString node;

    // We need the current node name to run the query.
    view=(CQATree* )(((CSplitterWnd *)GetParent())->GetPane(0,0));
    node=(*view->GetCurrentNodeName());

	// Trying to detect the amount of the elements
    query.Format("sev_get {%s} hitno",node);
    sysQuery=query.AllocSysString();
    hits=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 

	int amount=atoi(hits);
	if(amount<=0) return FALSE;
	return TRUE;

}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This public function will fill the given variable with the data from the currently 
// selected query results set. It will fill the variable with the nil_set if no set
// available.
//---------------------------------------------------------------------------------------
void CQAParams::GetCurrentSet(CString& to) {
CString query;
CString set;
BSTR sysQuery;
CQATree* view;
CString node;

    // We need the current node name to run the query.
    view=(CQATree* )(((CSplitterWnd *)GetParent())->GetPane(0,0));
    node=(*view->GetCurrentNodeName());

	// Trying to detect the amount of the elements
    query.Format("set %s [sev_get {%s} retval]",to,node);
    sysQuery=query.AllocSysString();
    set=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
}
//---------------------------------------------------------------------------------------

void CQAParams::FreeCurrentSet(CString & name) {
CString query;
BSTR sysQuery;

    query.Format("unset %s",name);
    sysQuery=query.AllocSysString();
    DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
}

bool CQAParams::IsReportAvailable() {
   CString reportFileName = GetReportName();
   return GetFileAttributes(reportFileName) != (DWORD)-1;
}

CString CQAParams::GetReportName() {
   char buf[1024];
   CString reportFileName;
   if(GetEnvironmentVariable("PSETHOME_LOCAL",buf,1024)!=0) {	     
         reportFileName=buf;
         reportFileName+="\\Reports\\";
   } else {
         reportFileName="C:\\Discover\\Reports\\";
   }
   reportFileName+="qa.htm";
   return reportFileName;
}

void CQAParams::UpdateReport() {
   CString reportFileName = GetReportName();

   CMainFrame* mainFrame;
   mainFrame=(CMainFrame *)AfxGetMainWnd();
   if(mainFrame->m_Lic[LIC_QAREPORT]>0 && IsReportAvailable()) {
		m_Reporter.Navigate(reportFileName,NULL,NULL,NULL,NULL);
   }
}

CString CQAParams::DataSourceAccessSync(BSTR* command) {
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

void CQAParams::EnableReport(bool bEnable)
{
	if(bEnable) {
		if(m_ModeSelector.GetItemCount()==1) {
			TC_ITEM item;
			item.mask         = TCIF_TEXT;
			item.pszText      = "Report";     
			item.cchTextMax   = 20;     
			m_ModeSelector.InsertItem(1,&item);
		}
	} else {
		if(m_ModeSelector.GetItemCount()>1) {
			m_ModeSelector.SetCurSel(0);
			LRESULT res = 0;
			OnModeChanged(NULL, &res);
			m_ModeSelector.DeleteItem(1);
		}
	}
}

void CQAParams::OnSelchangeScope() 
{
	m_Properties.SetFocus();
}
