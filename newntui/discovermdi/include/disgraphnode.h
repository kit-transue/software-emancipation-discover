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
// Defining this symbols we are attaching appropriate layout libraries to our 
// project
#ifndef __DATASOURCE_H
   #define __DATASOURCE_H
   #include "datasource.h"
#endif
#define TS_COMPILE_CIRCULAR
#define TS_COMPILE_HIERARCHICAL
#define TS_COMPILE_ORTHOGONAL
#define TS_COMPILE_SYMMETRIC

#include "utils/tsasciif.h"
#include "config/tsinit.h"
#include "drawing/tsdigrap.h"

#ifdef TS_COMPILE_CIRCULAR
  #include "clayout/tscgraph.h"
#endif

#ifdef TS_COMPILE_HIERARCHICAL
  #include "hlayout/tshdigra.h"
#endif

#ifdef TS_COMPILE_ORTHOGONAL
  #include "olayout/tsodigra.h"
#endif

#ifdef TS_COMPILE_SYMMETRIC
  #include "slayout/tssgraph.h"
#endif

#include "GraphView.h"

typedef void  (*CNodeDrawCallback)(int,CRect&,CString&,int,CItemsList*,CNodeProperties*,int side, BOOL status,int parentsSide, BOOL parentsStatus);
typedef void   (*CEdgeDrawCallbcak)(int,CPoint*, int,CEdgeProperties*);
typedef CSize  (*CNodeSizeCallback)(int,CString&,CItemsList*,CNodeProperties*, int& headerHeight) ;
typedef CPoint (*CHeaderPortCallback)(int,CString&,CItemsList*,CNodeProperties*,int side);
typedef CPoint (*CFieldPortCallback)(int,CString& nodeName,CItemsList* list,CNodeProperties* nodeProp,int index,int side);

class CDisGraphNode;
class CDisGraphEdge;

//-----------------------------------------------------------------------------------
// Gaph class represents the collection of nodes and it's relations. It also provides
// the server communication channel and can connect Discover entities in the
// global server array with client nodes and it's fields.
//-----------------------------------------------------------------------------------
#ifdef __cplusplus
#define APP_CLASS
class APP_CLASS CDisGraph : public TSDigraph {
	TS_CLASSDEF(CDisGraph)
    public: // Constructor will assign NULL to all pointers
		    CDisGraph();
	    
            void CreateGraph(CString& selProc,
							 CString& accessChildrenCreator,
							 CString& accessParentsCreator,
					         CString& accessFieldsCreator,
	                         CString& accessNodeName,
					         CString& accessFieldName,
					         CString& accessNodeImage,
			                 CString& accessFieldImage,
		                     CString& accessTestChildern,        
                             CString& accessTestParents,     

					         CNodeProperties* nodeUnselected,
					         CNodeProperties* nodeSelected,
					         CNodeProperties* nodeFocused,
		                     CEdgeProperties* edgeUnselected,
					         CEdgeProperties* edgeSelected,
					         CEdgeProperties* edgeFocused);

			// Returns the currently focused node or NULL if no such node
		    CDisGraphNode* GetFocusedNode(void);
			// Sets the node which will have focus
		    void SetFocusedNode(CDisGraphNode* node);
			// Returns the pointer to the edge that currently has focus.
			CDisGraphEdge* GetFocusedEdge(void);
			// Set the pointer to the edge that has focus
			void SetFocusedEdge(CDisGraphEdge *);
			// Will walk and re-reference all nodes and fields to have up-to-date
			// IDs. We need this when something is removed and we need to rebuild
			// global server array.
			void ReReference(void);
			// This method will assign a data source to the graph. Default data
			// source pointer is NULL, so any attempt to evaluate server script
			// by calling "Evaluate" method will fail.
			void SetDataSource(CDataSource* connection);
			// This method will return a pointer to the data source assigned to 
			// the graph. 
			CDataSource* GetDataSource(void);
			// This method evaluates the string passed as "cmd" parameter on the 
			// server and returns evaluation results as a string. Will do nothing
			// if no data source assigned to the graph.
			CString Evaluate(CString& cmd);
			// This method will find the node in the graph with specified ID
			CDisGraphNode* NodeFromID(int id);

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
			void SetNodeDrawCallback(CNodeDrawCallback nodeDrawCallback);


