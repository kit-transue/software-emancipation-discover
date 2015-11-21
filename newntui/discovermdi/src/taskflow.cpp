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
// TaskFlow.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "TaskFlow.h"
#include "TaskNewDlg.h"
#include "MainFrm.h"

#define SPACE  2

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif







DROPEFFECT CTaskDropTarget::OnDragEnter(CWnd* pWnd,COleDataObject* pDataObject,DWORD dwKeyState,CPoint point ) {
UINT formatID;
  // If the format with the spesified name was already registered,
  // this function will return existing format ID. If this is first 
  // time we try to register clipboard format, this function will
  // register it and return new ID.
  formatID=RegisterClipboardFormat("DiscoverTaskFlowNode");
  if(pDataObject->IsDataAvailable(formatID)==TRUE) {
     return DROPEFFECT_COPY;
  } else  {
	 return DROPEFFECT_NONE;
  }
}


DROPEFFECT  CTaskDropTarget::OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point ) {
	return DROPEFFECT_COPY;
}


BOOL  CTaskDropTarget::OnDrop( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point ) {
UINT formatID;
HGLOBAL dataID;
int* elements;
int operation;
int size;

    formatID=RegisterClipboardFormat("DiscoverTaskFlowNode");
	if(pDataObject->IsDataAvailable(formatID)==TRUE) {
		dataID=pDataObject->GetGlobalData(formatID);
		elements=(int *)GlobalLock(dataID);
		operation= elements[0];
		size     = elements[1];
		switch(operation) {
		   case OP_INSERT : m_Parent->InsertItems(size,elements+2,point); break;
           case OP_MOVE   : m_Parent->MoveItems(size,elements+2,point); break;
		}
		::GlobalFree(dataID);
		return TRUE;
	}
	return FALSE;
}






/////////////////////////////////////////////////////////////////////////////
// CTaskFlow

IMPLEMENT_DYNCREATE(CTaskFlow, CFormView)

CTaskFlow::CTaskFlow()
	: CFormView(CTaskFlow::IDD)
{
	EnableAutomation();
	m_Updated=FALSE;
	m_StopQuery=FALSE;
	m_Running = FALSE;
	//{{AFX_DATA_INIT(CTaskFlow)
	//}}AFX_DATA_INIT
}

CTaskFlow::~CTaskFlow() {
}

void CTaskFlow::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CFormView::OnFinalRelease();
}

void CTaskFlow::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTaskFlow)
	DDX_Control(pDX, IDC_TASKFLOW, m_TaskFlow);
	DDX_Control(pDX, IDC_SEQUENCESELECTOR, m_SequenceSelector);
	DDX_Control(pDX, IDC_SEQUENCECAPTION, m_SequenceCaption);
	DDX_Control(pDX, IDC_NEWSEQUENCE, m_NewBtn);
	DDX_Control(pDX, IDC_TASKSPREADSHEET, m_TaskSpreadsheet);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	//}}AFX_DATA_MAP

	CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
	CMainFrame* main = (CMainFrame *) AfxGetMainWnd();
	main->m_wndBrowser.ExecuteCommand(CString("setvar DiscoverScope $DiscoverScope"));
	main->m_wndBrowser.ExecuteCommand(CString("setsym DiscoverSelection $DiscoverSelection"));
	main->m_wndBrowser.ExecuteCommand(CString("setsym DiscoverGroups $DiscoverGroups"));
}


BEGIN_MESSAGE_MAP(CTaskFlow, CFormView)
	//{{AFX_MSG_MAP(CTaskFlow)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_SEQUENCESELECTOR, OnSelchangeSequenceSelector)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TASKFLOW, OnSelchangeNode)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_NOTIFY(TCN_KEYDOWN, IDC_TASKFLOW, OnKeydownTaskFlow)
	ON_BN_CLICKED(IDC_NEWSEQUENCE, OnNewTaskFlow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CTaskFlow, CFormView)
	//{{AFX_DISPATCH_MAP(CTaskFlow)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_ITaskFlow to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {77D99631-3132-11D2-AF08-00A0C9B71DC4}
