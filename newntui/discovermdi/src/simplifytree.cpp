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
// SimplifyTree.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "SimplifyTree.h"
#include "SimplifyChildFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSimplifyTree

IMPLEMENT_DYNCREATE(CSimplifyTree, CFormView)

CSimplifyTree::CSimplifyTree()
	: CFormView(CSimplifyTree::IDD)
{
	//{{AFX_DATA_INIT(CSimplifyTree)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Substitute=FALSE;
	m_CurrentItem=-1;
	m_SimplifyRunning = FALSE;
	m_FilesUpdated=FALSE;
	m_FilesToExclude = 0;
	m_bLicensed = false;
}

CSimplifyTree::~CSimplifyTree()
{
}

void CSimplifyTree::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSimplifyTree)
	DDX_Control(pDX, IDC_TREE, m_Tree);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	//}}AFX_DATA_MAP
	CString query;
	BSTR sysQuery;
//	CDiscoverMDIApp* app = (CDiscoverMDIApp *) AfxGetApp();
//	m_DataSource.SetSource(app->m_ServiceName,TRUE);

	query="set tmp 0";
	sysQuery=query.AllocSysString();
	DataSourceAccessSync(&sysQuery);
	::SysFreeString(sysQuery);

	query="source_dis els.dis";
	sysQuery=query.AllocSysString();
	DataSourceAccessSync(&sysQuery);
	::SysFreeString(sysQuery);
	query="source_dis simplify_nt.dis";
	sysQuery=query.AllocSysString();
	DataSourceAccessSync(&sysQuery);
	::SysFreeString(sysQuery);
}


BEGIN_MESSAGE_MAP(CSimplifyTree, CFormView)
	//{{AFX_MSG_MAP(CSimplifyTree)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_TREE, OnClickTree)
	ON_COMMAND(IDC_SIMPLIFY, OnSimplifyReady)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimplifyTree diagnostics

#ifdef _DEBUG
void CSimplifyTree::AssertValid() const
{
	CFormView::AssertValid();
}

void CSimplifyTree::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimplifyTree message handlers

void CSimplifyTree::OnInitialUpdate()  {
	CFormView::OnInitialUpdate();
    SIZE sizeTotal;
	sizeTotal.cx=0;
	sizeTotal.cy=0;
	SetScrollSizes( MM_TEXT, sizeTotal);

    CBitmap root;
    root.LoadBitmap(IDB_SIMPLIFY_ROOT);
    CBitmap permanent;
    permanent.LoadBitmap(IDB_SIMPLIFY_PERMANENT);
    CBitmap unchecked;
    unchecked.LoadBitmap(IDB_SIMPLIFY_UNCHECK);
    CBitmap unchecked_sel;
    unchecked_sel.LoadBitmap(IDB_SIMPLIFY_UNCHECK_SEL);
    CBitmap checked;
    checked.LoadBitmap(IDB_SIMPLIFY_CHECK);
    CBitmap checked_sel;
    checked_sel.LoadBitmap(IDB_SIMPLIFY_CHECK_SEL);
    CBitmap progress;
    progress.LoadBitmap(IDB_SIMPLIFY_PROCESSING);

    m_Images.Create(24,16,TRUE,8,0);

	m_Images.Add(&root,          (CBitmap *)NULL);
	m_Images.Add(&root,          (CBitmap *)NULL);
	m_Images.Add(&permanent,     (CBitmap *)NULL);
	m_Images.Add(&permanent,     (CBitmap *)NULL);
	m_Images.Add(&unchecked,     (CBitmap *)NULL);
	m_Images.Add(&unchecked_sel, (CBitmap *)NULL);
	m_Images.Add(&checked,       (CBitmap *)NULL);
	m_Images.Add(&checked_sel,   (CBitmap *)NULL);
	m_Images.Add(&progress,      (CBitmap *)NULL);
    m_Tree.SetImageList(&m_Images,TVSIL_NORMAL); 

	CString query="license n co";
    BSTR sysQuery=query.AllocSysString();
    CString res = DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery);
	m_bLicensed = (res=="1");
}

void CSimplifyTree::OnSize(UINT nType, int cx, int cy)  {
	CFormView::OnSize(nType, cx, cy);
	if(::IsWindow(m_Tree)) m_Tree.MoveWindow(0,0,cx,cy);
}


