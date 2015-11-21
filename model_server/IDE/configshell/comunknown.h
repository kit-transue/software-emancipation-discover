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
#ifndef __CUnknown_h__
#define __CUnknown_h__

#include <objbase.h>

///////////////////////////////////////////////////////////
//
// Nondelegating IUnknown interface
//   - Nondelegating version of IUnknown
//
interface INondelegatingUnknown
{
	virtual HRESULT __stdcall 
		NondelegatingQueryInterface(const IID& iid, void** ppv) = 0 ;
	virtual ULONG   __stdcall NondelegatingAddRef() = 0 ;
	virtual ULONG   __stdcall NondelegatingRelease() = 0 ;
} ;


///////////////////////////////////////////////////////////
//
// Declaration of CUnknown 
//   - Base class for implementing IUnknown
//

class CUnknown : public INondelegatingUnknown
{
public:
	// Nondelegating IUnknown implementation
	virtual HRESULT __stdcall NondelegatingQueryInterface(const IID&,
	                                                      void**) ;
	virtual ULONG   __stdcall NondelegatingAddRef() ;
	virtual ULONG   __stdcall NondelegatingRelease() ;

	// Constructor
	CUnknown(IUnknown* pUnknownOuter) ;

	// Destructor
	virtual ~CUnknown() ;

	// Initialization (especially for aggregates)
	virtual HRESULT Init() { return S_OK ;}

	// Notification to derived classes that we are releasing
	virtual void FinalRelease() ;

	// Count of currently active components
	static long ActiveComponents() 
		{ return s_cActiveComponents ;}
	
	// Helper function
	HRESULT FinishQI(IUnknown* pI, void** ppv) ;

protected:
	// Support for delegation
	IUnknown* GetOuterUnknown() const
		{ return m_pUnknownOuter ;}

private:
	// Reference count for this object
	long m_cRef ;
	
	// Pointer to (external) outer IUnknown
	IUnknown* m_pUnknownOuter ;

	// Count of all active instances
	static long s_cActiveComponents ; 
} ;


///////////////////////////////////////////////////////////
//
// Delegating IUnknown
//   - Delegates to the nondelegating IUnknown, or to the
//     outer IUnknown if the component is aggregated.
//
#define DECLARE_IUNKNOWN		                             \
	virtual HRESULT __stdcall	                             \
		QueryInterface(const IID& iid, void** ppv)           \
	{	                                                     \
		return GetOuterUnknown()->QueryInterface(iid,ppv) ;  \
	} ;	                                                     \
	virtual ULONG __stdcall AddRef()	                     \
	{	                                                     \
		return GetOuterUnknown()->AddRef() ;                 \
	} ;	                                                     \
	virtual ULONG __stdcall Release()	                     \
	{	                                                     \
		return GetOuterUnknown()->Release() ;                \
	} ;


///////////////////////////////////////////////////////////


#endif
