// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CGroupsCombo wrapper class

class CGroupsCombo : public CWnd
{
protected:
	DECLARE_DYNCREATE(CGroupsCombo)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x1487dd54, 0x1cb6, 0x11d2, { 0xae, 0xf0, 0x0, 0xa0, 0xc9, 0xb7, 0x1d, 0xc4 } };
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
	CString GetSelectionVariable();
	void SetSelectionVariable(LPCTSTR);
	LPDISPATCH GetDataSource();
	void SetDataSource(LPDISPATCH);
	CString GetOutputVariable();
	void SetOutputVariable(LPCTSTR);
	CString GetCaption();
	void SetCaption(LPCTSTR);
	BOOL GetControlsEnable();
	void SetControlsEnable(BOOL);

// Operations
public:
	void Update();
	CString GetSelectedGroups();
	void CreateGroup();
	void AddElements();
	void AboutBox();
};