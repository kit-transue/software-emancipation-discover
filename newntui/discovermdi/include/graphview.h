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
#if !defined(AFX_GRAPHVIEW_H__3444F0AF_A002_11D2_AF78_00A0C9B71DC4__INCLUDED_)
#define AFX_GRAPHVIEW_H__3444F0AF_A002_11D2_AF78_00A0C9B71DC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxtempl.h>
#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif

#define UNSELECTED 0
#define SELECTED 1
#define FOCUSED 2

#define NONE  -1
#define LEFT   0
#define RIGHT  1
#define TOP    2
#define BOTTOM 3

#define CALLTREE    0
#define INHERITANCE 1
#define ERD         2

//-------------------------------------------------------------------------------------
// This class was designed to save the graph setting
//-------------------------------------------------------------------------------------
class CGraphView;
class CGraphSettings : public CObject {
public: 
    DECLARE_SERIAL(CGraphSettings)
    enum GraphOrientation { LeftToRight, RightToLeft, TopToBottom, BottomToTop }; 
    enum GraphJustification { Start, Center, End }; 
    enum BorderThickness { Thin, Medium, Thick }; 
	enum ExchangeMethod {Get,Set};
	enum NameType {Short,Long};
	enum SortType {ByName,ByAccess};
	void DataExchange(CGraphView* target,  ExchangeMethod method);
public:
	void SetViewType(int type) {
		m_ViewType=type;
	}
	int  GetViewType(void) {
		return m_ViewType;
	}
	void Default(void);
	GraphOrientation m_Orientation;
	GraphJustification m_Justification;
	struct {
		BOOL m_Orthogonal;
		BOOL m_Merge;
	} m_Routing;

	struct {
		BOOL m_EnableVatiable;
		int  m_NodeSpacing;
		int  m_LevelSpacing;
		int  m_RowSpacing;
		int  m_ColumnSpacing;
	} m_Spacing;

	struct {
		unsigned char m_Red;
		unsigned char m_Green;
		unsigned char m_Blue;
	} m_NodeBackground[3];

	struct {
		unsigned char m_Red;
		unsigned char m_Green;
		unsigned char m_Blue;
	} m_NodeBorder[3];

	struct {
		unsigned char m_Red;
		unsigned char m_Green;
		unsigned char m_Blue;
	} m_EdgeColor[3];

	LOGFONT m_NodeFont;
	BorderThickness m_BorderThickness;
	BorderThickness m_EdgeThickness;

    struct {
		BOOL m_ShowClassMethods;
		BOOL m_ShowClassProperties;

		BOOL m_ShowPublicMembers;
		BOOL m_ShowProtectedMembers;
		BOOL m_ShowPackageAccessMembers;
		BOOL m_ShowPrivateMembers;

		NameType m_Name;
		SortType m_Sort;

		BOOL m_Merge;
	} m_Inheritance;

	struct {
		BOOL            m_Visible;
		int             m_Style;
		BorderThickness m_Thickness;
		struct {
			unsigned char m_Red;
			unsigned char m_Green;
			unsigned char m_Blue;
		} m_Color;
	} m_Relations[12];


	int m_ViewType;

    virtual void Serialize(CArchive& ar);
private:
	void GetGraphProperties(CGraphView* graphView);
	void GetNodeProperties(CGraphView* graphView);
	void GetEdgeProperties(CGraphView* graphView);
	void GetInheritanceProperties(CGraphView* graphView);
	void GetRelationsProperties(CGraphView* graphView);
	void SetGraphProperties(CGraphView* graphView);
	void SetNodeProperties(CGraphView* graphView);
	void SetEdgeProperties(CGraphView* graphView);
	void SetInheritanceProperties(CGraphView* graphView);
	void SetRelationsProperties(CGraphView* graphView);
};
//-------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
// Thismodule contains all classes we are going to use for the graphical representation of 
// the client graph model.This classes are used when drawing the nodes in the graph window.
// In reality, complete graphical subsysten on the client side is based on the DOC/View 
// model. The DOC part of the model present system-independent graph. This graph contains
// real objects and is able to calculate layout. Graphical view is responsible forgraph 
// screen representation and is exporting a set of callbacks defined in GCallback 
// interface to the DOC part of the graphical subsystem as well as node and edges styles.
//-----------------------------------------------------------------------------------------

