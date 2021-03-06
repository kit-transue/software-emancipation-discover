// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "cbrowserframe.h"

/////////////////////////////////////////////////////////////////////////////
// CCBrowserFrame

IMPLEMENT_DYNCREATE(CCBrowserFrame, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CCBrowserFrame properties

CString CCBrowserFrame::GetArguments()
{
	CString result;
	GetProperty(0x1, VT_BSTR, (void*)&result);
	return result;
}

void CCBrowserFrame::SetArguments(LPCTSTR propVal)
{
	SetProperty(0x1, VT_BSTR, propVal);
}

CString CCBrowserFrame::GetResultsFromCategory()
{
	CString result;
	GetProperty(0x2, VT_BSTR, (void*)&result);
	return result;
}

void CCBrowserFrame::SetResultsFromCategory(LPCTSTR propVal)
{
	SetProperty(0x2, VT_BSTR, propVal);
}

CString CCBrowserFrame::GetResultsFromQuery()
{
	CString result;
	GetProperty(0x3, VT_BSTR, (void*)&result);
	return result;
}

void CCBrowserFrame::SetResultsFromQuery(LPCTSTR propVal)
{
	SetProperty(0x3, VT_BSTR, propVal);
}

CString CCBrowserFrame::GetResultsFromGroup()
{
	CString result;
	GetProperty(0x4, VT_BSTR, (void*)&result);
	return result;
}

void CCBrowserFrame::SetResultsFromGroup(LPCTSTR propVal)
{
	SetProperty(0x4, VT_BSTR, propVal);
}

CString CCBrowserFrame::GetSelectionFromQuery()
{
	CString result;
	GetProperty(0x5, VT_BSTR, (void*)&result);
	return result;
}

void CCBrowserFrame::SetSelectionFromQuery(LPCTSTR propVal)
{
	SetProperty(0x5, VT_BSTR, propVal);
}

CString CCBrowserFrame::GetSelectionFromCategory()
{
	CString result;
	GetProperty(0x6, VT_BSTR, (void*)&result);
	return result;
}

void CCBrowserFrame::SetSelectionFromCategory(LPCTSTR propVal)
{
	SetProperty(0x6, VT_BSTR, propVal);
}

CString CCBrowserFrame::GetSelectionFromGroup()
{
	CString result;
	GetProperty(0x7, VT_BSTR, (void*)&result);
	return result;
}

void CCBrowserFrame::SetSelectionFromGroup(LPCTSTR propVal)
{
	SetProperty(0x7, VT_BSTR, propVal);
}

CString CCBrowserFrame::GetProjects()
{
	CString result;
	GetProperty(0x8, VT_BSTR, (void*)&result);
	return result;
}

void CCBrowserFrame::SetProjects(LPCTSTR propVal)
{
	SetProperty(0x8, VT_BSTR, propVal);
}

short CCBrowserFrame::GetMode()
{
	short result;
	GetProperty(0x9, VT_I2, (void*)&result);
	return result;
}

void CCBrowserFrame::SetMode(short propVal)
{
	SetProperty(0x9, VT_I2, propVal);
}

LPDISPATCH CCBrowserFrame::GetDataSource()
{
	LPDISPATCH result;
	GetProperty(0xa, VT_DISPATCH, (void*)&result);
	return result;
}

void CCBrowserFrame::SetDataSource(LPDISPATCH propVal)
{
	SetProperty(0xa, VT_DISPATCH, propVal);
}

CString CCBrowserFrame::GetSelectedCategories()
{
	CString result;
	GetProperty(0xb, VT_BSTR, (void*)&result);
	return result;
}

void CCBrowserFrame::SetSelectedCategories(LPCTSTR propVal)
{
	SetProperty(0xb, VT_BSTR, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CCBrowserFrame operations

void CCBrowserFrame::Update()
{
	InvokeHelper(0xc, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCBrowserFrame::SetSet(LPCTSTR name)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0xd, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 name);
}

void CCBrowserFrame::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}
