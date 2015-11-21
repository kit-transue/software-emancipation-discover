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
// CLoggerTrace.h : Declaration of the CLoggerTrace

#ifndef __LOGGERTRACE_H_
#define __LOGGERTRACE_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CLoggerTrace
class ATL_NO_VTABLE CLoggerTrace : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLoggerTrace, &CLSID_LoggerTrace>,
	public IDispatchImpl<ILoggerTrace, &IID_ILoggerTrace, &LIBID_LOGGERLib>
{
public:
	CLoggerTrace()
	{
		m_strCommandLine = NULL;
		m_PID = 0;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_LOGGERTRACE)

BEGIN_COM_MAP(CLoggerTrace)
	COM_INTERFACE_ENTRY(ILoggerTrace)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// ILoggerTrace
public:
	STDMETHOD(LogVal)(/*[in]*/ long objEntry, /*[in,string]*/ BSTR strName, /*[in]*/ VARIANT varVal);
	STDMETHOD(LogExit)(/*[in]*/ long objEntry);
	STDMETHOD(LogEntry)(/*[in]*/ long objEntry, /*[in,string]*/ BSTR strEntryName);
	STDMETHOD(LogInstance)(/*[in]*/ long PID, /*[in,string]*/ BSTR strCommandLine);
	STDMETHOD(get_PID)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_PID)(/*[in]*/ long newVal);
	STDMETHOD(get_CommandLine)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_CommandLine)(/*[in]*/ BSTR newVal);
	 ~CLoggerTrace();
private:
	long m_PID;
	BSTR m_strCommandLine;
};

#endif //__LOGGERTRACE_H_
