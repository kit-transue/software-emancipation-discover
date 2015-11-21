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
// TaskDriver.cpp: implementation of the TaskDriver class.
//
//////////////////////////////////////////////////////////////////////

#include "TaskDriver.h"

#define _MESSAGING

#ifdef _MESSAGING
#include "msg.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaskDriver::TaskDriver() {
	m_pPolicy    = NULL;
	m_pProject   = NULL;
	m_pHits      = NULL;
	m_strPolicy  = "";
	m_strProject = "";
	m_strHits    = "";
	m_durtyPolicy  = 0;
	m_durtyProject = 0;
	m_durtyHits    = 0;
}

TaskDriver::~TaskDriver() {
	save();
	if(m_pPolicy)    delete m_pPolicy;
	if(m_pProject)   delete m_pProject;
	if(m_pHits)      delete m_pHits;
}


//-------------------------------------------------------------------------------
// Sets policy file name and creates a policy tree from the given policy file.
//-------------------------------------------------------------------------------
int TaskDriver::openPolicy(string fileName) {
	if(m_durtyPolicy && m_strPolicy.length()>0) {
		savePolicy(m_strPolicy);
	    m_durtyPolicy = 0;
	}
	if(m_pPolicy) delete m_pPolicy;
    m_pPolicy= new PolicyTree();
	if(m_pPolicy->open(fileName)!=0) {
		delete m_pPolicy;
		m_pPolicy = NULL;
		return -1;
	}
	m_strPolicy = fileName;
	return 0;
}
//-------------------------------------------------------------------------------



//-------------------------------------------------------------------------------
// Sets project file name and creates a project tree from the given project file.
//-------------------------------------------------------------------------------
int TaskDriver::openProject(string fileName) {
	if(m_durtyProject && m_strProject.length()>0) {
		saveProject(m_strProject);
	    m_durtyProject = 0;
	}
	if(m_pProject) delete m_pProject;
    m_pProject= new ProjectTree();
	if(m_pProject->open(fileName)!=0) {
		delete m_pProject;
		m_pProject = NULL;
		return -1;
	}
	m_strProject = fileName;
	return 0;
} 
//-------------------------------------------------------------------------------



//-------------------------------------------------------------------------------
// Sets hits file name and creates a hits table from the given hits file.
//-------------------------------------------------------------------------------
int TaskDriver::openHits(string fileName) {
	if(m_strHits==fileName) return 0;
	if(m_durtyHits && m_strHits.length()>0) {
		saveHits(m_strHits);
	    m_durtyHits = 0;
	}
    if(m_pHits) delete m_pHits;
    m_pHits= new HitsTable();
	if(m_pHits->load(fileName)!=0) {
		delete m_pHits;
		m_pHits = NULL;
		return -1;
	}
	m_strHits = fileName;
	return 0;
} 
//-------------------------------------------------------------------------------

int TaskDriver::savePolicy(string fileName) {
	if(!m_pPolicy) return -2;
	return m_pPolicy->save(fileName);
}
int TaskDriver::saveProject(string fileName) {
	if(!m_pProject) return -2;
	return m_pProject->save(fileName);
}
int TaskDriver::saveHits(string fileName) {
	if(!m_pHits) return -2;
	return m_pHits->save(fileName);
}

