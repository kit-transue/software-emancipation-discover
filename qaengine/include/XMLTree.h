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
// XMLTree.h: interface for the XMLTree class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _QA_XML_GENERIC_TREE_
	#include "globals.h"

	#define NO_TREE   -3
	#define NO_NODE   -2
	#define NULL_NODE -1

	class XMLTreeNode {
	public :
		int getIndex(void);
		void setIndex(int index);
		string getAttributeValue(string key);
		void setAttributeValue(string key,string value);
		PropertiesMap *getAttributesMap();
		XMLTreeNode();
		XMLTreeNode(NodeKinds kind, string tag);

		string getName();
		void setName(string name);
		NodeKinds getKind();
		void      setKind(NodeKinds kind);
		XMLTreeNode* getParent();
		void setParent(XMLTreeNode* parent);
		XMLTreeNode* getFirstChild();
		void setFirstChild(XMLTreeNode* child);
		XMLTreeNode* getNextSibling();
		void setNextSibling(XMLTreeNode* sibling);
	protected:
		virtual int parseTag(string tag);
	private:
		int m_Index;
		NodeKinds          m_Kind;
		string             m_Name;
		PropertiesMap      m_Attributes;
		XMLTreeNode*       m_FirstChild;
		XMLTreeNode*       m_NextSibling;
		XMLTreeNode*       m_Parent;
	};

	class XMLTree   {
	public:
		XMLTreeNode* find(int id);
		XMLTreeNode* getPreviousSibling(XMLTreeNode* node);
		virtual int addChildFirst(XMLTreeNode* parent, XMLTreeNode* child);
		virtual int addChildLast(XMLTreeNode* parent, XMLTreeNode* child);
		virtual int addSiblingFirst(XMLTreeNode* to, XMLTreeNode* sibling);
		virtual int addSiblingLast(XMLTreeNode* to, XMLTreeNode* sibling);
		virtual void removeNode(XMLTreeNode* node);
		virtual XMLTreeNode* getRoot(void);
		virtual XMLTreeNode* getParent(XMLTreeNode* node);
		virtual XMLTreeNode* getFirstChild(XMLTreeNode* node);
		virtual XMLTreeNode* getNextSibling(XMLTreeNode* node);
		int save(string xmlFileName);
		void debugNodePrint();
		void debugTreePrint();
		XMLTree();
		int open(string xmlFileName);
		virtual ~XMLTree();

	protected:
		virtual XMLTreeNode* newTagNode(string tag)=0;
		virtual XMLTreeNode* newTextNode(string text)=0;
		XMLTreeNode* m_Root;

		virtual int parseXML(istream& in);
		virtual int closeTagNode(void);
		virtual int appendTagNode(string tag);
		virtual int appendTextNode(string text);
	private:
		void recursiveFree(XMLTreeNode* node);
		void recursiveNodePrint(ostream& out, XMLTreeNode* node);
		void recursiveTreePrint(ostream& out, XMLTreeNode* node);
		XMLTreeNode* m_CurrentNode;

		int m_NodeIndex;
		map<int,XMLTreeNode*> m_SearchMap;

	};
#define _QA_XML_GENERIC_TREE_
#endif