// This macros define the possible node shapes
#define RECTANGLE_BLOCK     0
#define ROUNDRECT_BLOCK     1
#define VERTEX_BLOCK        2
#define PARALELLOGRAM_BLOCK 3
#define LOOP_BLOCK          4
#define IF_BLOCK            5
#define CIRCLE_BLOCK        6

// This macros defines the side of the node to which the edge connected
#define LEFT_CONNECT   0
#define RIGHT_CONNECT  1
#define TOP_CONNECT    2
#define BOTTOM_CONNECT 3

// This define the edge direction
#define HORIZONTAL_EDGE 0
#define VERTICAL_EDGE   1


////////////////////////////////////////////////////////////////////////////////////////////
// This class describes the node field. It contain the field name and the field type, which
// is also an image index in the fields image list available in the CNodeProperties class
////////////////////////////////////////////////////////////////////////////////////////////
class CNodeItem {
public:
      int     m_FieldID;
	  CString m_FieldName;
	  int     m_FieldType;
	  CNodeItem& operator = (CNodeItem& other) {
         m_FieldID   = other.m_FieldID;
	     m_FieldName = other.m_FieldName;
	     m_FieldType = other.m_FieldType;
		 return *this;
	  }

};
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
// We need this class do deal with a list of CNodeItems.
////////////////////////////////////////////////////////////////////////////////////////////
class CItemsList {
public :
	   CNodeItem GetHead(void) { return m_Items.GetHead(); }
	   CNodeItem GetTail(void) { return m_Items.GetTail(); }
	   CNodeItem RemoveHead(void) { return m_Items.RemoveHead(); }
	   CNodeItem RemoveTail(void) { return m_Items.RemoveTail(); }
	   POSITION  AddHead(CNodeItem& item) { return m_Items.AddHead(item); }
	   POSITION  AddTail(CNodeItem& item) { return m_Items.AddTail(item); }
	   void RemoveAll(void) { m_Items.RemoveAll(); }
	   POSITION GetHeadPosition(void) { return m_Items.GetHeadPosition(); }
	   POSITION GetTailPosition(void) { return m_Items.GetTailPosition(); }
	   CNodeItem GetNext(POSITION& pos) { return m_Items.GetNext(pos); }
	   CNodeItem GetPrev(POSITION& pos) { return m_Items.GetPrev(pos); }
	   CNodeItem GetAt(POSITION pos) { return m_Items.GetAt(pos); }
	   void SetAt(POSITION pos,CNodeItem& item) { m_Items.SetAt(pos,item); }
	   void RemoveAt(POSITION pos) { m_Items.RemoveAt(pos); }
       POSITION InsertBefore(POSITION pos,CNodeItem& item) { return m_Items.InsertBefore(pos,item); }
       POSITION InsertAfter(POSITION pos,CNodeItem& item) { return m_Items.InsertAfter(pos,item); }
	   int GetCount(void) { return m_Items.GetCount(); }
	   BOOL IsEmpty(void) { return m_Items.IsEmpty(); }
private:
	   CList<CNodeItem,CNodeItem&> m_Items;
};
//////////////////////////////////////////////////////////////////////////////////////////////

class CBitmapList {
public :
	   ~CBitmapList();
	   CBitmap* GetHead(void) { return m_Items.GetHead(); }
	   CBitmap* GetTail(void) { return m_Items.GetTail(); }
	   CBitmap* RemoveHead(void) { return m_Items.RemoveHead(); }
	   CBitmap* RemoveTail(void) { return m_Items.RemoveTail(); }
	   POSITION  AddHead(CBitmap* item) { return m_Items.AddHead(item); }
	   POSITION  AddTail(CBitmap* item) { return m_Items.AddTail(item); }
	   void RemoveAll(void) { m_Items.RemoveAll(); }
	   POSITION GetHeadPosition(void) { return m_Items.GetHeadPosition(); }
	   POSITION GetTailPosition(void) { return m_Items.GetTailPosition(); }
	   CBitmap* GetNext(POSITION& pos) { return m_Items.GetNext(pos); }
	   CBitmap* GetPrev(POSITION& pos) { return m_Items.GetPrev(pos); }
	   CBitmap* GetAt(POSITION pos) { return m_Items.GetAt(pos); }
	   void SetAt(POSITION pos,CBitmap* item) { m_Items.SetAt(pos,item); }
	   void RemoveAt(POSITION pos) { m_Items.RemoveAt(pos); }
       POSITION InsertBefore(POSITION pos,CBitmap* item) { return m_Items.InsertBefore(pos,item); }
       POSITION InsertAfter(POSITION pos,CBitmap* item) { return m_Items.InsertAfter(pos,item); }
	   int GetCount(void) { return m_Items.GetCount(); }
	   BOOL IsEmpty(void) { return m_Items.IsEmpty(); }
	   CBitmap* GetBitmap(int index);
private:
	   CList<CBitmap*,CBitmap*> m_Items;
};



