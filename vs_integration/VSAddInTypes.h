/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Jul 07 12:34:19 2004
 */
/* Compiler settings for D:\rd\ci_emancipation\vs_integration\VSAddIn.odl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __VSAddInTypes_h__
#define __VSAddInTypes_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ICommands_FWD_DEFINED__
#define __ICommands_FWD_DEFINED__
typedef interface ICommands ICommands;
#endif 	/* __ICommands_FWD_DEFINED__ */


#ifndef __Commands_FWD_DEFINED__
#define __Commands_FWD_DEFINED__

#ifdef __cplusplus
typedef class Commands Commands;
#else
typedef struct Commands Commands;
#endif /* __cplusplus */

#endif 	/* __Commands_FWD_DEFINED__ */


#ifndef __ApplicationEvents_FWD_DEFINED__
#define __ApplicationEvents_FWD_DEFINED__

#ifdef __cplusplus
typedef class ApplicationEvents ApplicationEvents;
#else
typedef struct ApplicationEvents ApplicationEvents;
#endif /* __cplusplus */

#endif 	/* __ApplicationEvents_FWD_DEFINED__ */


#ifndef __DebuggerEvents_FWD_DEFINED__
#define __DebuggerEvents_FWD_DEFINED__

#ifdef __cplusplus
typedef class DebuggerEvents DebuggerEvents;
#else
typedef struct DebuggerEvents DebuggerEvents;
#endif /* __cplusplus */

#endif 	/* __DebuggerEvents_FWD_DEFINED__ */


void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __VSAddIn_LIBRARY_DEFINED__
#define __VSAddIn_LIBRARY_DEFINED__

/* library VSAddIn */
/* [helpstring][version][uuid] */ 


DEFINE_GUID(LIBID_VSAddIn,0xADD9091A,0xE555,0x4B40,0xB6,0x4B,0xAF,0x0E,0x91,0x03,0xD0,0x3D);

#ifndef __ICommands_INTERFACE_DEFINED__
#define __ICommands_INTERFACE_DEFINED__

/* interface ICommands */
/* [object][dual][oleautomation][uuid] */ 


DEFINE_GUID(IID_ICommands,0xADD90916,0xE555,0x4B40,0xB6,0x4B,0xAF,0x0E,0x91,0x03,0xD0,0x3D);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ADD90916-E555-4B40-B64B-AF0E9103D03D")
    ICommands : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CodeRoverInfoMethod( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CodeRoverOpenDefinitionMethod( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CodeRoverBrowseMethod( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CodeRoverBuildMethod( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CodeRoverActivateMethod( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommands __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommands __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommands __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICommands __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICommands __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICommands __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICommands __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CodeRoverInfoMethod )( 
            ICommands __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CodeRoverOpenDefinitionMethod )( 
            ICommands __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CodeRoverBrowseMethod )( 
            ICommands __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CodeRoverBuildMethod )( 
            ICommands __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CodeRoverActivateMethod )( 
            ICommands __RPC_FAR * This);
        
        END_INTERFACE
    } ICommandsVtbl;

    interface ICommands
    {
        CONST_VTBL struct ICommandsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommands_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommands_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommands_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommands_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICommands_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICommands_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICommands_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICommands_CodeRoverInfoMethod(This)	\
    (This)->lpVtbl -> CodeRoverInfoMethod(This)

#define ICommands_CodeRoverOpenDefinitionMethod(This)	\
    (This)->lpVtbl -> CodeRoverOpenDefinitionMethod(This)

#define ICommands_CodeRoverBrowseMethod(This)	\
    (This)->lpVtbl -> CodeRoverBrowseMethod(This)

#define ICommands_CodeRoverBuildMethod(This)	\
    (This)->lpVtbl -> CodeRoverBuildMethod(This)

#define ICommands_CodeRoverActivateMethod(This)	\
    (This)->lpVtbl -> CodeRoverActivateMethod(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE ICommands_CodeRoverInfoMethod_Proxy( 
    ICommands __RPC_FAR * This);


void __RPC_STUB ICommands_CodeRoverInfoMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ICommands_CodeRoverOpenDefinitionMethod_Proxy( 
    ICommands __RPC_FAR * This);


void __RPC_STUB ICommands_CodeRoverOpenDefinitionMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICommands_CodeRoverBrowseMethod_Proxy( 
    ICommands __RPC_FAR * This);


void __RPC_STUB ICommands_CodeRoverBrowseMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ICommands_CodeRoverBuildMethod_Proxy( 
    ICommands __RPC_FAR * This);


void __RPC_STUB ICommands_CodeRoverBuildMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ICommands_CodeRoverActivateMethod_Proxy( 
    ICommands __RPC_FAR * This);


void __RPC_STUB ICommands_CodeRoverActivateMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommands_INTERFACE_DEFINED__ */


DEFINE_GUID(CLSID_Commands,0xADD90917,0xE555,0x4B40,0xB6,0x4B,0xAF,0x0E,0x91,0x03,0xD0,0x3D);

#ifdef __cplusplus

class DECLSPEC_UUID("ADD90917-E555-4B40-B64B-AF0E9103D03D")
Commands;
#endif

DEFINE_GUID(CLSID_ApplicationEvents,0xADD90918,0xE555,0x4B40,0xB6,0x4B,0xAF,0x0E,0x91,0x03,0xD0,0x3D);

#ifdef __cplusplus

class DECLSPEC_UUID("ADD90918-E555-4B40-B64B-AF0E9103D03D")
ApplicationEvents;
#endif

DEFINE_GUID(CLSID_DebuggerEvents,0xADD90919,0xE555,0x4B40,0xB6,0x4B,0xAF,0x0E,0x91,0x03,0xD0,0x3D);

#ifdef __cplusplus

class DECLSPEC_UUID("ADD90919-E555-4B40-B64B-AF0E9103D03D")
DebuggerEvents;
#endif
#endif /* __VSAddIn_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