			// This method sets the edge drawing callback function. 
			// If you want the integration of the layout graph with the client
			// graphical view you need to implement edge drawing function 
			// which will receve the edge path as an array of CPoint'ers, amount,
			// of vertexes in a path and edge style.
			//  Example:
	        // void DrawEdge(CPoint* vertexes, 
		    //               int vertexAmount, 
			//               CEdgeProperties* edgeProperties);
			void SetEdgeDrawCallback(CEdgeDrawCallbcak edgeDrawCallback);



			// This method sets the node size calculation callback function. 
			// If you want the integration of the layout graph with the client
			// graphical view you need to implement node size calculation function 
			// which will receve node name, node fields list and node properties
			// and will return the size of the node . Examlpe:
	        //  CSize CalculateNodeSize(CString&    nodeName, 
            //                          CItemsList* nodeItems, 
			//    				        CNodeProperties* nodeProp);
			void SetNodeSizeCalculationCallback(CNodeSizeCallback nodeSizeCallback);


            // This method will set the node header port calculation callback.
			// This callback will call graph view to determine the amount of ports 
			// on the defined side and the port to connect to. The ports amount will
			// be placed in the x field of the returned function, the port to connect to
			// will be placed into the y field of the return structure.
			void SetNodeHeaderPortCalculationCallback(CHeaderPortCallback headerPort);


            // This method will set the specified field port calculation callback.
			// This callback will call graph view to determine the amount of ports 
			// on the defined side and the port to connect to. The ports amount will
			// be placed in the x field of the returned function, the port to connect to
			// will be placed into the y field of the return structure.
			void SetNodeFieldPortCalculationCallback(CFieldPortCallback  fieldPort);


			// This method will call node size calculation callback to determine the
			// desired size of every node in the graph.
			void DetermineNodeSizes(void);

			// This method will call node draw and edge draw callbacks when we need
			// to draw all graph. Use "DrawGraphRect" method to draw the nodes
			// and edges contained in the rectangle.
			void DrawGraph(void);

			// This method will try to find the node or edge located in near the
			// specified point

			int ParseCreationResults(CString& str, int** IDs);
	        int ParseRelationResults(CString& str, int** srcNodeIDs, int** srcFieldIDs, int** targetNodeIDs, int** targetFieldsIDs);
	        int ParseFieldResults(CString& str, int** IDs);


            // This method acts as a "virtual constructor" to allocate a graph
            // object. Developers who implement classes that derive behavior from
            // class TSDigraph should implement this method. void
            virtual TSDigraph * newDigraph(void);

            // This method acts as a "virtual constructor" to allocate an edge
            // object. Developers who implement classes that derive behavior from
            // class TSDigraph should implement this method if they also implement
            // a class that derives from TSEdge.
			virtual TSEdge * newEdge(void);

	        // This method acts as a "virtual constructor" to allocate a node
            // object. Developers who implement classes that derive behavior from
            // class TSDigraph should implement this method if they also implement
            // a class that derives from TSNode.
			virtual TSNode * newNode(void);
            virtual TSPathDigraph * newPathDigraph(TSEdge * pOwnerEdge);
			// This function will try to focus object located at the
			// (x,y) position
	        BOOL SetFocus(int x, int y);
            // This private function will search all graph trying to find 
            // the node with the specified ID. Returns pointer to this node 
            // or NULL if not found.
            CDisGraphNode* FindNodeWithID(int id);
public:
	void RemoveRelations(int id);
	void SetRelationsScript(CString& script);
	void CreateRelation(int id,CEdgeProperties* selected, CEdgeProperties* unselected, CEdgeProperties* focused);
	void Refresh(void);
	void SetClientID(int id);
	int GetClientID(void);
	        void PortAllEdges(void);
			CHeaderPortCallback m_HeaderPortCallback;
			CFieldPortCallback  m_FieldPortCallback;

private:
	CString m_RelationsScript;
	int m_ClientID;

            CDisGraphNode* m_NodeWithFocus;
			CDisGraphEdge* m_EdgeWithFocus;
			CDataSource*   m_DataSource;

			CNodeDrawCallback   m_NodeDrawCallback;
			CEdgeDrawCallbcak   m_EdgeDrawCallback;
			CNodeSizeCallback   m_NodeSizeCallback;
};
#else

TS_CAPI_CLASSDEF(CDisGraph)

#endif