static const IID IID_ITaskFlow =
{ 0x77d99631, 0x3132, 0x11d2, { 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };

BEGIN_INTERFACE_MAP(CTaskFlow, CFormView)
	INTERFACE_PART(CTaskFlow, IID_ITaskFlow, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTaskFlow diagnostics

#ifdef _DEBUG
void CTaskFlow::AssertValid() const
{
	CFormView::AssertValid();
}

void CTaskFlow::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTaskFlow message handlers

void CTaskFlow::OnSize(UINT nType, int cx, int cy)  {
	//CFormView::OnSize(nType, cx, cy);
	if(cx<100) cx=100;
	if(cy<100) cy=100;
	if(::IsWindow(m_SequenceSelector)) LayoutTaskPage(cx,cy);

}


//----------------------------------------------------------------------------------------
// This private function will reposition all controls on the task flow page.
// The page sizes are given as cx,cy parameters.
// Usually run in responce to WM_SIZE message
//----------------------------------------------------------------------------------------
void CTaskFlow::LayoutTaskPage(int cx, int cy) {
CRect selectorRect;
CRect newRect;
CRect captionRect;
int yPos=SPACE;

   m_SequenceSelector.GetWindowRect(&selectorRect);
   m_NewBtn.GetWindowRect(&newRect);
   m_SequenceSelector.MoveWindow(SPACE,
	                             yPos,
								 cx-SPACE-2*SPACE-newRect.Width(),
								 selectorRect.Height());
   m_NewBtn.MoveWindow(cx-SPACE-newRect.Width(),
	                   yPos,
					   newRect.Width(),
					   selectorRect.Height()
					   );
   yPos+=(selectorRect.Height());
   m_SequenceCaption.GetWindowRect(&captionRect);
   m_SequenceCaption.MoveWindow(SPACE,yPos,cx-2*SPACE,captionRect.Height());
   yPos+=(captionRect.Height());

   m_TaskFlow.MoveWindow(  -3,
	                       yPos,
					       cx+6,
						   26
					     );
   m_TaskSpreadsheet.MoveWindow( -3,
	                             yPos+26,
					             cx+6,
								 cy-(yPos+26)+3
					            );

}





//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void CTaskFlow::OnInitialUpdate()  {

	CFormView::OnInitialUpdate();

    if(m_Updated==TRUE) return;
	m_Updated=TRUE;

    SIZE sizeTotal;
    sizeTotal.cx=100;
    sizeTotal.cy=100;
    SetScrollSizes( MM_TEXT, sizeTotal);

	m_DropTarget.m_Parent=this;

	m_DropTarget.Register(&m_TaskFlow);

	CBitmap filter;
	filter.LoadMappedBitmap(IDB_FILTER);
	CBitmap assignments;
	assignments.LoadMappedBitmap(IDB_ASSIGNMENTS);
	CBitmap query;
	query.LoadMappedBitmap(IDB_QUERY);
	CBitmap diff;
	diff.LoadMappedBitmap(IDB_DIFF);
	CBitmap collect;
	collect.LoadMappedBitmap(IDB_COLLECT);
	CBitmap instances;
	instances.LoadMappedBitmap(IDB_INSTANCES);
	CBitmap pattern;
	pattern.LoadMappedBitmap(IDB_PATTERN);
	CBitmap report;
	report.LoadMappedBitmap(IDB_REPORT);
	CBitmap unknownTask;
	unknownTask.LoadMappedBitmap(IDB_NEWTASK);

    m_FlowImages.Create(16,16,TRUE,7,1);


	m_FlowImages.Add(&filter,        GetSysColor(COLOR_BTNFACE));
	m_FlowImages.Add(&assignments,   GetSysColor(COLOR_BTNFACE));
	m_FlowImages.Add(&query,         GetSysColor(COLOR_BTNFACE));
	m_FlowImages.Add(&diff,          GetSysColor(COLOR_BTNFACE));
	m_FlowImages.Add(&collect,       GetSysColor(COLOR_BTNFACE));
	m_FlowImages.Add(&instances,     GetSysColor(COLOR_BTNFACE));
	m_FlowImages.Add(&pattern,       GetSysColor(COLOR_BTNFACE));
	m_FlowImages.Add(&report,        GetSysColor(COLOR_BTNFACE));
	m_FlowImages.Add(&unknownTask,   GetSysColor(COLOR_BTNFACE));

    m_TaskFlow.SetImageList(&m_FlowImages); 

    m_TaskSpreadsheet.SetRows(2);
	m_TaskSpreadsheet.SetCols(3);
	m_TaskSpreadsheet.SetFixedRows(1);
	m_TaskSpreadsheet.SetFixedCols(0);
	m_TaskSpreadsheet.SetUserSelection(1);
	m_TaskSpreadsheet.SetColWidth(0,150);
	m_TaskSpreadsheet.SetColWidth(1,100);
	m_TaskSpreadsheet.SetColWidth(2,100);
    m_TaskSpreadsheet.SetCellText(0,0,"Property Name");
    m_TaskSpreadsheet.SetCellText(0,1,"Role");
    m_TaskSpreadsheet.SetCellText(0,2,"Value");
    FillSequencesSelector();
    FillTaskFlow();
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// This private function will create the property spreadsheet for the "Collect" task.
//----------------------------------------------------------------------------------------
void CTaskFlow::FillProperties() {
CString bt;
CString query;
BSTR sysQuery;
CString result;
    // Clear our properties spreadsheet
	m_TaskSpreadsheet.Clear();


   // If no elements selected
   if(m_SequenceSelector.GetCurSel()==-1) return;
   // If no node selected;
   if(m_TaskFlow.GetCurSel()==-1) return;

	// Starting work with the current node
	CString sequenceName;
	m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),sequenceName);
	query.Format("InitProperties {%s} %d",sequenceName,m_TaskFlow.GetCurSel());
    sysQuery=query.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

	// Getting properties amount
	query="GetPropertiesAmount";
    sysQuery=query.AllocSysString();
    result=DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);
	int rows=atoi(result);

	// Initialize node spreadshhet
    m_TaskSpreadsheet.SetRows(rows+1);

	// Scanning all properties one by one
	m_TaskSpreadsheet.SetEnableDraw(FALSE);
	for(register i=0;i<rows;i++) {
		// Property name
	    query.Format("GetPropertyName %d",i);
        sysQuery=query.AllocSysString();
        result=DataSourceAccessSync(&sysQuery);
        ::SysFreeString(sysQuery);
	    m_TaskSpreadsheet.SetCellText(i+1,0,result);
		// Property role
	    query.Format("GetPropertyRole %d",i);
        sysQuery=query.AllocSysString();
        result=DataSourceAccessSync(&sysQuery);
        ::SysFreeString(sysQuery);
	    m_TaskSpreadsheet.SetCellText(i+1,1,result);

		// Property base type
	    query.Format("GetPropBaseType %d",i);
        sysQuery=query.AllocSysString();
        bt=DataSourceAccessSync(&sysQuery);
        ::SysFreeString(sysQuery);

		// ENUMERATION
        if(bt == "enum") {
	        m_TaskSpreadsheet.SetCellType(i+1,2,3);
		    // Property initial value
	        query.Format("GetPropertyValue %d",i);
            sysQuery=query.AllocSysString();
            result=DataSourceAccessSync(&sysQuery);
            ::SysFreeString(sysQuery);
	        m_TaskSpreadsheet.SetCellText(i+1,2,result);
	        // Getting menu items amount
	        query.Format("GetPropertySetSize %d",i);
            sysQuery=query.AllocSysString();
            result=DataSourceAccessSync(&sysQuery);
            ::SysFreeString(sysQuery);
	        int items=atoi(result);
			// Filling property menu items
			for(register j=0;j<items;j++) {
		       // Getting menu item
	           query.Format("GetPropertySetItem %d %d",i,j);
               sysQuery=query.AllocSysString();
               result=DataSourceAccessSync(&sysQuery);
               ::SysFreeString(sysQuery);
			   // Appending to the spreadsheet menu
	           m_TaskSpreadsheet.AddMenuItem(i+1,2,result);
			}
		} else {
		    // BOOLEAN
            if(bt == "bool") {
	            m_TaskSpreadsheet.SetCellType(i+1,2,2);
		        // Property initial value
	            query.Format("GetPropertyValue %d",i);
                sysQuery=query.AllocSysString();
                result=DataSourceAccessSync(&sysQuery);
	            m_TaskSpreadsheet.SetCellCheck(1,2,!(result=="0"));
			} else {
		        // SET
                if(bt == "set") {
		            // Property initial value
	                query.Format("GetPropertyValue %d",i);
                    sysQuery=query.AllocSysString();
                    result=DataSourceAccessSync(&sysQuery);
	                m_TaskSpreadsheet.SetCellText(i+1,2,result);
				} else {
	                m_TaskSpreadsheet.SetCellType(i+1,2,1);
		            // Property initial value
	                query.Format("GetPropertyValue %d",i);
                    sysQuery=query.AllocSysString();
                    result=DataSourceAccessSync(&sysQuery);
	                m_TaskSpreadsheet.SetCellText(i+1,2,result);
				}
			}
		}
	}
	query="CloseProperties";
	m_TaskSpreadsheet.SetEnableDraw(TRUE);
    sysQuery=query.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// This function will fill combobox with all awailable tasks and will select the first 
