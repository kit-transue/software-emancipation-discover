// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "tabselector.h"

/////////////////////////////////////////////////////////////////////////////
// CTabSelector

IMPLEMENT_DYNCREATE(CTabSelector, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CTabSelector properties

LPDISPATCH CTabSelector::GetDataSource()
{
	LPDISPATCH result;
	GetProperty(0x1, VT_DISPATCH, (void*)&result);
	return result;
}

void CTabSelector::SetDataSource(LPDISPATCH propVal)
{
	SetProperty(0x1, VT_DISPATCH, propVal);
}

CString CTabSelector::GetOutputVariable()
{
	CString result;
	GetProperty(0x2, VT_BSTR, (void*)&result);
	return result;
}

void CTabSelector::SetOutputVariable(LPCTSTR propVal)
{
	SetProperty(0x2, VT_BSTR, propVal);
}

CString CTabSelector::GetSelectionVariable()
{
	CString result;
	GetProperty(0x3, VT_BSTR, (void*)&result);
	return result;
}

void CTabSelector::SetSelectionVariable(LPCTSTR propVal)
{
	SetProperty(0x3, VT_BSTR, propVal);
}

CString CTabSelector::GetCaption()
{
	CString result;
	GetProperty(DISPID_CAPTION, VT_BSTR, (void*)&result);
	return result;
}

void CTabSelector::SetCaption(LPCTSTR propVal)
{
	SetProperty(DISPID_CAPTION, VT_BSTR, propVal);
}

CString CTabSelector::GetSelectedCategories()
{
	CString result;
	GetProperty(0x4, VT_BSTR, (void*)&result);
	return result;
}

void CTabSelector::SetSelectedCategories(LPCTSTR propVal)
{
	SetProperty(0x4, VT_BSTR, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// CTabSelector operations

CString CTabSelector::AccessFromName(LPCTSTR name)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		name);
	return result;
}

void CTabSelector::Update()
{
	InvokeHelper(0x6, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

CString CTabSelector::AccessFromCategory(LPCTSTR category)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x7, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		category);
	return result;
}

CString CTabSelector::AccessFromQuery(LPCTSTR query)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x8, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		query);
	return result;
}

void CTabSelector::Unselect()
{
	InvokeHelper(0x9, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CTabSelector::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}