#ifndef TS_EXCLUDE_C_API
#ifdef __cplusplus
extern "C"
{
#endif

	TS_EXPORT_PRE_FUNC CDisGraph * TS_EXPORT_POST_FUNC CDisGraphCastDown(
		TSObject * pThisObject);

#ifdef __cplusplus
}
#endif
#endif

//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
#ifdef __cplusplus
#define APP_CLASS
class APP_CLASS CDisPathGraph: public TSPathDigraph
{
	TS_CLASSDEF(CDisPathGraph)

	public:
		CDisPathGraph(TSEdge * pOwnerEdge);

	protected:
		virtual TSEdge * newEdge();
		virtual TSNode * newNode();
	
	private:
};
#else

TS_CAPI_CLASSDEF(DisPathGraph)

#endif


#ifndef TS_EXCLUDE_C_API
#ifdef __cplusplus
extern "C"
{
#endif

	TS_EXPORT_PRE_FUNC CDisPathGraph * TS_EXPORT_POST_FUNC CDisPathGraphCastDown(
		TSObject * pThisObject);

#ifdef __cplusplus
}
#endif
#endif

//-----------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------
#ifdef __cplusplus
#define APP_CLASS
class APP_CLASS CDisGraphNode : public TSNode {
	TS_CLASSDEF(CDisGraphNode)
    public : CDisGraphNode(TSDigraph* graph);
	    void Refresh(void);
	    int GetNodeKind(void);
	    void ExtractNodeKind(void);
	         int m_HeaderHeight;
	         BOOL m_ParentNode;
	         BOOL m_ChildNode;
	         BOOL GetSourceConnectorRect(CRect& rect);
	         BOOL GetTargetConnectorRect(CRect& rect);
	         CDisGraphNode(TSPathDigraph* graph);

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
             void SetNode(int id,                        // entity index in the server array
						 CString& childCreator,        // script to create childs
                         CString& parentCreator,       // script to create parents
						 CString& fieldsCreator,       // script to create fields
						 CString& nodeNameExtractor,   // script to extract name
						 CString& fieldsNameExtractor, // script to extract field name
				         CString& nodeImageExtractor,  // script to extract image
						 CString& fieldsImageExtractor,// script to extract field image
						 CString& testChildern,        // script to test if has children
                         CString& testParents,         // script to test if has parents
						 CNodeProperties* nodeUnselected,  // unselected style
			             CNodeProperties* nodeSelected,    // selected style
						 CNodeProperties* nodeFocused,     // focused style
						 CEdgeProperties* edgeUnselected,  // unselected style
			             CEdgeProperties* edgeSelected,    // selected style
						 CEdgeProperties* edgeFocused      // focused style
						 );

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
			// The default behavior is to use parent node scripts and properties
            void CreateChildNodes(   CString* accessCreateChildren=NULL,
									 CString* accessCreateParent=NULL,
				                     CString* accessCreateFields=NULL,
					                 CString* accessGetNodeName=NULL,
					                 CString* accessGetFieldName=NULL,
					                 CString* accessGetNodeImage=NULL,
					                 CString* accessGetFieldImage=NULL,
		                             CString* accessTestChildern=NULL,        
                                     CString* accessTestParents=NULL,     
					                 CNodeProperties* nodeUnselected=NULL,
					                 CNodeProperties* nodeSelected=NULL,
					                 CNodeProperties* nodeFocused=NULL,
					                 CEdgeProperties* edgeUnselected=NULL,
					                 CEdgeProperties* edgeSelected=NULL,
					                 CEdgeProperties* edgeFocused=NULL);

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
			// The default behavior is to use parent node scripts and properties
            void CreateParentNodes(  CString* accessCreateChildren=NULL,
									 CString* accessCreateParent=NULL,
				                     CString* accessCreateFields=NULL,
					                 CString* accessGetNodeName=NULL,
					                 CString* accessGetFieldName=NULL,
					                 CString* accessGetNodeImage=NULL,
					                 CString* accessGetFieldImage=NULL,
		                             CString* accessTestChildern=NULL,        
                                     CString* accessTestParents=NULL,     
					                 CNodeProperties* nodeUnselected=NULL,
					                 CNodeProperties* nodeSelected=NULL,
					                 CNodeProperties* nodeFocused=NULL,
					                 CEdgeProperties* edgeUnselected=NULL,
					                 CEdgeProperties* edgeSelected=NULL,
					                 CEdgeProperties* edgeFocused=NULL);