// one.
//----------------------------------------------------------------------------------------
void CTaskFlow::FillSequencesSelector() {
register i;
CString query;
BSTR sysQuery;
CString results;
CString name;

   
   query="wftask info";
   sysQuery=query.AllocSysString();
   results = DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   m_SequenceSelector.ResetContent();
   if(results.GetLength()>0) {
       name="";
       int pos=0;
       for(i=0;i<=results.GetLength();i++) {
	       if(i==results.GetLength() || results[i]==' ') {
		       m_SequenceSelector.AddString(name);
		       name="";
		       continue;
		   }
	       name+=results[i];
	   }
       m_SequenceSelector.SetCurSel(0);  
   }
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This private function will fill task flow quering server for the items in the current
// task flow.
//----------------------------------------------------------------------------------------
void CTaskFlow::FillTaskFlow() {
register i;
CString query;
BSTR sysQuery;
CString results;
CString task;
TC_ITEM item;

   m_TaskFlow.DeleteAllItems();
   // If no elements selected
   if(m_SequenceSelector.GetCurSel()==-1) return;
 
   CString name;
   m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),name);
   query.Format("wftask print {%s}",name);
   sysQuery=query.AllocSysString();
   results = DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   item.mask         = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
   item.cchTextMax   = 20;     
   task="";
   int pos=0;
   int index=0;
   if(results.GetLength()!=0) {
       for(i=0;i<=results.GetLength();i++) {
	       if(i==results.GetLength() || results[i]==' ') {
               item.pszText      = (char *)(LPCSTR)task;
		       if(task=="Filter") {
			       item.iImage       = 0;
		           item.lParam       = 0;     
	               m_TaskFlow.InsertItem(m_TaskFlow.GetItemCount(),&item);
			       task="";
			       continue;
			   }
		       if(task=="Assignments") {
			       item.iImage       = 1;
		           item.lParam       = 1;     
	               m_TaskFlow.InsertItem(m_TaskFlow.GetItemCount(),&item);
			       task="";
			       continue;
			   }
		       if(task=="Query") {
			       item.iImage       = 2;
		           item.lParam       = 2;     
	               m_TaskFlow.InsertItem(m_TaskFlow.GetItemCount(),&item);
			       task="";
			       continue;
			   }
		       if(task=="TpmDiff") {
			       item.iImage       = 3;
		           item.lParam       = 3;     
	               m_TaskFlow.InsertItem(m_TaskFlow.GetItemCount(),&item);
			       task="";
			       continue;
			   }
		       if(task=="Collect") {
			       item.iImage       = 4;
		           item.lParam       = 4;     
	               m_TaskFlow.InsertItem(m_TaskFlow.GetItemCount(),&item);
			       task="";
			       continue;
			   }
		       if(task=="Instances") {
			       item.iImage       = 5;
		           item.lParam       = 5;     
	               m_TaskFlow.InsertItem(m_TaskFlow.GetItemCount(),&item);
			       task="";
			       continue;
			   }
		       if(task=="Pattern") {
			       item.iImage       = 6;
		           item.lParam       = 6;     
	               m_TaskFlow.InsertItem(m_TaskFlow.GetItemCount(),&item);
			       task="";
			       continue;
			   }
		       if(task=="Report") {
			       item.iImage       = 7;
		           item.lParam       = 7;     
	               m_TaskFlow.InsertItem(m_TaskFlow.GetItemCount(),&item);
			       task="";
			       continue;
			   }
		       item.iImage       = 8;
		       item.lParam       = 8;     
	           m_TaskFlow.InsertItem(m_TaskFlow.GetItemCount(),&item);
		       task="";
		       continue;
		   }
	       task+=results[i];
	   }
	   FillProperties();
	   m_TaskSpreadsheet.ShowWindow(SW_SHOW);
   } else m_TaskSpreadsheet.ShowWindow(SW_HIDE);
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This callback will run every time the user change currently selected task.
// It will fill task flow bar with the tasks from the new task sequence.
//----------------------------------------------------------------------------------------
void CTaskFlow::OnSelchangeSequenceSelector()  {
	if(!m_Running) FillTaskFlow();
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Call every time the user switch to the different node. We need to fill new node 
// properties.
//----------------------------------------------------------------------------------------
void CTaskFlow::OnSelchangeNode(NMHDR* pNMHDR, LRESULT* pResult)  {
	if(!m_Running) FillProperties();
	*pResult = 0;
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Need to unregister this window as a drop target.
//----------------------------------------------------------------------------------------
void CTaskFlow::OnClose()  {
	m_DropTarget.Revoke();
	CFormView::OnClose();
}
//----------------------------------------------------------------------------------------


int CTaskFlow::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here

	return 0;
}

//----------------------------------------------------------------------------------------
// This public function is used by DragAndDrop to insert new items at the position 
// specified. "size" show the number of items, elements are integer array with items 
// indexes, point is the screen coordinates point to insert new items.
//----------------------------------------------------------------------------------------
void CTaskFlow::InsertItems(int size, int* elements, CPoint& point) {
TC_HITTESTINFO test;
int item;
POINT p;
    p.x=point.x;
    p.y=point.y;
    test.pt.x=p.x;
    test.pt.y=p.y;
    test.flags=TCHT_NOWHERE;
    item=m_TaskFlow.HitTest(&test);
    if(item<0) item=m_TaskFlow.GetItemCount();
    for(int i=0;i<size;i++) InsertNewNode(item+i,elements[i]);
	m_TaskFlow.SetCurSel(item);
	FillProperties();
}
//-----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This public function is used by DragandDrop to move nodes in the taskflow;
//----------------------------------------------------------------------------------------
void CTaskFlow::MoveItems(int size, int * elements, CPoint & point) {
register i;
TC_HITTESTINFO test;
int item;
POINT p;
TC_ITEM node;
char text[30];
    p.x=point.x;
    p.y=point.y;
    test.pt.x=p.x;
    test.pt.y=p.y;
    test.flags=TCHT_NOWHERE;
    item=m_TaskFlow.HitTest(&test);
    if(item<0) item=m_TaskFlow.GetItemCount();
    for(i=0;i<size;i++) {
       node.mask         = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
	   node.pszText=text;
	   node.cchTextMax   = 20;
	   m_TaskFlow.GetItem(elements[i],&node);
	   ServerMoveNode(elements[i],item+i);
	   m_TaskFlow.DeleteItem(elements[i]);
	   m_TaskFlow.InsertItem(item+i,&node);
	}
}
//----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Inserts the new node at the specified position,
//-----------------------------------------------------------------------------------------
void CTaskFlow::InsertNewNode(int atPos, int id) {
CString query;
CString name;
CString result;
BSTR sysQuery;
TC_ITEM item;

   if(atPos==-1) atPos=m_TaskFlow.GetItemCount();
   query="wfnode info";
   sysQuery=query.AllocSysString();
   result = DataSourceAccessSync(&sysQuery);
   SysFreeString(sysQuery);

   item.mask         = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
   item.cchTextMax   = 20;
   item.lParam       = id;
   if(id>8) item.iImage=8;
   else     item.iImage=id;
   if(result.GetLength()!=0) {
      name="";
	  int index=0;
      for(int i=0;i<=result.GetLength();i++) {
	      if(i==result.GetLength() || result[i]==' ')  {
			  if(id==index) {
				  if(ServerAddNode(name,atPos-1)==1) {
			         item.pszText      = (char *)(LPCSTR)name;
	                 m_TaskFlow.InsertItem(atPos,&item);
	                 m_TaskFlow.SetCurSel(atPos);
                     FillProperties();
                     m_TaskSpreadsheet.ShowWindow(SW_SHOW);                  
				     m_TaskFlow.Invalidate();
				  }
				  return;
			  }
			  name="";
			  index++;
		  }
		  name+=result[i];
	  }
   }
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This function will remove node at the spesified position
//-----------------------------------------------------------------------------------------
void CTaskFlow::RemoveNode(int pos) {
	ServerRemoveNode(pos);
    m_TaskFlow.DeleteItem(pos);
	m_TaskFlow.Invalidate();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// We need this to analyse the keyboard events and find "Del" key to remove current node.
//-----------------------------------------------------------------------------------------
void CTaskFlow::OnKeydownTaskFlow(NMHDR* pNMHDR, LRESULT* pResult)  {
	TC_KEYDOWN* pTCKeyDown = (TC_KEYDOWN*)pNMHDR;
	// If Del key pressed
	if(pTCKeyDown->wVKey==46) {
	   int pos = m_TaskFlow.GetCurSel();
	   RemoveNode(pos);
	   if(m_TaskFlow.GetItemCount()>pos) m_TaskFlow.SetCurSel(pos);
	   else 
		   if(m_TaskFlow.GetItemCount()>0) m_TaskFlow.SetCurSel(pos-1);
	   FillProperties();
	}
	*pResult = 0;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Private function will query server to add the new node with the tag "Name" to the 
// taskflow at the position "pos".
//-----------------------------------------------------------------------------------------
int CTaskFlow::ServerAddNode(CString name, int pos) {
CString query;
BSTR sysQuery;

   if(m_SequenceSelector.GetCurSel()>=0) {
      CString taskName;
      m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),taskName);
      query.Format("wftask addnode {%s} %d %s",taskName,pos,name);
      sysQuery=query.AllocSysString();
      DataSourceAccessSync(&sysQuery);
      SysFreeString(sysQuery);

      query.Format("wftask save {%s}",taskName);
      sysQuery=query.AllocSysString();
      DataSourceAccessSync(&sysQuery);
      SysFreeString(sysQuery);
	  return 1;
   } else {
	  AfxMessageBox("Please create taskflow first.");
	  return 0;
   }
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Calls server to delete node at the position "pos"
//-----------------------------------------------------------------------------------------
void CTaskFlow::ServerRemoveNode(int pos) {
CString query;
BSTR sysQuery;


   CString taskName;
   m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),taskName);
   query.Format("wftask delnode {%s} %d" ,taskName,pos);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   SysFreeString(sysQuery);

   query.Format("wftask save {%s}",taskName);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   SysFreeString(sysQuery);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Calls server to move node from position "from" to position "to"
//-----------------------------------------------------------------------------------------
void CTaskFlow::ServerMoveNode(int from, int to) { 
CString query;
BSTR sysQuery;


   CString taskName;
   m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),taskName);
   query.Format("wftask move {%s} %d %d",taskName,from, to);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   SysFreeString(sysQuery);
   query.Format("wftask save {%s}",taskName);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   SysFreeString(sysQuery);

}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This callback will run every time the user press New button.
// It will create a new empty tasks sequence by calling server and it will insert the
// name of the group into selector combobox (the server will return you transformed name-
// all illegal characters will be removed)
//-----------------------------------------------------------------------------------------
void CTaskFlow::OnNewTaskFlow()  {
CTaskNewDlg dlg;
CString query;
BSTR sysQuery;
CString result;

	dlg.m_Name="";
	if(dlg.DoModal()==IDOK) {
       query.Format("wftask exists {%s}",dlg.m_Name);
       sysQuery=query.AllocSysString();
       result=DataSourceAccessSync(&sysQuery);
       SysFreeString(sysQuery);
	   if(result=="1") {
		   CString text;
		   text.Format("The name \"%s\" is already in use.\nPlease select different name.",dlg.m_Name);
		   MessageBox(text,"Task creation error",MB_ICONEXCLAMATION);
		   return;
	   }		
       query.Format("wftask new {%s}",dlg.m_Name);
       sysQuery=query.AllocSysString();
       result=DataSourceAccessSync(&sysQuery);
       SysFreeString(sysQuery);
	   m_SequenceSelector.SetCurSel(m_SequenceSelector.AddString(result));

       query.Format("wftask save {%s}",dlg.m_Name);
       sysQuery=query.AllocSysString();
       DataSourceAccessSync(&sysQuery);
       SysFreeString(sysQuery);

       FillTaskFlow();
	}
	
}
//-----------------------------------------------------------------------------------------


