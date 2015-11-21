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
// FilterDialog.cpp : implementation file
//

#include "stdafx.h"
#include "FilterDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define CS_STARTSWITH  "Starts with, CS"
#define CI_STARTSWITH  "Starts with, CI"
#define CS_EQUALTO     "Equal to, CS"
#define CI_EQUALTO     "Equal to, CI"
#define CS_CONTAINS    "Contains, CS"

/////////////////////////////////////////////////////////////////////////////
// CFilterDialog property page

IMPLEMENT_DYNCREATE(CFilterDialog, CPropertyPage)


CFilterDialog::CFilterDialog() : CPropertyPage(CFilterDialog::IDD)
{
	//{{AFX_DATA_INIT(CFilterDialog)
	//}}AFX_DATA_INIT
}

CFilterDialog::~CFilterDialog()
{
}

void CFilterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterDialog)
	DDX_Control(pDX, IDC_CHANGEBUTTON, m_ChangeButton);
	DDX_Control(pDX, IDC_REMOVEBUTTON, m_RemoveButton);
	DDX_Control(pDX, IDC_OR, m_OrButton);
	DDX_Control(pDX, IDC_AND, m_AndButton);
	DDX_Control(pDX, IDC_ARGUMENT, m_ArgumentEdit);
	DDX_Control(pDX, IDC_OPERATION, m_OperationCombo);
	DDX_Control(pDX, IDC_TABEXPRESSION, m_FlowTab);
	DDX_Control(pDX, IDC_EDITEXPRESSION, m_CommandEdit);
	DDX_Control(pDX, IDC_ATTRIBUTE, m_AttributeCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFilterDialog, CPropertyPage)
	//{{AFX_MSG_MAP(CFilterDialog)
	ON_CBN_SELCHANGE(IDC_ATTRIBUTE, OnSelchangeAttribute)
	ON_BN_CLICKED(IDC_AND, OnAnd)
	ON_BN_CLICKED(IDC_OR, OnOr)
	ON_BN_CLICKED(IDC_ADDBUTTON, OnAddbutton)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABEXPRESSION, OnSelchangeTabexpression)
	ON_BN_CLICKED(IDC_REMOVEBUTTON, OnRemovebutton)
	ON_BN_CLICKED(IDC_CHANGEBUTTON, OnChangebutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterDialog message handlers

BOOL CFilterDialog::OnInitDialog()  {
AttributeEntry entry;

	CDialog::OnInitDialog();

	// Filling the Attribute combo box
    m_AttributesTable.RemoveAll();
	for(register i=0;i<27;i++) {		                         
	   entry.Name.LoadString(IDS_NAME_COLUMN+i);
	   entry.Option.LoadString(IDS_NAME_COMMAND+i);
	   CString typeStr;
	   typeStr.LoadString(IDS_NAME_TYPE+i);
	   if(typeStr == "string")   entry.Type   = STRING_ATTRIBUTE;
	   else                      entry.Type   = INTEGER_ATTRIBUTE;
	   int index = m_AttributeCombo.AddString(entry.Name);
	   m_AttributesTable.SetAtGrow(i,entry);
       m_AttributeCombo.SetItemData(index,i);
	}
	int ind = m_AttributeCombo.FindString(0,"Symbol name");
	if(ind<0)
	   m_AttributeCombo.SetCurSel(0);
	else
	   m_AttributeCombo.SetCurSel(ind);
	FillOperationsCombo();
    m_TokensCount=0;	
	m_AndButton.SetCheck(1);
	m_OrButton.SetCheck(0);

	// Image list for the elements
	m_ImageList.Create(16,16,TRUE,2,0);
	CBitmap and;
	and.LoadBitmap(IDB_AND);
	CBitmap or;
	or.LoadBitmap(IDB_OR); 

	m_ImageList.Add(&and, RGB(0,0,0));
	m_ImageList.Add(&or,RGB(0,0,0));
    m_FlowTab.SetImageList(&m_ImageList);

	m_RemoveButton.EnableWindow(FALSE);
	m_ChangeButton.EnableWindow(FALSE);
	return TRUE;  
}

void CFilterDialog::FillOperationsCombo() {
AttributeEntry entry;
	m_OperationCombo.ResetContent();
	int index = m_AttributeCombo.GetItemData(m_AttributeCombo.GetCurSel());
	entry     = m_AttributesTable[index];
	switch(entry.Type) {
	   case INTEGER_ATTRIBUTE : m_OperationCombo.AddString("==");
		                         m_OperationCombo.AddString("!=");
		                        m_OperationCombo.AddString(">");
								m_OperationCombo.AddString("<");
								m_OperationCombo.AddString(">=");
								m_OperationCombo.AddString("<=");
								m_OperationCombo.SetCurSel(0);
								break;

	   case STRING_ATTRIBUTE  : m_OperationCombo.AddString(CS_STARTSWITH);
								m_OperationCombo.AddString(CI_STARTSWITH);
								m_OperationCombo.AddString(CS_EQUALTO);
								m_OperationCombo.AddString(CI_EQUALTO);
								m_OperationCombo.AddString(CS_CONTAINS);
								m_OperationCombo.SetCurSel(0);
								break;
	}
}

void CFilterDialog::OnSelchangeAttribute()  {
	FillOperationsCombo();
	
}



//-----------------------------------------------------------------------------------------
// Return TRUE if subexpression which will be formed is valid, FALSE if not.
// Now only simple check of the user intut performed - the argument field must be filled,
// but later all illegal user input will be checked
//-----------------------------------------------------------------------------------------
BOOL CFilterDialog::CheckSubexpression(void) {
CString argument;
	// calculating argument option
	char* buffer = new char [32000];
	buffer[0]=0;
    if(m_ArgumentEdit.GetLine(0,buffer,32000)==0) return FALSE;
	argument=buffer;
	delete [] buffer;

	if(argument.GetLength()>0) return TRUE;
	return FALSE;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// Returns the string with subexpression from the subexpression controls.
//-----------------------------------------------------------------------------------------
CString& CFilterDialog::GetSubexpression(void) {
int index;
static CString subexpression;
CString attribute;
CString operation;
CString argument;

    // calculating the access attribute option
    AttributeEntry entry;
	index      = m_AttributeCombo.GetItemData(m_AttributeCombo.GetCurSel());
	entry      = m_AttributesTable[index];
	attribute += entry.Option;

	// calculating operation option
	index = m_OperationCombo.GetCurSel();
	m_OperationCombo.GetLBText(index,operation); 

	// calculating argument option
	char* buffer = new char [32000];
    m_ArgumentEdit.GetLine(0,buffer,32000);
	argument=buffer;
	delete [] buffer;

	// forming result subexpression
	switch(entry.Type) {
	   case INTEGER_ATTRIBUTE :   subexpression=attribute+" "+operation+" "+argument;
		                          break;

	   case STRING_ATTRIBUTE :   // String starts with letters in argument, case sensitive
                                 if(operation == CS_STARTSWITH)
									 subexpression.Format("strncmp(%s,\"%s\",%d)==0",attribute,argument,argument.GetLength());
                                 if(operation == CI_STARTSWITH)
									 subexpression.Format("strnicmp(%s,\"%s\",%d)==0",attribute,argument,argument.GetLength());
                                 if(operation == CS_EQUALTO)
									 subexpression.Format("strcmp(%s,\"%s\")==0",attribute,argument);
                                 if(operation == CI_EQUALTO)
									 subexpression.Format("stricmp(%s,\"%s\")==0",attribute,argument);
                                 if(operation == CS_CONTAINS)
									 subexpression.Format("strstr(%s,\"%s\")==\"\"",attribute,argument);

	} 
	return  subexpression;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Selecting AND operation will cancel OR operation. This is And button callback.
//---------------------------------------------------------------------------------------
void CFilterDialog::OnAnd()  {
	m_OrButton.SetCheck(0);
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Selecting OR operation will cancel AND operation. This is Or button callback.
//---------------------------------------------------------------------------------------
void CFilterDialog::OnOr()  {
	m_AndButton.SetCheck(0);
}
//--------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Callback for the Add button. Adds subexpression to the result expression.
//---------------------------------------------------------------------------------------
void CFilterDialog::OnAddbutton()  {
TokenLoc loc;
char* buffer = new char [32000];

    // Check if the user field is valid
	if(CheckSubexpression()==FALSE) {
		CString mes;
		CString cap;
		mes.LoadString(IDS_FIELDMESSAGE);
		cap.LoadString(IDS_FIELDCAPTION);
		MessageBox(mes,cap, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	CString token = GetSubexpression();
	CString completeCommand;

	m_CommandEdit.GetLine(0,buffer);
	completeCommand=buffer;

    loc.From=completeCommand.GetLength();
	if(m_AndButton.GetCheck()==1) {
	     if(m_TokensCount>0) completeCommand+=" && ";
	} else {
	     if(m_TokensCount>0) completeCommand+=" || ";
	}
	completeCommand+=token;
    loc.To=completeCommand.GetLength();

	// Setting the new result string
	m_CommandEdit.SetSel(0,32000);
	m_CommandEdit.ReplaceSel(completeCommand);

	// Inserting new item into the flow tab
	TC_ITEM item;
    item.mask=TCIF_TEXT | TCIF_IMAGE;

    AttributeEntry entry;
	int index      = m_AttributeCombo.GetItemData(m_AttributeCombo.GetCurSel());
	entry          = m_AttributesTable[index];
	switch(entry.Type) {
	   case INTEGER_ATTRIBUTE : item.pszText="int"; break;
	   case STRING_ATTRIBUTE :  item.pszText="str"; break;
	}

	if(m_AndButton.GetCheck()==1)
	   item.iImage=0;
	else
	   item.iImage=1;
   	m_FlowTab.InsertItem(m_TokensCount,&item);
	m_FlowTab.SetCurSel(m_TokensCount);
	m_CommandEdit.SetFocus();
	m_CommandEdit.SetSel(loc.From,loc.To);
	m_AttributeCombo.SetCurSel(loc.Attribute);
	m_OperationCombo.SetCurSel(loc.Operation);

	// Filling the rest of the loc - attribute index, operation index and
	// user argument
	loc.Attribute      = m_AttributeCombo.GetCurSel();
	loc.Operation      = m_OperationCombo.GetCurSel();
	loc.Concatination = m_AndButton.GetCheck();
    m_ArgumentEdit.GetLine(0,buffer,32000);
	loc.User=buffer;

    // inserting new loc into the loc array
    m_TokensLocs.SetAtGrow(m_TokensCount,loc);


	// results string not empty - can change and remove
    m_RemoveButton.EnableWindow(TRUE);
	m_ChangeButton.EnableWindow(TRUE);
	m_TokensCount++;
	delete [] buffer;
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Subexpression flowchart change the current subexpression. We will use it to set new 
// current subexpression and hightlight it in the results string.
//---------------------------------------------------------------------------------------
void CFilterDialog::OnSelchangeTabexpression(NMHDR* pNMHDR, LRESULT* pResult)  {
TokenLoc loc;
int selection;

	selection=m_FlowTab.GetCurSel();
	loc= m_TokensLocs[selection];
	m_CommandEdit.SetFocus();
	m_CommandEdit.SetSel(loc.From,loc.To);

	m_AttributeCombo.SetCurSel(loc.Attribute);
	m_OperationCombo.SetCurSel(loc.Operation);
	if(loc.Concatination==1) {
	   m_AndButton.SetCheck(1);
	   m_OrButton.SetCheck(0);
	} else {
	   m_AndButton.SetCheck(0);
	   m_OrButton.SetCheck(1);
	}
    m_ArgumentEdit.SetSel(0,32000);
	m_ArgumentEdit.ReplaceSel(loc.User);
	*pResult = 0;
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
//  Callback runs every time Remove button clicked. Will remove currently selected 
//  subexpression.
//---------------------------------------------------------------------------------------
void CFilterDialog::OnRemovebutton()  {
register i=0;
CString oldCommand;
CString newCommand;
TokenLoc loc;
int selection;

    selection = m_FlowTab.GetCurSel();

    // Extracting the result string from the edit control
    if(m_TokensCount==0) return;
	char* buffer = new char [32000];
	m_CommandEdit.GetLine(0,buffer);
	oldCommand=buffer;
	delete [] buffer;

	// Removing the selected element from the result command string
	// Do not foget that first sub-item mustn't start with logical expression "&&" or "||"!
	loc=m_TokensLocs[selection];
	for(i=0;i<oldCommand.GetLength();i++) 
		 if(i<loc.From || i>=loc.To+((selection==0) ? 4 : 0)) 
		    newCommand+=oldCommand[i];

	// When sub-string will be removed the selectors must show to the
	// new subitems positions in the command
	int offset;
	if(selection==0) offset=4;
	else                         offset=0;
	for(i=selection+1;i<m_TokensCount;i++) {
		if( i==m_FlowTab.GetCurSel()+1) {
		   m_TokensLocs[i].From-=(loc.To-loc.From);
		   m_TokensLocs[i].To  -=(loc.To-loc.From+offset);
        } else {
		   m_TokensLocs[i].From-=(loc.To-loc.From+offset);
		   m_TokensLocs[i].To  -=(loc.To-loc.From+offset);
		}
	}
	m_TokensLocs.RemoveAt(selection,1);

    // removing element from the subexpressions flowchart
   	m_FlowTab.DeleteItem(selection);

    // new command will be displayed in the editor
	m_CommandEdit.SetSel(0,32000);
	m_CommandEdit.ReplaceSel(newCommand);

    m_TokensCount--;
	if(m_TokensCount==0) {
	   m_RemoveButton.EnableWindow(FALSE);
	   m_ChangeButton.EnableWindow(FALSE);
	} else {
		if(m_TokensCount-1>=selection) {
	       m_FlowTab.SetCurSel(selection);
	       loc=m_TokensLocs[selection];
		} else {
	       m_FlowTab.SetCurSel(m_TokensCount-1);
	       loc=m_TokensLocs[m_TokensCount-1];
		}
	}
	m_CommandEdit.SetFocus();
	m_CommandEdit.SetSel(loc.From,loc.To);
	m_AttributeCombo.SetCurSel(loc.Attribute);
	m_OperationCombo.SetCurSel(loc.Operation);

	m_ArgumentEdit.SetSel(0,32000);
	m_ArgumentEdit.ReplaceSel(loc.User);

}
//---------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Callback runs every time the user click Change button. Will change current element
// attributes to the new user selection.
//----------------------------------------------------------------------------------------
void CFilterDialog::OnChangebutton()  {
register i=0;
CString oldCommand;
CString newCommand;
TokenLoc loc;
int selection;
char* buffer = new char[32000];


	if(CheckSubexpression()==FALSE) {
		CString mes;
		CString cap;
		mes.LoadString(IDS_FIELDMESSAGE);
		cap.LoadString(IDS_FIELDCAPTION);
		MessageBox(mes,cap, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

    selection = m_FlowTab.GetCurSel();
	loc=m_TokensLocs[selection];

	// forming the new token - results will be in the "token" string
	CString token = GetSubexpression();
	if(m_AndButton.GetCheck()==1) {
	     if(selection>0) token =" && "+token;
	} else {
	     if(selection>0) token =" || "+token;
	}



	// Changing the token information
	int diff = token.GetLength()-(loc.To-loc.From);
    loc.To=m_TokensLocs[selection].To+=diff;
	loc.Attribute=m_TokensLocs[selection].Attribute      = m_AttributeCombo.GetCurSel();
	loc.Operation=m_TokensLocs[selection].Operation      = m_OperationCombo.GetCurSel();
	loc.Concatination=m_TokensLocs[selection].Concatination  = m_AndButton.GetCheck();
    m_ArgumentEdit.GetLine(0,buffer,32000);
	loc.User=m_TokensLocs[selection].User=buffer;

	for(i=selection+1;i<m_TokensCount;i++) {
		m_TokensLocs[i].From+=diff;
		m_TokensLocs[i].To  +=diff;
	}

    // Changing the selection into command-line editor
	m_CommandEdit.SetFocus();
	m_CommandEdit.SetSel(loc.From,loc.To);
	m_CommandEdit.ReplaceSel(token);


	TC_ITEM item;
    item.mask=TCIF_IMAGE;
    if(loc.Operation==1)
	   item.iImage=0;
	else
	   item.iImage=1;
   	m_FlowTab.SetItem(selection,&item);

	m_CommandEdit.SetSel(loc.From,loc.To);


	delete buffer;

}
//----------------------------------------------------------------------------------------