////////////////////////////////////////////////////////////////////////////////////////////
// This class describes the node graphical properties.
////////////////////////////////////////////////////////////////////////////////////////////
class CNodeProperties {
public  : CNodeProperties();
		  ~CNodeProperties();

		  // This method will return the node shape code - see *_BLOCK macros for the
		  // possible node shapes.
		  int GetNodeShape(void);
		  // This method will set the node shape code - see *_BLOCK macros for the
		  // possible node shapes.
		  void SetNodeShape(int shape);

		  // This method will set a pointer to the node name font. If autoDelete flag
		  // is set to TRUE, the font class will be automatically deleted in this class
		  // destructor. If the font pointer was already assigned, the old font class
		  // will be disconnected (if autoDelete was set to FALSE) or disconnected and
		  // removed (if autoDelete was set to TRUE).
		  // Name font class defines the font which will be used to draw the node caption.
		  void SetNameFont(CFont* font, BOOL autoDelete=TRUE);

		  // This method  will return the current name font pointer. It will return NULL
		  // if no font was assigned to the name.
		  CFont* GetNameFont(void);


		  // This method will set a pointer to the node fields font. If autoDelete flag
		  // is set to TRUE, the font class will be automatically deleted in this class
		  // destructor. If the font pointer was already assigned, the old font class
		  // will be disconnected (if autoDelete was set to FALSE) or disconnected and
		  // removed (if autoDelete was set to TRUE).
		  // The fields font class defines the font which will be used to display a list
		  // of fields attached to the node.
		  void SetFieldsFont(CFont* font, BOOL autoDelete=TRUE);

		  // This method  will return the current fields font pointer. It will return NULL
		  // if no font was assigned to the name.
		  CFont* GetFieldsFont(void);

          // This method returns the name border thikness for the name box.
          int GetNameBorderSize(void);

          // This method sets the name border thikness for the name box.
          void SetNameBorderSize(int size);

          // This method returns the fields border thikness for the fields list.
          int GetFieldsBorderSize(void);

          // This method sets the fields border thikness for the fields list.
          void SetFieldsBorderSize(int size);

          // This method will return a  COLORREF structure which describes 
		  // the  name box border color.
          COLORREF GetNameBorderColor(void);
		  // This method will fill the internal COLOREF structure which describe name box
		  // border color.
		  void SetNameBorderColor(COLORREF color);

          // This method will return the COLORREF structure which describes 
		  // the  name box caption color.
          COLORREF GetNameTextColor(void);
		  // This method will fill the internal COLOREF structure which describes name box
		  // caption color.
		  void SetNameTextColor(COLORREF color);


          // This method will return the COLORREF structure which describes
		  // the  name box background color.
          COLORREF GetNameBackgroundColor(void);
		  // This method will fill the internal COLOREF structure which describes name box
		  // background color.
		  void SetNameBackgroundColor(COLORREF color);


          // This method will return the COLORREF structure which describes 
		  // the  name box border color.
          COLORREF GetFieldsBorderColor(void);
		  // This method will fill the internal COLOREF structure which describes fields 
		  // list border color.
		  void SetFieldsBorderColor(COLORREF color);

          // This method will return the COLORREF structure which describes 
		  // the name box caption color.
          COLORREF GetFieldsTextColor(void);
		  // This method will fill the internal COLOREF structure which describes fields 
		  // list text color.
		  void SetFieldsTextColor(COLORREF color);

          // This method will return the COLORREF structure which describes 
		  // the name box background color.
          COLORREF GetFieldsBackgroundColor(void);
		  // This method will fill the internal COLOREF structure which describes fields 
		  // list background color.
		  void SetFieldsBackgroundColor(COLORREF color);