BEGIN_EVENTSINK_MAP(CTaskFlow, CFormView)
    //{{AFX_EVENTSINK_MAP(CTaskFlow)
	ON_EVENT(CTaskFlow, IDC_TASKSPREADSHEET, 1 /* BoolChanged */, OnBoolChanged, VTS_I4 VTS_I4 VTS_BSTR)
	ON_EVENT(CTaskFlow, IDC_TASKSPREADSHEET, 2 /* StringChanged */, OnStringChanged, VTS_I4 VTS_I4 VTS_BSTR)
	ON_EVENT(CTaskFlow, IDC_TASKSPREADSHEET, 3 /* SetChanged */, OnSetChanged, VTS_I4 VTS_I4 VTS_BSTR)
	ON_EVENT(CTaskFlow, IDC_TASKSPREADSHEET, 4 /* TextDoubleclicked */, OnTextDoubleclicked, VTS_I4 VTS_I4 VTS_BSTR)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

//-----------------------------------------------------------------------------------------
// Spreadsheet event processing in the case the set field in the spreadsheet changed.
//-----------------------------------------------------------------------------------------
void CTaskFlow::OnSetChanged(long row, long col, LPCTSTR value)  {
CString query;
BSTR sysQuery;
CString result;

   CString taskName;
   m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),taskName);

   // Initializing task flow session ...
   query.Format("InitProperties {%s} %d",taskName,m_TaskFlow.GetCurSel());
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   // Quering the server name of the property in the current row ...
   query.Format("GetPropertyServerName %d",row-1);
   sysQuery=query.AllocSysString();
   result=DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   // Setting the property value
   query.Format("wftask setparam {%s} -n %d -p {%s} -v {%s}",taskName,m_TaskFlow.GetCurSel(),result,value);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   // Closing task flow session ...
   query="CloseProperties";
   m_TaskSpreadsheet.SetEnableDraw(TRUE);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Spreadsheet event processing in the case the editable field in the spreadsheet changed.
