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
// FilterWizard.cpp : implementation file
//

#include "stdafx.h"
#include "FilterSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static TC_ITEM item;

/////////////////////////////////////////////////////////////////////////////
// CFilterWizard property page

IMPLEMENT_DYNCREATE(CFilterWizard, CPropertyPage)

CFilterWizard::CFilterWizard() : CPropertyPage(CFilterWizard::IDD) {
	//{{AFX_DATA_INIT(CFilterList)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_TokensTable.SetSize(0,100);
    m_AttributesTable.SetSize(0,100);
    m_OperationsTable.SetSize(0,100);

}

CFilterWizard::~CFilterWizard() {
}

void CFilterWizard::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterWizard)
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


BEGIN_MESSAGE_MAP(CFilterWizard, CPropertyPage)
	//{{AFX_MSG_MAP(CFilterWizard)
	ON_CBN_SELCHANGE(IDC_ATTRIBUTE, OnSelchangeAttribute)
	ON_BN_CLICKED(IDC_AND, OnAnd)
	ON_BN_CLICKED(IDC_OR, OnOr)
	ON_BN_CLICKED(IDC_ADDBUTTON, OnAddbutton)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABEXPRESSION, OnSelchangeTabexpression)
	ON_BN_CLICKED(IDC_REMOVEBUTTON, OnRemovebutton)
	ON_BN_CLICKED(IDC_CHANGEBUTTON, OnChangebutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------------------
// Fills the attributes combo box and attributes array with all available attributes 
// asked from the server.
//-----------------------------------------------------------------------------------------
void CFilterWizard::FillAttributesCombo(void) {
int reference;
CAttributeEntry entry;

    m_AttributesTable.RemoveAll();
    m_AttributeCombo.ResetContent(); 
	// Emulator. Will be changed when Alex Z give me the Access commands
	for(register i=0;i<27;i++) {
	   entry.Name.LoadString(IDS_NAME_COLUMN+i);
	   entry.Option.LoadString(IDS_NAME_COMMAND+i);
	   CString typeStr;
	   typeStr.LoadString(IDS_NAME_TYPE+i);
	   if(typeStr == "string")   entry.Type   = STR_TYPE;
	   else                      entry.Type   = INT_TYPE;
	   reference = m_AttributesTable.Add(entry);
	   m_AttributeCombo.SetItemData(m_AttributeCombo.AddString(entry.Name),reference); 
	}	
	int ind = m_AttributeCombo.FindString(0,"Symbol name");
	if(ind<0)
	   m_AttributeCombo.SetCurSel(0);
	else
	   m_AttributeCombo.SetCurSel(ind);
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Fills the possible token operations getting attribute type from the 
// selected entry in the attributes combo by referencing to the m_AttributesTable
// array.
//-----------------------------------------------------------------------------------------
void CFilterWizard::FillOperationsCombo(void) {
register i;
int attrSelected;
int attrIndex;
COperationEntry entry;
int reference;

    attrSelected=m_AttributeCombo.GetCurSel();
	if(attrSelected==-1) return;
	m_OperationCombo.ResetContent();
    m_OperationsTable.RemoveAll();
	attrIndex=m_AttributeCombo.GetItemData(attrSelected); 
	switch(m_AttributesTable[attrIndex].Type) {
	   case INT_TYPE: for(i=0;i<6;i++) {
	                      entry.Name.LoadString(IDS_OP_INT+i);
	                      entry.FormatString.LoadString(IDS_FMT_INT+i);
	                      reference = m_OperationsTable.Add(entry);
	                      m_OperationCombo.SetItemData(m_OperationCombo.AddString(entry.Name),reference); 
                      }
                      break; 
	   case STR_TYPE: for(i=0;i<5;i++) {
	                      entry.Name.LoadString(IDS_OP_STR+i);
	                      entry.FormatString.LoadString(IDS_FMT_STR+i);
	                      reference = m_OperationsTable.Add(entry);
	                      m_OperationCombo.SetItemData(m_OperationCombo.AddString(entry.Name),reference); 
					  }
		              break;
	}
	m_OperationCombo.SetCurSel(0);
}
//-----------------------------------------------------------------------------------------







//----------------------------------------------------------------------------------------
// This function asks all controls to form the token, exept From and To members.
// Returns TRUE and fills"token" members if the token was made successfully or
// FALSE if error was detected in the user input. Sets m_ErrorType member to
// indicate the error type.
//----------------------------------------------------------------------------------------
BOOL CFilterWizard::MakeToken(CToken& token) {
register i;
int     selectedAttribute;
int     attributeReference;
int     selectedCommand;
int     commandReference;
CString argument;
BOOL    isDigit;


    // We are going to get the reference to the attributes array from the attributes
    // combobox.
    selectedAttribute=m_AttributeCombo.GetCurSel();
	// If nothing is selected
	if(selectedAttribute==-1) {
		m_ErrorType = ERROR_NO_ATTRIBUTE;
		return FALSE;
	}
    // Now we will be able to get this attribute index in the attribute array.
	attributeReference = m_AttributeCombo.GetItemData(selectedAttribute);

    // We are going to get the reference to the operations array from the operations
    // combobox.
    selectedCommand=m_OperationCombo.GetCurSel();
	// If nothing is selected
	if(selectedCommand==-1) {
		m_ErrorType = ERROR_NO_OPERATION;
		return FALSE;
	}
    // Now we will be able to get this operation index in the operation  array.
	commandReference = m_OperationCombo.GetItemData(selectedCommand);


	// User argument we will take from the arguments editor.
    m_ArgumentEdit.GetWindowText(argument); 

	// We need to check user argument. (We can't use characters different from "0".."9")
	// for the integer arguments, we can't use empty strings)
	if(argument.GetLength()==0) {
		m_ErrorType = ERROR_EMPTY_ARGUMENT;
		return FALSE;
	}


	// Testing the argiment...
	switch(m_AttributesTable[attributeReference].Type) {
	   // If integer ...
	   case INT_TYPE : for(i=0;i<argument.GetLength();i++) {
		                  switch(argument[i]) {
						     case '0' :
						     case '1' :
						     case '2' :
						     case '3' :
						     case '4' :
						     case '5' :
						     case '6' :
						     case '7' :
						     case '8' :
						     case '9' : isDigit=TRUE; break;
							 default  : isDigit=FALSE; break;
						  }
						  if(isDigit==FALSE) {
		                     m_ErrorType = ERROR_WRONG_ARGUMENT;
		                     return FALSE;
						  }
					   }
					   if(argument.GetLength()>10) {
		                  m_ErrorType = ERROR_BIG_ARGUMENT;
		                  return FALSE;
					   }
					   break;
	   // If string ...
	   case STR_TYPE :    break;

	   // In future ...

	   // Wrong type ...
	   default :	      m_ErrorType = ERROR_UNSUPPORTED_TYPE;

	}

    // Forming the token 
	// Access command
    token.Attribute      = m_AttributesTable[attributeReference].Option; 
    token.AttributeIndex = selectedAttribute;
	// concatination operation
	if(m_AndButton.GetCheck()==1) token.Concatination=OP_AND;
	else                          token.Concatination=OP_OR;
    // Operation format string
    token.UseLength = FALSE;
    token.Operation      = m_OperationsTable[commandReference].FormatString;
    token.OperationIndex = selectedCommand;
	if(token.Operation.Find("strn")!=-1) token.UseLength = TRUE;
	token.Argument  = argument;
	return TRUE;
     
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// Update all property page fields to show the token information.
//----------------------------------------------------------------------------------------
void CFilterWizard::ShowToken(CToken& token) {
    m_AttributeCombo.SetCurSel(token.AttributeIndex);
	FillOperationsCombo();
    m_OperationCombo.SetCurSel(token.OperationIndex);
	m_ArgumentEdit.SetWindowText(token.Argument);
    switch(token.Concatination) {
	case OP_AND :	m_AndButton.SetCheck(1);
	                m_OrButton.SetCheck(0);
					break;
	case OP_OR :	m_AndButton.SetCheck(0);
	                m_OrButton.SetCheck(1);
					break;
	}
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// This function adds the token to the to the token array.
// Returns TRUE if the token was added successfully or FALSE if the same token
// is already in the string.
//----------------------------------------------------------------------------------------
BOOL CFilterWizard::AddToken(CToken& token) {
register i;

    // Check for the same token in the array
	for(i=0;i<m_TokensTable.GetSize();i++) if(token==m_TokensTable[i]) return FALSE;
	m_TokensTable.Add(token);
	return TRUE;
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This function removes the token with the given index from the token array.
// Returns TRUE if the index is valid or false if out of range.
//----------------------------------------------------------------------------------------
BOOL CFilterWizard::RemoveToken(int index) {
	if(index <0 || index>=m_TokensTable.GetSize()) return FALSE;
	m_TokensTable.RemoveAt(index); 
	return TRUE;
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This function changes the token in the tokens array to the given one. 
// Returns TRUE if successfull or FALSE if index is out of range.
//----------------------------------------------------------------------------------------
BOOL CFilterWizard::ChangeToken(CToken& token, int index) {
	if(index <0 || index>=m_TokensTable.GetSize()) return FALSE;
	m_TokensTable.SetAt(index,token);
	return TRUE;
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This function generates and filles m_Command access command string.
//----------------------------------------------------------------------------------------
void CFilterWizard::CommandFromTokens(void) {
register i,j;
CToken token;
CString expr;
CString str;

    m_Command="";
	for(i=0;i<m_TokensTable.GetSize();i++) {
		token=m_TokensTable[i];
		// First token will not contain logical concatination
		if(i!=0) {
           switch(token.Concatination) {
		      case OP_AND : expr = " && "; break;
		      case OP_OR  : expr = " || "; break;
		   }
		} else expr="";

		CString arg;
		for(j=0;j<token.Argument.GetLength();j++) {
			if(token.Argument[j]=='\\') arg+="\\";
			arg+=token.Argument[j];
		}
		if(token.UseLength==TRUE)
			str.Format(token.Operation,token.Attribute,arg,strlen(token.Argument));
	    else
			str.Format(token.Operation,token.Attribute,arg);
		expr+=str;

	    // Updating token text pointers
		token.From=m_Command.GetLength();
		token.To  =token.From+expr.GetLength();
        ChangeToken(token,i);
		m_Command+=expr;
	}
}
//----------------------------------------------------------------------------------------
		




//----------------------------------------------------------------------------------------
// This function highlights the token with the given index in the command
// editor. Returns TRUE if OK and FALSE if index is out of range.
//----------------------------------------------------------------------------------------
BOOL CFilterWizard::HighlightToken(int index) {
CToken token;

	if(index <0 || index>=m_TokensTable.GetSize()) return FALSE;
	token= m_TokensTable[index];
	m_CommandEdit.SetFocus();
	m_CommandEdit.SetSel(token.From,token.To);
	return TRUE;
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Show error message taking error type from the m_ErrorType;
//----------------------------------------------------------------------------------------
void CFilterWizard::SayError(void){
	switch(m_ErrorType) {
	case ERROR_NO_ATTRIBUTE     : 	
		MessageBox("No attribute selected","Filter Wizard Error",MB_ICONEXCLAMATION);
		break;
	case ERROR_NO_OPERATION     :
		MessageBox("No operation selected","Filter Wizard Error",MB_ICONEXCLAMATION);
		break;
	case ERROR_WRONG_ARGUMENT   :
		MessageBox("Argument string doesn't match attribute type","Filter Wizard Error",MB_ICONEXCLAMATION);
		break;
	case ERROR_BIG_ARGUMENT     :
		MessageBox("The argument value is too big","Filter Wizard Error",MB_ICONEXCLAMATION);
		break;
	case ERROR_EMPTY_ARGUMENT   :
		MessageBox("Nothing typed in the argument field","Filter Wizard Error",MB_ICONEXCLAMATION);
		break;
	case ERROR_UNSUPPORTED_TYPE :
		MessageBox("Unknown attribute type","Filter Wizard Error",MB_ICONEXCLAMATION);
		break;
	case ERROR_EXISTING_TOKEN :
		MessageBox("You are trying to add the existing expression","Filter Wizard Error",MB_ICONEXCLAMATION);
		break;
	}
}
//----------------------------------------------------------------------------------------











/////////////////////////////////////////////////////////////////////////////
// CFilterWizard message handlers

//-----------------------------------------------------------------------------------------
// Initialize the dialog : fills all attributes in the attribute combo box (as well as
// attributes array) and all commands according the current selection in the attributes
// combo box (as well as operations array). Creates the image list with AND and OR images.
// Disables Change and Remove buttons - nothing to change or remove at the beginning.
//-----------------------------------------------------------------------------------------
BOOL CFilterWizard::OnInitDialog()  {

	CPropertyPage::OnInitDialog();

	FillAttributesCombo();
	FillOperationsCombo();

	// Image list for the tokens shown in the tab
	m_ImageList.Create(16,16,TRUE,2,0);
	CBitmap and;
	and.LoadBitmap(IDB_AND);
	CBitmap or;
	or.LoadBitmap(IDB_OR); 
	m_ImageList.Add(&and, RGB(0,0,0));
	m_ImageList.Add(&or,RGB(0,0,0));
    m_FlowTab.SetImageList(&m_ImageList);

	m_AndButton.SetCheck(1);

    // We have nothing to change or remove
	m_RemoveButton.EnableWindow(FALSE);
	m_ChangeButton.EnableWindow(FALSE);
	return TRUE;  
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This callback will run every time attribute changed in the attribute combo box. We need
// re-fill operations combobox as well as operations array according to the new attribute 
// type.
//-----------------------------------------------------------------------------------------
void CFilterWizard::OnSelchangeAttribute()  {
	// Automaticaly checks current attribute selection
	FillOperationsCombo();
    int selection;
	selection=m_FlowTab.GetCurSel();
	m_ArgumentEdit.SetWindowText("");
	if(selection>=0) HighlightToken(selection);
}
//-----------------------------------------------------------------------------------------




//---------------------------------------------------------------------------------------
// Selecting AND operation will cancel OR operation. This is And button callback.
//---------------------------------------------------------------------------------------
void CFilterWizard::OnAnd()  {
	m_AndButton.SetCheck(1);
	m_OrButton.SetCheck(0);
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Selecting OR operation will cancel AND operation. This is Or button callback.
//---------------------------------------------------------------------------------------
void CFilterWizard::OnOr()  {
	m_AndButton.SetCheck(0);
	m_OrButton.SetCheck(1);
}
//--------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// Subexpression flowchart change the current subexpression. We will use it to set new 
// current subexpression and hightlight it in the results string.
//---------------------------------------------------------------------------------------
void CFilterWizard::OnSelchangeTabexpression(NMHDR* pNMHDR, LRESULT* pResult)  {
int selection;

	selection=m_FlowTab.GetCurSel();
	ShowToken(m_TokensTable[selection]);
	if(selection>=0) HighlightToken(selection);
}
//---------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------
// Callback for the Add button. Adds subexpression to the result expression.
//---------------------------------------------------------------------------------------
void CFilterWizard::OnAddbutton()  {
CToken token;

    // Make new token array element - error goes wrong.
    if(MakeToken(token) == FALSE) {
		SayError();
		return;
	}
	// New token will be in the array
	if(AddToken(token)==FALSE) {
		m_ErrorType = ERROR_EXISTING_TOKEN;
		SayError();
		return;
	}
	// Adding new token to the tokens array

	// Form the new command string
	CommandFromTokens();
	// New command will be shown in the command editor
	m_CommandEdit.SetWindowText(m_Command);
	// with the new token highlighted
	HighlightToken(m_TokensTable.GetSize()-1);

	// Insert and select new item in the tab
    item.mask=TCIF_TEXT | TCIF_IMAGE;
	switch(token.Concatination) {
	   case OP_AND : item.iImage=0; break;
	   case OP_OR  : item.iImage=1; break;
	}
	item.pszText=(char *)(LPCSTR)token.Attribute;
   	m_FlowTab.InsertItem(m_TokensTable.GetSize()-1,&item);
	m_FlowTab.SetCurSel(m_TokensTable.GetSize()-1);

	// Now we have something to change or remove
	m_RemoveButton.EnableWindow(TRUE);
	m_ChangeButton.EnableWindow(TRUE);

	// Can apply changes
	SetModified(TRUE);
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
//  Callback runs every time Remove button clicked. Will remove currently selected 
//  subexpression.
//---------------------------------------------------------------------------------------
void CFilterWizard::OnRemovebutton()  {
int selection;

	selection=m_FlowTab.GetCurSel();
	if(selection<0) return;

	// Removing token from the token array
	RemoveToken(selection);

    // Removing element from the subexpressions flowchart
   	m_FlowTab.DeleteItem(selection);

	// Form the new command string
	CommandFromTokens();

	// New command will be shown in the command editor
	m_CommandEdit.SetWindowText(m_Command);

	if(m_TokensTable.GetSize()>0) {
	   // Selecting new item int the flow tab
       if((m_TokensTable.GetSize()-1)>=selection);
       else selection=m_TokensTable.GetSize()-1;
       m_FlowTab.SetCurSel(selection);
	   // Highlighting new token
	   HighlightToken(selection);
	   ShowToken(m_TokensTable[selection]);
	} else {
       // We have nothing to change or remove
	   m_RemoveButton.EnableWindow(FALSE);
	   m_ChangeButton.EnableWindow(FALSE);
	}
	// Can apply changes
	SetModified(TRUE);
}
//---------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Callback runs every time the user click Change button. Will change current element
// attributes to the new user selection.
//----------------------------------------------------------------------------------------
void CFilterWizard::OnChangebutton()  {
int selection;
CToken token;

	selection=m_FlowTab.GetCurSel();
	if(selection<0) return;

    // Make new token array element - error goes wrong.
    if(MakeToken(token) == FALSE) {
		SayError();
		return;
	}

	// Update the token information
	ChangeToken(token,selection);

	// Form the new command string
	CommandFromTokens();

	// New command will be shown in the command editor
	m_CommandEdit.SetWindowText(m_Command);
	HighlightToken(selection);
	// Can apply changes
	SetModified(TRUE);
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This function will set current name and current command in the parent (FilterSheet)
// members  m_Name and m_Command;
//----------------------------------------------------------------------------------------
void CFilterWizard::UpdateParentName(void) {
FilterSheet* parent;

	if(m_Command.GetLength()==0) return;
    parent=(FilterSheet *)GetParent();
	parent->m_Command = m_Command;
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Callback will run when this page changed to the other one. Will update command 
// stored in the parent.
//----------------------------------------------------------------------------------------
BOOL CFilterWizard::OnKillActive()  {
	UpdateParentName();
	return CPropertyPage::OnKillActive();
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Callabck will run when user press OK button
// If this page is active, we will use it's command to filter list.
// If this page is not active, we will use current command stored in the parent to
// filter our list.
//----------------------------------------------------------------------------------------
void CFilterWizard::OnOK()  {
FilterSheet* parent;

    parent=(FilterSheet *)GetParent();
	if(parent->GetActiveIndex()==1) UpdateParentName();
	CPropertyPage::OnOK();
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Callabck will run when user press Apply button
// If this page is active, we will use it's command to filter list.
// If this page is not active, we will use current command stored in the parent to
// filter our list.
//----------------------------------------------------------------------------------------
BOOL CFilterWizard::OnApply()  {
FilterSheet* parent;

    parent=(FilterSheet *)GetParent();
	if(parent->GetActiveIndex()==1) {
		UpdateParentName();
        CQueryResults* ctrl;
        ctrl=parent->m_QueryResults;
	    if(ctrl!=NULL) {
	       CString filterQuery;
	       if(parent->m_Command.GetLength()>0) 
			   filterQuery.Format("filter {%s}",parent->m_Command);
	       else  
			   filterQuery=""; 
           ctrl->SetFilter(filterQuery);
           ctrl->Update();
		}
	}
	return CPropertyPage::OnApply();
}
//----------------------------------------------------------------------------------------

