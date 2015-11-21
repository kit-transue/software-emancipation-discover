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
// ResultsTree.cpp: implementation of the CResultsTree class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "ResultsTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResultEntity::CResultEntity() {
	m_priv = false;
	m_prot = false;
	m_line = -1;
}

CResultEntity::CResultEntity(CString& message, CString& name, CString& kind, CString& tag,CString& lang,CString&  priv, CString& prot,  CString& file, CString& line) {
  setName(name);
  setMessage(message);
  setKind(kind);
  setTag(tag);
  setLang(lang);
  setPrivate(priv);
  setProtected(prot);
  setFile(file);
  setLine(line);
}

CResultEntity::~CResultEntity() {
}

CString* CResultEntity::getMessage() {
	static CString szDescription;

        if(m_message.CompareNoCase("POSSIBLE_NAME_CONFLICT")==0) szDescription = "Potential name conflict";
        else if(m_message.CompareNoCase("NEED_CHANGE")==0) szDescription = "Entities that need to be changed";
        else if(m_message.CompareNoCase("NEED_DECLARATION")==0) szDescription = "Variables that should be explicitly declared";
        else if(m_message.CompareNoCase("INCLUDED")==0) szDescription = "Included in files";
        else if(m_message.CompareNoCase("METHOD_EXISTS")==0) szDescription = "Method already exists";
        else if(m_message.CompareNoCase("FIELD_EXISTS")==0) szDescription = "Field already exists";
        else if(m_message.CompareNoCase("FUNCTION_EXISTS")==0) szDescription = "Function already exists";
        else if(m_message.CompareNoCase("MEMBER_EXISTS")==0) szDescription = "Member already exists";
        else if(m_message.CompareNoCase("MACRO_EXISTS")==0) szDescription = "Macros with the same name where detected";
        else if(m_message.CompareNoCase("VAR_EXISTS")==0) szDescription = "Variable already exists";
        else if(m_message.CompareNoCase("STATIC_VAR_EXISTS")==0) szDescription = "Static variables with the same name";
        else if(m_message.CompareNoCase("TEMPLATE_EXISTS")==0) szDescription = "Templates with the same name";
        else if(m_message.CompareNoCase("OVERLOAD_VIRTUAL_BASE")==0) szDescription = "Base class virtual method overloaded";
        else if(m_message.CompareNoCase("HIDE_METHOD_BASE")==0) szDescription = "Method hides base class method";
        else if(m_message.CompareNoCase("HIDE_NONVIRTUAL_BASE")==0) szDescription = "Method hides base class method";
        else if(m_message.CompareNoCase("HIDE_NONVIRTUAL_DERIVED")==0) szDescription = "Derived class method hides this method";
        else if(m_message.CompareNoCase("CANNOT_CHANGE_VALUE_OF_FINAL_VARIABLE")==0) szDescription = "Value cannot be assigned to a constant variable";
        else if(m_message.CompareNoCase("OVERLOAD_VIRTUAL_DERIVED")==0) szDescription = "Derived class method overloads this method";
        else if(m_message.CompareNoCase("RECOMPILE_FILE")==0) szDescription = "Files that need to be recompiled";
        else if(m_message.CompareNoCase("EXTENDING_INTERFACES")==0) szDescription = "Extending interfaces, impact of deleting these may not be displayed";
        else if(m_message.CompareNoCase("FILES_NEEDING_DECL")==0) szDescription = "Files requiring definition of variable";
        else if(m_message.CompareNoCase("NATIVE_METHOD_CANNOT_HAVE_IMPLEMENTATION")==0) szDescription = "Native methods can't have body";
        else if(m_message.CompareNoCase("MULTIPLE_DEFINES")==0) szDescription = "Cannot have multiple definitions of variable";
        else if(m_message.CompareNoCase("INNER_CLASS")==0) szDescription = "This field is declared in an inner class and cannot be declared static";
		else if(m_message.CompareNoCase("REFERENCED_BUT_NOT_DEFINED")==0) szDescription = "Referenced but undefined entities";
        else if(m_message.CompareNoCase("POSSIBLE_ILLEGAL_USE")==0) szDescription = "Method or field may be in use";
        else if(m_message.CompareNoCase("POSSIBLE_VALUE_OVERWRITTING")==0) szDescription = "Possilbe value overwritting";
        else if(m_message.CompareNoCase("INCORRECT_VALUE")==0) szDescription = "Variable may have incorrect value";
        else if(m_message.CompareNoCase("ILLEGAL_ACCESS")==0) szDescription =  "Inaccessible entities";
        else if(m_message.CompareNoCase("CHECK_LOGIC")==0) szDescription = "Instances that need to be reviewed";
        else if(m_message.CompareNoCase("STOPS_HIDING_DERIVED")==0) szDescription = "Derived class method overloads this method";
        else if(m_message.CompareNoCase("STARTS_HIDING_DERIVED")==0) szDescription = "Derived class method hides this method";
        else if(m_message.CompareNoCase("FUNCTION_BODY_NOT_IN_THE_MODEL")==0) szDescription = CString("Functions defined outside of the information model. ") + CString("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.CompareNoCase("MACRO_BODY_NOT_IN_THE_MODEL")==0) szDescription = CString("Macros defined outside of the information model. ") + CString("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.CompareNoCase("TEMPLATE_BODY_NOT_IN_THE_MODEL")==0) szDescription = CString("Templates defined outside of the information model. ") + CString("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.CompareNoCase("CLASS_BODY_NOT_IN_THE_MODEL")==0) szDescription = CString("Classes defined outside of the information model. ") + CString("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.CompareNoCase("INTERFACE_NOT_IN_THE_MODEL")==0) szDescription = CString("Interfaces defined outside of the information model. ") + CString("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.CompareNoCase("FIELD_NOT_IN_THE_MODEL")==0) szDescription = CString("Fields defined outside of the information model. ") + CString("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.CompareNoCase("BODY_NOT_IN_THE_MODEL")==0) szDescription = CString("Entities defined outside of the information model. ") + CString("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.CompareNoCase("HIDES_BASE_CLASS_NONPRIVATE_FIELD")==0) szDescription = "Fields that hide a base class non-private field";
        else if(m_message.CompareNoCase("HIDES_THIS_CLASS_NONPRIVATE_FIELD")==0) szDescription = "Derived class fields  that hide this non-private field";
        else if(m_message.CompareNoCase("PURE_VIRTUAL_OVERLOAD")==0) szDescription = "Methods that need to be implemented in extending classes";
        else if(m_message.CompareNoCase("NEED_REMOVE_METHODS")==0) szDescription = "Methods that implement interface that need to be deleted";
        else if(m_message.CompareNoCase("NEED_IMPLEMENT_METHODS")==0) szDescription = "Interface methods that need to be implemented";
        else if(m_message.CompareNoCase("OVERLOAD_FINAL_METHOD")==0) szDescription = "Method declared as final in base class cannot be overriden!";
        else if(m_message.CompareNoCase("RENAME_VIRTUAL_DERIVED")==0) szDescription = "Derived class methods that should be reviewed";
		else if(m_message.CompareNoCase("ABSTRACT_CLASS")==0) szDescription="Classes that are abstract and can't be instantiated";
		else if(m_message.CompareNoCase("ABSTRACT_METHOD")==0) szDescription="Abstract methods that should be implemented";
		else if(m_message.CompareNoCase("FINAL_CLASS_CANNOT_HAVE_SUBCLASSES")==0) szDescription="Final class cannot have any subclasses";
		else if(m_message.CompareNoCase("NON_STATIC")==0) szDescription="Non static entities that can't be referenced from a static context";
		else if(m_message.CompareNoCase("METHOD_NEEDS_IMPL")==0) szDescription="Classes that should implement abstract method";
		else if(m_message.CompareNoCase("METHOD_NEEDS_IMPL_CPP")==0) szDescription="Classes that should implement virtual method";
				else if(m_message.CompareNoCase("METHOD_NEEDS_IMPL_CPP")==0) szDescription="Classes that should implement virtual method";
		else if(m_message.CompareNoCase("CALLING_NONCONST")==0) szDescription="Non const methods that cannot be referenced from a const context";
		else if(m_message.CompareNoCase("ASSIGNING_VALUES_IN_CONST_FUNCTION")==0) szDescription="Values modified in a const function";
		else if(m_message.CompareNoCase("SHOULD_BE_DECLARED_ABSTRACT")==0) szDescription="Classes that should be declared as abstract";
		else if(m_message.CompareNoCase("ENTITY")==0) szDescription=m_name;
		else if(m_message.CompareNoCase("INSTANCE")==0) {
			char szLine[10];
			sprintf(szLine,"%d",m_line);
			szDescription=m_name + " in " + m_file + " at " + szLine;
	} else szDescription=m_message;
	return &szDescription;
}

CResultsTree::CResultsTree() {
	m_pEntity = new CResultEntity();
	m_pChildren = NULL;
	m_pNext = NULL;
	m_pParent = NULL;
	m_pPrev = NULL;
}

CResultsTree::CResultsTree(CString& message, CString& name, CString& kind, CString& tag,CString& lang,CString&  priv, CString& prot,  CString& file, CString& line) {
	m_pEntity = new CResultEntity(message,name,kind,tag,lang,priv,prot,file,line);
	m_pChildren = NULL;
	m_pNext = NULL;
	m_pParent = NULL;
	m_pPrev = NULL;
}

CResultsTree::~CResultsTree() {
	if(m_pEntity!=NULL)	delete m_pEntity;
	if(m_pChildren!=NULL) delete m_pChildren;
	if(m_pNext!=NULL) delete m_pNext;
}

void CResultsTree::append(CResultsTree* pEntity) {
	pEntity->setParent(this->getParent());
	pEntity->setNext(NULL);
	pEntity->setPrev(NULL);
	CResultsTree* pTree = this;
	while(pTree->getNext()!=NULL) pTree=pTree->getNext();
	pTree->setNext(pEntity);
	pEntity->setPrev(pTree);
}

CResultsTree* CResultsTree::get(int idx) {
	CResultsTree* pRoot = this;
	while(idx>0) {
		pRoot = pRoot->getNext();
		idx--;
	}
	return pRoot;
}

CResultsTree* CResultsTree::getLevel(int level) {
	CResultsTree* pTree = this;
	while(pTree->getNext()!=NULL)
		pTree=pTree->getNext();

	if(level!=0) {
		if(pTree->children()!=NULL)
			return pTree->children()->getLevel(--level);
		else
			return NULL;
	}
	return pTree;
}

CResultsTree* CResultsTree::parse(CString& results) 
{
	TCHAR recordDelim = _T('\n');
	TCHAR fieldDelim = _T('\t');
	int recordStartIdx = 0;
	int recordEndIdx = 0;

	CString* field = NULL;
	CString level;
	CString message;
	CString name;
	CString kind;
	CString tag;
	CString lang;
	CString priv;
	CString prot;
	CString file;
	CString line;
	
	CResultsTree* pRoot = new CResultsTree();
	CResultsTree* pTree = pRoot->children();

	int nResultLength = results.GetLength();
	CString record;
	do {
		recordEndIdx = results.Find(recordDelim,recordStartIdx);
		if(recordEndIdx==-1) 
			record = results.Mid(recordStartIdx);
		else
			record = results.Mid(recordStartIdx,recordEndIdx-recordStartIdx);
		if(record.GetLength()>0) {
			int fieldStartIdx = 0;
			int fieldEndIdx = 0;
			int fieldNum = 0;
			
			field = &level;

			int nRecLen = record.GetLength();

			do {
				fieldEndIdx = record.Find(fieldDelim,fieldStartIdx);
				if(fieldEndIdx==-1) 
					*field = record.Mid(fieldStartIdx);
				else
					*field = record.Mid(fieldStartIdx,fieldEndIdx-fieldStartIdx);
				switch(++fieldNum) {
					case 1: field = &message; break;
					case 2: field = &name; break;
					case 3: field = &kind; break;
					case 4: field = &lang; break;
					case 5: field = &prot; break;
					case 6: field = &priv; break;
					case 7: field = &tag; break;
					case 8: field = &file; break;
					case 9: field = &line; break;
				}
				fieldStartIdx = fieldEndIdx;
				if(fieldStartIdx!=-1)
					fieldStartIdx++;
			}while(fieldStartIdx!=-1);
			if(!(level.IsEmpty() || message.IsEmpty())) {
				int nLevel = atoi(level);
				CResultsTree* pNew = new CResultsTree(message,name,kind,lang,prot,priv,tag,file,line);
				if(nLevel == 0) pRoot->addChild(pNew);
				else {
					CResultsTree* pLevelTree = pTree->getLevel(nLevel);
					if(pLevelTree==NULL)  {
						pLevelTree = pTree->getLevel(nLevel-1);
						pLevelTree->setChildren(pNew);
					} else
						pLevelTree->append(pNew);
				}
				if(nLevel == 0) pTree = pNew;
			}
		}
		recordStartIdx = recordEndIdx;
		if(recordStartIdx!=-1)
			recordStartIdx++;
	} while(recordStartIdx!=-1);
	return pRoot;
} 
