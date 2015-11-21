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
// Commands.cpp : implementation file
//

#include "stdafx.h"
#include "VSAddIn.h"
#include "Commands.h"
#include "SocketConnection.h"
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommands

CCommands::CCommands()
{
	m_pApplication = NULL;
	m_pApplicationEventsObj = NULL;
	m_pDebuggerEventsObj = NULL;
	connections = new SocketConnection((CCommandsObj *)this);
}

CCommands::~CCommands()
{
	ASSERT (m_pApplication != NULL);
	m_pApplication->Release();
	delete connections;
}


//------------------------------------------------------------------------------
HRESULT CCommands::ShowMessage(CString msg) {
HRESULT hr;

	CComBSTR bstr = msg;
	hr = m_pApplication->PrintToOutputWindow(bstr);
	return (hr);
}
//-------------------------------------------------------------------------------

void CCommands::SetApplicationObject(IApplication* pApplication)
{
	// This function assumes pApplication has already been AddRef'd
	//  for us, which CDSAddIn did in its QueryInterface call
	//  just before it called us.
	m_pApplication = pApplication;

	// Create Application event handlers
	XApplicationEventsObj::CreateInstance(&m_pApplicationEventsObj);
	m_pApplicationEventsObj->AddRef();
	m_pApplicationEventsObj->Connect(m_pApplication);
	m_pApplicationEventsObj->m_pCommands = this;

	// Create Debugger event handler
	CComPtr<IDispatch> pDebugger;
	if (SUCCEEDED(m_pApplication->get_Debugger(&pDebugger)) 
		&& pDebugger != NULL)
	{
		XDebuggerEventsObj::CreateInstance(&m_pDebuggerEventsObj);
		m_pDebuggerEventsObj->AddRef();
		m_pDebuggerEventsObj->Connect(pDebugger);
		m_pDebuggerEventsObj->m_pCommands = this;
	}
}

