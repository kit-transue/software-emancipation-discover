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
// XMLTree.cpp: implementation of the XMLTree class.
//
//////////////////////////////////////////////////////////////////////
#include "XMLTree.h"


static int printLevel;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


XMLTreeNode::XMLTreeNode() {
	m_Kind    = NullNode;
	m_Name        = "";
	m_FirstChild  = NULL;
	m_NextSibling = NULL;
	m_Parent      = NULL;
	m_Index       = 0;
}


XMLTreeNode::XMLTreeNode(NodeKinds kind,string tag) {
	m_Kind        = kind;
	m_Name        = "";
	m_FirstChild  = NULL;
	m_NextSibling = NULL;
	m_Parent      = NULL;
	if(kind==TagNode) parseTag(tag);
	else {
		int space = tag.find_last_not_of(" ");
		tag = tag.substr(0,space+1);
		m_Name = tag;
	}
}

string XMLTreeNode::getName() {
	return m_Name;
}

void XMLTreeNode::setName(string name) {
	m_Name = name;
}

NodeKinds XMLTreeNode::getKind() {
	return m_Kind;
}

void XMLTreeNode::setKind(NodeKinds kind) {
	m_Kind = kind;
}


XMLTreeNode* XMLTreeNode::getParent() {
	return m_Parent;
}

void XMLTreeNode::setParent(XMLTreeNode* parent) {
	m_Parent = parent;
}

XMLTreeNode* XMLTreeNode::getFirstChild() {
	return m_FirstChild;
}

void XMLTreeNode::setFirstChild(XMLTreeNode* child) {
	m_FirstChild = child;
}

XMLTreeNode* XMLTreeNode::getNextSibling() {
	return m_NextSibling;
}

void XMLTreeNode::setNextSibling(XMLTreeNode* sibling) {
	m_NextSibling = sibling;
}

PropertiesMap* XMLTreeNode::getAttributesMap() {
	return &m_Attributes;

}

int XMLTreeNode::parseTag(string tag) {
int i;
int stringMode=0;
enum {Name,Key,Value} mode;
string name;
string key;
string value;

    mode=Name;
	i=0;
	while(tag[i]==' ' && i<tag.length()) i++;
    for(;i<tag.length();i++) {
		if(tag[i]=='"') {
		    stringMode=!stringMode;
			continue;
		}
		if(tag[i]=='=') {
			mode = Value;
	        while(tag[i]==' ' && i<tag.length()) i++;
			continue;
		}
		if(tag[i]==' ' && !stringMode) {
			switch(mode) {
			    case Name : m_Name = name;          break;
				case Key  : m_Attributes[key]="";   break;
				case Value: m_Attributes[key]=value;break;
			}
			mode=Key;
			name = "";
			key  = "";
			value= "";
			continue;
		}
        if(tag[i]=='\n' && !stringMode) continue;
		switch(mode) {
		   case Name : name+=tag[i];  break;
		   case Key  : key+=tag[i];   break;
		   case Value: value+=tag[i]; break;
		}
	}
	switch(mode) {
	    case Name : m_Name = name;          break;
		case Key  : m_Attributes[key]="";   break;
		case Value: m_Attributes[key]=value;break;
	}
    return 0; 
}


XMLTree::XMLTree() {
	m_Root        = NULL;
	m_CurrentNode = NULL;
	m_NodeIndex = 0;

}

XMLTree::~XMLTree() {
    if(m_Root) {
		recursiveFree(m_Root);
        m_Root = NULL;
	}
}


