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
// QueryResultsCtl.h : Declaration of the CQueryResultsCtrl OLE control class.
#include <afxcmn.h>
#include <afxtempl.h>
#include "ElementsListCtrl.h"

#define IDC_ELEMENTSLIST 1077

#define EVAL_OK               1
#define ERROR_DATA_SOURCE    -1
#define ERROR_ACCESS_SUPPORT -2
#define ERROR_ACCESS_COMMAND -3
#define ERROR_ACCESS_SERVER  -4

#define ASCENDING  0
#define DECENDING 1


//------------------------------------------------------------------------------------------
// This class describes the entry in the attribute table.
//------------------------------------------------------------------------------------------
class CAttributeInfo {
   public :  int SetSortDirection(int new_direction) {
				 int old = Direction;
				 Direction=new_direction;
				 return old;
			 }


			 BOOL GetSortDirection() {
				 return Direction;
			 }


			 CString& GetAccessOption(void) {
				 return AccessName;
			 }

			 void SetAccessOption(CString new_name) {
				 AccessName=new_name;
			 }

			 CString& GetReadableName(void) {
				 return ReadableName;
			 }

			 void SetReadableName(CString new_name) {
				 ReadableName=new_name;
			 }
   private : CString AccessName;
			 CString ReadableName;
			 int     Direction;


};
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// This class was designed to map all attributes of the list view instead of server re-query
//------------------------------------------------------------------------------------------
class CElementsMap {
   public : // Construct new elements map with the maximum size  = size;
	        CElementsMap(int size);
			// Removes everything from the element map.
			~CElementsMap();

	        // Search in the attribute list for the element with spesified row and
	        // column. Returns the attribute string pointer if successfull or NULL
	        // if no such element.
	        CString** SearchInMap(int row);

            // Adds the attribute value at the row, column position.
			void AddToMap(int row, CString** value, int nSize);

			// Removes all elements from the element map.
			void ClearMap(void);
   private:
	        struct Element {
			   Element() { row=0; value=NULL; usage=0; size=0;}
			   int      row;
			   int		size;
			   CString**  value;
			   unsigned usage;
			} *ElementsMap;

			int Size;
			unsigned EntryTime;

			int FILO(void);
};
//-------------------------------------------------------------------------------------------


	  

/////////////////////////////////////////////////////////////////////////////
// CQueryResultsCtrl : See QueryResultsCtl.cpp for implementation.

class CQueryResultsCtrl : public COleControl
{
	DECLARE_DYNCREATE(CQueryResultsCtrl)

// Constructor
public:
	CQueryResultsCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQueryResultsCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString** GetItemDescription(int nIdx, bool bApplyFormat = true);
	bool HasAttributes();
	int GetAttributesCount();
	void FillSelectionVariable();
	LRESULT OnListKeyDown(WPARAM vKey, LPARAM flags);
	void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	void FillElementInfo(NMHDR* pNMHDR, LRESULT* pResult);
	void FillElementsList();
	int EvaluateSync(CString& command, CString* results=NULL);
	LPDISPATCH m_DataSource;
	CString m_Sort;
	CString m_Filter;
	CString m_Query;
	~CQueryResultsCtrl();

