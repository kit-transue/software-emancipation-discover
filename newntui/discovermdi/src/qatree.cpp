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
// QATree.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "QATree.h"
#include "QAParams.h"
#include "QAChildFrame.h"
#include "SevOpenDlg.h"
#include "SevSaveDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int inSubTree=0;

/////////////////////////////////////////////////////////////////////////////
// CQATree

IMPLEMENT_DYNCREATE(CQATree, CFormView)

CQATree::CQATree()
	: CFormView(CQATree::IDD)
{
	//{{AFX_DATA_INIT(CQATree)
	//}}AFX_DATA_INIT
	m_bLicensed = false;
}

CQATree::~CQATree() {
}

void CQATree::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQATree)
	DDX_Control(pDX, IDC_QATREETITLE, m_Title);
	DDX_Control(pDX, IDC_QATREE, m_Tree);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQATree, CFormView)
	//{{AFX_MSG_MAP(CQATree)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_QATREE, OnClickQatree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_QATREE, OnSelchanged)
	ON_NOTIFY(TVN_KEYDOWN, IDC_QATREE, OnKeydownQatree)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQATree diagnostics

#ifdef _DEBUG
void CQATree::AssertValid() const
{
	CFormView::AssertValid();
}

void CQATree::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CQATree message handlers


//---------------------------------------------------------------------------------------
// This callback will run every time the window created/changed size. We
// need it to position all controls on our page.
//---------------------------------------------------------------------------------------
void CQATree::OnSize(UINT nType, int cx, int cy)  {
	CFormView::OnSize(nType, cx, cy);
	LayoutControls(cx,cy);	
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This private function will check if controls is already available
// and will position them in appropriate places.
//---------------------------------------------------------------------------------------
void CQATree::LayoutControls(int cx, int cy) {
int titleHeight=0;
   
	if(::IsWindow(m_Title)) {
		CRect titleRect;
		m_Title.GetWindowRect(titleRect);
		titleHeight=titleRect.Height();
		m_Title.MoveWindow(0,0,cx,titleHeight);
	}
	if(::IsWindow(m_Tree)) {
		m_Tree.MoveWindow(0,titleHeight,cx,cy-titleHeight);
	}
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This callback will run when all windows are created but still 
// invisible.
//---------------------------------------------------------------------------------------
void CQATree::OnInitialUpdate()  {
CString query;
BSTR sysQuery;

	CFormView::OnInitialUpdate();
    SIZE sizeTotal;
    sizeTotal.cx=0;
    sizeTotal.cy=0;
    SetScrollSizes( MM_TEXT, sizeTotal);

	CBitmap folder_opened;
	folder_opened.LoadBitmap(IDB_FOLDER_OPENED);
	CBitmap folder_closed;
	folder_closed.LoadBitmap(IDB_FOLDER_CLOSED);

	CBitmap query1open;
	query1open.LoadBitmap(IDB_QUERY1_OPEN);
	CBitmap query1close;
	query1close.LoadBitmap(IDB_QUERY1_CLOSE);
	CBitmap query2open;
	query2open.LoadBitmap(IDB_QUERY2_OPEN);
	CBitmap query2close;
	query2close.LoadBitmap(IDB_QUERY2_CLOSE);

	CBitmap folder_opened_dis;
	folder_opened_dis.LoadBitmap(IDB_FOLDER_OPENED_DIS);
	CBitmap folder_closed_dis;
	folder_closed_dis.LoadBitmap(IDB_FOLDER_CLOSED_DIS);

	CBitmap query_open_dis;
	query_open_dis.LoadBitmap(IDB_QUERY_OPEN_DIS);

	CBitmap query_close_dis;
	query_close_dis.LoadBitmap(IDB_QUERY_CLOSE_DIS);

    m_ImageList.Create(24,16,TRUE,7,1);

	m_ImageList.Add(&folder_opened,    GetSysColor(COLOR_BTNFACE));
	m_ImageList.Add(&folder_closed,    GetSysColor(COLOR_BTNFACE));
	m_ImageList.Add(&query1open,       GetSysColor(COLOR_BTNFACE));
	m_ImageList.Add(&query1close,      GetSysColor(COLOR_BTNFACE));
	m_ImageList.Add(&query2open,       GetSysColor(COLOR_BTNFACE));
	m_ImageList.Add(&query2close,      GetSysColor(COLOR_BTNFACE));
	m_ImageList.Add(&folder_opened_dis,GetSysColor(COLOR_BTNFACE));
	m_ImageList.Add(&folder_closed_dis,GetSysColor(COLOR_BTNFACE));
	m_ImageList.Add(&query_open_dis,   GetSysColor(COLOR_BTNFACE));
	m_ImageList.Add(&query_close_dis,  GetSysColor(COLOR_BTNFACE));

	m_Tree.SetImageList(&m_ImageList,TVSIL_NORMAL); 

    query="license VI3 co";
    sysQuery=query.AllocSysString();
    CString res = DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery);
	if(res!="1") {
		MessageBox("No Quality license is available.","Error",MB_OK|MB_ICONERROR);
		CFrameWnd* pParent = this->GetParentFrame();
		pParent->PostMessage(WM_CLOSE);
	} else
		m_bLicensed = true;

    // This will fill "tmp" TCL list with all childeren internal names.
    query="sev_init";
    sysQuery=query.AllocSysString();
    DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery);

    query="source_dis sel2sym.dis";
    sysQuery=query.AllocSysString();
    DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery);
	FillTree();
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This protected function designed to fill the complete QA tree. It's only task is to
// run the recursive function AddChildern setting the parent node to the root.
//---------------------------------------------------------------------------------------
void CQATree::FillTree() {
    AddChildren(TVI_ROOT,CString("/"));
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This internal function designed to query the current node and to 
// receve it's complete child list. If node has children the child list
// will be the string with child names separated by "\n", if node has no
// childeren the result string will be empty. The node you are asking for
// the children must be defined as a string in the "from" parameter and
// must be the complete path to the tree node starting with "/" and 
// separated by "/".
// NOTE: All childs will be returned in it's internal names. Use sev_get
// with "name" parameter to convert it into readable name.
//---------------------------------------------------------------------------------------
int CQATree::QueryChildren(CString & from, CString & list) {
register i;
CString query;
CString result;
BSTR sysQuery;
int amount;

    // This will fill "tmp" TCL list with all childeren internal names.
    query.Format("set tmp [sev_get {%s} children]",from);
    sysQuery=query.AllocSysString();
    DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery);

    // This will return the number of the elements in the list
    query="llength $tmp";
    sysQuery=query.AllocSysString();
    result=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery);

	list="";
	amount=atoi(result);
	for(i=0;i<amount;i++) {
        query.Format("lindex $tmp %d",i);
        sysQuery=query.AllocSysString();
        result=DataSourceAccessSync(&sysQuery);
	    SysFreeString(sysQuery);
		list+=result;
		list+="\n";
	}
	return amount;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This recursive function will add complete sub-tree to the node "to"