//-------------------------------------------------------------------------------
// Thread wich will run if user press "Run" button.
//-------------------------------------------------------------------------------
DWORD WINAPI SimplifyRunThread(LPVOID param) {
CSimplifyTree* p=(CSimplifyTree*) param;
register i,j;
int oldLevel=0;
HTREEITEM addTo = NULL;
HTREEITEM lastItem;
int offset=0;
BOOL parentEnabled=TRUE;
CIncludeTreeNode* node;
TV_ITEM tvItem;
TV_INSERTSTRUCT tvInsert;

    CString str;
    CString query;
    BSTR sysQuery;

	CString shortName = p->GetShortName(p->m_FileName);


	node = new CIncludeTreeNode;
	node->m_FileName=p->m_FileName;
	node->m_ID=-1;
	node->m_Level=-1;
	node->m_Line=0;
	node->m_Marked=1;
	node->m_Status=0;

	//------- Inserting root into the tree ----------------------------------
    tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE  | TVIF_SELECTEDIMAGE;
    tvItem.pszText        = (char *)(LPCSTR)shortName;
    tvItem.cchTextMax     = shortName.GetLength();
    tvItem.lParam         = (long)node;
    tvItem.iImage         = 0;
    tvItem.iSelectedImage = 1;
    tvInsert.hParent      = NULL;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.item         = tvItem;
	addTo=lastItem=p->m_Tree.InsertItem(&tvInsert);
	//----------------------------------------------------------------------

	//------- Inserting process indicator the tree ----------------------------------
    tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE  | TVIF_SELECTEDIMAGE;
    tvItem.pszText        = "Parsing file ...";
    tvItem.cchTextMax     = 18;
    tvItem.lParam         = (long)0;
    tvItem.iImage         = 8;
    tvItem.iSelectedImage = 8;
    tvInsert.hParent      = addTo;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.item         = tvItem;
	HTREEITEM message = p->m_Tree.InsertItem(&tvInsert);
	p->m_Tree.EnsureVisible(message);
	//----------------------------------------------------------------------

	if(p->m_Substitute==TRUE) {
        query.Format("run_simplify -modify {%s}",p->m_FileName);
	} else {
        query.Format("run_simplify -modify -nosub {%s}",p->m_FileName);
	}
    sysQuery=query.AllocSysString();
    str=p->DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
	if(str.GetLength()>0) {
		AfxMessageBox(str,MB_ICONSTOP|MB_OK);
		p->m_Tree.SetItemText(message,"Parser errors");
	} else {
		p->m_Tree.SetItemText(message,"Waiting for data...");
		p->m_Tree.SendMessage(WM_PAINT);
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
		Sleep(15000); // this is a HACK
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
		p->m_Tree.SetItemText(message,"Simplifying headers...");
        query = "query_simplify";
        sysQuery=query.AllocSysString();
        str=p->DataSourceAccessSync(&sysQuery);
	    SysFreeString(sysQuery);
		p->m_Tree.DeleteItem(message);
		p->m_SimplifyRunning = TRUE;

	    // Results parsing  - we will extract element string from the strings list
	    CString includeElement;
        for(i=0;i<str.GetLength();i++) {
		    if(str[i]=='\n') {
			    includeElement+='\n';
			    // Element parsing - we will extract element information from the element string
			    CString levelStr;
			    CString lineStr;
			    CString markStr;
			    CString idStr;
			    CString fileStr;
			    int field=0;
			    node = new CIncludeTreeNode;
			    node->m_Status=FALSE;
			    for(j=0;j<includeElement.GetLength();j++) {
				    if(includeElement[j]=='@' || includeElement[j]=='\n') {
					    switch(field) {
					         case 0 : node->m_Level    = atoi(levelStr); break;
					         case 1 : node->m_Line     = atoi(lineStr);  break;
						     case 2 : node->m_Marked   = atoi(markStr);  break;
						     case 3 : node->m_ID       = atoi(idStr);    break;
						     case 4 : node->m_FileName = fileStr;        break;
						}
					    field++;
					    continue;
					}
				    switch(field) {
			           case 0 : levelStr += includeElement[j];  break;
			           case 1 : lineStr  += includeElement[j];  break;
				       case 2 : markStr  += includeElement[j];  break;
				       case 3 : idStr    += includeElement[j];  break;
				       case 4 : fileStr  += includeElement[j];  break;
					}
				}
			    if(node->m_Level==-1) {
				    delete node;
				} else {
				    CString shortName = p->GetShortName(node->m_FileName);
	                //------- Inserting item into the tree --------------------
                    tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                    tvItem.pszText        = (char *)(LPCSTR)shortName;
                    tvItem.cchTextMax     = shortName.GetLength();
                    tvItem.lParam         = (long)node;
					if(node->m_Marked==0) p->m_FilesToExclude++;

		            int image = p->FindNodeImage(node);
				    tvItem.iImage         = image;
                    tvItem.iSelectedImage = image+1;

	                while(node->m_Level < oldLevel) {
		                addTo = p->m_Tree.GetParentItem(addTo); 
		                oldLevel--;
					}
		            if(node->m_Level > oldLevel) {
			            addTo = lastItem;
					}

                    tvInsert.hParent      = addTo;
	                tvInsert.hInsertAfter = TVI_LAST;
	                tvInsert.item         = tvItem;
	                lastItem=p->m_Tree.InsertItem(&tvInsert);
					if(node->m_Marked==0)
	                     p->m_Tree.EnsureVisible(lastItem);

					// If substitute mode is turned off any files needed at the
					// bottom level will force all parents to be included
					if((p->m_Substitute==FALSE) && (node->m_Marked!=0)) {
                        CIncludeTreeNode* pNode;
						HTREEITEM parentItem = lastItem;
						parentItem = p->m_Tree.GetParentItem(parentItem);
						while(parentItem) {
							pNode=(CIncludeTreeNode *)p->m_Tree.GetItemData(parentItem);
							pNode->m_Marked=1;
		                    int img = p->FindNodeImage(pNode);
							p->m_Tree.SetItemImage(parentItem,img,img+1);
						    parentItem = p->m_Tree.GetParentItem(parentItem);
						}
					}
		            //----------------------------------------------------------
		            oldLevel=node->m_Level;
				}
			    includeElement="";
			    continue;
			}
		    includeElement+=str[i];
		}
	}
	p->PostMessage(WM_COMMAND,IDC_SIMPLIFY,0);
	return 0;
}
//-------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------
// This private function will parse selected file with the index "element" and will fill the
// tree with the data
//---------------------------------------------------------------------------------------------------
void CSimplifyTree::ParseAndFillTree(int element,const CString& filename) {
    CString str;
    CString query;
    BSTR sysQuery;


	CString shortName = GetShortName(filename);

    query.Format("fname {%s}",filename);
    sysQuery=query.AllocSysString();
    str=DataSourceAccessSync(&sysQuery);
	SysFreeString(sysQuery); 
	m_FileName = str;

	EndSimplify();

	ClearTree(m_Tree.GetRootItem());
	m_Tree.DeleteAllItems();
	m_SavedSelection=m_CurrentItem=element;
	
//	DWORD dwThreadId;
//	CreateThread(NULL,0,SimplifyRunThread,(LPVOID)this,0,&dwThreadId);
	SimplifyRunThread(this);
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------
void CSimplifyTree::SetNewElement(int element, const CString& filename) {
	if(m_CurrentItem!=element) {
	    ParseAndFillTree(element,filename);
		m_SavedFileName  = filename;
	}
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
// This private function transforms the long file name into short form.
//---------------------------------------------------------------------------------------------------
CString CSimplifyTree::GetShortName(const CString& longName) {
register i;

    for(i=longName.GetLength()-1;i>=0;i--) {
		if(longName[i]=='\\' || longName[i]=='/' || longName[i]==':') {
			   i++;
			   break;
		}
	}
	return longName.Right(longName.GetLength()-i);
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------
void CSimplifyTree::OnClickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
POINT curPos;
UINT flags;
HTREEITEM item;
CIncludeTreeNode* node;
CString status;

	GetCursorPos(&curPos);
	m_Tree.ScreenToClient(&curPos);
	item = m_Tree.HitTest(curPos,&flags);
	if(item!=NULL) {
        node=(CIncludeTreeNode *)m_Tree.GetItemData(item);
		if( (flags&TVHT_ONITEMICON) !=0 ) {
			 if(node && node->m_Marked==0) {			 
				 BOOL status = node->m_Status;
				 if(m_Substitute==TRUE) {
					 if(status==TRUE) item = GetFirstIncludedItem(item);
					 m_PropagateLevel=0;
					 ChildPropagateStatus(item,!status);
				 } else {
				     item = GetSourceFileItem(item);
					 m_PropagateLevel=0;
					 ChildPropagateStatus(item,!status);
				 }

			 }
		}
		if(node) {
		   CSimplifyChildFrame* frame = (CSimplifyChildFrame *) GetParent()->GetParent();
		   frame->m_Editor->LoadFile(node->m_FileName);
		   if(node->m_Level==-1) {
			   HTREEITEM child;
			   child=m_Tree.GetChildItem(item);
			   while(child) {
                   node=(CIncludeTreeNode *)m_Tree.GetItemData(child);
				   if(node) {
				       if(node->m_Marked==1) {
					       // Mark as needed
                           frame->m_Editor->MarkLine(node->m_Line-1,0);
					   } else {
					       if(node->m_Status==TRUE)
						        // Mark as forced by user
                                frame->m_Editor->MarkLine(node->m_Line-1,1);
					       else 
						        // Mark as excluded
                                frame->m_Editor->MarkLine(node->m_Line-1,2);
					   }
				   }
				   child=m_Tree.GetNextSiblingItem(child);
			   }
		   }
		}
	}
	*pResult = 0;
}
//---------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This private function will return the node image index in our image list
// calculating it from the node type and status + action check.
//---------------------------------------------------------------------------------------
int CSimplifyTree::FindNodeImage(CIncludeTreeNode* node) {
int index;

    if(node==NULL) {
	    index=0;
	} else {
		if(node->m_Marked!=0) {
			index=2;
		} else {
		    if(node->m_Status==FALSE) index=4;
		    else				      index=6;
		}
	}
	return index;
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// This private recursive function was designed to reflect the parent status change on 
// all it's children. If you will disable the parent, all sub-tree attached to the 
// parent will be also disabled and you will be unsbale to switch sub-tree items status
// while parent is disabled. If you enable the parent status, all sub-tree will return
// to it's initial status and you will be able to switch the sub-tree items status.
//---------------------------------------------------------------------------------------
void CSimplifyTree::ChildPropagateStatus(HTREEITEM item, BOOL newStatus) {
CIncludeTreeNode* node;
CString query;
CString status;

    // We need a pointer to the user structure attached to the UI element.
 	node=(CIncludeTreeNode *)m_Tree.GetItemData(item);

	// This node will contain it's parent status.
	node->m_Status=newStatus;

	// Setting the new status according to the parent status and 
	// the current node status.
    int img_ind = FindNodeImage(node);
	m_Tree.SetItemImage(item,img_ind,img_ind+1);

    HTREEITEM child   = m_Tree.GetChildItem(item);
	if(child!=NULL)  {
		m_PropagateLevel++;
		ChildPropagateStatus(child,(newStatus && node->m_Status==TRUE));
		m_PropagateLevel--;
	}
	if(m_PropagateLevel>0) {
        HTREEITEM sibling = m_Tree.GetNextSiblingItem(item);
	    if(sibling!=NULL)  ChildPropagateStatus(sibling,newStatus);
	}
}
//---------------------------------------------------------------------------------------


HTREEITEM CSimplifyTree::GetFirstIncludedItem(HTREEITEM item) {
CIncludeTreeNode* node;
 	node=(CIncludeTreeNode *)m_Tree.GetItemData(item);
	if(node->m_Status==FALSE || node->m_Level==0) return item;
    HTREEITEM parent = m_Tree.GetParentItem(item);
 	node=(CIncludeTreeNode *)m_Tree.GetItemData(parent);
	if(node->m_Status==FALSE) return item;
	return GetFirstIncludedItem(parent);
}

HTREEITEM CSimplifyTree::GetSourceFileItem(HTREEITEM item) {
CIncludeTreeNode* node;
 	node=(CIncludeTreeNode *)m_Tree.GetItemData(item);
	if(node->m_Level==0) return item;
    HTREEITEM parent = m_Tree.GetParentItem(item);
	return GetSourceFileItem(parent);
}

void CSimplifyTree::EnableInclusionMode(BOOL enable) {

	m_Substitute = enable;

	// End previous copy of the simplify
	if(m_SimplifyRunning == TRUE) {
        DisableAllExclusions(m_Tree.GetRootItem());
	    m_FilesUpdated=FALSE;
	    m_FilesToExclude = 0;
        CString query;
        BSTR sysQuery;
        query="quit_simplify";
        sysQuery=query.AllocSysString();
        DataSourceAccessSync(&sysQuery);
        SysFreeString(sysQuery); 
	    m_SimplifyRunning = FALSE;
	}
	m_CurrentItem=-1;
	if(!m_SavedFileName.IsEmpty())	SetNewElement(m_SavedSelection,m_SavedFileName);
}

BOOL CSimplifyTree::GetInclusionMode() {
	return m_Substitute;

}

//------------------------------------------------------------------------------------------
// This will inspect the include file tree and will tell simplify not comment the files
// marked by user
//------------------------------------------------------------------------------------------
void CSimplifyTree::DisableMarkedExclusions(HTREEITEM item) {
CIncludeTreeNode* node;
CString query;
CString status;

    if(item==NULL) return;

    // We need a pointer to the user structure attached to the UI element.
 	node=(CIncludeTreeNode *)m_Tree.GetItemData(item);


	// For all include files which we want to be included we will send command to the simplify
	if(node) {
		if(node->m_Marked==0) {
			if(node->m_Status==TRUE) {
                 CString query;
                 BSTR sysQuery;
                 query.Format("tell_simplify %d",node->m_ID);
                 sysQuery=query.AllocSysString();
                 DataSourceAccessSync(&sysQuery);
	             SysFreeString(sysQuery); 
			}
		}
	}
    HTREEITEM child   = m_Tree.GetChildItem(item);
	if(child!=NULL)  DisableMarkedExclusions(child);
    HTREEITEM sibling = m_Tree.GetNextSiblingItem(item);
	if(sibling!=NULL)  DisableMarkedExclusions(sibling);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This will inspect the include files tree and will tell simplify not to
// comment all files it marked as unneeded.
//-----------------------------------------------------------------------------------------
void CSimplifyTree::DisableAllExclusions(HTREEITEM item) {
CIncludeTreeNode* node;
CString query;
CString status;

    if(item==NULL) return;

    // We need a pointer to the user structure attached to the UI element.
 	node=(CIncludeTreeNode *)m_Tree.GetItemData(item);


	// For all include files which we want to be included we will send command to the simplify
	if(node) {
		if(node->m_Marked==0) {
             CString query;
             BSTR sysQuery;
             query.Format("tell_simplify %d",node->m_ID);
             sysQuery=query.AllocSysString();
             DataSourceAccessSync(&sysQuery);
             SysFreeString(sysQuery); 
		}
	}
    HTREEITEM child   = m_Tree.GetChildItem(item);
	if(child!=NULL)  DisableAllExclusions(child);
    HTREEITEM sibling = m_Tree.GetNextSiblingItem(item);
	if(sibling!=NULL)  DisableAllExclusions(sibling);
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This will free all tree nodes and clean tree UI
//-----------------------------------------------------------------------------------------
void CSimplifyTree::ClearTree(HTREEITEM item) {
    if(item==NULL) return;
    // We need a pointer to the user structure attached to the UI element.
	CIncludeTreeNode * node;
 	node=(CIncludeTreeNode *)m_Tree.GetItemData(item);
	if(node) delete node;
    HTREEITEM child   = m_Tree.GetChildItem(item);
	if(child!=NULL)  ClearTree(child);
    HTREEITEM sibling = m_Tree.GetNextSiblingItem(item);
	if(sibling!=NULL)  ClearTree(sibling);
}
//-----------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------
// This function will send all changes to the simplify and than close it.
//---------------------------------------------------------------------------------------------------
void CSimplifyTree::Done() {
CSimplifyChildFrame* frame = (CSimplifyChildFrame *) GetParent()->GetParent();

	DisableMarkedExclusions(m_Tree.GetRootItem());
    frame->m_Selection->SetItemStatus(m_CurrentItem,2);
	m_FilesUpdated = TRUE;
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------
void CSimplifyTree::Close()  {
	if(m_bLicensed) {
		CString query;
		BSTR sysQuery;
		
		query="license n ci";
		sysQuery=query.AllocSysString();
		DataSourceAccessSync(&sysQuery);
		SysFreeString(sysQuery);
		m_bLicensed = false;
	}
	EndSimplify();
	ClearTree(m_Tree.GetRootItem());
}
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
// This private function will send file ids to stop exclusion
// and will end simplify;
//---------------------------------------------------------------------------------------------------
void CSimplifyTree::EndSimplify() {
CSimplifyChildFrame* frame = (CSimplifyChildFrame *) GetParent()->GetParent();
CString query;
BSTR sysQuery;
        
	if(m_SimplifyRunning != FALSE) {
		if(m_FilesUpdated==FALSE && m_FilesToExclude>0) {
			if(MessageBox("Simplify has detected unnecessary header files.\nWould you like to remove them from your source code?",
						   "Simplify *.h", 
						   MB_ICONQUESTION | MB_YESNO)==IDYES) {
				   DisableMarkedExclusions(m_Tree.GetRootItem());
				   query="save_and_quit_simplify";
				   frame->m_Selection->SetItemStatus(m_CurrentItem,2);
			 } else {
				   DisableAllExclusions(m_Tree.GetRootItem());
				   query="quit_simplify";
				   frame->m_Selection->SetItemStatus(m_CurrentItem,1);
			}
		} else {
			if(m_FilesToExclude>0) query="save_and_quit_simplify";
			else                   query="quit_simplify";
			frame->m_Selection->SetItemStatus(m_CurrentItem,1);
		}
		m_FilesUpdated=FALSE;
		m_FilesToExclude = 0;
		sysQuery=query.AllocSysString();
		DataSourceAccessSync(&sysQuery);
		SysFreeString(sysQuery); 
		
		m_SimplifyRunning = FALSE;
	}	
}
//---------------------------------------------------------------------------------------------------

void CSimplifyTree::OnSimplifyReady()  {
	if(m_SavedSelection==m_CurrentItem) {
		m_Tree.Invalidate(FALSE);
        CSimplifyChildFrame* frame = (CSimplifyChildFrame *) GetParent()->GetParent();
	    frame->m_Selection->SetItemStatus(m_CurrentItem,1);
		AfxGetMainWnd()->SendMessage(WM_UPDATE_ERRORS,0,0);

		// Loading top-level source file into the editor
		HTREEITEM root = m_Tree.GetRootItem();
		if(root) {
           CIncludeTreeNode* node;
		   node = (CIncludeTreeNode*) m_Tree.GetItemData(root);
		   if(node) {
		       frame->m_Editor->LoadFile(node->m_FileName);
		       if(node->m_Level==-1) {
			       HTREEITEM child;
			       child=m_Tree.GetChildItem(root);
			       while(child) {
                       node=(CIncludeTreeNode *)m_Tree.GetItemData(child);
				       if(node) {
				           if(node->m_Marked==1) {
					           // Mark as needed
                               frame->m_Editor->MarkLine(node->m_Line-1,0);
						   } else {
					           if(node->m_Status==TRUE)
						            // Mark as forced by user
                                    frame->m_Editor->MarkLine(node->m_Line-1,1);
					           else 
						            // Mark as excluded
                                    frame->m_Editor->MarkLine(node->m_Line-1,2);
						   }
					   }
				       child=m_Tree.GetNextSiblingItem(child);
				   }
			   }
		   }
		}
	} else {
		Sleep(500);
		SetNewElement(m_SavedSelection,m_SavedFileName);
	}
}

//------------------------------------------------------------------------------------------
// This function check communications status after access datasource.
//------------------------------------------------------------------------------------------
CString CSimplifyTree::DataSourceAccessSync(BSTR* command) {
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
	} else if (m_DataSource.IsConnectionLost() == FALSE) {
		bCommDlgShown = FALSE;
	}
	return res;
}

bool CSimplifyTree::IsLicensed()
{
	return m_bLicensed;
}
