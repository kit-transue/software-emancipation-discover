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
// Commands.h : header file
//

#if !defined(AFX_COMMANDS_H__ADD9092A_E555_4B40_B64B_AF0E9103D03D__INCLUDED_)
#define AFX_COMMANDS_H__ADD9092A_E555_4B40_B64B_AF0E9103D03D__INCLUDED_

#include "VSAddInTypes.h"


class SocketConnection;
class CDSAddIn;

class CCommands : 
	public CComDualImpl<ICommands, &IID_ICommands, &LIBID_VSAddIn>, 
	public CComObjectRoot,
	public CComCoClass<CCommands, &CLSID_Commands>
{
protected:
	IApplication* m_pApplication;

public:
	CCommands();
	~CCommands();
	void SetApplicationObject(IApplication* m_pApplication);
	IApplication* GetApplicationObject() { return m_pApplication; }
	void UnadviseFromEvents();
    HRESULT ShowMessage(CString msg);

	BEGIN_COM_MAP(CCommands)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ICommands)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CCommands)

protected:
	// This class template is used as the base class for the Application
	//  event handler object and the Debugger event handler object,
	//  which are declared below.
	template <class IEvents, const IID* piidEvents, const GUID* plibid,
		class XEvents, const CLSID* pClsidEvents>
	class XEventHandler :
		public CComDualImpl<IEvents, piidEvents, plibid>,
		public CComObjectRoot,
		public CComCoClass<XEvents, pClsidEvents>
	{
	public:
		BEGIN_COM_MAP(XEvents)
			COM_INTERFACE_ENTRY(IDispatch)
			COM_INTERFACE_ENTRY_IID(*piidEvents, IEvents)
		END_COM_MAP()
		DECLARE_NOT_AGGREGATABLE(XEvents)
		void Connect(IUnknown* pUnk)
		{ VERIFY(SUCCEEDED(AtlAdvise(pUnk, this, *piidEvents, &m_dwAdvise))); }
		void Disconnect(IUnknown* pUnk)
		{ AtlUnadvise(pUnk, *piidEvents, m_dwAdvise); }

		CCommands* m_pCommands;

	protected:
		DWORD m_dwAdvise;
	};

	// This object handles events fired by the Application object
	class XApplicationEvents : public XEventHandler<IApplicationEvents, 
		&IID_IApplicationEvents, &LIBID_VSAddIn, 
		XApplicationEvents, &CLSID_ApplicationEvents>
	{
	public:
		// IApplicationEvents methods
		STDMETHOD(BeforeBuildStart)(THIS);
		STDMETHOD(BuildFinish)(THIS_ long nNumErrors, long nNumWarnings);
		STDMETHOD(BeforeApplicationShutDown)(THIS);
		STDMETHOD(DocumentOpen)(THIS_ IDispatch * theDocument);
		STDMETHOD(BeforeDocumentClose)(THIS_ IDispatch * theDocument);
		STDMETHOD(DocumentSave)(THIS_ IDispatch * theDocument);
		STDMETHOD(NewDocument)(THIS_ IDispatch * theDocument);
		STDMETHOD(WindowActivate)(THIS_ IDispatch * theWindow);
		STDMETHOD(WindowDeactivate)(THIS_ IDispatch * theWindow);
		STDMETHOD(WorkspaceOpen)(THIS);
		STDMETHOD(WorkspaceClose)(THIS);
		STDMETHOD(NewWorkspace)(THIS);
	};
	typedef CComObject<XApplicationEvents> XApplicationEventsObj;
	XApplicationEventsObj* m_pApplicationEventsObj;

	// This object handles events fired by the Application object
	class XDebuggerEvents : public XEventHandler<IDebuggerEvents, 
		&IID_IDebuggerEvents, &LIBID_VSAddIn, 
		XDebuggerEvents, &CLSID_DebuggerEvents>
	{
	public:
		// IDebuggerEvents method
	    STDMETHOD(BreakpointHit)(THIS_ IDispatch * pBreakpoint);
	};
	typedef CComObject<XDebuggerEvents> XDebuggerEventsObj;
	XDebuggerEventsObj* m_pDebuggerEventsObj;

public:
	void CloseAllCommunications(void);
	STDMETHOD(CodeRoverBrowseMethod)();
	STDMETHOD(CodeRoverInfoMethod)(THIS);
	STDMETHOD(CodeRoverOpenDefinitionMethod)(THIS);
	STDMETHOD(CodeRoverBuildMethod)(THIS);
	STDMETHOD(CodeRoverActivateMethod)(THIS);
    bool OpenFileOnLine(CString& file, int line);
    bool MakeSelection(CString& file, int line,int col,int len);
    void FreezeSelection();
    CString GetSelectedFile();
    int GetSelectedLine();
    int GetSelectedColumn();
    int GetSelectedLength();
	void StartCodeRover(const char *projectName, const char *projectFileName);
private:
	SocketConnection* connections;
	CString m_SelectedFile;
	int     m_SelectedLine;
	int     m_SelectedColumn;
	int     m_SelectedLength;
	bool getSelection(CComBSTR& file, CComBSTR& token, long& offset, long& line, long& column);
};

typedef CComObject<CCommands> CCommandsObj;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMANDS_H__ADD9092A_E555_4B40_B64B_AF0E9103D03D__INCLUDED)

