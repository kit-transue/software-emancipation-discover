// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CProjectModules wrapper class

class CProjectModules : public CWnd
{
protected:
	DECLARE_DYNCREATE(CProjectModules)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x5743ffdd, 0xa20, 0x11d2, { 0xae, 0xe7, 0x0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); }

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID,
		CFile* pPersist = NULL, BOOL bStorage = FALSE,
		BSTR bstrLicKey = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); }

// Attributes
public:
	LPDISPATCH GetDataSource();
	void SetDataSource(LPDISPATCH);
	CString GetProject();
	void SetProject(LPCTSTR);

// Operations
public:
	void AboutBox();
};
