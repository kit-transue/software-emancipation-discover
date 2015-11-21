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
#ifndef _SMARTPOINTER_H_
#define _SMARTPOINTER_H_

///////////////////////////////////////////////////////////////////
#include <assert.h>
template <class T> class IPtr
{
  public:
    IPtr() { m_pI = NULL; }
   ~IPtr() { Release(); }

    IPtr(IPtr<T>& TT) 
      { 
        m_pI = TT.m_pI;
        if ( m_pI != NULL )
          m_pI->AddRef();
      }

    IPtr(T* lp)
    {
      m_pI = lp;
      if (m_pI != NULL )
        m_pI->AddRef();
    }

    void Release()
    {
      if ( m_pI != NULL )
      {
        T* pOld = m_pI;
        m_pI = NULL;
        pOld->Release();
      }
    }

    operator T*() { return m_pI; }

    T&  operator*()  
      { 
        assert(m_pI != NULL); 
        return *m_pI; 
      }

    T** operator&()  
      { 
        assert(m_pI == NULL); 
        return &m_pI; 
      }

	T*  operator->() { assert(m_pI != NULL); return  m_pI; }

    T* operator=(T* pI)
    {
      if ( m_pI != pI )
      {
        T* pOld = m_pI;
        m_pI = pI;
        if ( m_pI != NULL )
          m_pI->AddRef();
        if ( pOld != NULL )
          pOld->Release();
      }
      return m_pI;
    }


    T* operator=(IPtr<T> TT)
    {
      if ( m_pI != TT.m_pI )
      {
        T* pOld = m_pI;
        m_pI = TT.m_pI;
        if ( m_pI != NULL )
          m_pI->AddRef();
        if ( pOld != NULL )
          pOld->Release();
      }
      return m_pI;
    }

  int operator!() { return (m_pI == NULL) ? -1 : 0; }

  private:
    T*  m_pI;
};

#endif // #ifndef _SMARTPOINTER_H_