//------------------------------------------------------------------------------
// This function will return a map with property name/property value pairs
// represented as string/string. Will be used to dump node properties.
//------------------------------------------------------------------------------
PropertiesMap TaskDriver::getNodeProperties(int id,ScopeSet scope) {
PropertiesMap map;
string queryId;

    if(!m_pPolicy) return map;
	// Scannining selection-independent properties
	PolicyTreeNode* node = (PolicyTreeNode *)m_pPolicy->find(id);
	if(node==NULL) return map;


	map["Kind"]      = node->getName();
	if(node->getName()=="Folder" || node->getName()=="Query") {
	    map["Name"]      = node->getNodeReadableName();
	    map["ID"]    = node->getNodeId();
	}
	if(node->getName()=="Query") {
	    map["Script"]    = node->getNodeScriptFile();
	    map["Procedure"] = queryId = node->getNodeProcedureName();
	}

    int hasEnabled  =0;
	int hasArguments=0;
	int hasWeight   =0;

	// Scanning selection properties
    if(m_pProject) {
        ScopeSet::iterator i;
        for(i=scope.begin();i!=scope.end();i++) {
	        string item = *i;
		    PropertiesMap* projPropMap;
			PolicyTreeNode* n = node;
			do {
				queryId = n->getNodeProcedureName();
				if(queryId!="") { 
		            projPropMap=m_pProject->getNodeProperties(item,queryId);
                    PropertiesMap::iterator j;
                    for(j=projPropMap->begin();j!=projPropMap->end();j++) {
			            map[j->first] = j->second;
				        if(j->first=="Enabled")   hasEnabled   = 1;
				        if(j->first=="Arguments") hasArguments = 1;
				        if(j->first=="Weight")    hasWeight    = 1;
					}
		            delete projPropMap;
				} else {
					string folderId = n->getNodeReadableName();
					if(folderId!="") {
		                projPropMap=m_pProject->getFolderProperties(item,folderId);
                        PropertiesMap::iterator j;
                        for(j=projPropMap->begin();j!=projPropMap->end();j++) {
							string curAttr = map[j->first];
							if(j->first=="Enabled") {
								if(curAttr.length()==0 || curAttr=="true")
									map[j->first] = j->second;
								if(!hasEnabled) hasEnabled = 2;
							} else {
								if(curAttr.length()==0)
									map[j->first] = j->second;
								if(j->first=="Arguments") hasArguments = 1;
								if(j->first=="Weight")    hasWeight    = 1;
							}
						}
		                delete projPropMap;
					}
				}
				n=(PolicyTreeNode *)m_pPolicy->getParent(n);
			} while(n);
		}
	}

	string parentEnable = map["Enabled"];
	if(parentEnable.length()==0) {
		PolicyTreeNode* parent = (PolicyTreeNode*)node->getParent();
		while(parent) {
			parentEnable = parent->getAttributeValue("Enabled");
			if(parentEnable.length()>0) {
				map["Enabled"]=parentEnable;
				hasEnabled=2;
				if(parentEnable == "false")  break;
			}
			parent = (PolicyTreeNode*)parent->getParent();
		}
	}
	if(parentEnable=="true" && node->getNodeKind()!="Folder" && hasEnabled==2) hasEnabled = 0;
	if(parentEnable.length()==0 || (parentEnable.length()!=0 && parentEnable=="true")) {
		if(!hasEnabled) {
			string curEnabledState = node->getAttributeValue("Enabled");
			if(curEnabledState.length()!=0 && curEnabledState=="false") {
				map["Enabled"] = "false";
				hasEnabled = 1;
			} else {
				if(curEnabledState.length()!=0) {
					map["Enabled"] = curEnabledState;
					hasEnabled = 1;
				}
			}
		}
	}
	if(!hasEnabled)  {
		string val = node->getNodeParameterValue("Disabled",parentEnable.length()==0);
		if(val=="1") {
		   map["Enabled"]="false";
		   hasEnabled = 1;
		} else {
		   map["Enabled"]="true";
		}
	}

	if(!hasArguments)  {
		string val = node->getAttributeValue("Arguments");
		if(val=="") {
			val = node->getNodeParameterValue("Threshold");
			if(val!="") {
			   map["Arguments"]=val;
			   hasArguments = 1;
			}
		} else {
			map["Arguments"]=val;
			hasArguments = 1;
		}
	}

	if(!hasWeight)  {
        string val = node->getAttributeValue("Weight");
		if(val=="") {
			val = node->getNodeParameterValue("Weight");
			if(val!="") {
			   map["Weight"]=val;
			   hasWeight = 1;
			}
		} else {
			map["Weight"]=val;
			hasWeight = 1;
		}
	}
	if(!hasEnabled)   map["Enabled"]  ="true";
	if(!hasWeight)    map["Weight"]   ="100";

	return map;
}
//------------------------------------------------------------------------------