		  // This methid returns the space size in HIMETRIC between node name box and node
		  // fields list.
		  int GetNameFieldsSpace(void);

		  // This method sets the space between node name box and node fields list.
		  void SetNameFieldsSpace(int space);

		  // This function assignes the image to the node name..
		  // If aoutoDelete flag is set, the image list will be removed when this 
		  // class removed.
		  void SetNameImageList(CBitmapList* list, CSize& size, BOOL autoDelete=TRUE);
		  // This function returns the pointer to the node name image or NULL
		  // if no image list assigned.
		  CBitmapList* GetNameImageList(void);
		  // Will return the desired image size for the header image in HIMETRIC
		  CSize GetNameImageSize(void);


          // *** REMARK
		  //     Every field in the fields list contains the field type, which is
		  //     at the same time the index of the field image in the fields image
		  //     list.

		  // This function will set the image list which is used to mark fields.
		  // If aoutoDelete flag is set, the image list will be removed when this 
		  // class removed.
		  void SetFieldsImageList(CBitmapList* list,CSize& size, BOOL autoDelete=TRUE);
		  // This function will return the pointer to the fields image list or NULL
		  // if no image list assigned.
		  CBitmapList* GetFieldsImageList(void);
		  // Will return the desired image size for the fields images in HIMETRIC
		  CSize GetFieldsImageSize(void);
private : int          m_NodeShape;

	      CFont*       m_NameFont;
		  BOOL         m_NameFontAutodelete;
	      int          m_NameBorderSize;
		  COLORREF     m_NameBorderColor;
		  COLORREF     m_NameTextColor;
		  COLORREF     m_NameBackgroundColor;
		  CSize        m_NameImageSize;

		  int          m_NameToFieldsSpace;

		  CFont*       m_FieldsFont;
		  BOOL         m_FieldsFontAutodelete;
		  int          m_FieldsBorderSize;
		  COLORREF     m_FieldsBorderColor; 
		  COLORREF     m_FieldsTextColor; 
		  COLORREF     m_FieldsBackgroundColor; 
		  CSize        m_FieldsImageSize;

		  CBitmapList* m_NodeBitmapList;
		  BOOL         m_NodeBitmapAutodelete;
		  CBitmapList* m_FieldsImageList;
		  BOOL         m_FieldsImageListAutodelete;
};
////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
// This class describes the edge graphical properties.
////////////////////////////////////////////////////////////////////////////////////////////
class CEdgeProperties {
public: CEdgeProperties();
		~CEdgeProperties();

		// This method sets the line style used ti show edge (solid, dotted etc)
		void SetEdgeStyle(int style);
		// This method returns the edge line style
		int GetEdgeStyle(void);

		// This method sets the thikness of the edge line (HIMETRIC coordinate system)
		void     SetEdgeThikness(int thikness);

		// This method returns the thikness of the edge line (HIMETRIC coordinate system)
		int      GetEdgeThikness(void);

		// This method sets the edge drawing color
		void SetEdgeColor(COLORREF color);

		// This method returns the edge drawing color
		COLORREF GetEdgeColor(void);

		// This method sets the imagelist containing 4 images to draw the edge start point.
		// We need this four images because the edge can go from left to right, from right
		// to left, from top to bottom and from bottom to top. We will not rotate the 
		// original bitmap but we will set an index in the image list.
		// If the autoDelete flag is set this imagelist will be destroyed in this class 
		// destructor.
		void        SetEdgeSourceImages(CBitmapList* image,CSize& imageSize,BOOL autoDelete=TRUE);
		// Returns the image list for the edge start point image.
		CBitmapList* GetEdgeSourceImages(void);
		// Returns the size of the source image in HIMETRIC
		CSize GetEdgeSourceSize(void);

		// This method sets the imagelist containing 4 images to draw the edge end point.
		// We need this four images because the edge can go from left to right, from right 
		// to left, from top to bottom and from bottom to top. We will not rotate the 
		// original bitmap but we will set an index in the image list.
		// If the autoDelete flag is set this imagelist will be destroyed in this class 
		// destructor.
		void        SetEdgeTargertImages(CBitmapList* image, CSize& imageSize, BOOL autoDelete=TRUE);
		// Returns the image list for the edge start point image.
		CBitmapList* GetEdgeTargetImages(void);
		// Returns the size of the target image in HIMETRIC
		CSize GetEdgeTargetSize(void);


