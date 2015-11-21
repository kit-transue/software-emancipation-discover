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
// AttributesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AttributesDialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAttributesDialog dialog


CAttributesDialog::CAttributesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAttributesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAttributesDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Durty=FALSE;
	m_RefillOnCancel=FALSE;
	m_InitialState.SetSize(0,100);
}


void CAttributesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttributesDialog)
	DDX_Control(pDX, IDC_ONE_TO_SOURCE, m_ToLeft);
	DDX_Control(pDX, IDC_ONE_TO_DEST, m_ToRight);
	DDX_Control(pDX, IDC_DESTATTR, m_Dest);
	DDX_Control(pDX, IDC_SOURCEATTR, m_Source);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAttributesDialog, CDialog)
	//{{AFX_MSG_MAP(CAttributesDialog)
	ON_BN_CLICKED(IDC_ONE_TO_DEST, OnOneToDest)
	ON_BN_CLICKED(IDC_ONE_TO_SOURCE, OnOneToSource)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttributesDialog message handlers


//----------------------------------------------------------------------------------------
// We will form the available and selected attributes list asking server for the complete
// attributes list and asking results spreadsheet for the attributes status.
//----------------------------------------------------------------------------------------
BOOL CAttributesDialog::OnInitDialog()  {
CString readableName;
CString accessName;

	CDialog::OnInitDialog();
	// No filling lists if no query results destination window
	if(m_QueryResults==NULL) return TRUE;

	for(register i=0;i<26;i++) { //****** This is emulator! 
		                         //****** Ask Alex Z about real access commands!
	   readableName.LoadString(IDS_FILE_COLUMN+i);
	   accessName.LoadString(IDS_FILE_COMMAND+i);
	   int index = m_QueryResults->CheckAttribute(readableName,accessName);
	   if(index!=-1) {
		   m_InitialState.SetAtGrow(index,readableName);
	   } else {
	       m_Source.AddString(readableName); 
	   }
	}
	// Fill the destination list box from the initial state array
	FillListFromInitialState();

	m_ArLeft.LoadMappedBitmap(IDB_ARROWLEFT);
	m_ToLeft.SetBitmap(m_ArLeft);
	m_ArRight.LoadMappedBitmap(IDB_ARROWRIGHT);
	m_ToRight.SetBitmap(m_ArRight);
	return TRUE;  
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Reply to the right arrow button click. Will remove the attribute from the left source
// column and will place it to the right destination column. 
//-----------------------------------------------------------------------------------------
void CAttributesDialog::OnOneToDest()  {
CString readableName;
CString accessName;
int index;

	if(m_QueryResults==NULL) return;
    index = m_Source.GetCurSel();
    if(index<0) return;
    m_Source.GetText(index,readableName);
    accessName=AccessFromName(readableName);
    m_Source.DeleteString(index);
    m_Dest.AddString(readableName);
    m_Source.SetCurSel(index);
    m_QueryResults->AddAttribute(readableName,accessName);
    m_Durty=TRUE;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Reply to the left arrow button click. Will remove the attribute from the right 
// destination column and will place it to the left source column. 
//-----------------------------------------------------------------------------------------
void CAttributesDialog::OnOneToSource() {
CString readableName;
CString accessName;
int index;

	if(m_QueryResults==NULL) return;
    index = m_Dest.GetCurSel();
    if(index<0) return;
    m_Dest.GetText(index,readableName);
    accessName=AccessFromName(readableName);
    m_Dest.DeleteString(index);
    m_Source.AddString(readableName); 
    m_Dest.SetCurSel(index);
    m_QueryResults->RemoveAttribute(readableName,accessName);
    m_Durty=TRUE;
	
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Callback runs when Apply button pressed.
//-----------------------------------------------------------------------------------------
void CAttributesDialog::OnApply()  {
	if(m_QueryResults==NULL) return;
    if(m_Durty==TRUE) m_QueryResults->ApplyAttributeChanges();
    m_Durty=FALSE;
    m_RefillOnCancel=TRUE;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Callback runs when Apply button pressed.
//-----------------------------------------------------------------------------------------
void CAttributesDialog::OnOK()  {
   if(m_QueryResults!=NULL) {
     if(m_Durty==TRUE) m_QueryResults->ApplyAttributeChanges();
   }
   CDialog::OnOK();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Callabck runs when Cancel button pressed.
//-----------------------------------------------------------------------------------------
void CAttributesDialog::OnCancel()  {
CString readableName;
CString accessName;
    
   if(m_QueryResults!=NULL) {
      if(m_RefillOnCancel==TRUE) {
         for(register i=0;i<27;i++) { 
	         readableName.LoadString(IDS_FILE_COLUMN+i);
	         accessName.LoadString(IDS_FILE_COMMAND+i);
             m_QueryResults->RemoveAttribute(readableName,accessName);
         }
         FillListFromInitialState();
	     m_QueryResults->ApplyAttributeChanges();
      } else {
	     if(m_Durty == TRUE ) m_QueryResults->CancelAttributeChanges();
	     m_Durty=FALSE;
      }
   }
   CDialog::OnCancel();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Converting the readable attribute name into it's access option. This function must be 
// changed when Alex Z will add access commands to get attributes with names from the server
//-----------------------------------------------------------------------------------------
CString& CAttributesDialog::AccessFromName(CString& name) {
register i;
static CString accessName="";
CString readableName;

	for(i=0;i<27;i++) { 
	   readableName.LoadString(IDS_FILE_COLUMN+i);
	   if(readableName==name) {
	       accessName.LoadString(IDS_FILE_COMMAND+i);
		   break;
	   }
	}
    return accessName;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Fills destination list box with the attributes stored in the initial state array.
//-----------------------------------------------------------------------------------------
void CAttributesDialog::FillListFromInitialState() {
register i;

   for(i=0;i<m_InitialState.GetSize();i++) m_Dest.AddString(m_InitialState[i]);
    
}
//-----------------------------------------------------------------------------------------

void CAttributesDialog::SetApplyTaget(CQueryResults* workWith) {
	m_QueryResults=workWith;
}
