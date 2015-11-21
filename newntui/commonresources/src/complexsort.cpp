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
// ComplexSort.cpp : implementation file
//

#include "stdafx.h"
#include "ComplexSort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CComplexSort dialog


CComplexSort::CComplexSort(CWnd* pParent /*=NULL*/)
	: CDialog(CComplexSort::IDD, pParent)
{
	//{{AFX_DATA_INIT(CComplexSort)
	//}}AFX_DATA_INIT
	m_Durty=FALSE;
	m_RefillOnCancel=FALSE;
}


void CComplexSort::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CComplexSort)
	DDX_Control(pDX, IDAPPLY, m_Apply);
	DDX_Control(pDX, IDC_DECENDING, m_Decending);
	DDX_Control(pDX, IDC_ACENDING, m_Acending);
	DDX_Control(pDX, IDC_REMOVE, m_Remove);
	DDX_Control(pDX, IDC_ADD, m_Add);
	DDX_Control(pDX, IDC_SOURCE, m_Source);
	DDX_Control(pDX, IDC_DESTINATION, m_Destination);
	//}}AFX_DATA_MAP
}

//------------------------------------------------------------------------------------------
// Collect all destination attributes to the single string.
//------------------------------------------------------------------------------------------
void CComplexSort::MakeCollection(CString& collection) {
register i;
int index;

   collection = "";
   for(i=0;i<m_Destination.GetCount();i++) {
	   index=m_Destination.GetItemData(i);
	   collection+=m_CommandsTable[index];
	   collection+=' ';
   }

}
//------------------------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CComplexSort, CDialog)
	//{{AFX_MSG_MAP(CComplexSort)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_ACENDING, OnAcending)
	ON_BN_CLICKED(IDC_DECENDING, OnDecending)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComplexSort message handlers


//------------------------------------------------------------------------------------------
// Dialog initialization. Will fill source list with all available attributes and will left
// destination list empty. Will load and set images to the Add and Remove buttons.
//------------------------------------------------------------------------------------------
BOOL CComplexSort::OnInitDialog()  {
CString readableName;
CString accessName;
int index;


	CDialog::OnInitDialog();
	if(m_QueryResults==NULL) return TRUE;

	m_StartupSort=m_QueryResults->GetSort();
	m_CommandsTable.SetSize(0,100);
	for(register i=0;i<28;i++) { //****** This is emulator! 
		                         //****** Ask Alex Z about real access commands!
	   readableName.LoadString(IDS_NAME_COLUMN+i);
	   accessName.LoadString(IDS_NAME_COMMAND+i);
	   index=m_CommandsTable.Add(accessName);
	   m_Source.SetItemData(m_Source.AddString(readableName),index); 
	}
	m_ArLeft.LoadMappedBitmap(IDB_ARROWLEFT);
	m_Remove.SetBitmap(m_ArLeft);
	m_ArRight.LoadMappedBitmap(IDB_ARROWRIGHT);
	m_Add.SetBitmap(m_ArRight);

	m_ArUp.LoadMappedBitmap(IDB_ARROWUP);
	m_Decending.SetBitmap(m_ArUp);
	m_ArDn.LoadMappedBitmap(IDB_ARROWDN);
	m_Acending.SetBitmap(m_ArDn);

	m_Acending.SetCheck(0);
	m_Decending.SetCheck(1);

    m_Apply.EnableWindow(FALSE);

	return TRUE;  
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Callback runs when "Apply" button clicked. Check the durty flag and, if set, set the new
// sorting command at the list and re-fill the list.
//------------------------------------------------------------------------------------------
void CComplexSort::OnApply() {
CString sort;
CString collection;

   if(m_QueryResults!=NULL && m_Durty==TRUE) {
	  // concatinate all attributes tothe single string
	  MakeCollection(collection);
	  if(m_Acending.GetState()==1)   sort.Format("sort -a %s",collection);
	  else                           sort.Format("sort -d %s",collection);
	  m_QueryResults->SetSort(sort);
	  m_QueryResults->Update();
      m_Durty=FALSE;
      m_RefillOnCancel=TRUE;
      m_Apply.EnableWindow(FALSE);
   }
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Callback runs when "Cancel" button clicked. If m_RefillOnCalcel flag is set (this means
// the "Apply" command was used) the function will restore previous sort command and will
// re-fill the list.
//------------------------------------------------------------------------------------------
void CComplexSort::OnCancel() {

   if(m_QueryResults!=NULL && m_RefillOnCancel==TRUE) {
      m_QueryResults->SetSort(m_StartupSort);
      m_QueryResults->Update();
	  m_RefillOnCancel=FALSE;
   }
   CDialog::OnCancel();
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Callback runs when "OK" button pressed. set new sort command and re-fill the results list.
//------------------------------------------------------------------------------------------
void CComplexSort::OnOK() {
CString sort;
CString collection;

   if(m_QueryResults!=NULL && m_Durty==TRUE) {
	  // concatinate all attributes tothe single string
	  MakeCollection(collection);
	  if(m_Acending.GetState()==1)   sort.Format("sort -a %s",collection);
	  else                           sort.Format("sort -d %s",collection);
	  m_QueryResults->SetSort(sort);
	  m_QueryResults->Update();
	  m_Durty=FALSE;
      m_RefillOnCancel=TRUE;
      m_Apply.EnableWindow(FALSE);
   }
   CDialog::OnOK();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Callback runs when add button pressed. Remove the element from the source list and
// put this element into destination list.
//------------------------------------------------------------------------------------------
void CComplexSort::OnAdd() {
CString name;
int selection;
int index;

   selection=m_Source.GetCurSel();
   if(selection<0) return;
   m_Source.GetText(selection,name);
   index=m_Source.GetItemData(selection);
   m_Source.DeleteString(selection);
   m_Destination.SetItemData(m_Destination.AddString(name),index);
   m_Durty=TRUE;
   m_Apply.EnableWindow(TRUE);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Callback runs when remove button pressed. Remove the element from the destination list 
// and put this element into the source list.
//------------------------------------------------------------------------------------------
void CComplexSort::OnRemove() {
CString name;
int selection;
int index;

   selection=m_Destination.GetCurSel();
   if(selection<0) return;
   m_Destination.GetText(selection,name);
   index=m_Destination.GetItemData(selection);
   m_Destination.DeleteString(selection);
   m_Source.SetItemData(m_Source.AddString(name),index);
   m_Durty=TRUE;
   m_Apply.EnableWindow(TRUE);
}


//------------------------------------------------------------------------------------------
// Will use this window to execute Apply, OK and Cancel commands.
//------------------------------------------------------------------------------------------
void CComplexSort::SetApplyTaget(CQueryResults* workWith) {
	m_QueryResults=workWith;
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Acending direction selected by clicking acending button.
//------------------------------------------------------------------------------------------
void CComplexSort::OnAcending()  {
	m_Acending.SetCheck(1);
	m_Decending.SetCheck(0);
	m_Durty=TRUE;
    m_Apply.EnableWindow(TRUE);
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Decending direction selected by clicking decending button.
//------------------------------------------------------------------------------------------
void CComplexSort::OnDecending() {
   m_Acending.SetCheck(0);
   m_Decending.SetCheck(1);
   m_Durty=TRUE;
   m_Apply.EnableWindow(TRUE);
}
//------------------------------------------------------------------------------------------