		// This method sets the imagelist containing 2 images to draw the edge marker.
		// We need this 2 images because the edge marker point can be set on the horizontal 
		// or on the vertical edge part.
		// If the autoDelete flag is set this imagelist will be destroyed in this class 
		// destructor.
		void        SetEdgeMarkerImages(CBitmapList* image, CSize& imageSize, BOOL autoDelete=TRUE);
		// Returns the image list for the edge marker image.
		CBitmapList* GetEdgeMarkerImages(void);
		// Returns the size of the marker image in HIMETRIC
		CSize GetEdgeMarkerSize(void);
private:
	    int          m_LineStyle;
		int          m_LineThikness;
		COLORREF     m_LineColor;
		CBitmapList* m_SourceImages;
		BOOL         m_SourceImagesAutodelete;
		CBitmapList* m_TargetImages;
		BOOL         m_TargetImagesAutodelete;
		CBitmapList* m_MarkerImages;
		BOOL         m_MarkerImagesAutodelete;
		CSize        m_SourceImageSize;
		CSize        m_TargetImageSize;
		CSize        m_MarkerImageSize;

};
////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////
// CGraphView class exports GCALLBACK interface to the Client Graph. It supports all
// view-dependent drawing functions and styles, it also calculates the node sizes for the
// layout process in the client graph.
////////////////////////////////////////////////////////////////////////////////////////////
class CDisGraph;
class CGraphView : public CScrollView
{
protected:
	CGraphView();                    // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CGraphView)

// Attributes
public:

// Operations
public:
	CEdgeProperties* GetRelationsProp(int id);
	void SetSettingsFile(CString filename, int viewType);
	void AssignBitmaps(int* =NULL, int =-1, int* = NULL, int = -1);
	CNodeProperties* GetUnselectedNodeProp(void);
	CNodeProperties* GetSelectedNodeProp(void);
	CNodeProperties* GetFocusedNodeProp(void);
	CEdgeProperties* GetUnselectedEdgeProp(void);
	CEdgeProperties* GetSelectedEdgeProp(void);
	CEdgeProperties* GetFocusedEdgeProp(void);

	void DrawNodePort(CRect& nodeRect, int headerHeight, int side, BOOL status);
	void GraphLayout(void);
	CDisGraph* GetGraph(void);
	//========================= GCallback interface ==================================
	// GCALLBACK interface. This view function must draw the node using it's size and
	// position calculated in the Client Graph during layout seccion. It will use the
	// node properties class assigned to the node which determines node shape, node 
	// colors and node spacing. It will also use the node name and node fields list 
	// obtained from the server, This function must be assigned to the client graph 
	// and will be called every time the graph needs to draw the node.
	// "nodeRect" describes the position of the node in HIMETRIC coordinates.
	// "nodeName" contains the name of the node
	// "fieldsList" points to the class which contains the set of node fields 
	// (can be NULL if empty).
	// "nodeProperties" points to the class which contain the node drawing attributes,
	// such as node color, node shape and node spacing.
	void DrawNode(CRect&           nodeRect, 
		          CString&         nodeName, 
				  int              nodeKind,
				  CItemsList*      fieldsList,
				  CNodeProperties* nodeProperties,
				  int              connectorSize,
				  BOOL             expanded,
				  int              parentsConnectorSize,
				  BOOL             parentsExpanded);


	// GCALLBACK interface. This view function draws the given edge using it's path 
	// calculated during layout process in the client graph. It will use the edge style 
	// class to determine  the edge color, edge line style, edge source/target images and 
	// edge bitmap. This function must been assigned and is called by the Client Graph 
	// every time it needs to draw the edge.
	// "vertexes" is a pointer to the array of edge vertexes, it forms edge drawing path.
	// "vertexAmount" contains the amount of vertexes in the vertex array.
	// "edgeProperties" describes the edge color, linestyle, thikness and images.
	void DrawEdge(CPoint* vertexes, 
		          int vertexAmount, 
				  CEdgeProperties* edgeProperties);

