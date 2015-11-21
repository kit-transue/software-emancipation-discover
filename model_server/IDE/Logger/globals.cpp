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
// Globals.cpp

#include "stdafx.h"
#include "Logger.h"
#include "Globals.h"
//------------------------------------------
//
BSTR StrToBSTR(char* psz)
{
  if ( psz == NULL ) 
    return NULL;

  BSTR			bstr = NULL;
  int       nLen = strlen( psz ) +1;
  wchar_t*  wsz = new wchar_t [ nLen * sizeof(wchar_t) ];
  if ( wsz == NULL )
    return NULL;

  mbstowcs( wsz, psz, nLen );
  bstr = ::SysAllocString( wsz );
  delete [] wsz;
	return bstr;
}

//------------------------------------------
//
char* BSTRToStr(BSTR bstr)
{
  if ( bstr == NULL ) 
    return NULL;

  wchar_t*  wsz = bstr;
  int       nLen = wcslen( wsz ) +1;
  char*     psz = new char [ nLen ];
  if ( psz == NULL )
    return NULL;
  
  wcstombs(psz, wsz, nLen);
	return psz;
}

///////////////////////////////////////////////////////////////////////////
//
//	CVariant
//
///////////////////////////////////////////////////////////////////////////
CVariant::CVariant( int n )
{
	  ::VariantInit(this);
	  this->vt = VT_I4;
	  this->intVal = n;
};

CVariant::CVariant( bool b)
{
	  ::VariantInit(this);
	  this->vt = VT_BOOL;
	  this->boolVal = b;
};

CVariant::CVariant( VARIANT_BOOL b)
{
	  ::VariantInit(this);
	  this->vt = VT_BOOL;
	  this->boolVal = b;
};

CVariant::CVariant( BSTR bstr)
{
	  ::VariantInit(this);
	  this->vt = VT_BSTR;
	  this->bstrVal = bstr;
};

CVariant::CVariant( char* psz )
{
  ::VariantInit(this);
  this->vt = VT_BSTR;
  this->bstrVal = ::StrToBSTR( psz );
}