//-----------------------------------------------------------------------------------------
// This method loads the tree from the XML policy file.
//-----------------------------------------------------------------------------------------
int XMLTree::open(string xmlFileName) {
   ifstream in(xmlFileName.c_str(),ios::in);
   if(!in.is_open()) return -1;
   if(m_Root) {
       recursiveFree(m_Root);
       m_Root = NULL;
   }
   m_NodeIndex = 0;
   int result = parseXML(in);
   in.close();
   return result;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// This method reads XML data from the valid file handle and generates tree nodes.
//-----------------------------------------------------------------------------------------
int XMLTree::parseXML(istream& in) {
char   ch;
int    inTag    = 0;
int    closeTag = 0;
char   prevCh=0;
string tag;
string val;

  while(!in.eof()) {
	  in.read(&ch,1);
	  if(ch=='\n') continue;
	  if(prevCh=='<' && ch=='/') {
		  closeTag=1;
	  }
	  if(ch=='<') {
		  if(val.length()>0) {
			  if(val.length()>0) appendTextNode(val);
			  val="";
		  }
		  prevCh = ch;
		  inTag = 1;
		  continue;
	  }
	  if(ch=='>') {
		  if(!closeTag) {
			  if(prevCh=='/') tag.erase(tag.length()-1,1);
			  if(tag.length()>0)
			      appendTagNode(tag);
			  if(prevCh=='/') closeTagNode();
		  } else{
			  closeTagNode();
		  }
		  tag="";
		  inTag   = 0;
		  closeTag = 0;
		  prevCh = ch;
		  continue;
	  }
	  prevCh = ch;
	  if(inTag) { 
		  if(tag.length()>0 || ch!=' ') tag+=ch;
	  } else {
		  if(val.length()>0 || ch!=' ') val+=ch;
	  }

  } 
  return 0;
}
//-----------------------------------------------------------------------------------------

int XMLTree::appendTagNode(string tag) {
XMLTreeNode* newNode;

     newNode = newTagNode(tag);
	 newNode->setIndex(m_NodeIndex);
	 m_SearchMap[m_NodeIndex++] = newNode;

	 newNode->setParent(m_CurrentNode);
	 if(!m_CurrentNode) {
		 if(m_Root==NULL) m_Root = newNode;
		 else {
			 XMLTreeNode* p = m_Root;
   	         while(p->getNextSibling()) p = p->getNextSibling();
             p->setNextSibling(newNode);
		 }
		 m_CurrentNode = newNode;
		 return 1;
	 }
	 XMLTreeNode* p = m_CurrentNode->getFirstChild();
	 if(!p) {
		 m_CurrentNode->setFirstChild(newNode);
		 m_CurrentNode = newNode;
		 return 1;
	 }
	 while(p->getNextSibling()) p = p->getNextSibling();
	 p->setNextSibling(newNode);
	 m_CurrentNode = newNode;
	 return 1;
}

int XMLTree::appendTextNode(string text) {
XMLTreeNode* newNode;

     newNode = newTextNode(text);
	 newNode->setKind(TextNode);
	 newNode->setParent(m_CurrentNode);
	 if(!m_CurrentNode) {
		 if(m_Root==NULL) {
			 m_Root = newNode;
		 } else {
			 XMLTreeNode* p = m_Root;
   	         while(p->getNextSibling()) p = p->getNextSibling();
             p->setNextSibling(newNode);
		 }
	     m_CurrentNode = newNode;
		 return 1;
	 }
	 XMLTreeNode* p = m_CurrentNode->getFirstChild();
	 if(!p) {
		 m_CurrentNode->setFirstChild(newNode);
		 return 1;
	 }
	 while(p->getNextSibling()) p = p->getNextSibling();
	 p->setNextSibling(newNode);
	 return 1;
}


int XMLTree::closeTagNode() {
	if(!m_CurrentNode) return 0;
	m_CurrentNode = m_CurrentNode->getParent();
	return 1;
}

void XMLTree::debugNodePrint() {
	printLevel = 0;
	if(m_Root) {
       recursiveNodePrint(cout,m_Root);
	} else {
	   cout<<"Tree is empty"<<endl;
	}
}

void XMLTree::debugTreePrint() {
	printLevel = 0;
	if(m_Root) {
       recursiveTreePrint(cout,m_Root);
	} else {
	   cout<<"Tree is empty"<<endl;
	}
}


void XMLTree::recursiveNodePrint(ostream& out, XMLTreeNode *node) {
int i;	
	// Printing node with its attributes on the desired level
	for(i =0; i<printLevel;i++) out<<"  ";
	if(node->getKind()==TagNode) out<<"<";
    out<<node->getName();
	PropertiesMap *attr = node->getAttributesMap();
	if(node->getKind()==TagNode && attr->size()>0) {
        PropertiesMap::iterator j=attr->begin();
		while(j!=attr->end()) {
			out<<" "<< j->first<<"="<<"\""<< j->second <<"\"";
			j++;
		}
	}
	if(node->getKind()==TagNode) out<<">";
	out<<endl;

	// Processing child node
	if(node->getFirstChild()) {
		printLevel++;
		recursiveNodePrint(out,node->getFirstChild());
		printLevel--;
	} 

    // Printing close tag for this node
    if(node->getKind()==TagNode) {
        for(i =0; i<printLevel;i++) out<<"  ";
        out<<"</";
        out<<node->getName();
        out<<">"<<endl;
	}

	// Processing next sibling
	if(node->getNextSibling()) {
		recursiveNodePrint(out,node->getNextSibling());
	} 

}


void XMLTree::recursiveTreePrint(ostream& out, XMLTreeNode *node) {
int i;	
	// Printing node with its attributes on the desired level
	for(i =0; i<printLevel;i++) out<<"  ";
	if(node->getKind()==TagNode) out<<"<";
    out<<node->getName();
	PropertiesMap *attr = node->getAttributesMap();
	if(node->getKind()==TagNode && attr->size()>0) {
        PropertiesMap::iterator j=attr->begin();
		while(j!=attr->end()) {
			out<<" "<< j->first<<"="<<"\""<< j->second <<"\"";
			j++;
		}
	}
	if(node->getKind()==TagNode) out<<">";
	out<<endl;

	// Processing child node
	if(getFirstChild(node)) {
		printLevel++;
		recursiveTreePrint(out,getFirstChild(node));
		printLevel--;
	} 

    // Printing close tag for this node
    if(node->getKind()==TagNode) {
        for(i =0; i<printLevel;i++) out<<"  ";
        out<<"</";
        out<<node->getName();
        out<<">"<<endl;
	}

	// Processing next sibling
	if(getNextSibling(node)) {
		recursiveTreePrint(out,getNextSibling(node));
	} 

}



int XMLTree::save(string xmlFileName) {
   ofstream out(xmlFileName.c_str(),ios::out|ios::trunc);
   if(!out.is_open()) return -1;
   recursiveNodePrint(out,m_Root);
   out.close();
   return 0;
}

void XMLTree::recursiveFree(XMLTreeNode *node) {
	if(node->getFirstChild()) recursiveFree(node->getFirstChild());
	if(node->getNextSibling()) recursiveFree(node->getNextSibling());
	delete node;
}

string XMLTreeNode::getAttributeValue(string key) {
	PropertiesMap::iterator index = m_Attributes.find(key);
	if(index==m_Attributes.end()) return "";
	return index->second;

}

void XMLTreeNode::setAttributeValue(string key,string value) {
	if(value=="") m_Attributes.erase(key);
	else m_Attributes[key]=value;
}


XMLTreeNode* XMLTree::getRoot() {
	return m_Root;

}

XMLTreeNode* XMLTree::getFirstChild(XMLTreeNode *node) {
	return node->getFirstChild();

}

XMLTreeNode* XMLTree::getParent(XMLTreeNode *node) {
	return node->getParent();

}

XMLTreeNode* XMLTree::getNextSibling(XMLTreeNode *node) {
	return node->getNextSibling();

}

int XMLTree::addChildLast(XMLTreeNode* parent, XMLTreeNode* child) {
int ret = m_NodeIndex;

    child->setIndex(m_NodeIndex);
    m_SearchMap[m_NodeIndex++] = child;
	if(!parent) {
		if(!m_Root) {
			m_Root = child;
			child->setParent(NULL);
			return ret;
		} else {
			parent = m_Root;
		}
	}
		
	XMLTreeNode* p = parent->getFirstChild();
	if(p==NULL) {
		parent->setFirstChild(child);
		child->setParent(parent);
		return ret;
	}
	while(p->getNextSibling()) p = p->getNextSibling();
	p->setNextSibling(child);
	child->setParent(parent);
	return ret;
}

int XMLTree::addChildFirst(XMLTreeNode* parent, XMLTreeNode* child) {
int ret = m_NodeIndex;

    child->setIndex(m_NodeIndex);
	m_SearchMap[m_NodeIndex++] = child;
	if(!parent) {
		if(!m_Root) {
			m_Root = child;
			child->setParent(NULL);
			return ret;
		} else {
			parent = m_Root;
		}
	}
		
	XMLTreeNode* p = parent->getFirstChild();
	child->setParent(parent);
	parent->setFirstChild(child);
	child->setNextSibling(p);
	return ret;
}


int XMLTree::addSiblingLast(XMLTreeNode* to, XMLTreeNode* sibling) {
int ret=m_NodeIndex;

    sibling->setIndex(m_NodeIndex);
    m_SearchMap[m_NodeIndex++] = sibling;
	if(!to) {
		if(!m_Root) {
			m_Root = sibling;
			sibling->setParent(NULL);
			return ret;
		} else {
		    to = m_Root;
		}
	}		
	while(to->getNextSibling()) to = to->getNextSibling();
	to->setNextSibling(sibling);
	sibling->setParent(to->getParent());
	return ret;
}

int XMLTree::addSiblingFirst(XMLTreeNode* to, XMLTreeNode* sibling) {
int ret = m_NodeIndex;
    sibling->setIndex(m_NodeIndex);
    m_SearchMap[m_NodeIndex++] = sibling;
	if(!to) {
		if(!m_Root) {
			m_Root = sibling;
			sibling->setParent(NULL);
			return ret;
		} else {
		    to = m_Root;
		}
	}
	sibling->setNextSibling(to);
	sibling->setParent(to->getParent());
	if(!to->getParent()) m_Root = sibling;
	return ret;
}


void XMLTree::removeNode(XMLTreeNode* node) {
	if(!node) return;
	if(node->getParent() && node->getParent()->getFirstChild()==node) {
		node->getParent()->setFirstChild(node->getNextSibling());
	} else {
		XMLTreeNode* prev = getPreviousSibling(node);
		if(prev) {
			prev->setNextSibling(node->getNextSibling());
		}
	}
	node->setNextSibling(NULL);
	recursiveFree(node);
}

XMLTreeNode* XMLTree::getPreviousSibling(XMLTreeNode *node) {
	XMLTreeNode* first = node->getParent() ? node->getParent()->getFirstChild() : m_Root;
	if(!first||first==node) return NULL;
	while(first->getNextSibling()!=node) {
		first = first->getNextSibling();
	}
	return first;
}

XMLTreeNode* XMLTree::find(int id) {
	return m_SearchMap[id];

}

void XMLTreeNode::setIndex(int index) {
	m_Index = index;

}

int XMLTreeNode::getIndex() {
	return m_Index;
}
