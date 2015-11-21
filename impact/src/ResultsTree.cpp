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
#include "ResultsTree.h"

CResultEntity::CResultEntity() {
        m_priv = false;
        m_prot = false;
        m_line = -1;
}

CResultEntity::CResultEntity(string& message, string& name, string& kind, string& tag,string& lang,string&  priv, string& prot,  string& file, string& line) {
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

string* CResultEntity::getMessage() {
        static string szDescription;

         if(m_message.compare("POSSIBLE_NAME_CONFLICT")==0) szDescription = "Potential name conflict";
        else if(m_message.compare("NEED_CHANGE")==0) szDescription = "Entities that need to be changed";
        else if(m_message.compare("NEED_DECLARATION")==0) szDescription = "Variables that should be explicitly declared.";
        else if(m_message.compare("INCLUDED")==0) szDescription = "Included in files";
        else if(m_message.compare("METHOD_EXISTS")==0) szDescription = "Method already exists";
        else if(m_message.compare("FIELD_EXISTS")==0) szDescription = "Field already exists";
        else if(m_message.compare("FUNCTION_EXISTS")==0) szDescription = "Function already exists";
        else if(m_message.compare("MEMBER_EXISTS")==0) szDescription = "Member already exists";
        else if(m_message.compare("MACRO_EXISTS")==0) szDescription = "Macros with the same name where detected";
        else if(m_message.compare("VAR_EXISTS")==0) szDescription = "Variable already exists";
        else if(m_message.compare("STATIC_VAR_EXISTS")==0) szDescription = "Static variables with the same name";
        else if(m_message.compare("TEMPLATE_EXISTS")==0) szDescription = "Templates with the same name";
        else if(m_message.compare("OVERLOAD_VIRTUAL_BASE")==0) szDescription = "Base class virtual method overloaded";
        else if(m_message.compare("HIDE_METHOD_BASE")==0) szDescription = "Method hides base class method";
        else if(m_message.compare("HIDE_NONVIRTUAL_BASE")==0) szDescription = "Method hides base class method";
        else if(m_message.compare("HIDE_NONVIRTUAL_DERIVED")==0) szDescription = "Derived class method hides this method";
        else if(m_message.compare("OVERLOAD_VIRTUAL_DERIVED")==0) szDescription = "Derived class method overloads this method";
        else if(m_message.compare("INNER_CLASS")==0) szDescription = "This field is declared in an inner class and cannot be declared static.";
        else if(m_message.compare("RECOMPILE_FILE")==0) szDescription = "Files that need to be recompiled";
        else if(m_message.compare("EXTENDING_INTERFACES")==0) szDescription = "Extending interfaces, impact of deleting these may not be displayed";
        else if(m_message.compare("FILES_NEEDING_DECL")==0) szDescription = "Files requiring definition of variable.";
		else if(m_message.compare("REFERENCED_BUT_NOT_DEFINED")==0) szDescription = "Referenced but undefined entities";
        else if(m_message.compare("POSSIBLE_ILLEGAL_USE")==0) szDescription = "Method or field may be in use";
        else if(m_message.compare("INCORRECT_VALUE")==0) szDescription = "Variable may have incorrect value";
        else if(m_message.compare("POSSIBLE_VALUE_OVERWRITTING")==0) szDescription = "Possible value overwritting";
        else if(m_message.compare("ILLEGAL_ACCESS")==0) szDescription =  "Inaccessible entities";
        else if(m_message.compare("CHECK_LOGIC")==0) szDescription = "Instances that need to be reviewed";
        else if(m_message.compare("STOPS_HIDING_DERIVED")==0) szDescription = "Derived class method overloads this method";
        else if(m_message.compare("STARTS_HIDING_DERIVED")==0) szDescription = "Derived class method hides this method";
        else if(m_message.compare("FUNCTION_BODY_NOT_IN_THE_MODEL")==0) szDescription = string("Functions defined outside of the information model. ") + string("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.compare("MACRO_BODY_NOT_IN_THE_MODEL")==0) szDescription = string("Macros defined outside of the information model. ") + string("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.compare("TEMPLATE_BODY_NOT_IN_THE_MODEL")==0) szDescription = string("Templates defined outside of the information model. ") + string("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.compare("CLASS_BODY_NOT_IN_THE_MODEL")==0) szDescription = string("Classes defined outside of the information model. ") + string("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.compare("INTERFACE_NOT_IN_THE_MODEL")==0) szDescription = string("Interfaces defined outside of the information model. ") + string("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.compare("FIELD_NOT_IN_THE_MODEL")==0) szDescription = string("Fields defined outside of the information model. ") + string("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.compare("BODY_NOT_IN_THE_MODEL")==0) szDescription = string("Entities defined outside of the information model. ") + string("To generate an accurate Impact Analysis, all source files must be included in the information model");
        else if(m_message.compare("HIDES_BASE_CLASS_NONPRIVATE_FIELD")==0) szDescription = "Fields that hide a base class non-private field";
        else if(m_message.compare("HIDES_THIS_CLASS_NONPRIVATE_FIELD")==0) szDescription = "Derived class fields  that hide this non-private field";
        else if(m_message.compare("CANNOT_CHANGE_VALUE_OF_FINAL_VARIABLE")==0) szDescription = "Value can not be assigned to a constant variable";
        else if(m_message.compare("PURE_VIRTUAL_OVERLOAD")==0) szDescription = "Methods that need to be implemented in extending classes";
        else if(m_message.compare("NEED_REMOVE_METHODS")==0) szDescription = "Methods that need to be reviewed";
        else if(m_message.compare("NATIVE_METHOD_CANNOT_HAVE_IMPLEMENTATION")==0) szDescription = "Native methods can't have body";
        else if(m_message.compare("MULTIPLE_DEFINES")==0) szDescription = "Cannot have multiple definitions of variable.";
        else if(m_message.compare("NEED_IMPLEMENT_METHODS")==0) szDescription = "Interface methods that need to be implemented";
        else if(m_message.compare("OVERLOAD_FINAL_METHOD")==0) szDescription = "Method declared as final in base class cannot be overriden!";
        else if(m_message.compare("RENAME_VIRTUAL_DERIVED")==0) szDescription = "Derived class methods that should be reviewed";
		else if(m_message.compare("ABSTRACT_CLASS")==0) szDescription="Classes that are abstract and can't be instantiated";
		else if(m_message.compare("ABSTRACT_METHOD")==0) szDescription="Abstract methods that should be implemented";
		else if(m_message.compare("FINAL_CLASS_CANNOT_HAVE_SUBCLASSES")==0) szDescription="Final class cannot have any subclasses";
		else if(m_message.compare("NON_STATIC")==0) szDescription="Non static entities that can't be referenced from a static context";
		else if(m_message.compare("METHOD_NEEDS_IMPL")==0) szDescription="Classes that should implement abstract method";
		else if(m_message.compare("METHOD_NEEDS_IMPL_CPP")==0) szDescription="Classes that should implement virtual method";
        else if(m_message.compare("CALLING_NONCONST")==0) szDescription="Non const methods that cannot be referenced from a const context";
		else if(m_message.compare("ASSIGNING_VALUES_IN_CONST_FUNCTION")==0) szDescription="Values modified in a const function";
		else if(m_message.compare("SHOULD_BE_DECLARED_ABSTRACT")==0) szDescription="Classes that should be declared as abstract";
		else if(m_message.compare("ENTITY")==0) szDescription=m_name;
        else if(m_message.compare("INSTANCE")==0) {
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

CResultsTree::CResultsTree(string& message, string& name, string& kind, string& tag,string& lang,string&  priv, string& prot,  string& file, string& line) {
        m_pEntity = new CResultEntity(message,name,kind,tag,lang,priv,prot,file,line);
        m_pChildren = NULL;
        m_pNext = NULL;
        m_pParent = NULL;
        m_pPrev = NULL;
}

CResultsTree::~CResultsTree() {
        if(m_pEntity!=NULL)     delete m_pEntity;
        if(m_pChildren!=NULL)   delete m_pChildren;
        if(m_pNext!=NULL)       delete m_pNext;
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
        while (pTree->getNext()!=NULL)
            pTree=pTree->getNext();

        if(level!=0) {
            if(pTree->children()!=NULL)
                return pTree->children()->getLevel(--level);
            else
                return NULL;
        }
        return pTree;
}

CResultsTree* CResultsTree::parse(string results) {
        string recordDelim = "\n";
        string fieldDelim = "\t";
        string::size_type recordStartIdx = 0;
        string::size_type recordEndIdx = 0;
        
        CResultsTree* pRoot = new CResultsTree();
        CResultsTree* pTree = pRoot->children();

        int nResultLength = results.length();
        string record;
        do {
                recordEndIdx = results.find(recordDelim,recordStartIdx);
                if(recordEndIdx==string::npos) 
                        record = results.substr(recordStartIdx);
                else
                        record = results.substr(recordStartIdx,recordEndIdx-recordStartIdx);
                if(record.length()>0) {
                        string::size_type fieldStartIdx = 0;
                        string::size_type fieldEndIdx = 0;
                        
                        string level;
                        string message;
                        string name;
                        string kind;
                        string tag;
                        string lang;
                        string priv;
                        string prot;
                        string file;
                        string line;

                        int fieldNum = 0;
                        string* field = &level;

                        int nRecLen = record.length();

                        do {
                                fieldEndIdx = record.find(fieldDelim,fieldStartIdx);
                                if(fieldEndIdx==string::npos) 
                                        *field = record.substr(fieldStartIdx);
                                else
                                        *field = record.substr(fieldStartIdx,fieldEndIdx-fieldStartIdx);
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
                                if(fieldStartIdx!=string::npos)
                                        fieldStartIdx++;
                        }while(fieldStartIdx!=string::npos);
                        if(!(level.empty() || message.empty())) {
                                int nLevel = atoi(level.c_str());
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
                if(recordStartIdx!=string::npos)
                        recordStartIdx++;
        } while(recordStartIdx!=string::npos);
        return pRoot;
}