//-----------------------------------------------------------------------------------------
void CTaskFlow::OnStringChanged(long row, long col, LPCTSTR value)  {
CString query;
BSTR sysQuery;
CString result;

   CString taskName;
   m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),taskName);

   // Initializing task flow session ...
   query.Format("InitProperties {%s} %d",taskName,m_TaskFlow.GetCurSel());
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   // Quering the server name of the property in the current row ...
   query.Format("GetPropertyServerName %d",row-1);
   sysQuery=query.AllocSysString();
   result=DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   // Setting the property value
   query.Format("wftask setparam {%s} -n %d -p {%s} -v {%s}",taskName,m_TaskFlow.GetCurSel(),result,value);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   // Closing task flow session ...
   query="CloseProperties";
   m_TaskSpreadsheet.SetEnableDraw(TRUE);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Spreadsheet event processing in the case the boolean field in the spreadsheet changed.
//-----------------------------------------------------------------------------------------
void CTaskFlow::OnBoolChanged(long row, long col, LPCTSTR value)  {
CString query;
BSTR sysQuery;
CString result;

   CString taskName;
   m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),taskName);

   // Initializing task flow session ...
   query.Format("InitProperties {%s} %d",taskName,m_TaskFlow.GetCurSel());
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   // Quering the server name of the property in the current row ...
   query.Format("GetPropertyServerName %d",row-1);
   sysQuery=query.AllocSysString();
   result=DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   // Setting the property value
   query.Format("wftask setparam {%s} -n %d -p {%s} -v {%s}",taskName,m_TaskFlow.GetCurSel(),result,value);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);

   // Closing task flow session ...
   query="CloseProperties";
   m_TaskSpreadsheet.SetEnableDraw(TRUE);
   sysQuery=query.AllocSysString();
   DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This private function will run the selected task sequence.
