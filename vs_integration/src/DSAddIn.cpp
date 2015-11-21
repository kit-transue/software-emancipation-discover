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
// AddInMod.cpp : implementation file
//

#include "stdafx.h"
#include "VSAddIn.h"
#include "DSAddIn.h"
#include "Commands.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static IApplication* theApp;
static HINSTANCE theInstance;

// This is called when the user first loads the add-in, and on start-up
//  of each subsequent Developer Studio session
STDMETHODIMP CDSAddIn::OnConnection(IApplication* pApp, VARIANT_BOOL bFirstTime,
		long dwCookie, VARIANT_BOOL* OnConnection)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp = pApp;
	
	// Store info passed to us
	IApplication* pApplication = NULL;
	if (FAILED(pApp->QueryInterface(IID_IApplication, (void**) &pApplication))
		|| pApplication == NULL)
	{
		*OnConnection = VARIANT_FALSE;
		return S_OK;
	}

	m_dwCookie = dwCookie;

	// Create command dispatch, send info back to DevStudio
	CCommandsObj::CreateInstance(&m_pCommands);
	m_pCommands->AddRef();

	// The QueryInterface above AddRef'd the Application object.  It will
	//  be Release'd in CCommand's destructor.
	m_pCommands->SetApplicationObject(pApplication);
	// (see stdafx.h for the definition of VERIFY_OK)

    theInstance = AfxGetInstanceHandle();
	VERIFY_OK(pApplication->SetAddInInfo((long) AfxGetInstanceHandle(),
		(LPDISPATCH) m_pCommands, IDR_TOOLBAR_MEDIUM, IDR_TOOLBAR_LARGE, m_dwCookie));

	// Inform DevStudio of the commands we implement

	// TODO: Replace the AddCommand call below with a series of calls,
	//  one for each command your add-in will add.

	// The command name should not be localized to other languages.  The 
	//  tooltip, command description, and other strings related to this
	//  command are stored in the string table (IDS_CMD_STRING) and should
	//  be localized.
	VARIANT_BOOL bRet;

    // Adding "Build" command
	LPCTSTR szBuildCommand = _T("CodeRoverBuild");
	CString strBuildCmdString;
	strBuildCmdString.LoadString(IDS_BUILD_CMD_STRING);
	strBuildCmdString = szBuildCommand + strBuildCmdString;
	CComBSTR bszBuildCmdString(strBuildCmdString);
	CComBSTR bszBuildMethod(_T("CodeRoverBuildMethod"));
	CComBSTR bszBuildCmdName(szBuildCommand);
	VERIFY_OK(pApplication->AddCommand(bszBuildCmdString, bszBuildMethod, 0, m_dwCookie, &bRet));
	if (bRet == VARIANT_FALSE) {
		*OnConnection = VARIANT_FALSE;
		return S_OK;
	}

	
    // Adding "Info" command
	LPCTSTR szInfoCommand = _T("CodeRoverInfo");
	CString strInfoCmdString;
	strInfoCmdString.LoadString(IDS_INFO_CMD_STRING);
	strInfoCmdString = szInfoCommand + strInfoCmdString;
	CComBSTR bszInfoCmdString(strInfoCmdString);
	CComBSTR bszInfoMethod(_T("CodeRoverInfoMethod"));
	CComBSTR bszInfoCmdName(szInfoCommand);
	VERIFY_OK(pApplication->AddCommand(bszInfoCmdString, bszInfoMethod, 1, m_dwCookie, &bRet));
	if (bRet == VARIANT_FALSE) {
		*OnConnection = VARIANT_FALSE;
		return S_OK;
	}

	// Adding "Open definition" command
	LPCTSTR szOpenDefCommand = _T("CodeRoverOpenDefinition");
	CString strOpenDefCmdString;
	strOpenDefCmdString.LoadString(IDS_OPENDEF_CMD_STRING);
	strOpenDefCmdString = szOpenDefCommand + strOpenDefCmdString;
	CComBSTR bszOpenDefCmdString(strOpenDefCmdString);
	CComBSTR bszOpenDefMethod(_T("CodeRoverOpenDefinitionMethod"));
	CComBSTR bszOpenDefCmdName(szOpenDefCommand);
	VERIFY_OK(pApplication->AddCommand(bszOpenDefCmdString, bszOpenDefMethod, 2, m_dwCookie, &bRet));
	if (bRet == VARIANT_FALSE) {
		*OnConnection = VARIANT_FALSE;
		return S_OK;
	}

	// Adding "Browse" command
	LPCTSTR szBrowseCommand = _T("CodeRoverBrowse");
	CString strBrowseCmdString;
	strBrowseCmdString.LoadString(IDS_BROWSE_CMD_STRING);
	strBrowseCmdString = szBrowseCommand + strBrowseCmdString;
	CComBSTR bszBrowseCmdString(strBrowseCmdString);
	CComBSTR bszBrowseMethod(_T("CodeRoverBrowseMethod"));
	CComBSTR bszBrowseCmdName(szBrowseCommand);
	VERIFY_OK(pApplication->AddCommand(bszBrowseCmdString, bszBrowseMethod, 3, m_dwCookie, &bRet));
	if (bRet == VARIANT_FALSE) {
		*OnConnection = VARIANT_FALSE;
		return S_OK;
	}

	// Adding "Activate" command
	LPCTSTR szActivateCommand = _T("CodeRoverActivate");
	CString strActivateCmdString;
	strActivateCmdString.LoadString(IDS_ACTIVATE_CMD_STRING);
	strActivateCmdString = szActivateCommand + strActivateCmdString;
	CComBSTR bszActivateCmdString(strActivateCmdString);
	CComBSTR bszActivateMethod(_T("CodeRoverActivateMethod"));
	CComBSTR bszActivateCmdName(szActivateCommand);
	VERIFY_OK(pApplication->AddCommand(bszActivateCmdString, bszActivateMethod, 4, m_dwCookie, &bRet));
	if (bRet == VARIANT_FALSE) {
		*OnConnection = VARIANT_FALSE;
		return S_OK;
	}


	// Add toolbar buttons only if this is the first time the add-in
	//  is being loaded.  Toolbar buttons are automatically remembered
	//  by Developer Studio from session to session, so we should only
	//  add the toolbar buttons once.
	if (bFirstTime == VARIANT_TRUE)
	{
		if(getenv("CODE_ROVER_HOME")!=NULL) { // add this button only for CodeRover
			VERIFY_OK(pApplication->
				AddCommandBarButton(dsGlyph, bszBuildCmdName, m_dwCookie));
		} 

		VERIFY_OK(pApplication->
			AddCommandBarButton(dsGlyph, bszInfoCmdName, m_dwCookie));
		VERIFY_OK(pApplication->
			AddCommandBarButton(dsGlyph, bszOpenDefCmdName, m_dwCookie));
		VERIFY_OK(pApplication->
			AddCommandBarButton(dsGlyph, bszBrowseCmdName, m_dwCookie));
		VERIFY_OK(pApplication->
			AddCommandBarButton(dsGlyph, bszActivateCmdName, m_dwCookie));
	}

	*OnConnection = VARIANT_TRUE;
	return S_OK;
}

// This is called on shut-down, and also when the user unloads the add-in
STDMETHODIMP CDSAddIn::OnDisconnection(VARIANT_BOOL bLastTime)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_pCommands->CloseAllCommunications(); 
	m_pCommands->UnadviseFromEvents();
	m_pCommands->Release();
	m_pCommands = NULL;

	// TODO: Perform any cleanup work here

	return S_OK;
}
