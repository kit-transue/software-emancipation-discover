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
// FilterWizard.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFilterWizard dialog
#include <afxcmn.h>
#include <afxtempl.h>
#ifndef __QUERYRESULTS_H
   #define __QUERYRESULTS_H
   #include "queryresults.h"
#endif

#define INT_TYPE 0
#define STR_TYPE 1

#define ERROR_NO_ATTRIBUTE     -1
#define ERROR_NO_OPERATION     -2
#define ERROR_WRONG_ARGUMENT   -3
#define ERROR_BIG_ARGUMENT     -4
#define ERROR_EMPTY_ARGUMENT   -5
#define ERROR_UNSUPPORTED_TYPE -6
#define ERROR_EXISTING_TOKEN   -7

#define OP_AND 0
#define OP_OR  1


//-----------------------------------------------------------------------------------------
// This is attribute array entry - it contains readable name stored as Name,
// access attribute command stored as Option and attribute type stored as Type.
//-----------------------------------------------------------------------------------------
class CAttributeEntry {
public:
	 CString Name;
	 CString Option;
	 int     Type;
};
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This is operations array entry - it contains the operatin visible name and operation
// format string (we will use this format string in the CString.Format to form the command)
//-----------------------------------------------------------------------------------------
class COperationEntry {
public:
	CString Name;
	CString FormatString;
};
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This represent a token in the token array - Attribute contains the access name
// of the attribute, Operation contains inside-token operation code,  Concatination
// contains the logical operation by which this token will be concatinated with the
// others to form a string.
// From and To contains the position of the token in the result string
//-----------------------------------------------------------------------------------------
class CToken {
public:
	 CString  Attribute;       // Attribute Access name
	 int      AttributeIndex;  // Attribute index in the combobox. 
	 CString  Operation;       // Operation format string. UseLength == TRUE - 3 operands
	 int      OperationIndex;  // Operation index in the combobox

	 CString Argument;

	 BOOL    UseLength;       // TRUE if we need 3-d arguent in format - length
	 int     Concatination;   // concationation ("and" or "or")

	 int     From;            // starts at the position
	 int     To;              // ends at the position


	 operator ==(CToken& t) {
		 if(Attribute == t.Attribute &&
            Operation == t.Operation &&
			Argument  == t.Argument) return 1;
		 return 0;
	 }
};
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
// This property page represents filter wizard.
//-----------------------------------------------------------------------------------------
class CFilterWizard : public CPropertyPage {
	DECLARE_DYNCREATE(CFilterWizard)

// Construction
public:
	CFilterWizard();
	~CFilterWizard();
// Dialog Data
	//{{AFX_DATA(CFilterWizard)
	enum { IDD = IDD_FILTERWIZARD };
	CButton	m_ChangeButton;
	CButton	m_RemoveButton;
	CButton	m_OrButton;
	CButton	m_AndButton;
	CEdit	m_ArgumentEdit;
	CComboBox	m_OperationCombo;
	CTabCtrl	m_FlowTab;
	CEdit	m_CommandEdit;
	CComboBox	m_AttributeCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilterWizard)
	public:
	virtual BOOL OnKillActive();
	virtual void OnOK();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFilterWizard)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeAttribute();
	afx_msg void OnAnd();
	afx_msg void OnOr();
	afx_msg void OnAddbutton();
	afx_msg void OnSelchangeTabexpression(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRemovebutton();
	afx_msg void OnChangebutton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    void UpdateParentName(void);
private:

	CImageList m_ImageList;


	// Result string that will be used as Access command. This string is displayed 
	// in the CommandEdit editor.
	CString m_Command;

    // Attributes array contains attributes name, attribute access command and 
	// attribute type. Attributes combo box reference to this array elements using
	// user data field available in the Windows combo box.
	CArray<CAttributeEntry,CAttributeEntry&> m_AttributesTable;

    // Operations array contains operations visible names and operations format strings.
	// Operations combo box reference to this array elements using user data field 
	// available in the Windows combo box.
	CArray<COperationEntry,COperationEntry&> m_OperationsTable;

	// Array of the tokens - from this array Access command will be formed.
	CArray<CToken,CToken&> m_TokensTable;

	int m_ErrorType;

	// This function asks all controls to form the token, exept From and To members.
	// Returns TRUE and fills"token" members if the token was made successfully or
	// FALSE if error was detected in the user input. Sets m_ErrorType member to
	// indicate the error type.
	BOOL MakeToken(CToken& token);

	// Update all property page fields to show the token information.
	void ShowToken(CToken& token);

	// This function adds the token to the to the token array.
	// Returns TRUE if the token was added successfully or FALSE if the same token
	// is already in the string.
	BOOL AddToken(CToken& token);

	// This function removes the token with the given index from the token array.
	// Returns TRUE if the index is valid or false if out of range.
    BOOL RemoveToken(int index);

	// This function changes the token in the tokens array to the given one. 
	// Returns TRUE if successfull or FALSE if index is out of range.
	BOOL ChangeToken(CToken& token, int index);

	// This function generates and filles m_Command access command string.
	void CommandFromTokens(void);

	// This function highlights the token with the given index in the command
	// editor. Returns TRUE if OK and FALSE if index is out of range.
	BOOL HighlightToken(int index);

    // Fills the attributes combo box and attributes array with all available 
	// attributes asked from the server.
	void FillAttributesCombo(void);

	// Fills the possible token operations getting attribute type from the 
	// selected entry in the attributes combo by referencing to the m_AttributesTable
	// array.
	void FillOperationsCombo(void);

	// Show error message taking error type from the m_ErrorType;
	void SayError(void);
};