//-----------------------------------------------------------------------------------------
void CTaskFlow::RunFlow() {
register i;
CString query;
BSTR sysQuery;
CString result;

   if(m_SequenceSelector.GetCurSel()<0) return;
   m_Running = TRUE;
   CString taskName;
   m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),taskName);
   query.Format("wftask validate {%s}",taskName);
   sysQuery=query.AllocSysString();
   result=DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);
   if(result=="1") {
	  for(i=0;i<m_TaskFlow.GetItemCount();i++) {
         m_TaskFlow.SetCurSel(i);
         FillProperties();
		 query.Format("wftask run {%s} -from %d -n 1",taskName,i);
         sysQuery=query.AllocSysString();
         result=DataSourceAccessSync(&sysQuery);
         ::SysFreeString(sysQuery);
		 FillProperties();
		 if(result.GetLength()!=0) {
			 AfxMessageBox(result);
			 break;
		 }
		 if(m_StopQuery==TRUE) {
			 m_StopQuery=FALSE;
			 break;
		 }
	  }
   } else {
	   CString text;
	   text.Format("Can't execute task \"%s\".\n Check the order of the nodes.",taskName);
	   MessageBox(text,"Task execution error",MB_ICONEXCLAMATION);
   }
   CMainFrame* main;
   main=(CMainFrame *) AfxGetMainWnd();
   main->m_wndBrowser.PostMessage(UM_REFRESH_FILE_LIST,0,0);
   m_Running = FALSE;

}
//-----------------------------------------------------------------------------------------

