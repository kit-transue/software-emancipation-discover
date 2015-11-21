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
// CLoggerTrace.cpp : Implementation of CLoggerTrace
#include "stdafx.h"
#include "Logger.h"
#include "CLoggerTrace.h"
#include "Globals.h"

#define TRACE_FILENAME  "c:\\Temp\\Trace.log"

/////////////////////////////////////////////////////////////////////////////
// CLoggerTrace


CLoggerTrace::~CLoggerTrace()
{
	if ( m_strCommandLine )
		::SysFreeString(m_strCommandLine);
}

STDMETHODIMP CLoggerTrace::get_CommandLine(BSTR * pVal)
{
	if ( m_strCommandLine )
		{
			*pVal = ::SysAllocString( m_strCommandLine );
			if ( *pVal )
				return S_OK;
		}

	return S_FALSE;
}

STDMETHODIMP CLoggerTrace::put_CommandLine(BSTR newVal)
{
	if ( m_strCommandLine )
		::SysFreeString(m_strCommandLine);
	m_strCommandLine = ::SysAllocString( newVal );
	
	if ( m_strCommandLine )
		return S_OK;
	else
		return S_FALSE;
}

STDMETHODIMP CLoggerTrace::get_PID(long * pVal)
{
	*pVal = m_PID;

	return S_OK;
}

STDMETHODIMP CLoggerTrace::put_PID(long newVal)
{
	m_PID = newVal;

	return S_OK;
}

STDMETHODIMP CLoggerTrace::LogInstance(long PID, BSTR strCommandLine)
{
	put_PID(PID);
	return put_CommandLine(strCommandLine);
}

STDMETHODIMP CLoggerTrace::LogEntry(long objEntry, BSTR strEntryName)
{
	char*	pszEntryName = BSTRToStr( strEntryName );
	if ( pszEntryName )
	{
		FILE* f = fopen(TRACE_FILENAME, "a+t" );
		if ( f )
		{
			fprintf(f, "%s\n", pszEntryName );
			fclose(f);
		}

		delete [] pszEntryName;
	}

	return S_OK;
}

STDMETHODIMP CLoggerTrace::LogExit(long objEntry)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CLoggerTrace::LogVal(long objEntry, BSTR strName, VARIANT varVal)
{
	char*	pszName = BSTRToStr( strName );

	if ( pszName )
	{
		FILE* f = fopen(TRACE_FILENAME, "a+t" );
		if ( f )
		{
			fprintf(f, "%s = ", pszName );
			switch ( varVal.vt )
			{
				case VT_BSTR:
				{
					char* pszVal = BSTRToStr( varVal.bstrVal );
					if ( pszVal )
					{
						fprintf(f, "%s\n", pszVal );
						delete [] pszVal;
					}
					else
						fprintf(f, "??? (str=NULL)\n" );
					break;
				}

				case VT_INT: case VT_UINT: case VT_I4: case VT_UI4:
				{
					fprintf(f, "%d\n", varVal.intVal );
					break;
				}
				
				case VT_BOOL:
				{
					if ( varVal.boolVal )
						fprintf(f, "true\n" );
					else
						fprintf(f, "false\n" );
					break;
				}
				
				case VT_PTR: case VT_VOID:
				{
					fprintf(f, "x0%x\n", varVal.intVal );
					break;
				}
				
				default:
				{
					fprintf(f, "??? (x0%x)\n", varVal.intVal );
					break;
				}

			}
			fclose(f);
		}

		delete [] pszName;
	}
	return S_OK;
}
