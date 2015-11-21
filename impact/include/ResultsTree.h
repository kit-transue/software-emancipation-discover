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
#ifndef __RESULTS_TREE_HEADER_FILE
#define __RESULTS_TREE_HEADER_FILE

#include "stl.h"

class CResultEntity {
public:
  CResultEntity();
  CResultEntity(string& message, string& name, string& kind, string& tag,
	       string& lang,string&  priv, string& prot,  string& file,
	       string& line);
  ~CResultEntity();

  string* getName() { return &m_name; };
  void setName(string& name) { m_name = name; };

  string* getMessage();
  void setMessage(string& message) { m_message = message; };
  
  string* getKind() { return &m_kind; };
  void setKind(string& kind) { m_kind = kind; };

  string* getTag() { return &m_tag; };
  void setTag(string& tag) { m_tag = tag; };

  string* getLang() { return &m_lang; };
  void setLang(string& lang) { m_lang = lang; };

  void setPrivate(string& priv) { m_priv = (priv.compare("1")==0); };
  void setProtected(string& prot) { m_prot = (prot.compare("1")==0); };

  bool isPrivate() { return m_priv; };
  bool isProtected() { return m_prot; };
  bool isPublic() { return !(isPrivate() || isProtected()); };
 
  string* getFile() { return &m_file; };
  void setFile(string& file) { m_file = file; };

  int getLine() { return m_line; };
  void setLine(string& line) { m_line = atoi(line.c_str()); };
private:
  string m_message;
  string m_name;
  string m_kind;
  string m_tag;
  string m_lang;
  bool m_priv;
  bool m_prot;
  string m_file;
  int m_line;
};

class CResultsTree;
class CResultsTree {
public:
  CResultsTree();
  CResultsTree(string& message, string& name, string& kind, string& tag,
	       string& lang,string&  priv, string& prot,  string& file,
	       string& line);
  ~CResultsTree();
  CResultEntity* getEntity() { return m_pEntity; };
  CResultsTree* getNext() { return m_pNext; };
  CResultsTree* getPrev() { return m_pPrev; };
  void setNext(CResultsTree* pNext) { m_pNext = pNext; };
  void setPrev(CResultsTree* pPrev) { m_pPrev = pPrev; };
  void setChildren(CResultsTree* pChildren) { pChildren->setParent(this);m_pChildren = pChildren;};
  CResultsTree* children() { return m_pChildren; };
  void addChild(CResultsTree* pChild) { 
	  if(m_pChildren==NULL)
		m_pChildren = pChild;
	  else
		m_pChildren->append(pChild); 
  };

  void setParent(CResultsTree* pParent) { m_pParent = pParent; };
  CResultsTree* getParent() { return m_pParent; };
  bool isRoot() { return m_pParent == NULL; };

  CResultsTree* getLevel(int level);
  CResultsTree* get(int idx);
  void append(CResultsTree* pEntity); 
  static CResultsTree* parse(string results);
private:
  CResultEntity* m_pEntity;
  CResultsTree* m_pChildren;
  CResultsTree* m_pNext;
  CResultsTree* m_pParent;
  CResultsTree* m_pPrev;
};

#endif //__RESULTS_TREE_HEADER_FILE