void CTaskFlow::StopFlow() {
	m_StopQuery=TRUE;

}

void CTaskFlow::StepFlow() {
CString query;
BSTR sysQuery;
CString result;
CString taskName;

   if(m_SequenceSelector.GetCurSel()<0) return;
   m_Running = TRUE;
   if(m_TaskFlow.GetCurSel()>=m_TaskFlow.GetItemCount()) return;
   m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),taskName);
   query.Format("wftask validate {%s}",taskName);
   sysQuery=query.AllocSysString();
   result=DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);
   if(result=="1") {
	  query.Format("wftask run {%s} -from %d -n 1",taskName,m_TaskFlow.GetCurSel());
      sysQuery=query.AllocSysString();
      result=DataSourceAccessSync(&sysQuery);
      ::SysFreeString(sysQuery);
	  if(result.GetLength()!=0) {
		 AfxMessageBox(result);
	  }
      if(m_TaskFlow.GetCurSel()>=m_TaskFlow.GetItemCount()-1) 
          m_TaskFlow.SetCurSel(0);
	  else
          m_TaskFlow.SetCurSel(m_TaskFlow.GetCurSel()+1);         
   } else {
	   CString text;
	   text.Format("Can't execute task \"%s\".\n Check the order of the nodes.",taskName);
	   MessageBox(text,"Task execution error",MB_ICONEXCLAMATION);
   }
   CMainFrame* main;
   main=(CMainFrame *) AfxGetMainWnd();
   main->m_wndBrowser.PostMessage(UM_REFRESH_FILE_LIST,0,0);
   m_Running = FALSE;

}