int TaskDriver::moveNode(int id,int parentId) {
    if(!m_pPolicy) return -1;
	PolicyTreeNode* node = (PolicyTreeNode *)m_pPolicy->find(id);
	PolicyTreeNode* parent = (PolicyTreeNode *)m_pPolicy->find(parentId);
	if(node==NULL || parent==NULL) return -2;
	PolicyTreeNode* currentParent = (PolicyTreeNode *)node->getParent();
	if(currentParent==NULL) return -3;
	PolicyTreeNode* prevSibling = (PolicyTreeNode *)currentParent->getFirstChild();
	if(prevSibling==NULL) return -4;
	if(prevSibling != node) {
		while(prevSibling!=NULL && prevSibling->getNextSibling()!=node) {
			prevSibling = (PolicyTreeNode *)prevSibling->getNextSibling();
		}
		if(prevSibling!=NULL) {
			prevSibling->setNextSibling(node->getNextSibling());
			node->setParent(parent);
			PolicyTreeNode* child = (PolicyTreeNode*)parent->getFirstChild();
			if(child == NULL)
				parent->setFirstChild(node);
			else {
				while(child->getNextSibling()!=NULL) 
					child = (PolicyTreeNode*)child->getNextSibling();
				child->setNextSibling(node);
			}
			node->setNextSibling(NULL);
		} else {
			return -5;
		}
	} else {
		currentParent->setFirstChild(node->getNextSibling());
	}
    return 0;
}

int TaskDriver::setNodeProperty(int id, ScopeSet scope, string key, string value) {
    if(!m_pPolicy) return -1;
	PolicyTreeNode* node = (PolicyTreeNode *)m_pPolicy->find(id);
	if(node==NULL) return -2;
	if(key=="Name") {
		m_durtyPolicy = 1;
		node->setNodeReadableName(value);
		return 0;
	}
	if(key=="Script") {
		m_durtyPolicy = 1;
		node->setNodeScriptFile(value);
		return 0;
	}
	if(key=="Procedure") {
		m_durtyPolicy = 1;
		node->setNodeProcedureName(value);
		return 0;
	}

	if(scope.begin()==scope.end()) {
	    if(m_pPolicy) {
		    m_durtyPolicy = 1;
			node->setAttributeValue(key,value);
			m_durtyProject = 1;
		}
	} 

	// Setting property walues for all selected projects/modules
	string queryId = node->getNodeProcedureName();
	if(m_pProject) {
	    if(queryId!="") {
	        ScopeSet::iterator i;
	        for(i=scope.begin();i!=scope.end();i++) {
		        string item = *i;
			    m_pProject->setNodeProperty(queryId,item,key,value);
			}		
		    m_durtyProject = 1;
		} else {
			string folder = node->getNodeReadableName();
			if(folder!="") {
	            ScopeSet::iterator i;
	            for(i=scope.begin();i!=scope.end();i++) {
		            string item = *i;
			        m_pProject->setFolderProperty(folder,item,key,value);
				}		
		        m_durtyProject = 1;
			}
		}
	}
    return 0;
}


int TaskDriver::getRoot() {
	if(!m_pPolicy) return NO_TREE;
	return m_pPolicy->getRoot()->getIndex();
}

int TaskDriver::getFirstChild(int parent) {
	if(!m_pPolicy) return NO_TREE;

    PolicyTreeNode* n = (PolicyTreeNode *)m_pPolicy->find(parent);
	if(!n) return NO_NODE;

	if(m_pPolicy->getFirstChild(n)!=NULL) 
		return m_pPolicy->getFirstChild(n)->getIndex();

	return NULL_NODE;
}

int TaskDriver::getLastChild(int parent) {
	if(!m_pPolicy) return NO_TREE;

    PolicyTreeNode* n = (PolicyTreeNode *)m_pPolicy->find(parent);
	if(!n) return NO_NODE;

	PolicyTreeNode* child = m_pPolicy->getFirstChild(n);
	if(!child) return NULL_NODE;

	while(m_pPolicy->getNextSibling(child)) child = m_pPolicy->getNextSibling(child);		

	return child->getIndex();
}

int TaskDriver::getChildAt(int parent, int n) {
	if(!m_pPolicy) return NO_TREE;

    PolicyTreeNode* nd = (PolicyTreeNode *)m_pPolicy->find(parent);
	if(!nd) return NO_NODE;

	PolicyTreeNode* child = (PolicyTreeNode *)m_pPolicy->getFirstChild(nd);
	if(!child) return NULL_NODE;

	int i = 0;
	do {
		if(i==n) return child->getIndex();
		child = (PolicyTreeNode *)m_pPolicy->getNextSibling(child);
		i++;
	} while(child);

	return NULL_NODE;
}

