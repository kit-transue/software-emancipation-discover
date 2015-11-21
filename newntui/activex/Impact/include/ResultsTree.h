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
// ResultsTree.h: interface for the CResultsTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESULTSTREE_H__33116D05_A6C6_48BE_8DD9_03A437D6E192__INCLUDED_)
#define AFX_RESULTSTREE_H__33116D05_A6C6_48BE_8DD9_03A437D6E192__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CResultEntity {
public:
  CResultEntity();
  CResultEntity(CString& message, CString& name, CString& kind, CString& tag,
	       CString& lang,CString&  priv, CString& prot,  CString& file,
	       CString& line);
  ~CResultEntity();

  CString* getName() { return &m_name; };
  void setName(CString& name) { m_name = name; };

  CString* getMessage();
  void setMessage(CString& message) { m_message = message; };
  
  CString* getKind() { return &m_kind; };
  void setKind(CString& kind) { m_kind = kind; };

  CString* getTag() { return &m_tag; };
  void setTag(CString& tag) { m_tag = tag; };

  CString* getLang() { return &m_lang; };
  void setLang(CString& lang) { m_lang = lang; };

  void setPrivate(CString& priv) { m_priv = (priv.CompareNoCase("1")==0); };
  void setProtected(CString& prot) { m_prot = (prot.CompareNoCase("1")==0); };

  bool isPrivate() { return m_priv; };
  bool isProtected() { return m_prot; };
  bool isPublic() { return !(isPrivate() || isProtected()); };
 
  CString* getFile() { return &m_file; };
  void setFile(CString& file) { m_file = file; };

  int getLine() { return m_line; };
  void setLine(CString& line) { m_line = atoi((LPCSTR)line); };
private:
  CString m_message;
  CString m_name;
  CString m_kind;
  CString m_tag;
  CString m_lang;
  bool m_priv;
  bool m_prot;
  CString m_file;
  int m_line;
};

class CResultsTree;
class CResultsTree {
public:
  CResultsTree();
  CResultsTree(CString& message, CString& name, CString& kind, CString& tag,
	       CString& lang,CString&  priv, CString& prot,  CString& file,
	       CString& line);
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
  static CResultsTree* parse(CString& results);
private:
  CResultEntity* m_pEntity;
  CResultsTree* m_pChildren;
  CResultsTree* m_pNext;
  CResultsTree* m_pParent;
  CResultsTree* m_pPrev;
};
 
#endif // !defined(AFX_RESULTSTREE_H__33116D05_A6C6_48BE_8DD9_03A437D6E192__INCLUDED_)