void CCommands::UnadviseFromEvents()
{
	ASSERT (m_pApplicationEventsObj != NULL);
	m_pApplicationEventsObj->Disconnect(m_pApplication);
	m_pApplicationEventsObj->Release();
	m_pApplicationEventsObj = NULL;

	if (m_pDebuggerEventsObj != NULL)
	{
		// Since we were able to connect to the Debugger events, we
		//  should be able to access the Debugger object again to
		//  unadvise from its events (thus the VERIFY_OK below--see stdafx.h).
		CComPtr<IDispatch> pDebugger;
		VERIFY_OK(m_pApplication->get_Debugger(&pDebugger));
		ASSERT (pDebugger != NULL);
		m_pDebuggerEventsObj->Disconnect(pDebugger);
		m_pDebuggerEventsObj->Release();
		m_pDebuggerEventsObj = NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Event handlers

// TODO: Fill out the implementation for those events you wish handle
//  Use m_pCommands->GetApplicationObject() to access the Developer
//  Studio Application object

// Application events

HRESULT CCommands::XApplicationEvents::BeforeBuildStart()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::BuildFinish(long nNumErrors, long nNumWarnings)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::BeforeApplicationShutDown()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::DocumentOpen(IDispatch* theDocument)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::BeforeDocumentClose(IDispatch* theDocument)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::DocumentSave(IDispatch* theDocument)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::NewDocument(IDispatch* theDocument)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WindowActivate(IDispatch* theWindow)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WindowDeactivate(IDispatch* theWindow)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WorkspaceOpen()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WorkspaceClose()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::NewWorkspace()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

// Debugger event

HRESULT CCommands::XDebuggerEvents::BreakpointHit(IDispatch* pBreakpoint)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CCommands methods

STDMETHODIMP CCommands::CodeRoverOpenDefinitionMethod()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

    CComBSTR file;
    CComBSTR token;
    long line;
    long column;
	long offset;

    getSelection(file,token,offset,line,column);
	CString eventStr;
	CString fileName;
	fileName=file.m_str;
    eventStr.Format("open_def\t%s\t%d\t%d\t%d\n",fileName,offset,line,column);

    connections->sendEvent(eventStr);
	return S_OK;
}

STDMETHODIMP CCommands::CodeRoverInfoMethod() {
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

    CComBSTR file;
    CComBSTR token;
    long line;
    long column;
	long offset;
    getSelection(file,token,offset,line,column);
	CString eventStr;
	CString fileName;
	fileName=file.m_str;
    eventStr.Format("inst_def\t%s\t%d\t%d\t%d\n",fileName,offset,line,column);

    connections->sendEvent(eventStr);
	return S_OK;
}

STDMETHODIMP CCommands::CodeRoverBrowseMethod() {
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
    CComBSTR file;
    CComBSTR token;
    long line;
    long column;
	long offset;
    getSelection(file,token,offset,line,column);
	CString eventStr;
	CString fileName;
	fileName=file.m_str;
    eventStr.Format("query\t%s\t%d\t%d\t%d\n",fileName,offset,line,column);

    connections->sendEvent(eventStr);
	return S_OK;
}

STDMETHODIMP CCommands::CodeRoverActivateMethod() {
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

    connections->sendEvent("popup");
	return S_OK;
}

void CCommands::FreezeSelection() {
CComBSTR file;
CComBSTR token;
long line;
long column;
long offset;

    getSelection(file,token,offset,line,column);
	m_SelectedFile = file.m_str;
	m_SelectedFile.Replace("\\","/");
	m_SelectedLine = line;
	m_SelectedColumn = column;
	m_SelectedLength = token.Length();
}

CString CCommands::GetSelectedFile() {
	return m_SelectedFile;
}

int CCommands::GetSelectedLine() {
	return m_SelectedLine;
}

int CCommands::GetSelectedColumn() {
	return m_SelectedColumn;
}

int CCommands::GetSelectedLength() {
	return m_SelectedLength;
}

//------------------------------------------------------------------------------
// This function fills file name, line number and ending column number
// for the selected token as well as a token itself.
//------------------------------------------------------------------------------
bool CCommands::getSelection(CComBSTR& file, CComBSTR& token, long& offset, long& line, long& column) {
ITextDocument* iDocument;
ITextSelection* iSelection;

	// Getting the currently opened document object.

	if(m_pApplication->get_ActiveDocument((IDispatch **)&iDocument)!=S_OK) 
		return false;
	if(iDocument==NULL) return false;

	// Reading the file name
	BSTR bstrName;
	iDocument->get_FullName(&bstrName);
	file=bstrName;

	// Getting text selection object from the active document
	if(iDocument->get_Selection((IDispatch **)&iSelection)!=S_OK) 
		return false;

	// We do not need active document object any more.
	iDocument->Release();

	if(iSelection==NULL) return false;

	// Reading currently selected token
	BSTR bstrToken;
	if(iSelection->get_Text(&bstrToken)!=S_OK) 
		return false;
	token = bstrToken;

	long size = token.Length();
	if(iSelection->get_TopLine(&line)!=S_OK)     return false;
	if(iSelection->get_CurrentColumn(&column)!=S_OK) return false;

	VARIANT selMove;
	selMove.vt = VT_INT;
	selMove.intVal = dsMove;
	VARIANT selExtend;
	selExtend.vt = VT_INT;
	selExtend.intVal = dsExtend;
	VARIANT repCount;
	repCount.vt = VT_INT;
	repCount.intVal = 1;

	// To determine which end of the selection the cursor is at,
	// we extend the selection one character left from the cursor 
	// and see whether the selection has gotten longer or shorter.

	bool cursor_at_right_end = true;

	if (column <= 1)
	{
		cursor_at_right_end = false;
	}
	else
	{
		long newsize;
		iSelection->CharLeft(selExtend, repCount);

		if(iSelection->get_Text(&bstrToken)==S_OK) 
		{
			CComBSTR newToken = bstrToken;
			newsize = newToken.Length();
			if (newsize > size)
			{
				// If extending left increases the size, the
				// cursor must be at the left end.
				cursor_at_right_end = false;
			}
		}
		iSelection->CharRight(selExtend, repCount);
	}

	// Calculating column in file characters, not in screen
	// position (detecting tabs)
	long lineOffset = 0;
	long tempColumn = column;
	while (tempColumn > 1) {
		iSelection->CharLeft(selMove, repCount);
		iSelection->get_CurrentColumn(&tempColumn);
		lineOffset++;
	}

	// Restoring selection after calculation:
	// First, go to the end without the cursor
	if (cursor_at_right_end)
	{
		iSelection->MoveTo(line, column-size, selMove);
	}
	else // Cursor at left end of selection
	{
		iSelection->MoveTo(line, column+size, selMove);
	}
	// Now make the selection
	iSelection->MoveTo(line, column, selExtend);

	iSelection->Release();
	column = lineOffset;
	if(size!=0)
		column--;

	offset = -1; // can not determine the offset
	return true;
}
//------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// This public member function opens file with the given name in the Developer Studio
// and highlights the siven line. It also gives focus to the  Developer Studio.
//-------------------------------------------------------------------------------------------
bool CCommands::OpenFileOnLine(CString& file, int line) {
ITextDocument* iDocument;
IDocuments* iDocuments;
ITextSelection* iSelection;

    // First, check if the given file exists. (Developer Studio will hang for a while
    // trying to open non-existing file).
    if(_access(file,04)==-1) return false;


	// Get document collection from the application object.
    if(m_pApplication->get_Documents((IDispatch **)&iDocuments)!=S_OK) 
		return false;

    if(iDocuments==NULL) return false;

	// Open new document and add it to the documents collection (if not exists).
    VARIANT varType;
	varType.vt = VT_BSTR;
	varType.bstrVal=CString("Auto").AllocSysString();

    VARIANT varReadonly;
	varReadonly.vt = VT_BOOL;
	varReadonly.boolVal = FALSE;

    if(iDocuments->Open(file.AllocSysString(),varType,varReadonly,(IDispatch **)&iDocument)!=S_OK) {
		 iDocuments->Release();
		 return false;
	}


	// We do not need documents collection any more
	iDocuments->Release();

    if(iDocument==NULL) return false;

	// Get text selection from the current document
    if(iDocument->get_Selection((IDispatch **)&iSelection)!=S_OK) return false;


	// We do not need document object any more.
	iDocument->Release();

    if(iSelection==NULL) return false;

	// Move to the first position in the given line
    VARIANT selAct;
	selAct.vt = VT_INT;
	selAct.intVal = dsMove;
	iSelection->MoveTo(line,1,selAct);

	// Move to the end of this line in "Shift key pressed" mode.
	selAct.intVal = dsExtend;
	iSelection->EndOfLine(selAct);

	// We do not need text selection object any more
	iSelection->Release();


	// Pop-up MSDEV application.
	m_pApplication->put_Active(VARIANT_TRUE);

	return true;

}



//-------------------------------------------------------------------------------------------
// This public member function opens file with the given name in the Developer Studio
// and highlights the siven line. It also gives focus to the  Developer Studio.
//-------------------------------------------------------------------------------------------
bool CCommands::MakeSelection(CString& file, int line, int col, int len) {
ITextDocument* iDocument;
IDocuments* iDocuments;
ITextSelection* iSelection;

    // First, check if the given file exists. (Developer Studio will hang for a while
    // trying to open non-existing file).
    if(_access(file,04)==-1) return false;


	// Get document collection from the application object.
    if(m_pApplication->get_Documents((IDispatch **)&iDocuments)!=S_OK) 
		return false;

    if(iDocuments==NULL) return false;

	// Open new document and add it to the documents collection (if not exists).
    VARIANT varType;
	varType.vt = VT_BSTR;
	varType.bstrVal=CString("Auto").AllocSysString();

    VARIANT varReadonly;
	varReadonly.vt = VT_BOOL;
	varReadonly.boolVal = FALSE;

    if(iDocuments->Open(file.AllocSysString(),varType,varReadonly,(IDispatch **)&iDocument)!=S_OK) {
		 iDocuments->Release();
		 return false;
	}


	// We do not need documents collection any more
	iDocuments->Release();

    if(iDocument==NULL) return false;

	// Get text selection from the current document
    if(iDocument->get_Selection((IDispatch **)&iSelection)!=S_OK) return false;


	// We do not need document object any more.
	iDocument->Release();

    if(iSelection==NULL) return false;

	// Move to the first position in the given line
    VARIANT selAct;
	selAct.vt = VT_INT;
	selAct.intVal = dsMove;
	iSelection->MoveTo(line,1,selAct);

	VARIANT repCount;
	repCount.vt = VT_INT;
	repCount.intVal = col-1;
	iSelection->CharRight(selAct, repCount);

	// Move to the end of this line in "Shift key pressed" mode.
	selAct.intVal = dsExtend;
	repCount.intVal = len;
	iSelection->CharRight(selAct, repCount);

	// We do not need text selection object any more
	iSelection->Release();


	// Pop-up MSDEV application.
	m_pApplication->put_Active(VARIANT_TRUE);

	return true;

}


//------------------------------------------------------------------------------
// This function determines the current project and calls StartCodeRover
// with the appropriate arguments.
//------------------------------------------------------------------------------
STDMETHODIMP CCommands::CodeRoverBuildMethod() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	/*
	VERIFY_OK(m_pApplication->EnableModeless(VARIANT_FALSE));
	::MessageBox(NULL, "VSAddIn Command invoked.", "VSAddIn", MB_OK | MB_ICONINFORMATION);
	VERIFY_OK(m_pApplication->EnableModeless(VARIANT_TRUE));
	*/

	CComPtr<IDispatch> pProject;
	m_pApplication->get_ActiveProject(&pProject);
	if(pProject != NULL) {
		IGenericProject *p = NULL;
		pProject.QueryInterface(&p);
		CComBSTR bszProjectName("");
		CComBSTR bszProjectFileName("");
		p->get_Name(&bszProjectName);
		p->get_FullName(&bszProjectFileName);
		p->Release();
		StartCodeRover(CString(BSTR(bszProjectName)), CString(BSTR(bszProjectFileName)));
	} else {
		StartCodeRover(NULL, NULL);
	}

	return S_OK;
}