int TaskDriver::getNextSibling(int node) {
	if(!m_pPolicy) return NO_TREE;

    PolicyTreeNode* n = (PolicyTreeNode *)m_pPolicy->find(node);
	if(!n) return NO_NODE;

	PolicyTreeNode* sibling = m_pPolicy->getNextSibling(n);
	if(sibling) return sibling->getIndex();
	return NULL_NODE;
}



int TaskDriver::getPreviousSibling(int node) {
	if(!m_pPolicy) return NO_TREE;

    PolicyTreeNode* n = (PolicyTreeNode *)m_pPolicy->find(node);
	if(!n) return NO_NODE;

	PolicyTreeNode* prev = NULL;
	PolicyTreeNode* sibling = (PolicyTreeNode *)m_pPolicy->getFirstChild((PolicyTreeNode *)n->getParent());

	while(sibling) {
        if(n==sibling) {
			if(prev) return prev->getIndex();
			return NULL_NODE;
		}
		prev = sibling;
		sibling = m_pPolicy->getNextSibling(sibling);
	}
	return NULL_NODE;
}

string TaskDriver::nodeKind(int node) {
	if(!m_pPolicy) return "";
    PolicyTreeNode* n = (PolicyTreeNode *)m_pPolicy->find(node);
	if(!n) return "";
	return n->getName();
}

int TaskDriver::getParent(int child) {
	if(!m_pPolicy) return NO_TREE;
    PolicyTreeNode* n = (PolicyTreeNode *)m_pPolicy->find(child);
	if(!n) return NO_NODE;
	PolicyTreeNode* parent = (PolicyTreeNode *)n->getParent();
	if(parent) return parent->getIndex();
	return NULL_NODE;
}

string TaskDriver::getNodeProperty(int id, ScopeSet scope, string key) {
string queryId;

    if(!m_pPolicy) return "";
	// Scannining selection-independent properties
	PolicyTreeNode* node = (PolicyTreeNode *)m_pPolicy->find(id);
	if(node==NULL) return "";


	if(key=="Kind")      return node->getName();
	if(key=="Name")      return node->getNodeReadableName();
	if(key=="Script")    return node->getNodeScriptFile();
	if(key=="Procedure") return node->getNodeProcedureName();


	queryId = node->getNodeProcedureName();

	// Scanning selection properties
	if(queryId!="") {
	    if(m_pProject) {
	        ScopeSet::iterator i;
	        for(i=scope.begin();i!=scope.end();i++) {
		        string item = *i;
			    PropertiesMap* projPropMap;
			    projPropMap=m_pProject->getNodeProperties(item,queryId);
                PropertiesMap::iterator j;
	            for(j=projPropMap->begin();j!=projPropMap->end();j++) {
                    if(j->first==key) {
						string val = j->second;
						delete projPropMap;
						return val;

					}
				}
			    delete projPropMap;
			}
		}
	}
	return "";
}

HitsSet TaskDriver::getNodeHits(int id, ScopeSet scope) {
HitsSet set;
string queryId;

    if(!m_pPolicy) return set;
	// Scannining selection-independent properties
	PolicyTreeNode* node = (PolicyTreeNode *)m_pPolicy->find(id);
	if(node==NULL) return set;

	if(node->getName()=="Query") {
	   queryId = node->getNodeProcedureName();
	}

	if(queryId!="") {
	    if(m_pHits) {
			set = m_pHits->search(queryId,scope);
		}
	}
	return set;
}

int TaskDriver::getNodeHitsCount(int id, ScopeSet scope) {
int count=0;
string queryId;
static int deep = 0;

    if(!m_pPolicy) return count;
	// Scannining selection-independent properties
	PolicyTreeNode* node = (PolicyTreeNode *)m_pPolicy->find(id);
	if(node==NULL) return count;
    if(node->getName()=="Query") {
        queryId = node->getNodeProcedureName();
	    if(queryId!="") {
	       if(m_pHits) {
	  	       count+=m_pHits->count(queryId,scope);
		   }
	   }
	}

	if(node->getName()!="Query") {
        int child = getFirstChild(node->getIndex());
        if(child>=0)  {
	        deep++;
	        count+=getNodeHitsCount(child,scope);
	        deep--;
		}
	}
    if(deep!=0) {
       int sibling = getNextSibling(node->getIndex());
       if(sibling>=0) 
   	       count+=getNodeHitsCount(sibling,scope);
	}
	return count;
}


