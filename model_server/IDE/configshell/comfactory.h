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
#ifndef __CFactory_h__
#define __CFactory_h__

#include "ComUnknown.h"


//
//   - Helper functions registering and unregistering a component.
//

// This function will register a component in the Registry.
// The component calls this function from its DllRegisterServer function.
HRESULT RegisterServer(HMODULE hModule, 
                       const CLSID& clsid, 
                       const char* szFriendlyName,
                       const char* szVerIndProgID,
                       const char* szProgID) ;

// This function will unregister a component.  Components
// call this function from their DllUnregisterServer function.
HRESULT UnregisterServer(const CLSID& clsid,
                         const char* szVerIndProgID,
                         const char* szProgID) ;


///////////////////////////////////////////////////////////
 
// Forward reference
class CFactoryData ;

// Global data used by CFactory
extern CFactoryData g_FactoryDataArray[] ;
extern int g_cFactoryDataEntries ;

//////////////////////////////////////////////////////////
//
//  Component creation function
//
class CUnknown ;

typedef HRESULT (*FPCREATEINSTANCE)(IUnknown*, CUnknown**) ;

///////////////////////////////////////////////////////////
//
// CFactoryData
//   - Information CFactory needs to create a component
//     supported by the DLL
//
class CFactoryData
{
public:
	// The class ID for the component
	const CLSID* m_pCLSID ;

	// Pointer to the function that creates it
	FPCREATEINSTANCE CreateInstance ;

	// Name of the component to register in the registry
	const char* m_RegistryName ;

	// ProgID
	const char* m_szProgID ;

	// Version-independent ProgID
	const char* m_szVerIndProgID ;

	// Helper function for finding the class ID
	BOOL IsClassID(const CLSID& clsid) const
		{ return (*m_pCLSID == clsid) ;}

	//
	// Out of process server support
	//

	// Pointer to running class factory for this component
	IClassFactory* m_pIClassFactory ;

	// Magic cookie to identify running object
	DWORD m_dwRegister ;
} ;


///////////////////////////////////////////////////////////
//
// Class Factory
//
class CFactory : public IClassFactory
{
public:
	// IUnknown
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv) ;
	virtual ULONG   __stdcall AddRef() ;
	virtual ULONG   __stdcall Release() ;
	
	// IClassFactory
	virtual HRESULT __stdcall CreateInstance(IUnknown* pUnknownOuter,
	                                         const IID& iid,
	                                         void** ppv) ;
	virtual HRESULT __stdcall LockServer(BOOL bLock) ; 

	// Constructor - Pass pointer to data of component to create.
	CFactory(const CFactoryData* pFactoryData) ;

	// Destructor
	~CFactory() { }

	//
	// Static FactoryData support functions
	//

	// DllGetClassObject support
	static HRESULT GetClassObject(const CLSID& clsid, 
	                              const IID& iid, 
	                              void** ppv) ;

	// Helper function for DllCanUnloadNow 
 	static BOOL IsLocked()
		{ return (s_cServerLocks > 0) ;}

	// Functions to [un]register all components
	static HRESULT RegisterAll() ;
	static HRESULT UnregisterAll() ;

	// Function to determine if component can be unloaded
	static HRESULT CanUnloadNow() ;


	//
	// Out-of-process server support
	//

	static BOOL StartFactories() ;
	static void StopFactories() ;

	static DWORD s_dwThreadID ;

	// Shut down the application.
	static void CloseExe()
	{
		if (CanUnloadNow() == S_OK)
		{
			::PostThreadMessage(s_dwThreadID, WM_QUIT, 0, 0) ;
		}
	}

public:
	// Reference Count
	LONG m_cRef ;

	// Pointer to information about class this factory creates
	const CFactoryData* m_pFactoryData ;

	// Count of locks
	static LONG s_cServerLocks ;   

	// Module handle
	static HMODULE s_hModule ;
} ;

#endif
