// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "coderoverintegrator.h"

/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegrator

IMPLEMENT_DYNCREATE(CCodeRoverIntegrator, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegrator properties

CString CCodeRoverIntegrator::GetEditorName()
{
	CString result;
	GetProperty(0x1, VT_BSTR, (void*)&result);
	return result;
}

void CCodeRoverIntegrator::SetEditorName(LPCTSTR propVal)
{
	SetProperty(0x1, VT_BSTR, propVal);
}

CString CCodeRoverIntegrator::GetEditorsList()
{
	CString result;
	GetProperty(0x2, VT_BSTR, (void*)&result);
	return result;
}

void CCodeRoverIntegrator::SetEditorsList(LPCTSTR propVal)
{
	SetProperty(0x2, VT_BSTR, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CCodeRoverIntegrator operations

void CCodeRoverIntegrator::MakeSelection(LPCTSTR filename, long line, long column, long len)
{
	static BYTE parms[] =
		VTS_BSTR VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 filename, line, column, len);
}

void CCodeRoverIntegrator::OpenFile(LPCTSTR filename, long line)
{
	static BYTE parms[] =
		VTS_BSTR VTS_I4;
	InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 filename, line);
}

void CCodeRoverIntegrator::SetInfo(LPCTSTR text)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 text);
}

BOOL CCodeRoverIntegrator::Reconnect()
{
	BOOL result;
	InvokeHelper(0x6, DISPATCH_METHOD, VT_BOOL, (void*)&result, NULL);
	return result;
}

long CCodeRoverIntegrator::GetModelLine(LPCTSTR pszLocalName, LPCTSTR pszBaseName, long nLine)
{
	long result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR VTS_I4;
	InvokeHelper(0x7, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		pszLocalName, pszBaseName, nLine);
	return result;
}
