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
///////////////////////////////////////////////////////////////////////////
//
//	Logger
//
///////////////////////////////////////////////////////////////////////////
#ifndef __TRACE_H
#define __TRACE_H

static char* __ClassName () { return ""; }
static void* __ThisObject() { return NULL; }

///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef  __NOTRACE
#define __TRACEBODY    {}
#else   
#define __TRACEBODY
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

class __CLoggerTrace
{
	int		m_bEnable;
	int		m_cRef;

public:
  __CLoggerTrace(int iLine,char* pszFName,char* pszMethod,char* pszClass, void* Object) __TRACEBODY;
 ~__CLoggerTrace() __TRACEBODY;

    void TraceString    (int,char*,char*    strVal=NULL) __TRACEBODY;
    void TraceString    (int,char*,wchar_t* strVal=NULL) __TRACEBODY;
    void TraceInt       (int,char*,int) __TRACEBODY;
};

#define TRACE_ENTRY(X)    __CLoggerTrace __Trace(__LINE__,__FILE__,#X,__ClassName(),__ThisObject())

#define TRACE_STRING(X)   __Trace.TraceString(__LINE__,#X,(char*)X);
#define TRACE_INT(X)      __Trace.TraceInt(__LINE__,#X,(int)X);

#define TRACE_CLASS(X)     char* __ClassName(){return #X;} void* __ThisObject() {return (void*)this;}

#endif
