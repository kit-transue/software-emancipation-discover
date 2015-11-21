/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.01.75 */
/* at Wed Sep 03 12:03:46 1997
 */
/* Compiler settings for Logger.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __Logger_h__
#define __Logger_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ILoggerTrace_FWD_DEFINED__
#define __ILoggerTrace_FWD_DEFINED__
typedef interface ILoggerTrace ILoggerTrace;
#endif 	/* __ILoggerTrace_FWD_DEFINED__ */


#ifndef __ILoggerDebug_FWD_DEFINED__
#define __ILoggerDebug_FWD_DEFINED__
typedef interface ILoggerDebug ILoggerDebug;
#endif 	/* __ILoggerDebug_FWD_DEFINED__ */


#ifndef __LoggerTrace_FWD_DEFINED__
#define __LoggerTrace_FWD_DEFINED__

#ifdef __cplusplus
typedef class LoggerTrace LoggerTrace;
#else
typedef struct LoggerTrace LoggerTrace;
#endif /* __cplusplus */

#endif 	/* __LoggerTrace_FWD_DEFINED__ */


#ifndef __LoggerDebug_FWD_DEFINED__
#define __LoggerDebug_FWD_DEFINED__

#ifdef __cplusplus
typedef class LoggerDebug LoggerDebug;
#else
typedef struct LoggerDebug LoggerDebug;
#endif /* __cplusplus */

#endif 	/* __LoggerDebug_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ILoggerTrace_INTERFACE_DEFINED__
#define __ILoggerTrace_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ILoggerTrace
 * at Wed Sep 03 12:03:46 1997
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][dual][uuid][object] */ 



EXTERN_C const IID IID_ILoggerTrace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("74A56660-1A59-11D1-96A6-00A0245836D0")
    ILoggerTrace : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CommandLine( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CommandLine( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PID( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PID( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LogInstance( 
            /* [in] */ long PID,
            /* [string][in] */ BSTR strCommandLine) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LogEntry( 
            /* [in] */ long objEntry,
            /* [string][in] */ BSTR strEntryName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LogExit( 
            /* [in] */ long objEntry) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LogVal( 
            /* [in] */ long objEntry,
            /* [string][in] */ BSTR strName,
            /* [in] */ VARIANT varVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILoggerTraceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ILoggerTrace __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ILoggerTrace __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ILoggerTrace __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CommandLine )( 
            ILoggerTrace __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CommandLine )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PID )( 
            ILoggerTrace __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PID )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LogInstance )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ long PID,
            /* [string][in] */ BSTR strCommandLine);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LogEntry )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ long objEntry,
            /* [string][in] */ BSTR strEntryName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LogExit )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ long objEntry);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LogVal )( 
            ILoggerTrace __RPC_FAR * This,
            /* [in] */ long objEntry,
            /* [string][in] */ BSTR strName,
            /* [in] */ VARIANT varVal);
        
        END_INTERFACE
    } ILoggerTraceVtbl;

    interface ILoggerTrace
    {
        CONST_VTBL struct ILoggerTraceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILoggerTrace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILoggerTrace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILoggerTrace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILoggerTrace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILoggerTrace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILoggerTrace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILoggerTrace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILoggerTrace_get_CommandLine(This,pVal)	\
    (This)->lpVtbl -> get_CommandLine(This,pVal)

#define ILoggerTrace_put_CommandLine(This,newVal)	\
    (This)->lpVtbl -> put_CommandLine(This,newVal)

#define ILoggerTrace_get_PID(This,pVal)	\
    (This)->lpVtbl -> get_PID(This,pVal)

#define ILoggerTrace_put_PID(This,newVal)	\
    (This)->lpVtbl -> put_PID(This,newVal)

#define ILoggerTrace_LogInstance(This,PID,strCommandLine)	\
    (This)->lpVtbl -> LogInstance(This,PID,strCommandLine)

#define ILoggerTrace_LogEntry(This,objEntry,strEntryName)	\
    (This)->lpVtbl -> LogEntry(This,objEntry,strEntryName)

#define ILoggerTrace_LogExit(This,objEntry)	\
    (This)->lpVtbl -> LogExit(This,objEntry)

#define ILoggerTrace_LogVal(This,objEntry,strName,varVal)	\
    (This)->lpVtbl -> LogVal(This,objEntry,strName,varVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ILoggerTrace_get_CommandLine_Proxy( 
    ILoggerTrace __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB ILoggerTrace_get_CommandLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ILoggerTrace_put_CommandLine_Proxy( 
    ILoggerTrace __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB ILoggerTrace_put_CommandLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE ILoggerTrace_get_PID_Proxy( 
    ILoggerTrace __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB ILoggerTrace_get_PID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE ILoggerTrace_put_PID_Proxy( 
    ILoggerTrace __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB ILoggerTrace_put_PID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILoggerTrace_LogInstance_Proxy( 
    ILoggerTrace __RPC_FAR * This,
    /* [in] */ long PID,
    /* [string][in] */ BSTR strCommandLine);


void __RPC_STUB ILoggerTrace_LogInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILoggerTrace_LogEntry_Proxy( 
    ILoggerTrace __RPC_FAR * This,
    /* [in] */ long objEntry,
    /* [string][in] */ BSTR strEntryName);


void __RPC_STUB ILoggerTrace_LogEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILoggerTrace_LogExit_Proxy( 
    ILoggerTrace __RPC_FAR * This,
    /* [in] */ long objEntry);


void __RPC_STUB ILoggerTrace_LogExit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILoggerTrace_LogVal_Proxy( 
    ILoggerTrace __RPC_FAR * This,
    /* [in] */ long objEntry,
    /* [string][in] */ BSTR strName,
    /* [in] */ VARIANT varVal);


void __RPC_STUB ILoggerTrace_LogVal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILoggerTrace_INTERFACE_DEFINED__ */


#ifndef __ILoggerDebug_INTERFACE_DEFINED__
#define __ILoggerDebug_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ILoggerDebug
 * at Wed Sep 03 12:03:46 1997
 * using MIDL 3.01.75
 ****************************************/
/* [unique][helpstring][dual][uuid][object] */ 



EXTERN_C const IID IID_ILoggerDebug;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("74A56662-1A59-11D1-96A6-00A0245836D0")
    ILoggerDebug : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct ILoggerDebugVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ILoggerDebug __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ILoggerDebug __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ILoggerDebug __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ILoggerDebug __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ILoggerDebug __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ILoggerDebug __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ILoggerDebug __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } ILoggerDebugVtbl;

    interface ILoggerDebug
    {
        CONST_VTBL struct ILoggerDebugVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILoggerDebug_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILoggerDebug_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILoggerDebug_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILoggerDebug_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILoggerDebug_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILoggerDebug_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILoggerDebug_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ILoggerDebug_INTERFACE_DEFINED__ */



#ifndef __LOGGERLib_LIBRARY_DEFINED__
#define __LOGGERLib_LIBRARY_DEFINED__

/****************************************
 * Generated header for library: LOGGERLib
 * at Wed Sep 03 12:03:46 1997
 * using MIDL 3.01.75
 ****************************************/
/* [helpstring][version][uuid] */ 



EXTERN_C const IID LIBID_LOGGERLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_LoggerTrace;

class DECLSPEC_UUID("74A56661-1A59-11D1-96A6-00A0245836D0")
LoggerTrace;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_LoggerDebug;

class DECLSPEC_UUID("74A56663-1A59-11D1-96A6-00A0245836D0")
LoggerDebug;
#endif
#endif /* __LOGGERLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
