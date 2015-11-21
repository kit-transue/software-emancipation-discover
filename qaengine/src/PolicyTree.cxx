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
// PolicyTreeNode.cpp: implementation of the PolicyTreeNode class.
//
//////////////////////////////////////////////////////////////////////

#include "PolicyTree.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PolicyTreeNode::PolicyTreeNode(NodeKinds kind,string tag):XMLTreeNode(kind,tag) {
}

PolicyTreeNode::~PolicyTreeNode()
{

}



PolicyTree::PolicyTree()
{

}

PolicyTree::~PolicyTree()
{

}

XMLTreeNode* PolicyTree::newTagNode(string tag) {
	return new PolicyTreeNode(TagNode,tag);
}

XMLTreeNode* PolicyTree::newTextNode(string text) {
	return new PolicyTreeNode(TextNode,text);
}


string PolicyTreeNode::getNodeId() {
   return getAttributeValue("ID");
}

string PolicyTreeNode::getNodeReadableName() {
   return getAttributeValue("Name");
}

void PolicyTreeNode::setNodeReadableName(string name) {
   setAttributeValue("Name",name);
}


string PolicyTreeNode::getNodeKind() {
	return getName();
}


string PolicyTreeNode::getNodeParameterValue(string key,bool recursive) {
	return getNodeParameterValue(this,key,recursive);
}

void PolicyTreeNode::setNodeParameterValue(PolicyTreeNode* node,string key,string val) {
PolicyTreeNode* child;
PolicyTreeNode* p;

    if(node->getNodeParameterValue(node,key)==val) return;
	p=child=(PolicyTreeNode *)node->getFirstChild();
	if(child) {
		while(p && p->getName()!="Defn") p = (PolicyTreeNode *)p->getNextSibling();
		if(p) {
		    PolicyTreeNode* pch = (PolicyTreeNode *)p->getFirstChild();
		    while(pch) {
			    if(pch->getName()==key) {
				    PolicyTreeNode* value=(PolicyTreeNode *)pch->getFirstChild();
				    value->setName(val);
					return;
				}
			    pch = (PolicyTreeNode *)pch->getNextSibling();
			}
			// Given node has "Defn" child but no given key
		    PolicyTreeNode* newKey = new PolicyTreeNode(TagNode,key);
		    PolicyTreeNode* newVal = new PolicyTreeNode(TextNode,val);

		    newKey->setFirstChild(newVal);
		    newVal->setParent(newKey);

			newKey->setNextSibling(p->getFirstChild());
		    p->setFirstChild(newKey);
		    newKey->setParent(p);

		} else {
            // Given node has children but not "Defn" child
		    PolicyTreeNode* newDef = new PolicyTreeNode(TagNode,"Defn");
		    PolicyTreeNode* newKey = new PolicyTreeNode(TagNode,key);
		    PolicyTreeNode* newVal = new PolicyTreeNode(TextNode,val);

		    newKey->setFirstChild(newVal);
		    newVal->setParent(newKey);

		    newDef->setFirstChild(newKey);
		    newKey->setParent(newDef);

			newDef->setNextSibling(node->getFirstChild());
  	        node->setFirstChild(newDef);
	        newDef->setParent(node);
		    newDef->setNextSibling(child);
		}
	} else {
		// Given node has no children
		PolicyTreeNode* newDef = new PolicyTreeNode(TagNode,"Defn");
		PolicyTreeNode* newKey = new PolicyTreeNode(TagNode,key);
		PolicyTreeNode* newVal = new PolicyTreeNode(TextNode,val);

		newKey->setFirstChild(newVal);
		newVal->setParent(newKey);

		newDef->setFirstChild(newKey);
		newKey->setParent(newDef);

		node->setFirstChild(newDef);
		newDef->setParent(node);
	}
}

void PolicyTreeNode::setNodeParameterValue(string key,string val) {
	setNodeParameterValue(this,key,val);
}

string PolicyTreeNode::getNodeParameterValue(PolicyTreeNode* node,string key,bool recursive) {
PolicyTreeNode* parent;
PolicyTreeNode* child;

    parent = (PolicyTreeNode *)node->getParent();
	child = (PolicyTreeNode *)node->getFirstChild();
	while(child && child->getName()!="Defn") child = (PolicyTreeNode *)child->getNextSibling();
	if(child) {
		child = (PolicyTreeNode *)child->getFirstChild();
		while(child) {
			if(child->getName()==key) {
				 PolicyTreeNode* value=(PolicyTreeNode *)child->getFirstChild();
				 if(value!=NULL) return value->getName();
			}
			child = (PolicyTreeNode *)child->getNextSibling();
		}
	} 
	if(parent && recursive) return getNodeParameterValue(parent,key,recursive);
	return "";
}


string PolicyTreeNode::getNodeScriptFile() {
	return getNodeParameterValue("Source");
}

void PolicyTreeNode::setNodeScriptFile(string file) {
	setNodeParameterValue("Source",file);
}

string PolicyTreeNode::getNodeProcedureName() {
	return getNodeParameterValue("Command");
}

void PolicyTreeNode::setNodeProcedureName(string proc) {
	setNodeParameterValue("Command",proc);
}

PolicyTreeNode* PolicyTree::getFirstChild(PolicyTreeNode* node) {
  PolicyTreeNode* p = (PolicyTreeNode *)node->getFirstChild();
  while(p && p->getName()!="Folder" && p->getName()!="Query") {
	  p=(PolicyTreeNode *)p->getNextSibling();
  }
  return p;
}

PolicyTreeNode* PolicyTree::getNextSibling(PolicyTreeNode* node) {
  PolicyTreeNode* p = (PolicyTreeNode *)node->getNextSibling();
  while(p && p->getName()!="Folder" && p->getName()!="Query") {
	  p=(PolicyTreeNode *)p->getNextSibling();
  }
  return p;
}
