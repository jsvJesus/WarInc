

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Thu Jul 10 17:43:02 2014
 */
/* Compiler settings for .\Async.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __Async_h_h__
#define __Async_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IWarIncLaunch_FWD_DEFINED__
#define __IWarIncLaunch_FWD_DEFINED__
typedef interface IWarIncLaunch IWarIncLaunch;
#endif 	/* __IWarIncLaunch_FWD_DEFINED__ */


#ifndef __CWarIncLaunch_FWD_DEFINED__
#define __CWarIncLaunch_FWD_DEFINED__

#ifdef __cplusplus
typedef class CWarIncLaunch CWarIncLaunch;
#else
typedef struct CWarIncLaunch CWarIncLaunch;
#endif /* __cplusplus */

#endif 	/* __CWarIncLaunch_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IWarIncLaunch_INTERFACE_DEFINED__
#define __IWarIncLaunch_INTERFACE_DEFINED__

/* interface IWarIncLaunch */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IWarIncLaunch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("665F78E1-81B2-46F9-B9B3-6AE3B89F0B0E")
    IWarIncLaunch : public IDispatch
    {
    public:
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_cmdLine( 
            /* [in] */ BSTR cmdLine) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_cmdLine( 
            /* [retval][out] */ BSTR *cmdLine) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWarIncLaunchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWarIncLaunch * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWarIncLaunch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWarIncLaunch * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWarIncLaunch * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWarIncLaunch * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWarIncLaunch * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWarIncLaunch * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_cmdLine )( 
            IWarIncLaunch * This,
            /* [in] */ BSTR cmdLine);
        
        /* [id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_cmdLine )( 
            IWarIncLaunch * This,
            /* [retval][out] */ BSTR *cmdLine);
        
        END_INTERFACE
    } IWarIncLaunchVtbl;

    interface IWarIncLaunch
    {
        CONST_VTBL struct IWarIncLaunchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWarIncLaunch_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IWarIncLaunch_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IWarIncLaunch_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IWarIncLaunch_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IWarIncLaunch_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IWarIncLaunch_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IWarIncLaunch_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IWarIncLaunch_put_cmdLine(This,cmdLine)	\
    ( (This)->lpVtbl -> put_cmdLine(This,cmdLine) ) 

#define IWarIncLaunch_get_cmdLine(This,cmdLine)	\
    ( (This)->lpVtbl -> get_cmdLine(This,cmdLine) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IWarIncLaunch_INTERFACE_DEFINED__ */



#ifndef __WarIncLib_LIBRARY_DEFINED__
#define __WarIncLib_LIBRARY_DEFINED__

/* library WarIncLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_WarIncLib;

EXTERN_C const CLSID CLSID_CWarIncLaunch;

#ifdef __cplusplus

class DECLSPEC_UUID("64F2ECD6-505E-4A9A-9606-4727B70DF3E9")
CWarIncLaunch;
#endif
#endif /* __WarIncLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


