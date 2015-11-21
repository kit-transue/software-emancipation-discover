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
// ProjectTree.cpp: implementation of the ProjectTree class.
//
//////////////////////////////////////////////////////////////////////

#include "ProjectTree.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ProjectTree::ProjectTree()
{

}

ProjectTree::~ProjectTree()
{

}
XMLTreeNode* ProjectTree::newTagNode(string tag) {
	return new ProjectTreeNode(TagNode,tag);
}

XMLTreeNode* ProjectTree::newTextNode(string text) {
	return new ProjectTreeNode(TextNode,text);
}

PropertiesMap* ProjectTree::getNodeProperties(string item, string queryId) {
string subproj = "";
PropertiesMap* properties = new PropertiesMap();


    // Parsing the given project or file name to find the deepest
    // project tree node which contains query parameters for this
    // project or module
	ProjectTreeNode* p = (ProjectTreeNode*)m_Root;
	for(int i = 0; i<=item.length();i++) {
		if(item[i]=='/' || item[i]=='\\' || i==item.length()) {
			if(subproj.length()>0) {
				ProjectTreeNode* n = (ProjectTreeNode*)p->getFirstChild();
				while(n) {
					if(n->getName()==subproj&&n->getAttributeValue("QueryId")=="") {
						p=n;
						break;
					}
					n=(ProjectTreeNode*)n->getNextSibling();
				}
				if(n==NULL) break;
			}
			subproj="";
			continue;
		}
		subproj+=item[i];
	}


	// Going up the project tree collecting redefined properties on each
	// level. Important! The upper level propery re-definition does not
	// owerrride the lower level property definition.
	do {
        ProjectTreeNode* n = (ProjectTreeNode*)p->getFirstChild();
		while(n) {
			if(n->getAttributeValue("QueryId")==queryId) {
				PropertiesMap* map = n->getAttributesMap();
				PropertiesMap::iterator j = map->begin();
				while(j!=map->end()) {
					if((j->first!="QueryId") && 
				 	   (properties->find(j->first)==properties->end())) {
						(*properties)[j->first] = j->second;
					}
					j++;
				}
			}
			n=(ProjectTreeNode *)n->getNextSibling();
		}
		p = (ProjectTreeNode *)p->getParent();
	} while(p!=NULL);

	// All properties which are not defined in the project tree will 
	// be taken from the root as a default properties.
	fillDefaultProperties(properties);
	return properties;

}

PropertiesMap* ProjectTree::getFolderProperties(string item, string queryId) {
string subproj = "";
PropertiesMap* properties = new PropertiesMap();


    // Parsing the given project or file name to find the deepest
    // project tree node which contains query parameters for this
    // project or module
	ProjectTreeNode* p = (ProjectTreeNode*)m_Root;
	for(int i = 0; i<=item.length();i++) {
		if(item[i]=='/' || item[i]=='\\' || i==item.length()) {
			if(subproj.length()>0) {
				ProjectTreeNode* n = (ProjectTreeNode*)p->getFirstChild();
				while(n) {
					if(n->getName()==subproj&&n->getAttributeValue("FolderId")=="") {
						p=n;
						break;
					}
					n=(ProjectTreeNode*)n->getNextSibling();
				}
				if(n==NULL) break;
			}
			subproj="";
			continue;
		}
		subproj+=item[i];
	}


	// Going up the project tree collecting redefined properties on each
	// level. Important! The upper level propery re-definition does not
	// owerrride the lower level property definition.
	do {
        ProjectTreeNode* n = (ProjectTreeNode*)p->getFirstChild();
		while(n) {
			if(n->getAttributeValue("FolderId")==queryId) {
				PropertiesMap* map = n->getAttributesMap();
				PropertiesMap::iterator j = map->begin();
				while(j!=map->end()) {
					if((j->first!="FolderId") && 
				 	   (properties->find(j->first)==properties->end())) {
						string first = j->first;
						string second = j->second;
						(*properties)[j->first] = j->second;
					}
					j++;
				}
			}
			n=(ProjectTreeNode *)n->getNextSibling();
		}
		p = (ProjectTreeNode *)p->getParent();
	} while(p!=NULL);

	// All properties which are not defined in the project tree will 
	// be taken from the root as a default properties.
	fillDefaultProperties(properties);
	return properties;

}