void CTaskFlow::ContinueFlow() {
register i;
CString query;
BSTR sysQuery;
CString result;
   CString taskName;
   m_SequenceSelector.GetLBText(m_SequenceSelector.GetCurSel(),taskName);
   query.Format("wftask validate {%s}",taskName);
   sysQuery=query.AllocSysString();
   result=DataSourceAccessSync(&sysQuery);
   ::SysFreeString(sysQuery);
   if(result=="1") {
	  for(i=m_TaskFlow.GetCurSel();i<m_TaskFlow.GetItemCount();i++) {
         m_TaskFlow.SetCurSel(i);         
		 query.Format("wftask run {%s} -from %d -n 1",taskName,i);
         sysQuery=query.AllocSysString();
         result=DataSourceAccessSync(&sysQuery);
         ::SysFreeString(sysQuery);
		 if(result.GetLength()!=0) {
			 AfxMessageBox(result);
			 break;
		 }
		 if(m_StopQuery==TRUE) {
			 m_StopQuery=FALSE;
			 break;
		 }
	  }
   } else {
	   CString text;
	   text.Format("Can't execute task \"%s\".\n Check the order of the nodes.",taskName);
	   MessageBox(text,"Task execution error",MB_ICONEXCLAMATION);
   }

}

void CTaskFlow::Update() {
CString init   = "proc InitProperties {name number} { global __PropertiesList; set __PropertiesList [wftask dump $name $number]; set __PropertiesList [lindex $__PropertiesList 0]; set __PropertiesList [lrange $__PropertiesList 1 999]; }";
CString amount = "proc GetPropertiesAmount   {        } {global __PropertiesList; return [llength $__PropertiesList];}";
CString name   = "proc GetPropertyName       { number } { global __PropertiesList; set temp [lindex $__PropertiesList $number];return [wfparam describe [lindex $temp 0]];}";
CString sname  = "proc GetPropertyServerName { number } { global __PropertiesList; set temp [lindex $__PropertiesList $number];return [lindex $temp 0];}";
CString role   = "proc GetPropertyRole       { number } { global __PropertiesList; set temp [lindex $__PropertiesList $number];return [lindex $temp 1];}";
CString type   = "proc GetPropertyType       { number } { global __PropertiesList; set temp [lindex $__PropertiesList $number];return [lindex $temp 2];}";
CString base   = "proc GetPropBaseType       { number } { global __PropertiesList; set temp [lindex $__PropertiesList $number];set full_type [lindex $temp 3]; return [lindex $full_type 0];}";
CString value  = "proc GetPropertyValue      { number } { global __PropertiesList;set temp [lindex $__PropertiesList $number];return [lindex $temp 5];}";
CString setsize= "proc GetPropertySetSize    { number } { global __PropertiesList;set temp [lindex $__PropertiesList $number];set full_type [lindex $temp 3]; return [llength [lindex $full_type 1]];}";
CString setitem= "proc GetPropertySetItem    { number item } { global __PropertiesList;set temp [lindex $__PropertiesList $number];set full_type [lindex $temp 3]; return [lindex [lindex $full_type 1] $item];}";
CString close  = "proc CloseProperties       {             } { unset __PropertiesList; }";
BSTR sysQuery;

    // register user-defined TCL procedures
    sysQuery=init.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=amount.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=name.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=sname.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=role.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=base.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=type.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=value.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=setsize.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=setitem.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);

    sysQuery=close.AllocSysString();
    DataSourceAccessSync(&sysQuery);
    ::SysFreeString(sysQuery);
	//---------------------------------

    FillSequencesSelector();
    FillTaskFlow();
}


//-------------------------------------------------------------------------------------------------------
// This callback will run every time the user doubleclicked the cell
//-------------------------------------------------------------------------------------------------------
void CTaskFlow::OnTextDoubleclicked(long row, long col, LPCTSTR text)  {
CMainFrame* main;

    main=(CMainFrame *) AfxGetMainWnd();
	if(main) {
		if(strnicmp(text,"_DI_",4)==0) {
		   main->m_wndBrowser.BrowseGivenSet(CString(text),1);
		}
	}
	
}
//-------------------------------------------------------------------------------------------------------
// This function check communications status after access datasource.
//------------------------------------------------------------------------------------------
CString CTaskFlow::DataSourceAccessSync(BSTR* command) {
	CString res;
	static BOOL bCommDlgShown = FALSE;
	res = m_DataSource.AccessSync(command);
	if (m_DataSource.IsConnectionLost() == TRUE &&
		bCommDlgShown != TRUE) {
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
