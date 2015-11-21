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
// DiscoverMDIDoc.cpp : implementation of the CDiscoverMDIDoc class
//

#include "stdafx.h"
#include "DiscoverMDI.h"

#include "DiscoverMDIDoc.h"
#include "CntrItem.h"
#include "SrvrItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIDoc

IMPLEMENT_DYNCREATE(CDiscoverMDIDoc, COleServerDoc)

BEGIN_MESSAGE_MAP(CDiscoverMDIDoc, COleServerDoc)
	//{{AFX_MSG_MAP(CDiscoverMDIDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Enable default OLE container implementation
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleServerDoc::OnUpdatePasteMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleServerDoc::OnUpdatePasteLinkMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleServerDoc::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_OLE_EDIT_CONVERT, COleServerDoc::OnEditConvert)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleServerDoc::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, COleServerDoc::OnEditLinks)
	ON_UPDATE_COMMAND_UI(ID_OLE_VERB_FIRST, COleServerDoc::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_FILE_SEND_MAIL, OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateFileSendMail)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CDiscoverMDIDoc, COleServerDoc)
	//{{AFX_DISPATCH_MAP(CDiscoverMDIDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//      DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IDiscoverMDI to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {64FA0D58-261E-11D2-AEFB-00A0C9B71DC4}
static const IID IID_IDiscoverMDI =
{ 0x64fa0d58, 0x261e, 0x11d2, { 0xae, 0xfb, 0x0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };

BEGIN_INTERFACE_MAP(CDiscoverMDIDoc, COleServerDoc)
	INTERFACE_PART(CDiscoverMDIDoc, IID_IDiscoverMDI, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIDoc construction/destruction

CDiscoverMDIDoc::CDiscoverMDIDoc()
{
	// Use OLE compound files
	EnableCompoundFile();

	// TODO: add one-time construction code here

	EnableAutomation();

	AfxOleLockApp();
}

CDiscoverMDIDoc::~CDiscoverMDIDoc()
{
	AfxOleUnlockApp();
}

BOOL CDiscoverMDIDoc::OnNewDocument()
{
	if (!COleServerDoc::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIDoc server implementation

COleServerItem* CDiscoverMDIDoc::OnGetEmbeddedItem()
{
	// OnGetEmbeddedItem is called by the framework to get the COleServerItem
	//  that is associated with the document.  It is only called when necessary.

	CDiscoverMDISrvrItem* pItem = new CDiscoverMDISrvrItem(this);
	ASSERT_VALID(pItem);
	return pItem;
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIDoc ActiveX Document server implementation

CDocObjectServer *CDiscoverMDIDoc::GetDocObjectServer(LPOLEDOCUMENTSITE pDocSite)
{   
	return new CDocObjectServer(this, pDocSite);
}



/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIDoc serialization

void CDiscoverMDIDoc::Serialize(CArchive& ar) {
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}

	// Calling the base class COleServerDoc enables serialization
	//  of the container document's COleClientItem objects.
	COleServerDoc::Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIDoc diagnostics

#ifdef _DEBUG
void CDiscoverMDIDoc::AssertValid() const
{
	COleServerDoc::AssertValid();
}

void CDiscoverMDIDoc::Dump(CDumpContext& dc) const
{
	COleServerDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDiscoverMDIDoc commands

void CDiscoverMDIDoc::OnDeactivateUI(BOOL bUndoable) 
{
	// TODO: Add your specialized code here and/or call the base class
	
   COleServerDoc::OnDeactivateUI(bUndoable);
}

void CDiscoverMDIDoc::OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow)  {
	COleServerDoc::OnShowControlBars(pFrameWnd, bShow);
}

void CDiscoverMDIDoc::OnDocWindowActivate(BOOL bActivate) 
{
		// TODO: Add your specialized code here and/or call the base class
	COleServerDoc::OnDocWindowActivate(bActivate);
}

void CDiscoverMDIDoc::OnShowDocument(BOOL bShow) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	COleServerDoc::OnShowDocument(bShow);
}

void CDiscoverMDIDoc::OnFrameWindowActivate(BOOL bActivate) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	COleServerDoc::OnFrameWindowActivate(bActivate);
}

void CDiscoverMDIDoc::OnShowViews(BOOL bVisible) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	COleServerDoc::OnShowViews(bVisible);
}