int TaskDriver::removeNode(int id) {
    if(!m_pPolicy) return NO_TREE;
	PolicyTreeNode* node = (PolicyTreeNode *)m_pPolicy->find(id);
	if(node==NULL) return NO_NODE;
	m_pPolicy->removeNode(node);
    m_durtyPolicy = 1;
	return 0;
}

int TaskDriver::execute(ScopeSet scope, ExecFunction f) {
int  retVal = 0;
    ScopeSet::iterator i;
    for(i=scope.begin();i!=scope.end();i++) {
		  string module = *i;
		  
		  msg("Processing file $1.", normal_sev) <<  module << eoarg << eom;
	
		  
		  int res = recursiveExec((PolicyTreeNode *)m_pPolicy->getRoot(),module,f);
		  if(res!=0) retVal = res;
		  
		  report_progress("qa_file");
	}
    return retVal;
}

string TaskDriver::count(ScopeSet scope) {
string result;

    if(m_pPolicy==NULL) return result;
    ScopeSet::iterator i;
	int xx = scope.size();
	for(i=scope.begin();i!=scope.end();i++) {
		string str = *i;
	}
    int size = recursiveCount((PolicyTreeNode *)m_pPolicy->getRoot(),scope,result);
	char buf[100];
	sprintf(buf,"%d",size);
	result+="stat:files=";
	result+=buf;
	result+="\n";
    return result;
}

string TaskDriver::hitsmap(ScopeSet scope) {
string result;

    if(m_pPolicy==NULL) return result;
    ScopeSet::iterator i;
	int xx = scope.size();
	for(i=scope.begin();i!=scope.end();i++) {
		string str = *i;
	}
    int size = recursiveHitsmap((PolicyTreeNode *)m_pPolicy->getRoot(),scope,result);
    return result;
}

string TaskDriver::glass(ScopeSet scope) {
string result;

    if(m_pPolicy==NULL) return result;
    ScopeSet::iterator i;
	int xx = scope.size();
	for(i=scope.begin();i!=scope.end();i++) {
		string str = *i;
	}
    recursiveGlass((PolicyTreeNode *)m_pPolicy->getRoot(),scope,result);
    return result;
}



int TaskDriver::recursiveExec(PolicyTreeNode *node, string module, ExecFunction f) {
string arguments="";
string isEnabled="";

	if(node->getNodeKind()=="Query") {
		string script    = node->getNodeScriptFile();
		string procedure = node->getNodeProcedureName();
		if(m_pProject!=NULL) {
	        PropertiesMap* projPropMap;
	        projPropMap=m_pProject->getNodeProperties(module,procedure);
            PropertiesMap::iterator j;
	        for(j=projPropMap->begin();j!=projPropMap->end();j++) {
                if(j->first=="Arguments") {
			 	    arguments = j->second;
				} 
                if(j->first=="Enabled") {
			 	    isEnabled = j->second;
				}
			}
	        delete projPropMap;
		}

		if(arguments.length()==0) {
			arguments = node->getNodeParameterValue("Threshold");
		}

		if(isEnabled.length()==0) {
			string disabled = node->getNodeParameterValue("Disabled");
			if(disabled=="1") isEnabled="false";
			else              isEnabled="true";
		}

		if(isEnabled=="true") {
		    (*f)(this,node->getIndex(),module,script,procedure,arguments);
		} 	
	}
	if(node->getFirstChild()) {
		recursiveExec((PolicyTreeNode *)node->getFirstChild(),module,f);
	}
	if(node->getNextSibling()) {
		recursiveExec((PolicyTreeNode *)node->getNextSibling(),module,f);
	}
	return 0;
}