// using the path "path"
//---------------------------------------------------------------------------------------
void CQATree::AddChildren(HTREEITEM to, CString& path,BOOL folderStatus) {
register i;
CString query;
CString status;
CString name;
CString type;
BSTR sysQuery;
int amount;
CString childList;
CString childPath;

     // Query the server for the existing nodes
     amount=QueryChildren(path,childList);
	 // If this node has no children -  exit.
	 if(amount==0) return;
	 CString token;
	 for(i=0;i<childList.GetLength();i++) {
		 if(childList[i]=='\n') {

			  if(to!=TVI_ROOT)
                 childPath=path+"/"+token;
			  else
                 childPath=path+token;

			  // Getting the type of the node
              query.Format("sev_get {%s} type",childPath);
              sysQuery=query.AllocSysString();
              type=DataSourceAccessSync(&sysQuery);
	          SysFreeString(sysQuery); 

			  // Getting the node status
              query.Format("sev_get {%s} on",childPath);
              sysQuery=query.AllocSysString();
              status=DataSourceAccessSync(&sysQuery);
	          SysFreeString(sysQuery); 

			  // Getting the node name
              query.Format("sev_get {%s} name",childPath);
              sysQuery=query.AllocSysString();
              name=DataSourceAccessSync(&sysQuery);
	          SysFreeString(sysQuery); 
			  if(name[0]!='.') {


			  //------- Inserting item into the tree --------------------
              TV_ITEM tvItem;

			  // New node info structure will be added to the UI node
			  // to keep information about real name and folder status
			  CTreeNodeInfo* info;
			  info=new CTreeNodeInfo;
			  info->m_NodeName=childPath;
			  info->m_NodeType=type;
			  info->m_RootEnabled=folderStatus;


	          tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
              tvItem.pszText        = (char *)(LPCSTR)name;
	          tvItem.cchTextMax     = name.GetLength();
			  tvItem.lParam         = (long)info;
            
              // If the tree node is of type "Folder" we will show it
			  // depending on the current folder status
			  if(type=="folder") {
				  if(status=="1" && folderStatus==TRUE) {
	                 tvItem.iImage         = 1;
	                 tvItem.iSelectedImage = 0;
				  } else {
	                 tvItem.iImage         = 7;
	                 tvItem.iSelectedImage = 6;
				  }
			  }
              // If the tree node is of type "Query 0" we will show it
			  // depending on it's status
			  if(type=="query_0") {
				  if(status=="1" && folderStatus==TRUE) {
	                 tvItem.iImage         = 3;
	                 tvItem.iSelectedImage = 2;
				  } else  {
	                 tvItem.iImage         = 9;
	                 tvItem.iSelectedImage = 8;
				  }
			  }

              // If the tree node is of type "Query 1" we will show it
			  // depending on it's status
			  if(type=="query_1") {
				  if(status=="1" && folderStatus==TRUE) {
	                 tvItem.iImage         = 5;
	                 tvItem.iSelectedImage = 4;
				  } else {
	                 tvItem.iImage         = 9;
	                 tvItem.iSelectedImage = 8;
				  }
			  }

 	          TV_INSERTSTRUCT tvInsert;
	          tvInsert.hParent      = to;
	          tvInsert.hInsertAfter = TVI_LAST;
	          tvInsert.item         = tvItem;

	          HTREEITEM newItem= m_Tree.InsertItem(&tvInsert);

	          //---------------------------------------------------------
			  if(type=="folder") 
				  AddChildren(newItem,childPath, (status=="1" && folderStatus==TRUE) );
			  }
			  token="";
			  continue;
		 }
		 token+=childList[i];
	 }
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This private recursive function was designed to reflect the parent status change on 
// all it's children. If you will disable the parent, all sub-tree attached to the 
// parent will be also disabled and you will be unsbale to switch sub-tree items status
// while parent is disabled. If you enable the parent status, all sub-tree will return
// to it's initial status and you will be able to switch the sub-tree items status.
//---------------------------------------------------------------------------------------
void CQATree::PropagateStatus(HTREEITEM node, BOOL newStatus) {
CTreeNodeInfo* info;
CString query;
CString status;
BSTR    sysQuery;

    // We need a pointer to the user structure attached to the UI element.
 	info=(CTreeNodeInfo *)m_Tree.GetItemData(node);

	// This node will contain it's parent status.
	info->m_RootEnabled=newStatus;

    // Getting the node status
    query.Format("sev_get {%s} on",info->m_NodeName);
    sysQuery=query.AllocSysString();
    status=DataSourceAccessSync(&sysQuery);
    SysFreeString(sysQuery); 
	
	// Setting the new status according to the parent status and 
	// the current node status.
	if(info->m_NodeType=="folder") {
	    if(status=="1" && newStatus==TRUE) m_Tree.SetItemImage(node,1,0);
		else                               m_Tree.SetItemImage(node,7,6);
	}
	if(info->m_NodeType=="query_0") {
	    if(status=="1" && newStatus==TRUE) m_Tree.SetItemImage(node,3,2);
		else                               m_Tree.SetItemImage(node,9,8);
	}
	if(info->m_NodeType=="query_1") {
	    if(status=="1" && newStatus==TRUE) m_Tree.SetItemImage(node,5,4);
		else                               m_Tree.SetItemImage(node,9,8);
	}
    HTREEITEM child   = m_Tree.GetChildItem(node);
	if(child!=NULL)     PropagateStatus(child,(newStatus && status=="1"));
    HTREEITEM sibling = m_Tree.GetNextSiblingItem(node);
	if(sibling!=NULL)  PropagateStatus(sibling,newStatus);

}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This private recursive function was designed to remove all user structures.
//---------------------------------------------------------------------------------------
void CQATree::Free(HTREEITEM node) {
CTreeNodeInfo* info;
    if(node==NULL) return;
    // We need a pointer to the user structure attached to the UI element.
 	info=(CTreeNodeInfo *)m_Tree.GetItemData(node);
    HTREEITEM child   = m_Tree.GetChildItem(node);
	if(child!=NULL)     Free(child);
    HTREEITEM sibling = m_Tree.GetNextSiblingItem(node);
	if(sibling!=NULL)   Free(sibling);
	m_Tree.SetItemData(node,NULL);
	delete info;

}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This callback runs every time the user clicked the mouse left button inside the tree
// control (CTreeControl sends us nofification message in the form WM_NOTIFY). We will
// use this event to change node status, if the click was done inside the status box.
// The node status can be changed if this node has no parent with the status set to off,
// this mean that turning off folder state will turn off all it's elements and it will
// be impossible to switch the status of the elements while the folder is off.
//---------------------------------------------------------------------------------------
void CQATree::OnClickQatree(NMHDR* pNMHDR, LRESULT* pResult)  {
POINT curPos;
UINT flags;
HTREEITEM item;
CTreeNodeInfo* info;
CString query;
CString status;
BSTR    sysQuery;

	GetCursorPos(&curPos);
	m_Tree.ScreenToClient(&curPos);
	item = m_Tree.HitTest(curPos,&flags);
	if(item!=NULL) {
		if( (flags&TVHT_ONITEMICON) !=0 ) {
 	         info=(CTreeNodeInfo *)m_Tree.GetItemData(item);
			 if(info->m_RootEnabled!=FALSE) {
                  // Getting the node status
                  query.Format("sev_get {%s} on",info->m_NodeName);
                  sysQuery=query.AllocSysString();
                  status=DataSourceAccessSync(&sysQuery);
                  SysFreeString(sysQuery)
					  ; 
				  // Switching the status
				  if(status=="1") status = "0";
			      else            status = "1";

                  // Setting the node status
                  query.Format("sev_set {%s} on %s",info->m_NodeName,status);
                  sysQuery=query.AllocSysString();
                  CString res = DataSourceAccessSync(&sysQuery);
                  SysFreeString(sysQuery); 
				 
	              // Setting the new status according to the parent status and 
	              // the current node status.
	              if(info->m_NodeType=="folder") {
	                 if(status=="1") m_Tree.SetItemImage(item,1,0);
		             else            m_Tree.SetItemImage(item,7,6);
				  }
	              if(info->m_NodeType=="query_0") {
	                 if(status=="1") m_Tree.SetItemImage(item,3,2);
		             else            m_Tree.SetItemImage(item,9,8);
				  }
	              if(info->m_NodeType=="query_1") {
	                 if(status=="1") m_Tree.SetItemImage(item,5,4);
		             else            m_Tree.SetItemImage(item,9,8);
				  }
				  HTREEITEM child=m_Tree.GetChildItem(item);
				  if(child!=NULL) PropagateStatus(child,status=="1");
                  CQAParams* view;
                  view=(CQAParams* )(((CSplitterWnd *)GetParent())->GetPane(0,1));
                  view->FillPropertiesOfNode(info->m_NodeName);
			 }
		}
	}
	*pResult = 0;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// This callback will run every time the user changed the selected item.
// We will use it to inform right window about the node which properties 
// we ned to see/edit.
//---------------------------------------------------------------------------------------
void CQATree::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)  {
    HTREEITEM current = m_Tree.GetSelectedItem();
	if(current!=NULL) {
          CTreeNodeInfo* info;
		  info = (CTreeNodeInfo*) m_Tree.GetItemData(current);
		  if(info!=NULL) {
             CQAParams* view;
             view=(CQAParams* )(((CSplitterWnd *)GetParent())->GetPane(0,1));
             view->FillPropertiesOfNode(info->m_NodeName);
			 CQAChildFrame* parent = (CQAChildFrame *)GetParent()->GetParent();
			 parent->SetCurrentSetEnable(view->GetCurrentSetEnable());
		  }
	}
	*pResult = 0;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This function will change the current item status.
//---------------------------------------------------------------------------------------
void CQATree::SetCurrentItemStatus(BOOL status) {
CTreeNodeInfo* info;
CString query;
BSTR    sysQuery;

    HTREEITEM item = m_Tree.GetSelectedItem();
	if(item!=NULL) {
 	    info=(CTreeNodeInfo *)m_Tree.GetItemData(item);
	    if(info->m_RootEnabled!=FALSE) {
			CString stStr;
			if(status==TRUE) stStr="1";
			else             stStr="0";
            // Setting the node status
            query.Format("sev_set {%s} on %s",info->m_NodeName,stStr);
            sysQuery=query.AllocSysString();
            CString res = DataSourceAccessSync(&sysQuery);
            SysFreeString(sysQuery); 
				 
	        // Setting the new status according to the parent status and 
	        // the current node status.
	        if(info->m_NodeType=="folder") {
	            if(stStr=="1") m_Tree.SetItemImage(item,1,0);
		        else            m_Tree.SetItemImage(item,7,6);
			}
	        if(info->m_NodeType=="query_0") {
	            if(stStr=="1") m_Tree.SetItemImage(item,3,2);
		        else            m_Tree.SetItemImage(item,9,8);
			}
	        if(info->m_NodeType=="query_1") {
	            if(stStr=="1") m_Tree.SetItemImage(item,5,4);
		        else            m_Tree.SetItemImage(item,9,8);
			}
			HTREEITEM child=m_Tree.GetChildItem(item);
			if(child!=NULL) PropagateStatus(child,stStr=="1");
		}
	}
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// This callback will run every time the user press a key while in the QA
// tree. We are using it to catch all space keys and to use them to change
// the item status. The default behavior - incremental search - turned in 
// this case off by setting *pResult = 1;
//---------------------------------------------------------------------------------------
void CQATree::OnKeydownQatree(NMHDR* pNMHDR, LRESULT* pResult)  {
TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
CTreeNodeInfo* info;
CString query;
CString status;
BSTR    sysQuery;
    
	*pResult = 0;
    HTREEITEM item = m_Tree.GetSelectedItem();
	if(item!=NULL) {
		if(pTVKeyDown->wVKey==32) {
 	         info=(CTreeNodeInfo *)m_Tree.GetItemData(item);
			 if(info->m_RootEnabled!=FALSE) {
                  // Getting the node status
                  query.Format("sev_get {%s} on",info->m_NodeName);
                  sysQuery=query.AllocSysString();
                  status=DataSourceAccessSync(&sysQuery);
                  SysFreeString(sysQuery)
					  ; 
				  // Switching the status
				  if(status=="1") status = "0";
			      else            status = "1";

                  // Setting the node status
                  query.Format("sev_set {%s} on %s",info->m_NodeName,status);
                  sysQuery=query.AllocSysString();
                  CString res = DataSourceAccessSync(&sysQuery);
                  SysFreeString(sysQuery); 
				 
	              // Setting the new status according to the parent status and 
	              // the current node status.
	              if(info->m_NodeType=="folder") {
	                 if(status=="1") m_Tree.SetItemImage(item,1,0);
		             else            m_Tree.SetItemImage(item,7,6);
				  }
	              if(info->m_NodeType=="query_0") {
	                 if(status=="1") m_Tree.SetItemImage(item,3,2);
		             else            m_Tree.SetItemImage(item,9,8);
				  }
	              if(info->m_NodeType=="query_1") {
	                 if(status=="1") m_Tree.SetItemImage(item,5,4);
		             else            m_Tree.SetItemImage(item,9,8);
				  }
				  HTREEITEM child=m_Tree.GetChildItem(item);
				  if(child!=NULL) PropagateStatus(child,status=="1");
                  CQAParams* view;
                  view=(CQAParams* )(((CSplitterWnd *)GetParent())->GetPane(0,1));
                  view->FillPropertiesOfNode(info->m_NodeName);
	             *pResult = 1;
			 }
		}
	}
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
void CQATree::OnClose()  {
	if(m_bLicensed) {
		CString query="license VI3 ci";
		BSTR sysQuery=query.AllocSysString();
		DataSourceAccessSync(&sysQuery);
		SysFreeString(sysQuery);
	}
    Free(m_Tree.GetRootItem());
	CFormView::OnClose();
}
//---------------------------------------------------------------------------------------



CString* CQATree::GetCurrentNodeName() {
CTreeNodeInfo* info;

   HTREEITEM item = m_Tree.GetSelectedItem();
   if(item==NULL) return NULL;
   info=(CTreeNodeInfo *)m_Tree.GetItemData(item);
   return &(info->m_NodeName);
}


//--------------------------------------------------------------------------------------------
// This callback will run when user press "Reset" button.
//--------------------------------------------------------------------------------------------
void CQATree::OnReset()  {
CTreeNodeInfo* info;
CString query;
BSTR    sysQuery;
CQAParams* view;

	if(MessageBox("Are you sure that you\nwant to reset all weights and thresholds?",
		          "QA Warning",
				  MB_ICONQUESTION|MB_OKCANCEL)==IDOK) {
            query="sev_init";
            sysQuery=query.AllocSysString();
            DataSourceAccessSync(&sysQuery);
            SysFreeString(sysQuery); 

            query="source_dis sel2sym.dis";
            sysQuery=query.AllocSysString();
            DataSourceAccessSync(&sysQuery);
	        SysFreeString(sysQuery);

            Free(m_Tree.GetRootItem());
			m_Tree.DeleteAllItems();
			FillTree();
            HTREEITEM item = m_Tree.GetSelectedItem();
            view=(CQAParams* )(((CSplitterWnd *)GetParent())->GetPane(0,1));
	        if(item!=NULL) {
 	            info=(CTreeNodeInfo *)m_Tree.GetItemData(item);
				view->Refresh(info->m_NodeName);
			} else {
				view->Refresh(CString(""));
			}
	}			
}
//--------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// This callback will run when user press "Run All" button.
//--------------------------------------------------------------------------------------------
void CQATree::OnRunAll()  {
CString query;
BSTR    sysQuery;
CQAParams* view;
CTreeNodeInfo* info;
CMainFrame* main;
main=(CMainFrame *)AfxGetMainWnd(); 
 CString scope;

    view=(CQAParams* )(((CSplitterWnd *)GetParent())->GetPane(0,1));
    view->GetScope(scope);
	if(scope=="Browser Selection" && (main->m_SelectionSize==0 || (main->m_wndBrowser.SelectionHasKind("module")==FALSE) && main->m_wndBrowser.SelectionHasKind("proj")==FALSE)) {
		MessageBox("No projects/modules were selected in the browser.\nPlease select modules to process.","QA Error",
	    MB_ICONSTOP|MB_OK);
		return;
	}
	if(scope=="Group Selection") {
		query = "size [filter {kind==\"module\" || kind==\"proj\"} $DiscoverGroups]";
        sysQuery=query.AllocSysString();
        CString res = DataSourceAccessSync(&sysQuery);
        SysFreeString(sysQuery); 
		if(atoi(res)==0) {
		    MessageBox("No groups containing projects/modules were selected in the browser.\nPlease select groups to process.","QA Error",
	        MB_ICONSTOP|MB_OK);
			return;
		}
	}
	if(MessageBox("Running your queries will take a long time.\nAre you sure that you want to do this?",
		          "QA Warning",
				  MB_ICONQUESTION|MB_OKCANCEL)==IDOK) {

            query.Format("sev_run \"/\" \"%s\"",scope);
            sysQuery=query.AllocSysString();
            CString res = DataSourceAccessSync(&sysQuery);
            SysFreeString(sysQuery); 
			if(res.GetLength()>0) {
                CString errorMsg;
		        errorMsg="================== ERRORS DETECTED ==================\r\n";
			    errorMsg+=res;
	            main->m_wndErrors.SetErrorLog(errorMsg);
                main->ShowControlBar( &main->m_wndErrors, TRUE, FALSE);	
			} else 
				GenerateReport(GetReportName());
            HTREEITEM item = m_Tree.GetSelectedItem();
	        if(item!=NULL) {
 	            info=(CTreeNodeInfo *)m_Tree.GetItemData(item);
				view->Refresh(info->m_NodeName);
			    CQAChildFrame* parent = (CQAChildFrame *)GetParent()->GetParent();
			    parent->SetCurrentSetEnable(view->GetCurrentSetEnable());
			}
	}			
}
//--------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// This callback will run when user press "Run Selected" button.
//--------------------------------------------------------------------------------------------
void CQATree::OnRunSelected()  {
CString query;
BSTR    sysQuery;
CTreeNodeInfo* info;
CQAParams* view;
CMainFrame* main;
main=(CMainFrame *)AfxGetMainWnd(); 

   view=(CQAParams* )(((CSplitterWnd *)GetParent())->GetPane(0,1));
   HTREEITEM item = m_Tree.GetSelectedItem();
   CString scope;
   view->GetScope(scope);

   if(scope=="Browser Selection" && (main->m_SelectionSize==0 || (main->m_wndBrowser.SelectionHasKind("module")==FALSE && main->m_wndBrowser.SelectionHasKind("proj")==FALSE))) {
		MessageBox("No projects/modules are selected in the browser.\nPlease select modules to process.","QA Error",
	    MB_ICONSTOP|MB_OK);
		return;
   }
   if(scope=="Group Selection") {
		query = "size [filter {kind==\"module\" || kind==\"proj\"} $DiscoverGroups]";
        sysQuery=query.AllocSysString();
        CString res = DataSourceAccessSync(&sysQuery);
        SysFreeString(sysQuery); 
		if(atoi(res)==0) {
		    MessageBox("No groups containing projects/modules are selected in the browser.\nPlease select groups to process.","QA Error",
	        MB_ICONSTOP|MB_OK);
			return;
		}
   }
   if(item==NULL)  {
	   MessageBox("No query selected.\nPlease select a query to run first.","QA Error",MB_ICONEXCLAMATION);
	   return;
   }
   info=(CTreeNodeInfo *)m_Tree.GetItemData(item);
   query.Format("sev_run \"%s\" {%s}",info->m_NodeName,scope);
   sysQuery=query.AllocSysString();
   CString res = DataSourceAccessSync(&sysQuery);
   SysFreeString(sysQuery); 
   if(res.GetLength()>0) {
       CString errorMsg;
       errorMsg="================== ERRORS DETECTED ==================\r\n";
       errorMsg+=res;
       main->m_wndErrors.SetErrorLog(errorMsg);
       main->ShowControlBar( &main->m_wndErrors, TRUE, FALSE);	
   } else 
		GenerateReport(GetReportName());
   item = m_Tree.GetSelectedItem();
   if(item!=NULL) {
       info=(CTreeNodeInfo *)m_Tree.GetItemData(item);
	   view->Refresh(info->m_NodeName);
	   CQAChildFrame* parent = (CQAChildFrame *)GetParent()->GetParent();
	   parent->SetCurrentSetEnable(view->GetCurrentSetEnable());
   }
}
//--------------------------------------------------------------------------------------------

void CQATree::FillNewTree() {
CQAParams* view;
CTreeNodeInfo* info;

   Free(m_Tree.GetRootItem());
   m_Tree.DeleteAllItems();
   FillTree();
   HTREEITEM item = m_Tree.GetSelectedItem();
   view=(CQAParams* )(((CSplitterWnd *)GetParent())->GetPane(0,1));
   if(item!=NULL) {
       info=(CTreeNodeInfo *)m_Tree.GetItemData(item);
	   view->Refresh(info->m_NodeName);
   } else view->Refresh(CString(""));

}


void CQATree::Update() {
CString query;
BSTR    sysQuery;

      query="sev_init";
      sysQuery=query.AllocSysString();
      DataSourceAccessSync(&sysQuery);
      SysFreeString(sysQuery); 
      query="source_dis sel2sym.dis";
      sysQuery=query.AllocSysString();
      DataSourceAccessSync(&sysQuery);
	  SysFreeString(sysQuery);
	  FillNewTree();
}

BOOL CQATree::Open() {
CSevOpenDlg dlg;
CString query;
CString result;
BSTR    sysQuery;
CMainFrame* main;
main=(CMainFrame *)AfxGetMainWnd(); 
register i;
    
     query="sev_list";
     sysQuery=query.AllocSysString();
     result = DataSourceAccessSync(&sysQuery);
     SysFreeString(sysQuery); 

	 CString name;
	 name="";
	 int in=0;
	 for(i=0;i<result.GetLength();i++) {
		 if(result[i]==' ' && in==0) {
			 name.TrimRight();
			 name.TrimLeft();
			 if(name.GetLength()>0) {
				 if(name!="default_run")
			         dlg.m_NamesList.AddTail(name);
				 name = "";
				 continue;
			 }
		 }
		 if(result[i]=='{') {
			 in++;
			 continue;
		 }
		 if(result[i]=='}') {
			 in--;
			 continue;
		 }
		 name+=result[i];
	 }
	 name.TrimRight();
	 name.TrimLeft();
     if(name.GetLength()>0) {
	    if(name!="default_run")
	       dlg.m_NamesList.AddTail(name);
	    name = "";
	 }
     if(dlg.DoModal()==IDOK) {
          query.Format("sev_load %s",dlg.m_Name);
          sysQuery=query.AllocSysString();
          DataSourceAccessSync(&sysQuery);
          SysFreeString(sysQuery); 
          GenerateReport(GetReportName());
		  return TRUE;
	 }
	 return FALSE;
}

void CQATree::Save() {
CSevSaveDlg dlg;
CString query;
BSTR    sysQuery;

     if(dlg.DoModal()==IDOK) {
		 if(dlg.m_Name.GetLength()>0) {
			 CString name;
			 for(int i=0;i<dlg.m_Name.GetLength();i++) {
				 if(dlg.m_Name[i]=='{') {
					 name+="<";
					 continue;
				 }
				 if(dlg.m_Name[i]=='}') {
					 name+=">";
					 continue;
				 }
				 name+=dlg.m_Name[i];
			 }
             query.Format("sev_save {%s}",name);
             sysQuery=query.AllocSysString();
             DataSourceAccessSync(&sysQuery);
             SysFreeString(sysQuery);
			 
			 CString sz;
	         query="set sss [group info]";
             sysQuery=query.AllocSysString();
             DataSourceAccessSync(&sysQuery);
             SysFreeString(sysQuery);
	         query="llength $sss";
             sysQuery=query.AllocSysString();
             sz = DataSourceAccessSync(&sysQuery);
             SysFreeString(sysQuery);
  	         for(i=0;i<atoi(sz);i++) {
		         query.Format("group kind [lindex $sss %d]",i);
                 sysQuery=query.AllocSysString();
                 CString type = DataSourceAccessSync(&sysQuery);
                 SysFreeString(sysQuery);
		         if (type=="persistant") {
		               query.Format("group save [lindex $sss %d]",i);
                       sysQuery=query.AllocSysString();
                       DataSourceAccessSync(&sysQuery);
                       SysFreeString(sysQuery);
				 }
			 }

		 } else {
			 AfxMessageBox("Can't save QA session with empty name");
		 }
	 }
}


CString CQATree::GetReportName(void) {
char buf[1024];
CString reportFileName;

      if(GetEnvironmentVariable("PSETHOME_LOCAL",buf,1024)!=0) {
               reportFileName=buf;
               reportFileName+="/Reports/";
	  } else {
               reportFileName="C:/Discover/Reports/";
	  }
      reportFileName+="qa.htm";

	  CString report;
      for(register i=0;i<reportFileName.GetLength();i++)  {
	         if(reportFileName[i]=='\\') report+='/';
		     else report+=reportFileName[i];
	  }
	  return report;

}

void CQATree::GenerateReport(CString &to) {
CMainFrame* main;
main=(CMainFrame *)AfxGetMainWnd(); 
CString query;
BSTR sysQuery;
    if(main->m_Lic[LIC_QAREPORT]>0) {
		  Sleep(1000);
		  if(to.Right(4) == ".txt") {
              query.Format("sev_create_report \"DETAILED TEXT\" {%s} 1",to);
		  } else {
              query.Format("sev_create_report \"HTML\" {%s} 1",to);
		  }
          sysQuery=query.AllocSysString();
          CString res = DataSourceAccessSync(&sysQuery);
	      SysFreeString(sysQuery); 
	}
}

CString CQATree::DataSourceAccessSync(BSTR* command) {
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
