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
#if !defined(AFX_REPORTTEMOLATECTL_H__B0A92BB4_41AC_11D2_AF18_00A0C9B71DC4__INCLUDED_)
#define AFX_REPORTTEMOLATECTL_H__B0A92BB4_41AC_11D2_AF18_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define IDC_DOCTREE 1881
#include "ReportTreeCtrl.h"


#define CHILD   0
#define SIBLING 1
#define PARENT  2
#define ROOT    3
// ReportTemolateCtl.h : Declaration of the CReportTemplateCtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// CReportTemplateCtrl : See ReportTemolateCtl.cpp for implementation.


//-------------------------------------------------------------------------------
// This class represent the node in the report node tree. Report node tree is
// binary tree with the nodes containing Acess query to form set, access commands
// to describe set elements, static text to display and maybe the sub-tree of
// the same possible types. All tree structure is stored in the CTreeCtrl, the
// data pointer of this control is used to point to this class instance.
//-------------------------------------------------------------------------------
class CReportNode : public CObject {
    public  : CReportNode();
			  ~CReportNode();
			  // This function will return 2 if the node is set iterator and
			  // need to form the set by evaluating query (the text templates
			  // instances will appear at the output document for every element
			  // in a set),1 if this node contains query and query results must
			  // must be printed or 0 if the node contain only static text.
			  int GetNodeType();
			  // This function will set the node type - see description above.
			  void SetNodeType(BOOL isIterator);

			  // Returns the node tag. It will be used in a stylesheet fo
			  // format and present the node textual results.
			  CString& GetTag(void);
			  // Sets the node tag. It will be used in a stylesheet fo
			  // format and present the node textual results.
			  void SetTag(CString& text);

			  // Returns the node text the meaning of which depends from the
			  // node type.
			  CString& GetText(void);
			  // Sets the node text the meaning of which depends from the
			  // node type.
			  void SetText(CString& text);

              // This class will support stream operations. We will walk
			  // around the tree to save or restore nodes from the stream.
	          virtual void Serialize(CArchive& ar);

			  // This function will write header into the stream in the
			  // form of:
			  // 1. Static text nodes: <nodename> text
			  // 2. Access commands  : <nodename>
              //                         <COMMAND Query="text"/>
			  // 3. Set iterator     : <nodename>
			  //                           <LOOP Query = "text"/>
			  virtual void WriteHeader(CString& out);

			  // This function will write footer into the text file in the
			  // form of :
			  // 1. Static text nodes: </nodename>
			  // 2. Access commands  : </nodename>
			  // 3. Set iterator     : </LOOP>
			  //                       </nodename>
			  virtual void WriteFooter(CString& out);

    private : 
		      int         m_NodeType;    // This flag indicates the node type.
			                             // The possible walues are:
			                             // 0 - static text
			                             // 1 - Acess command
			                             // 2 - Set loop
			  CString      m_Tag;        // This node tag name
			  CString      m_Text;       // The text assosiated with the node.
			                             // The meaning of this text depends from
			                             // the node type:
			                             // Type           Meaning
			                             //  0       Static text to display
			                             //  1       Acess command (display result)
			                             //  2       Set query (used to form set) 

DECLARE_SERIAL(CReportNode)
};
//-------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This class based on the CReportNode designed to represent any sub-tree separated
// from the control. We need this because of two things: any re-parenting or 
// drag-and-drop operations will work with the selection whoch can be sub-tree and
// we will need sub-trees to store ready components for Wizard and for component 
// gallery. 
//-----------------------------------------------------------------------------------
class CSubTreeNode : public CReportNode {
   public : CSubTreeNode();
			~CSubTreeNode();

			// Gets and sets this node parent
			CSubTreeNode* GetParent();
			void SetParent(CSubTreeNode* newParent);

			// Gets and sets this node child list (the pointer to the first child 
			// in a list)
			CSubTreeNode* GetChildList();
			void SetChildList(CSubTreeNode* newChild);

			// Gets and sets this node next sibling (walking forward in the child 
			// list connected to this node parent)
			CSubTreeNode* GetNextSibling();
			void SetNextSibling(CSubTreeNode* newSibling);

			// Gets and sets this node previous sibling (walking backward in the 
			// child  list connected to this node parent)
			CSubTreeNode* GetPrevSibling();
			void SetPrevSibling(CSubTreeNode* newSibling);
   private:
	       CSubTreeNode* m_Parent;
		   CSubTreeNode* m_ChildList;
		   CSubTreeNode* m_Next;
		   CSubTreeNode* m_Prev;
};
//-----------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------
// We need this class derived from COleDropTarget to owerwrite some drag-and-drop
// finctions.
//-----------------------------------------------------------------------------------
class CReportTemplateCtrl;
class CTreeDropTarget : public COleDropTarget {
public:
	CReportTemplateCtrl* m_Parent;
public:
	// This function will be called when the mouse enter the window while in drag-and
	// drop mode
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd,COleDataObject* pDataObject,DWORD dwKeyState,CPoint point );
	// This function will be called when the mouse move in the window while in drag-and
	// drop mode
	virtual DROPEFFECT OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
	// This function will be called when the user release mouse the window while in 
	// drag-and drop mode
	virtual BOOL OnDrop( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point );
};



class CReportTemplateCtrl : public COleControl
{
	DECLARE_DYNCREATE(CReportTemplateCtrl)

// Constructor
public:
	CReportTemplateCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReportTemplateCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CReportTemplateCtrl();