int TaskDriver::recursiveCount(PolicyTreeNode *node, ScopeSet scope, string& result) {
int size=0;
	if(node->getNodeKind()=="Query") {
		if(isStatistical(node->getIndex())) {
		    string procedure = node->getNodeProcedureName();
			int globalCounter = 0;
			HitsSet statHits = m_pHits->search(procedure,scope);
			size = statHits.size();
			HitsSet::iterator i;
			for(i=statHits.begin();i!=statHits.end();i++) {
				Hit* hit = *i;
				string max = hit->getAttribute("Maximum");
				if(max.length()>0) globalCounter+=atoi(max.c_str());
			}
			result+=procedure;
			result+="=";
			char buf[100];
			sprintf(buf,"%d",globalCounter);
			result+=buf;
			result+="\n";
		}
	}
	if(node->getFirstChild()) {
		int sz = recursiveCount((PolicyTreeNode *)node->getFirstChild(),scope,result);
		if(size<sz) size = sz;
	}
	if(node->getNextSibling()) {
		int sz = recursiveCount((PolicyTreeNode *)node->getNextSibling(),scope,result);
		if(size<sz) size = sz;
	}
	return size;
}

int TaskDriver::recursiveHitsmap(PolicyTreeNode *node, ScopeSet scope, string& result) {
int size=0;
	if(node->getNodeKind()=="Query") {
		if(!isStatistical(node->getIndex())) {
		    string procedure = node->getNodeProcedureName();
			int totalHits = m_pHits->count(procedure,scope);
			result+=procedure;
			result+="=";
			char buf[100];
			sprintf(buf,"%d",totalHits);
			result+=buf;
			result+="\n";
		}
	}
	if(node->getFirstChild()) {
		int sz = recursiveHitsmap((PolicyTreeNode *)node->getFirstChild(),scope,result);
		if(size<sz) size = sz;
	}
	if(node->getNextSibling()) {
		int sz = recursiveHitsmap((PolicyTreeNode *)node->getNextSibling(),scope,result);
		if(size<sz) size = sz;
	}
	return size;
}


void TaskDriver::recursiveGlass(PolicyTreeNode *node, ScopeSet scope, string& result) {
char buf[100];

    sprintf(buf,"%d",node->getIndex());
	result+=buf;
	result+="\t";
	PropertiesMap map = getNodeProperties(node->getIndex(),scope);
	PropertiesMap::iterator i;
	for(i=map.begin();i!=map.end();i++) {
		if(i->first.length()==0) result+=" ";
		else result+=i->first;
		result+="\t";
		if(i->second.length()==0) result+=" ";
		else result+=i->second;
		result+="\t";
	}
    int statistical = isStatistical(node->getIndex());
	if(statistical) {
		result+="Statistical";
		result+="\t";
		result+="true";
	}
	result+="\n";

	if(node->getNodeKind()!="Query") {
	    if(m_pPolicy->getFirstChild(node)) {
		    recursiveGlass((PolicyTreeNode *)m_pPolicy->getFirstChild(node),scope,result);
		}
	}
	if(m_pPolicy->getNextSibling(node)) {
		recursiveGlass((PolicyTreeNode *)m_pPolicy->getNextSibling(node),scope,result);
	}
}


int TaskDriver::addHit(string query,Hit *hit) {
	if(!m_pHits) return -1;
	m_pHits->append(query,hit);
    m_durtyHits = 1;
    return 0;
}

void TaskDriver::createHits() {
	if(m_pHits) delete m_pHits;
	m_pHits = new HitsTable();
}


void TaskDriver::createPolicy(string name) {
	if(m_durtyPolicy && m_strPolicy.length()>0) {
		savePolicy(m_strPolicy);
	    m_durtyPolicy = 0;
	}
	if(m_pPolicy) delete m_pPolicy;
	m_pPolicy = new PolicyTree();
	PolicyTreeNode* node =new PolicyTreeNode(TagNode,"Policy");
	m_pPolicy->addChildFirst(NULL,node);
	m_strPolicy = name;
	m_durtyPolicy=1;
}


void TaskDriver::createProject(string name) {
	if(m_durtyProject && m_strProject.length()>0) {
		saveProject(m_strProject);
	    m_durtyProject = 0;
	}
	if(m_pProject) delete m_pProject;
	m_pProject = new ProjectTree();
	ProjectTreeNode* node =new ProjectTreeNode(TagNode,"root");
	m_pProject->addChildFirst(NULL,node);
	m_strProject = name;
	m_durtyProject=1;
}

