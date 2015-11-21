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
// ImpactAction.h: interface for the CImpactAction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMPACTACTION_H__31D63A6A_2574_4528_A932_9B9305020D84__INCLUDED_)
#define AFX_IMPACTACTION_H__31D63A6A_2574_4528_A932_9B9305020D84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Entity.h"
#include "ResultsTree.h"

class CImpactCtrl;

class CActionData {
public:
	CActionData(CString* pszEntityTag,CString* pszDataTag) {
		m_pszEntityTag = pszEntityTag;
		m_pszDataTag = pszDataTag;
	}

	~CActionData() {
		delete m_pszEntityTag;
		delete m_pszDataTag;
	}
	
	CString* m_pszEntityTag;
	CString* m_pszDataTag;
};

class TActionDataList : public CList<CActionData* ,CActionData*&> {
public:
	POSITION FindTag(CString& szDataTag){
		POSITION pos = GetHeadPosition();
		while(pos!=NULL) {
			CActionData* pData = GetAt(pos);
			if(pData->m_pszDataTag->CompareNoCase(szDataTag)==0)
				return pos;
			GetNext(pos);
		}
		return NULL;
	}
};

class TAcomplishedHash : public CMap<CString, const TCHAR*, TActionDataList*, TActionDataList*&> {
public:
	~TAcomplishedHash() {
		POSITION pos = GetStartPosition();
		while( pos != NULL ) {
			TActionDataList* pList;
			CString szKey;
			// Gets key and value
			GetNextAssoc( pos, szKey, pList );
			pList->RemoveAll();
			delete pList;
		}
		// RemoveAll deletes the keys
		RemoveAll();
	}
};


class CImpactAction : public CObject
{
public:
	virtual CActionData* GetData();
	virtual void SetData(CActionData* pData);
	virtual void Undo();
	bool IsDuplicated();
	void SetDuplicated(bool state);
	void SetCtrl(CImpactCtrl* pCtrl);
	bool HasHash() { return m_pHash!=NULL; };
	TAcomplishedHash* GetHash() { return m_pHash; };
	void SetHash(TAcomplishedHash* pHash) { m_pHash = pHash; };
	CImpactCtrl* GetCtrl();
	DECLARE_DYNCREATE(CImpactAction)

	bool DoRepeat();
	bool IsWait();
	void SetWait(bool state);
	CImpactAction();
	CImpactAction(CString& name,CImpactCtrl* pCtrl,bool repeat = false);
	virtual ~CImpactAction();

    CString& GetName();
    CString& GetDescription() { return m_szDescription; };
    void SetDescription(CString& desc) { m_szDescription = desc; };
    virtual bool ActionPerformed(CEntity* pEntity);
  
    bool IsExecuted() { return m_bExecuted; };
    void SetExecuted(bool state) { 
      m_bExecuted = state; 
      if(!m_bExecuted) {
		if(m_pResults!=NULL) {
			delete m_pResults;
			m_pResults = NULL;
		}
      }
    };

    CResultsTree* GetResults() { return m_pResults; };
    static CString PrepareQuery(const CString& query);

 protected:
	CActionData* m_pData;
    void ParseResult(CString& result) { m_pResults = CResultsTree::parse(result); };
	CImpactCtrl* m_pCtrl;
	TAcomplishedHash* m_pHash;

 private:
	 bool m_bDuplicated;
	 bool m_bRepeatAction;
    CString m_szName;
    CString m_szDescription;
    bool   m_bExecuted;
    CResultsTree* m_pResults;
	bool m_bWaitState;
};

#endif // !defined(AFX_IMPACTACTION_H__31D63A6A_2574_4528_A932_9B9305020D84__INCLUDED_)
