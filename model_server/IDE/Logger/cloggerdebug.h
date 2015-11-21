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
// CLoggerDebug.h : Declaration of the CLoggerDebug

#ifndef __LOGGERDEBUG_H_
#define __LOGGERDEBUG_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CLoggerDebug
class ATL_NO_VTABLE CLoggerDebug : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLoggerDebug, &CLSID_LoggerDebug>,
	public IDispatchImpl<ILoggerDebug, &IID_ILoggerDebug, &LIBID_LOGGERLib>
{
public:
	CLoggerDebug()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_LOGGERDEBUG)

BEGIN_COM_MAP(CLoggerDebug)
	COM_INTERFACE_ENTRY(ILoggerDebug)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// ILoggerDebug
public:
};

#endif //__LOGGERDEBUG_H_
