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
///////////////////////////////////////////////////////////////////////////
//
//	Tracer
//
///////////////////////////////////////////////////////////////////////////
#ifndef	__NOTRACE

#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <stdio.h>

#define __LOGGER_CPP_

#include "loggerauto.h"
#include "COM_trace.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
class __CLoggerTraceInstance
{
public:

	ILoggerTrace*	m_pILoggerTrace;
	BOOL					m_bEnable;

	__CLoggerTraceInstance()
		{
			m_pILoggerTrace	 = NULL;
			m_bEnable  = TRUE;
			CoInitialize(NULL);
		}

 ~__CLoggerTraceInstance()
		{
			if ( m_pILoggerTrace )
			{
				m_pILoggerTrace->Release();
				m_pILoggerTrace = NULL;
			}
			CoUninitialize();
		}

static	char*		BSTRToStr		(BSTR bstr);
static	BSTR		StrToBSTR		(char* psz);

	BOOL	TraceStart();
	BOOL	TraceEnabled();

	BOOL	TraceEntry	(void* TarceObject, int iLine,char* pszFName,char* pszMethod,char* pszClass);
	void	TraceExit		(void* TraceObject);
	void	TraceString	(void* TarceObject, int iLine, char*	pszName, char*	pszVal=NULL);
	void	TraceString	(void* TarceObject, int iLine, char*	pszName, wchar_t* strVal=NULL);
	void	TraceInt		(void* TarceObject, int iLine, char*	pszName, int nVal);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
BOOL __CLoggerTraceInstance::TraceStart()
{
	if ( !m_bEnable )
		return FALSE;
	if ( m_pILoggerTrace )
		return TRUE;

	HRESULT		hr = S_OK;
	CLSID			clsid;

	wchar_t*  wszLoggerProgID = L"LoggerTrace.Logger.1";
	hr = CLSIDFromProgID(wszLoggerProgID, &clsid);
	if (FAILED(hr))
	{
		m_bEnable = FALSE;
		return m_bEnable;
	}

	LPUNKNOWN pUnk = NULL;
	hr = CoCreateInstance(clsid, NULL, CLSCTX_SERVER, 
												IID_IUnknown, (void**)&pUnk);
	if (FAILED(hr))
	{
		m_bEnable = FALSE;
		return FALSE;
	}

	hr = pUnk->QueryInterface(IID_ILoggerTrace, (void**)&m_pILoggerTrace );
	pUnk->Release();
	if ( FAILED(hr) )
		{
			m_bEnable = FALSE;
			return FALSE;
		}
	BSTR bstrCommandLine = StrToBSTR( (char*)GetCommandLine() );
	if (bstrCommandLine)
	{
		hr = m_pILoggerTrace->LogInstance( GetCurrentProcessId(), bstrCommandLine );
		::SysFreeString(bstrCommandLine);

		if (FAILED( hr ))
			m_bEnable = FALSE;
	}
	return TRUE;
}

//------------------------------------------
//
BOOL __CLoggerTraceInstance::TraceEnabled()
{
	if ( !m_bEnable )
		return FALSE;
	if ( m_pILoggerTrace )
		return TRUE;
	if ( !TraceStart() )
		return FALSE;
	return TRUE;
}
//------------------------------------------
//
BSTR __CLoggerTraceInstance::StrToBSTR(char* psz)
{
  if ( psz == NULL ) 
    return NULL;

  BSTR			bstr = NULL;
  int       nLen = strlen( psz ) +1;
  wchar_t*  wsz = new wchar_t [ nLen * sizeof(wchar_t) ];
  if ( wsz == NULL )
    return NULL;

  mbstowcs( wsz, psz, nLen );
  bstr = ::SysAllocString( wsz );
  delete [] wsz;
	return bstr;
}

//------------------------------------------
//
char* __CLoggerTraceInstance::BSTRToStr(BSTR bstr)
{
  if ( bstr == NULL ) 
    return NULL;

  wchar_t*  wsz = bstr;
  int       nLen = wcslen( wsz ) +1;
  char*     psz = new char [ nLen ];
  if ( psz == NULL )
    return NULL;
  
  wcstombs(psz, wsz, nLen);
	return psz;
}

//------------------------------------------
//
BOOL __CLoggerTraceInstance::TraceEntry	(void* TarceObject, int iLine,char* pszFName,char* pszMethod,char* pszClass)
{
	if ( !TraceEnabled() )
		return FALSE;

	int		nLen = strlen(pszFName) + strlen(pszMethod) + strlen(pszClass) + 64;
	char*	pszEntryName = new char [ nLen ];
	if ( pszEntryName == NULL )
		return FALSE;

	sprintf( pszEntryName, "%s:%d:%s:%s", pszFName, iLine, pszClass, pszMethod );
	BSTR	strEntryName = StrToBSTR(pszEntryName);
	delete [] pszEntryName;
	if ( strEntryName == NULL )
		return FALSE;

	HRESULT hr = m_pILoggerTrace->LogEntry( (long)TarceObject, strEntryName );
	if (FAILED( hr ))
	{
		m_bEnable = FALSE;
		return FALSE;
	}

	return ( hr == S_OK) ;
}

//------------------------------------------
//
void __CLoggerTraceInstance::TraceExit(void* TraceObject)
{
	if ( !TraceEnabled() )
		return;

	HRESULT hr = m_pILoggerTrace->LogExit( (long)TraceObject );
	if (FAILED( hr ))
		m_bEnable = FALSE;
}

//------------------------------------------
//
void __CLoggerTraceInstance::TraceString(void* TarceObject, int iLine, char*	pszName, char*	pszVal) 
{
	if ( !TraceEnabled() )
		return;

	if ( pszName == NULL || *pszName == 0 )
		return;
	if ( pszVal == NULL )
		pszVal = "";

	int		nLen = strlen(pszName) + 64;
	char*	pszValName = new char [ nLen ];
	if ( pszValName == NULL )
		return;

	sprintf( pszValName, "%s:%d", pszName, iLine );
	BSTR	strName = StrToBSTR(pszName);
	delete [] pszValName;
	if ( strName == NULL )
		return;

	BSTR	strVal = StrToBSTR(pszVal);
	if ( strVal == NULL )
		return;

	VARIANT		varVal;
	::VariantInit(&varVal);
	varVal.vt = VT_BSTR;
	varVal.bstrVal = strVal;

	HRESULT hr = m_pILoggerTrace->LogVal( (long)TarceObject, strName, varVal );

	::SysFreeString( strName );
	::SysFreeString( strVal  );
	if (FAILED( hr ))
		m_bEnable = FALSE;
}

//------------------------------------------
//
void __CLoggerTraceInstance::TraceString(void* TarceObject, int iLine, char*	pszName, wchar_t*	wszVal) 
{
	if ( !TraceEnabled() )
		return;

	if ( pszName == NULL || *pszName == 0 )
		return;
	if ( wszVal == NULL )
		wszVal = L"";

	int		nLen = strlen(pszName) + 64;
	char*	pszValName = new char [ nLen ];
	if ( pszValName == NULL )
		return;

	sprintf( pszValName, "%s:%d", pszName, iLine );
	BSTR	strName = StrToBSTR(pszName);
	delete [] pszValName;
	if ( strName == NULL )
		return;

	BSTR	strVal = wszVal;
	if ( strVal == NULL )
		return;

	VARIANT		varVal;
	::VariantInit(&varVal);
	varVal.vt = VT_BSTR;
	varVal.bstrVal = strVal;

	HRESULT hr = m_pILoggerTrace->LogVal( (long)TarceObject, strName, varVal );

	::SysFreeString( strName );
	if (FAILED( hr ))
		m_bEnable = FALSE;
}

//------------------------------------------
//
void __CLoggerTraceInstance::TraceInt(void* TarceObject, int iLine, char*	pszName, int nVal)
{
	if ( !TraceEnabled() )
		return;

	int		nLen = strlen(pszName) + 64;
	char*	pszValName = new char [ nLen ];
	if ( pszValName == NULL )
		return;

	sprintf( pszValName, "%s:%d", pszName, iLine );
	BSTR	strName = StrToBSTR(pszName);
	delete [] pszValName;
	if ( strName == NULL )
		return;

	VARIANT		varVal;
	::VariantInit(&varVal);
	varVal.vt = VT_I4;
	varVal.intVal = nVal;

	HRESULT hr = m_pILoggerTrace->LogVal( (long)TarceObject, strName, varVal );

	::SysFreeString( strName );
	if (FAILED( hr ))
		m_bEnable = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

static __CLoggerTraceInstance __TraceInstance;


////////////////////////////////////////////////////////////////////////////////////////////////////////////
__CLoggerTrace::~__CLoggerTrace()
 {
	if ( m_bEnable )
		__TraceInstance.TraceExit( (void*)this );
 }

//----------------------------------
//
__CLoggerTrace::__CLoggerTrace(int iLine, char* pszFName, char* pszMethod, char* pszClass, void* Object )
{
	m_cRef = 1;
	m_bEnable = __TraceInstance.TraceEntry( (void*)this, iLine, pszFName, pszMethod, pszClass );
}

//----------------------------------
//
void __CLoggerTrace::TraceString(int iLine, char* pszName, char* pszString )
{
	if ( m_bEnable )
		__TraceInstance.TraceString( (void*)this, iLine, pszName, pszString );
}

//----------------------------------------------------
//
void __CLoggerTrace::TraceString(int iLine, char* pszName, wchar_t* wszString )
{
	if ( m_bEnable )
		__TraceInstance.TraceString( (void*)this, iLine, pszName, wszString );
}

//----------------------------------------------------
//
void __CLoggerTrace::TraceInt(int iLine, char* pszName, int nInt )
{
	if ( m_bEnable )
		__TraceInstance.TraceInt( (void*)this, iLine, pszName, nInt );
}

#endif	// #ifndef __NOTRACE
