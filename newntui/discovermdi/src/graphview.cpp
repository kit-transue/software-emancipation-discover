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
// GraphView.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "GraphView.h"
#include "disgraphnode.h"
#include "GraphPropertiesSheet.h"
#include "GraphSettingsParser.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define XMARGIN 500
#define YMARGIN 500
#define X_BORDER 300
#define Y_BORDER 300

IMPLEMENT_SERIAL(CGraphSettings,CObject,10 )


static CGraphView *viewInstance[1000];


void DrawNodeCallback (int clientID,
					   CRect& nodeRect,
					   CString& nodeName,
					   int nodeKind,
					   CItemsList* fieldsList,
					   CNodeProperties* nodeProperties,
					   int connectorSize,
					   BOOL expanded,
					   int parentsConnectorSide,
					   BOOL parentsExpanded) {
    viewInstance[clientID]->DrawNode(nodeRect,nodeName,nodeKind,fieldsList,nodeProperties,connectorSize,expanded,parentsConnectorSide,parentsExpanded);
}


CSize SizeNodeCallback(int clientID,
					   CString&    nodeName, 
		               CItemsList* nodeItems, 
					   CNodeProperties* nodeProp,
					   int& headerHeight) {
    CSize size;
	size = viewInstance[clientID]->CalculateNodeSize(nodeName,nodeItems,nodeProp,headerHeight);
	return size;
}


void DrawEdgeCallback(int clientID,
					  CPoint* vertexes, 
					  int vertexAmount, 
					  CEdgeProperties* edgeProperties) {
	viewInstance[clientID]->DrawEdge(vertexes,vertexAmount,edgeProperties);
}


CPoint HeaderPortCallback(int clientID,
						  CString&    nodeName, 
		                  CItemsList* nodeItems, 
					      CNodeProperties* nodeProp,
						  int side) {
    CPoint connect;
	connect = viewInstance[clientID]->GetHeaderPort(nodeName,nodeItems,nodeProp,side);
	return connect;
}

CPoint FieldsPortCallback(    int clientID,
						  CString&    nodeName, 
		                  CItemsList* nodeItems, 
					      CNodeProperties* nodeProp,
						  int index,
						  int side) {
    CPoint connect;
	connect = viewInstance[clientID]->GetFieldPort(nodeName,nodeItems,nodeProp,index,side);
	return connect;
}




CBitmapList::~CBitmapList() {
POSITION p = GetHeadPosition();

     while(p) {
		 CBitmap* at;
		 at=GetAt(p);
		 delete at;
		 GetNext(p);
	 }
}

CBitmap* CBitmapList::GetBitmap(int index) {
POSITION p = GetHeadPosition();


    if(index>=GetCount()) return NULL;
    for(register i=0;i<index;i++) GetNext(p);
	return GetAt(p);
}

	




////////////////////////////////////////////////////////////////////////////////////////////
// This class describes the node graphical properties.
////////////////////////////////////////////////////////////////////////////////////////////



//----------------------------------------------------------------------------------------------
// Constructor will set default properties to the newly created node.
//----------------------------------------------------------------------------------------------
CNodeProperties::CNodeProperties() {

   //  Setting default values at the beginning
   m_NodeShape                 = RECTANGLE_BLOCK; // rectangular node
   m_NameFont                  = NULL;            // no font assigned to the node name 
   m_NameFontAutodelete        = FALSE;           // and will not be automatically removed
   m_NameBorderSize            = 50;              // 1/2 mm name border thikness
   m_NameBorderColor           = 0;               // black name border color
   m_NameTextColor             = 0;               // black name text color
   m_NameBackgroundColor       = 0x00ffffff;      // white name background

   m_NameToFieldsSpace         = 100;              // 1/4 mm fields border thikness

   m_FieldsFont                = NULL;            // no font assigned to the node fields
   m_FieldsFontAutodelete      = FALSE;           // and will not be automatically removed
   m_FieldsBorderSize          = 25;             // 1 mm space between the name box and the field box

   m_FieldsBorderColor         = 0;               // black fields border 
   m_FieldsTextColor           = 0;               // black fields text
   m_FieldsBackgroundColor     = 0x00ffffff;      // white fields background;

   m_NodeBitmapList            = NULL;            // no bitmap assigned to the node
   m_NodeBitmapAutodelete      = FALSE;

   m_FieldsImageList           = NULL;             // no image list assigned to the fields
   m_FieldsImageListAutodelete = FALSE;
   m_FieldsImageSize=0;

}
//----------------------------------------------------------------------------------------------




//----------------------------------------------------------------------------------------------
// Destructor will remove all objects for which autodelete flag is set.
//----------------------------------------------------------------------------------------------
CNodeProperties::~CNodeProperties() {
	if(m_NameFont!=NULL && m_NameFontAutodelete==TRUE) {
		delete m_NameFont;
		m_NameFont=NULL;
		m_NameFontAutodelete=FALSE;
	}
	if(m_FieldsFont!=NULL && m_FieldsFontAutodelete==TRUE) {
		delete m_FieldsFont;
		m_FieldsFont=NULL;
		m_FieldsFontAutodelete=FALSE;
	}
	if(m_NodeBitmapList!=NULL && m_NodeBitmapAutodelete==TRUE) {
		delete m_NodeBitmapList;
		m_NodeBitmapList=NULL;
		m_NodeBitmapAutodelete=FALSE;
	}
	if(m_FieldsImageList!=NULL && m_FieldsImageListAutodelete==TRUE) {
		delete m_FieldsImageList;
		m_FieldsImageList=NULL;
		m_FieldsImageListAutodelete=FALSE;
	}

}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method will return the node shape code - see *_BLOCK macros for the
// possible node shapes.
//----------------------------------------------------------------------------------------------
int CNodeProperties::GetNodeShape(void) {
	return m_NodeShape;
}
//----------------------------------------------------------------------------------------------
		  

//----------------------------------------------------------------------------------------------
// This method will set the node shape code - see *_BLOCK macros for the
// possible node shapes.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetNodeShape(int shape) {
	m_NodeShape=shape;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method will set a pointer to the node name font. If autoDelete flag
// is set to TRUE, the font class will be automatically deleted in this class
// destructor. If the font pointer was already assigned, the old font class
// will be disconnected (if autoDelete was set to FALSE) or disconnected and
// removed (if autoDelete was set to TRUE).
// Name font class defines the font which will be used to draw the node caption.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetNameFont(CFont* font, BOOL autoDelete) {
	if(m_NameFont!=NULL && m_NameFontAutodelete==TRUE) delete m_NameFont;
	m_NameFont           = font;
	if(font!=NULL)
	    m_NameFontAutodelete = autoDelete;
	else
	    m_NameFontAutodelete = FALSE;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method  will return the current name font pointer. It will return NULL
