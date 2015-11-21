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
// TaskDriver.h: interface for the TaskDriver class.
//
//////////////////////////////////////////////////////////////////////
#include "globals.h"
#include "PolicyTree.h"
#include "ProjectTree.h"
#include "HitsTable.h"

#ifndef _QA_TASK_DRIVER_
#define _QA_TASK_DRIVER_
class TaskDriver;

typedef int (*ExecFunction)(TaskDriver*,int,string,string,string,string);

class TaskDriver  
{
public:
	void save(void);
	void close(void);
	void save(string policy,string project,string hits);
	int moveNode(int node,int parent);
	int addNode(int parent,string kind,string name);
	int setHitAttribute(int id,string name,string value);
	HitsTable* getHitsTablePtr(void);
	int addHits(string query, HitsSet hits);
	int isStatistical(int nodeId);
	TaskDriver();
	virtual ~TaskDriver();

	void createHits(void);
	void createPolicy(string name);
	void createProject(string name);
	void createPolicy(string name,string copyFrom);
	void createProject(string name,string copyFrom);

	int openPolicy(string fileName);
	int openProject(string fileName);
	int openHits(string fileName);

	int savePolicy(string fileName);
	int saveProject(string fileName);
	int saveHits(string fileName);

	int getRoot();
	int getParent(int child);
	int getFirstChild(int parent);
	int getLastChild(int parent);
	int getChildAt(int parent, int n);
	int getNextSibling(int node);
	int getPreviousSibling(int node);
	int removeNode(int id);

	string nodeKind(int node);
	PropertiesMap getNodeProperties(int id,ScopeSet scope);
	string getNodeProperty(int id, ScopeSet scope, string key);
	int setNodeProperty(int id,ScopeSet scope,string key, string value);

	int getNodeHitsCount(int id,ScopeSet scope);
	HitsSet getNodeHits(int id,ScopeSet scope);

	int addHit(string query,Hit* hit);
	int execute(ScopeSet scope, ExecFunction f);
	string count(ScopeSet scope);
	string hitsmap(ScopeSet scope);
	string glass(ScopeSet scope);

private:
	int m_durtyPolicy;
	string m_strPolicy;
	PolicyTree*  m_pPolicy;

	int m_durtyProject;
	string m_strProject;
	ProjectTree* m_pProject;

	int m_durtyHits;
	string m_strHits;
	HitsTable*   m_pHits;

	int recursiveExec(PolicyTreeNode* node,string module, ExecFunction f);
	int recursiveCount(PolicyTreeNode* node,ScopeSet scope, string& result);
	int recursiveHitsmap(PolicyTreeNode* node,ScopeSet scope, string& result);
	void recursiveGlass(PolicyTreeNode* node,ScopeSet scope, string& result);
};
#endif
