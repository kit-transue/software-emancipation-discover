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
#include "stdafx.h"
#include "disgraphnode.h"



//-----------------------------------------------------------------------------------
// The default constructor for class MyPathDigraph.
//-----------------------------------------------------------------------------------
CDisPathGraph::CDisPathGraph(TSEdge * pOwnerEdge)
	: TSPathDigraph(pOwnerEdge) {
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// The default destructor for class MyPathDigraph.
//-----------------------------------------------------------------------------------
CDisPathGraph::~CDisPathGraph() {
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This method implements the shared part of the memberwise
// initialization constructor and memberwise assignment constructors.
// To accomplish this, it calls each base class copy constructor to
// copy the shared elements of the input instance to this instance. 
// This method then copies the elements that are particular to this 
// class.
//-----------------------------------------------------------------------------------
void CDisPathGraph::TS_CLASS_UNIQUE_NAME(CDisPathGraph, memberCopy)(
	CDisPathGraph & rMyPathDigraph)
{
	// "Effective C++" recommends this formulation to copy the shared
	// elements of these two instances.

	(TSPathDigraph &) *this = rMyPathDigraph;

	return;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This method creates a new path edge and returns a pointer to it.
// Classes that derive behavior from class TSDigraph should implement
// this method if they would like to extend the functionality of path
// edges. Notice that the implementation of this method requires the
// implementation of MyEdge::MyEdge(TSPathDigraph *).
//-----------------------------------------------------------------------------------
TSEdge * CDisPathGraph::newEdge() {
	return(new CDisGraphEdge(this));
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This method creates a new path node and returns a pointer to it.
// Classes that derive behavior from class TSDigraph should implement
// this method if they would like to extend the functionality of path
// nodes. Notice that the implementation of this method requires the
// implementation of MyNode::MyNode(TSPathDigraph *).
//-----------------------------------------------------------------------------------
TSNode * CDisPathGraph::newNode() {
	return(new CDisGraphNode(this));
}
//-----------------------------------------------------------------------------------




/////////////////////////////////////////////////////////////////////////////////////
// Gaph class represents the collection of nodes and it's relations. It also provides
// the server communication channel and can connect Discover entities in the
// global server array with client nodes and it's fields.
/////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------
// Constructor will assign NULL to all graph pointers
//-----------------------------------------------------------------------------------
CDisGraph::CDisGraph(void) {
    m_NodeWithFocus =NULL;
	m_EdgeWithFocus = NULL;
	m_DataSource    = NULL;
    m_NodeDrawCallback=NULL;
	m_EdgeDrawCallback=NULL;
	m_NodeSizeCallback=NULL;
	m_HeaderPortCallback=NULL;
	m_FieldPortCallback=NULL;
	m_ClientID=0;
}
//-----------------------------------------------------------------------------------


CDisGraph::~CDisGraph() {
}


//-----------------------------------------------------------------------------------
// This method implements the shared part of the memberwise
// initialization constructor and memberwise assignment constructors.
// To accomplish this, it calls each base class copy constructor to
// copy the shared elements of the input instance to this instance. 
// This method then copies the elements that are particular to this 
// class.
//-----------------------------------------------------------------------------------
void CDisGraph::TS_CLASS_UNIQUE_NAME(CDisGraph, memberCopy)(
	CDisGraph & rAppDigraph)
{
	// "Effective C++" recommends this formulation to copy the shared
	// elements of these two instances.

	(TSDigraph &) *this = rAppDigraph;

    m_NodeWithFocus     = rAppDigraph.m_NodeWithFocus;
	m_EdgeWithFocus     = rAppDigraph.m_EdgeWithFocus;
	m_DataSource        = rAppDigraph.m_DataSource;
    m_NodeDrawCallback  = rAppDigraph.m_NodeDrawCallback;
	m_EdgeDrawCallback  = rAppDigraph.m_EdgeDrawCallback;
	m_NodeSizeCallback  = rAppDigraph.m_NodeSizeCallback;
	m_HeaderPortCallback= rAppDigraph.m_HeaderPortCallback;
	m_FieldPortCallback = rAppDigraph.m_FieldPortCallback;
	m_ClientID          = rAppDigraph.m_ClientID;

}



//-----------------------------------------------------------------------------------
// This method acts as a "virtual constructor" to allocate a graph
// object. Developers who implement classes that derive behavior from
// class TSDigraph should implement this method. void
//-----------------------------------------------------------------------------------
TSDigraph * CDisGraph::newDigraph(void) {
	return (new CDisGraph());
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// This method acts as a "virtual constructor" to allocate an edge
// object. Developers who implement classes that derive behavior from
// class TSDigraph should implement this method if they also implement
// a class that derives from TSEdge.
//-----------------------------------------------------------------------------------
TSEdge * CDisGraph::newEdge(void) {
	return (new CDisGraphEdge(this));
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// This method acts as a "virtual constructor" to allocate a node
// object. Developers who implement classes that derive behavior from
// class TSDigraph should implement this method if they also implement
// a class that derives from TSNode.
//-----------------------------------------------------------------------------------
TSNode * CDisGraph::newNode(void) {
	return (new CDisGraphNode(this));
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// This method creates a new MyPathDigraph and returns a pointer to it.
// Classes that derive behavior from class TSDigraph should implement
// this method.
//-----------------------------------------------------------------------------------
TSPathDigraph * CDisGraph::newPathDigraph(TSEdge * pOwnerEdge) {
	return(new CDisPathGraph(pOwnerEdge));
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Returns the currently focused node or NULL if no such node
//-----------------------------------------------------------------------------------
CDisGraphNode* CDisGraph::GetFocusedNode(void) {
	return m_NodeWithFocus;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Sets the node which will have focus
//-----------------------------------------------------------------------------------
void CDisGraph::SetFocusedNode(CDisGraphNode* node) {
   m_NodeWithFocus = node;
}
//-----------------------------------------------------------------------------------
			

//-----------------------------------------------------------------------------------
// Returns the pointer to the edge that currently has focus.
//-----------------------------------------------------------------------------------
CDisGraphEdge* CDisGraph::GetFocusedEdge(void) {
	return m_EdgeWithFocus;
}
//-----------------------------------------------------------------------------------
			

//-----------------------------------------------------------------------------------
// Set the pointer to the edge that has focus
//-----------------------------------------------------------------------------------
void CDisGraph::SetFocusedEdge(CDisGraphEdge* edge) {
	m_EdgeWithFocus=edge;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This method will assign a data source to the graph. Default data
// source pointer is NULL, so any attempt to evaluate server script
// by calling "Evaluate" method will fail.
//-----------------------------------------------------------------------------------
void CDisGraph::SetDataSource(CDataSource* connection) {
	m_DataSource=connection;
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// This method will return a pointer to the data source assigned to 
// the graph. 
//-----------------------------------------------------------------------------------
CDataSource* CDisGraph::GetDataSource(void) {
	return m_DataSource;
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// This method evaluates the string passed as "cmd" parameter on the 
// server and returns evaluation results as a string. Will do nothing
// if no data source assigned to the graph.
//-----------------------------------------------------------------------------------
CString CDisGraph::Evaluate(CString& cmd) {
BSTR sysStr;
CString results;
static BOOL bCommDlgShown = FALSE;

	sysStr=cmd.AllocSysString();
	results=m_DataSource->AccessSync(&sysStr);
    ::SysFreeString(sysStr);
	if (m_DataSource->IsConnectionLost() == TRUE &&
		bCommDlgShown != TRUE ) {
	  ::MessageBox(m_hWnd, 
			   _T("Connection with server is lost."
			   " Make sure the server is running."),
			   _T("Server Communication Error."), 
			   MB_OK | MB_ICONINFORMATION);
		bCommDlgShown = TRUE;
	} else if (m_DataSource->IsConnectionLost() != TRUE) {
		bCommDlgShown = FALSE;
	}	
	return results;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// Will walk and re-reference all nodes and fields to have up-to-date
// IDs. We need this when something is removed and we need to rebuild
// global server array.
//-----------------------------------------------------------------------------------
void CDisGraph::ReReference(void) {
TSDListIterator nodeIterator(pNodeDList());
TSDListIterator hideIterator(pHideDigraph()->pNodeDList());
CDisGraphNode* pNode;
CString result;
CItemsList* nodeItems;
int oldIndex;
int newIndex;
CString command;


	Evaluate(CString("graph_copy_start"));
    while(nodeIterator)  {
		pNode = (CDisGraphNode *) nodeIterator.pObject();

		// We will copy entity connected to the node from the original global
		// server array into the new one and we will assign the node an index
		// of the entity in the new array.
		oldIndex = pNode->GetNodeID();
		command.Format("graph_copy_element %u",oldIndex);
		result = Evaluate(command);
		newIndex = atoi(result);
		pNode->SetNodeID(newIndex);

		// We will copy all node fields from the original global server array
		// to the new array and we will assign the entitiy index in the new
		// array to the client node field.
		nodeItems=pNode->GetItemsList();
		POSITION pos = nodeItems->GetHeadPosition();
		while(pos) {
            oldIndex = nodeItems->GetAt(pos).m_FieldID;
		    command.Format("graph_copy_element %u",oldIndex);
		    result = Evaluate(command);
		    newIndex = atoi(result);
			CNodeItem item;
            item = nodeItems->GetAt(pos);
			item.m_FieldID=newIndex;
			nodeItems->SetAt(pos,item);
			nodeItems->GetNext(pos);
		}
		nodeIterator++;
	}
    while(hideIterator)  {
		pNode = (CDisGraphNode *) hideIterator.pObject();

		// We will copy entity connected to the node from the original global
		// server array into the new one and we will assign the node an index
		// of the entity in the new array.
		oldIndex = pNode->GetNodeID();
		command.Format("graph_copy_element %u",oldIndex);
		result = Evaluate(command);
		newIndex = atoi(result);
		pNode->SetNodeID(newIndex);

		// We will copy all node fields from the original global server array
		// to the new array and we will assign the entitiy index in the new
		// array to the client node field.
		nodeItems=pNode->GetItemsList();
		POSITION pos = nodeItems->GetHeadPosition();
		while(pos) {
            oldIndex = nodeItems->GetAt(pos).m_FieldID;
		    command.Format("graph_copy_element %u",oldIndex);
		    result = Evaluate(command);
		    newIndex = atoi(result);
			CNodeItem item;
            item = nodeItems->GetAt(pos);
			item.m_FieldID=newIndex;
			nodeItems->SetAt(pos,item);
			nodeItems->GetNext(pos);
		}
		hideIterator++;
	}
	Evaluate(CString("graph_copy_complete"));
}
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// This method will find the node in the graph with specified ID
//-----------------------------------------------------------------------------------
CDisGraphNode* CDisGraph::NodeFromID(int id) {
TSDListIterator nodeIterator(pNodeDList());
CDisGraphNode* pNode;
    while(nodeIterator)  {
		pNode = (CDisGraphNode *) nodeIterator.pObject();
		if(pNode->GetNodeID()==id) return pNode;
		nodeIterator++;
	}
	return NULL;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This method sets the node drawing callback function. 
// If you want the integration of the layout graph with the client
// graphical view you need to implement node drawing function 
// which will receve node rectangle (after layout), node name, 
// node fields list and node properties and will draw this node in a 
// view. Example:
// void DrawNode(CRect&      nodeRect,
//               CString&    nodeName, 
//               CItemsList* nodeItems, 
//    			 CNodeProperties* nodeProp);
//-----------------------------------------------------------------------------------
void CDisGraph::SetNodeDrawCallback(CNodeDrawCallback nodeDrawCallback) {
	m_NodeDrawCallback=nodeDrawCallback;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This method sets the edge drawing callback function. 
// If you want the integration of the layout graph with the client
// graphical view you need to implement edge drawing function 
// which will receve the edge path as an array of CPoint'ers, amount,
// of vertexes in a path and edge style.
//  Example:
// void DrawEdge(CPoint* vertexes, 
//               int vertexAmount, 
//               CEdgeProperties* edgeProperties);
//-----------------------------------------------------------------------------------
void CDisGraph::SetEdgeDrawCallback(CEdgeDrawCallbcak edgeDrawCallback) {
   m_EdgeDrawCallback=edgeDrawCallback;
}
//-----------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------
// This method sets the node size calculation callback function. 
// If you want the integration of the layout graph with the client
// graphical view you need to implement node size calculation function 
// which will receve node name, node fields list and node properties
// and will return the size of the node . Examlpe:
//  CSize CalculateNodeSize(CString&    nodeName, 
//                          CItemsList* nodeItems, 
//    				        CNodeProperties* nodeProp);
//-----------------------------------------------------------------------------------
void CDisGraph::SetNodeSizeCalculationCallback(CNodeSizeCallback nodeSizeCallback) {
	m_NodeSizeCallback=nodeSizeCallback;
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This method will set the node header port calculation callback.
// This callback will call graph view to determine the amount of ports 
// on the defined side and the port to connect to. The ports amount will
// be placed in the x field of the returned function, the port to connect to
// will be placed into the y field of the return structure.
//-----------------------------------------------------------------------------------
void CDisGraph::SetNodeHeaderPortCalculationCallback(CHeaderPortCallback headerPort) {
	m_HeaderPortCallback=headerPort;
}


//-----------------------------------------------------------------------------------
// This method will set the specified field port calculation callback.
// This callback will call graph view to determine the amount of ports 
// on the defined side and the port to connect to. The ports amount will
// be placed in the x field of the returned function, the port to connect to
// will be placed into the y field of the return structure.
//-----------------------------------------------------------------------------------
void CDisGraph::SetNodeFieldPortCalculationCallback(CFieldPortCallback  fieldPort) {
	m_FieldPortCallback = fieldPort;
}


//-----------------------------------------------------------------------------------
// This method will call node size calculation callback to determine the
// desired size of every node in the graph.
// We will use two node functions: "GetReadableName" returns the node readable name
// extracted by name extraction script when node was created and 
// "GetActiveNodeProperties" will examin node selection status and node focus status
// to return one of the three available node properties 
// (unselected,selected or focused)
//-----------------------------------------------------------------------------------
void CDisGraph::DetermineNodeSizes(void) {
TSDListIterator nodeIterator(pNodeDList());
CDisGraphNode* pNode;
CItemsList* nodeItems;
CSize nodeSize;

    if(m_NodeSizeCallback==NULL) return;
    while(nodeIterator)  {
		pNode = (CDisGraphNode *) nodeIterator.pObject();
	    nodeItems=pNode->GetItemsList();
		nodeSize=(*m_NodeSizeCallback)(m_ClientID,pNode->GetReadableName(),nodeItems,pNode->GetActiveNodeProperties(),pNode->m_HeaderHeight); 
		pNode->resize(nodeSize.cx, nodeSize.cy);
		nodeIterator++;
	}
}
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This method will call node draw and edge draw callbacks when we need
// to draw all graph. Use "DrawGraphRect" method to draw the nodes
// and edges contained in the rectangle.
//-----------------------------------------------------------------------------------
void CDisGraph::DrawGraph(void) {
TSDListIterator nodeIterator(pNodeDList());
TSDListIterator edgeIterator(pEdgeDList());
CDisGraphNode* pNode;
CDisGraphEdge* pEdge;
CItemsList* nodeItems;
CRect nodeRect;
CPoint path[1024];
TSPathDigraph* edgePath;
TSEdge* pPathEdge;
    
	// Drawing all edges
	if(m_EdgeDrawCallback!=NULL) {
        while(edgeIterator)  {
		    pEdge = CDisGraphEdge::castDown(edgeIterator.pObject());
		    int pathVertex=0;
		    edgePath=pEdge->pChildPathDigraph();
			if(edgePath!=NULL) {
               TSDListIterator pathIterator(edgePath->pEdgeDList());
		       while(pathIterator) {
		           pPathEdge = (TSEdge *) pathIterator.pObject();
			       path[pathVertex].x=pPathEdge->fromX();
			       path[pathVertex].y=pPathEdge->fromY();
			       path[pathVertex+1].x=pPathEdge->toX();
			       path[pathVertex+1].y=pPathEdge->toY();
			       pathIterator++;
			       pathVertex++;
			   }
               (m_EdgeDrawCallback)(m_ClientID,path,pathVertex+1,pEdge->GetActiveEdgeProperties());
			} else {
	               TSPad * pFromPad = pEdge->pFromPad();
	               TSPad * pToPad   = pEdge->pToPad();
				   pathVertex=0;
			       path[pathVertex].x=pFromPad->x();
			       path[pathVertex].y=pFromPad->y();
			       path[pathVertex+1].x=pToPad->x();
			       path[pathVertex+1].y=pToPad->y();
                   pathVertex=2;
                   (m_EdgeDrawCallback)(m_ClientID,path,pathVertex,pEdge->GetActiveEdgeProperties());

			}
		    edgeIterator++;
		}
	}

    // Drawing all nodes
    if(m_NodeDrawCallback!=NULL) {
        while(nodeIterator)  {
            int connectorSide=NONE;
            BOOL status;
		    pNode = CDisGraphNode::castDown(nodeIterator.pObject());
	        nodeItems=pNode->GetItemsList();
			// Determine wether to display expand/collapse buttons and where
			if(pNode->m_ChildNode && pNode->HasChildren()==TRUE) {
				if(pNode->AreChildrenCreated()==TRUE)  {
					if(pNode->IsCollapsed()==TRUE) status=TRUE;
					else                           status=FALSE;
				} else status=TRUE;
				switch(pHDigraph()->pHTailor()->hlevelOrientation()) {
				    case TS_LEFT_TO_RIGHT : connectorSide = RIGHT;  break;
				    case TS_RIGHT_TO_LEFT : connectorSide = LEFT;   break;
				    case TS_TOP_TO_BOTTOM : connectorSide = BOTTOM; break;
				    case TS_BOTTOM_TO_TOP : connectorSide = TOP;    break;
				}
			} 
			// Determine wether to display expand/collapse buttons for parents
            int parentsConnectorSide=NONE;
            BOOL parentsStatus;
			if(pNode->m_ParentNode && pNode->HasParents()==TRUE) {
				if(pNode->AreParentsCreated()==TRUE)  {
					if(pNode->IsParentsCollapsed()==TRUE) parentsStatus=TRUE;
					else                                  parentsStatus=FALSE;
				} else parentsStatus=TRUE;
				switch(pHDigraph()->pHTailor()->hlevelOrientation()) {
				    case TS_LEFT_TO_RIGHT : parentsConnectorSide = LEFT;  break;
				    case TS_RIGHT_TO_LEFT : parentsConnectorSide = RIGHT; break;
				    case TS_TOP_TO_BOTTOM : parentsConnectorSide = TOP;   break;
				    case TS_BOTTOM_TO_TOP : parentsConnectorSide = BOTTOM;break;
				}
			} 

		    // Extracting new node rectangle from the graph - layout changed
		    // this rectangle position.
		    CRect nodeRect;
            nodeRect.top    = pNode->top();
		    nodeRect.bottom = pNode->bottom();
		    nodeRect.left   = pNode->left();
		    nodeRect.right  = pNode->right();
            // Drawinfg the node using node drawing callback
            (*m_NodeDrawCallback)(m_ClientID,nodeRect,pNode->GetReadableName(),pNode->GetNodeKind(), nodeItems,pNode->GetActiveNodeProperties(),connectorSide,status,parentsConnectorSide,parentsStatus) ;
		    nodeIterator++;
		}
	}
}
//-----------------------------------------------------------------------------------



/////////////////////////////////////////////////////////////////////////////////////
// Node is the client view on the set element - entity. Every node contain pointer 
// to it's graph. Node use this pointer to evaluate scripts using graph 
// communications with server and also it uses ReReference graph method in the
// case some nodes or fields removed. All new nodes created by the node creation
// script place the entities they present into the global server array. The entity
// number in this array is the node identification number. Node identification number
// can change in the case we remove some nodes or node fields from the global array.
// In this case ReReference method of the graph will process all remaining 
// (not deleted) nodes and their fields copying the connected entities to the new 
// global array and re-setting the node and it's fieldds identification numbers.
/////////////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------------
// This constructor creates new empty node - a child of the "graph". You need to 
// call "SetNode" member function to assign scripts and styles to this node
//----------------------------------------------------------------------------------
CDisGraphNode::CDisGraphNode(TSDigraph* graph):TSNode(graph) {

	m_ParentGraph=(CDisGraph *)graph;

	m_NodeID = -1;

	m_NodeNameExtractor   = "";
	m_NodeImageExtractor  = "";
	m_FieldNameExtractor  = "";
	m_FieldImageExtractor = "";


	m_NodeUnselected  = NULL;
	m_NodeSelected    = NULL;
	m_NodeFocused     = NULL;
	m_EdgeUnselected  = NULL;
	m_EdgeSelected    = NULL;
	m_EdgeFocused     = NULL;

	m_SelectStatus= FALSE;
	m_ChildrenCreated = FALSE;
	m_ParentsCreated = FALSE;
	m_Collapsed=FALSE;
	m_ParentsCollapsed=FALSE;
	m_ChildNode=TRUE;
	m_ParentNode=TRUE;
	m_HeaderHeight = 0;
}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// This constructor creates new empty node - a child of the path graph "graph". 
// You need to call "SetNode" member function to assign scripts and styles 
// to this node
//----------------------------------------------------------------------------------
CDisGraphNode::CDisGraphNode(TSPathDigraph* graph):TSNode(graph) {

	m_ParentGraph=(CDisGraph *)graph;

	m_NodeID = -1;

	m_NodeNameExtractor   = "";
	m_NodeImageExtractor  = "";
	m_FieldNameExtractor  = "";
	m_FieldImageExtractor = "";


	m_NodeUnselected  = NULL;
	m_NodeSelected    = NULL;
	m_NodeFocused     = NULL;
	m_EdgeUnselected  = NULL;
	m_EdgeSelected    = NULL;
	m_EdgeFocused     = NULL;

	m_SelectStatus= FALSE;
	m_ChildrenCreated = FALSE;
	m_ParentsCreated = FALSE;
	m_Collapsed=FALSE;
	m_ParentsCollapsed=FALSE;
	m_ChildNode=TRUE;
	m_ParentNode=TRUE;
	m_HeaderHeight = 0;
}
//----------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// This function will assign extracting scripts and node properties to this node:
// "id"                 is the index of the entity in the server global array. 
// "childCreator"        script to create childs
// "parentCreator"       script to create parents
// "fieldsExtractor"     script to create fields
// "nodeNameExtractor"   is a script which will extract the node name from the
//                       Discover entity.
// "nodeImageExtractior" is a script which will return the index of the image 
//                       to show by quering Discover entity
// "fieldsNameExtractor" is a script which will create a symbol set and place it
//                       into the global server array. This symbols will be displayed
//                       together with the node.
// "fieldsImageExtractor"is a script which will return an index of an image to 
//                       show with the node field
// "unselected"          node display style if unselected
// "selected"            node display style if selected
// "focused"             node display style if has focus
//------------------------------------------------------------------------------------
void CDisGraphNode::SetNode(int id,                        // entity index in the server array
							 CString& childCreator,        // script to create childs
                             CString& parentCreator,       // script to create parents
							 CString& fieldsCreator,       // script to create fields
						     CString& nodeNameExtractor,   // script to extract name
						     CString& fieldsNameExtractor, // script to extract field name
				             CString& nodeImageExtractor,  // script to extract image
						     CString& fieldsImageExtractor,// script to extract field image
					         CString& testChildren,        // script to test children 
					         CString& testParents,         // script to test parents
						     CNodeProperties* nodeUnselected,  // unselected style
			                 CNodeProperties* nodeSelected,    // selected style
						     CNodeProperties* nodeFocused,     // focused style
						     CEdgeProperties* edgeUnselected,  // unselected style
			                 CEdgeProperties* edgeSelected,    // selected style
						     CEdgeProperties* edgeFocused      // focused style
							 ) {
	m_NodeID = id;
	m_ChildrenCreator  = childCreator;
	m_ParentsCreator   = parentCreator;
	m_FieldsCreator   = fieldsCreator;

	m_NodeNameExtractor   = nodeNameExtractor;
	m_NodeImageExtractor  = nodeImageExtractor;
	m_FieldNameExtractor  = fieldsNameExtractor;
	m_FieldImageExtractor = fieldsImageExtractor;

	m_ChildTestScript = testChildren;
	m_ParentTestScript= testParents;


	m_NodeUnselected  = nodeUnselected;
	m_NodeSelected    = nodeSelected;
	m_NodeFocused     = nodeFocused;

	m_EdgeUnselected  = edgeUnselected;
	m_EdgeSelected    = edgeSelected;
	m_EdgeFocused     = edgeFocused;

	m_SelectStatus= FALSE;
	m_ChildrenCreated = FALSE;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
CDisGraphNode::~CDisGraphNode() {
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void CDisGraphNode::TS_CLASS_UNIQUE_NAME(CDisGraphNode, memberCopy)(
	CDisGraphNode & rAppNode) {
	// "Effective C++" recommends this formulation to copy the shared
	// elements of these two instances.

	(TSNode &) *this = rAppNode;

	m_NodeID              = rAppNode.m_NodeID;
	m_ParentGraph         = rAppNode.m_ParentGraph;

	m_ChildrenCreator     = rAppNode.m_ChildrenCreator;
	m_ParentsCreator      = rAppNode.m_ParentsCreator;
	m_FieldsCreator       = rAppNode.m_FieldsCreator;
	m_NodeNameExtractor   = rAppNode.m_NodeNameExtractor;
	m_NodeImageExtractor  = rAppNode.m_NodeImageExtractor;
	m_FieldNameExtractor  = rAppNode.m_FieldNameExtractor;
	m_FieldImageExtractor = rAppNode.m_FieldImageExtractor;

	m_ReadableName        = rAppNode.m_ReadableName;
	//m_FieldsList          = rAppNode.m_FieldsList;
	m_SelectStatus        = rAppNode.m_SelectStatus;
	m_Collapsed           = rAppNode.m_Collapsed;
	m_HasChildren         = rAppNode.m_HasChildren;
	m_HasParents          = rAppNode.m_HasParents;
	m_ChildrenCreated     = rAppNode.m_ChildrenCreated;
	m_ParentsCreated      = rAppNode.m_ParentsCreated;


	m_NodeUnselected          = rAppNode.m_NodeUnselected;
	m_NodeSelected            = rAppNode.m_NodeSelected;
	m_NodeFocused             = rAppNode.m_NodeFocused;
	m_EdgeUnselected          = rAppNode.m_EdgeUnselected;
	m_EdgeSelected            = rAppNode.m_EdgeSelected;
	m_EdgeFocused             = rAppNode.m_EdgeFocused;

	m_ChildNode=rAppNode.m_ChildNode;
	m_ParentNode=rAppNode.m_ParentNode;
	m_HeaderHeight = rAppNode.m_HeaderHeight;

	return;
}
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// This function will evaluate name extraction script and will fill node internal
// variable with the node readable name.
//------------------------------------------------------------------------------------
void CDisGraphNode::ExtractName(void) {
CString cmd;

      cmd.Format("%s %u",m_NodeNameExtractor,m_NodeID);
	  m_ReadableName = m_ParentGraph->Evaluate(cmd);
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This method will evaluate "accessGet" server script passing this
// node entity as a parameter.It will assign the ID's for all newly
// created nodes (which are indexes in the global set), will assign
// to all new nodes the script which will extract the name from the 
// entity ("accessNodeName"),will assign to all newly created nodes
// the script which will return image index receving the node entity
// as an argument, will assing the script to extract node fields, 
// extract the name and image index from the field. Thee styles will
// be assigned to the new nodes : unselected,selected and focused
// styles.
// The "accessGet" script will return a list of indexes for the 
// newly created nodes in the global server array.
//------------------------------------------------------------------------------------
void CDisGraphNode::CreateChildNodes(CString* accessCreateChildren, 
									 CString* accessCreateParent,
				                     CString* accessCreateFields,
					                 CString* accessGetNodeName,
					                 CString* accessGetFieldName,
					                 CString* accessGetNodeImage,
					                 CString* accessGetFieldImage,
									 CString* accessTestChildren,
                                     CString* accessTestParents,
					                 CNodeProperties* nodeUnselected,
					                 CNodeProperties* nodeSelected,
					                 CNodeProperties* nodeFocused,
					                 CEdgeProperties* edgeUnselected,
					                 CEdgeProperties* edgeSelected,
					                 CEdgeProperties* edgeFocused) {
CString command;
CString result;
int* nodeIDs;
register i;

   if(m_ChildrenCreated == TRUE) return;

   // Determining which scripts will be assigned to children
   CString childrenCreator;
   CString parentsCreator;
   CString fieldsCreator;
   CString nodeNameExtractor;
   CString fieldNameExtractor;
   CString nodeImageExtractor;
   CString fieldImageExtractor;
   CString testChildren;
   CString testParents;
   // Script to create children
   if(accessCreateChildren!=NULL) childrenCreator=*accessCreateChildren;
   else childrenCreator = m_ChildrenCreator;
   // Script to create parent
   if(accessCreateParent!=NULL) parentsCreator=*accessCreateParent;
   else parentsCreator = m_ParentsCreator;
   // Script to create node fields
   if(accessCreateFields!=NULL) fieldsCreator=*accessCreateFields;
   else fieldsCreator = m_FieldsCreator;
   // Script to extract node name
   if(accessGetNodeName!=NULL) nodeNameExtractor=*accessGetNodeName;
   else nodeNameExtractor = m_NodeNameExtractor;
   // Script to extract field name
   if(accessGetFieldName!=NULL) fieldNameExtractor=*accessGetFieldName;
   else fieldNameExtractor = m_FieldNameExtractor;
   // Script to extract node image
   if(accessGetNodeImage!=NULL) nodeImageExtractor=*accessGetNodeImage;
   else nodeImageExtractor = m_NodeImageExtractor;
   // Script to extract field image
   if(accessGetFieldImage!=NULL) fieldImageExtractor=*accessGetFieldImage;
   else fieldImageExtractor = m_FieldImageExtractor;
   // Script to test if the node has children
   if(accessTestChildren!=NULL) testChildren=*accessTestChildren;
   else testChildren = m_ChildTestScript;
   // Script to test if node has parents
   if(accessTestParents!=NULL) testParents=*accessTestParents;
   else testParents = m_ParentTestScript;


   // Determining which propertires we will use
   CNodeProperties* pNodeUnselected;
   CNodeProperties* pNodeSelected;
   CNodeProperties* pNodeFocused;
   CEdgeProperties* pEdgeUnselected;
   CEdgeProperties* pEdgeSelected;
   CEdgeProperties* pEdgeFocused;

   // Nodes
   if(nodeUnselected!=NULL) pNodeUnselected = nodeUnselected;
   else pNodeUnselected = m_NodeUnselected;
   if(nodeSelected!=NULL) pNodeSelected = nodeSelected;
   else pNodeSelected = m_NodeSelected;
   if(nodeFocused!=NULL) pNodeFocused = nodeFocused;
   else pNodeFocused = m_NodeFocused;

   // Edges
   if(edgeUnselected!=NULL) pEdgeUnselected = edgeUnselected;
   else pEdgeUnselected = m_EdgeUnselected;
   if(edgeSelected!=NULL) pEdgeSelected = edgeSelected;
   else pEdgeSelected = m_EdgeSelected;
   if(edgeFocused!=NULL) pEdgeFocused = edgeFocused;
   else pEdgeFocused = m_EdgeFocused;


   // this script will create entities set from the given one and will
   // place all entities to the global graph array. It will return the
   // list of new entities indexes to create related client nodes.
   command.Format("%s %u",childrenCreator,m_NodeID);
   result = m_ParentGraph->Evaluate(command);
   int nodesAmount = m_ParentGraph->ParseCreationResults(result,&nodeIDs);
   if(nodesAmount==0) return;
   for(i=0;i<nodesAmount;i++) {
	   CDisGraphNode* pNode;

	   // Maybe new node in the graph ?
	   pNode=m_ParentGraph->FindNodeWithID(nodeIDs[i]);
	   if(pNode==NULL) {
	       // Inserting child nodes into the graph
	       pNode = (CDisGraphNode *)m_ParentGraph->addNode();
	       pNode->SetNode( nodeIDs[i],
					       childrenCreator,
					       parentsCreator,
					       fieldsCreator,
					       nodeNameExtractor,
                           fieldNameExtractor,
                           nodeImageExtractor,
                           fieldImageExtractor,
					       testChildren,
					       testParents,
					       pNodeUnselected,
					       pNodeSelected,
					       pNodeFocused,
					       pEdgeUnselected,
					       pEdgeSelected,
					       pEdgeFocused
					     );
	       pNode->m_ChildNode=TRUE;
	       pNode->m_ParentNode=FALSE;
	       pNode->SetParentsCreated(TRUE);

	       pNode->ExtractFields();
	       pNode->ExtractName();
	       pNode->ExtractNodeKind();
	       pNode->ExtractParentChildEnable();

	   }
	   // Adding parent-child connections
	   CDisGraphEdge* pEdge;
	   pEdge = (CDisGraphEdge *)m_ParentGraph->addEdge(this,pNode);
	   pEdge->Port();
	   pEdge->SetEdge(0,pEdgeUnselected,pEdgeSelected,pEdgeFocused);
   }
   // removing new nodes IDs array created by ParseCreationResults
   delete nodeIDs;

   m_ChildrenCreated = TRUE;
   m_Collapsed=FALSE;
}
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// This method will evaluate "accessGet" server script passing this
// node entity as a parameter.It will assign the ID's for all newly
// created nodes (which are indexes in the global set), will assign
// to all new nodes the script which will extract the name from the 
// entity ("accessNodeName"),will assign to all newly created nodes
// the script which will return image index receving the node entity
// as an argument, will assing the script to extract node fields, 
// extract the name and image index from the field. Thee styles will
// be assigned to the new nodes : unselected,selected and focused
// styles.
// The "accessGet" script will return a list of indexes for the 
// newly created nodes in the global server array.
//------------------------------------------------------------------------------------
void CDisGraphNode::CreateParentNodes(CString* accessCreateChildren, 
									 CString* accessCreateParent,
				                     CString* accessCreateFields,
					                 CString* accessGetNodeName,
					                 CString* accessGetFieldName,
					                 CString* accessGetNodeImage,
					                 CString* accessGetFieldImage,
					                 CString* accessTestChildren,
					                 CString* accessTestParents,
					                 CNodeProperties* nodeUnselected,
					                 CNodeProperties* nodeSelected,
					                 CNodeProperties* nodeFocused,
					                 CEdgeProperties* edgeUnselected,
					                 CEdgeProperties* edgeSelected,
					                 CEdgeProperties* edgeFocused) {
CString command;
CString result;
int* nodeIDs;
register i;

   if(m_ParentsCreated == TRUE) return;

   // Determining which scripts will be assigned to children
   CString childrenCreator;
   CString parentsCreator;
   CString fieldsCreator;
   CString nodeNameExtractor;
   CString fieldNameExtractor;
   CString nodeImageExtractor;
   CString fieldImageExtractor;
   CString testChildren;
   CString testParents;

   // Script to create children
   if(accessCreateChildren!=NULL) childrenCreator=*accessCreateChildren;
   else childrenCreator = m_ChildrenCreator;
   // Script to create parent
   if(accessCreateParent!=NULL) parentsCreator=*accessCreateParent;
   else parentsCreator = m_ParentsCreator;
   // Script to create node fields
   if(accessCreateFields!=NULL) fieldsCreator=*accessCreateFields;
   else fieldsCreator = m_FieldsCreator;
   // Script to extract node name
   if(accessGetNodeName!=NULL) nodeNameExtractor=*accessGetNodeName;
   else nodeNameExtractor = m_NodeNameExtractor;
   // Script to extract field name
   if(accessGetFieldName!=NULL) fieldNameExtractor=*accessGetFieldName;
   else fieldNameExtractor = m_FieldNameExtractor;
   // Script to extract node image
   if(accessGetNodeImage!=NULL) nodeImageExtractor=*accessGetNodeImage;
   else nodeImageExtractor = m_NodeImageExtractor;
   // Script to extract field image
   if(accessGetFieldImage!=NULL) fieldImageExtractor=*accessGetFieldImage;
   else fieldImageExtractor = m_FieldImageExtractor;
   // Script to test if the node has children
   if(accessTestChildren!=NULL) testChildren=*accessTestChildren;
   else testChildren = m_ChildTestScript;
   // Script to test if node has parents
   if(accessTestParents!=NULL) testParents=*accessTestParents;
   else testParents = m_ParentTestScript;


   // Determining which propertires we will use
   CNodeProperties* pNodeUnselected;
   CNodeProperties* pNodeSelected;
   CNodeProperties* pNodeFocused;
   CEdgeProperties* pEdgeUnselected;
   CEdgeProperties* pEdgeSelected;
   CEdgeProperties* pEdgeFocused;

   // Nodes
   if(nodeUnselected!=NULL) pNodeUnselected = nodeUnselected;
   else pNodeUnselected = m_NodeUnselected;
   if(nodeSelected!=NULL) pNodeSelected = nodeSelected;
   else pNodeSelected = m_NodeSelected;
   if(nodeFocused!=NULL) pNodeFocused = nodeFocused;
   else pNodeFocused = m_NodeFocused;

   // Edges
   if(edgeUnselected!=NULL) pEdgeUnselected = edgeUnselected;
   else pEdgeUnselected = m_EdgeUnselected;
   if(edgeSelected!=NULL) pEdgeSelected = edgeSelected;
   else pEdgeSelected = m_EdgeSelected;
   if(edgeFocused!=NULL) pEdgeFocused = edgeFocused;
   else pEdgeFocused = m_EdgeFocused;


   // this script will create entities set from the given one and will
   // place all entities to the global graph array. It will return the
   // list of new entities indexes to create related client nodes.
   command.Format("%s %u",parentsCreator,m_NodeID);
   result = m_ParentGraph->Evaluate(command);
   int nodesAmount = m_ParentGraph->ParseCreationResults(result,&nodeIDs);
   if(nodesAmount==0) return;
   for(i=0;i<nodesAmount;i++) {
	   CDisGraphNode* pNode;
	   // Maybe this node is in the graph ?
	   pNode=m_ParentGraph->FindNodeWithID(nodeIDs[i]);
	   if(pNode==NULL) {
	       // Inserting child nodes into the graph
	       pNode = (CDisGraphNode *)m_ParentGraph->addNode();
	       pNode->SetNode( nodeIDs[i],
					       childrenCreator,
					       parentsCreator,
					       fieldsCreator,
					       nodeNameExtractor,
                           fieldNameExtractor,
                           nodeImageExtractor,
                           fieldImageExtractor,
					       testChildren,
					       testParents,
					       pNodeUnselected,
					       pNodeSelected,
					       pNodeFocused,
					       pEdgeUnselected,
					       pEdgeSelected,
					       pEdgeFocused
					     );
	       pNode->m_ChildNode=FALSE;
	       pNode->m_ParentNode=TRUE;
	       pNode->ExtractFields();
	       pNode->ExtractName();
	       pNode->ExtractNodeKind();
	       pNode->ExtractParentChildEnable();
	       pNode->SetChildrenCreated(TRUE);
	   }
	   // Adding parent-child connections
	   CDisGraphEdge* pEdge;
	   pEdge = (CDisGraphEdge *)m_ParentGraph->addEdge(pNode,this);
	   pEdge->Port();
	   pEdge->SetEdge(0,pEdgeUnselected,pEdgeSelected,pEdgeFocused);
   }
   // removing new nodes IDs array created by ParseCreationResults
   delete nodeIDs;

   m_ParentsCreated = TRUE;
   m_Collapsed=FALSE;
}
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// This function will evaluate accessCmd command to receve the list of
// the related objects, will search the graph to locate if and where
// this existing objects located in the graph and will add edges of the
// type "relationID" connecting this node with all located elements 
// (nodes or node fields) in the graph.
//------------------------------------------------------------------------------------
void CDisGraphNode::CreateRelations(CString& acccessCmd, 
				                    int relationID, 
				                    CEdgeProperties* unselected,
								    CEdgeProperties* selected,
									CEdgeProperties* focused) {
CString command;
CString result;
int i;
int* sourceNodeIDs;
int* sourceFieldIDs;
int* targetNodeIDs;
int* targetFieldIDs;

   // the relations script procedure will receve this node Id as a parameter and
   // will return a list containing as many 4-group elements as the quantity of
   // the relations it located. Each 4-digit entry contains the source node id
   //(it always be this node ID), source node field ID if any or 0 if node itself,
   // target node id and target node field id.
   // line this:
   // { SourceNode Sourcefield TargetNode TargetField }
   // { SourceNode Sourcefield TargetNode TargetField }
   // ......
   // { SourceNode Sourcefield TargetNode TargetField }
   command.Format("%s",acccessCmd);
   result = m_ParentGraph->Evaluate(command);
   int amount = m_ParentGraph->ParseRelationResults(result,&sourceNodeIDs,&sourceFieldIDs,&targetNodeIDs,&targetFieldIDs);
   for(i=0;i<amount;i++) {
	   CDisGraphNode* sourceNode;
	   CDisGraphNode* targetNode;
	   int sourceFieldNumber=-1;
	   int targetFieldNumber=-1;

	   // We need to locate source and target node to connect
       sourceNode = m_ParentGraph->NodeFromID(sourceNodeIDs[i]);
       targetNode = m_ParentGraph->NodeFromID(targetNodeIDs[i]);

	   // We will use ports on this nodes to connect fields properly.
	   // We will find port indexes using the returned fields IDs
	   if(sourceFieldIDs[i]!=-1)
	       sourceFieldNumber = sourceNode->FindFieldNumber(sourceFieldIDs[i]);
	   if(targetFieldIDs[i]!=-1)
	       targetFieldNumber = targetNode->FindFieldNumber(targetFieldIDs[i]);

       // TS API manipulations to connect new edge using ports 
	   // (will be implemented later)
       if((sourceNodeIDs[i]!=targetNodeIDs[i]) && (sourceFieldIDs[i]!=-1 || targetFieldIDs[i])) {
	       CDisGraphEdge* pEdge;
	       pEdge = (CDisGraphEdge *) m_ParentGraph->addEdge(sourceNode,targetNode);
	       pEdge->SetEdge(relationID,
		                  unselected!=NULL ? unselected : m_EdgeUnselected,
					      selected!=NULL   ? selected   : m_EdgeSelected,
					      focused!=NULL    ? focused    : m_EdgeFocused);
	       pEdge->PortRelation(sourceFieldNumber,targetFieldNumber);
	   }
   }
   // removing id arrays created by ParseRelationsResults
   if(amount>0) {
      delete sourceNodeIDs;
      delete sourceFieldIDs;
      delete targetNodeIDs;
      delete targetFieldIDs;
   }
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will remove all outgoing edges with relation type
// "relationID"
//------------------------------------------------------------------------------------
void CDisGraphNode::RemoveRelations(int relationID) {
TSDListIterator* edgeIterator=new TSDListIterator(pOutEdgeDList());
CDisGraphEdge* pEdge;

    while(*edgeIterator)  {
		pEdge = (CDisGraphEdge *) edgeIterator->pObject();
		if(pEdge->GetRelationID()==relationID) {
			m_ParentGraph->removeEdge(pEdge);
			delete edgeIterator;
            edgeIterator=new TSDListIterator(pOutEdgeDList());
			continue;
		}		
		(*edgeIterator)++;
	}
	delete edgeIterator;
	// We do not need re-referencing because we'v delete objects without server
	// global array elements
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Will run the extract fields script, will parse the results and
// will fill the items list with the resulted indexes. It will use
// field name extraction and field image extraction scripts to assing
// the name and image to the item.
//------------------------------------------------------------------------------------
void CDisGraphNode::ExtractFields(void) {
register i;
CString command;
CString result;
int* fieldIDs;
   
   // This script will create additional entities in the server
   m_FieldsList.RemoveAll();
   command.Format("%s %u",m_FieldsCreator, m_NodeID);
   result = m_ParentGraph->Evaluate(command);
   int nodesAmount = m_ParentGraph->ParseFieldResults(result,&fieldIDs);
   if(nodesAmount==0) return;
   for(i=0;i<nodesAmount;i++) {
	   // Inserting child nodes into the graph
	   CNodeItem item;
	   // Assigning ID to the node
	   item.m_FieldID=fieldIDs[i];
	   // Extracting the field name
       command.Format("%s %u",m_FieldNameExtractor,fieldIDs[i]);
	   result = m_ParentGraph->Evaluate(command);
	   item.m_FieldName=result;
	   // Extracting the field image index
       command.Format("%s %u",m_FieldImageExtractor,fieldIDs[i]);
	   result = m_ParentGraph->Evaluate(command);
	   item.m_FieldType=atoi(result);
	   m_FieldsList.AddTail(item);
   }
   delete fieldIDs;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Will remove all extracted fields from the fields list, as well as 
// from the server array. This function will use parent graph to
// modify the server array as well as all references from the graph
// node.
//------------------------------------------------------------------------------------
void CDisGraphNode::RemoveFields(void) {
	m_FieldsList.RemoveAll();
	m_ParentGraph->ReReference();
}
//------------------------------------------------------------------------------------
			

//------------------------------------------------------------------------------------
// This method will change node selection status
//------------------------------------------------------------------------------------
void CDisGraphNode::SetSelectionStatus(BOOL selected) {
	m_SelectStatus=selected;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This method will return node selection status
//------------------------------------------------------------------------------------
BOOL CDisGraphNode::GetSelectionStatus(void) {
	return m_SelectStatus;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This method will return this node identification number
//------------------------------------------------------------------------------------
int CDisGraphNode::GetNodeID(void) {
	return m_NodeID;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This methid will change this node identification number
//------------------------------------------------------------------------------------
void CDisGraphNode::SetNodeID(int to) {
	m_NodeID=to;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This method will search in the items list to locate the item
// with the selected id. It will return the number of this item (
// starting from list head) or -1 if no such field in the list.
//------------------------------------------------------------------------------------
int CDisGraphNode::FindFieldNumber(int id) {
int num=0;

	POSITION pos = m_FieldsList.GetHeadPosition();
	while(pos) {
		if(m_FieldsList.GetAt(pos).m_FieldID==id) return num;
		num++;
		m_FieldsList.GetNext(pos);
	}
	return -1;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Returns the readable name of this node
//------------------------------------------------------------------------------------
CString CDisGraphNode::GetReadableName(void) {
	return m_ReadableName;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Returns the node properties depending from the node state
// (unselected,selected or focused)
//------------------------------------------------------------------------------------
CNodeProperties* CDisGraphNode::GetActiveNodeProperties(void) {
	if(m_ParentGraph->GetFocusedNode()==this) return m_NodeFocused;
	if(m_SelectStatus==TRUE) return m_NodeSelected;
	return m_NodeUnselected;
}
//------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////
// This class represents the relation between graph nodes. Every relation has 
// it's id, passed to the constructor as "relationID". Thee edge properties pointers
// specify the way the edge will look if unselected, if selected or if focused.
// The edges are created from the node by calling "CreateChildNodes" method with the
// relation id = 0 (for parent-child relations) or by the "CreateRelations" method
// with relations ID  passed as a parameter. The edges will be removed if any node
// it connects is removed or by calling "RemoveRelations" node method with the 
// relation ID.
/////////////////////////////////////////////////////////////////////////////////////
CDisGraphEdge::CDisGraphEdge(CDisGraph* graph,
				             int relationID,
						     CEdgeProperties* unselected,
						     CEdgeProperties* selected,
							 CEdgeProperties* focused):TSEdge(graph) {
	m_ParentGraph  = graph;
	m_RelationID   = relationID;
	m_Unselected   = unselected;
    m_Selected     = selected;
	m_Focused      = focused;
	m_SelectStatus = FALSE;

}

CDisGraphEdge::CDisGraphEdge(TSDigraph* graph):TSEdge(graph) {
	m_ParentGraph  = (CDisGraph *)graph;
	m_RelationID   = -1;
	m_Unselected   = NULL;
    m_Selected     = NULL;
	m_Focused      = NULL;
	m_SelectStatus = FALSE;

}

CDisGraphEdge::CDisGraphEdge(TSPathDigraph* path):TSEdge(path) {
	m_ParentGraph  = (CDisGraph *)path;
	m_RelationID   = -1;
	m_Unselected   = NULL;
    m_Selected     = NULL;
	m_Focused      = NULL;
	m_SelectStatus = FALSE;

}

void CDisGraphEdge::SetEdge( int relationID,
						     CEdgeProperties* unselected,
						     CEdgeProperties* selected,
							 CEdgeProperties* focused) {
	m_RelationID   = relationID;
	m_Unselected   = unselected;
    m_Selected     = selected;
	m_Focused      = focused;
	m_SelectStatus = FALSE;

}


//------------------------------------------------------------------------------------
// This method implements the shared part of the memberwise
// initialization constructor and memberwise assignment constructors.
// To accomplish this, it calls each base class copy constructor to
// copy the shared elements of the input instance to this instance. 
// This method then copies the elements that are particular to this 
// class.
//------------------------------------------------------------------------------------
void CDisGraphEdge::TS_CLASS_UNIQUE_NAME(CDisGraphEdge, memberCopy)(
	CDisGraphEdge & rAppEdge)
{
	// "Effective C++" recommends this formulation to copy the shared
	// elements of these two instances.

	(TSEdge &) *this = rAppEdge;
	m_RelationID=rAppEdge.GetRelationID();
	m_ParentGraph=rAppEdge.GetParent();
	m_Focused=rAppEdge.GetFocused();
	m_Selected=rAppEdge.GetSelected();
	m_Unselected=rAppEdge.GetUnselected();
	m_SelectStatus=rAppEdge.GetSelectionStatus();
	return;
}
//------------------------------------------------------------------------------------



CDisGraphEdge::~CDisGraphEdge() {

}


CDisGraph* CDisGraphEdge::GetParent(void) {
	return m_ParentGraph;

}


//------------------------------------------------------------------------------------
// This function returns unselected edge drawing properties
//------------------------------------------------------------------------------------
CEdgeProperties* CDisGraphEdge::GetUnselected(void) {
	return m_Unselected;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function returns selected edge drawing properties
//------------------------------------------------------------------------------------
CEdgeProperties* CDisGraphEdge::GetSelected(void) {
	return m_Selected;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function returns focused edge drawing properties
//------------------------------------------------------------------------------------
CEdgeProperties* CDisGraphEdge::GetFocused(void) {
	return m_Focused;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This method will set edge selection status
//------------------------------------------------------------------------------------
void CDisGraphEdge::SetSelectionStatus(BOOL selected) {
	m_SelectStatus=selected;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This method will return the edge selection status
//------------------------------------------------------------------------------------
BOOL CDisGraphEdge::GetSelectionStatus(void) {
	return m_SelectStatus;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will determine the source and destionation nodes for this edge,
// will analyse the graph orientation, will create the appropriate amount of the  
// ports of source and destination node and will connect the edge to the appropriate 
// ports
//------------------------------------------------------------------------------------
void CDisGraphEdge::Port(void) {
int side;
CPoint sourceConnection;
CPoint targetConnection;
CDisGraphNode* pSource;
CDisGraphNode* pTarget;

    if(m_ParentGraph->m_HeaderPortCallback!=NULL) {
        pSource=(CDisGraphNode *)pFromNode();
        pTarget=(CDisGraphNode *)pToNode();
		if(pSource!=NULL && pTarget!=NULL) {
            switch(m_ParentGraph->pHDigraph()->pHTailor()->hlevelOrientation()) {
	            case TS_RIGHT_TO_LEFT : side=RIGHT_CONNECT;
			                            break;
	            case TS_LEFT_TO_RIGHT : side=LEFT_CONNECT;
			                            break;
	            case TS_BOTTOM_TO_TOP : side=BOTTOM_CONNECT;
				                        break;
	            case TS_TOP_TO_BOTTOM : side=TOP_CONNECT;
			                            break;
			}
            sourceConnection = (*(m_ParentGraph->m_HeaderPortCallback)) (
				                        m_ParentGraph->GetClientID(),
			                            pSource->GetReadableName(), 
		                                pSource->GetItemsList(), 
			                            pSource->GetActiveNodeProperties(),
						                side
					           );
            targetConnection = (*(m_ParentGraph->m_HeaderPortCallback)) (
				                        m_ParentGraph->GetClientID(),
			                            pTarget->GetReadableName(), 
		                                pTarget->GetItemsList(), 
			                            pTarget->GetActiveNodeProperties(),
						                side
					           );
	        pSource->pHNode()->numberOfOutPorts(sourceConnection.x);
	        pTarget->pHNode()->numberOfInPorts(targetConnection.x);
            pHEdge()->toPortNumber(targetConnection.y);
	        pHEdge()->fromPortNumber(sourceConnection.y);
		}
	}
}
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// This function will determine the source and destionation nodes for this edge,
// will analyse the graph orientation, will create the appropriate amount of the  
// ports of source and destination node and will connect the edge to the appropriate 
// ports
//------------------------------------------------------------------------------------
void CDisGraphEdge::PortRelation(int sourceField, int targetField) {
int side;
CPoint sourceConnection;
CPoint targetConnection;
CDisGraphNode* pSource;
CDisGraphNode* pTarget;

    if(m_ParentGraph->m_HeaderPortCallback!=NULL) {
        pSource=(CDisGraphNode *)pFromNode();
        pTarget=(CDisGraphNode *)pToNode();
		if(pSource!=NULL && pTarget!=NULL) {
            switch(m_ParentGraph->pHDigraph()->pHTailor()->hlevelOrientation()) {
	            case TS_RIGHT_TO_LEFT : side=RIGHT_CONNECT;
			                            break;
	            case TS_LEFT_TO_RIGHT : side=LEFT_CONNECT;
			                            break;
	            case TS_BOTTOM_TO_TOP : side=BOTTOM_CONNECT;
				                        break;
	            case TS_TOP_TO_BOTTOM : side=TOP_CONNECT;
			                            break;
			}
			if(m_ParentGraph->m_FieldPortCallback!=NULL) {
                sourceConnection = (*(m_ParentGraph->m_FieldPortCallback)) (
				                        m_ParentGraph->GetClientID(),
			                            pSource->GetReadableName(), 
		                                pSource->GetItemsList(), 
			                            pSource->GetActiveNodeProperties(),
										sourceField,
						                side										
					               );
                targetConnection = (*(m_ParentGraph->m_FieldPortCallback)) (
				                        m_ParentGraph->GetClientID(),
			                            pTarget->GetReadableName(), 
		                                pTarget->GetItemsList(), 
			                            pTarget->GetActiveNodeProperties(),
										targetField,
						                side
					               );
	            pSource->pHNode()->numberOfOutPorts(sourceConnection.x);
	            pTarget->pHNode()->numberOfInPorts(targetConnection.x);
                pHEdge()->toPortNumber(targetConnection.y);
	            pHEdge()->fromPortNumber(sourceConnection.y);
			}
		}
	}
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Returns the node properties depending from the node state
// (unselected,selected or focused)
//------------------------------------------------------------------------------------
CEdgeProperties* CDisGraphEdge::GetActiveEdgeProperties(void) {
	if(m_ParentGraph->GetFocusedEdge()==this) return m_Focused;
	if(m_SelectStatus==TRUE) return m_Selected;
	return m_Unselected;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This private function will parse the results of the node creation script send as 
// a string of '/n' separated node IDs. It will allocate memory for the array of
// node IDs and it will place all node ID's in this array. It is your task to free
// this array when no longer needed.
//------------------------------------------------------------------------------------
int CDisGraph::ParseCreationResults(CString &str, int** IDs) {
register i;
int     idCounter;
CString val;

    idCounter=0;
	for(i=0;i<str.GetLength();i++) {
		if(str[i]=='\n') idCounter++;
	}
	if(idCounter==0) return 0;
	*IDs = new int [idCounter];
    idCounter=0;
	val="";
	for(i=0;i<str.GetLength();i++) {
		if(str[i]=='\n') {
			int id = atoi(val);
			(*IDs)[idCounter]=id;
			val="";
			idCounter++;
			continue;
		}
		val+=str[i];
	}
	return idCounter;

}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This private function will parse the results of the relations script send as 
// a string of '/n' separated 4-digit IDs like shown:
// <source node> <source field> <target node> <target field> \n
//  .........................................................
// <source node> <source field> <target node> <target field> \n
// It will allocate memory for the array of
// node IDs and it will place all node ID's in this array. It is your task to free
// this array when no longer needed.
//------------------------------------------------------------------------------------
int CDisGraph::ParseRelationResults(CString& str, int** srcNodeIDs, int** srcFieldIDs, int** targetNodeIDs, int** targetFieldIDs) {
register i;
int     idCounter;
int     colCounter;
CString val[4];

    idCounter=0;
	for(i=0;i<str.GetLength();i++) {
		if(str[i]=='\n') idCounter++;
	}
	if(idCounter==0) return 0;
	*srcNodeIDs     = new int [idCounter];
	*srcFieldIDs    = new int [idCounter];
	*targetNodeIDs  = new int [idCounter];
	*targetFieldIDs = new int [idCounter];
    idCounter=0;
	colCounter=0;
	val[0]="";
	val[1]="";
	val[2]="";
	val[3]="";
	for(i=0;i<str.GetLength();i++) {
		if((str[i]==' ' || str[i]==':') && (colCounter<3)) {
			colCounter++;
			continue;
		}
		if(str[i]=='\n') {
			int id1 = atoi(val[0]);
			int id2 = atoi(val[1]);
			int id3 = atoi(val[2]);
			int id4 = atoi(val[3]);
			(*srcNodeIDs)[idCounter]=id1;
			(*srcFieldIDs)[idCounter]=id2;
			(*targetNodeIDs)[idCounter]=id3;
			(*targetFieldIDs)[idCounter]=id4;
			val[0]="";
			val[1]="";
			val[2]="";
			val[3]="";
			if(colCounter>=3) idCounter++;
			colCounter=0;
			continue;
		}
		val[colCounter]+=str[i];
	}
	return idCounter;

}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This private function will parse the results of the field creation script send as 
// a string of '/n' separated field IDs. It will allocate memory for the array of
// node IDs and it will place all node ID's in this array. It is your task to free
// this array when no longer needed.
//------------------------------------------------------------------------------------
int CDisGraph::ParseFieldResults(CString &str, int** IDs) {
register i;
int     idCounter;
CString val;

    idCounter=0;
	for(i=0;i<str.GetLength();i++) {
		if(str[i]=='\n') idCounter++;
	}
	if(idCounter==0) return 0;
	*IDs = new int [idCounter];
    idCounter=0;
	val="";
	for(i=0;i<str.GetLength();i++) {
		if(str[i]=='\n') {
			int id = atoi(val);
			(*IDs)[idCounter]=id;
			val="";
			idCounter++;
			continue;
		}
		val+=str[i];
	}
	return idCounter;

}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// We will use this method to pass browser selection into the graph. It will call
// selection script which is passed into this function in the "selProc" string and
// it will create the unconnected nodes in the graph. It will also call 
// "CreateChildNodes" on the selected nodes to expand them one level.
//------------------------------------------------------------------------------------
void CDisGraph::CreateGraph(CString& selProc,
							CString& accessChildrenCreator,
							CString& accessParentsCreator,
					        CString& accessFieldsCreator,
	                        CString& accessNodeName,
					        CString& accessFieldName,
					        CString& accessNodeImage,
					        CString& accessFieldImage,
                            CString& accessTestChildren,        
                            CString& accessTestParents,     

					        CNodeProperties* nodeUnselected,
					        CNodeProperties* nodeSelected,
					        CNodeProperties* nodeFocused,
		                    CEdgeProperties* edgeUnselected,
					        CEdgeProperties* edgeSelected,
					        CEdgeProperties* edgeFocused) {
CString result;
int* nodeIDs;

   result = Evaluate(selProc);
   int nodesAmount = ParseCreationResults(result,&nodeIDs);
   if(nodesAmount==0) return;
   for(register i=0;i<nodesAmount;i++) {
	   // Inserting child nodes into the graph
	   CDisGraphNode* pNode;
	   pNode = (CDisGraphNode *) addNode();

       pNode->SetNode(nodeIDs[i],           // entity index in the server array
			          accessChildrenCreator,// script to create childs
                      accessParentsCreator, // script to create parents
				      accessFieldsCreator,  // script to create fields
				      accessNodeName,       // script to extract name
				      accessFieldName,      // script to extract field name
				      accessNodeImage,      // script to extract image
					  accessFieldImage,     // script to extract field image
				      accessTestChildren,   // script to test if has children
					  accessTestParents,    // script to test if has parents
					  nodeUnselected,       // unselected style
			          nodeSelected,         // selected style
					  nodeFocused,          // focused style
					  edgeUnselected,       // unselected style
			          edgeSelected,         // selected style
					  edgeFocused );        // focused style



	   pNode->ExtractFields();
	   pNode->ExtractName();
       pNode->ExtractNodeKind();
	   pNode->ExtractParentChildEnable();
//	   pNode->CreateChildNodes();
   }
  // removing new nodes IDs array created by ParseCreationResults
  delete nodeIDs;

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This will return a pointer to the node items list.
//------------------------------------------------------------------------------------
CItemsList* CDisGraphNode::GetItemsList(void) {
   return &m_FieldsList;
}

//------------------------------------------------------------------------------------
// Will return the relation ID assigned in constructor
//------------------------------------------------------------------------------------
int CDisGraphEdge::GetRelationID(void) {
	return m_RelationID;
}


TS_ONE_BASE_CLASSBODY(CDisGraph,TSDigraph)

//------------------------------------------------------------------------------------
// This macro incorporates several hundred lines of boiler plate source
// code that all Graph Layout Toolkit classes share.
//------------------------------------------------------------------------------------
TS_ONE_BASE_CLASSBODY(CDisGraphEdge,TSEdge)
//------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// This macro incorporates several hundred lines of boiler plate source
// code that all Graph Layout Toolkit classes share.
//-----------------------------------------------------------------------------------
TS_ONE_BASE_CLASSBODY(CDisPathGraph,TSPathDigraph)
//-----------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This macro incorporates several hundred lines of boiler plate source
// code that all Graph Layout Toolkit classes share.
//------------------------------------------------------------------------------------
TS_ONE_BASE_CLASSBODY(CDisGraphNode,TSNode)
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function will be called from the view every time the user press mouse
// button.
//------------------------------------------------------------------------------------
BOOL CDisGraph::SetFocus(int x, int y) {
TSDList touchingNodeDList;   
TSDList touchingEdgeDList;
TSDList touchingLabelDList;
TSRect  queryRect;   
CDisGraphNode* pNode;
CDisGraphNode* pOldFocus;
CItemsList* nodeItems;
CRect nodeRect;

    setRectX(&queryRect,x);
    setRectY(&queryRect,y);
    setRectWidth(&queryRect,600);
    setRectHeight(&queryRect,600);

    pOldFocus=m_NodeWithFocus;

    buildDListsTouchingRect(&queryRect,
                            &touchingNodeDList,    
							&touchingEdgeDList,
                            &touchingLabelDList,    
							FALSE);
    TSDListIterator nodeIterator(&touchingNodeDList);
    if(nodeIterator)  {
         pNode= (CDisGraphNode *)nodeIterator.pObject();
		 // Reading this node rectanfle
         nodeRect.top=pNode->top();
		 nodeRect.bottom=pNode->bottom();
		 nodeRect.left=pNode->left();
		 nodeRect.right=pNode->right();

		 // If point is inside the node rectangle  - we will set focus to this rectangle.
		 // If the point is outside node rectangle - we will try to expand/collapse
		 // the node if within expand/collapse rectangle
         if(x>nodeRect.left && x<nodeRect.right && y<nodeRect.top && y>nodeRect.bottom) {
		     m_NodeWithFocus=pNode;
		     // Determine wether to display expand/collapse buttons and where
	         BOOL status;
		     int connectorSide=NONE;
		     if(pNode->m_ChildNode && pNode->HasChildren()==TRUE) {
		        if(pNode->AreChildrenCreated()==TRUE)  {
		            if(pNode->IsCollapsed()==TRUE) status=TRUE;
		            else                           status=FALSE;
				} else status=TRUE;
			    switch(pHDigraph()->pHTailor()->hlevelOrientation()) {
			        case TS_LEFT_TO_RIGHT : connectorSide = RIGHT;  break;
			        case TS_RIGHT_TO_LEFT : connectorSide = LEFT;   break;
			        case TS_TOP_TO_BOTTOM : connectorSide = BOTTOM; break;
			        case TS_BOTTOM_TO_TOP : connectorSide = TOP;    break;
				}
			 } 
			 // Determine wether to display expand/collapse buttons for parents
             int parentsConnectorSide=NONE;
             BOOL parentsStatus;
			 if(pNode->m_ParentNode && pNode->HasParents()==TRUE) {
				if(pNode->AreParentsCreated()==TRUE)  {
					if(pNode->IsParentsCollapsed()==TRUE) parentsStatus=TRUE;
					else                                  parentsStatus=FALSE;
				} else parentsStatus=TRUE;
				switch(pHDigraph()->pHTailor()->hlevelOrientation()) {
				    case TS_LEFT_TO_RIGHT : parentsConnectorSide = LEFT;  break;
				    case TS_RIGHT_TO_LEFT : parentsConnectorSide = RIGHT; break;
				    case TS_TOP_TO_BOTTOM : parentsConnectorSide = TOP;   break;
				    case TS_BOTTOM_TO_TOP : parentsConnectorSide = BOTTOM;break;
				}
			 } 
             // Highlighting the new node
	         nodeItems=pNode->GetItemsList();
             (*m_NodeDrawCallback)(m_ClientID,nodeRect,pNode->GetReadableName(),pNode->GetNodeKind(),nodeItems,pNode->GetActiveNodeProperties(),connectorSide,status,parentsConnectorSide,parentsStatus) ;
		 } else {
			 // If source connector rectangle exists
			 CRect connectorRect;
			 if(pNode->m_ChildNode && pNode->GetSourceConnectorRect(connectorRect)) {
				 // If mouse was pressed in the source connector rect
				 if(x>connectorRect.left && x<connectorRect.right && y<connectorRect.top && y>connectorRect.bottom) {
					 // Performing expand.collapse
                     if(pNode->AreChildrenCreated()==FALSE) {
                          pNode->CreateChildNodes();
					 } else {
		                  if(pNode->IsCollapsed()==TRUE) pNode->Expand();
		                  else                           pNode->Collapse();

					 }
					 return TRUE;
				 }
			 }
			 if(pNode->m_ParentNode && pNode->GetTargetConnectorRect(connectorRect)) {
				 // If mouse was pressed in the source connector rect
				 if(x>connectorRect.left && x<connectorRect.right && y<connectorRect.top && y>connectorRect.bottom) {
					 // Performing expand.collapse
                     if(pNode->AreParentsCreated()==FALSE) {
                          pNode->CreateParentNodes();
					 } else {
		                  if(pNode->IsParentsCollapsed()==TRUE) pNode->ExpandParents();
		                  else                                  pNode->CollapseParents();

					 }
					 return TRUE;
				 }
			 }
			 return FALSE;
		 }
	}
	// hiding old focus, if any
	if(pOldFocus!=NULL) {
	    nodeItems=pOldFocus->GetItemsList();
        nodeRect.top=pOldFocus->top();
	    nodeRect.bottom=pOldFocus->bottom();
		nodeRect.left=pOldFocus->left();
		nodeRect.right=pOldFocus->right();
		// Determine wether to display expand/collapse buttons and where
		BOOL status;
		int connectorSide=NONE;
		if(pOldFocus->m_ChildNode && pOldFocus->HasChildren()==TRUE) {
		     if(pOldFocus->AreChildrenCreated()==TRUE)  {
			     if(pOldFocus->IsCollapsed()==TRUE) status=TRUE;
				  else                              status=FALSE;
			  } else status=TRUE;
			  switch(pHDigraph()->pHTailor()->hlevelOrientation()) {
			      case TS_LEFT_TO_RIGHT : connectorSide = RIGHT;  break;
			      case TS_RIGHT_TO_LEFT : connectorSide = LEFT;   break;
			      case TS_TOP_TO_BOTTOM : connectorSide = BOTTOM; break;
			      case TS_BOTTOM_TO_TOP : connectorSide = TOP;    break;
			  }
		} 
		// Determine wether to display expand/collapse buttons for parents
        int parentsConnectorSide=NONE;
        BOOL parentsStatus;
	    if(pOldFocus->m_ParentNode && pOldFocus->HasParents()==TRUE) {
			if(pOldFocus->AreParentsCreated()==TRUE)  {
				if(pOldFocus->IsParentsCollapsed()==TRUE) parentsStatus=TRUE;
				else                                      parentsStatus=FALSE;
			} else parentsStatus=TRUE;
			switch(pHDigraph()->pHTailor()->hlevelOrientation()) {
			    case TS_LEFT_TO_RIGHT : parentsConnectorSide = LEFT;  break;
			    case TS_RIGHT_TO_LEFT : parentsConnectorSide = RIGHT; break;
			    case TS_TOP_TO_BOTTOM : parentsConnectorSide = TOP;   break;
			    case TS_BOTTOM_TO_TOP : parentsConnectorSide = BOTTOM;break;
			}
		} 
        (*m_NodeDrawCallback)(m_ClientID,nodeRect,pOldFocus->GetReadableName(),pOldFocus->GetNodeKind(),nodeItems,pOldFocus->GetActiveNodeProperties(),connectorSide,status,parentsConnectorSide,parentsStatus) ;
	}
	return FALSE;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will return TRUE if the node children was already created and
// FALSE if not;
//------------------------------------------------------------------------------------
BOOL CDisGraphNode::AreChildrenCreated() {
     return m_ChildrenCreated;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void CDisGraphNode::SetChildrenCreated(BOOL val) {
     m_ChildrenCreated=TRUE;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will return TRUE if the node parent was already created and
// FALSE if not;
//------------------------------------------------------------------------------------
BOOL CDisGraphNode::AreParentsCreated() {
     return m_ParentsCreated;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void CDisGraphNode::SetParentsCreated(BOOL val) {
     m_ParentsCreated=val;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will recursively hide node parents.
//------------------------------------------------------------------------------------
void CDisGraphNode::HideChildren(void) {
TSDList newNodeDList;
TSDList newEdgeDList;
CDisGraphNode* pNode;

   findChildren(&newNodeDList,&newEdgeDList,1);
   TSDListIterator nodeIterator(&newNodeDList);
   while(nodeIterator) {
	   pNode = (CDisGraphNode *) nodeIterator.pObject();
	   pNode->HideChildren();
	   nodeIterator++;
   }
   hideChildren(&newNodeDList,&newEdgeDList,1);
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will hide all node children
//------------------------------------------------------------------------------------
void CDisGraphNode::Collapse() {
TSDList newNodeDList;
TSDList newEdgeDList;

     m_Collapsed=TRUE;
     hideChildren(&newNodeDList,&newEdgeDList,10000);
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will show all node children
//------------------------------------------------------------------------------------
void CDisGraphNode::Expand() {
TSDList newNodeDList;
TSDList newEdgeDList;
CDisGraphNode* pNode;

   unhideChildren(&newNodeDList,&newEdgeDList,1);
   m_Collapsed=FALSE;
   TSDListIterator nodeIterator(&newNodeDList);
   while(nodeIterator) {
	   pNode = (CDisGraphNode *) nodeIterator.pObject();
	   if(pNode->IsCollapsed()==FALSE) pNode->Expand();
	   nodeIterator++;
   }


}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will recursively hide node parents.
//------------------------------------------------------------------------------------
void CDisGraphNode::HideParents(void) {
TSDList newNodeDList;
TSDList newEdgeDList;
CDisGraphNode* pNode;

   findParents(&newNodeDList,&newEdgeDList,1);
   TSDListIterator nodeIterator(&newNodeDList);
   while(nodeIterator) {
	   pNode = (CDisGraphNode *) nodeIterator.pObject();
	   pNode->HideParents();
	   nodeIterator++;
   }
   hideParents(&newNodeDList,&newEdgeDList,1);
}
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// This function will hide all node parents, the first one will be marked as collapsed
//------------------------------------------------------------------------------------
void CDisGraphNode::CollapseParents() {
TSDList newNodeDList;
TSDList newEdgeDList;

   m_ParentsCollapsed=TRUE;
   hideParents(&newNodeDList,&newEdgeDList,10000);
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function will show all node parents
//------------------------------------------------------------------------------------
void CDisGraphNode::ExpandParents() {
TSDList newNodeDList;
TSDList newEdgeDList;
CDisGraphNode* pNode;

   unhideParents(&newNodeDList,&newEdgeDList,1);
   m_ParentsCollapsed=FALSE;
   TSDListIterator nodeIterator(&newNodeDList);
   while(nodeIterator) {
	   pNode = (CDisGraphNode *) nodeIterator.pObject();
	   if(pNode->IsParentsCollapsed()==FALSE) pNode->ExpandParents();
	   nodeIterator++;
   }

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function will return TRUE if the node is collapsed;
//------------------------------------------------------------------------------------
BOOL CDisGraphNode::IsCollapsed() {
	return m_Collapsed;

}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function will return TRUE if the node parents are is collapsed;
//------------------------------------------------------------------------------------
BOOL CDisGraphNode::IsParentsCollapsed() {
	return m_ParentsCollapsed;

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Checks if this node has a children
//------------------------------------------------------------------------------------
BOOL CDisGraphNode::HasChildren(void) {
	return m_HasChildren;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Checks if this node has a parents
//------------------------------------------------------------------------------------
BOOL CDisGraphNode::HasParents(void) {
	return m_HasParents;
}
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// This private function calculates if parents or childs are enabled.
//------------------------------------------------------------------------------------
void CDisGraphNode::ExtractParentChildEnable() {
CString command;
CString result;

   command.Format("%s %u",m_ChildTestScript,m_NodeID);
   result = m_ParentGraph->Evaluate(command);
   if(atoi(result)>0) m_HasChildren=TRUE;
   else               m_HasChildren=FALSE;

   command.Format("%s %u",m_ParentTestScript,m_NodeID);
   result = m_ParentGraph->Evaluate(command);
   if(atoi(result)>0) m_HasParents=TRUE;
   else               m_HasParents=FALSE;


}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function returns TRUE and sets the bounding rectangle for the expand/collapse
// control (node connection with children). Returns FALSE if this node has no children.
//------------------------------------------------------------------------------------
BOOL CDisGraphNode::GetSourceConnectorRect(CRect &rect) {
CPoint topLeft;
    
	if(m_HasChildren!=FALSE) {
        switch(m_ParentGraph->pHDigraph()->pHTailor()->hlevelOrientation()) {
	        case TS_RIGHT_TO_LEFT  : topLeft.x=left()-300;
			                         topLeft.y=top()-m_HeaderHeight/2+150;
					                 break;
	        case TS_LEFT_TO_RIGHT :  topLeft.x=right();
			                         topLeft.y=top()-m_HeaderHeight/2+150;
					                 break;
	        case TS_BOTTOM_TO_TOP :  topLeft.x=left()+width()/2-150;
			                         topLeft.y=top();
					                 break;
	        case TS_TOP_TO_BOTTOM :  topLeft.x=left()+width()/2-150;
			                         topLeft.y=bottom();
					                 break;
		}
		rect.left=topLeft.x;
		rect.top=topLeft.y;
		rect.right=rect.left+300;
		rect.bottom=rect.top-300;
		return TRUE;
	}
	rect.left=rect.right=rect.top=rect.bottom=0;
	return FALSE;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function returns TRUE and sets the bounding rectangle for the expand/collapse
// control (node connection with parents). Returns FALSE if this node has no children.
//------------------------------------------------------------------------------------
BOOL CDisGraphNode::GetTargetConnectorRect(CRect &rect) {
CPoint topLeft;
    
	if(m_HasParents!=FALSE) {
        switch(m_ParentGraph->pHDigraph()->pHTailor()->hlevelOrientation()) {
	        case TS_LEFT_TO_RIGHT :  topLeft.x=left()-300;
			                         topLeft.y=top()-m_HeaderHeight/2+150;
					                 break;
	        case TS_RIGHT_TO_LEFT :  topLeft.x=right();
			                         topLeft.y=top()-m_HeaderHeight/2+150;
					                 break;
	        case TS_TOP_TO_BOTTOM  :  topLeft.x=left()+width()/2-150;
			                         topLeft.y=top()+300;
					                 break;
	        case TS_BOTTOM_TO_TOP :  topLeft.x=left()+width()/2-150;
			                         topLeft.y=bottom();
					                 break;
		}
		rect.left=topLeft.x;
		rect.top=topLeft.y;
		rect.right=rect.left+300;
		rect.bottom=rect.top-300;
		return TRUE;
	}
	rect.left=rect.right=rect.top=rect.bottom=0;
	return FALSE;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This private function will search all graph trying to find the node with the
// specified ID. Returns pointer to this node or NULL if not found.
//------------------------------------------------------------------------------------
CDisGraphNode* CDisGraph::FindNodeWithID(int id) {
CDisGraphNode* pNode;
   TSDListIterator nodeIterator(pNodeDList());
   while(nodeIterator) {
       pNode = CDisGraphNode::castDown(nodeIterator.pObject());
	   if(pNode->GetNodeID()==id) return pNode;
	   nodeIterator++;
   }
   return NULL;

}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This public function will port all edges using current graph direction.
//------------------------------------------------------------------------------------
void CDisGraph::PortAllEdges() {
TSDListIterator edgeIterator(pEdgeDList());
CDisGraphEdge* pEdge;

	// Porting all edges
	if(m_EdgeDrawCallback!=NULL) {
        while(edgeIterator)  {
		    pEdge = CDisGraphEdge::castDown(edgeIterator.pObject());
			pEdge->Port();
		    edgeIterator++;
		}
	}

}
//------------------------------------------------------------------------------------

void CDisGraphNode::ExtractNodeKind() {
CString cmd;
CString str;

      cmd.Format("%s %u",m_NodeImageExtractor,m_NodeID);
	  str = m_ParentGraph->Evaluate(cmd);
	  m_Kind = atoi(str);
}

int CDisGraphNode::GetNodeKind() {
	return m_Kind;

}

int CDisGraph::GetClientID() {
	return m_ClientID;

}

void CDisGraph::SetClientID(int id) {
	m_ClientID=id;

}

//------------------------------------------------------------------------------------
// This public function will update all nodes  (visible and hidden) in the graph.
// It will update node fields, node name and node image index.
//------------------------------------------------------------------------------------
void CDisGraph::Refresh() {
CDisGraphNode* pNode;

    // Refreshing all visible nodes
	TSDListIterator nodeIterator(pNodeDList());
    while(nodeIterator)  {
		pNode = (CDisGraphNode *) nodeIterator.pObject();
		pNode->Refresh();
		nodeIterator++;
	}
	// Refreshing all invisible nodes
    TSDListIterator hideIterator(pHideDigraph()->pNodeDList());
    while(hideIterator)  {
		pNode = (CDisGraphNode *) hideIterator.pObject();
		pNode->Refresh();
		hideIterator++;
	}
    // updating server array
	ReReference();

}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This public function will update all node elements by running access queries.
//------------------------------------------------------------------------------------
void CDisGraphNode::Refresh() {
	ExtractFields();
	ExtractName();
	ExtractNodeKind();
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function will inspect all visible and hidden nodes and will try to create
// relation edges with relations described in "name".
//------------------------------------------------------------------------------------
void CDisGraph::CreateRelation(int id,
							   CEdgeProperties* unselected, 
							   CEdgeProperties* selected,
							   CEdgeProperties* focused) {
CDisGraphNode* pNode;
CString relationType;
CString cmd;


    // Refreshing all visible nodes
    relationType.LoadString(id);
	TSDListIterator nodeIterator(pNodeDList());
    while(nodeIterator)  {
		pNode = (CDisGraphNode *) nodeIterator.pObject();
		cmd.Format("%s %d \"%s\"",m_RelationsScript,pNode->GetNodeID(),relationType);
		pNode->CreateRelations(cmd,id,unselected,selected,focused);
		nodeIterator++;
	}
	// Refreshing all invisible nodes
    TSDListIterator hideIterator(pHideDigraph()->pNodeDList());
    while(hideIterator)  {
		pNode = (CDisGraphNode *) hideIterator.pObject();
		cmd.Format("%s %d \"%s\"",m_RelationsScript,pNode->GetNodeID(),relationType);
		pNode->CreateRelations(cmd,id,unselected,selected,focused);
		hideIterator++;
	}

}
//------------------------------------------------------------------------------------

void CDisGraph::SetRelationsScript(CString &script) {
	m_RelationsScript=script;

}

void CDisGraph::RemoveRelations(int id) {
CDisGraphNode* pNode;

    // Refreshing all visible nodes
	TSDListIterator nodeIterator(pNodeDList());
    while(nodeIterator)  {
		pNode = (CDisGraphNode *) nodeIterator.pObject();
		pNode->RemoveRelations(id);
		nodeIterator++;
	}
	// Refreshing all invisible nodes
    TSDListIterator hideIterator(pHideDigraph()->pNodeDList());
    while(hideIterator)  {
		pNode = (CDisGraphNode *) hideIterator.pObject();
		pNode->RemoveRelations(id);
		hideIterator++;
	}

}
