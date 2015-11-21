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
//	IDE_editorCOM.h
//
///////////////////////////////////////////////////////////////////////////
#ifndef _IDEEDITORCOM_H
#define _IDEEDITORCOM_H

#pragma warning( disable: 4270 )

typedef unsigned __int8     boolval;
#define false               '\0'
#define true                '\1'

//////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
  EDITOR_OK,
	
  EDITOR_W_FILENOTFOUND,
  EDITOR_W_BADPATH,
  EDITOR_W_INVALIDFILE,
  EDITOR_W_ACCESSDENIED,
  EDITOR_W_SHARINGVIOLATION,

  EDITOR_W_NOTTEXTDOCUMENT,

  EDITOR_W_NOTSTARTED,
  EDITOR_W_NOTINITIALIZED,
  EDITOR_W_NOTIMPLEMENTED,
  EDITOR_W_INVALIDCOMMAND,

  EDITOR_W_WRONGPARAMETER,
  EDITOR_W_WRONGMEMORY,
  EDITOR_E_OUTOFMEMORY,

  EDITOR_E_SYSTEMERROR,
  EDITOR_E_UNKNOWNERROR,

  EDITOR_E_NOOBJECT,
  EDITOR_E_OBJECTEXIST,
} 
  EDITOR_RC;


enum EDITORS
{
	editorMSDEV,
	editorMSWORD
};


enum editSaveChanges
{
  editSaveChangesNo     = 0,
  editSaveChangesYes    = 1, 
  editSaveChangesPrompt = 2
};

struct tagEditPosition
{
  union 
   {
    long  offset;
    struct
     {
      short line;
      short column;
     };
   };

#ifdef __cplusplus

  tagEditPosition(short nLine, short nCol)
  { 
   line= nLine; 
   column = nCol; 
  }
  tagEditPosition(long Offset=0)
  { 
   offset= Offset;
  }

	operator long() { return offset; }
	operator int () { return offset; }

#endif

};
typedef struct tagEditPosition EDITPOSITION;


struct tagEditSelection
{
  EDITPOSITION	cursor;
  int           nLineOffset;
  short         nTab;
  char*         pszText;

#ifdef __cplusplus

  tagEditSelection()
  { 
   nLineOffset = 0;
   pszText = NULL;
  }
 ~tagEditSelection()
  { 
   if ( pszText )
     delete [] pszText;
  }

#endif
};
typedef struct tagEditSelection EDITSELECTION;


struct tagEditDocumentStatus
{
  char*          pszFileName;
  boolval        bOpened;
  boolval        bSaved;
  boolval        bActive;
  boolval        bReadOnly;
  boolval        bTextMode;
  int            nTabSize;
  EDITSELECTION  sel;

#ifdef __cplusplus

  tagEditDocumentStatus()
  {
   pszFileName = NULL;
   bOpened     = false;
   bSaved      = false;
   bActive     = false;
   bReadOnly   = true;
   bTextMode   = false;
   nTabSize    = 0;
  }

 ~tagEditDocumentStatus()
  {
   if ( pszFileName )
      delete [] pszFileName;
  }

#endif
};
typedef struct tagEditDocumentStatus EDITDOCSTATUS;


///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

class CEditor;
class CEditDocument;

typedef CEditor     				EDITOR;
typedef IPtr<CEditor>				LPEDITOR;
typedef IPtr<CEditDocument> LPDOCUMENT;

typedef void CIndex;

#endif

///////////////////////////////////////////////////////////////////////////

typedef char* ( *LPCHARALLOC )( int   );
typedef void  ( *LPCHARFREE  )( char* );

typedef void  ( *LPINITIALIZEFUN )( LPCHARALLOC, LPCHARFREE );

typedef void  ( *LPCREATEFUN )( void**, enum EDITORS );


extern "C" void __declspec(dllexport) CreateEditor( void** ppInstance, enum EDITORS editor );
extern "C" void __declspec(dllexport) InitAllocation( LPCHARALLOC pAllocFun, LPCHARFREE pFreeFun );

///////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

class CEditor
{
public:
virtual	int          AddRef() = 0;
virtual	int          Release() = 0;

virtual	boolval      IsEnable         ()= 0;
virtual	EDITOR_RC    GetLastError     (char* pszErrText = NULL)= 0;
virtual	EDITOR_RC    Start            ()= 0;
virtual	EDITOR_RC    Close            ()= 0;
virtual	EDITOR_RC    SetVisible	      (boolval b = true)= 0;
virtual	boolval      GetVisible       ()= 0;
virtual	EDITOR_RC    Activate         ()= 0;
virtual	EDITOR_RC    ExecCommand      (char* pszCommandName = NULL)= 0;

virtual boolval      IsEmpty       ()= 0;
virtual int          GetIndexCount ()= 0;
virtual CIndex*      GetIndexFirst ()= 0;
virtual CIndex*      GetIndexLast  ()= 0;
virtual CIndex*      GetIndexNext  (CIndex* pIndex = NULL )= 0;
virtual CIndex*      GetIndexPrev  (CIndex* pIndex = NULL )= 0;

virtual LPDOCUMENT   GetDocument      (CIndex* pIndex = NULL )= 0;
virtual	LPDOCUMENT   GetActiveDocument()= 0;
virtual	LPDOCUMENT   OpenDocument     (char* pszFileName = NULL )= 0;

};


///////////////////////////////////////////////////////////////////////////
class CEditDocument
{
public:

virtual	int          AddRef() = 0;
virtual	int          Release() = 0;

virtual	boolval         IsOpened   ()= 0;
virtual	boolval         IsActive   ()= 0;
virtual boolval         IsReadOnly ()= 0;
virtual boolval         IsSaved    ()= 0;

virtual	EDITOR_RC       GetLastError(char* pszErrText = NULL)= 0;
virtual char*           GetFullName ()= 0;
virtual EDITOR_RC       Close       (enum editSaveChanges = editSaveChangesNo)= 0;
virtual EDITOR_RC       Save        (boolval	bSavePrompt  = false )= 0;
virtual EDITOR_RC       SaveAs      (char*	pszFileName = NULL, boolval bSavePrompt = false )= 0;

virtual	EDITOR_RC       SetReadOnly       (boolval nReadOnly = true)= 0;
virtual EDITPOSITION    GetCursorPosition ()= 0;
virtual EDITOR_RC       MoveCursorPosition( EDITPOSITION pos=(1,0), boolval bSelect = false )= 0;
virtual EDITOR_RC       GetSelection      ( EDITSELECTION* pSel= NULL )= 0;
virtual EDITOR_RC       MakeSelection     ( EDITPOSITION posBeg=(1,0), EDITPOSITION posEnd=(-1,-1) )= 0;
virtual int             GetTabSize        ()= 0;
virtual EDITOR_RC       GetStatus         ( EDITDOCSTATUS* pStatus = NULL ) = 0;
virtual EDITOR_RC       InsertText        ( char* pszText = NULL ) = 0;
virtual LPEDITOR        GetEditor         ()= 0;

};

#endif	//#ifdef  __cplusplus
#endif  //#ifndef _CEDITOR_H