			// This function will evaluate accessCmd command to receve the list of
			// the related objects, will search the graph to locate if and where
			// this existing objects located in the graph and will add edges of the
			// type "relationID" connecting this node with all located elements 
			// (nodes or node fields) in the graph.
			void CreateRelations(CString& acccessCmd, 
				                 int relationID, 
				                 CEdgeProperties* unselected=NULL,
								 CEdgeProperties* selected=NULL,
								 CEdgeProperties* focused=NULL);

			// This function will remove all outgoing edges with relation type
			// "relationID"
			void RemoveRelations(int relationID);
			// Will run the extract fields script, will parse the results and
			// will fill the items list with the resulted indexes. It will use
			// field name extraction and field image extraction scripts to assing
			// the name and image to the item.
		    void ExtractFields(void);
			// Will remove all extracted fields from the fields list, as well as 
			// from the server array. This function will use parent graph to
			// modify the server array as well as all references from the graph
			// node.
			void RemoveFields(void);
			// This method will change node selection status
			void SetSelectionStatus(BOOL selected);
			// This method will return node selection status
			BOOL GetSelectionStatus(void);
			// This method will return this node identification number
			int GetNodeID(void);
			// This methid will change this node identification number
			void SetNodeID(int to);

			// This will return a pointer to the node items list.
            CItemsList* GetItemsList(void);
			// This method will search in the items list to locate the item
			// with the selected id. It will return the number of this item (
			// starting from list head) or -1 if no such field in the list.
			int FindFieldNumber(int id);

			// Returns the readable name of this node
			CString GetReadableName(void);

			// Returns the node properties depending from the node state
			// (unselected,selected or focused)
			CNodeProperties* GetActiveNodeProperties(void);

            // This function will evaluate node name extraction script and will
			// fill internal node name variable with the readable node name. We
			// need this variable to minimize client-server communications (we
			// are not going to ask the server about node name every time we
			// need to redraw node or to calculate it's size).
            void CDisGraphNode::ExtractName(void);

			// Returns TRUE if the children of this node are collapsed
	        BOOL IsCollapsed(void);
			// Returns TRUE if the parents of this node are collapsed
	        BOOL IsParentsCollapsed(void);
			// Expands all children of this node
	        void Expand(void);
			// Collapse all children of this node
	        void Collapse(void);
			// Expands all parents of this node
	        void ExpandParents(void);
			// Collapse all parents of this node
	        void CollapseParents(void);

			// Checks if the children of this node are already created
	        BOOL AreChildrenCreated(void);
	        void SetChildrenCreated(BOOL val);

			// Checks if the children of this node are already created
	        BOOL AreParentsCreated(void);
	        void SetParentsCreated(BOOL val);

			// Checks if this node has  children
			BOOL HasChildren(void);
			// Checks if this node has parents
			BOOL HasParents(void);

	        void ExtractParentChildEnable(void);
   private:
	        void HideChildren(void);
			void HideParents(void);

   private:
	        int m_Kind;
		    CDisGraph* m_ParentGraph;     // pointer to the parent graph - we
			                              // will use it to evaluate access commands
			                              // and to perform nodes and fields 
			                              // re-referencing
		    int m_NodeID;                 // The entity number in the server array
			CString m_ReadableName;       // The node name to display
			CString m_NodeNameExtractor;  // Script to extract readable name from 
			                              // the entity. Will return the entity
			                              // readable name
			CString m_NodeImageExtractor; // Script to extract image index from the
	                                      // the server entity. Will return image 
			                              // index.
			CString m_FieldsCreator;      // This script will create node fields
			                              // receving the node entity as an argument
			                              // and will return the list of indexes
			                              // of the created node fields (in the
			                              // same global array as the nodes)
			CString m_FieldNameExtractor; // This script will extract the name of the
			                              // field. Will return readable name 
			                              // receving the item ID as a parameter.
			CString m_FieldImageExtractor;// Will extract the image index from the
			                              // field receving the field ID as a 
			                              // parameter.
			CItemsList  m_FieldsList;     // This is list of the fiels indexes which
			                              // will be filled by ExtractFields method
			                              // using m_FieldsCreator script results.
			BOOL m_SelectStatus;              // Selected flag. True if this node 
			                              // selected. Focused flag is located in 
			                              // the graph itself pointing to the node
		                                  // which is focused.
	        CString m_ChildrenCreator;    // Script to create childs
	        CString m_ParentsCreator;     // Script to create parents
	        CString m_ParentTestScript;   // Script to test if node has parents
	        CString m_ChildTestScript;    // Script to test if node has children

