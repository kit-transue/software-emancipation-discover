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

#include "stdafx.h"
#include "afxctl.h"
#include "ReportTemolate.h"
#include "ReportTemolateCtl.h"
#include "ReportTemolatePpg.h"

#define STATIC_TEXT  0
#define ATTRIBUTE    1
#define LOOP         2
#define QUERY        3 
#define SWITCH       4

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int level=0;
static HTREEITEM oldNode=NULL;
static HTREEITEM startNode=NULL;
static BOOL asChild;

IMPLEMENT_SERIAL(CReportNode,CObject,10)
//===============================================================================
// This class represent the node in the report node tree. Report node tree is
// binary tree with the nodes containing Acess query to form set, template to
// describe set and maybe the relations to the inner nodes of the same type.


//-------------------------------------------------------------------------------
CReportNode::CReportNode() {
	m_NodeType=STATIC_TEXT;
	m_Text="";
	m_Tag="";
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
CReportNode::~CReportNode() {
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This function will return 2 if the node is set iterator and need to form 
// the set by evaluating query (the text templates instances will appear at 
// the output document for every element in a set),1 if this node contains query 
// and query results must be printed or 0 if the node contain only static text.
//-------------------------------------------------------------------------------
int CReportNode::GetNodeType() {
	return m_NodeType;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This function will set the node type - see description above.
//-------------------------------------------------------------------------------
void CReportNode::SetNodeType(int type) {
	m_NodeType=type;
}
//-------------------------------------------------------------------------------



//-------------------------------------------------------------------------------
// Returns the node tag. It will be used in a stylesheet to	 format and present 
// the node textual results.
//-------------------------------------------------------------------------------
CString& CReportNode::GetTag(void) {
	return m_Tag;
}
//-------------------------------------------------------------------------------
			  

//-------------------------------------------------------------------------------
// Sets the node tag. It will be used in a stylesheet to format and present 
// the node textual results.
//-------------------------------------------------------------------------------
void CReportNode::SetTag(CString& text) {
	m_Tag=text;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// Returns the node text the meaning of which depends from the node type.
// ==0 Static text nodes
// ==1 Attribute
// ==2 Loop (iterator)
// ==3 Query (creating a new set)
// ==4 Switch (condition to go into sub-tree)
//-------------------------------------------------------------------------------
CString& CReportNode::GetText(void) {
	return m_Text;
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// Sets the node text the meaning of which depends from the node type.
// ==0 Static text nodes
// ==1 Attribute
// ==2 Loop (iterator)
// ==3 Query (creating a new set)
// ==4 Switch (condition to go into sub-tree)
//-------------------------------------------------------------------------------
void CReportNode::SetText(CString& text) {
	m_Text=text;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This class will support stream operations. We will walk
// around the tree to save or restore nodes from the stream.
//-------------------------------------------------------------------------------
void CReportNode::Serialize(CArchive& ar) {
    CObject::Serialize( ar );
    if( ar.IsStoring() ) {
        ar<<m_NodeType<<m_Tag<<m_Text;
	} else {
        ar>>m_NodeType;
		ar>>m_Tag;
		ar>>m_Text;
	}
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This function will write header into the stream in the
// form of:
// 1. Static text nodes: <nodename> text
// 2. Access commands  : <nodename>
//                         <COMMAND Query="text">
// 3. Set iterator     : <nodename>
//                           <LOOP Query = "text">
//-------------------------------------------------------------------------------
void CReportNode::WriteHeader(CString& out){
CString header;
   switch(m_NodeType) {
     case STATIC_TEXT : header.Format("<%s> %s\r\n",m_Tag,m_Text); break;
     case ATTRIBUTE   : header.Format("<%s>\r\n    <ACCESSCOMM QUERY=\"%s\">\r\n",m_Tag,m_Text); break;
     case LOOP        : header.Format("<%s>\r\n    <ACCESSLOOP QUERY=\"%s\">\r\n",m_Tag,m_Text); break;
     case QUERY       : header.Format("<%s>\r\n    <ACCESSQUERY QUERY=\"%s\">\r\n",m_Tag,m_Text); break;
     case SWITCH      : header.Format("<%s>\r\n    <ACCESSCOND QUERY=\"%s\">\r\n",m_Tag,m_Text); break;
   }
   out+=header;
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This function will write footer into the text file in the
// form of :
// 1. Static text nodes: </nodename>
// 2. Access commands  :    </COMMAND>
//                       </nodename>
// 3. Set iterator     :    </LOOP>
//                       </nodename>
//-------------------------------------------------------------------------------
void CReportNode::WriteFooter(CString& out){
CString footer;
   switch(m_NodeType) {
     case STATIC_TEXT : footer.Format("    </%s>\r\n",m_Tag);             break;
     case ATTRIBUTE   : footer.Format("    </ACCESSCOMM>\r\n</%s>\r\n",m_Tag);  break;
     case LOOP        : footer.Format("    </ACCESSLOOP>\r\n</%s>\r\n",m_Tag);  break;
     case QUERY       : footer.Format("    </ACCESSQUERY>\r\n</%s>\r\n",m_Tag);break;
     case SWITCH      : footer.Format("    </ACCESSCOND>\r\n</%s>\r\n",m_Tag);break;
   }
   out+=footer;
}
//-------------------------------------------------------------------------------

//===============================================================================



//==================================================================================
// This class based on the CReportNode designed to represent any sub-tree separated
// from the control. We need this because of two things: any re-parenting or 
// drag-and-drop operations will work with the selection whoch can be sub-tree and
// we will need sub-trees to store ready components for Wizard and for component 
// gallery. 
//==================================================================================

//----------------------------------------------------------------------------------
// Constructor will create a disconnected STATIC_TEXT type tree node with
// empty tag and text fields.
//----------------------------------------------------------------------------------
CSubTreeNode::CSubTreeNode() {
   m_Parent=NULL;
   m_ChildList=NULL;
   m_Next=NULL;
   m_Prev=NULL;
}
//-----------------------------------------------------------------------------------
	
//-----------------------------------------------------------------------------------
// Now distructor does nothing.
//-----------------------------------------------------------------------------------
CSubTreeNode::~CSubTreeNode() {
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Gets and sets this node parent
//-----------------------------------------------------------------------------------
CSubTreeNode* CSubTreeNode::GetParent() {
	return m_Parent;
}

void CSubTreeNode::SetParent(CSubTreeNode* newParent) {
	m_Parent=newParent;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Gets and sets this node child list (the pointer to the first child  in a list)
//-----------------------------------------------------------------------------------
CSubTreeNode* CSubTreeNode::GetChildList() {
    return m_ChildList;
}

void CSubTreeNode::SetChildList(CSubTreeNode* newChild) {
	m_ChildList=newChild;
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Gets and sets this node next sibling (walking forward in the child  list 
// connected to this node parent)
//-----------------------------------------------------------------------------------
CSubTreeNode* CSubTreeNode::GetNextSibling() {
	return m_Next;
}


void CSubTreeNode::SetNextSibling(CSubTreeNode* newSibling) {
	m_Next=newSibling;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Gets and sets this node previous sibling (walking backward in the  child  list 
// connected to this node parent)
//-----------------------------------------------------------------------------------
CSubTreeNode* CSubTreeNode::GetPrevSibling() {
	return m_Prev;
}

void CSubTreeNode::SetPrevSibling(CSubTreeNode* newSibling) {
	m_Prev=newSibling;
}
//-----------------------------------------------------------------------------------



//===================================================================================



//-----------------------------------------------------------------------------------
// This callback will run every time the user moves the cursor into the window while
// in drag-and-drop mode.
//----------------------------------------------------------------------------------- 
DROPEFFECT CTreeDropTarget::OnDragEnter(CWnd* pWnd,
										COleDataObject* pDataObject,
										DWORD dwKeyState,CPoint point ) {
UINT formatID;
UINT formatTempl;
CPoint clientPoint=point;

  // If the format with the spesified name was already registered,
  // this function will return existing format ID. If this is first 
  // time we try to register clipboard format, this function will
  // register it and return new ID.
  formatID=RegisterClipboardFormat("DiscoverReportNode");
  // The template will come in this format
  formatTempl=RegisterClipboardFormat("DiscoverReportTemplate");
  if(pDataObject->IsDataAvailable(formatID)==TRUE) {
	 HTREEITEM node = m_Parent->NodeFromPoint(clientPoint);
	 if(node!=NULL) startNode=node;
	 if((dwKeyState & MK_SHIFT)!=0) asChild=TRUE;
	 else asChild=FALSE;
     if((dwKeyState & MK_CONTROL)==0)   return DROPEFFECT_MOVE;
	 else   return DROPEFFECT_COPY;
  } else  {
     if(pDataObject->IsDataAvailable(formatID)==TRUE) {
	     HTREEITEM node = m_Parent->NodeFromPoint(clientPoint);
	     if(node!=NULL) startNode=node;
	     if((dwKeyState & MK_SHIFT)!=0) asChild=TRUE;
	     else                           asChild=FALSE;
	     return DROPEFFECT_COPY;
	 }
  }
  return DROPEFFECT_NONE;
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// This callback will run every time the user moves the cursor inside the window while
// in drag-and-drop mode.
//----------------------------------------------------------------------------------- 
DROPEFFECT  CTreeDropTarget::OnDragOver( CWnd* pWnd, 
										 COleDataObject* pDataObject, 
									     DWORD dwKeyState, CPoint point ) {
CPoint clientPoint=point;
UINT formatID;
UINT formatTempl;

    formatID=RegisterClipboardFormat("DiscoverReportNode");
    // The template will come in this format
    formatTempl=RegisterClipboardFormat("DiscoverReportTemplate");

	// Check if the data in the specified format is available
	if(pDataObject->IsDataAvailable(formatID)==TRUE) {
		HTREEITEM node = m_Parent->NodeFromPoint(clientPoint);
		if(node!=NULL && startNode!=node) { 
			 if(oldNode!=node) {
                m_Parent->MarkDropTarget(node,oldNode); 
			    oldNode=node;
			 }
			 if((dwKeyState & MK_SHIFT)!=0) asChild=TRUE;
			 else asChild=FALSE;
			 if((dwKeyState & MK_CONTROL)==0)
	             return DROPEFFECT_MOVE;
			 else
	             return DROPEFFECT_COPY;
		}
	} else {
	    if(pDataObject->IsDataAvailable(formatTempl)==TRUE) {
		    HTREEITEM node = m_Parent->NodeFromPoint(clientPoint);
		    if(node!=NULL && startNode!=node) { 
			    if(oldNode!=node) {
                    m_Parent->MarkDropTarget(node,oldNode); 
			        oldNode=node;
				}
			    if((dwKeyState & MK_SHIFT)!=0) asChild=TRUE;
			    else asChild=FALSE;
	            return DROPEFFECT_COPY;
			}
		}
	}
    m_Parent->MarkDropTarget(NULL,oldNode); 
	oldNode=NULL;
	return DROPEFFECT_NONE;

}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This callback will run every time the user drops the cursor inside the window while
// in drag-and-drop mode.
//----------------------------------------------------------------------------------- 
BOOL  CTreeDropTarget::OnDrop( CWnd* pWnd, COleDataObject* pDataObject, 
							   DROPEFFECT dropEffect, CPoint point ) {
UINT   formatID;
CPoint clientPoint=point;
STGMEDIUM dataStruct;
CString filename;
UINT formatTempl;

    m_Parent->MarkDropTarget(NULL,oldNode); 
    oldNode=NULL;
    formatID=RegisterClipboardFormat("DiscoverReportNode");
    formatTempl=RegisterClipboardFormat("DiscoverReportTemplate");
	// Check if the data in the specified format is available
	if(pDataObject->IsDataAvailable(formatID)==TRUE) {
		HTREEITEM node = m_Parent->NodeFromPoint(clientPoint);
		if(node!=NULL && startNode!=node ) { 
	         startNode=NULL;
             pDataObject->GetData(formatID,&dataStruct);
			 filename=dataStruct.lpszFileName;
			 if(asChild==TRUE)
			     m_Parent->AddSubTreeFileAsChild(node,filename);
			 else
			     m_Parent->AddSubTreeFileAsSibling(node,filename);
			 m_Parent->SayAttributesNeeded(filename);
		     return TRUE;
		}
	} else {
	    if(pDataObject->IsDataAvailable(formatTempl)==TRUE) {
		    HTREEITEM node = m_Parent->NodeFromPoint(clientPoint);
		    if(node!=NULL && startNode!=node ) { 
	            startNode=NULL;
                pDataObject->GetData(formatTempl,&dataStruct);
			    filename=dataStruct.lpszFileName;
			    if(asChild==TRUE)
			        m_Parent->AddSubTreeFileAsChild(node,filename);
			    else
			        m_Parent->AddSubTreeFileAsSibling(node,filename);
			    filename=dataStruct.lpszFileName;
				m_Parent->SayAttributesNeeded(filename);
		        return TRUE;
			}
		}
	}
	return FALSE;
}
//-----------------------------------------------------------------------------------







IMPLEMENT_DYNCREATE(CReportTemplateCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CReportTemplateCtrl, COleControl)
	//{{AFX_MSG_MAP(CReportTemplateCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_NOTIFY(TVN_SELCHANGED, IDC_DOCTREE, OnItemChanged)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CReportTemplateCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CReportTemplateCtrl)
	DISP_FUNCTION(CReportTemplateCtrl, "AddChild", AddChild, VT_EMPTY, VTS_I2 VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CReportTemplateCtrl, "AddSibling", AddSibling, VT_EMPTY, VTS_I2 VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CReportTemplateCtrl, "Save", Save, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CReportTemplateCtrl, "Load", Load, VT_BOOL, VTS_BSTR)
	DISP_FUNCTION(CReportTemplateCtrl, "DeleteCurrent", DeleteCurrent, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CReportTemplateCtrl, "MakeXML", MakeXML, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CReportTemplateCtrl, "ChangeCurrent", ChangeCurrent, VT_BOOL, VTS_I2 VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CReportTemplateCtrl, "GetType", GetType, VT_I2, VTS_NONE)
	DISP_FUNCTION(CReportTemplateCtrl, "GetTag", GetTag, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CReportTemplateCtrl, "GetText", GetText, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CReportTemplateCtrl, "GetTagsList", GetTagsList, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CReportTemplateCtrl, "Clear", Clear, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CReportTemplateCtrl, "AddParent", AddParent, VT_EMPTY, VTS_I2 VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CReportTemplateCtrl, "FindToken", FindToken, VT_I4, VTS_BSTR)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CReportTemplateCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CReportTemplateCtrl, COleControl)
	//{{AFX_EVENT_MAP(CReportTemplateCtrl)
	EVENT_CUSTOM("SelectionChanged", FireSelectionChanged, VTS_BSTR  VTS_BSTR  VTS_I2)
	EVENT_CUSTOM("NodeRemoving", FireNodeRemoving, VTS_BSTR)
	EVENT_CUSTOM("AskAttributeFile", FireAskAttributeFile, VTS_BSTR  VTS_BSTR)
	EVENT_CUSTOM("IncludeAttributeFile", FireIncludeAttributeFile, VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CReportTemplateCtrl, 1)
	PROPPAGEID(CReportTemplatePropPage::guid)
END_PROPPAGEIDS(CReportTemplateCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CReportTemplateCtrl, "REPORTTEMOLATE.ReportTemolateCtrl.1",
	0xb0a92ba6, 0x41ac, 0x11d2, 0xaf, 0x18, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CReportTemplateCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DReportTemolate =
		{ 0xb0a92ba4, 0x41ac, 0x11d2, { 0xaf, 0x18, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
const IID BASED_CODE IID_DReportTemolateEvents =
		{ 0xb0a92ba5, 0x41ac, 0x11d2, { 0xaf, 0x18, 0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwReportTemolateOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CReportTemplateCtrl, IDS_REPORTTEMOLATE, _dwReportTemolateOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CReportTemplateCtrl::CReportTemplateCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CReportTemplateCtrl

BOOL CReportTemplateCtrl::CReportTemplateCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_REPORTTEMOLATE,
			IDB_REPORTTEMOLATE,
			afxRegInsertable | afxRegApartmentThreading,
			_dwReportTemolateOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CReportTemplateCtrl::CReportTemplateCtrl - Constructor

CReportTemplateCtrl::CReportTemplateCtrl() {
	InitializeIIDs(&IID_DReportTemolate, &IID_DReportTemolateEvents);
	m_CurrentItem=NULL;
    m_LockChangeAction=FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CReportTemplateCtrl::~CReportTemplateCtrl - Destructor

CReportTemplateCtrl::~CReportTemplateCtrl() {
}


/////////////////////////////////////////////////////////////////////////////
// CReportTemplateCtrl::OnDraw - Drawing function

void CReportTemplateCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
}


/////////////////////////////////////////////////////////////////////////////
// CReportTemplateCtrl::DoPropExchange - Persistence support

void CReportTemplateCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CReportTemplateCtrl::OnResetState - Reset control to default state

void CReportTemplateCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CReportTemplateCtrl::AboutBox - Display an "About" box to the user

void CReportTemplateCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_REPORTTEMOLATE);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CReportTemplateCtrl message handlers



//-------------------------------------------------------------------------------
// Private function called recursively to load the templates tree from the
// archive. We need to supply parent item ID, the way the new node will be
// connected to the parent(if child==TRUE) it will be connected as a child,
// if child==FALSE  it will be connected as a sibling) and source stream (from).
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::LoadTree(HTREEITEM current,int connect, CArchive& from) {
CReportNode* newNode;
HTREEITEM    nodeID;
BOOL hasChild;
BOOL hasSibling;

    // Creates new node and load it's fields.
    TRY {
         from >> newNode;
         from >> hasChild;
		 from >> hasSibling;
	} CATCH (CArchiveException, ae) {
	     return;
	}
    END_CATCH

    // Inserts this node into our tree control
    nodeID=InsertIntoTree(current,connect,newNode);

	// We will try to load next child, if available
    if(hasChild==TRUE) 
		LoadTree(nodeID,CHILD,from);
	// If no childs available, we will try toload node sibling
    if(hasSibling==TRUE) 
		LoadTree(nodeID,SIBLING,from);
	// If this is a leaf, we will go one level up
	return;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This public function will be called every time the user asks to load new 
// template.
// It will call LoadTree recursive function to load the tree.
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::LoadTemplatesTree(CString& filename) {
CFile file(filename,CFile::modeRead);
CArchive archive(&file,CArchive::load);
   m_CurrentItem=NULL;
   LoadTree(NULL,CHILD,archive);
   archive.Close();
   file.Close();
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This function will save the template tree recursively node by node trying
// to go into sub-tree first and to the sibling next.
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::SaveTree(HTREEITEM nodeID, CArchive& to) {
CReportNode* node;
BOOL hasChild;
BOOL hasSibling;


    // We have nothing to save;
    if(nodeID==NULL) return;

	// Saving node data to the stream
    node=(CReportNode*)m_DocTree.GetItemData(nodeID);
    to << node;
	// Saving connections flags to the stream
	if(m_DocTree.GetChildItem(nodeID)!=NULL)       hasChild   = TRUE;
	else                                           hasChild   = FALSE;
	if(m_DocTree.GetNextSiblingItem(nodeID)!=NULL) hasSibling = TRUE;
	else                                           hasSibling = FALSE;
	to << hasChild;
	to << hasSibling;

	// We will try to save child at first
	if(hasChild==TRUE) {
	   SaveTree(m_DocTree.GetChildItem(nodeID),to);
	}
	// Then we will try to save sibling
	if(hasSibling==TRUE) {
	   SaveTree(m_DocTree.GetNextSiblingItem(nodeID),to);
	}
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This public function will be called every time the user asks to save current 
// template.
// It will call SaveTree recursive function to save the tree.
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::SaveTemplatesTree(CString& filename) {
CFile file(filename,CFile::modeCreate | CFile::modeWrite);
CArchive archive(&file,CArchive::store);
	SaveTree(m_DocTree.GetRootItem(),archive);
    archive.Close();
    file.Close();
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This function will be called  by WINDOWS when ActiveX control changing
// it's status into active.
//-------------------------------------------------------------------------------
int CReportTemplateCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_DocTree.Create(WS_BORDER|WS_VISIBLE |  TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_LINESATROOT | TVS_DISABLEDRAGDROP,
		           CRect(lpCreateStruct->x+2,
		                 lpCreateStruct->y+2,
			             lpCreateStruct->x+lpCreateStruct->cx-4,
					     lpCreateStruct->y+lpCreateStruct->cy-4),
			       this,
				   IDC_DOCTREE);
	m_DocTree.m_Parent=this;
	m_DocTree.m_DropTarget=&m_DropTarget;
	m_DocTree.ModifyStyleEx(0,WS_EX_CLIENTEDGE);
	m_DropTarget.Register(&m_DocTree);

	// Creating our image list and connecting it to the tree control
	m_ImageList.Create(16,16,FALSE,3,0);

	CBitmap textBmp;
	if(textBmp.LoadBitmap(IDB_USERTEXT)==FALSE) AfxMessageBox("Error loading static text bitmap");

	CBitmap attribBmp;
	if(attribBmp.LoadBitmap(IDB_ATTRIBUTE)==FALSE) AfxMessageBox("Error loading command bitmap");

	CBitmap loopBmp;
	if(loopBmp.LoadBitmap(IDB_LOOP)==FALSE)   AfxMessageBox("Error loading query bitmap");

	CBitmap queryBmp;
	if(queryBmp.LoadBitmap(IDB_QUERY)==FALSE)    AfxMessageBox("Error loading query bitmap");

	CBitmap switchBmp;
	if(switchBmp.LoadBitmap(IDB_SWITCH)==FALSE) AfxMessageBox("Error loading query bitmap");


	m_ImageList.Add(&textBmp,   RGB(0,0,0));
	m_ImageList.Add(&attribBmp,RGB(0,0,0));
	m_ImageList.Add(&loopBmp,   RGB(0,0,0));
	m_ImageList.Add(&queryBmp,RGB(0,0,0));
	m_ImageList.Add(&switchBmp, RGB(0,0,0));
	m_DocTree.SetImageList(&m_ImageList,TVSIL_NORMAL); 

	m_DropTarget.m_Parent=this;
	
	return 0;
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// Callback runs when ActiveX changed size. We will position tree control to
// take all window space.
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::OnSize(UINT nType, int cx, int cy)  {
	COleControl::OnSize(nType, cx, cy);
	if(::IsWindow(m_DocTree)) m_DocTree.MoveWindow(0,0,cx,cy,TRUE);
	
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// Private function will insert item with the text tag "tag", type "type" , 
// pointer to the tree node class CReportNode "data" into  the tree control 
// attaching it to the  current item as a next item if subTree == FALSE or as a 
// child item if subTree==TRUE.
// It will automatically update child and sibling flags of the current node.
//-------------------------------------------------------------------------------
HTREEITEM CReportTemplateCtrl::InsertIntoTree(HTREEITEM current, 
										      int connect,
										      CReportNode* data) {
TV_INSERTSTRUCT tvInsert;
TV_ITEM tvItem;


    
    // Forming item descriptor with text, image and data fields valid.
	tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE  |TVIF_PARAM;
    tvItem.pszText        = (char *)(data->GetTag().GetBuffer(20));
	tvItem.cchTextMax     = data->GetTag().GetLength();
	tvItem.iImage         = data->GetNodeType();
	tvItem.iSelectedImage = data->GetNodeType();
	tvItem.lParam         = (long)data;


	switch(connect) {
	   case SIBLING : tvInsert.hParent      = m_DocTree.GetParentItem(current);
	                  tvInsert.hInsertAfter = current;
					  break;
	   case PARENT :  tvInsert.hParent      = m_DocTree.GetParentItem(current);
	                  tvInsert.hInsertAfter = m_DocTree.GetParentItem(current);
					  break;
	   case CHILD   : tvInsert.hParent = current;
			          tvInsert.hInsertAfter = TVI_FIRST;		  
					  break;
	}
    tvInsert.item         = tvItem;
    return m_DocTree.InsertItem(&tvInsert);
}
//-------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// This message processing function will be called every time the selected tree node changed. It will fire
//---------------------------------------------------------------------------------------------------------
void CReportTemplateCtrl::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)  {
CString query;
CReportNode* curNode; 
  
    if(m_LockChangeAction==FALSE)  {
       m_CurrentItem=m_DocTree.GetSelectedItem();
	   if(m_CurrentItem!=NULL) {
          curNode=(CReportNode *)m_DocTree.GetItemData(m_CurrentItem);
	      FireSelectionChanged(curNode->GetTag(),curNode->GetText(),curNode->GetNodeType());
	   }
    }
	*pResult = 0;
}
//----------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This method will add sub-tree to the current tree node
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::AddChild(short type, LPCTSTR tag, LPCTSTR text)  {
CReportNode* newNode;

   m_CurrentItem=m_DocTree.GetSelectedItem();
   newNode = new CReportNode;
   // Filling the new node attributes
   newNode->SetNodeType(type);
   newNode->SetTag(CString(tag));
   newNode->SetText(CString(text));
   m_CurrentItem=InsertIntoTree(m_CurrentItem,CHILD,newNode);
   m_DocTree.Select(m_CurrentItem,TVGN_CARET );
   m_DocTree.Invalidate(TRUE);
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This method will add sibling to the current tree node.
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::AddSibling(short type, LPCTSTR tag, LPCTSTR text)  {
CReportNode* newNode;

   newNode = new CReportNode;
   // Filling the new node attributes
   newNode->SetNodeType(type);
   newNode->SetTag(CString(tag));
   newNode->SetText(CString(text));
   m_CurrentItem=InsertIntoTree(m_CurrentItem,SIBLING,newNode);
   m_DocTree.Select(m_CurrentItem,TVGN_CARET );
   m_CurrentItem=m_DocTree.GetSelectedItem();
   m_DocTree.Invalidate(TRUE);
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This METHOD will write the tree from the memory into the file with
// file name "fname" writing the childeren first and the siblings next.
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::Save(LPCTSTR fname)  {
    SaveTemplatesTree(CString(fname));
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This METHOD will read the tree into the memory from the file with filename 
// "fname". Will return TRUE if OK or FALSE if the file doesn't exist or
// it's type is wrong or corrupted.
//-------------------------------------------------------------------------------
BOOL CReportTemplateCtrl::Load(LPCTSTR fname)  {
HTREEITEM p;
    m_LockChangeAction=TRUE;
    p=m_DocTree.GetRootItem();
	while(p!=NULL) {
		FreeItem(p);
		p=m_DocTree.GetNextSiblingItem(p);
	}
	m_DocTree.DeleteAllItems();
    m_LockChangeAction=FALSE;
    LoadTemplatesTree(CString(fname));
	m_CurrentItem=m_DocTree.GetRootItem();
    m_DocTree.Select(m_CurrentItem,TVGN_CARET);
	return TRUE;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This local function called recussively to remove our internal item 
// classes.
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::FreeItem(HTREEITEM nodeID) {
CReportNode* node;
   
	if(nodeID==NULL) return;

    node=(CReportNode*)m_DocTree.GetItemData(nodeID);
	if(m_DocTree.GetChildItem(nodeID)!=NULL) {
       level++;
	   FreeItem(m_DocTree.GetChildItem(nodeID));
	   level--;
	}
	if(m_DocTree.GetNextSiblingItem(nodeID)!=NULL && level>0) {
	   FreeItem(m_DocTree.GetNextSiblingItem(nodeID));
	}
	CString tag=node->GetTag();
	delete node;
    m_DocTree.SetItemData(nodeID,NULL);
	FireNodeRemoving(tag);
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This METHOD will remove complete sub-tree which root is currently
// selected element. 
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::DeleteCurrent()  {

    m_CurrentItem=m_DocTree.GetSelectedItem();
	if(m_CurrentItem==NULL) return;
	DeleteItem(m_CurrentItem);
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This function will remove complete sub-tree with root mentioned.
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::DeleteItem(HTREEITEM item)  {
	level=0;
	FreeItem(item);
	m_DocTree.DeleteItem(item);
	m_CurrentItem=m_DocTree.GetSelectedItem();
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This function will save all nodes of the tree in the form of XML.
// This XML will contain queries, so we will call it DiscoverActivePage.
// The result will be the argument for the server query.
//-------------------------------------------------------------------------------
void CReportTemplateCtrl::WriteXML(HTREEITEM nodeID,CString& result) {
CReportNode* node;

    // If node ID is NULL, we will start from the root
    if(nodeID==NULL) nodeID=m_DocTree.GetRootItem();
    node=(CReportNode*)m_DocTree.GetItemData(nodeID);

	node->WriteHeader(result);
	if(m_DocTree.GetChildItem(nodeID)!=NULL) {
	   WriteXML(m_DocTree.GetChildItem(nodeID),result);
	}
    node->WriteFooter(result);
	if(m_DocTree.GetNextSiblingItem(nodeID)!=NULL) {
	   WriteXML(m_DocTree.GetNextSiblingItem(nodeID),result);
	}

}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This METHOD will generate XML string from the tree. In reality, this
// XML string is not ready to parse into HTML string as it contain
// the Access queries. It must be first processed by server to form
// real XML document and only after this we can parse this string into
// HTML. You can think about this string as if it is MS ActivePage.
//-------------------------------------------------------------------------------
BSTR CReportTemplateCtrl::MakeXML()  {
CString strResult;
    if(m_DocTree.GetRootItem()!=NULL) {
       strResult="<XML>\r\n";
       WriteXML(NULL,strResult);
       strResult+="</XML>\r\n";
	} else {
       strResult="<XML>\r\n";
       strResult+="</XML>\r\n";
	}
	return strResult.AllocSysString();
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// This METHOD will set the current node type,node tag and node text. 
// Will do nothing if current node is not selected and will return FALSE.
//-------------------------------------------------------------------------------
BOOL CReportTemplateCtrl::ChangeCurrent(short type, LPCTSTR tag, LPCTSTR text)  {
CReportNode* curNode;
   
    m_CurrentItem=m_DocTree.GetSelectedItem();
	if((m_DocTree.GetRootItem()==NULL) || (m_CurrentItem==NULL)) return FALSE;
    curNode=(CReportNode *)m_DocTree.GetItemData(m_CurrentItem);
    curNode->SetTag(CString(tag));
    curNode->SetText(CString(text));
	curNode->SetNodeType(type);
	m_DocTree.SetItemText(m_CurrentItem,CString(tag));
	m_DocTree.SetItemImage(m_CurrentItem,type,type);
	return TRUE;
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This METHOD will return current node type or -1 if no current node selected.
//-------------------------------------------------------------------------------
short CReportTemplateCtrl::GetType()  {
CReportNode* curNode;   

	if((m_DocTree.GetRootItem()==NULL) || (m_CurrentItem==NULL)) return -1;
    curNode=(CReportNode *)m_DocTree.GetItemData(m_CurrentItem);
	return curNode->GetNodeType();
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This METHOD will return current node tag or empty string if no 
// current node selected.
//-------------------------------------------------------------------------------
BSTR CReportTemplateCtrl::GetTag()  {
CReportNode* curNode;   
CString strResult;

	if((m_DocTree.GetRootItem()==NULL) || (m_CurrentItem==NULL)) {
		strResult="";
	} else {
        curNode=(CReportNode *)m_DocTree.GetItemData(m_CurrentItem);
		strResult=curNode->GetTag();
	}
	return strResult.AllocSysString();
}
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// This METHOD will return current node text or empty string if no 
// current node selected.
//-------------------------------------------------------------------------------
BSTR CReportTemplateCtrl::GetText()  {
CReportNode* curNode;   
CString strResult;

	if((m_DocTree.GetRootItem()==NULL) || (m_CurrentItem==NULL) ) {
		strResult="";
	} else {
        curNode=(CReportNode *)m_DocTree.GetItemData(m_CurrentItem);
		strResult=curNode->GetText();
	}
	return strResult.AllocSysString();
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This function is used to recursively pass the tree and collect all available
// tags, The duplicated tags will not be included into the string.
// All tags will be devided by "\n" in collector string.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::NodeTag(HTREEITEM nodeID, CString& collector) {
CString      curTag;
CReportNode* node;
      if(m_DocTree.GetRootItem()==NULL) return;
      if(nodeID==NULL) nodeID=m_DocTree.GetRootItem();
	  node=(CReportNode*)m_DocTree.GetItemData(nodeID);
      curTag=node->GetTag();
	  curTag+="\r\n";
	  // Maybe this tag is already in the collector string?
	  int res = collector.Find(curTag);
	  if(res==-1) {
		  // If not, we will add new tag to the collector string.
		  // with \n separator
		  collector+=curTag;
	  }
      if(m_DocTree.GetChildItem(nodeID)!=NULL) {
	      NodeTag(m_DocTree.GetChildItem(nodeID),collector);
	  }
	  if(m_DocTree.GetNextSiblingItem(nodeID)!=NULL) {
	      NodeTag(m_DocTree.GetNextSiblingItem(nodeID),collector);
	  }
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This method can be used to obtain complete list of tags used in the tree.
//---------------------------------------------------------------------------------
BSTR CReportTemplateCtrl::GetTagsList()  {
CString strResult;
    NodeTag(NULL,strResult);
	return strResult.AllocSysString();
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// Removes everything from the tree
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::Clear()  {
HTREEITEM p;
    m_LockChangeAction=TRUE;
    p=m_DocTree.GetRootItem();
	while(p!=NULL) {
		FreeItem(p);
		p=m_DocTree.GetNextSiblingItem(p);
	}
	m_DocTree.DeleteAllItems();
    m_LockChangeAction=FALSE;
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This private function will iterate the Tree Control structure forming sub-tree
// by recurse calls.
//---------------------------------------------------------------------------------
CSubTreeNode* CReportTemplateCtrl::SubTreeExtractor(HTREEITEM controlNode, CSubTreeNode* treeNode, int connection) {
CSubTreeNode* newTreeNode;
CSubTreeNode* nodeList;
CSubTreeNode* p;
CReportNode* node;
      // Creating a new node
      newTreeNode = new CSubTreeNode;
      // Extracting information about this node from the tree
	  node=(CReportNode*)m_DocTree.GetItemData(controlNode);
	  // setting this information into our new node
      newTreeNode->SetNodeType(node->GetNodeType());
	  m_TagsList+=node->GetTag();
	  m_TagsList+="\n";
      newTreeNode->SetTag(node->GetTag());
      newTreeNode->SetText(node->GetText());
	  switch(connection) {
	      case ROOT  : m_SubTreeLevel=0;
			           break;
		  case CHILD : // The parent of our new node is our current node
			           newTreeNode->SetParent(treeNode);
			           nodeList=treeNode->GetChildList();
			           if(nodeList==NULL) {
						   treeNode->SetChildList(newTreeNode);
						   newTreeNode->SetPrevSibling(NULL);
						   newTreeNode->SetNextSibling(NULL);
					   } else { // If the parent node already has the node 
						        // list we will look for the lsat node in it
						   p=nodeList;
						   while(p->GetNextSibling()) p->GetNextSibling();
						   // And we will attach our node to this list
						   p->SetNextSibling(newTreeNode);
						   newTreeNode->SetNextSibling(NULL);
						   newTreeNode->SetPrevSibling(p);
					   }
					   break;
		  case SIBLING:// Current node and our new one will have the same parent.
			           newTreeNode->SetParent(treeNode->GetParent());
					   // We will look throw the current list to find the last node
					   p=treeNode;
					   while(p->GetNextSibling()) p->GetNextSibling();
					   // Connecting our new node to the tree
					   p->SetNextSibling(newTreeNode);
	           		   newTreeNode->SetNextSibling(NULL);
					   newTreeNode->SetPrevSibling(p);
					   break;
	  }
      // Recursive calls depending on the TreeControl tree structure.
      if(m_DocTree.GetChildItem(controlNode)!=NULL) {
		  m_SubTreeLevel++;
	      SubTreeExtractor(m_DocTree.GetChildItem(controlNode),newTreeNode,CHILD);
		  m_SubTreeLevel--;
	  }
	  if(m_DocTree.GetNextSiblingItem(controlNode)!=NULL && m_SubTreeLevel>0) {
	      SubTreeExtractor(m_DocTree.GetNextSiblingItem(controlNode),newTreeNode,SIBLING);
	  }
	  return newTreeNode;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This private function will iterate our sub-tree and will add it's data
// to the TreeControl tree.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::SubTreeEmbagger(HTREEITEM controlNode, CSubTreeNode* treeNode, int connect) {
HTREEITEM newControlNode;
TV_INSERTSTRUCT tvInsert;
TV_ITEM tvItem;
CReportNode* node;

    // Creating new structure connected to the TreeControl
    node=new CReportNode;

    // Filling this structure with information from the sub-tree
	node->SetNodeType(treeNode->GetNodeType());
	node->SetTag(treeNode->GetTag());
	node->SetText(treeNode->GetText());
    
    // Creating control item structure
	tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE  |TVIF_PARAM;
    tvItem.pszText        = (char *)(node->GetTag().GetBuffer(20));
	tvItem.cchTextMax     = node->GetTag().GetLength();
	tvItem.iImage         = node->GetNodeType();
	tvItem.iSelectedImage = node->GetNodeType();
	tvItem.lParam         = (long)node;


    // Connecting item to the ControlTree
	switch(connect) {
	   case SIBLING : tvInsert.hParent      = m_DocTree.GetParentItem(controlNode);
	                  tvInsert.hInsertAfter = controlNode;
					  break;
	   case CHILD   : tvInsert.hParent = controlNode;
			          tvInsert.hInsertAfter = TVI_FIRST;		  
					  break;
	}
    tvInsert.item         = tvItem;
    newControlNode=m_DocTree.InsertItem(&tvInsert);


    // Recursive calls depending on the TreeControl tree structure.
    if(treeNode->GetChildList()!=NULL) {
       SubTreeEmbagger(newControlNode,treeNode->GetChildList(),CHILD);
	}
	if(treeNode->GetNextSibling()!=NULL) {
	   SubTreeEmbagger(newControlNode,treeNode->GetNextSibling(),SIBLING);
	}
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This private function will iterate our sub-tree and will remove all items.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::SubTreeRemover(CSubTreeNode* treeNode) {
    // Recursive calls depending on the TreeControl tree structure.
    if(treeNode->GetChildList()!=NULL) {
       SubTreeRemover(treeNode->GetChildList());
	}
	if(treeNode->GetNextSibling()!=NULL) {
	   SubTreeRemover(treeNode->GetNextSibling());
	}
	delete treeNode;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This function will create a sub-tree containing "from" node as a root
// with all children connected,
//---------------------------------------------------------------------------------
CSubTreeNode* CReportTemplateCtrl::ExtractSubTree(HTREEITEM from) {
    m_TagsList="";
	return SubTreeExtractor(from,NULL,ROOT);
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This function will connect the "tree" sub-tree to the current node.
// If the flag "asChild" is set to TRUE, it will add the sub-tree as a children,
// if the flag "asChild" is set to FALSE, it will add the sub-tree as a sibling.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::InsertSubTree(CSubTreeNode* tree, HTREEITEM to) {
	SubTreeEmbagger(to,tree,CHILD);
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This function will connect the "tree" sub-tree to the current node.
// If the flag "asChild" is set to TRUE, it will add the sub-tree as a children,
// if the flag "asChild" is set to FALSE, it will add the sub-tree as a sibling.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::AddSubTree(CSubTreeNode* tree, HTREEITEM to) {
	SubTreeEmbagger(to,tree,SIBLING);
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This function will remove all elements from a sub-tree
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::RemoveSubTree(CSubTreeNode* tree) {
	SubTreeRemover(tree);
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::AddParent(short type, LPCTSTR tag, LPCTSTR text)  {
CReportNode* node;
CSubTreeNode* subTree;
HTREEITEM parent;
HTREEITEM sibling;

   m_CurrentItem=m_DocTree.GetSelectedItem();
   if(m_CurrentItem!=NULL) {
      // Extracting complete sub-tree starting with the current node
      subTree=ExtractSubTree(m_CurrentItem);

      // Removing old item, but saving it's tree place
      parent  = m_DocTree.GetParentItem(m_CurrentItem);
      sibling = m_DocTree.GetPrevSiblingItem(m_CurrentItem);
      FreeItem(m_CurrentItem);
      m_DocTree.DeleteItem(m_CurrentItem);

      // Creating and attaching new item to the removed item position
      node=new CReportNode;
      node->SetNodeType(type);
      node->SetTag(CString(tag));
      node->SetText(CString(text));
      if(sibling==NULL) 
          m_CurrentItem = InsertIntoTree(parent,CHILD,node);
      else
          m_CurrentItem = InsertIntoTree(sibling,SIBLING,node);

      // Attaching sub-tree to the current item
      InsertSubTree(subTree,m_CurrentItem);
      m_DocTree.Select(m_CurrentItem,TVGN_CARET );
      m_DocTree.Invalidate(TRUE); 

      // Removing temporary sub-tree
      RemoveSubTree(subTree);
   } else  {
       // Creating and attaching new item to the removed item position
       node=new CReportNode;
       node->SetNodeType(type);
       node->SetTag(CString(tag));
       node->SetText(CString(text));
 	   m_CurrentItem = InsertIntoTree(NULL,CHILD,node);
   }


}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// Need this to unregister this window as a drag-and-drop target.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::OnClose()  {
	CWnd::OnClose();
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// Returnns the node ID if any node is in the point or NULL if no nodes.
//---------------------------------------------------------------------------------
HTREEITEM CReportTemplateCtrl::NodeFromPoint(CPoint & point) {
UINT flags;
HTREEITEM item;

    item = m_DocTree.HitTest(point,&flags);
	if(flags==TVHT_ONITEMLABEL) {
		return item;        
	}
	return NULL;
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This function will mark the item as a target of drag-and-drop operation.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::MarkDropTarget(HTREEITEM newItem,HTREEITEM oldItem) {
    if(newItem!=NULL) m_DocTree.SetItemState(newItem,TVIS_BOLD,TVIS_BOLD);
    if(oldItem!=NULL) m_DocTree.SetItemState(oldItem,0,TVIS_BOLD);
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This private function will iterate our sub-tree and will remove all items.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::SubTreeSaver(CSubTreeNode* treeNode,CArchive& to) {
BOOL hasChild;
BOOL hasSibling;

    // We have nothing to save;
    if(treeNode==NULL) return;

    to << treeNode;
	// Saving connections flags to the stream
	if(treeNode->GetChildList())         hasChild   = TRUE;
	else                                 hasChild   = FALSE;
	if(treeNode->GetNextSibling()!=NULL) hasSibling = TRUE;
	else                                 hasSibling = FALSE;
	to << hasChild;
	to << hasSibling;
    
    // Recursive calls depending on the TreeControl tree structure.
    if(hasChild) {
       SubTreeSaver(treeNode->GetChildList(),to);
	}
	if(hasSibling) {
	   SubTreeSaver(treeNode->GetNextSibling(),to);
	}
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This private function will iterate our sub-tree and will remove all items.
//---------------------------------------------------------------------------------
CSubTreeNode* CReportTemplateCtrl::SubTreeLoader(CSubTreeNode* treeNode,CArchive& from, int connect) {
BOOL hasChild;
BOOL hasSibling;
CSubTreeNode* newTreeNode;
CSubTreeNode* nodeList;
CSubTreeNode* p;
CReportNode* node;

    // Creates new node and load it's fields.
    TRY {
         from >> node;
         from >> hasChild;
		 from >> hasSibling;
	} CATCH (CArchiveException, ae) {
	     return NULL;
	}
    END_CATCH
    newTreeNode=new CSubTreeNode;
	newTreeNode->SetNodeType(node->GetNodeType());
	newTreeNode->SetTag(node->GetTag());
	newTreeNode->SetText(node->GetText());
	delete node;


    switch(connect) {
	      case ROOT  : break;
		  case CHILD : // The parent of our new node is our current node
			           newTreeNode->SetParent(treeNode);
			           nodeList=treeNode->GetChildList();
			           if(nodeList==NULL) {
						   treeNode->SetChildList(newTreeNode);
						   newTreeNode->SetPrevSibling(NULL);
						   newTreeNode->SetNextSibling(NULL);
					   } else { // If the parent node already has the node 
						        // list we will look for the lsat node in it
						   p=nodeList;
						   while(p->GetNextSibling()) p->GetNextSibling();
						   // And we will attach our node to this list
						   p->SetNextSibling(newTreeNode);
						   newTreeNode->SetNextSibling(NULL);
						   newTreeNode->SetPrevSibling(p);
					   }
					   break;
		  case SIBLING:// Current node and our new one will have the same parent.
			           newTreeNode->SetParent(treeNode->GetParent());
					   // We will look throw the current list to find the last node
					   p=treeNode;
					   while(p->GetNextSibling()) p->GetNextSibling();
					   // Connecting our new node to the tree
					   p->SetNextSibling(newTreeNode);
	           		   newTreeNode->SetNextSibling(NULL);
					   newTreeNode->SetPrevSibling(p);
					   break;
  }
  // Recursive calls depending on the TreeControl tree structure.
  if(hasChild) {
      SubTreeLoader(newTreeNode,from,CHILD);
  }
  if(hasSibling) {
      SubTreeLoader(newTreeNode,from,SIBLING);
  }
  return newTreeNode;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This function will create sub-tree from the root, will save it on disk and
// will return filename.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::MakeSubTreeFile(HTREEITEM root,CString& filename) {
CFile file(filename,CFile::modeCreate | CFile::modeWrite);;
CArchive archive(&file,CArchive::store);
CSubTreeNode* subRoot;

   subRoot=ExtractSubTree(root);
   SubTreeSaver(subRoot,archive);

}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This function will create sub-tree from the root, will save it on disk and
// will return filename.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::MakeAttributesFile(HTREEITEM root,CString& filename) {
	 FireAskAttributeFile(filename,m_TagsList);
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This function will create sub-tree from the file, will add this sub-tree as
// a "to" node child.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::AddSubTreeFileAsChild(HTREEITEM to,CString& filename) {
CFile file(filename,CFile::modeRead);
CArchive archive(&file,CArchive::load);
CSubTreeNode* subRoot;

     subRoot=SubTreeLoader(NULL,archive,ROOT);
     InsertSubTree(subRoot,to);

}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// This function will create sub-tree from the file, will add this sub-tree as
// a "to" node sibling.
//---------------------------------------------------------------------------------
void CReportTemplateCtrl::AddSubTreeFileAsSibling(HTREEITEM to,CString& filename) {
CFile file(filename,CFile::modeRead);
CArchive archive(&file,CArchive::load);
CSubTreeNode* subRoot;
     subRoot=SubTreeLoader(NULL,archive,ROOT);
     AddSubTree(subRoot,to);

}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This private function designed for the recursive tree search
//---------------------------------------------------------------------------------
HTREEITEM CReportTemplateCtrl::TreeSearch(HTREEITEM item, CString & token) {
HTREEITEM result=NULL;
CReportNode* node;

    if(item==NULL) return NULL;
    node=(CReportNode*)m_DocTree.GetItemData(item);
	if(node!=NULL) {
	   if(node->GetTag()==token) {
		   return item;
	   }
	}

	if(m_DocTree.GetChildItem(item)!=NULL) {
	   result=TreeSearch(m_DocTree.GetChildItem(item),token);
	}
	if(m_DocTree.GetNextSiblingItem(item)!=NULL) {
	   result=TreeSearch(m_DocTree.GetNextSiblingItem(item),token);
	}
	return result;
}
//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
// This METHOD will try to find the node with the token specified and will
// return HTREEITEM of this node if OK or NULL if no token was found
//---------------------------------------------------------------------------------
long CReportTemplateCtrl::FindToken(LPCTSTR token)  {
HTREEITEM result;
    result=TreeSearch(m_DocTree.GetRootItem(),CString(token));
	return (long)result;
}
//---------------------------------------------------------------------------------


void CReportTemplateCtrl::SayAttributesNeeded(CString& name) {
	FireIncludeAttributeFile(name);

}
