// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "cmintegrator.h"

/////////////////////////////////////////////////////////////////////////////
// CCMIntegrator

IMPLEMENT_DYNCREATE(CCMIntegrator, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CCMIntegrator properties

CString CCMIntegrator::GetCMName()
{
	CString result;
	GetProperty(0x1, VT_BSTR, (void*)&result);
	return result;
}

void CCMIntegrator::SetCMName(LPCTSTR propVal)
{
	SetProperty(0x1, VT_BSTR, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CCMIntegrator operations

CString CCMIntegrator::GetCMs()
{
	CString result;
	InvokeHelper(0x2, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

CString CCMIntegrator::GetCommands()
{
	CString result;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

BOOL CCMIntegrator::Connect(LPCTSTR szName)
{
	BOOL result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x4, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		szName);
	return result;
}

BOOL CCMIntegrator::IsConnected()
{
	BOOL result;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_BOOL, (void*)&result, NULL);
	return result;
}

BOOL CCMIntegrator::IsAvailable()
{
	BOOL result;
	InvokeHelper(0x6, DISPATCH_METHOD, VT_BOOL, (void*)&result, NULL);
	return result;
}

CString CCMIntegrator::GetAttributes()
{
	CString result;
	InvokeHelper(0x7, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

CString CCMIntegrator::ExecCommand(LPCTSTR szCommand)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x8, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		szCommand);
	return result;
}

BOOL CCMIntegrator::Reconnect()
{
	BOOL result;
	InvokeHelper(0x9, DISPATCH_METHOD, VT_BOOL, (void*)&result, NULL);
	return result;
}