	// GCALLBACK interface. This function calculates the width and length of the node 
	// in HIMETRIC coordinate system. To do this we need the node name ("nodeName")
	// the list of the fields assigned to the node ("nodeItems") and node graphical 
	// properties.
	CSize CalculateNodeSize(CString&    nodeName, 
		                    CItemsList* nodeItems, 
							CNodeProperties* nodeProp,
							int& headerHeight);

	// GCALLBACK interface. This function will return the node header port (connection
	// point) at the desired node side. If the port is unavailable it will return FALSE;
	// The resulted port will be given in the HIMETRIC coordinate system relative to the
	// left-bottom corner of the node rectangle.
	CPoint GetHeaderPort(CString& nodeName, 
		                 CItemsList* list, 
						 CNodeProperties* nodeProp,
						 int side);

	// GCALLBACK interface. This function will return the node field port (left or right,
	// depending from the side value).You must specify the field index for which you need
	// to get the port. If there is no fields or no field with this index the function
	// will return the header port.
	CPoint GetFieldPort(CString& nodeName, 
		                CItemsList* list, 
						CNodeProperties* nodeProp,
						int index,
						int side);

//===================================================================================
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphView)
	public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CGraphView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CGraphView)
	afx_msg void OnPrintPreview();
	afx_msg void OnFilePrint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnViewProperties();
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int FindMarkerImageIndex(CPoint* vertexes, int vertexAmount, CPoint* markerImagePos, int* markerVertexNumber);
	int FindTargetImageIndex(CPoint* vertexes, int vertexAmount);
	int FindSourceImageIndex(CPoint* vertexes, int vertexAmount);
private:
	int m_ViewType;
	CString m_SettingsFile;
	CGraphSettings* m_Settings;
	int m_NameMapsAmount;
	int* m_NameMapID;
	int m_FieldsMapsAmount;
	int* m_FieldsMapID;
	CBitmapList      m_PortImages;
	CEdgeProperties* m_EdgeFocused;
	CEdgeProperties* m_EdgeSelected;
	CEdgeProperties* m_EdgeUnselected;
	CNodeProperties* m_FocusedNode;
	CNodeProperties* m_SelectedNode;
	CNodeProperties* m_UnselectedNode;

    // Relations properties
	CEdgeProperties *m_Relations[12]; // how to display different relations


	CDC* InitDC(CDC* pDC,BOOL initOrigin=TRUE);
	CDataSource m_DataSource;
	CDisGraph* m_ClientGraph;
	void DrawPrintFrame(CDC* pDC,CPrintInfo* pInfo);
	CSize m_GraphSize;
	int CalculateNeededColumns(CSize& pageSize);
	int CalculateNeededRows(CSize& pageSize);
	CRect GetPageClientSpace(CDC* pDC,BOOL logical=TRUE);
	void DrawRectangleHeader(CRect& rect,CString& nodeName, int nodeKind, CNodeProperties* nodeProperties);
	void DrawRoundrectHeader(CRect& rect,CString& nodeName, int nodeKind, CNodeProperties* nodeProperties);
	void DrawVertexHeader(CRect& rect,CString& nodeName, int nodeKind, CNodeProperties* nodeProperties);
	void DrawParalellogramHeader(CRect& rect, CString& nodeName, int nodeKind, CNodeProperties* nodeProperties);
	void DrawLoopHeader(CRect& rect, CString& nodeName, int nodeKind, CNodeProperties* nodeProperties);
	void DrawIfHeader(CRect& rect, CString& nodeName, int nodeKind, CNodeProperties* nodeProperties);
	void DrawCircleHeader(CRect& rect, CString& nodeName, int nodeKind, CNodeProperties* nodeProperties);
    void DrawNodeItemsList(CRect & rect, CItemsList* list, CNodeProperties * nodeProperties);
    int  GetHeaderRectHeight(CString& nodeName, CNodeProperties* prop);
	int  GetFieldsRectHeight(CString& nodeName, CNodeProperties* prop, int items);
	int  GetHeaderRectWidth(CNodeProperties* prop, CString& nodeText);
	int  GetFieldsRectWidth(CNodeProperties* prop, CItemsList& items);
	CDC* m_ClientDC;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPHVIEW_H__3444F0AF_A002_11D2_AF78_00A0C9B71DC4__INCLUDED_)