	DECLARE_OLECREATE_EX(CQueryResultsCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CQueryResultsCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CQueryResultsCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CQueryResultsCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CQueryResultsCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnUserSelection(WPARAM,LPARAM);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CQueryResultsCtrl)
	CString m_FullListName;
	afx_msg void OnFullListNameChanged();
	CString m_FilteredListName;
	afx_msg void OnFilteredListNameChanged();
	afx_msg LPDISPATCH GetDataSource();
	afx_msg void SetDataSource(LPDISPATCH newValue);
	afx_msg BSTR GetQuery();
	afx_msg void SetQuery(LPCTSTR lpszNewValue);
	afx_msg BSTR GetFilter();
	afx_msg void SetFilter(LPCTSTR lpszNewValue);
	afx_msg BSTR GetSort();
	afx_msg void SetSort(LPCTSTR lpszNewValue);
	afx_msg long GetElement();
	afx_msg void SetElement(long nNewValue);
	afx_msg BSTR GetSelectionName();
	afx_msg void SetSelectionName(LPCTSTR lpszNewValue);
	afx_msg BOOL GetShortNames();
	afx_msg void SetShortNames(BOOL bNewValue);
	afx_msg void Push();
	afx_msg BOOL Pop();
	afx_msg void Update();
	afx_msg long AmountSelected();
	afx_msg void Clear();
	afx_msg long RowAmount();
	afx_msg long FilteredAmount();
	afx_msg void ApplyAttributeChanges();
	afx_msg void CancelAttributeChanges();
	afx_msg void AddAttribute(LPCTSTR logname, LPCTSTR option);
	afx_msg void RemoveAttribute(LPCTSTR logname, LPCTSTR option);
	afx_msg short CheckAttribute(LPCTSTR logname, LPCTSTR option);
	afx_msg void Calculate();
	afx_msg BOOL SaveSelectionTo(LPCTSTR szFileName, short cDelimiter);
	afx_msg BSTR GetPrintformat();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CQueryResultsCtrl)
	void FireElementsSorted(short column)
		{FireEvent(eventidElementsSorted,EVENT_PARAM(VTS_I2), column);}
	void FireRButtonClicked(short x, short y)
		{FireEvent(eventidRButtonClicked,EVENT_PARAM(VTS_I2  VTS_I2), x, y);}
	void FireElementDoubleclicked(LPCTSTR parmstring)
		{FireEvent(eventidElementDoubleclicked,EVENT_PARAM(VTS_BSTR), parmstring);}
	void FireSelectionChanged(LPCTSTR curinfo)
		{FireEvent(eventidSelectionChanged,EVENT_PARAM(VTS_BSTR), curinfo);}
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CQueryResultsCtrl)
	dispidFullListName = 1L,
	dispidFilterListName = 2L,
	dispidDataSource = 3L,
	dispidQuery = 4L,
	dispidFilter = 5L,
	dispidSort = 6L,
	dispidElement = 7L,
	dispidSelectionName = 8L,
	dispidShortNames = 9L,
	dispidPush = 10L,
	dispidPop = 11L,
	dispidUpdate = 12L,
	dispidAmountSelected = 13L,
	dispidClear = 14L,
	dispidRowAmount = 15L,
	dispidFilteredAmount = 16L,
	dispidApplyAttributeChanges = 17L,
	dispidCancelAttributeChanges = 18L,
	dispidAddAttribute = 19L,
	dispidRemoveAttribute = 20L,
	dispidCheckAttribute = 21L,
	dispidCalculate = 22L,
	dispidSaveSelectionTo = 23L,
	dispidGetPrintformat = 24L,
	eventidElementsSorted = 1L,
	eventidRButtonClicked = 2L,
	eventidElementDoubleclicked = 3L,
	eventidSelectionChanged = 4L,
	//}}AFX_DISP_ID
	};
private:
	BOOL m_ShortNames;
	int m_NameSortOrder;
	int m_RowElementsAmount;
	BOOL m_InSelection;
	BOOL m_SortDurty;
	BOOL m_FilterDurty;
	BOOL m_QueryDurty;
	CString m_SelectionName;
	int m_CurrentRow;
	CString m_IncSearchString;
	int m_CurrentColumn;
	int EvaluateOutput(CString& command, CString* results);
	CString& FormPrintformat(void);
	CString m_OldFull;
	CString m_OldFilter;
	void RunSort(void);
	void RunFilter(void);
	void RunQuery(void);
	void InsertElementRow(int num);
	void CreateListHeader(void);
	// Query the server for the attributes and names. Forms thr m_AttributeList table.
	void FormAttributeTable();
	CList<CAttributeInfo,CAttributeInfo&> m_AttributeList;
	CList<CAttributeInfo,CAttributeInfo&> m_ModifiedAttributes;
	CList<int,int&>                       m_SelectionIndexes;
	CImageList                            m_ImageList;
	CElementsListCtrl                     m_ElementsList;
	CElementsMap*                         m_AttributeMap;
};