void TaskDriver::createPolicy(string name,string copyFrom) {
	if(m_durtyPolicy && m_strPolicy.length()>0) {
		savePolicy(m_strPolicy);
	    m_durtyPolicy = 0;
	}
	if(m_pPolicy) delete m_pPolicy;
	m_pPolicy = new PolicyTree();
    if(m_pPolicy->open(copyFrom)!=0) {
	    PolicyTreeNode* node =new PolicyTreeNode(TagNode,"Policy");
	    m_pPolicy->addChildFirst(NULL,node);
    }
	m_strPolicy = name;
	m_durtyPolicy=1;
}


void TaskDriver::createProject(string name,string copyFrom) {
	if(m_durtyProject && m_strProject.length()>0) {
		saveProject(m_strProject);
	    m_durtyProject = 0;
	}
	if(m_pProject) delete m_pProject;
	m_pProject = new ProjectTree();
    if(m_pProject->open(copyFrom)!=0) {
	    ProjectTreeNode* node =new ProjectTreeNode(TagNode,"Root");
	    m_pProject->addChildFirst(NULL,node);
	}
	m_strProject = name;
	m_durtyProject=1;
}


int TaskDriver::isStatistical(int nodeId) {
    if(!m_pPolicy) return NO_TREE;
	// Scannining selection-independent properties
	PolicyTreeNode* node = (PolicyTreeNode *)m_pPolicy->find(nodeId);
	if(!node) return NO_NODE;
	do {
	    if(node->getNodeId()=="stat") {
		    return 1;
		}
		node = (PolicyTreeNode *)node->getParent();
	} while(node);
	return 0;

}

int TaskDriver::addHits(string query, HitsSet hits) {
	if(!m_pHits) return -1;
	HitsSet::iterator i;
	for(i=hits.begin();i!=hits.end();i++) {
	    m_pHits->append(query,*i);
	}
	m_durtyHits = 1;
	return 0;

}

HitsTable* TaskDriver::getHitsTablePtr() {
	return m_pHits;
}

int TaskDriver::setHitAttribute(int id, string name, string value) {
	if(m_pHits==NULL) return -1;
	Hit* hit = m_pHits->find(id);
	if(hit==NULL) return -1;
	PropertiesMap::iterator pos = hit->getAttributes()->find(name);
	if(value.length()==0) {
		if(pos!=hit->getAttributes()->end()) {
            hit->getAttributes()->erase(pos);
		}
	} else {
	    (*hit->getAttributes())[name]=value;
    }
	m_durtyHits = 1;
	return 0;
}


int TaskDriver::addNode(int parent, string kind, string name) {
    if(!m_pPolicy) return NO_TREE;
	// Scannining selection-independent properties
	PolicyTreeNode* node = (PolicyTreeNode *)m_pPolicy->find(parent);
	if(!node) return NO_NODE;
	PolicyTreeNode* newNode=new PolicyTreeNode(TagNode,kind);
	newNode->setNodeReadableName(name);
	m_durtyPolicy=1;
	return m_pPolicy->addChildLast(node,newNode);
}

void TaskDriver::save() {
	if(m_durtyPolicy && m_strPolicy.length()>0) {
		savePolicy(m_strPolicy);
	    m_durtyPolicy = 0;
	}
	if(m_durtyProject && m_strProject.length()>0) {
		saveProject(m_strProject);
	    m_durtyProject = 0;
	}
	if(m_durtyHits && m_strHits.length()>0) {
		saveHits(m_strHits);
	    m_durtyHits = 0;
	}
}

void TaskDriver::close() {
   m_durtyPolicy = 0;
   m_durtyProject = 0;
   m_durtyHits = 0;
   if(m_pPolicy)    delete m_pPolicy;
   m_pPolicy = NULL;
   if(m_pProject)   delete m_pProject;
   m_pProject   = NULL;
   if(m_pHits)      delete m_pHits;
   m_pHits      = NULL;

}


void TaskDriver::save(string policy,string project,string hits) {
	if(policy.length()>0)
        m_strPolicy  = policy;
	if(project.length()>0)
        m_strProject = project;
	if(hits.length()>0)
        m_strHits    = hits;

	if(m_strPolicy.length()>0) {
		savePolicy(m_strPolicy);
	    m_durtyPolicy = 0;
	}
	if(m_strProject.length()>0) {
		saveProject(m_strProject);
	    m_durtyProject = 0;
	}
	if(m_strHits.length()>0) {
		saveHits(m_strHits);
	    m_durtyHits = 0;
	}
}
