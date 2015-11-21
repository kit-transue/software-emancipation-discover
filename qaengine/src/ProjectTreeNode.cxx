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
// ProjectTreeNode.cpp: implementation of the ProjectTreeNode class.
//
//////////////////////////////////////////////////////////////////////

#include "ProjectTreeNode.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ProjectTreeNode::ProjectTreeNode():XMLTreeNode(){

}

ProjectTreeNode::ProjectTreeNode(NodeKinds kind, string tag):XMLTreeNode(kind,tag){

}

ProjectTreeNode::~ProjectTreeNode()
{

}

string ProjectTreeNode::getNodeQueryValue(string queryId,string key) {
	return getNodeQueryValue(this,queryId,key);
}

string ProjectTreeNode::getNodeQueryValue(ProjectTreeNode* node,string queryId,string key) {
ProjectTreeNode* parent;
ProjectTreeNode* child;

    parent = (ProjectTreeNode *)node->getParent();
	child = (ProjectTreeNode *)node->getFirstChild();
	while(child && child->getName()!=queryId) child = (ProjectTreeNode *)child->getNextSibling();
	if(child) {
		child = (ProjectTreeNode *)child->getFirstChild();
		while(child) {
			if(child->getName()==key) {
				 ProjectTreeNode* value=(ProjectTreeNode *)child->getFirstChild();
				 if(value!=NULL) return value->getName();
			}
			child = (ProjectTreeNode *)child->getNextSibling();
		}
	} 
	if(parent) return getNodeQueryValue(parent,queryId,key);
	return "";
}


void ProjectTreeNode::setNodeQueryValue(string queryId,string key,string val) {
	setNodeQueryValue(this,queryId,key,val);
}



void ProjectTreeNode::setNodeQueryValue(ProjectTreeNode* node,string queryId, string key,string val) {
ProjectTreeNode* child;
ProjectTreeNode* p;

    if(node->getNodeQueryValue(node,queryId,key)==val) return;
	p=child=(ProjectTreeNode *)node->getFirstChild();
	if(child) {
		while(p && p->getName()!=queryId) p = (ProjectTreeNode *)p->getNextSibling();
		if(p) {
		    ProjectTreeNode* pch = (ProjectTreeNode *)p->getFirstChild();
		    while(pch) {
			    if(pch->getName()==key) {
				    ProjectTreeNode* value=(ProjectTreeNode *)pch->getFirstChild();
				    value->setName(val);
					return;
				}
			    pch = (ProjectTreeNode *)pch->getNextSibling();
			}
			// Given node has queryId child but no given key
		    ProjectTreeNode* newKey = new ProjectTreeNode(TagNode,key);
		    ProjectTreeNode* newVal = new ProjectTreeNode(TextNode,val);

		    newKey->setFirstChild(newVal);
		    newVal->setParent(newKey);

			newKey->setNextSibling(p->getFirstChild());
		    p->setFirstChild(newKey);
		    newKey->setParent(p);
		} else {
            // Given node has children but not queryId child
		    ProjectTreeNode* newDef = new ProjectTreeNode(TagNode,queryId);
		    ProjectTreeNode* newKey = new ProjectTreeNode(TagNode,key);
		    ProjectTreeNode* newVal = new ProjectTreeNode(TextNode,val);

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
		ProjectTreeNode* newDef = new ProjectTreeNode(TagNode,queryId);
		ProjectTreeNode* newKey = new ProjectTreeNode(TagNode,key);
		ProjectTreeNode* newVal = new ProjectTreeNode(TextNode,val);

		newKey->setFirstChild(newVal);
		newVal->setParent(newKey);

		newDef->setFirstChild(newKey);
		newKey->setParent(newDef);

		node->setFirstChild(newDef);
		newDef->setParent(node);
	}
}