void ProjectTree::setNodeProperty(string query,string item,string key,string value) {
string subproj = "";

    // Parsing the given project or file name to find the deepest
    // project tree node which contains query parameters for this
    // project or module
	ProjectTreeNode* p = (ProjectTreeNode*)m_Root;
	for(int i = 0; i<=item.length();i++) {
		if(item[i]=='/' || item[i]=='\\' || i==item.length()) {
			if(subproj.length()>0) {
				ProjectTreeNode* n = (ProjectTreeNode*)p->getFirstChild();
				while(n) {
					if(n->getName()==subproj) {
						p=n;
						break;
					}
					n=(ProjectTreeNode*)n->getNextSibling();
				}
				if(n==NULL) {
	                // I will add code to check if our current key value
					// is equal to the key value I am adding
					ProjectTreeNode* newProj = new ProjectTreeNode(TagNode,subproj);
					addChildLast(p,newProj);
					p = newProj;
				}
			}
			subproj="";
			continue;
		}
		subproj+=item[i];
	}

	// Now p contains the pointer to the project or module
	// to which we are adding the property;

    ProjectTreeNode* n = (ProjectTreeNode*)p->getFirstChild();
	while(n) {
		if(n->getAttributeValue("QueryId")==query) {
			n->setAttributeValue(key,value);
			break;
		}
		n=(ProjectTreeNode *)n->getNextSibling();
	}

    if(n==NULL) {
		ProjectTreeNode* queryRecord = new ProjectTreeNode(TagNode,"Query");
		queryRecord->setAttributeValue(key,value);
		queryRecord->setAttributeValue("QueryId",query);
		queryRecord->setParent(p);
		queryRecord->setNextSibling(p->getFirstChild());
		p->setFirstChild(queryRecord);
	}
}

void ProjectTree::setFolderProperty(string folder,string item,string key,string value) {
string subproj = "";

    // Parsing the given project or file name to find the deepest
    // project tree node which contains query parameters for this
    // project or module
	ProjectTreeNode* p = (ProjectTreeNode*)m_Root;
	for(int i = 0; i<=item.length();i++) {
		if(item[i]=='/' || item[i]=='\\' || i==item.length()) {
			if(subproj.length()>0) {
				ProjectTreeNode* n = (ProjectTreeNode*)p->getFirstChild();
				while(n) {
					if(n->getName()==subproj) {
						p=n;
						break;
					}
					n=(ProjectTreeNode*)n->getNextSibling();
				}
				if(n==NULL) {
	                // I will add code to check if our current key value
					// is equal to the key value I am adding
					ProjectTreeNode* newProj = new ProjectTreeNode(TagNode,subproj);
					addChildLast(p,newProj);
					p = newProj;
				}
			}
			subproj="";
			continue;
		}
		subproj+=item[i];
	}

	// Now p contains the pointer to the project or module
	// to which we are adding the property;

    ProjectTreeNode* n = (ProjectTreeNode*)p->getFirstChild();
	while(n) {
		if(n->getAttributeValue("FolderId")==folder) {
			n->setAttributeValue(key,value);
			break;
		}
		n=(ProjectTreeNode *)n->getNextSibling();
	}

    if(n==NULL) {
		ProjectTreeNode* queryRecord = new ProjectTreeNode(TagNode,"Folder");
		queryRecord->setAttributeValue(key,value);
		queryRecord->setAttributeValue("FolderId",folder);
		queryRecord->setParent(p);
		queryRecord->setNextSibling(p->getFirstChild());
		p->setFirstChild(queryRecord);
	}
}

void ProjectTree::fillDefaultProperties(PropertiesMap *properties) {
}
