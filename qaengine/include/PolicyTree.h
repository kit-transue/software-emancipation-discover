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
// PolicyTreeNode.h: interface for the PolicyTreeNode class.
//
//////////////////////////////////////////////////////////////////////
#include "globals.h"
#include "XMLTree.h"
#ifndef _QA_POLICY_TREE_
#define _QA_POLICY_TREE_
class PolicyTreeNode : public XMLTreeNode  
{
public:
    PolicyTreeNode(NodeKinds kind,string tag);
	virtual ~PolicyTreeNode();

	string getNodeKind();
	string getNodeReadableName();
    void   setNodeReadableName(string name);
	string getNodeId();

	string getNodeScriptFile(void);
    void setNodeScriptFile(string file);

	string getNodeProcedureName(void);
    void   setNodeProcedureName(string proc);
	string getNodeParameterValue(string key,bool recursive=true);
    string getNodeParameterValue(string queryId,string key,bool recursive=true);
    void setNodeParameterValue(string key,string val);
protected:
	virtual string getNodeParameterValue(PolicyTreeNode* node,string key,bool recursive=true);
    virtual void setNodeParameterValue(PolicyTreeNode* node,string key,string val);
};

class PolicyTree : public XMLTree  
{
public:
	PolicyTree();
	virtual ~PolicyTree();
	virtual PolicyTreeNode* getFirstChild(PolicyTreeNode* node);
	virtual PolicyTreeNode* getNextSibling(PolicyTreeNode* node);
protected:
	virtual XMLTreeNode* newTagNode(string tag);
	virtual XMLTreeNode* newTextNode(string text);

};
#endif
