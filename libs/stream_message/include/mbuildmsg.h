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
#ifndef __MBUILDMSG_H__
#define __MBUILDMSG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "minidom.h"
#include <stdlib.h>

namespace MBDriver {
template <class T>
class Wrapper
{
public:
    Wrapper(const T& _t) : t(_t)
    {
    }

#ifdef WIN32
    template <typename _Ty>
	void operator delete(void *pMem, _Ty* p) 
#else
    void operator delete(void *pMem, void *p)
#endif
    {
    }

#ifdef WIN32
    template <typename _Ty>
    void *operator new(size_t, _Ty* p)
#else
    void *operator new(size_t, void* p)
#endif
    {
	return p;
    }
    T t;
};

template <class T> class TArray
{
private:
    // prevent generic copying
    TArray(const TArray<T> &other) {}

public:
    T* m_aT;
    int m_nSize;
    int m_nAllocSize;

// Construction/destruction
    TArray() : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
    { }


    ~TArray()
    {
	RemoveAll();
    }

// Operations
    int GetSize() const
    {
	return m_nSize;
    }

    int Append(const TArray<T> &src) {
	if(m_nSize + src.m_nSize > m_nAllocSize) {
	    T* aT;
	    int nNewAllocSize = (m_nAllocSize + src.m_nSize == 0) ? 1 : m_nAllocSize + src.m_nSize;
	    aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
	    if(aT == NULL)
		return false;
	    m_nAllocSize = nNewAllocSize;
	    m_aT = aT;
	}
	int j	 = m_nSize;
	m_nSize += src.m_nSize;
	for(int i = 0; i < src.m_nSize; i++, j++)
	    SetAtIndex(j, src[i]);
	return m_nSize - 1;
    }
    
    int Add(const T& t)
    {
	if(m_nSize == m_nAllocSize)
	{
	    T* aT;
	    int nNewAllocSize = (m_nAllocSize == 0) ? 1 : (m_nSize * 2);
	    aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
	    if(aT == NULL)
		return false;
	    m_nAllocSize = nNewAllocSize;
	    m_aT = aT;
	}
	m_nSize++;
	SetAtIndex(m_nSize - 1, t);
	return m_nSize - 1;
    }
    bool Remove(T& t)
    {
	int nIndex = Find(t);
	if(nIndex == -1)
	    return false;
	return RemoveAt(nIndex);
    }
    bool RemoveAt(int nIndex)
    {
	if(nIndex != (m_nSize - 1))
	{
	    m_aT[nIndex].~T();
	    memmove((void*)&m_aT[nIndex], (void*)&m_aT[nIndex + 1], (m_nSize - (nIndex + 1)) * sizeof(T));
	}
	m_nSize--;
	return true;
    }
    void RemoveAll()
    {
	if(m_aT != NULL)
	{
	    for(int i = 0; i < m_nSize; i++)
		m_aT[i].~T();
	    free(m_aT);
	    m_aT = NULL;
	}
	m_nSize = 0;
	m_nAllocSize = 0;
    }
    T& operator[] (int nIndex) const
    {
	return GetAt(nIndex);
    }
    T& GetAt(int nIndex) const
    {
	if(nIndex < 0 || nIndex >= m_nSize)
	{
	    //char szBuf[256];
	    //sprintf(szBuf,"Parameter out of range: file %s, line %d",__FILE__, __LINE__);
	    //throw xln_error(1,szBuf);
	    throw;
	}
	return m_aT[nIndex];
    }
    T* GetData() const
    {
	return m_aT;
    }

// Implementation
    void SetAtIndex(int nIndex, const T& t)
    {
	if(nIndex < 0 || nIndex >= m_nSize)
	{
	    //char szBuf[256];
	    //sprintf(szBuf,"Parameter out of range: file %s, line %d",__FILE__, __LINE__);
	    //throw t_error(1,szBuf);
	    throw;
	}
	new(&m_aT[nIndex]) Wrapper<T>(t);
    }
    int Find(T& t) const
    {
	for(int i = 0; i < m_nSize; i++)
	{
	    if(m_aT[i] == t)
		return i;
	}
	return -1;  // not found
    }
};

class Message;
class Attribute {
    typedef MiniXML::DOMString DOMString;
public:
    Attribute(const DOMString attributeName, const DOMString attributeValue) :
      m_attributeName(attributeName), m_attributeValue(attributeValue) {}

protected:
    DOMString m_attributeName;
    DOMString m_attributeValue;

    friend class Message;
};

typedef TArray<Attribute> AttributeArray;

class Message {
    typedef MiniXML::DOMString DOMString;
    typedef MiniXML::XMLCh XMLCh;
    // typedef MiniXML::DOM_Node DOM_Node;
public:
    Message(const DOMString &elementName /* command name */) : m_elementName(elementName) {};

    Message(const Message &src) {
	m_elementName = src.m_elementName;
	m_text	      = src.m_text;
	m_attributes.Append(src.m_attributes);
	m_children.Append(src.m_children);
    }

    void GetTag(DOMString &tag) const {
	tag = m_elementName;
    }

    void SetText(const DOMString &text) {
	m_text = text;
    }

    void AddAttribute(const DOMString attributeName, const DOMString attributeValue) {
	m_attributes.Add(Attribute(attributeName, attributeValue));
    }

    void AddChild(Message &msg) {
	m_children.Add(msg);
    }

    int GetAttributeCount() const {
	return m_attributes.GetSize();
    }

    void GetAttribute(int index, DOMString &value) const {
	value = m_attributes[index].m_attributeValue;
    }

    bool EnumerateChildren(int &enumerator) const;
    bool GetNextChild(int &enumerator, Message **msg) const;
    void AddText(const DOMString &text);
    void Encode(DOMString &buffer);
    bool GetAttributeByName(const DOMString &attributeName, DOMString &value) const;
    bool GetAttributeByName(const XMLCh *attributeName, DOMString &value) const;
    void GetText(DOMString &value) const;

    static Message *Decode(const char *bufPtr, unsigned long bufLen);

protected:
    Message() {};

    void GetTextInternal(DOMString &value) const;
    void EncodeString(DOMString &result, DOMString &toWrite);

#if 0
    static Message *ParseMsg(DOM_Node &node);
#endif

    DOMString	      m_elementName;
    DOMString	      m_text;
    AttributeArray m_attributes;
    TArray<Message>   m_children;

};
}
#endif