//------------------------------------------------------------------------------
// This function launches the ModelBuild GUI.
//------------------------------------------------------------------------------
void CCommands::StartCodeRover(const char *projectName, const char *projectFileName) {
	char homeName[2048];

	DWORD result = GetEnvironmentVariable("CODE_ROVER_HOME", homeName, sizeof(homeName));
	if(result == 0) {
		VERIFY_OK(m_pApplication->EnableModeless(VARIANT_FALSE));
		::MessageBox(NULL, "Enviroment variable CODE_ROVER_HOME is not set.", "Code Rover Addin", MB_OK | MB_ICONINFORMATION);
		VERIFY_OK(m_pApplication->EnableModeless(VARIANT_TRUE));
		return;
	}
	STARTUPINFO sInfo;
	memset(&sInfo, 0, sizeof(sInfo));
	sInfo.cb = sizeof(sInfo);
	PROCESS_INFORMATION pInfo;
	memset(&pInfo,0,sizeof(PROCESS_INFORMATION));
	
	CString commandLine;
	if(projectName != NULL)
		commandLine += "-projectName \"" + CString(projectName) + "\"";
	if(projectFileName != NULL)
		commandLine += " -projectFile \"" + CString(projectFileName) + "\"";
	
	char *commandLineBuf = new char[commandLine.GetLength() + 2];
	strcpy(commandLineBuf, commandLine);
	char	callString[2048];
	strcpy(callString,"\"");
	strcat(callString,homeName);
	strcat(callString,"\\bin\\mbdriver.exe\" ");
	strcat(callString," ");
	strcat(callString,commandLineBuf);
	sInfo.wShowWindow =SW_HIDE;
	sInfo.dwFlags = STARTF_USESHOWWINDOW;
	if(!::CreateProcess(NULL, callString, NULL, NULL, TRUE, 0,
		NULL, NULL, &sInfo, &pInfo)) {
		free(commandLineBuf);
		CString msg;
		msg.Format("Unable to start %s\\bin\\mbdriver.exe", homeName);
		VERIFY_OK(m_pApplication->EnableModeless(VARIANT_FALSE));
		::MessageBox(NULL, msg, "Code Rover Addin", MB_OK | MB_ICONINFORMATION);
		VERIFY_OK(m_pApplication->EnableModeless(VARIANT_TRUE));
		return;
	
	}
	free(commandLineBuf);
}


void CCommands::CloseAllCommunications() {
	connections->RemoveAllClients();

}
