/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.15 */
/* at Tue Jul 08 18:10:20 1997
 */
/* Compiler settings for comserver.idl:
    Os, W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __comIface_h__
#define __comIface_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IConfigShell_FWD_DEFINED__
#define __IConfigShell_FWD_DEFINED__
typedef interface IConfigShell IConfigShell;
#endif 	/* __IConfigShell_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IConfigShell_INTERFACE_DEFINED__
#define __IConfigShell_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IConfigShell
 * at Tue Jul 08 18:10:20 1997
 * using MIDL 3.00.15
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IConfigShell;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IConfigShell : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPSETCFG( 
            /* [string][in] */ wchar_t __RPC_FAR *wcsPSETCFG) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitCM( 
            /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *wcsCMName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Run( 
            /* [string][in] */ wchar_t __RPC_FAR *wcsCmd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResult( 
            /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *wcsResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IConfigShellVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IConfigShell __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IConfigShell __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IConfigShell __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPSETCFG )( 
            IConfigShell __RPC_FAR * This,
            /* [string][in] */ wchar_t __RPC_FAR *wcsPSETCFG);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitCM )( 
            IConfigShell __RPC_FAR * This,
            /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *wcsCMName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Run )( 
            IConfigShell __RPC_FAR * This,
            /* [string][in] */ wchar_t __RPC_FAR *wcsCmd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetResult )( 
            IConfigShell __RPC_FAR * This,
            /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *wcsResult);
        
        END_INTERFACE
    } IConfigShellVtbl;

    interface IConfigShell
    {
        CONST_VTBL struct IConfigShellVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConfigShell_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConfigShell_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConfigShell_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConfigShell_SetPSETCFG(This,wcsPSETCFG)	\
    (This)->lpVtbl -> SetPSETCFG(This,wcsPSETCFG)

#define IConfigShell_InitCM(This,wcsCMName)	\
    (This)->lpVtbl -> InitCM(This,wcsCMName)

#define IConfigShell_Run(This,wcsCmd)	\
    (This)->lpVtbl -> Run(This,wcsCmd)

#define IConfigShell_GetResult(This,wcsResult)	\
    (This)->lpVtbl -> GetResult(This,wcsResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IConfigShell_SetPSETCFG_Proxy( 
    IConfigShell __RPC_FAR * This,
    /* [string][in] */ wchar_t __RPC_FAR *wcsPSETCFG);


void __RPC_STUB IConfigShell_SetPSETCFG_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConfigShell_InitCM_Proxy( 
    IConfigShell __RPC_FAR * This,
    /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *wcsCMName);


void __RPC_STUB IConfigShell_InitCM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConfigShell_Run_Proxy( 
    IConfigShell __RPC_FAR * This,
    /* [string][in] */ wchar_t __RPC_FAR *wcsCmd);


void __RPC_STUB IConfigShell_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConfigShell_GetResult_Proxy( 
    IConfigShell __RPC_FAR * This,
    /* [string][out] */ wchar_t __RPC_FAR *__RPC_FAR *wcsResult);


void __RPC_STUB IConfigShell_GetResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IConfigShell_INTERFACE_DEFINED__ */



#ifndef __ServerLib_LIBRARY_DEFINED__
#define __ServerLib_LIBRARY_DEFINED__

/****************************************
 * Generated header for library: ServerLib
 * at Tue Jul 08 18:10:20 1997
 * using MIDL 3.00.15
 ****************************************/
/* [helpstring][version][uuid] */ 



EXTERN_C const IID LIBID_ServerLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_ConfigShell;

class ConfigShell;
#endif
#endif /* __ServerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