			// Node display properties
			CNodeProperties* m_NodeUnselected; // how to display if unselected
			CNodeProperties* m_NodeSelected;   // how to display if selected
			CNodeProperties* m_NodeFocused;    // how to display if has focus

			// Edge display properties
			CEdgeProperties* m_EdgeUnselected; // how to display if unselected
			CEdgeProperties* m_EdgeSelected;   // how to display if selected
			CEdgeProperties* m_EdgeFocused;    // how to display if has focus

	        BOOL m_Collapsed;
	        BOOL m_ParentsCollapsed;

	        BOOL m_HasChildren;            // TRUE is this node has children
	        BOOL m_ChildrenCreated;        // TRUE is this node creates it's 
			                               // expanded 
			BOOL m_HasParents;
	        BOOL m_ParentsCreated;         // TRUE is this node creates it's 
			                               // parents
};
#else

TS_CAPI_CLASSDEF(CDisGraphNode)

#endif

#ifndef TS_EXCLUDE_C_API
#ifdef __cplusplus
extern "C"
{
#endif

	TS_EXPORT_PRE_FUNC CDisGraphNode * TS_EXPORT_POST_FUNC CDisGraphNodeCastDown(
			TSObject * pThisObject);

#ifdef __cplusplus
}
#endif
#endif
//------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------
// This class represents the relation between graph nodes. Every relation has 
// it's id, passed to the constructor as "relationID". Thee edge properties pointers
// specify the way the edge will look if unselected, if selected or if focused.
// The edges are created from the node by calling "CreateChildNodes" method with the
// relation id = 0 (for parent-child relations) or by the "CreateRelations" method
// with relations ID  passed as a parameter. The edges will be removed if any node
// it connects is removed or by calling "RemoveRelations" node method with the 
// relation ID.
//-----------------------------------------------------------------------------------
#ifdef __cplusplus
#define APP_CLASS
class APP_CLASS CDisGraphEdge : public TSEdge {
	TS_CLASSDEF(CDisGraphEdge)
    public : 
		    CDisGraphEdge(TSDigraph * pDigraph = NULL);
		    CDisGraphEdge(TSPathDigraph * pPathDigraph);
	
		    CDisGraphEdge(CDisGraph* graph,
				           int relationID,
						   CEdgeProperties* unselected,
						   CEdgeProperties* selected,
						   CEdgeProperties* focused);
             void SetEdge( int relationID,
						   CEdgeProperties* unselected,
						   CEdgeProperties* selected,
						    CEdgeProperties* focused);
           CDisGraph* GetParent(void);
			// This method will set edge selection status
			void SetSelectionStatus(BOOL selected);
			// This method will return the edge selection status
			BOOL GetSelectionStatus(void);
			// Will return the relation ID assigned in constructor
			int GetRelationID(void);

			// Returns the edge properties depending from the edge state
			// (unselected,selected or focused)
			CEdgeProperties* GetActiveEdgeProperties(void);

			// This function returns unselected edge drawing properties
            CEdgeProperties* GetUnselected(void);
			// This function returns selected edge drawing properties
            CEdgeProperties* GetSelected(void);
			// This function returns focused edge drawing properties
            CEdgeProperties* GetFocused(void);

			// Port this edge to the nodes it connected
            void Port(void);
			// Connects class fields
            void PortRelation(int sourceField, int targetField);

	private:
		    CDisGraph* m_ParentGraph;
		    int        m_RelationID;
			CEdgeProperties* m_Unselected;
			CEdgeProperties* m_Selected;
			CEdgeProperties* m_Focused;

			BOOL m_SelectStatus;

};


#else

TS_CAPI_CLASSDEF(CDisGraphEdge)

#endif


//
// The following functions provide an ANSI C interface to this class.
//

#ifndef TS_EXCLUDE_C_API
#ifdef __cplusplus
extern "C"
{
#endif

	TS_EXPORT_PRE_FUNC CDisGraphEdge * TS_EXPORT_POST_FUNC CDisGraphEdgeCastDown(
		TSObject * pThisObject);

#ifdef __cplusplus
}
#endif
#endif
//----------------------------------------------------------------------------------

