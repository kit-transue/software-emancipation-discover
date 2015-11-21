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
// VSAddIn.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "stdio.h"
#include <initguid.h>
#include "VSAddIn.h"
#include "DSAddIn.h"
#include "Commands.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_DSAddIn, CDSAddIn)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSAddInApp

class CVSAddInApp : public CWinApp
{
public:
	CVSAddInApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVSAddInApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CVSAddInApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CVSAddInApp

BEGIN_MESSAGE_MAP(CVSAddInApp, CWinApp)
	//{{AFX_MSG_MAP(CVSAddInApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// The one and only CVSAddInApp object

CVSAddInApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CVSAddInApp construction

CVSAddInApp::CVSAddInApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// CVSAddInApp initialization

BOOL CVSAddInApp::InitInstance()
{
	_Module.Init(ObjectMap, m_hInstance);
	return CWinApp::InitInstance();
}

int CVSAddInApp::ExitInstance()
{
	_Module.Term();
	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// Special entry points required for inproc servers

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return _Module.GetClassObject(rclsid, riid, ppv);
}

STDAPI DllCanUnloadNow(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

// by exporting DllRegisterServer, you can use regsvr32.exe
STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT hRes = S_OK;
	
	// Registers object, typelib and all interfaces in typelib
	hRes = _Module.RegisterServer(TRUE);
	if (FAILED(hRes))
		return hRes;

	// Register description of this add-in object in its own
	//  "/Description" subkey.
	// TODO:  If you add more add-ins to this module, you need
	//  to register all of their descriptions, each description
	//  in each add-in object's registry CLSID entry:
	// HKEY_CLASSES_ROOT\Clsid\{add-in CLSID}\Description="add-in description"
	_ATL_OBJMAP_ENTRY* pEntry = _Module.m_pObjMap;
	CRegKey key;
	LONG lRes = key.Open(HKEY_CLASSES_ROOT, _T("CLSID"), KEY_READ);
	if (lRes == ERROR_SUCCESS)
	{
		char	str[1024];
		USES_CONVERSION;
		LPOLESTR lpOleStr;
		StringFromCLSID(*pEntry->pclsid, &lpOleStr);
		LPTSTR lpsz = OLE2T(lpOleStr);

		lRes = key.Open(key, lpsz, KEY_WRITE);
		if (lRes == ERROR_SUCCESS)
		{
			CString strDescription;
			strDescription.LoadString(IDS_VSADDIN_DESCRIPTION);
			key.SetKeyValue(_T("Description"), strDescription);
		} else {
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL );
			// Free the buffer.
 			sprintf(str,"Can not write %s subkey\nReason: %s\nIntegration disabled.",lpsz,lpMsgBuf);
			MessageBox(NULL,str,"Warning",MB_ICONINFORMATION);
			LocalFree( lpMsgBuf );
		}
		CoTaskMemFree(lpOleStr);
	} else {
		MessageBox(NULL,"Can not read key \"CLSID\"\nIntegration disabled.","Warning",MB_ICONINFORMATION);
	}
	if (lRes != ERROR_SUCCESS)
		hRes = HRESULT_FROM_WIN32(lRes);

	return hRes;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hRes = S_OK;
	_Module.UnregisterServer();
	return hRes;
}


/////////////////////////////////////////////////////////////////////////////
// Debugging support

// GetLastErrorDescription is used in the implementation of the VERIFY_OK
//  macro, defined in stdafx.h.

#ifdef _DEBUG

void GetLastErrorDescription(CComBSTR& bstr)
{
	CComPtr<IErrorInfo> pErrorInfo;
	if (GetErrorInfo(0, &pErrorInfo) == S_OK)
		pErrorInfo->GetDescription(&bstr);
}

#endif //_DEBUG