	DECLARE_OLECREATE_EX(CReportTemplateCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CReportTemplateCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CReportTemplateCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CReportTemplateCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CReportTemplateCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	//}}AFX_MSG
    afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CReportTemplateCtrl)
	afx_msg void AddChild(short type, LPCTSTR tag, LPCTSTR text);
	afx_msg void AddSibling(short type, LPCTSTR tag, LPCTSTR text);
	afx_msg void Save(LPCTSTR fname);
	afx_msg BOOL Load(LPCTSTR fname);
	afx_msg void DeleteCurrent();
	afx_msg BSTR MakeXML();
	afx_msg BOOL ChangeCurrent(short type, LPCTSTR tag, LPCTSTR text);
	afx_msg short GetType();
	afx_msg BSTR GetTag();
	afx_msg BSTR GetText();
	afx_msg BSTR GetTagsList();
	afx_msg void Clear();
	afx_msg void AddParent(short type, LPCTSTR tag, LPCTSTR text);
	afx_msg long FindToken(LPCTSTR token);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CReportTemplateCtrl)
	void FireSelectionChanged(LPCTSTR name, LPCTSTR text, short type)
		{FireEvent(eventidSelectionChanged,EVENT_PARAM(VTS_BSTR  VTS_BSTR  VTS_I2), name, text, type);}
	void FireNodeRemoving(LPCTSTR tag)
		{FireEvent(eventidNodeRemoving,EVENT_PARAM(VTS_BSTR), tag);}
	void FireAskAttributeFile(LPCTSTR name, LPCTSTR nodes)
		{FireEvent(eventidAskAttributeFile,EVENT_PARAM(VTS_BSTR  VTS_BSTR), name, nodes);}
	void FireIncludeAttributeFile(LPCTSTR name)
		{FireEvent(eventidIncludeAttributeFile,EVENT_PARAM(VTS_BSTR), name);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	void SayAttributesNeeded(CString& name);
    void MakeSubTreeFile(HTREEITEM node,CString& filename);	
    void MakeAttributesFile(HTREEITEM root,CString& filename);
    void AddSubTreeFileAsChild(HTREEITEM to,CString& filename);
    void AddSubTreeFileAsSibling(HTREEITEM to,CString& filename);
	void MarkDropTarget(HTREEITEM newItem,HTREEITEM oldItem);
	HTREEITEM NodeFromPoint(CPoint& p);
	// This function will read all report tree structure from the archive.
	void LoadTemplatesTree(CString& filename);
	// This function will write all report tree structure into the archive.
	void SaveTemplatesTree(CString& filename);
	// This function will form XML-like template to run this template on a server
	// (server will execute all access commands and will return back real XML
	// with all commands/queries results in it.)
	void FormAccessXML(CString& output);
	// This function will create a sub-tree containing "from" node as a root
	// with all children connected,
	CSubTreeNode* ExtractSubTree(HTREEITEM from);
	// This function will connect the "tree" sub-tree to the current node 
	// as a child.
	void InsertSubTree(CSubTreeNode* tree, HTREEITEM to);
	// This function will connect the "tree" sub-tree to the current node 
	// as a sibling.
	void AddSubTree(CSubTreeNode* tree, HTREEITEM to);
	// Will remove all items from the existing sub-tree
	void RemoveSubTree(CSubTreeNode* tree);
	// Will remove all sub-tree starting with item
    void DeleteItem(HTREEITEM item);
	enum {
	//{{AFX_DISP_ID(CReportTemplateCtrl)
	dispidAddChild = 1L,
	dispidAddSibling = 2L,
	dispidSave = 3L,
	dispidLoad = 4L,
	dispidDeleteCurrent = 5L,
	dispidMakeXML = 6L,
	dispidChangeCurrent = 7L,
	dispidGetType = 8L,
	dispidGetTag = 9L,
	dispidGetText = 10L,
	dispidGetTagsList = 11L,
	dispidClear = 12L,
	dispidAddParent = 13L,
	dispidFindToken = 14L,
	eventidSelectionChanged = 1L,
	eventidNodeRemoving = 2L,
	eventidAskAttributeFile = 3L,
	eventidIncludeAttributeFile = 4L,
	//}}AFX_DISP_ID
	};
private:
	CString m_TagsList;
	HTREEITEM TreeSearch(HTREEITEM node, CString& token);
	int m_SubTreeLevel;
	BOOL m_LockChangeAction;
	HTREEITEM m_CurrentItem;
	CTreeDropTarget m_DropTarget;



	HTREEITEM InsertIntoTree(HTREEITEM current, 
						     int connect, 
						     CReportNode* data);
	CImageList m_ImageList;
	// This tree control presents complete document structure.
	CReportTreeCtrl m_DocTree;
	void NodeTag(HTREEITEM item, CString& collector);
	void LoadTree(HTREEITEM item, int connection, CArchive& from);
	void SaveTree(HTREEITEM item, CArchive& from);
    void WriteXML(HTREEITEM item, CString& result);
    void FreeItem(HTREEITEM item);
    CSubTreeNode* SubTreeExtractor(HTREEITEM controlNode, CSubTreeNode* treeNode, int connection);
    void SubTreeEmbagger(HTREEITEM controlNode, CSubTreeNode* treeNode, int connection);
    void SubTreeRemover(CSubTreeNode* treeNode);
    void SubTreeSaver(CSubTreeNode* treeNode,CArchive& to);
    CSubTreeNode* SubTreeLoader(CSubTreeNode* treeNode,CArchive& to, int connect);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REPORTTEMOLATECTL_H__B0A92BB4_41AC_11D2_AF18_00A0C9B71DC4__INCLUDED)