// if no font was assigned to the name.
//----------------------------------------------------------------------------------------------
CFont* CNodeProperties::GetNameFont(void) {
	return m_NameFont;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method will set a pointer to the node fields font. If autoDelete flag
// is set to TRUE, the font class will be automatically deleted in this class
// destructor. If the font pointer was already assigned, the old font class
// will be disconnected (if autoDelete was set to FALSE) or disconnected and
// removed (if autoDelete was set to TRUE).
// The fields font class defines the font which will be used to display a list
// of fields attached to the node.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetFieldsFont(CFont* font, BOOL autoDelete) {
	if(m_FieldsFont!=NULL && m_FieldsFontAutodelete==TRUE) delete m_FieldsFont;
	m_FieldsFont           = font;
	if(font!=NULL)
	    m_FieldsFontAutodelete = autoDelete;
	else
	    m_FieldsFontAutodelete = FALSE;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method  will return the current fields font pointer. It will return NULL
// if no font was assigned to the name.
//----------------------------------------------------------------------------------------------
CFont* CNodeProperties::GetFieldsFont(void) {
	return m_FieldsFont;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method returns the name border thikness for the name box.
//----------------------------------------------------------------------------------------------
int CNodeProperties::GetNameBorderSize(void) {
	return m_NameBorderSize;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method sets the name border thikness for the name box.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetNameBorderSize(int size) {
	m_NameBorderSize=size;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method returns the fields border thikness for the fields list.
//----------------------------------------------------------------------------------------------
int CNodeProperties::GetFieldsBorderSize(void) {
	return m_FieldsBorderSize;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method sets the fields border thikness for the fields list.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetFieldsBorderSize(int size) {
	m_FieldsBorderSize=size;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method will return COLORREF structure which describes 
// the  name box border color.
//----------------------------------------------------------------------------------------------
COLORREF CNodeProperties::GetNameBorderColor(void) {
	return m_NameBorderColor;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method will fill the internal COLOREF structure which describe name box
// border color.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetNameBorderColor(COLORREF color) {
	m_NameBorderColor=color;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method will return a pointer to the COLORREF structure whichs describes 
// the  name box caption color.
//----------------------------------------------------------------------------------------------
COLORREF CNodeProperties::GetNameTextColor(void) {
	return m_NameTextColor;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method will fill the internal COLOREF structure which describes name box
// caption color.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetNameTextColor(COLORREF color) {
	m_NameTextColor=color;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method will return a pointer to the COLORREF structure which describes
// the  name box background color.
//----------------------------------------------------------------------------------------------
COLORREF CNodeProperties::GetNameBackgroundColor(void) {
	return m_NameBackgroundColor;
}
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
// This method will fill the internal COLOREF structure which describes name box
// background color.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetNameBackgroundColor(COLORREF color) {
	m_NameBackgroundColor=color;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method will return a pointer to the COLORREF structure which describes 
// the  name box border color.
//----------------------------------------------------------------------------------------------
COLORREF CNodeProperties::GetFieldsBorderColor(void) {
	return m_FieldsBorderColor;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method will fill the internal COLOREF structure which describes fields 
// list border color.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetFieldsBorderColor(COLORREF color) {
	m_FieldsBorderColor=color;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method will return a pointer to the COLORREF structure which describes 
// the name box caption color.
//----------------------------------------------------------------------------------------------
COLORREF CNodeProperties::GetFieldsTextColor(void) {
	return m_FieldsTextColor;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method will fill the internal COLOREF structure which describes fields 
// list text color.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetFieldsTextColor(COLORREF color) {
    m_FieldsTextColor=color;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method will return a pointer to the COLORREF structure which describes 
// the name box background color.
//----------------------------------------------------------------------------------------------
COLORREF CNodeProperties::GetFieldsBackgroundColor(void) {
	return m_FieldsBackgroundColor;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method will fill the internal COLOREF structure which describes fields 
// list background color.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetFieldsBackgroundColor(COLORREF color) {
	m_FieldsBackgroundColor=color;
}
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
// This methid returns the space size in HIMETRIC between node name box and node
// fields list.
//----------------------------------------------------------------------------------------------
int CNodeProperties::GetNameFieldsSpace(void) {
	return m_NameToFieldsSpace;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method sets the space between node name box and node fields list.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetNameFieldsSpace(int space) {
	m_NameToFieldsSpace=space;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This function assignes the image to the node name..
// If aoutoDelete flag is set, the image list will be removed when this 
// class removed.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetNameImageList(CBitmapList* list, CSize& size, BOOL autoDelete) {
	if(m_NodeBitmapList!=NULL && m_NodeBitmapAutodelete==TRUE) 
		delete m_NodeBitmapList;
	m_NodeBitmapList=list;
	if(list!=NULL)  m_NodeBitmapAutodelete = autoDelete;
	else            m_NodeBitmapAutodelete = FALSE;
	m_NameImageSize=size;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// Will return the desired image size for the header image in HIMETRIC
//----------------------------------------------------------------------------------------------
CSize CNodeProperties::GetNameImageSize(void) {
    return m_NameImageSize;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This function returns the pointer to the node name image or NULL
// if no image list assigned.
//----------------------------------------------------------------------------------------------
CBitmapList* CNodeProperties::GetNameImageList(void) {
	return m_NodeBitmapList;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This function will set the image list which is used to mark fields.
// If aoutoDelete flag is set, the image list will be removed when this 
// class removed.
//----------------------------------------------------------------------------------------------
void CNodeProperties::SetFieldsImageList(CBitmapList* list,CSize& size,BOOL autoDelete) {
	if(m_FieldsImageList!=NULL && m_FieldsImageListAutodelete==TRUE) 
		delete m_FieldsImageList;
	m_FieldsImageList=list;
	if(list!=NULL) m_FieldsImageListAutodelete = autoDelete;
	else            m_FieldsImageListAutodelete = FALSE;
	m_FieldsImageSize=size;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// Will return the desired image size for the fields images in HIMETRIC
//----------------------------------------------------------------------------------------------
CSize CNodeProperties::GetFieldsImageSize(void) {
	return m_FieldsImageSize;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This function will return the pointer to the fields image list or NULL
// if no image list assigned.
//----------------------------------------------------------------------------------------------
CBitmapList* CNodeProperties::GetFieldsImageList(void) {
	return m_FieldsImageList;
}
//----------------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////
// This class describes the edge graphical properties.
////////////////////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------------------------
// Constructor will assign default values for the edge properties
//----------------------------------------------------------------------------------------------
CEdgeProperties::CEdgeProperties() {
	    m_LineStyle              = PS_SOLID; // default line style is solid line
		m_LineThikness           = 50;       // default line thikness is 1/2 mm
		m_LineColor              = 0;        // default line color is black
		m_SourceImages           = NULL;     // no images by default.
		m_SourceImagesAutodelete = FALSE;
		m_TargetImages           = NULL;
		m_TargetImagesAutodelete = FALSE;
		m_MarkerImages           = NULL;
		m_MarkerImagesAutodelete = FALSE;

}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// Destructor will remove images assigned to the edge if autodelete flag is set.
//----------------------------------------------------------------------------------------------
CEdgeProperties::~CEdgeProperties() {
	    if(m_SourceImages!=NULL && m_SourceImagesAutodelete==TRUE) delete m_SourceImages;
		if(m_TargetImages!=NULL && m_TargetImagesAutodelete==TRUE) delete m_TargetImages;
		if(m_MarkerImages!=NULL && m_MarkerImagesAutodelete==TRUE) delete m_MarkerImages;
}
//----------------------------------------------------------------------------------------------

		

//----------------------------------------------------------------------------------------------
// This method sets the line style used ti show edge (solid, dotted etc)
//----------------------------------------------------------------------------------------------
void CEdgeProperties::SetEdgeStyle(int style) {
	m_LineStyle=style;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method returns the edge line style
//----------------------------------------------------------------------------------------------
int CEdgeProperties::GetEdgeStyle(void) {
	return m_LineStyle;
}
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
// This method sets the thikness of the edge line (HIMETRIC coordinate system)
//----------------------------------------------------------------------------------------------
void CEdgeProperties::SetEdgeThikness(int thikness) {
   m_LineThikness=thikness;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method returns the thikness of the edge line (HIMETRIC coordinate system)
//----------------------------------------------------------------------------------------------
int CEdgeProperties::GetEdgeThikness(void) {
	return m_LineThikness;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method sets the edge drawing color
//----------------------------------------------------------------------------------------------
void CEdgeProperties::SetEdgeColor(COLORREF color) {
	m_LineColor=color;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method returns the edge drawing color
//----------------------------------------------------------------------------------------------
COLORREF CEdgeProperties::GetEdgeColor(void) {
	return m_LineColor;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method sets the imagelist containing 4 images to draw the edge start point.
// We need this four images because the edge can go from left to right, from right
// to left, from top to bottom and from bottom to top. We will not rotate the 
// original bitmap but we will set an index in the image list.
// If the autoDelete flag is set this imagelist will be destroyed in this class 
// destructor.
//----------------------------------------------------------------------------------------------
void CEdgeProperties::SetEdgeSourceImages(CBitmapList* image, CSize& imageSize, BOOL autoDelete) {
	if(m_SourceImages!=NULL && m_SourceImagesAutodelete==TRUE) delete m_SourceImages;
	m_SourceImages=image;
	if(image!=NULL) m_SourceImagesAutodelete = autoDelete;
	else            m_SourceImagesAutodelete = FALSE;
	m_SourceImageSize=imageSize;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method returns the desired size of the source image
//----------------------------------------------------------------------------------------------
CSize CEdgeProperties::GetEdgeSourceSize(void) {
	return m_SourceImageSize;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// Returns the image list for the edge start point image.
//----------------------------------------------------------------------------------------------
CBitmapList* CEdgeProperties::GetEdgeSourceImages(void) {
	return m_SourceImages;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method sets the imagelist containing 4 images to draw the edge end point.
// We need this four images because the edge can go from left to right, from right 
// to left, from top to bottom and from bottom to top. We will not rotate the 
// original bitmap but we will set an index in the image list.
// If the autoDelete flag is set this imagelist will be destroyed in this class 
// destructor.
//----------------------------------------------------------------------------------------------
void CEdgeProperties::SetEdgeTargertImages(CBitmapList* image, CSize& imageSize, BOOL autoDelete) {
	if(m_TargetImages!=NULL && m_TargetImagesAutodelete==TRUE) delete m_TargetImages;
	m_TargetImages=image;
	if(image!=NULL) m_TargetImagesAutodelete = autoDelete;
	else            m_TargetImagesAutodelete = FALSE;
	m_TargetImageSize=imageSize;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method returns the desired size of the target image
//----------------------------------------------------------------------------------------------
CSize CEdgeProperties::GetEdgeTargetSize(void) {
	return m_TargetImageSize;
}
//----------------------------------------------------------------------------------------------

	
//----------------------------------------------------------------------------------------------
// Returns the image list for the edge start point image.
//----------------------------------------------------------------------------------------------
CBitmapList* CEdgeProperties::GetEdgeTargetImages(void) {
	return m_TargetImages;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This method sets the imagelist containing 2 images to draw the edge marker.
// We need this 2 images because the edge marker point can be set on the horizontal 
// or on the vertical edge part.
// If the autoDelete flag is set this imagelist will be destroyed in this class 
// destructor.
//----------------------------------------------------------------------------------------------
void CEdgeProperties::SetEdgeMarkerImages(CBitmapList* image, CSize& imageSize, BOOL autoDelete) {
	if(m_MarkerImages!=NULL && m_MarkerImagesAutodelete==TRUE) delete m_MarkerImages;
	m_MarkerImages=image;
	if(image!=NULL) m_MarkerImagesAutodelete = autoDelete;
	else            m_MarkerImagesAutodelete = FALSE;
	m_MarkerImageSize=imageSize;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This method returns the desired size of the marker image
//----------------------------------------------------------------------------------------------
CSize CEdgeProperties::GetEdgeMarkerSize(void) {
	return m_MarkerImageSize;
}
//----------------------------------------------------------------------------------------------
		
//----------------------------------------------------------------------------------------------
// Returns the image list for the edge marker image.
//----------------------------------------------------------------------------------------------
CBitmapList* CEdgeProperties::GetEdgeMarkerImages(void) {
	return m_MarkerImages;
}
//----------------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////////////////














/////////////////////////////////////////////////////////////////////////////
// CGraphView

IMPLEMENT_DYNCREATE(CGraphView, CScrollView)

CGraphView::CGraphView() {
   m_GraphSize.cx=0;
   m_GraphSize.cy=0;
   m_ClientGraph = new CDisGraph;
   m_ClientDC = NULL;
   m_NameMapsAmount=0;
   m_NameMapID=NULL;
   m_FieldsMapsAmount=0;
   m_FieldsMapID=NULL;
   m_SettingsFile = "";
}

CGraphView::~CGraphView() {
	delete m_ClientGraph;
	if(m_NameMapID) delete m_NameMapID;
	if(m_FieldsMapID) delete m_FieldsMapID;
}


BEGIN_MESSAGE_MAP(CGraphView, CScrollView)
	//{{AFX_MSG_MAP(CGraphView)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnPrintPreview)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(IDC_GRAPHPROP, OnViewProperties)
	ON_WM_RBUTTONDBLCLK()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphView drawing

void CGraphView::OnInitialUpdate() {

	CScrollView::OnInitialUpdate();

	//=========== Extracting all parameters from the settings file ================
	CGraphSettingsParser parser;
	parser.ParseSettings(m_SettingsFile);
	CString selectStr          = parser.GetValue(CString("SelectionScript"));
	CString childCreateStr     = parser.GetValue(CString("ChildCreationScript"));
	CString childTestStr       = parser.GetValue(CString("ChildTestScript"));
	CString parentsCreateStr   = parser.GetValue(CString("ParentCreationScript"));
	CString parentsTestStr     = parser.GetValue(CString("ParentTestScript"));
	CString fieldsCreateStr    = parser.GetValue(CString("FieldsCreationScript"));
	CString nodeNameStr        = parser.GetValue(CString("NodeNameExtractionScript"));
	CString nodeImageStr       = parser.GetValue(CString("NodeImageExtractionScript"));
	CString fieldNameStr       = parser.GetValue(CString("FieldNameExtractionScript"));
	CString fieldImageStr      = parser.GetValue(CString("FieldImageExtractionScript"));
    CString scriptStr          = parser.GetValue(CString("Script"));
	CString nodeImages         = parser.GetValue(CString("NodeImages"));
	CString fieldImages        = parser.GetValue(CString("FieldImages"));
	CString relations          = parser.GetValue(CString("RelationsCreationScript"));
	//=============================================================================

	//======== Assigning expand and collapse icons ===========
	CBitmap* expand;
	expand = new CBitmap;
	expand->LoadBitmap(IDB_GRAPH_EXPAND);
    m_PortImages.AddTail(expand);
	CBitmap* collapse;
	collapse = new CBitmap;
	collapse->LoadBitmap(IDB_GRAPH_COLLAPSE);
    m_PortImages.AddTail(collapse);
	//=========================================================



	// ==========We will create datasource object which will be used ===========
	// to evaluate server access scripts from the client graph
	CRect zeroRect;
	zeroRect.left=zeroRect.right=zeroRect.top=zeroRect.bottom=0;
	m_DataSource.Create("DataSource", WS_CHILD,
		                 zeroRect, 
						 this, 
						 43724);

	// This will assign datasource to the client graph
	m_ClientGraph->SetDataSource(&m_DataSource);
    //m_ClientGraph->Evaluate(CString("source_dis treeviewscripts.dis"));
	CString sourceCmd;
	sourceCmd.Format("source_dis \"%s\"",scriptStr);
    m_ClientGraph->Evaluate(sourceCmd);
	//=========================================================================

	
	m_ClientGraph->SetRelationsScript(relations);

	// This callbacks will connect client graph with it's view
	m_ClientGraph->SetNodeDrawCallback(DrawNodeCallback);
	m_ClientGraph->SetNodeSizeCalculationCallback(SizeNodeCallback);
	m_ClientGraph->SetEdgeDrawCallback(DrawEdgeCallback);
	m_ClientGraph->SetNodeHeaderPortCalculationCallback(HeaderPortCallback);
	m_ClientGraph->SetNodeFieldPortCalculationCallback(FieldsPortCallback);

	int id;
	for(id=0;id<999;id++) if(viewInstance[id]==NULL) break;
	viewInstance[id] = this;
    m_ClientGraph->SetClientID(id);

    m_ClientGraph->pHDigraph()->pHTailor()->usePorts(TRUE);

    // Creating edge and node properties. We will set them using
	// "DataExchange" method of the m_GraphSettings class
	m_UnselectedNode = new CNodeProperties;
	m_SelectedNode   = new CNodeProperties;
	m_FocusedNode    = new CNodeProperties;
	m_EdgeUnselected = new  CEdgeProperties;
	m_EdgeSelected   = new  CEdgeProperties;
	m_EdgeFocused    = new  CEdgeProperties;

	for(id=0;id<12;id++) m_Relations[id]=new CEdgeProperties;


	// ===== Assigning images to the entity kinds =======================
    CString val;
	int headerCounter=0;
	int fieldCounter=0;
    int headerBitmaps[1000];
    int fieldsBitmaps[1000];
	register i;
	for(i=0;i<nodeImages.GetLength();i++) {
		if(nodeImages[i]!=',') {
			val+=nodeImages[i];
		} else {
			headerBitmaps[headerCounter++]=atoi(val);
			val="";
		}
	}
	if(val.GetLength()>0) headerBitmaps[headerCounter++]=atoi(val);
	val="";
	for(i=0;i<fieldImages.GetLength();i++) {
		if(fieldImages[i]!=',') {
			val+=fieldImages[i];
		} else {
			fieldsBitmaps[fieldCounter++]=atoi(val);
			val="";
		}
	}
	if(val.GetLength()>0) fieldsBitmaps[fieldCounter++]=atoi(val);
	AssignBitmaps(headerBitmaps,headerCounter,fieldsBitmaps,fieldCounter);
	//=======================================================================



	CString sel;
	sel.Format("%s $DiscoverSelection",selectStr);

	m_ClientGraph->CreateGraph(sel,
		                       childCreateStr,
							   parentsCreateStr, 
							   fieldsCreateStr,
							   nodeNameStr,
							   fieldNameStr,
							   nodeImageStr,
							   fieldImageStr,
							   childTestStr,
							   parentsTestStr,
							   m_UnselectedNode,
							   m_SelectedNode,
							   m_FocusedNode,
							   m_EdgeUnselected,
							   m_EdgeSelected,
							   m_EdgeFocused);

    //======== Reading the settings from the archive. We will use
	// default if no archive found.
	char buf[1024];
	CString settingsPath;
    if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
         settingsPath=buf;
         settingsPath+="/Settings/";
	} else {
        settingsPath="C:/Discover/Settings/";
	}
	CString settingsName;
	BOOL trigger = FALSE;
	for(i=m_SettingsFile.GetLength()-1;i>=0;i--) {
		if(m_SettingsFile[i]=='.') {
			trigger=TRUE;
			continue;
		}
		if(m_SettingsFile[i]=='\\' || m_SettingsFile[i]=='/') break;
		if(trigger==TRUE) settingsName=CString(m_SettingsFile[i])+settingsName;			
	}
	settingsName+=".dat";
    settingsPath+=settingsName;
    CFile file;
	if(file.Open(settingsPath,CFile::modeRead)==TRUE) {
		CArchive ar(&file,CArchive::load);
		ar >> m_Settings;
	} else {
		m_Settings = new CGraphSettings;
		m_Settings->Default();
	}
	m_Settings->SetViewType(m_ViewType);
	m_Settings->DataExchange(this,CGraphSettings::Set);
	//==============================================================


	//====== Initial layout ================
	m_ClientDC=InitDC(GetDC());
	GraphLayout();
	ReleaseDC(m_ClientDC);
	//=======================================

}
//-----------------------------------------------------------------------------------

void CGraphView::OnDraw(CDC* pDC) {
	m_ClientDC = InitDC(pDC,FALSE);
	m_ClientGraph->DrawGraph();

}

/////////////////////////////////////////////////////////////////////////////
// CGraphView diagnostics

#ifdef _DEBUG
void CGraphView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CGraphView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGraphView message handlers




//=============================================================================================
//=============================== GCallback interface =========================================
//=============================================================================================
	
//----------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------
void CGraphView::DrawNode(CRect&           nodeRect, 
		                  CString&         nodeName,
						  int              nodeKind,
				          CItemsList*      fieldsList,
						  CNodeProperties* nodeProperties,
						  int connectorSide,
						  BOOL expanded,
						  int parentsConnectorSide,
						  BOOL parentsExpanded) {
CRect headerRect=nodeRect;
CRect itemsRect = nodeRect;

    // Extracting rectangles for two base node elements - header and items list.
	int headerHeight  = GetHeaderRectHeight(nodeName,nodeProperties);
	headerRect.bottom = headerRect.top-headerHeight;
	itemsRect.top    -=(headerHeight+nodeProperties->GetNameFieldsSpace());
	switch(nodeProperties->GetNodeShape()) {
	   case RECTANGLE_BLOCK     : DrawRectangleHeader(headerRect,nodeName,nodeKind,nodeProperties);
		                          break;
       case ROUNDRECT_BLOCK     : DrawRoundrectHeader(headerRect,nodeName,nodeKind,nodeProperties);
		                          break;
       case VERTEX_BLOCK        : DrawVertexHeader(headerRect,nodeName,nodeKind,nodeProperties);
		                          break;
       case PARALELLOGRAM_BLOCK : DrawParalellogramHeader(headerRect,nodeName,nodeKind,nodeProperties);
		                          break;
       case LOOP_BLOCK          : DrawLoopHeader(headerRect,nodeName,nodeKind,nodeProperties);
		                          break;
       case IF_BLOCK            : DrawIfHeader(headerRect,nodeName,nodeKind,nodeProperties);
		                          break;
       case CIRCLE_BLOCK        : DrawCircleHeader(headerRect,nodeName,nodeKind,nodeProperties);
		                          break;
	}
	DrawNodeItemsList(itemsRect,fieldsList,nodeProperties);
    DrawNodePort(nodeRect,headerHeight,connectorSide,expanded);
    DrawNodePort(nodeRect,headerHeight,parentsConnectorSide,parentsExpanded);
}
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
// GCALLBACK interface. This view function draws the given edge using it's path 
// calculated during layout process in the client graph. It will use the edge style 
// class to determine  the edge color, edge line style, edge source/target images and 
// edge bitmap. This function must been assigned and is called by the Client Graph 
// every time it needs to draw the edge.
// "vertexes" is a pointer to the array of edge vertexes, it forms edge drawing path.
// "vertexAmount" contains the amount of vertexes in the vertex array.
// "edgeProperties" describes the edge color, linestyle, thikness and images.
//----------------------------------------------------------------------------------------------
void CGraphView::DrawEdge(CPoint* vertexes, 
		                  int vertexAmount, 
						  CEdgeProperties* edgeProperties) {
int       oldMapMode;
int       sourceImageIndex;
int       targetImageIndex;
int       markerImageIndex;
CPoint    markerImagePos;
int       markerVertexNumber;
CPoint    toMarkerImage;
CPoint    fromMarkerImage;
CSize     sourceSize;
CSize     targetSize;
CSize     markerSize;
CSize     baseSize;

    oldMapMode=m_ClientDC->SetMapMode(MM_HIMETRIC);

	// We will use a pen which will use edge pre-set color, style and thikness.
	CBrush colBrush(edgeProperties->GetEdgeColor());
	m_ClientDC->SelectObject(&colBrush);
    LOGBRUSH logBrush;
	colBrush.GetLogBrush(&logBrush);
	CPen linePen (edgeProperties->GetEdgeStyle()|PS_ENDCAP_FLAT|PS_GEOMETRIC|PS_JOIN_BEVEL ,
		          edgeProperties->GetEdgeThikness(), 
		          &logBrush); 


	m_ClientDC->SelectObject(&linePen);


    // We need to determine image index for the source image, destination image and
	// marker image. The reason is very simple: edge can connect with the node on it's
	// top, left,right or bottom side and we need to switch between 4 connection images
	// for source and destination. The part of the we are placing the node marker can be 
	// horizontal or vertical, so we need to switch between 2 marker images.


	// Source image calculation & drawing
	if(edgeProperties->GetEdgeSourceImages()!=NULL) {
		// Calculating which bitmap we need to use acording the edge direction
		sourceImageIndex = FindSourceImageIndex(vertexes,vertexAmount);
		// Getting the source bitmap from the properties
		CBitmap* bitmap = edgeProperties->GetEdgeSourceImages()->GetBitmap(sourceImageIndex);
		if(bitmap!=NULL) {
	       baseSize = bitmap->GetBitmapDimension();
		   sourceSize=edgeProperties->GetEdgeSourceSize();
		   int x,y;
		   switch(sourceImageIndex) {
			    case LEFT_CONNECT   : x=vertexes[0].x-abs(sourceSize.cx);
					                  y=vertexes[0].y-(abs(sourceSize.cy)+1)/2;
									  break;
			    case RIGHT_CONNECT  : x=vertexes[0].x;
					                  y=vertexes[0].y-(abs(sourceSize.cy)+1)/2;
									  break;
			    case TOP_CONNECT    : x=vertexes[0].x-(abs(sourceSize.cx)+1)/2;
					                  y=vertexes[0].y;
									  break;
			    case BOTTOM_CONNECT : x=vertexes[0].x-(abs(sourceSize.cx)+1)/2;
					                  y=vertexes[0].y+abs(sourceSize.cy);
									  break;
		   }
		   SIZE p;
		   p.cx=p.cy=1;
		   m_ClientDC->DPtoLP(&p);


	       BITMAPINFOHEADER dibHeader;
		   ZeroMemory(&dibHeader,sizeof(BITMAPINFOHEADER));
		   dibHeader.biSize=sizeof(BITMAPINFOHEADER);
		   GetDIBits(*GetWindowDC(),*bitmap,0,0,NULL,(LPBITMAPINFO)&dibHeader,DIB_RGB_COLORS);
		   BITMAPINFO* dib;
		   dib =  (BITMAPINFO *)new char [sizeof(BITMAPINFOHEADER)+
		   dibHeader.biSizeImage+
		   dibHeader.biClrUsed*sizeof(RGBQUAD)+5000];
           memcpy(dib,&dibHeader,sizeof(BITMAPINFOHEADER));
		   GetDIBits(*GetWindowDC(),*bitmap,0,dibHeader.biHeight,(char *)dib+sizeof(BITMAPINFOHEADER)+3000,dib,DIB_RGB_COLORS);
		   ::StretchDIBits(*m_ClientDC,
		                   x,
			               y+p.cy,
		                   abs(sourceSize.cx), 
						   abs(sourceSize.cy),
						   0,
						   0,
						   dib->bmiHeader.biWidth,
						   dib->bmiHeader.biHeight,
						   (char *)dib+sizeof(BITMAPINFOHEADER)+3000,
						   dib,
						   DIB_RGB_COLORS,
						   SRCCOPY);
		   delete dib;
		}
	}


    // Target image calculation & drawing
	if(edgeProperties->GetEdgeTargetImages()!=NULL) {
		targetImageIndex = FindTargetImageIndex(vertexes,vertexAmount);
		// Getting target bitmap from the properties
		CBitmap* bitmap = edgeProperties->GetEdgeTargetImages()->GetBitmap(targetImageIndex);
		if(bitmap!=NULL) {
	       baseSize = bitmap->GetBitmapDimension();
		   targetSize=edgeProperties->GetEdgeTargetSize();
		   int x,y;
		   switch(targetImageIndex) {
			    case LEFT_CONNECT   : x=vertexes[vertexAmount-1].x-abs(targetSize.cx);
					                  y=vertexes[vertexAmount-1].y-(abs(targetSize.cy)+1)/2;
									  break;
			    case RIGHT_CONNECT  : x=vertexes[vertexAmount-1].x;
					                  y=vertexes[vertexAmount-1].y-(abs(targetSize.cy)+1)/2;
									  break;
			    case TOP_CONNECT    : x=vertexes[vertexAmount-1].x-(abs(targetSize.cx)+1)/2;
					                  y=vertexes[vertexAmount-1].y;
									  break;
			    case BOTTOM_CONNECT : x=vertexes[vertexAmount-1].x-(abs(targetSize.cx)+1)/2;
					                  y=vertexes[vertexAmount-1].y+abs(targetSize.cy);
									  break;
		   }
		   SIZE p;
		   p.cx=p.cy=1;
		   m_ClientDC->DPtoLP(&p);
	       BITMAPINFOHEADER dibHeader;
		   ZeroMemory(&dibHeader,sizeof(BITMAPINFOHEADER));
		   dibHeader.biSize=sizeof(BITMAPINFOHEADER);
		   GetDIBits(*GetWindowDC(),*bitmap,0,0,NULL,(LPBITMAPINFO)&dibHeader,DIB_RGB_COLORS);
		   BITMAPINFO* dib;
		   dib =  (BITMAPINFO *)new char [sizeof(BITMAPINFOHEADER)+
		   dibHeader.biSizeImage+
		   dibHeader.biClrUsed*sizeof(RGBQUAD)+5000];
           memcpy(dib,&dibHeader,sizeof(BITMAPINFOHEADER));
		   GetDIBits(*GetWindowDC(),*bitmap,0,dibHeader.biHeight,(char *)dib+sizeof(BITMAPINFOHEADER)+3000,dib,DIB_RGB_COLORS);
		   ::StretchDIBits(*m_ClientDC,
		                   x,
			               y+p.cy,
		                   abs(targetSize.cx), 
						   abs(targetSize.cy),
						   0,
						   0,
						   dib->bmiHeader.biWidth,
						   dib->bmiHeader.biHeight,
						   (char *)dib+sizeof(BITMAPINFOHEADER)+3000,
						   dib,
						   DIB_RGB_COLORS,
						   SRCCOPY);
		   delete dib;
		}

	}
    
    if(edgeProperties->GetEdgeMarkerImages()!=NULL) {
		// marker image drawing position is a point on the edge which
		// divide the edge into two equal-length parts.
		markerImageIndex = FindMarkerImageIndex(vertexes,vertexAmount,&markerImagePos,&markerVertexNumber);
		// Getting target bitmap from the properties
		CBitmap* bitmap = edgeProperties->GetEdgeMarkerImages()->GetBitmap(markerImageIndex);
		if(bitmap!=NULL) {
	       baseSize = bitmap->GetBitmapDimension();
		   markerSize=edgeProperties->GetEdgeMarkerSize();
		   int x,y;
		   x=markerImagePos.x-abs(markerSize.cx)/2;
           y=markerImagePos.y-abs(markerSize.cy)/2;
		   SIZE p;
		   p.cx=p.cy=1;
		   m_ClientDC->DPtoLP(&p);
	       BITMAPINFOHEADER dibHeader;
		   ZeroMemory(&dibHeader,sizeof(BITMAPINFOHEADER));
		   dibHeader.biSize=sizeof(BITMAPINFOHEADER);
		   GetDIBits(*GetWindowDC(),*bitmap,0,0,NULL,(LPBITMAPINFO)&dibHeader,DIB_RGB_COLORS);
		   BITMAPINFO* dib;
		   dib =  (BITMAPINFO *)new char [sizeof(BITMAPINFOHEADER)+
		   dibHeader.biSizeImage+
		   dibHeader.biClrUsed*sizeof(RGBQUAD)+5000];
           memcpy(dib,&dibHeader,sizeof(BITMAPINFOHEADER));
		   GetDIBits(*GetWindowDC(),*bitmap,0,dibHeader.biHeight,(char *)dib+sizeof(BITMAPINFOHEADER)+3000,dib,DIB_RGB_COLORS);
		   ::StretchDIBits(*m_ClientDC,
		                   x,
			               y+p.cy,
		                   abs(markerSize.cx), 
						   abs(markerSize.cy),
						   0,
						   0,
						   dib->bmiHeader.biWidth,
						   dib->bmiHeader.biHeight,
						   (char *)dib+sizeof(BITMAPINFOHEADER)+3000,
						   dib,
						   DIB_RGB_COLORS,
						   SRCCOPY);
		   delete dib;
		}
	}
    // This code will draw complete edge using selected edge color,style and thikness
	// bypassing the edge source, target and marker images.
	CPoint from;
	CPoint to;
	int pos=0;
	POINT pathList[500];
	for(register i=0;i<vertexAmount-1; i++) {
		from=vertexes[i];
		to  =vertexes[i+1];
		// Will not draw on the source image
		if(i==0 && edgeProperties->GetEdgeSourceImages()!=NULL) {
			switch(sourceImageIndex) {
			    case LEFT_CONNECT   : from.x-=abs(sourceSize.cx); break;
			    case RIGHT_CONNECT  : from.x+=abs(sourceSize.cx); break;
			    case TOP_CONNECT    : from.y+=abs(sourceSize.cy); break;
			    case BOTTOM_CONNECT : from.y-=abs(sourceSize.cy); break;
			}

		}

		// Will not draw on the targret image
        if(i==vertexAmount-2 && edgeProperties->GetEdgeTargetImages()!=NULL) {
			switch(targetImageIndex) {
			    case LEFT_CONNECT   : to.x-=abs(targetSize.cx); break;
			    case RIGHT_CONNECT  : to.x+=abs(targetSize.cx); break;
			    case TOP_CONNECT    : to.y+=abs(targetSize.cy); break;
			    case BOTTOM_CONNECT : to.y-=abs(targetSize.cy); break;
			}
		}
		// Will not draw on the marker image
		if(i==markerVertexNumber && edgeProperties->GetEdgeMarkerImages()!=NULL) {
			switch(markerImageIndex) {
			    case HORIZONTAL_EDGE :  toMarkerImage.x   = markerImagePos.x+abs(markerSize.cx)/2;
					                    toMarkerImage.y   = markerImagePos.y;
										fromMarkerImage.x = markerImagePos.x-abs(markerSize.cx)/2;
					                    fromMarkerImage.y = markerImagePos.y;
										break;
				case VERTICAL_EDGE   :  toMarkerImage.x   = markerImagePos.x;
					                    toMarkerImage.y   = markerImagePos.y+abs(markerSize.cy)/2;
										fromMarkerImage.x = markerImagePos.x;
					                    fromMarkerImage.y = markerImagePos.y-abs(markerSize.cy)/2;
										break;
			}
			// We will make a hole in the polyline to bypass the image
			pathList[pos].x=from.x+edgeProperties->GetEdgeThikness()/4;
			pathList[pos].y=from.y-edgeProperties->GetEdgeThikness()/4;
			pos++;
			pathList[pos].x=toMarkerImage.x+edgeProperties->GetEdgeThikness()/4;
			pathList[pos].y=toMarkerImage.y-edgeProperties->GetEdgeThikness()/4;
			pos++;
            
			// Drawing polyline from the source to the marker image
			m_ClientDC->Polyline((LPPOINT)pathList,pos);
			pos=0;

			// Starting to fill polyline from the marker image to the target
			pathList[pos].x=fromMarkerImage.x+edgeProperties->GetEdgeThikness()/4;
			pathList[pos].y=fromMarkerImage.y-edgeProperties->GetEdgeThikness()/4;
			pos++;
			pathList[pos].x=to.x+edgeProperties->GetEdgeThikness()/4;
			pathList[pos].y=to.y-edgeProperties->GetEdgeThikness()/4;
			pos++;
		} else {
			pathList[pos].x=from.x+edgeProperties->GetEdgeThikness()/4;
			pathList[pos].y=from.y-edgeProperties->GetEdgeThikness()/4;
			pos++;
			pathList[pos].x=to.x+edgeProperties->GetEdgeThikness()/4;
			pathList[pos].y=to.y-edgeProperties->GetEdgeThikness()/4;
			pos++;
		}
		// Drawing the polyline from the source to the target if no marker image present
		// or from the marker image to the target in the case the marker image assigned to 
		// the edge.
	    m_ClientDC->Polyline((LPPOINT)pathList,pos);

	}
	m_ClientDC->SetMapMode(oldMapMode);
}
//----------------------------------------------------------------------------------------------


	
//----------------------------------------------------------------------------------------------
// GCALLBACK interface. This function calculates the width and length of the node 
// in HIMETRIC coordinate system. To do this it needs the maximum characters amount
// in the node name or any of it's fields, amount of fields assigned to this node
// and node graphical properties.
//---------------------------------------------------------------------------------------------
CSize CGraphView::CalculateNodeSize(CString& nodeName, 
		                            CItemsList* list, 
									CNodeProperties* nodeProp,
									int& headerHeight) {
int width;
int hWidth;
int fWidth;
int height;   

   hWidth=GetHeaderRectWidth(nodeProp,nodeName);
   headerHeight=GetHeaderRectHeight(nodeName,nodeProp);
   if(list!=NULL && list->GetCount()>0)
       fWidth=GetFieldsRectWidth(nodeProp,*list);
   else 
	   fWidth=0;
   width= hWidth>fWidth ? hWidth : fWidth;

   if(list!=NULL && list->GetCount()>0)
       height = headerHeight+
	            GetFieldsRectHeight(nodeName,nodeProp,list->GetCount())+
			    nodeProp->GetNameFieldsSpace();
   else
       height = headerHeight;
   return CSize(width,height);
}
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
// GCALLBACK interface. This function will return the node header port (connection
// point) at the desired node side. If the port is unavailable it will return FALSE;
// The resulted port will be given in the HIMETRIC coordinate system relative to the
// left-bottom corner of the node rectangle.
//----------------------------------------------------------------------------------------------
CPoint CGraphView::GetHeaderPort(CString& nodeName, 
		                         CItemsList* list, 
								 CNodeProperties* nodeProp,
								 int side) {
int x,y;
CSize headerSize;
CSize nodeSize;
int height;
int scale;


   headerSize.cx = GetHeaderRectWidth(nodeProp,nodeName);
   nodeSize=CalculateNodeSize(nodeName,list,nodeProp,height);
   headerSize.cy=height;
   scale = abs(nodeSize.cy)/900+1;
   switch(side) {
	   case LEFT_CONNECT   : x=(abs(nodeSize.cy)+scale/2)/scale+1;
		                     y=x-(abs(headerSize.cy)+scale)/(2*scale)+1; 
							 break;
	   case RIGHT_CONNECT  : x=(abs(nodeSize.cy)+scale/2)/scale+1;
		                     y=x-(abs(headerSize.cy)+scale)/(2*scale)+1; 
							 break;
	   case TOP_CONNECT    : x=abs(nodeSize.cx/20)+1; 
		                     y=abs(nodeSize.cx/40)+1; 
							 break;
	   case BOTTOM_CONNECT : x=abs(nodeSize.cx/20)+1; 
		                     y=abs(nodeSize.cx/40)+1; 
							 break;
	}
   return CPoint(x,y);
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// GCALLBACK interface. This function will return the node field port (left or right,
// depending from the side value).You must specify the field index for which you need
// to get the port. If there is no fields or no field with this index the function
// will return the header port.
//----------------------------------------------------------------------------------------------
CPoint CGraphView::GetFieldPort(CString& nodeName, 
		                        CItemsList* list, 
					            CNodeProperties* nodeProp,
					            int index,
					            int side) {
int x,y;
CSize headerSize;
CSize nodeSize;
int height;
int scale;


   headerSize.cx = GetHeaderRectWidth(nodeProp,nodeName);
   nodeSize=CalculateNodeSize(nodeName,list,nodeProp,height);
   headerSize.cy=height;
   int itemsAmount=list->GetCount();
   int separatorHeight = nodeProp->GetNameFieldsSpace();
   int itemsHeight     = abs(nodeSize.cy) -abs(headerSize.cy) - separatorHeight;

   scale = abs(nodeSize.cy)/900+1;
   if(index==-1) { // We will return header port if field is unavailable
       switch(side) {
	       case LEFT_CONNECT   : x=(abs(nodeSize.cy)+scale/2)/scale+1;
		                         y=x-(abs(headerSize.cy)+scale)/(2*scale)+1; 
							     break;
	       case RIGHT_CONNECT  : x=(abs(nodeSize.cy)+scale/2)/scale+1;
		                         y=x-(abs(headerSize.cy)+scale)/(2*scale)+1; 
							     break;
	       case TOP_CONNECT    : x=abs(nodeSize.cx/20)+1; 
		                         y=abs(nodeSize.cx/40)+1; 
							     break;
	       case BOTTOM_CONNECT : x=abs(nodeSize.cx/20)+1; 
		                         y=abs(nodeSize.cx/40)+1; 
							     break;
	   }
   } else {
       switch(side) {
	       case LEFT_CONNECT   : x=(abs(nodeSize.cy)+scale/2)/scale+1;
		                         y=x-(  abs(headerSize.cy)+separatorHeight+
									    (itemsHeight*index/itemsAmount)+itemsHeight/itemsAmount/2
									  )/scale+1; 
							     break;
	       case RIGHT_CONNECT  : x=(abs(nodeSize.cy)+scale/2)/scale+1;
		                         y=x-(  abs(headerSize.cy)+separatorHeight+
									    (itemsHeight*index/itemsAmount)+itemsHeight/itemsAmount/2
									  )/scale+1; 
							     break;
	       case TOP_CONNECT    : x=abs(nodeSize.cx/20)+1; 
		                         y=abs(nodeSize.cx/40)+1; 
							     break;
	       case BOTTOM_CONNECT : x=abs(nodeSize.cx/20)+1; 
		                         y=abs(nodeSize.cx/40)+1; 
							     break;
	   }
   }
   return CPoint(x,y);
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This private member function designed to locate bitmap index in the image list. It
// will analyse the edge direction and will select the proper image.
// used when selecting the source node connection image.
//----------------------------------------------------------------------------------------------
int CGraphView::FindSourceImageIndex(CPoint * vertexes, int vertexAmount) {

	int ret_val = 0;

	if (vertexAmount >= 2) {
		int xdiff=vertexes[1].x-vertexes[0].x;
		int ydiff=vertexes[1].y-vertexes[0].y;

		// If this is horizontal edge connection
		if(abs(xdiff)>abs(ydiff)) {
        // If this is left-to-right direction
			if(xdiff>0) {
				ret_val = RIGHT_CONNECT;
			} else {
				ret_val = LEFT_CONNECT;
			} 
		} else {
        // if this is  the bottom-to-top connection
			if(ydiff>0) {
				ret_val = TOP_CONNECT;
			} else {
				ret_val = BOTTOM_CONNECT;
			}
		}
	}
	return ret_val;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This private member function designed to locate bitmap index in the image list. It
// will analyse the edge direction and will select the proper image.
// Used when selecting the target node connection image.
//----------------------------------------------------------------------------------------------
int CGraphView::FindTargetImageIndex(CPoint * vertexes, int vertexAmount) {
int xdiff;
int ydiff;

	if(vertexAmount<2) return 0;

	xdiff=vertexes[vertexAmount-1].x-vertexes[vertexAmount-2].x;
	ydiff=vertexes[vertexAmount-1].y-vertexes[vertexAmount-2].y;

	// If this is horizontal edge connection
    if(abs(xdiff)>abs(ydiff)) {
        // If this is left-to-right direction
		if(xdiff>0) {
            return LEFT_CONNECT;
		} else {
			return RIGHT_CONNECT;
		} 
	} else {
        // if this is  the bottom-to-top connection
		if(ydiff>0) {
			return BOTTOM_CONNECT;
		} else {
			return TOP_CONNECT;
		}
	}
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This private function designed to calculate three parameters:
// 1. Image index for the edge marker image depending from the direction of the edge in the
//    marker place; (return value)
// 2. Marker place itself - this is a HIMETRIC point in which the middle of the marker image
//    must be located.(markerImagePos parameter)
// 3. The vertex number after which the marker image must be located. (markerVertexNumber
//    parameter)
// To calculate this values the function needs the set of vertexes (vertexes parameter)
// and amount of vertexes in the path.
//----------------------------------------------------------------------------------------------
int CGraphView::FindMarkerImageIndex(CPoint * vertexes, 
									 int vertexAmount, 
									 CPoint * markerImagePos, 
									 int * markerVertexNumber) {
register i;
int totalLength;
int lengthToMarkerPoint;
int l;
int xdiff;
int ydiff;

      // We need to calculate the total lenght of the edge
     totalLength=0;
     for(i=0; i< vertexAmount-1; i++)  {
	    l = (int)sqrt((double)(vertexes[i+1].x-vertexes[i].x)*(double)(vertexes[i+1].x-vertexes[i].x) +
                 (double)(vertexes[i+1].y-vertexes[i].y)*(double)(vertexes[i+1].y-vertexes[i].y)
			    );
        totalLength+=l;
	 }

	 // The marker will be placed at the middle of the edge
	 lengthToMarkerPoint = totalLength/2;

     // We need to find the vertex number after which we will place our marker point.
	 // We will also calculate the offset from the last vertex to the marker point
	 int currentLength = 0;
	 int offset;
     for(i=0;i<vertexAmount;i++) {
	    l = (int)sqrt((double)(vertexes[i+1].x-vertexes[i].x)*(double)(vertexes[i+1].x-vertexes[i].x) +
                 (double)(vertexes[i+1].y-vertexes[i].y)*(double)(vertexes[i+1].y-vertexes[i].y)
			    );
        currentLength+=l;
		if(currentLength>=lengthToMarkerPoint) {
            offset = lengthToMarkerPoint-(currentLength-l);
			break;
		}
	 }
	 *markerVertexNumber = i;

     markerImagePos->x=vertexes[i+1].x - ((vertexes[i+1].x-vertexes[i].x)*(l-offset)/l);
     markerImagePos->y=vertexes[i+1].y - ((vertexes[i+1].y-vertexes[i].y)*(l-offset)/l);

	 // We need to calculate marker image index depending from the direction of the edge
	 xdiff=vertexes[i+1].x-vertexes[i].x;
	 ydiff=vertexes[i+1].y-vertexes[i].y;

     if(abs(xdiff)>abs(ydiff)) {
		 return HORIZONTAL_EDGE;
	 } else {
		 return VERTICAL_EDGE;
	 }
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This private function calculates node header height using the node shape, header font 
// properties and header border size.
//----------------------------------------------------------------------------------------------
int CGraphView::GetHeaderRectHeight(CString& nodeName,CNodeProperties * prop) {
int height=0;
CSize textSize;;
int textHeight;
int imageHeight;
CSize imageSize;
CFont* nameFont; 

    int savedDC = m_ClientDC->SaveDC();
	m_ClientDC->SetMapMode(MM_HIMETRIC);
	switch(prop->GetNodeShape()) {
	   case RECTANGLE_BLOCK     : 
       case ROUNDRECT_BLOCK     : 
       case VERTEX_BLOCK        : 
       case PARALELLOGRAM_BLOCK : // We need to calculate up and down border sizes
		                          height+=(2*prop->GetNameBorderSize()); 
								  // We also need to calculate node font size
	                              nameFont=prop->GetNameFont();
	                              if(nameFont!=NULL) 
									  m_ClientDC->SelectObject(nameFont);
								  textSize=m_ClientDC->GetTextExtent(nodeName);
								  textHeight=abs(textSize.cy);
								  imageSize=prop->GetNameImageSize();
								  imageHeight=abs(imageSize.cy);
								  height+=imageHeight>textHeight ? imageHeight : textHeight;
								  height+=100;
		                          break;
       case LOOP_BLOCK          : 
		                          break;
       case IF_BLOCK            : 
		                          break;
       case CIRCLE_BLOCK        : 
		                          break;
	}
    m_ClientDC->RestoreDC(savedDC);	
    return height;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This private function will calculate the header rectangle desired width based on the
// node properties and the text we need to display. This doesn't mean that the node will have 
// this width, it can be smaller in the case desired width is too big.
//----------------------------------------------------------------------------------------------
int CGraphView::GetHeaderRectWidth(CNodeProperties* prop, CString & nodeText) {
int width=0;
CSize textSize;
CSize imageSize;
CFont* nameFont; 

    int savedDC = m_ClientDC->SaveDC();
	m_ClientDC->SetMapMode(MM_HIMETRIC);
    // We need to calculate up and down border sizes
	width+=(2*prop->GetNameBorderSize()); 
	// We also need to calculate node font size
	nameFont=prop->GetNameFont();
	if(nameFont!=NULL) 
		  m_ClientDC->SelectObject(nameFont);
    textSize=m_ClientDC->GetTextExtent(nodeText) ;
	imageSize=prop->GetNameImageSize();
	width+=(textSize.cx+imageSize.cx+100);
	switch(prop->GetNodeShape()) {
	   case RECTANGLE_BLOCK     : break;
       case ROUNDRECT_BLOCK     : width+=GetHeaderRectHeight(nodeText,prop);
								  break;
       case VERTEX_BLOCK        : width+=GetHeaderRectHeight(nodeText,prop);
								  break;
       case PARALELLOGRAM_BLOCK : width+=GetHeaderRectHeight(nodeText,prop);
		                          break;
       case LOOP_BLOCK          : 
		                          break;
       case IF_BLOCK            : 
		                          break;
       case CIRCLE_BLOCK        : 
		                          break;
	}
    m_ClientDC->RestoreDC(savedDC);	
    return width;

}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This function will calculate the desired node fields rectangle width based on the maximum
// character length in the list of items and the node properties.
//----------------------------------------------------------------------------------------------
int  CGraphView::GetFieldsRectWidth(CNodeProperties* prop, CItemsList& items) {
CNodeItem item;  
int        maxTextWidth;
int        width=0;
CFont*     fieldsFont;
CSize      imageSize;
CSize      textSize;
register   i;

    // Save the current DC settings
    int savedDC = m_ClientDC->SaveDC();
	m_ClientDC->SetMapMode(MM_HIMETRIC);

    // Select the fields font, if any, to calculate string dimentions properly;
	fieldsFont=prop->GetFieldsFont();
	if(fieldsFont!=NULL) 
	     m_ClientDC->SelectObject(fieldsFont);


	// We need to calculate the maximum width of text string
	POSITION p = items.GetHeadPosition();
	maxTextWidth=0;
	for(i=0;i<items.GetCount();i++) {
        item = items.GetAt(p);
	    textSize=m_ClientDC->GetTextExtent(item.m_FieldName) ;
		if(textSize.cx>maxTextWidth) maxTextWidth=textSize.cx;
		items.GetNext(p);
	}

    // We need to calculate up and down border sizes
    width+=(2*prop->GetFieldsBorderSize()); 

	// We also need to calculate field image size
	imageSize=prop->GetFieldsImageSize();

	// Final calculation of the rectangle width
	width+=(maxTextWidth+imageSize.cx+150);
    m_ClientDC->RestoreDC(savedDC);	

	return width;
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This function calculates the node fields list height based on the selected node style and 
// the amount of items in the list.
//----------------------------------------------------------------------------------------------
int CGraphView::GetFieldsRectHeight(CString& nodeName,CNodeProperties * prop, int items) {
int height=0;
CSize textSize;;
int textHeight;
int imageHeight;
CSize imageSize;
CFont* fieldsFont; 

  int savedDC = m_ClientDC->SaveDC();
  m_ClientDC->SetMapMode(MM_HIMETRIC);
  // We need to calculate up and down border sizes+division  border sizes
  height+=((items+1)*prop->GetFieldsBorderSize()); 
  // We also need to calculate node font size
  fieldsFont=prop->GetFieldsFont();
  if(fieldsFont!=NULL) 
		m_ClientDC->SelectObject(fieldsFont);
  textSize=m_ClientDC->GetTextExtent(nodeName);
  textHeight=abs(textSize.cy);
  // And we need image size, if any, assigned to the field								  
  imageSize=prop->GetNameImageSize();
  imageHeight=abs(imageSize.cy);
  // The height of the item will be determined by it's image or it's text, whichever bigger
  height+=items*(imageHeight>textHeight ? imageHeight : textHeight);
  height+=80*items;
   m_ClientDC->RestoreDC(savedDC);	
  return height;
}
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
// This private function designed to draw the rectangular node header - maybe the only one
// node element if no 
//----------------------------------------------------------------------------------------------
void CGraphView::DrawRectangleHeader(CRect & rect, CString & nodeName, int nodeKind,CNodeProperties * nodeProperties) {
int savedDC;
CSize imageSize;
CSize baseSize;
int   userFieldHeight;
int diff;

    savedDC = m_ClientDC->SaveDC();

    m_ClientDC->SetMapMode(MM_HIMETRIC);
    m_ClientDC->SetBkMode(TRANSPARENT);
	// Name border will be solid with the thikness and color defined in the node
	// properties.
    CPen borderPen(PS_SOLID,
		           nodeProperties->GetNameBorderSize(), 
				   nodeProperties->GetNameBorderColor());
    m_ClientDC->SelectObject(&borderPen);

	// We need to select the brush to fill the background
	CBrush background(nodeProperties->GetNameBackgroundColor());
    m_ClientDC->SelectObject(&background);

	// Need to correct border rectangle because of the frame thikness
	CRect borderRect;
	borderRect.left   = rect.left+nodeProperties->GetNameBorderSize()/2;
	borderRect.top    = rect.top-nodeProperties->GetNameBorderSize();
	borderRect.right  = rect.right-nodeProperties->GetNameBorderSize()/2;
	borderRect.bottom = rect.bottom;

	// This is internal rectangle height - we will use it to center.
	userFieldHeight = abs(borderRect.Height());

	// This will draw the name rectangle.
	CPoint ellipse;
	ellipse.x=0;
	ellipse.y=0;
	m_ClientDC->RoundRect(borderRect,ellipse);

	// We need to place header bitmap, if any, into the header rectangle
	imageSize.cx=imageSize.cy=0;
	CBitmapList* bitmapList = nodeProperties->GetNameImageList();
	if(bitmapList && nodeKind>=0) {
		CBitmap* bitmap = bitmapList->GetBitmap(nodeKind);
		if(bitmap!=NULL) {
            baseSize = bitmap->GetBitmapDimension();
		    imageSize=nodeProperties->GetNameImageSize();
	        BITMAPINFOHEADER dibHeader;
		    ZeroMemory(&dibHeader,sizeof(BITMAPINFOHEADER));
		    dibHeader.biSize=sizeof(BITMAPINFOHEADER);
		    GetDIBits(*m_ClientDC,*bitmap,0,0,NULL,(LPBITMAPINFO)&dibHeader,DIB_RGB_COLORS);
		    BITMAPINFO* dib;
		    dib =  (BITMAPINFO *)new char [dibHeader.biSize+
		                                   dibHeader.biSizeImage+
		                                   dibHeader.biClrUsed*sizeof(RGBQUAD)+1000];
            memcpy(dib,&dibHeader,sizeof(BITMAPINFOHEADER));
		    CDC* pDC = GetDC();
		    int offset = dibHeader.biSize+(dibHeader.biClrUsed+1)*sizeof(RGBQUAD)+500;
		    GetDIBits(*pDC,*bitmap,0,dibHeader.biHeight,(char *)dib+offset,dib,DIB_RGB_COLORS);
		    ReleaseDC(pDC);
            if(imageSize.cy<userFieldHeight)  diff=(userFieldHeight-abs(imageSize.cy))/2;
	        else diff = 0;
		    ::StretchDIBits(*m_ClientDC,
		                    borderRect.left+50,
			                borderRect.top-diff,
		                    abs(imageSize.cx), 
						    -abs(imageSize.cy),
						    0,
						    0,
						    dib->bmiHeader.biWidth,
						    dib->bmiHeader.biHeight,
						    (char *)dib+offset,
						    dib,
						    DIB_RGB_COLORS,
						    SRCCOPY);
		   delete dib;
		}
	}


    // This will print node header text centered vertically
	m_ClientDC->SetTextColor(nodeProperties->GetNameTextColor());
	CFont* headerFont; 
	headerFont=nodeProperties->GetNameFont();
	if(headerFont!=NULL) {
		if(m_ClientDC->SelectObject(headerFont)==NULL) {
			AfxMessageBox("Can't select font");
		}
	} else AfxMessageBox("Can't access font");
	m_ClientDC->SetTextAlign(TA_LEFT | TA_BOTTOM);
	CSize textSize=m_ClientDC->GetTextExtent(nodeName);
	if(userFieldHeight>abs(textSize.cy)) diff=(userFieldHeight-abs(textSize.cy))/2;
	else diff=0;
	m_ClientDC->TextOut(borderRect.left+100+imageSize.cx,
                            borderRect.bottom+diff,
						    nodeName);

    m_ClientDC->RestoreDC(savedDC);	

}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// This private function designed to draw the round rectangular node header - maybe the only one
// node element if no 
//----------------------------------------------------------------------------------------------
void CGraphView::DrawRoundrectHeader(CRect & rect,CString & nodeName, int nodeKind, CNodeProperties * nodeProperties) {
int   savedDC;
CSize imageSize;
CSize baseSize;
int   userFieldHeight;
int   diff;
CRect textRect;

    savedDC = m_ClientDC->SaveDC();

    m_ClientDC->SetMapMode(MM_HIMETRIC);
    m_ClientDC->SetBkMode(TRANSPARENT);

	// Name border will be solid with the thikness and color defined in the node
	// properties.
    CPen borderPen(PS_SOLID,
		           nodeProperties->GetNameBorderSize(), 
				   nodeProperties->GetNameBorderColor());
    m_ClientDC->SelectObject(&borderPen);

	// We need to select the brush to fill the background
	CBrush background(nodeProperties->GetNameBackgroundColor());
    m_ClientDC->SelectObject(&background);

	// Need to correct border rectangle because of the frame thikness
	CRect borderRect;
	borderRect.left   = rect.left+nodeProperties->GetNameBorderSize()/2;
	borderRect.top    = rect.top-nodeProperties->GetNameBorderSize()/2;
	borderRect.right  = rect.right;
	borderRect.bottom = rect.bottom;

	// This is internal rectangle height - we will use it to center.
	userFieldHeight = abs(rect.Height())-2*nodeProperties->GetNameBorderSize();

	// This will draw the name rectangle.
	CPoint ellipse;
	ellipse.x=rect.Height();
	ellipse.y=rect.Height();
	m_ClientDC->RoundRect(borderRect,ellipse);

	// Only part of the roundrect internal space can be used to draw text and image
	textRect=rect;
	textRect.left+=abs(rect.Height()/2);
	textRect.right-=abs(rect.Height()/2);

	// We need to place header bitmap, if any, into the header rectangle
	CBitmapList* bitmapList = nodeProperties->GetNameImageList();
	if(bitmapList && nodeKind>=0) {
		CBitmap* bitmap = bitmapList->GetBitmap(nodeKind);
		if(bitmap!=NULL) {
            baseSize = bitmap->GetBitmapDimension();
		    imageSize=nodeProperties->GetNameImageSize();
	        BITMAPINFOHEADER dibHeader;
		    ZeroMemory(&dibHeader,sizeof(BITMAPINFOHEADER));
		    dibHeader.biSize=sizeof(BITMAPINFOHEADER);
		    GetDIBits(*GetWindowDC(),*bitmap,0,0,NULL,(LPBITMAPINFO)&dibHeader,DIB_RGB_COLORS);
		    BITMAPINFO* dib;
		    dib =  (BITMAPINFO *)new char [sizeof(BITMAPINFOHEADER)+
		    dibHeader.biSizeImage+
		    dibHeader.biClrUsed*sizeof(RGBQUAD)+5000];
            memcpy(dib,&dibHeader,sizeof(BITMAPINFOHEADER));
		    GetDIBits(*GetWindowDC(),*bitmap,0,dibHeader.biHeight,(char *)dib+sizeof(BITMAPINFOHEADER)+3000,dib,DIB_RGB_COLORS);
		    int itemHeight=abs(borderRect.Height());
            if(imageSize.cy<itemHeight)  diff=(abs(itemHeight)-abs(imageSize.cy))/2;
	        else diff = 0;
	        SIZE p;
		    p.cx=p.cy=1;
		    m_ClientDC->DPtoLP(&p);
		    ::StretchDIBits(*m_ClientDC,
		                    textRect.left+nodeProperties->GetNameBorderSize(),
			                rect.top-nodeProperties->GetNameBorderSize()-diff+p.cy,
		                    abs(imageSize.cx), 
						    -abs(imageSize.cy),
						     0,
						     0,
						     dib->bmiHeader.biWidth,
						     dib->bmiHeader.biHeight,
						     (char *)dib+sizeof(BITMAPINFOHEADER)+3000,
						     dib,
						     DIB_RGB_COLORS,
						     SRCCOPY);
		    delete dib;
		}
	}



    // This will print node header text centered vertically
	m_ClientDC->SetTextColor(nodeProperties->GetNameTextColor());
	CFont* headerFont; 
	headerFont=nodeProperties->GetNameFont();
	if(headerFont!=NULL) m_ClientDC->SelectObject(headerFont);
	m_ClientDC->SetTextAlign(TA_LEFT | TA_TOP);
	TEXTMETRIC tm;
	m_ClientDC->GetTextMetrics(&tm);
	if(abs(userFieldHeight)>abs(tm.tmHeight)) diff=(abs(userFieldHeight)-abs(tm.tmHeight))/2;
	else diff=0;
	m_ClientDC->TextOut(textRect.left+nodeProperties->GetNameBorderSize()+50+imageSize.cx,
                        rect.top-nodeProperties->GetNameBorderSize()-diff,
						nodeName);
    m_ClientDC->RestoreDC(savedDC);	
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This private function designed to draw the round rectangular node header - maybe the only one
// node element if no 
//----------------------------------------------------------------------------------------------
void CGraphView::DrawVertexHeader(CRect & rect,CString & nodeName,int nodeKind,CNodeProperties * nodeProperties) {
int   savedDC;
CSize imageSize;
CSize baseSize;
int   userFieldHeight;
int   diff;
CRect textRect;

    savedDC = m_ClientDC->SaveDC();

    m_ClientDC->SetMapMode(MM_HIMETRIC);
    m_ClientDC->SetBkMode(TRANSPARENT);

	// Name border will be solid with the thikness and color defined in the node
	// properties.
    CBrush borderBrush(nodeProperties->GetNameBorderColor());

	// We need to select the brush to fill the background
	CBrush background(nodeProperties->GetNameBackgroundColor());

	// This is internal rectangle height - we will use it to center.
	userFieldHeight = abs(rect.Height())-2*nodeProperties->GetNameBorderSize();


	// Need to correct border rectangle because of the frame thikness
	CRect borderRect;
	borderRect.left   = rect.left+nodeProperties->GetNameBorderSize()/2;
	borderRect.top    = rect.top-nodeProperties->GetNameBorderSize()/2;
	borderRect.right  = rect.right;
	borderRect.bottom = rect.bottom;


	// This will draw the name rectangle.
	POINT path[6];
	path[0].x=borderRect.left;
	path[0].y=borderRect.bottom+abs(borderRect.top-borderRect.bottom)/2;

	path[1].x=borderRect.left+abs(borderRect.top-borderRect.bottom)/2;
	path[1].y=borderRect.top;

	path[2].x=borderRect.right-abs(borderRect.top-borderRect.bottom)/2;
	path[2].y=borderRect.top;

	path[3].x=borderRect.right;
	path[3].y=borderRect.bottom+abs(borderRect.top-borderRect.bottom)/2;

	path[4].x=borderRect.right-abs(borderRect.top-borderRect.bottom)/2;
	path[4].y=borderRect.bottom;

	path[5].x=borderRect.left+abs(borderRect.top-borderRect.bottom)/2;
	path[5].y=borderRect.bottom;

	CRgn pathRgn;
	pathRgn.CreatePolygonRgn((POINT *)&path,6,WINDING);


	m_ClientDC->FillRgn(&pathRgn,&background);
	m_ClientDC->FrameRgn(&pathRgn,&borderBrush,nodeProperties->GetNameBorderSize(), 
				                       nodeProperties->GetNameBorderSize());

	// Only part of the vertex internal space can be used to draw text and image
	textRect=rect;
	textRect.left+=abs(rect.Height()/2);
	textRect.right-=abs(rect.Height()/2);

	// We need to place header bitmap, if any, into the header rectangle
	imageSize.cx=imageSize.cy=0;
	CBitmapList* bitmapList = nodeProperties->GetNameImageList();
	if(bitmapList && nodeKind>=0) {
		CBitmap* bitmap = bitmapList->GetBitmap(nodeKind);
		if(bitmap!=NULL) {
            baseSize = bitmap->GetBitmapDimension();
		    imageSize=nodeProperties->GetNameImageSize();
	        BITMAPINFOHEADER dibHeader;
		    ZeroMemory(&dibHeader,sizeof(BITMAPINFOHEADER));
		    dibHeader.biSize=sizeof(BITMAPINFOHEADER);
		    GetDIBits(*GetWindowDC(),*bitmap,0,0,NULL,(LPBITMAPINFO)&dibHeader,DIB_RGB_COLORS);
		    BITMAPINFO* dib;
		    dib =  (BITMAPINFO *)new char [sizeof(BITMAPINFOHEADER)+
		    dibHeader.biSizeImage+
		    dibHeader.biClrUsed*sizeof(RGBQUAD)+5000];
            memcpy(dib,&dibHeader,sizeof(BITMAPINFOHEADER));
		    GetDIBits(*GetWindowDC(),*bitmap,0,dibHeader.biHeight,(char *)dib+sizeof(BITMAPINFOHEADER)+3000,dib,DIB_RGB_COLORS);
		    int itemHeight=abs(borderRect.Height());
            if(imageSize.cy<itemHeight)  diff=(abs(itemHeight)-abs(imageSize.cy))/2;
	        else diff = 0;
	        SIZE p;
		    p.cx=p.cy=1;
		    m_ClientDC->DPtoLP(&p);
		    ::StretchDIBits(*m_ClientDC,
		                    textRect.left+nodeProperties->GetNameBorderSize(),
			                rect.top-nodeProperties->GetNameBorderSize()-diff+p.cy,
		                    abs(imageSize.cx), 
						    -abs(imageSize.cy),
						    0,
						    0,
						    dib->bmiHeader.biWidth,
						    dib->bmiHeader.biHeight,
						    (char *)dib+sizeof(BITMAPINFOHEADER)+3000,
						    dib,
						    DIB_RGB_COLORS,
						    SRCCOPY);
		   delete dib;
		}
	}



    // This will print node header text centered vertically
	m_ClientDC->SetTextColor(nodeProperties->GetNameTextColor());
	CFont* headerFont; 
	headerFont=nodeProperties->GetNameFont();
	if(headerFont!=NULL) m_ClientDC->SelectObject(headerFont);
	m_ClientDC->SetTextAlign(TA_LEFT | TA_TOP);
	TEXTMETRIC tm;
	m_ClientDC->GetTextMetrics(&tm);
	if(abs(userFieldHeight)>abs(tm.tmHeight)) diff=(abs(userFieldHeight)-abs(tm.tmHeight))/2;
	else diff=0;
	m_ClientDC->TextOut(textRect.left+nodeProperties->GetNameBorderSize()+50+imageSize.cx,
                        rect.top-nodeProperties->GetNameBorderSize()-diff,
						nodeName);
    m_ClientDC->RestoreDC(savedDC);	
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This private function designed to draw the rectangular node header - maybe the only one
// node element if no 
//----------------------------------------------------------------------------------------------
void CGraphView::DrawParalellogramHeader(CRect & rect,CString & nodeName,int nodeKind,CNodeProperties * nodeProperties) {
int   savedDC;
CSize imageSize;
CSize baseSize;
int   userFieldHeight;
int   diff;
CRect textRect;

    savedDC = m_ClientDC->SaveDC();

    m_ClientDC->SetMapMode(MM_HIMETRIC);
    m_ClientDC->SetBkMode(TRANSPARENT);

	// Name border will be solid with the thikness and color defined in the node
	// properties.
    CBrush borderBrush(nodeProperties->GetNameBorderColor());

	// We need to select the brush to fill the background
	CBrush background(nodeProperties->GetNameBackgroundColor());

	// This is internal rectangle height - we will use it to center.
	userFieldHeight = abs(rect.Height())-2*nodeProperties->GetNameBorderSize();


	// Need to correct border rectangle because of the frame thikness
	CRect borderRect;
	borderRect.left   = rect.left+nodeProperties->GetNameBorderSize()/2;
	borderRect.top    = rect.top-nodeProperties->GetNameBorderSize()/2;
	borderRect.right  = rect.right;
	borderRect.bottom = rect.bottom;


	// This will draw the name rectangle.
	POINT path[4];
	path[0].x=borderRect.left;
	path[0].y=borderRect.bottom;

	path[1].x=borderRect.left+abs(borderRect.top-borderRect.bottom)/2;
	path[1].y=borderRect.top;

	path[2].x=borderRect.right;
	path[2].y=borderRect.top;

	path[3].x=borderRect.right-abs(borderRect.top-borderRect.bottom)/2;
	path[3].y=borderRect.bottom;


	CRgn pathRgn;
	pathRgn.CreatePolygonRgn((POINT *)&path,4,WINDING);


	m_ClientDC->FillRgn(&pathRgn,&background);
	m_ClientDC->FrameRgn(&pathRgn,&borderBrush,nodeProperties->GetNameBorderSize(), 
				                       nodeProperties->GetNameBorderSize());

	// Only part of the vertex internal space can be used to draw text and image
	textRect=rect;
	textRect.left+=abs(rect.Height()/2);
	textRect.right-=abs(rect.Height()/2);

	// We need to place header bitmap, if any, into the header rectangle
	imageSize.cx=imageSize.cy=0;
	CBitmapList* bitmapList = nodeProperties->GetNameImageList();
	if(bitmapList && nodeKind>=0) {
		CBitmap* bitmap = bitmapList->GetBitmap(nodeKind);
		if(bitmap!=NULL) {
	        CBitmap* bitmap = bitmapList->GetBitmap(nodeKind);
            baseSize = bitmap->GetBitmapDimension();
		    imageSize=nodeProperties->GetNameImageSize();
	        BITMAPINFOHEADER dibHeader;
		    ZeroMemory(&dibHeader,sizeof(BITMAPINFOHEADER));
		    dibHeader.biSize=sizeof(BITMAPINFOHEADER);
		    GetDIBits(*GetWindowDC(),*bitmap,0,0,NULL,(LPBITMAPINFO)&dibHeader,DIB_RGB_COLORS);
		    BITMAPINFO* dib;
		    dib =  (BITMAPINFO *)new char [sizeof(BITMAPINFOHEADER)+
		    dibHeader.biSizeImage+
		    dibHeader.biClrUsed*sizeof(RGBQUAD)+5000];
            memcpy(dib,&dibHeader,sizeof(BITMAPINFOHEADER));
			CDC* pDC;
			pDC = GetDC();
		    GetDIBits(*pDC,*bitmap,0,dibHeader.biHeight,(char *)dib+sizeof(BITMAPINFOHEADER)+3000,dib,DIB_RGB_COLORS);
			ReleaseDC(pDC);
		    int itemHeight=abs(borderRect.Height());
            if(imageSize.cy<itemHeight)  diff=(abs(itemHeight)-abs(imageSize.cy))/2;
	        else diff = 0;
	        SIZE p;
		    p.cx=p.cy=1;
		    m_ClientDC->DPtoLP(&p);
		    ::StretchDIBits(*m_ClientDC,
		                textRect.left+nodeProperties->GetNameBorderSize(),
			            rect.top-nodeProperties->GetNameBorderSize()-diff+p.cy,
		                abs(imageSize.cx), 
						-abs(imageSize.cy),
						 0,
						 0,
						 dib->bmiHeader.biWidth,
						 dib->bmiHeader.biHeight,
						 (char *)dib+sizeof(BITMAPINFOHEADER)+3000,
						 dib,
						 DIB_RGB_COLORS,
						 SRCCOPY);
		    delete dib;
		}
	}



    // This will print node header text centered vertically
	m_ClientDC->SetTextColor(nodeProperties->GetNameTextColor());
	CFont* headerFont; 
	headerFont=nodeProperties->GetNameFont();
	if(headerFont!=NULL) m_ClientDC->SelectObject(headerFont);
	m_ClientDC->SetTextAlign(TA_LEFT | TA_TOP);
	TEXTMETRIC tm;
	m_ClientDC->GetTextMetrics(&tm);
	if(abs(userFieldHeight)>abs(tm.tmHeight)) diff=(abs(userFieldHeight)-abs(tm.tmHeight))/2;
	else diff=0;
	m_ClientDC->TextOut(textRect.left+nodeProperties->GetNameBorderSize()+50+imageSize.cx,
                        rect.top-nodeProperties->GetNameBorderSize()-diff,
						nodeName);
    m_ClientDC->RestoreDC(savedDC);	
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This private function designed to draw the rectangular node header - maybe the only one
// node element if no 
//----------------------------------------------------------------------------------------------
void CGraphView::DrawLoopHeader(CRect & rect,CString & nodeName,int nodeKind,CNodeProperties * nodeProperties) {

}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This private function designed to draw the rectangular node header - maybe the only one
// node element if no 
//----------------------------------------------------------------------------------------------
void CGraphView::DrawIfHeader(CRect & rect, CString & nodeName,int nodeKind, CNodeProperties * nodeProperties) {

}
//----------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------
// This private function designed to draw the rectangular node header - maybe the only one
// node element if no 
//----------------------------------------------------------------------------------------------
void CGraphView::DrawCircleHeader(CRect & rect, CString & nodeName, int nodeKind, CNodeProperties * nodeProperties) {

}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
// This private function designed to draw the rectangular node header - maybe the only one
// node element if no 
//----------------------------------------------------------------------------------------------
void CGraphView::DrawNodeItemsList(CRect & rect, CItemsList* list, CNodeProperties * nodeProperties) {
int savedDC;
int itemHeight;
register i;
int diff;
    
    if(list->GetCount()==0) return;

    savedDC = m_ClientDC->SaveDC();

    m_ClientDC->SetMapMode(MM_HIMETRIC);
    m_ClientDC->SetBkMode(TRANSPARENT);

	// Name border will be solid with the thikness and color defined in the node
	// properties.
    CPen borderPen(PS_SOLID,
		           nodeProperties->GetFieldsBorderSize(), 
				   nodeProperties->GetFieldsBorderColor());
    m_ClientDC->SelectObject(&borderPen);

	// We need to select the brush to draw the background.
	CBrush background(nodeProperties->GetFieldsBackgroundColor());
    m_ClientDC->SelectObject(&background);

	// Need to correct border rectangle because of the frame thikness
	CRect borderRect;
	borderRect.left=rect.left+nodeProperties->GetFieldsBorderSize()/2;
	borderRect.right=rect.right-nodeProperties->GetFieldsBorderSize()/2;
	borderRect.bottom=rect.bottom+nodeProperties->GetFieldsBorderSize()/2;
	borderRect.top=rect.top-nodeProperties->GetFieldsBorderSize()/2;

	// This will draw the name rectangle.
	CPoint ellipse;
	ellipse.x=0;
	ellipse.y=0;
	m_ClientDC->RoundRect(borderRect,ellipse);

	itemHeight=(rect.top-rect.bottom)/list->GetCount();
	int userFieldHeight = itemHeight-2*nodeProperties->GetFieldsBorderSize();

	// This will draw fields separators
	POSITION p = list->GetHeadPosition();
	for(i=0;i<list->GetCount()-1;i++) {
	    m_ClientDC->MoveTo(rect.left,rect.top-((i+1)*itemHeight));
	    m_ClientDC->LineTo(rect.right,rect.top-((i+1)*itemHeight));
		list->GetNext(p);
	}

	// This will draw the fields text + field type icon
	     // Need  to select text color from node properties
	m_ClientDC->SetTextColor(nodeProperties->GetFieldsTextColor());
	     // Need to select node font, if any.
	CFont* fieldsFont; 
	fieldsFont=nodeProperties->GetFieldsFont();
	if(fieldsFont!=NULL) m_ClientDC->SelectObject(fieldsFont);

	// Text alignment
    m_ClientDC->SetTextAlign(TA_LEFT | TA_TOP);
    
	// Will print fields names one by one + field type icon
    p = list->GetHeadPosition();
	for(i=0;i<list->GetCount();i++) {
		// We need to calculate the icon size and draw it properly first.
		if(nodeProperties->GetFieldsImageList()!=NULL) {
		    CSize imageSize;
		    CSize baseSize;
			CDC compDC;

			// We need to extract the proper image from the image list
			CBitmap* bitmap = nodeProperties->GetFieldsImageList()->GetBitmap(list->GetAt(p).m_FieldType);
			if(bitmap!=NULL) {
			     baseSize = bitmap->GetBitmapDimension();
				 imageSize = nodeProperties->GetFieldsImageSize();

				 BITMAPINFOHEADER dibHeader;
				 ZeroMemory(&dibHeader,sizeof(BITMAPINFOHEADER));
				 dibHeader.biSize=sizeof(BITMAPINFOHEADER);
				 CDC* pDC;
				 pDC=GetDC();
				 GetDIBits(*pDC,*bitmap,0,0,NULL,(LPBITMAPINFO)&dibHeader,DIB_RGB_COLORS);
		  
				 BITMAPINFO* dib;
				 dib =  (BITMAPINFO *)new char [sizeof(BITMAPINFOHEADER)+
					              dibHeader.biSizeImage+
								  dibHeader.biClrUsed*sizeof(RGBQUAD)+5000];
                 memcpy(dib,&dibHeader,sizeof(BITMAPINFOHEADER));

				 GetDIBits(*pDC,*bitmap,0,dibHeader.biHeight,(char *)dib+sizeof(BITMAPINFOHEADER)+3000,dib,DIB_RGB_COLORS);
				 ReleaseDC(pDC);

			     if(imageSize.cy<itemHeight)  diff=(abs(itemHeight)-abs(imageSize.cy))/2;
			     else diff = 0;
	             SIZE p;
		         p.cx=p.cy=1;
		         m_ClientDC->DPtoLP(&p);
				 ::StretchDIBits(*m_ClientDC,
					             rect.left+nodeProperties->GetFieldsBorderSize()+50,
			                     rect.top-nodeProperties->GetFieldsBorderSize()-i*itemHeight-diff+p.cy,
		                         abs(imageSize.cx), 
							     -abs(imageSize.cy),
								 0,
								 0,
								 dib->bmiHeader.biWidth,
								 dib->bmiHeader.biHeight,
								 (char *)dib+sizeof(BITMAPINFOHEADER)+3000,
								 dib,
								 DIB_RGB_COLORS,
								 SRCCOPY);
				 delete dib;
			} else {
				imageSize.cx=imageSize.cy=0;
			}

	        TEXTMETRIC tm;
	        m_ClientDC->GetTextMetrics(&tm);
	        if(abs(userFieldHeight)>abs(tm.tmHeight)) diff=(abs(userFieldHeight)-abs(tm.tmHeight))/2;
	        else diff=0;
	  
	        m_ClientDC->TextOut(rect.left+nodeProperties->GetFieldsBorderSize()+100+imageSize.cx,
			                    rect.top-i*itemHeight-nodeProperties->GetFieldsBorderSize()-diff,
							    list->GetAt(p).m_FieldName);
		} else {

	        TEXTMETRIC tm;
	        m_ClientDC->GetTextMetrics(&tm);
	        if(abs(userFieldHeight)>abs(tm.tmHeight)) diff=(abs(userFieldHeight)-abs(tm.tmHeight))/2;
	        else diff=0;

	        m_ClientDC->TextOut(rect.left+nodeProperties->GetFieldsBorderSize()+100,
			                    rect.top-i*itemHeight-nodeProperties->GetFieldsBorderSize()-diff,
							    list->GetAt(p).m_FieldName);
		}
		list->GetNext(p);
	}
    m_ClientDC->RestoreDC(savedDC);	

}
//----------------------------------------------------------------------------------------------



BOOL CGraphView::OnPreparePrinting(CPrintInfo* pInfo) {
	return DoPreparePrinting(pInfo);
	
}


void CGraphView::OnPrintPreview()  {
	CScrollView::OnFilePrintPreview();
}

void CGraphView::OnFilePrint()  {
	CScrollView::OnFilePrint();
}

//----------------------------------------------------------------------------------------------------
// This function is called by framework every time the framework needs to display document page.
// When it's displaing page on the screen, it passes no CPrintInfo class (==NULL), if it is printing
// document it will pass CPrintInfo class that contains all page information includong printing page
// number. We will use this function to set origins to fit to the appropriate page and to set clipping
// rectangle as well.
//----------------------------------------------------------------------------------------------------
void CGraphView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)  {
CPoint origin;

	CScrollView::OnPrepareDC(pDC, pInfo);
	// We need to set origins only if we are printing or previewing.
	// On the screen the CScrollView will do this for you asking
	// the scrollers positions
	if(pInfo!=NULL) {
		// Initializing this DC with the defaults
	    m_ClientDC=InitDC(pDC,FALSE);

        // Layout will be performed on this DC 
	    GraphLayout();

		// Calculating client rectangle size
		CRect cliPage = GetPageClientSpace(pDC);
		CSize cliSize;
		cliSize.cx=abs(cliPage.Width());
        cliSize.cy=abs(cliPage.Height());

		// calculating needed rows 
		int rows = CalculateNeededRows(cliSize);
		int cols = CalculateNeededColumns(cliSize);

        // calculating current row and column
		int curCol=(pInfo->m_nCurPage-1)%cols;
		int curRow=(pInfo->m_nCurPage-1)/cols;
       
        // Calculating and setting the page origin
	    origin.x =-(cliSize.cx-2*XMARGIN)*curCol+XMARGIN;
	    origin.y =(cliSize.cy-4*YMARGIN)*curRow-2*YMARGIN;
        m_ClientDC->LPtoDP(&origin);
        m_ClientDC->SetViewportOrg(origin.x,origin.y);

		// Testing if it's time to quit printing
		int pages=rows*cols;
		if((int)(pInfo->m_nCurPage-1)<(int)pages)
		    pInfo->m_bContinuePrinting=TRUE;
	    else
		    pInfo->m_bContinuePrinting=FALSE;
	} else {
		// Initializing this DC with the defaults
	    m_ClientDC=InitDC(pDC,FALSE);

	}

}
//----------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------
// This private function will calculate printer page client area depending on the current page size,
// printer properties and frame rectangles. It will give results in HIMETRIC relative to the 
// left-bottom corner of the page if logical flag is set, and it will return results in physical dots
// relative to the upper-left corner of the page if logical flag is FALSE;
//----------------------------------------------------------------------------------------------------
CRect CGraphView::GetPageClientSpace(CDC * pDC, BOOL logical) {
CRect pageRect;

	 int xlogic=pDC->GetDeviceCaps(HORZSIZE);
	 int ylogic=pDC->GetDeviceCaps(VERTSIZE);
	 int xdots=pDC->GetDeviceCaps(HORZRES);
     int ydots=pDC->GetDeviceCaps(VERTRES);
	 int xDPI=pDC->GetDeviceCaps(LOGPIXELSX);
	 int yDPI=pDC->GetDeviceCaps(LOGPIXELSY);

     if(logical==TRUE) {
	     pageRect.left=0;
	     pageRect.bottom=0;
	     pageRect.right=xlogic*100;
	     pageRect.top=ylogic*100;
	 } else {
	     pageRect.left=0;
	     pageRect.top=0;
	     pageRect.right=xdots;
	     pageRect.bottom=ydots;
	 }
	 return pageRect;
}
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
// This function vill calculate amount of rows to cover complete document. It needs the size of the 
// client arear on the page and it uses the m_GraphSize member variable to find out the complete 
// document size. It works in HIMETRIC.
//----------------------------------------------------------------------------------------------------
int CGraphView::CalculateNeededRows(CSize & pageSize) {
	return (m_GraphSize.cy+(pageSize.cy-4*YMARGIN-100))/(pageSize.cy-4*YMARGIN);

}
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
// This function vill calculate amount of colimns to cover complete document. It needs the size of the 
// client arear on the page and it uses the m_GraphSize member variable to find out the complete 
// document size. It works in HIMETRIC.
//----------------------------------------------------------------------------------------------------
int CGraphView::CalculateNeededColumns(CSize & pageSize) {
	return (m_GraphSize.cx+(pageSize.cx-2*XMARGIN-100))/(pageSize.cx-2*XMARGIN);

}
//----------------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------------
// This private function will draw the frame in print or pront preview mode. The frame will
// consist of two frames: one external solid frame to bound the visible arear and one
// dotted frame which bound the graph drawing arear.
// Four titles will be drawn : left-top will show the inage name, right-top will show
// current page and amount of pages, bottom-left will show the trademark and bottom-right
// will show thetime the document was printed.
//-----------------------------------------------------------------------------------------------------
void CGraphView::DrawPrintFrame(CDC * pDC,CPrintInfo* pInfo) {
CFont textFont;


    // Calculating client rectangle size
	CRect cliPage = GetPageClientSpace(pDC);
	CSize cliSize;
	cliSize.cx=abs(cliPage.Width());
    cliSize.cy=abs(cliPage.Height());
    // calculating needed rows 
	int rows = CalculateNeededRows(cliSize);
	int cols = CalculateNeededColumns(cliSize);
    // calculating current row and column
	int curCol=(pInfo->m_nCurPage-1)%cols;
	int curRow=(pInfo->m_nCurPage-1)/cols;
	CPoint origin;
    origin.x =-(-(cliSize.cx-2*XMARGIN)*curCol+XMARGIN);
    origin.y =-((cliSize.cy-4*YMARGIN)*curRow-2*YMARGIN);

	// External solid rectangle position calculation
    CRect externalFrame;
	externalFrame.left   = origin.x;
	externalFrame.right  = origin.x+cliSize.cx;
	externalFrame.top    = origin.y-YMARGIN;
	externalFrame.bottom = origin.y-cliSize.cy+YMARGIN;

	// Internal dotted rectangle position calculation
    CRect internalFrame;
	internalFrame.left    = externalFrame.left+XMARGIN;
	internalFrame.right   = externalFrame.right-XMARGIN;
	internalFrame.top     = externalFrame.top-YMARGIN;
	internalFrame.bottom  = externalFrame.bottom+YMARGIN;

    // Calculating the pages numbers on which this page will continue
	int topPage;
	int bottomPage;
	int leftPage;
	int rightPage;
	if(curRow==0) topPage=0;
	else topPage=(curRow-1)*rows+curCol+1;
	if(curRow==(rows-1)) bottomPage=0;
    else bottomPage=(curRow+1)*rows+curCol+1;
	if(curCol==0) leftPage=0;
	else leftPage=curRow*rows+(curCol-1)+1;
	if(curCol==(cols-1)) rightPage=0;
	else rightPage=curRow*rows+(curCol+1)+1;

	// Printing the document header - document name
	textFont.CreateFont(300,0,0,0,0,FALSE,FALSE,FALSE,0,0,0,0,0,"Courrier New");
	pDC->SelectObject(&textFont);
	pDC->SetTextColor(RGB(0,0,0));
	pDC->SetTextAlign(TA_LEFT | TA_TOP);
	pDC->TextOut(externalFrame.left,origin.y-50,"Call Tree");

	// Printing document header - page and pages amount
	CString pageLabel;
	pageLabel.Format("Page %u of %u",pInfo->m_nCurPage,rows*cols);
	CSize textSize=pDC->GetTextExtent(pageLabel) ;
	pDC->TextOut(externalFrame.right-textSize.cx,origin.y-50,pageLabel);

	// Printing document footer - date and time
	CTime curTime = CTime::GetCurrentTime();
	CString dateStr;
	dateStr.Format("%u/%u/%u",curTime.GetMonth(),curTime.GetDay(),curTime.GetYear());
	CSize dateSize=pDC->GetTextExtent(dateStr) ;
	pDC->TextOut(externalFrame.right-dateSize.cx,externalFrame.bottom-50,dateStr);


    // Printing the document footer - the trademark
	pDC->TextOut(externalFrame.left,externalFrame.bottom-50,"DIScover Browser");
	

    // printing where to continue on the top edge 
    CFont capFont;
	capFont.CreateFont(300,0,0,0,0,FALSE,FALSE,FALSE,0,0,0,0,0,"Times New Roman");
	pDC->SelectObject(&capFont);
    CString direct;
	if(topPage!=0) {
		direct.Format("Continued on page %u",topPage);
	    pDC->SetTextAlign(TA_CENTER | TA_TOP);
	    pDC->TextOut(origin.x+cliSize.cx/2,externalFrame.top-50,direct);
	}
    // printing where to continue on the bottom edge 
	if(bottomPage!=0) {
		direct.Format("Continued on page %u",bottomPage);
	    pDC->SetTextAlign(TA_CENTER | TA_TOP);
	    pDC->TextOut(origin.x+cliSize.cx/2,internalFrame.bottom-50,direct);
	}
    // printing where to continue on the left edge 
	CFont verFont;
	verFont.CreateFont(300,0,2700,2700,0,FALSE,FALSE,FALSE,0,0,0,0,0,"Times New Roman");
	pDC->SelectObject(&verFont);
	if(leftPage!=0) {
		direct.Format("Continued on page %u",leftPage);
	    pDC->SetTextAlign(TA_CENTER | TA_TOP);
	    pDC->TextOut(externalFrame.left+50,origin.y-cliSize.cy/2,direct);
	}
    // printing where to continue on the right edge 
	CFont ver2Font;
	ver2Font.CreateFont(300,0,900,900,0,FALSE,FALSE,FALSE,0,0,0,0,0,"Times New Roman");
	pDC->SelectObject(&ver2Font);
	if(rightPage!=0) {
		direct.Format("Continued on page %u",rightPage);
	    pDC->SetTextAlign(TA_CENTER | TA_TOP);
	    pDC->TextOut(externalFrame.right-50,
			         origin.y-cliSize.cy/2,direct);
	}

	// Internal frame drawing
    CPen internalPen(PS_DOT,5,RGB(0,0,0));
	pDC->SelectObject(&internalPen);
	pDC->MoveTo(internalFrame.left, internalFrame.top);
	pDC->LineTo(internalFrame.right,internalFrame.top);
	pDC->LineTo(internalFrame.right,internalFrame.bottom);
	pDC->LineTo(internalFrame.left, internalFrame.bottom);
	pDC->LineTo(internalFrame.left, internalFrame.top);

    // External frame drawing
    CPen externalPen(PS_SOLID,10,RGB(0,0,0));
	pDC->SelectObject(&externalPen);
	pDC->MoveTo(externalFrame.left,externalFrame.top);
	pDC->LineTo(externalFrame.right,externalFrame.top);
	pDC->LineTo(externalFrame.right,externalFrame.bottom);
	pDC->LineTo(externalFrame.left,externalFrame.bottom);
	pDC->LineTo(externalFrame.left,externalFrame.top);


	// No drawing outside internal frame
	internalFrame.left   -=50;
	internalFrame.right  +=50;
	internalFrame.top    +=50;
	internalFrame.bottom -=50;
    pDC->IntersectClipRect(&internalFrame);
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// This function is called by framework when printing or previewing document.
// It will draw page frame and then the framework will call common graph drawing
// function "OnDraw"
//----------------------------------------------------------------------------------
void CGraphView::OnPrint(CDC* pDC, CPrintInfo* pInfo)  {
	
	DrawPrintFrame(pDC,pInfo);
	CScrollView::OnPrint(pDC, pInfo);
}
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
// This callback runs when the window changes it's size.
// Will set scrolling sizes.
//----------------------------------------------------------------------------------
void CGraphView::OnSize(UINT nType, int cx, int cy)  {
CSize scrollSize;
	CScrollView::OnSize(nType, cx, cy);
    SetScrollSizes(MM_HIMETRIC,m_GraphSize);
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// This callback runs when user press the mouse on the grap view. It will 
// recalculate the mouse position and will pass tge call to the client graph.
//----------------------------------------------------------------------------------
void CGraphView::OnLButtonDown(UINT nFlags, CPoint point)  {
CPoint shift = GetScrollPosition();

    m_ClientDC = InitDC(GetDC());
	m_ClientDC->DPtoLP(&point);
    if(m_ClientGraph->SetFocus(point.x,point.y)==TRUE) {
		GraphLayout();
	    Invalidate(TRUE);
	}
	ReleaseDC(m_ClientDC);
}
//----------------------------------------------------------------------------------



//----------------------------------------------------------------------------------
// This function will translate mouse left button doubleclick to the graph
//----------------------------------------------------------------------------------
void CGraphView::OnLButtonDblClk(UINT nFlags, CPoint point)  {


    m_ClientDC = InitDC(GetDC());
	m_ClientDC->DPtoLP(&point);
    m_ClientGraph->SetFocus(point.x,point.y);
	CDisGraphNode* focus = m_ClientGraph->GetFocusedNode();
	if(focus!=NULL) {
        if(focus->AreChildrenCreated()==FALSE) {
            focus->CreateChildNodes();
		} else {
		    if(focus->IsCollapsed()==TRUE) focus->Expand();
		    else                           focus->Collapse();

		}
		GraphLayout();
	    ReleaseDC(m_ClientDC);
	    Invalidate(TRUE);
	}
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// This function will pop-up graph properties dialog.
// It will run every time the user press menu bar or tool button which
// generates IDC_GRAPHPROP command.
//----------------------------------------------------------------------------------
void CGraphView::OnViewProperties()  {
register i;
CGraphPropertiesSheet prop("Graph properties",this,0);

    m_Settings->DataExchange(this,CGraphSettings::Get);
	prop.SetParameters(m_Settings);

    if(prop.DoModal()==IDOK) {
        m_Settings->DataExchange(this,CGraphSettings::Set);
		// Settings saving
	    char buf[1024];
	    CString settingsPath;
        if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
            settingsPath=buf;
            settingsPath+="/Settings/";
		} else {
            settingsPath="C:/Discover/Settings/";
		}
		CString settingsName;
		BOOL trigger = FALSE;
		for(i=m_SettingsFile.GetLength()-1;i>=0;i--) {
			if(m_SettingsFile[i]=='.') {
				trigger=TRUE;
				continue;
			}
			if(m_SettingsFile[i]=='\\' || m_SettingsFile[i]=='/') break;
			if(trigger==TRUE) settingsName=CString(m_SettingsFile[i])+settingsName;			
		}
		settingsName+=".dat";
        settingsPath+=settingsName;
        CFile file;
	    if(file.Open(settingsPath,CFile::modeWrite | CFile::modeCreate)==TRUE) {
		   CArchive ar(&file,CArchive::store);
		   ar << m_Settings;
		}
        m_ClientDC = InitDC(GetDC());
		m_ClientGraph->Refresh();
	    m_ClientGraph->PortAllEdges();
		GraphLayout();
	    Invalidate(TRUE);
	}
	
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// This public function will return the pointer to the client graph
//----------------------------------------------------------------------------------
CDisGraph* CGraphView::GetGraph() {
	return m_ClientGraph;

}
//----------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// This function will run graph layout and will transform the graph to the HIMETRIC coordinate
// system with the top-left corner at (0,0) positive X and negative Y
//------------------------------------------------------------------------------------------------
void CGraphView::GraphLayout() {
	AssignBitmaps();
	m_ClientGraph->RemoveRelations(GRAPH_CONTAINS_INSTANCE_OF);
	m_ClientGraph->CreateRelation(GRAPH_CONTAINS_INSTANCE_OF,
		                          m_Relations[0],
								  m_Relations[0],
								  m_Relations[0]);
	m_ClientGraph->RemoveRelations(GRAPH_CONTAINED_IN);
	m_ClientGraph->CreateRelation(GRAPH_CONTAINED_IN,
		                          m_Relations[1],
								  m_Relations[1],
								  m_Relations[1]);
	m_ClientGraph->RemoveRelations(GRAPH_POINTS_TO);
	m_ClientGraph->CreateRelation(GRAPH_POINTS_TO,
		                          m_Relations[0],
								  m_Relations[0],
								  m_Relations[0]);
	m_ClientGraph->RemoveRelations(GRAPH_HAVE_ARGUMENTS);
	m_ClientGraph->CreateRelation(GRAPH_HAVE_ARGUMENTS,
		                          m_Relations[3],
								  m_Relations[3],
								  m_Relations[3]);

	// Calculates the nodes sizes for this DC (m_ClientDC)
    m_ClientGraph->DetermineNodeSizes();
	// Hierarchical layout style will be used
	m_ClientGraph->layoutStyle(TS_HIERARCHICAL);
	// layout itself
	m_ClientGraph->layout();
    // Calculate the current size and center of the client graph
	TSRect* rect = m_ClientGraph->pBoundingRect();
    CSize size;
    size.cx=rect->width();
    size.cy=rect->height();
	CPoint center;
	center.x=rect->x();
	center.y=rect->y();
	// Move it to start at 0,0 goting +x and -y
	m_ClientGraph->move(size.cx/2-center.x+X_BORDER,-(center.y+size.cy/2+Y_BORDER));
    size.cx+=2*X_BORDER;
    size.cy+=2*Y_BORDER;
	m_GraphSize=size;
	// Setting the scroll sizes
    SetScrollSizes(MM_HIMETRIC,m_GraphSize);
}
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// This function will set default values for the DC. It will use them when no proper values
// specified in the node or edge properties
//------------------------------------------------------------------------------------------------
CDC* CGraphView::InitDC(CDC *pDC,BOOL initOrigin) {
CFont defFont;
CPoint origin;

    pDC->SetMapMode(MM_HIMETRIC);
    if(m_SelectedNode->GetNameFont()!=NULL)
        pDC->SelectObject(m_SelectedNode->GetNameFont());
	if(initOrigin) {
        CPoint shift = GetScrollPosition();
		shift.x=-shift.x;
		shift.y=-shift.y;
	    pDC->LPtoDP(&shift);
	    pDC->SetViewportOrg(shift);
	}
	return pDC;
}
//------------------------------------------------------------------------------------------------



void CGraphView::OnRButtonDblClk(UINT nFlags, CPoint point)  {
    m_ClientDC = InitDC(GetDC());
	m_ClientDC->DPtoLP(&point);
    m_ClientGraph->SetFocus(point.x,point.y);
	CDisGraphNode* focus = m_ClientGraph->GetFocusedNode();
	if(focus!=NULL) {
        if(focus->AreParentsCreated()==FALSE) {
            focus->CreateParentNodes();
		} 
		GraphLayout();
	    ReleaseDC(m_ClientDC);
	    Invalidate(TRUE);
	}
	
}


//------------------------------------------------------------------------------------
// This function will draw the node port at the selected size with the selected
// open/close status.
//------------------------------------------------------------------------------------
void CGraphView::DrawNodePort(CRect& nodeRect,int headerHeight,int side, BOOL status) {
CPoint topLeft;
CSize headerSize;

    switch(side) {
	    case LEFT  : topLeft.x=nodeRect.left-300;
			         topLeft.y=nodeRect.top-headerHeight/2-150;
					 break;
	    case RIGHT : topLeft.x=nodeRect.right;
			         topLeft.y=nodeRect.top-headerHeight/2-150;
					 break;
	    case TOP   : topLeft.x=nodeRect.left+nodeRect.Width()/2-150;
			         topLeft.y=nodeRect.top;
					 break;
	    case BOTTOM: topLeft.x=nodeRect.left+nodeRect.Width()/2-150;
			         topLeft.y=nodeRect.bottom-300;
					 break;
		default:    return;
	}
	POSITION pos = m_PortImages.GetHeadPosition();
	if(status==FALSE) m_PortImages.GetNext(pos);
    if(pos!=NULL) {
        CBitmap* bitmap = m_PortImages.GetAt(pos);

	    BITMAPINFOHEADER dibHeader;
		ZeroMemory(&dibHeader,sizeof(BITMAPINFOHEADER));
		dibHeader.biSize=sizeof(BITMAPINFOHEADER);
		GetDIBits(*m_ClientDC,*bitmap,0,0,NULL,(LPBITMAPINFO)&dibHeader,DIB_RGB_COLORS);
		BITMAPINFO* dib;
		dib =  (BITMAPINFO *)new char [sizeof(BITMAPINFOHEADER)+
		                               dibHeader.biSizeImage+
		                               dibHeader.biClrUsed*sizeof(RGBQUAD)+5000];
        memcpy(dib,&dibHeader,sizeof(BITMAPINFOHEADER));
		CDC* pDC = GetDC();
		GetDIBits(*pDC,*bitmap,0,dibHeader.biHeight,(char *)dib+sizeof(BITMAPINFOHEADER)+3000,dib,DIB_RGB_COLORS);
		ReleaseDC(pDC);

		::StretchDIBits(*m_ClientDC,
		                 topLeft.x,
			             topLeft.y,
		                 300, 
						 300,
						 0,
						 0,
						 dib->bmiHeader.biWidth,
						 dib->bmiHeader.biHeight,
						 (char *)dib+sizeof(BITMAPINFOHEADER)+3000,
						 dib,
						 DIB_RGB_COLORS,
						 SRCCOPY);
		delete dib;
	}
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This public function will make dynamic data exchange between client graph and
// graph settings class (this class). "target" parameter defines which graph
// graph will be used to get/set parameters, "method" parameter defines direction
// of the exchange. Can be "Get" to extract data from the graph and "Set" to set
// graph properties.
//------------------------------------------------------------------------------------
void CGraphSettings::DataExchange(CGraphView *target, ExchangeMethod method) {
	switch(method) {
	    case CGraphSettings::Get  :  GetGraphProperties(target);
			                         GetNodeProperties(target);
									 GetEdgeProperties(target);
									 if(m_ViewType==INHERITANCE || m_ViewType==ERD)
									     GetInheritanceProperties(target);
									 if(m_ViewType==ERD)
									     GetRelationsProperties(target);
									 break;
	    case CGraphSettings::Set  :  SetGraphProperties(target);
			                         SetNodeProperties(target);
									 SetEdgeProperties(target);
									 if(m_ViewType==ERD)
									      SetRelationsProperties(target);
									 break;
	}

}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This private function will fill this class with all client graph properties.
//------------------------------------------------------------------------------------
void CGraphSettings::GetGraphProperties(CGraphView *graphView) {
int orientation;

    // Extracting graph orientation and graph nodes justification from the target graph
    orientation = graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelOrientation();
    switch(orientation) {
	    case TS_LEFT_TO_RIGHT : m_Orientation = CGraphSettings::LeftToRight;
			                    switch(graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification()) {
								   case TS_LEFT_JUSTIFIED :   m_Justification = CGraphSettings::Start;
									                          break;
								   case TS_CENTER_JUSTIFIED:  m_Justification = CGraphSettings::Center;
									                          break;
								   case TS_RIGHT_JUSTIFIED  : m_Justification = CGraphSettings::End;
									                          break;
								}
		                        break;
	    case TS_RIGHT_TO_LEFT : m_Orientation = CGraphSettings::RightToLeft;
			                    switch(graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification()) {
								   case TS_RIGHT_JUSTIFIED : m_Justification = CGraphSettings::Start;
									                         break;
								   case TS_CENTER_JUSTIFIED: m_Justification = CGraphSettings::Center;
									                         break;
								   case TS_LEFT_JUSTIFIED  : m_Justification = CGraphSettings::End;
									                         break;
								}
		                        break;
	    case TS_TOP_TO_BOTTOM : m_Orientation = CGraphSettings::TopToBottom;
			                    switch(graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification()) {
								   case TS_TOP_JUSTIFIED   : m_Justification = CGraphSettings::Start;
									                         break;
								   case TS_CENTER_JUSTIFIED: m_Justification = CGraphSettings::Center;
									                         break;
								   case TS_BOTTOM_JUSTIFIED: m_Justification = CGraphSettings::End;
									                         break;
								}
		                        break;
	    case TS_BOTTOM_TO_TOP : m_Orientation = CGraphSettings::BottomToTop;
			                    switch(graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification()) {
								   case TS_BOTTOM_JUSTIFIED: m_Justification = CGraphSettings::Start;
									                         break;
								   case TS_CENTER_JUSTIFIED: m_Justification = CGraphSettings::Center;
									                         break;
								   case TS_TOP_JUSTIFIED:    m_Justification = CGraphSettings::End;
									                         break;
								}
		                        break;
	}
    // Edge routing style - can be orthogonal or direct
	m_Routing.m_Orthogonal     = graphView->GetGraph()->pHDigraph()->pHTailor()->orthogonalRouting();
	// Edge mergin style if orthogonal routing style is on. No effect if direct routhing.
	m_Routing.m_Merge          = graphView->GetGraph()->pHDigraph()->pHTailor()->orthogonalChannelMerging();
	// Tree levels variable spacing. Can be enabled or disabled
	m_Spacing.m_EnableVatiable = graphView->GetGraph()->pHDigraph()->pHTailor()->variableLevelSpacing();
	// Graph node spacing - the empty space around any node
    m_Spacing.m_NodeSpacing    = graphView->GetGraph()->pHDigraph()->pHTailor()->hnodeSpacing();
	// Graph level spacing - the empty space between tree levels
	m_Spacing.m_LevelSpacing   = graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelSpacing();
	// Graph row spacing - the empty space between node rows (the same level, vertical orientation)
    m_Spacing.m_RowSpacing     = graphView->GetGraph()->pHDigraph()->pHTailor()->rowSpacing();
	// Graph column spacing - the empty space between node columns (same level, horizontal orientation )
	m_Spacing.m_ColumnSpacing  = graphView->GetGraph()->pHDigraph()->pHTailor()->columnSpacing();

}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This private function will fill this class with all client graph node properties.
//------------------------------------------------------------------------------------
void CGraphSettings::GetNodeProperties(CGraphView *graphView) {
COLORREF nodeBackground;
COLORREF nodeFrame;


    // Extracting current node font. It will be the same for unselected, 
    // selected and focused nodes
	if(graphView->GetUnselectedNodeProp()->GetNameFont()!=NULL) {
		graphView->GetUnselectedNodeProp()->GetNameFont()->GetLogFont(&m_NodeFont);
	} 

    // Extracting node frame thickness. It will be the same for unselected, 
    // selected and focused nodes
    if(graphView->GetUnselectedNodeProp()->GetNameBorderSize()<=20) 
		m_BorderThickness=CGraphSettings::Thin;
	else
		if(graphView->GetUnselectedNodeProp()->GetNameBorderSize()<=50)
		    m_BorderThickness=CGraphSettings::Medium;
		    else 
		        m_BorderThickness=CGraphSettings::Thick;
    
    //==== Extracting unselected node parameters
    // Extracting node header background color
	nodeBackground = graphView->GetUnselectedNodeProp()->GetNameBackgroundColor();
	m_NodeBackground[UNSELECTED].m_Red = (unsigned char)(nodeBackground & 255);
	m_NodeBackground[UNSELECTED].m_Green=(unsigned char)((nodeBackground>>8) & 255);
	m_NodeBackground[UNSELECTED].m_Blue=(unsigned char)((nodeBackground>>16) & 255);
	// Extracting node header frame color
	nodeFrame = graphView->GetUnselectedNodeProp()->GetNameBorderColor();
	m_NodeBorder[UNSELECTED].m_Red = (unsigned char)(nodeFrame & 255);
	m_NodeBorder[UNSELECTED].m_Green=(unsigned char)((nodeFrame>>8) & 255);
	m_NodeBorder[UNSELECTED].m_Blue=(unsigned char)((nodeFrame>>16) & 255);

    //==== Extracting selected node parameters
    // Extracting node header background color
	nodeBackground = graphView->GetSelectedNodeProp()->GetNameBackgroundColor();
	m_NodeBackground[SELECTED].m_Red = (unsigned char)(nodeBackground & 255);
	m_NodeBackground[SELECTED].m_Green=(unsigned char)((nodeBackground>>8) & 255);
	m_NodeBackground[SELECTED].m_Blue=(unsigned char)((nodeBackground>>16) & 255);
	// Extracting node header frame color
	nodeFrame = graphView->GetSelectedNodeProp()->GetNameBorderColor();
	m_NodeBorder[SELECTED].m_Red = (unsigned char)(nodeFrame & 255);
	m_NodeBorder[SELECTED].m_Green=(unsigned char)((nodeFrame>>8) & 255);
	m_NodeBorder[SELECTED].m_Blue=(unsigned char)((nodeFrame>>16) & 255);

    //==== Extracting focused node parameters
    // Extracting node header background color
	nodeBackground = graphView->GetFocusedNodeProp()->GetNameBackgroundColor();
	m_NodeBackground[FOCUSED].m_Red = (unsigned char)(nodeBackground & 255);
	m_NodeBackground[FOCUSED].m_Green=(unsigned char)((nodeBackground>>8) & 255);
	m_NodeBackground[FOCUSED].m_Blue=(unsigned char)((nodeBackground>>16) & 255);
	// Extracting node header frame color
	nodeFrame = graphView->GetFocusedNodeProp()->GetNameBorderColor();
	m_NodeBorder[FOCUSED].m_Red = (unsigned char)(nodeFrame & 255);
	m_NodeBorder[FOCUSED].m_Green=(unsigned char)((nodeFrame>>8) & 255);
	m_NodeBorder[FOCUSED].m_Blue=(unsigned char)((nodeFrame>>16) & 255);

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This private function will fill this class with all client graph edge properties.
//------------------------------------------------------------------------------------
void CGraphSettings::GetEdgeProperties(CGraphView *graphView) {

	COLORREF edgeColor;
	edgeColor = graphView->GetUnselectedEdgeProp()->GetEdgeColor();
	m_EdgeColor[UNSELECTED].m_Red = (unsigned char)(edgeColor & 255);
	m_EdgeColor[UNSELECTED].m_Green=(unsigned char)((edgeColor>>8) & 255);
	m_EdgeColor[UNSELECTED].m_Blue=(unsigned char)((edgeColor>>16) & 255);

	edgeColor = graphView->GetSelectedEdgeProp()->GetEdgeColor();
	m_EdgeColor[SELECTED].m_Red = (unsigned char)(edgeColor & 255);
	m_EdgeColor[SELECTED].m_Green=(unsigned char)((edgeColor>>8) & 255);
	m_EdgeColor[SELECTED].m_Blue=(unsigned char)((edgeColor>>16) & 255);

	edgeColor = graphView->GetFocusedEdgeProp()->GetEdgeColor();
	m_EdgeColor[FOCUSED].m_Red = (unsigned char)(edgeColor & 255);
	m_EdgeColor[FOCUSED].m_Green=(unsigned char)((edgeColor>>8) & 255);
	m_EdgeColor[FOCUSED].m_Blue=(unsigned char)((edgeColor>>16) & 255);


    if(graphView->GetUnselectedEdgeProp()->GetEdgeThikness()<=20) 
		m_EdgeThickness=CGraphSettings::Thin;
	else
		if(graphView->GetUnselectedEdgeProp()->GetEdgeThikness()<=50)
		    m_EdgeThickness=CGraphSettings::Medium;
		    else 
		        m_EdgeThickness=CGraphSettings::Thick;

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This private function will fill client graph with graph properties.
//------------------------------------------------------------------------------------
void CGraphSettings::SetGraphProperties(CGraphView *graphView) {

	// Setting graph orientation and node alignment
    switch(m_Orientation) {
		case CGraphSettings::LeftToRight : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelOrientation(TS_LEFT_TO_RIGHT);
		                                   switch(m_Justification) {
							                   case CGraphSettings::Start : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_LEFT_JUSTIFIED);;
			                                                                break;
		                                       case CGraphSettings::Center: graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_CENTER_JUSTIFIED);
			                                                                break;
		                                       case CGraphSettings::End   : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_RIGHT_JUSTIFIED);
			                                                                break;
										   }
			                               break;
	    case CGraphSettings::RightToLeft : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelOrientation(TS_RIGHT_TO_LEFT);
		                                   switch(m_Justification) {
		                                      case CGraphSettings::Start : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_RIGHT_JUSTIFIED);;
			                                                               break;
		                                      case CGraphSettings::Center: graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_CENTER_JUSTIFIED);
			                                                               break;
		                                      case CGraphSettings::End :   graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_LEFT_JUSTIFIED);
			                                                               break;
										   }
			                               break;
		case CGraphSettings::TopToBottom : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelOrientation(TS_TOP_TO_BOTTOM);
		                                   switch(m_Justification) {
		                                      case CGraphSettings::Start : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_TOP_JUSTIFIED);
			                                                               break;
		                                      case CGraphSettings::Center: graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_CENTER_JUSTIFIED);
			                                                               break;
		                                      case CGraphSettings::End :   graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_BOTTOM_JUSTIFIED);
			                                                               break;
										   }
			                               break;
		case CGraphSettings::BottomToTop : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelOrientation(TS_BOTTOM_TO_TOP);
		                                   switch(m_Justification) {
		                                        case CGraphSettings::Start  : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_BOTTOM_JUSTIFIED);
			                                                                  break;
		                                        case CGraphSettings::Center : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_CENTER_JUSTIFIED);
			                                                                  break;
		                                         case CGraphSettings::End   : graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelJustification(TS_TOP_JUSTIFIED);
			                                                                  break;
										   }
			                               break;
	}
    // Edge routing style - can be orthogonal or direct
	graphView->GetGraph()->pHDigraph()->pHTailor()->orthogonalRouting(m_Routing.m_Orthogonal);
	// Edge mergin style if orthogonal routing style is on. No effect if direct routhing.
	graphView->GetGraph()->pHDigraph()->pHTailor()->orthogonalChannelMerging(m_Routing.m_Merge);
	// Tree levels variable spacing. Can be enabled or disabled
	graphView->GetGraph()->pHDigraph()->pHTailor()->variableLevelSpacing(m_Spacing.m_EnableVatiable);
	// Graph node spacing - the empty space around any node
	graphView->GetGraph()->pHDigraph()->pHTailor()->hnodeSpacing(m_Spacing.m_NodeSpacing);
	// Graph level spacing - the empty space between tree levels
	graphView->GetGraph()->pHDigraph()->pHTailor()->hlevelSpacing(m_Spacing.m_LevelSpacing);
	// Graph row spacing - the empty space between node rows (the same level, vertical orientation)
	graphView->GetGraph()->pHDigraph()->pHTailor()->rowSpacing(m_Spacing.m_RowSpacing);
	// Graph column spacing - the empty space between node columns (same level, horizontal orientation )
	graphView->GetGraph()->pHDigraph()->pHTailor()->columnSpacing(m_Spacing.m_ColumnSpacing);
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This private function will fill this  client graph node with its properties.
//------------------------------------------------------------------------------------
void CGraphSettings::SetNodeProperties(CGraphView *graphView) {

	// New node header font - old one will be deleted automatically while SetNameFont.
	// Font is the same for all node styles
    CFont* font;
    font = new CFont;
    font->CreateFontIndirect(&m_NodeFont);
    graphView->GetUnselectedNodeProp()->SetNameFont(font);
    font = new CFont;
    font->CreateFontIndirect(&m_NodeFont);
    graphView->GetSelectedNodeProp()->SetNameFont(font);
    font = new CFont;
    font->CreateFontIndirect(&m_NodeFont);
    graphView->GetFocusedNodeProp()->SetNameFont(font);


	// Setting the node header frame thickness
	if(m_BorderThickness==CGraphSettings::Thin) {
	      graphView->GetUnselectedNodeProp()->SetNameBorderSize(20);
	      graphView->GetSelectedNodeProp()->SetNameBorderSize(20);
	      graphView->GetFocusedNodeProp()->SetNameBorderSize(20);
	} else {
		if(m_BorderThickness==CGraphSettings::Medium) {
	           graphView->GetUnselectedNodeProp()->SetNameBorderSize(50);
	           graphView->GetSelectedNodeProp()->SetNameBorderSize(50);
	           graphView->GetFocusedNodeProp()->SetNameBorderSize(50);
		}  else {
			if(m_BorderThickness==CGraphSettings::Thick) {
	                graphView->GetUnselectedNodeProp()->SetNameBorderSize(100);
	                graphView->GetSelectedNodeProp()->SetNameBorderSize(100);
	                graphView->GetFocusedNodeProp()->SetNameBorderSize(100);
			}
		}
	}


    // === Unselected node colors
    // Node header background color
    graphView->GetUnselectedNodeProp()->SetNameBackgroundColor(RGB(
			                                       m_NodeBackground[UNSELECTED].m_Red,
												   m_NodeBackground[UNSELECTED].m_Green,
		                                           m_NodeBackground[UNSELECTED].m_Blue));
	// Node header frame color
    graphView->GetUnselectedNodeProp()->SetNameBorderColor(RGB(
			                                       m_NodeBorder[UNSELECTED].m_Red,
												   m_NodeBorder[UNSELECTED].m_Green,
		                                           m_NodeBorder[UNSELECTED].m_Blue));
	// === Selected node colors
    // Node header background color
    graphView->GetSelectedNodeProp()->SetNameBackgroundColor(RGB(
			                                       m_NodeBackground[SELECTED].m_Red,
												   m_NodeBackground[SELECTED].m_Green,
		                                           m_NodeBackground[SELECTED].m_Blue));
	// Node header frame color
    graphView->GetSelectedNodeProp()->SetNameBorderColor(RGB(
			                                       m_NodeBorder[SELECTED].m_Red,
												   m_NodeBorder[SELECTED].m_Green,
		                                           m_NodeBorder[SELECTED].m_Blue));
    // === Focused node colors
    // Node header background color
    graphView->GetFocusedNodeProp()->SetNameBackgroundColor(RGB(
			                                       m_NodeBackground[FOCUSED].m_Red,
												   m_NodeBackground[FOCUSED].m_Green,
		                                           m_NodeBackground[FOCUSED].m_Blue));
	// Node header frame color
    graphView->GetFocusedNodeProp()->SetNameBorderColor(RGB(
			                                       m_NodeBorder[FOCUSED].m_Red,
												   m_NodeBorder[FOCUSED].m_Green,
		                                           m_NodeBorder[FOCUSED].m_Blue));


}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This private function will fill this client graph edge with its properties.
//------------------------------------------------------------------------------------
void CGraphSettings::SetEdgeProperties(CGraphView *graphView) {
    if(m_EdgeThickness==CGraphSettings::Thin) {
		graphView->GetUnselectedEdgeProp()->SetEdgeThikness(20);
		graphView->GetSelectedEdgeProp()->SetEdgeThikness(20);
		graphView->GetFocusedEdgeProp()->SetEdgeThikness(20);
	} else {
	    if(m_EdgeThickness==CGraphSettings::Medium) {
		    graphView->GetUnselectedEdgeProp()->SetEdgeThikness(50);
		    graphView->GetUnselectedEdgeProp()->SetEdgeThikness(50);
		    graphView->GetUnselectedEdgeProp()->SetEdgeThikness(50);
		}  else {
		    if(m_EdgeThickness==CGraphSettings::Thick) {
		        graphView->GetUnselectedEdgeProp()->SetEdgeThikness(100);
		        graphView->GetSelectedEdgeProp()->SetEdgeThikness(100);
		        graphView->GetFocusedEdgeProp()->SetEdgeThikness(100);
			}
		}
	}
	graphView->GetUnselectedEdgeProp()->SetEdgeColor(RGB(m_EdgeColor[UNSELECTED].m_Red,
								                                     m_EdgeColor[UNSELECTED].m_Green,
		                                                             m_EdgeColor[UNSELECTED].m_Blue));
	graphView->GetSelectedEdgeProp()->SetEdgeColor(RGB(  m_EdgeColor[SELECTED].m_Red,
									                                 m_EdgeColor[SELECTED].m_Green,
		                                                             m_EdgeColor[SELECTED].m_Blue));
	graphView->GetFocusedEdgeProp()->SetEdgeColor(RGB(   m_EdgeColor[FOCUSED].m_Red,
									                                 m_EdgeColor[FOCUSED].m_Green,
		                                                             m_EdgeColor[FOCUSED].m_Blue));

}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function will access server to extract current values for the inheritance
// properties
//------------------------------------------------------------------------------------
void CGraphSettings::GetInheritanceProperties(CGraphView* graphView) {
CString res;

	res = graphView->GetGraph()->Evaluate(CString("InheritanceProperties get class_members 0"));
	if(res=="Show") m_Inheritance.m_ShowClassMethods=TRUE;
	else            m_Inheritance.m_ShowClassMethods=FALSE;

	res = graphView->GetGraph()->Evaluate(CString("InheritanceProperties get class_properties 0"));
	if(res=="Show") m_Inheritance.m_ShowClassProperties=TRUE;
	else            m_Inheritance.m_ShowClassProperties=FALSE;

	res = graphView->GetGraph()->Evaluate(CString("InheritanceProperties get public_members 0"));
	if(res=="Show") m_Inheritance.m_ShowPublicMembers=TRUE;
	else            m_Inheritance.m_ShowPublicMembers=FALSE;

	res = graphView->GetGraph()->Evaluate(CString("InheritanceProperties get protected_members 0"));
	if(res=="Show") m_Inheritance.m_ShowProtectedMembers=TRUE;
	else            m_Inheritance.m_ShowProtectedMembers=FALSE;

	res = graphView->GetGraph()->Evaluate(CString("InheritanceProperties get package_access_members 0"));
	if(res=="Show") m_Inheritance.m_ShowPackageAccessMembers=TRUE;
	else            m_Inheritance.m_ShowPackageAccessMembers=FALSE;

	res = graphView->GetGraph()->Evaluate(CString("InheritanceProperties get private_members 0"));
	if(res=="Show") m_Inheritance.m_ShowPrivateMembers=TRUE;
	else            m_Inheritance.m_ShowPrivateMembers=FALSE;

	res = graphView->GetGraph()->Evaluate(CString("InheritanceProperties get nameformat 0"));
	if(res=="Long") m_Inheritance.m_Name=NameType::Long;
	else            m_Inheritance.m_Name=NameType::Short;

	res = graphView->GetGraph()->Evaluate(CString("InheritanceProperties get sort 0"));
	if(res=="Name") m_Inheritance.m_Sort=SortType::ByName;
	else            m_Inheritance.m_Sort=SortType::ByAccess;

	res = graphView->GetGraph()->Evaluate(CString("InheritanceProperties get merge 0"));
	if(res=="Merge") m_Inheritance.m_Merge=TRUE;
	else             m_Inheritance.m_Merge=FALSE;

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Set relation edges properties using this class values
//------------------------------------------------------------------------------------
void CGraphSettings::SetRelationsProperties(CGraphView* graphView) {
int id;

    for(id=0;id<12;id++) {	

	    // Relation edge color
        graphView->GetRelationsProp(id)->SetEdgeColor(RGB(
			                                          m_Relations[id].m_Color.m_Red,
											          m_Relations[id].m_Color.m_Green,
		                                              m_Relations[id].m_Color.m_Blue)
												  );
	    // Relation edge thickness
	    if(m_Relations[id].m_Thickness==CGraphSettings::Thin)
              graphView->GetRelationsProp(id)->SetEdgeThikness(20);
	    if(m_Relations[id].m_Thickness==CGraphSettings::Medium)
              graphView->GetRelationsProp(id)->SetEdgeThikness(50);
	    if(m_Relations[id].m_Thickness==CGraphSettings::Thick)
              graphView->GetRelationsProp(id)->SetEdgeThikness(100);
	    // Relation edge style
        graphView->GetRelationsProp(id)->SetEdgeStyle(m_Relations[id].m_Style);
	}
}
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// Set relation edges properties using this class values
//------------------------------------------------------------------------------------
void CGraphSettings::GetRelationsProperties(CGraphView* graphView) {
int id;

    for(id=0;id<12;id++) {	
	    // Relation edge color
		m_Relations[id].m_Color.m_Red = graphView->GetRelationsProp(id)->GetEdgeColor()&255;
		m_Relations[id].m_Color.m_Green = (graphView->GetRelationsProp(id)->GetEdgeColor()>>8)&255;
		m_Relations[id].m_Color.m_Blue = (graphView->GetRelationsProp(id)->GetEdgeColor()>>16)&255;

	    // Relation edge thickness
		if(graphView->GetRelationsProp(id)->GetEdgeThikness()<=20)
			m_Relations[id].m_Thickness=CGraphSettings::Thin;
		else if(graphView->GetRelationsProp(id)->GetEdgeThikness()<=50)
			      m_Relations[id].m_Thickness=CGraphSettings::Medium;
	          else
			      m_Relations[id].m_Thickness=CGraphSettings::Thick;
	    // Relation edge style
        m_Relations[id].m_Style = graphView->GetRelationsProp(id)->GetEdgeStyle();
	}
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will access server to extract current values for the inheritance
// properties
//------------------------------------------------------------------------------------
void CGraphSettings::SetInheritanceProperties(CGraphView* graphView) {

	if(m_Inheritance.m_ShowClassMethods) 
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set class_members Show"));
	else           
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set class_members Hide"));

	if(m_Inheritance.m_ShowClassProperties) 
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set class_properties Show"));
	else           
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set class_properties Hide"));

	if(m_Inheritance.m_ShowPublicMembers) 
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set public_members Show"));
	else           
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set public_members Hide"));

	if(m_Inheritance.m_ShowProtectedMembers) 
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set protected_members Show"));
	else           
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set protected_members Hide"));

	if(m_Inheritance.m_ShowPackageAccessMembers) 
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set package_access_members Show"));
	else           
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set package_access_members Hide"));

	if(m_Inheritance.m_ShowPrivateMembers) 
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set private_members Show"));
	else           
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set private_members Hide"));

	if(m_Inheritance.m_Name==NameType::Long) 
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set nameformat Long"));
	else           
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set nameformat Short"));

	if(m_Inheritance.m_Sort==SortType::ByName) 
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set sort Name"));
	else           
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set sort Access"));

	if(m_Inheritance.m_Merge==TRUE) 
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set merge Merge"));
	else           
		graphView->GetGraph()->Evaluate(CString("InheritanceProperties set merge Divide"));
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will assign default values to all settings
//------------------------------------------------------------------------------------
void CGraphSettings::Default() {
	m_Orientation=CGraphSettings::TopToBottom;
	m_Justification=CGraphSettings::Start;
    m_BorderThickness=CGraphSettings::Medium;
    m_EdgeThickness = CGraphSettings::Medium;

	m_NodeBackground[0].m_Red    =255;
	m_NodeBackground[0].m_Green  =255;
	m_NodeBackground[0].m_Blue   =255;

	m_NodeBackground[1].m_Red    = 100;
	m_NodeBackground[1].m_Green  = 100;
	m_NodeBackground[1].m_Blue   = 255;

	m_NodeBackground[2].m_Red    = 255;
	m_NodeBackground[2].m_Green  = 255;
	m_NodeBackground[2].m_Blue   = 200;

	m_NodeBorder[0].m_Red    = 0;
	m_NodeBorder[0].m_Green  = 0;
	m_NodeBorder[0].m_Blue   = 0;

	m_NodeBorder[1].m_Red    = 0;
	m_NodeBorder[1].m_Green  = 0;
	m_NodeBorder[1].m_Blue   = 0;

    m_NodeBorder[2].m_Red   =0;
	m_NodeBorder[2].m_Green =0;
	m_NodeBorder[2].m_Blue  =0;

    m_EdgeColor[0].m_Red    =0;
    m_EdgeColor[0].m_Green  =0;
    m_EdgeColor[0].m_Blue   =0;

    m_EdgeColor[1].m_Red   = 0;
    m_EdgeColor[1].m_Green = 0;
    m_EdgeColor[1].m_Blue  = 200;


    m_EdgeColor[2].m_Red   = 200;
    m_EdgeColor[2].m_Green = 150;
    m_EdgeColor[2].m_Blue  = 0;

    m_Routing.m_Orthogonal = TRUE;
	m_Routing.m_Merge      = TRUE;

	m_Spacing.m_EnableVatiable = TRUE;
	m_Spacing.m_NodeSpacing    = 600;
	m_Spacing.m_LevelSpacing   = 600;
	m_Spacing.m_RowSpacing     = 600;
	m_Spacing.m_ColumnSpacing  = 600;

	m_NodeFont.lfHeight  = 400;
	m_NodeFont.lfWidth   = 0;
	m_NodeFont.lfEscapement =0;
    m_NodeFont.lfOrientation =0;
	m_NodeFont.lfWeight=0;
	m_NodeFont.lfItalic=0;
	m_NodeFont.lfUnderline=0;
    m_NodeFont.lfStrikeOut=0;
	m_NodeFont.lfCharSet=0;
	m_NodeFont.lfOutPrecision=0;
    m_NodeFont.lfClipPrecision=0;
	m_NodeFont.lfQuality=0;
	m_NodeFont.lfPitchAndFamily=0;
    strcpy(m_NodeFont.lfFaceName,"Arial");

   m_Inheritance.m_ShowClassMethods=TRUE;
   m_Inheritance.m_ShowClassProperties=TRUE;
   m_Inheritance.m_ShowPublicMembers=TRUE;
   m_Inheritance.m_ShowProtectedMembers=TRUE;
   m_Inheritance.m_ShowPackageAccessMembers=TRUE;
   m_Inheritance.m_ShowPrivateMembers=TRUE;
   m_Inheritance.m_Name= CGraphSettings::Short;
   m_Inheritance.m_Sort=CGraphSettings::ByName;
   m_Inheritance.m_Merge = FALSE;

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will work in the serialization procedure to load/save all settings
// in the archive.
//------------------------------------------------------------------------------------
void CGraphSettings::Serialize(CArchive& ar) {
   CObject::Serialize( ar );
   if( ar.IsStoring() ) {
	  ar << (int)m_Orientation;
	  ar << (int)m_Justification;

      ar<<(int)m_BorderThickness;
      ar<<(int)m_EdgeThickness;

	  ar<<m_NodeBackground[0].m_Red;
	  ar<<m_NodeBackground[0].m_Green;
	  ar<<m_NodeBackground[0].m_Blue;
	  ar<<m_NodeBackground[1].m_Red;
	  ar<<m_NodeBackground[1].m_Green;
	  ar<<m_NodeBackground[1].m_Blue;
	  ar<<m_NodeBackground[2].m_Red;
	  ar<<m_NodeBackground[2].m_Green;
	  ar<<m_NodeBackground[2].m_Blue;

	  ar<<m_NodeBorder[0].m_Red;
	  ar<<m_NodeBorder[0].m_Green;
	  ar<<m_NodeBorder[0].m_Blue;
	  ar<<m_NodeBorder[1].m_Red;
	  ar<<m_NodeBorder[1].m_Green;
	  ar<<m_NodeBorder[1].m_Blue;
	  ar<<m_NodeBorder[2].m_Red;
	  ar<<m_NodeBorder[2].m_Green;
	  ar<<m_NodeBorder[2].m_Blue;

      ar<<m_EdgeColor[0].m_Red;
      ar<<m_EdgeColor[0].m_Green;
      ar<<m_EdgeColor[0].m_Blue;
      ar<<m_EdgeColor[1].m_Red;
      ar<<m_EdgeColor[1].m_Green;
      ar<<m_EdgeColor[1].m_Blue;
      ar<<m_EdgeColor[2].m_Red;
      ar<<m_EdgeColor[2].m_Green;
      ar<<m_EdgeColor[2].m_Blue;

      ar<<m_Routing.m_Orthogonal;
	  ar<<m_Routing.m_Merge;

	  ar<<m_Spacing.m_EnableVatiable;
	  ar<<m_Spacing.m_NodeSpacing;
	  ar<<m_Spacing.m_LevelSpacing;
	  ar<<m_Spacing.m_RowSpacing;
	  ar<<m_Spacing.m_ColumnSpacing;

	  ar<<m_NodeFont.lfHeight;    
	  ar<<m_NodeFont.lfWidth;    
	  ar<<m_NodeFont.lfEscapement; 
      ar<<m_NodeFont.lfOrientation;    
	  ar<<m_NodeFont.lfWeight;    
	  ar<<m_NodeFont.lfItalic;    
	  ar<<m_NodeFont.lfUnderline; 
      ar<<m_NodeFont.lfStrikeOut;    
	  ar<<m_NodeFont.lfCharSet;    
	  ar<<m_NodeFont.lfOutPrecision; 
      ar<<m_NodeFont.lfClipPrecision;    
	  ar<<m_NodeFont.lfQuality;    
	  ar<<m_NodeFont.lfPitchAndFamily; 
      ar<<CString(m_NodeFont.lfFaceName); 

	  ar << m_Inheritance.m_ShowClassMethods;
	  ar << m_Inheritance.m_ShowClassProperties;
	  ar << m_Inheritance.m_ShowPublicMembers;
	  ar << m_Inheritance.m_ShowProtectedMembers;
	  ar << m_Inheritance.m_ShowPackageAccessMembers;
	  ar << m_Inheritance.m_ShowPrivateMembers;
	  ar << (int)m_Inheritance.m_Name;
	  ar << (int)m_Inheritance.m_Sort;
	  ar << m_Inheritance.m_Merge;


   } else {
	  int tmp;
	  ar >> tmp;
	  m_Orientation     = (CGraphSettings::GraphOrientation)tmp;
	  ar >> tmp;
	  m_Justification   = (CGraphSettings::GraphJustification)tmp;
	  ar >> tmp;
      m_BorderThickness = (CGraphSettings::BorderThickness)tmp;
      ar>> tmp;
	  m_EdgeThickness   = (CGraphSettings::BorderThickness)tmp;

	  ar>>m_NodeBackground[0].m_Red;
	  ar>>m_NodeBackground[0].m_Green;
	  ar>>m_NodeBackground[0].m_Blue;
	  ar>>m_NodeBackground[1].m_Red;
	  ar>>m_NodeBackground[1].m_Green;
	  ar>>m_NodeBackground[1].m_Blue;
	  ar>>m_NodeBackground[2].m_Red;
	  ar>>m_NodeBackground[2].m_Green;
	  ar>>m_NodeBackground[2].m_Blue;

	  ar>>m_NodeBorder[0].m_Red;
	  ar>>m_NodeBorder[0].m_Green;
	  ar>>m_NodeBorder[0].m_Blue;
	  ar>>m_NodeBorder[1].m_Red;
	  ar>>m_NodeBorder[1].m_Green;
	  ar>>m_NodeBorder[1].m_Blue;
	  ar>>m_NodeBorder[2].m_Red;
	  ar>>m_NodeBorder[2].m_Green;
	  ar>>m_NodeBorder[2].m_Blue;

      ar>>m_EdgeColor[0].m_Red;
      ar>>m_EdgeColor[0].m_Green;
      ar>>m_EdgeColor[0].m_Blue;
      ar>>m_EdgeColor[1].m_Red;
      ar>>m_EdgeColor[1].m_Green;
      ar>>m_EdgeColor[1].m_Blue;
      ar>>m_EdgeColor[2].m_Red;
      ar>>m_EdgeColor[2].m_Green;
      ar>>m_EdgeColor[2].m_Blue;

      ar>>m_Routing.m_Orthogonal;
	  ar>>m_Routing.m_Merge;

	  ar>>m_Spacing.m_EnableVatiable;
	  ar>>m_Spacing.m_NodeSpacing;
	  ar>>m_Spacing.m_LevelSpacing;
	  ar>>m_Spacing.m_RowSpacing;
	  ar>>m_Spacing.m_ColumnSpacing;

	  ar>>m_NodeFont.lfHeight;    
	  ar>>m_NodeFont.lfWidth;    
	  ar>>m_NodeFont.lfEscapement; 
      ar>>m_NodeFont.lfOrientation;    
	  ar>>m_NodeFont.lfWeight;    
	  ar>>m_NodeFont.lfItalic;    
	  ar>>m_NodeFont.lfUnderline; 
      ar>>m_NodeFont.lfStrikeOut;    
	  ar>>m_NodeFont.lfCharSet;    
	  ar>>m_NodeFont.lfOutPrecision; 
      ar>>m_NodeFont.lfClipPrecision;    
	  ar>>m_NodeFont.lfQuality;    
	  ar>>m_NodeFont.lfPitchAndFamily; 
	  CString fontName;
      ar>>fontName;
	  strcpy(m_NodeFont.lfFaceName,fontName.GetBuffer(10)); 

	  ar >> m_Inheritance.m_ShowClassMethods;
	  ar >> m_Inheritance.m_ShowClassProperties;
	  ar >> m_Inheritance.m_ShowPublicMembers;
	  ar >> m_Inheritance.m_ShowProtectedMembers;
	  ar >> m_Inheritance.m_ShowPackageAccessMembers;
	  ar >> m_Inheritance.m_ShowPrivateMembers;
	  ar >> tmp;
	  m_Inheritance.m_Name= (CGraphSettings::NameType)tmp;
	  ar >> tmp;
	  m_Inheritance.m_Sort=(CGraphSettings::SortType)tmp;
	  ar >> m_Inheritance.m_Merge;
   }
}
//------------------------------------------------------------------------------------












//------------------------------------------------------------------------------------
// Returns the pointer to the unselected node properties class stored in a view.
//------------------------------------------------------------------------------------
CNodeProperties* CGraphView::GetUnselectedNodeProp() {
	return m_UnselectedNode;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Returns the pointer to the selected node properties class stored in a view.
//------------------------------------------------------------------------------------
CNodeProperties* CGraphView::GetSelectedNodeProp() {
	return m_SelectedNode;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Returns the pointer to the focused node properties class stored in a view.
//------------------------------------------------------------------------------------
CNodeProperties* CGraphView::GetFocusedNodeProp() {
	return m_FocusedNode;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Returns the pointer to the unselected node properties class stored in a view.
//------------------------------------------------------------------------------------
CEdgeProperties* CGraphView::GetUnselectedEdgeProp() {
	return m_EdgeUnselected;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Returns the pointer to the selected node properties class stored in a view.
//------------------------------------------------------------------------------------
CEdgeProperties* CGraphView::GetSelectedEdgeProp() {
	return m_EdgeSelected;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Returns the pointer to the focused node properties class stored in a view.
//------------------------------------------------------------------------------------
CEdgeProperties* CGraphView::GetFocusedEdgeProp() {
	return m_EdgeFocused;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function will load and map selected bitmap to the nodes.
//------------------------------------------------------------------------------------
void CGraphView::AssignBitmaps(int* headerIds, int headerAmount, int* fieldsIds, int fieldsAmount) {
register i;
COLORMAP map;
CBitmap* unselectedBitmap;
CBitmap* selectedBitmap;
CBitmap* focusedBitmap;


    if((headerIds==NULL) && (headerAmount==0)) {
	    m_UnselectedNode->SetNameImageList(NULL,CSize(800,400));
	    m_SelectedNode->SetNameImageList(NULL,CSize(800,400));
	    m_FocusedNode->SetNameImageList(NULL,CSize(800,400));
	} else {
        CBitmapList* unselectedList = new CBitmapList;
        CBitmapList* selectedList   = new CBitmapList;
        CBitmapList* focusedList    = new CBitmapList;
        if(headerIds!=NULL)  {
		    if(m_NameMapID!=NULL) delete m_NameMapID;
		    m_NameMapID=NULL;
		    if(headerAmount>0) {
		        m_NameMapID = new int [headerAmount];
                for(i=0;i<headerAmount;i++) m_NameMapID[i]=headerIds[i];
			} 
		    m_NameMapsAmount=headerAmount;
		}
        for(i=0;i<(headerAmount<0 ? m_NameMapsAmount : headerAmount);i++) {
	        unselectedBitmap = new CBitmap();
	        map.from = RGB(255,255,255);
	        map.to   = m_UnselectedNode->GetNameBackgroundColor();
	        unselectedBitmap->LoadMappedBitmap(m_NameMapID[i],0,&map,1);
		    unselectedList->AddTail(unselectedBitmap);


	        selectedBitmap = new CBitmap();
	        map.from = RGB(255,255,255);
	        map.to   = m_SelectedNode->GetNameBackgroundColor();
	        selectedBitmap->LoadMappedBitmap(m_NameMapID[i],0,&map,1);
		    selectedList->AddTail(selectedBitmap);

	        focusedBitmap = new CBitmap();
	        map.from = RGB(255,255,255);
	        map.to   = m_FocusedNode->GetNameBackgroundColor();
	        focusedBitmap->LoadMappedBitmap(m_NameMapID[i],0,&map,1);
		    focusedList->AddTail(focusedBitmap);
		}
	    m_UnselectedNode->SetNameImageList(unselectedList,CSize(800,400));
	    m_SelectedNode->SetNameImageList(selectedList,CSize(800,400));
	    m_FocusedNode->SetNameImageList(focusedList,CSize(800,400));
	}



	if((fieldsIds==NULL) && (fieldsAmount==0)) {
	    m_UnselectedNode->SetFieldsImageList(NULL,CSize(800,400));
	    m_SelectedNode->SetFieldsImageList(NULL,CSize(800,400));
	    m_FocusedNode->SetFieldsImageList(NULL,CSize(800,400));
	} else {
        CBitmapList* unselectedList = new CBitmapList;
        CBitmapList* selectedList   = new CBitmapList;
        CBitmapList* focusedList    = new CBitmapList;
        if(fieldsIds!=NULL)  {
		    if(m_FieldsMapID!=NULL) delete m_FieldsMapID;
		    m_FieldsMapID=NULL;
		    if(fieldsAmount>0) {
		       m_FieldsMapID = new int [fieldsAmount];
               for(i=0;i<fieldsAmount;i++) m_FieldsMapID[i]=fieldsIds[i];
			} 
		    m_FieldsMapsAmount=fieldsAmount;
		}
        for(i=0;i<(fieldsAmount<0 ? m_FieldsMapsAmount : fieldsAmount);i++) {
	        unselectedBitmap = new CBitmap();
	        map.from = RGB(255,255,255);
	        map.to   = m_UnselectedNode->GetFieldsBackgroundColor();
	        unselectedBitmap->LoadMappedBitmap(m_FieldsMapID[i],0,&map,1);
		    unselectedList->AddTail(unselectedBitmap);


	        selectedBitmap = new CBitmap();
	        map.from = RGB(255,255,255);
	        map.to   = m_SelectedNode->GetFieldsBackgroundColor();
	        selectedBitmap->LoadMappedBitmap(m_FieldsMapID[i],0,&map,1);
		    selectedList->AddTail(selectedBitmap);

	        focusedBitmap = new CBitmap();
	        map.from = RGB(255,255,255);
	        map.to   = m_FocusedNode->GetFieldsBackgroundColor();
	        focusedBitmap->LoadMappedBitmap(m_FieldsMapID[i],0,&map,1);
		    focusedList->AddTail(focusedBitmap);
		}
	    m_UnselectedNode->SetFieldsImageList(unselectedList,CSize(800,400));
	    m_SelectedNode->SetFieldsImageList(selectedList,CSize(800,400));
	    m_FocusedNode->SetFieldsImageList(focusedList,CSize(800,400));
	}
}
//------------------------------------------------------------------------------------


void CGraphView::SetSettingsFile(CString filename, int view) {
	m_SettingsFile=filename;
	m_ViewType = view;

}

void CGraphView::OnDestroy() 
{
	CScrollView::OnDestroy();
    viewInstance[m_ClientGraph->GetClientID()]=NULL;	
	
}



CEdgeProperties* CGraphView::GetRelationsProp(int id) {
	return m_Relations[id];
}
