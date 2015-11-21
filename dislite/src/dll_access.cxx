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
#include "../include/stdafx.h"
#define	REG_INTEGRATOR "Integrator"

#include "../include/SmartPointer.h"
#include "../include/IDE_editorDLL.h"
#include "../include/DisRegistry.h"
#include "../include/dll_access.h"

///////////////////////////////////////////////////////////////////////////
enum IDEEDITORS
{
	IDEeditorNo,
	IDEeditorMSWORD,
	IDEeditorMSDEV2,
	IDEeditorMSDEV4,
	IDEeditorMSDEV5,
	IDEeditorMSDEV6
};

class IDE_editor {

private: HINSTANCE        m_hDll;
         LPCREATEFUN      m_pCreateEditorProc;
         LPEDITOR	      m_pDevStudio;
         LPEDITOR	      m_pMSWORD;
         LPEDITOR	      m_pDefaultEditor;
         enum IDEEDITORS  m_IDEEditor;
         enum IDEVERSION  m_verIDE;
         enum EDITORS     m_editorDefault;


public: IDE_editor() {
            m_IDEEditor = IDEeditorNo;
            m_verIDE	= NoIDE;
            m_hDll	= NULL;
            m_pCreateEditorProc = NULL;
            m_editorDefault	= editorMSDEV;
		}
        ~IDE_editor() {
            m_pDefaultEditor = NULL;
            m_pDevStudio = NULL;
            m_pMSWORD  = NULL;
            if ( m_hDll )
                FreeLibrary( m_hDll );
		}

        LPEDITOR   EditorGetDevStudio();
        LPEDITOR   EditorGetMSWORD();

        BOOL		    IsDevStudio();
        BOOL		    IsOLEeditor();
        LPEDITOR        DefaultEditor(){ return m_pDefaultEditor; }
        enum EDITORS    EditorName(){ return m_editorDefault; }
        enum IDEVERSION	IDEVersion();
        BOOL            LoadDll();
        EDITOR_RC       InitEditor();
        EDITOR_RC       ActivateEditor();
        EDITOR_RC       ActivateIntegration ();
        EDITOR_RC       MakeSelection( EDITPOSITION posBeg, EDITPOSITION posEnd );
        EDITOR_RC       OpenDocument( char* pszFileName );
        EDITOR_RC       ViewDocument( char* pszFileName );
        int             GetTabSize() { return m_pDefaultEditor->GetActiveDocument()->GetTabSize(); }
};

static IDE_editor IDE;

#define M6_DEVELOPERKEY   "Software\\Microsoft\\DevStudio\\6."
#define M5_DEVELOPERKEY   "Software\\Microsoft\\DevStudio\\5."
#define M4_DEVELOPERKEY   "Software\\Microsoft\\Developer"
#define M2_DEVELOPERKEY   "Software\\Microsoft\\Visual C++ 2."


///////////////////////////////////////////////////////////////////////////////////////////

enum IDEVERSION	IDE_editor::IDEVersion()
{
  char	szBuffer[256];
  if ( m_verIDE == NoIDE  && 
       GetDisRegistryStr( REG_INTEGRATOR, "DeveloperKey", szBuffer, sizeof(szBuffer), "", NULL) )
  {
    if ( strnicmp( szBuffer, M6_DEVELOPERKEY, strlen(M6_DEVELOPERKEY)-2 ) == 0 )
      {
	if ( strnicmp( szBuffer, M6_DEVELOPERKEY, strlen(M6_DEVELOPERKEY) ) == 0 ) 
	  m_verIDE = MSDEV6x;
	else
	  m_verIDE = MSDEV5x;

        GetDisRegistryStr( REG_INTEGRATOR, "EnableOLE", szBuffer, sizeof(szBuffer), "1", NULL);
        if ( *szBuffer == '0' )
          m_verIDE = MSDEV4x;
      }
    else if ( strnicmp( szBuffer, M4_DEVELOPERKEY, strlen(M4_DEVELOPERKEY)) == 0 )
      m_verIDE = MSDEV4x;
    else if ( strnicmp( szBuffer, M2_DEVELOPERKEY, strlen(M2_DEVELOPERKEY)) == 0 )
      m_verIDE = MSDEV2x;
  }
  return m_verIDE;
}

//------------------------------------------
//
BOOL IDE_editor::IsDevStudio()
{ 
  return (m_verIDE == MSDEV5x || m_verIDE == MSDEV6x ); 
}

//------------------------------------------
//
BOOL IDE_editor::IsOLEeditor()
{ 
  if ( m_verIDE == NoIDE )
    IDEVersion();
  
  if ( EditorName() == editorMSDEV )
    return IsDevStudio();
  
  return ( EditorName() == editorMSWORD );
}

//------------------------------------------
//
char* AllocMemory( int n )
{
  char* psz = NULL;

  if ( n > 0 )
    psz = new char [ n ];
  return psz;
}

void FreeMemory( char* psz )
{
  if ( psz )
    delete [] psz;
}

//------------------------------------------
//
BOOL IDE_editor::LoadDll()
{
  char  szDllPath[_MAX_PATH +1 ]= "";
  char  c;
  
  GetDisRegistryStr( REG_INTEGRATOR, "Location", szDllPath, sizeof(szDllPath), "", NULL);
  if ( szDllPath[0] == 0 )
  {
    strcpy( szDllPath, getenv("PSETHOME") );
    c = szDllPath[ strlen(szDllPath) -1 ];
    if ( c != '\\' && c != '/' )
      strcat( szDllPath, "\\" );
    if ( IDEVersion() == MSDEV6x )
      strcat( szDllPath, "VC6int" );
    else
      strcat( szDllPath, "VC5int" );
  }
  c = szDllPath[ strlen(szDllPath) -1 ];
  if ( c != '\\' && c != '/' )
      strcat( szDllPath, "\\" );
  strcat( szDllPath, "IDE_Editor.dll" );
  m_hDll = LoadLibrary(szDllPath);

  if(m_hDll==NULL) {
	  char name[1024];
      GetCurrentDirectory(1000,name);
      strcat( name, "\\IDE_Editor.dll" );
      m_hDll = LoadLibrary(name);
  }

  if(m_hDll==NULL) {
	  char name[1024];
      GetSystemDirectory(name,1000);
      strcat( name, "\\IDE_Editor.dll" );
      m_hDll = LoadLibrary(name);
  }
  if(m_hDll==NULL) {
	  char name[1024];
      GetWindowsDirectory(name,1000);
      strcat( name, "\\IDE_Editor.dll" );
      m_hDll = LoadLibrary(name);
  }


  LPINITIALIZEFUN pInitialize = NULL;

  if ( m_hDll )
  {
    pInitialize = (LPINITIALIZEFUN)GetProcAddress( m_hDll, "Initialize" );
    m_pCreateEditorProc = (LPCREATEFUN)GetProcAddress( m_hDll, "CreateEditor" );
    if ( pInitialize && m_pCreateEditorProc )
    {
      (*pInitialize)( AllocMemory, FreeMemory );
    }
    else
    {
      FreeLibrary( m_hDll );
      m_hDll = NULL;
    }
  }
  return (BOOL)m_hDll;
}

//------------------------------------------
//
LPEDITOR IDE_editor::EditorGetDevStudio() {

  if ( m_hDll == NULL ) {
    m_pDevStudio = NULL;
    if ( !LoadDll() ) return NULL;
  }

  if ( (CEditor*)m_pDevStudio== NULL  )
       (*m_pCreateEditorProc)((void**)&m_pDevStudio, editorMSDEV );

  if ( !m_pDevStudio->IsEnable() )
    m_pDevStudio->Start();
  
  return m_pDevStudio;
}

//------------------------------------------
//
LPEDITOR IDE_editor::EditorGetMSWORD() {

  if ( m_hDll == NULL )
  {
    m_pMSWORD = NULL;
    if ( !LoadDll() )
      return NULL;
  }

  if ( (CEditor*)m_pMSWORD == NULL )
    (*m_pCreateEditorProc)( (void**)&m_pMSWORD, editorMSWORD );

  if ( !m_pMSWORD->IsEnable() )
    m_pMSWORD->Start();

  return m_pMSWORD;
}

//------------------------------------------
//
EDITOR_RC	IDE_editor::ActivateEditor() {
  
  EDITOR_RC rc = EDITOR_W_NOTIMPLEMENTED;
  
  if ( IsOLEeditor() )
  {
    if ( m_pDefaultEditor == NULL )
       rc= EDITOR_W_NOTINITIALIZED;
    else
    {
     rc= m_pDefaultEditor->SetVisible();
     if ( rc == EDITOR_OK )
       rc= m_pDefaultEditor->Activate();
    }
  }
  return rc;
}

//------------------------------------------
//
EDITOR_RC  IDE_editor::InitEditor() {
  EDITOR_RC rc = EDITOR_W_WRONGPARAMETER;
  m_pDefaultEditor = NULL;
  m_IDEEditor = IDEeditorNo;

  //------------ MSWORD
  if ( m_editorDefault == editorMSWORD )
  {
    m_pDefaultEditor = EditorGetMSWORD();
    if ( m_pDefaultEditor && (rc = m_pDefaultEditor->GetLastError()) == EDITOR_OK )
      m_IDEEditor = IDEeditorMSWORD;
    else
      m_pDefaultEditor = NULL;
  }

  //------------ MSDEV
  else if ( m_editorDefault == editorMSDEV )
  {
    enum IDEVERSION ver = IDEVersion();
    if ( ver == MSDEV6x ) // IDE = DevStudio
    {
      m_pDefaultEditor = EditorGetDevStudio();
      if ( m_pDefaultEditor && (rc = m_pDefaultEditor->GetLastError()) == EDITOR_OK )
	m_IDEEditor = IDEeditorMSDEV6;
      else
	m_pDefaultEditor = NULL;
    }
    else if (ver == MSDEV5x ) // IDE = DevStudio
    {
      m_pDefaultEditor = EditorGetDevStudio();
      if ( m_pDefaultEditor && (rc = m_pDefaultEditor->GetLastError()) == EDITOR_OK )
	       m_IDEEditor = IDEeditorMSDEV5;
      else
	       m_pDefaultEditor = NULL;
    }
    else if (ver == MSDEV4x )	// IDE = MSDEV 4.x
    {
      m_IDEEditor = IDEeditorMSDEV4;
      rc = EDITOR_OK;
    }
    else if (ver == MSDEV2x )	// IDE = MSVC 2.x
    {
      m_IDEEditor = IDEeditorMSDEV2;
      rc = EDITOR_OK;
    }
    else	// No Integration
      rc = EDITOR_W_WRONGPARAMETER;
  }

  //------------ UNKNOWN PARAMETER enum EDITORS editor
  else
    rc = EDITOR_W_WRONGPARAMETER;

  return rc;
}

//------------------------------------------
//
EDITOR_RC IDE_editor::ActivateIntegration() {
	
  EDITOR_RC rc = EDITOR_W_NOTIMPLEMENTED;

  if ( IsDevStudio() )
  {
    if ( m_pDefaultEditor == NULL )
      return EDITOR_W_NOTINITIALIZED;
    else
    {
      rc= m_pDefaultEditor->SetVisible();
      if ( rc == EDITOR_OK )
      {
        rc= m_pDefaultEditor->Activate();
        if ( rc == EDITOR_OK )
          rc= m_pDefaultEditor->ExecCommand( "DisAddInCommand" );
      }
    }
  }

  return rc;
}

//------------------------------------------
//
EDITOR_RC	IDE_editor::OpenDocument( char* pszFileName ) {
  
  EDITOR_RC rc = EDITOR_W_NOTIMPLEMENTED;
  
  if ( IsOLEeditor() )
  {
    if ( m_pDefaultEditor == NULL )
      rc= EDITOR_W_NOTINITIALIZED;
    else
    {
      m_pDefaultEditor->OpenDocument(pszFileName);
      rc = m_pDefaultEditor->GetLastError();
    }
  }
  return rc;
}

//------------------------------------------
//
EDITOR_RC	IDE_editor::ViewDocument( char* pszFileName ) {
  
  EDITOR_RC rc = EDITOR_W_NOTIMPLEMENTED;
  
  if ( IsOLEeditor() )
  {
    if ( m_pDefaultEditor == NULL )
      rc= EDITOR_W_NOTINITIALIZED;
    else
    {
      m_pDefaultEditor->ViewDocument(pszFileName);
      rc = m_pDefaultEditor->GetLastError();
    }
  }
  return rc;
}


//------------------------------------------
//
EDITOR_RC IDE_editor::MakeSelection( EDITPOSITION posBeg, EDITPOSITION posEnd )
{
  
  EDITOR_RC rc = EDITOR_W_NOTIMPLEMENTED;

  if ( IsOLEeditor() )
  {
    if ( m_pDefaultEditor == NULL )
      rc= EDITOR_W_NOTINITIALIZED;
    else
      rc= m_pDefaultEditor->GetActiveDocument()->MakeSelection(posBeg, posEnd);
  }

  return rc;
}

///////////////////////////////////////////////////////////////////////////
//
//	Interface for pset_server
//
///////////////////////////////////////////////////////////////////////////

void dis_integration_send_command (char* command, char *fname, int line, int col=0, int tab=0, char* tok=NULL );

enum IDEVERSION	dis_integration_get_IDE()
{
  return IDE.IDEVersion();
}

//-------------------------------------------
void dis_integration_send_command (char* command, char *fname, int line, int col, int tab, char* tok )
{
  CString str;
  if ( line <= 0 ) line = 1;
  if ( tok == NULL )
    str.Format("dis_ide_send_command {%s 0 {\"%s\"} %d}", command, fname, line);
  else
  {
    str.Format("dis_ide_send_command {%s 0 {\"%s\"} %d %d %d {{%s}}}", command, fname, line, tab, col, tok);
  }
//  rcall_dis_DISui_eval_async (Application::findApplication("DISui"), (vchar*)(char*)str);
}

//-------------------------------------------
void dis_integration_open_file (char *fname, int line )
{
  if ( IDE.IsOLEeditor() )
  {
    if ( IDE.InitEditor() == EDITOR_OK && IDE.ActivateEditor() == EDITOR_OK )
    {
      IDE.ActivateIntegration();
      if ( IDE.OpenDocument(fname) == EDITOR_OK && line >0 )
        IDE.MakeSelection( EDITPOSITION(line,0), EDITPOSITION(line, -1) );
      
      if (IDE.IsDevStudio() )
      {
        IDE.ActivateEditor();
        IDE.ActivateIntegration();
        dis_integration_send_command( "file_opened", fname, line );
      }
    }
  }

  else if (IDE.IDEVersion() != NoIDE ) // IDE = MSDEV 4.x or MSVC 2.x
  {
    dis_integration_send_command( "dis_open_file", fname, line );
    dis_integration_send_command( "file_opened", fname, line );
  }
}

void dis_integration_view_file (char *fname, int line )
{
  if ( IDE.IsOLEeditor() )
  {
    if ( IDE.InitEditor() == EDITOR_OK && IDE.ActivateEditor() == EDITOR_OK )
    {
      IDE.ActivateIntegration();
      if ( IDE.ViewDocument(fname) == EDITOR_OK && line >0 )
        IDE.MakeSelection( EDITPOSITION(line,0), EDITPOSITION(line, -1) );
      
      if (IDE.IsDevStudio() )
      {
        IDE.ActivateEditor();
        IDE.ActivateIntegration();
        dis_integration_send_command( "file_opened", fname, line );
      }
    }
  }

  else if (IDE.IDEVersion() != NoIDE ) // IDE = MSDEV 4.x or MSVC 2.x
  {
    dis_integration_send_command( "dis_open_file", fname, line );
    dis_integration_send_command( "file_opened", fname, line );
  }
}

void dis_integration_activate()
{
  if ( IDE.IsOLEeditor() ) {
    if ( IDE.InitEditor() == EDITOR_OK && IDE.ActivateEditor() == EDITOR_OK) {
      IDE.ActivateIntegration();
    }
  }
}



//-------------------------------------------
void dis_integration_make_selection_internal( int line, int col, int len  )
{
  if ( IDE.IsOLEeditor() )
  {
    if ( IDE.InitEditor() == EDITOR_OK && IDE.ActivateEditor() == EDITOR_OK )
    {
      EDITPOSITION posBeg(line, col);
      EDITPOSITION posEnd(line, col + len );
      IDE.MakeSelection( posBeg, posEnd );
      
      if ( IDE.IsDevStudio() )
      {
        IDE.ActivateEditor();
        IDE.ActivateIntegration();
      }
    }
  }
}


void dis_integration_make_selection_internal2( int line, int col, int len  )
{
    if ( IDE.InitEditor() == EDITOR_OK && IDE.ActivateEditor() == EDITOR_OK) {
      EDITPOSITION posBeg(line, col);
      EDITPOSITION posEnd(line, col + len );
      IDE.MakeSelection( posBeg, posEnd );
	}
}




void dis_integration_make_selection ( char* fname, int line, int col, char* tok  )
{
  if ( IDE.IsOLEeditor() )
  {
    if ( IDE.InitEditor() == EDITOR_OK && IDE.ActivateEditor() == EDITOR_OK )
    {
      if ( fname && *fname )
        IDE.OpenDocument(fname);
      dis_integration_make_selection_internal( line, col, strlen(tok)  );
    }
  }

  else if (IDE.IDEVersion() != NoIDE ) // IDE = MSDEV 4.x or MSVC 2.x
    dis_integration_send_command( "dis_open_file", fname, line );

  if ( tok == NULL || *tok == 0 || (line== 1 && col <= 0) )
    dis_integration_send_command( "file_opened", fname, line );
  else
    dis_integration_send_command( "dis_ide_set_info", fname, line, col, -dis_integration_get_tab_size(), tok );
}

//-------------------------------------------
void dis_integration_load_file (char *fname)
{
  if ( IDE.IsOLEeditor() )
  {
    if ( IDE.InitEditor() == EDITOR_OK && IDE.ActivateEditor() == EDITOR_OK )
      IDE.OpenDocument(fname);
  }
}

//-------------------------------------------
int dis_integration_get_tab_size ()
{
  int tabs = 1;
  if ( IDE.IsDevStudio() )
  {
    if ( IDE.InitEditor() == EDITOR_OK )
      tabs = (IDE.DefaultEditor())->GetActiveDocument()->GetTabSize();
  }
  return tabs;
}
//-------------------------------------------
struct gSelection
{
  int   lineoffset;
  int   line;
  int   col;
  int   tab;
  char* token;
  char* fname;

 gSelection(){ lineoffset=0; line = 0; col  = 0; tab  = 1; token = 0; fname = 0; }
~gSelection(){ ClearSelection(); }

void ClearSelection()
{
  if ( token ) delete token;
  if ( fname ) delete fname;
  lineoffset=0; line = 0; col  = 0; tab  = 1; token = 0; fname = 0;
}

};

static gSelection gSel;

int dis_integration_is_changed(void) {
int ret = 0;

  if ( IDE.IsOLEeditor() )  {
    if ( IDE.InitEditor() == EDITOR_OK )  {
      LPEDITOR   pEditor = IDE.DefaultEditor();
      LPDOCUMENT pDoc = pEditor->GetActiveDocument();
      char *pszDocName = NULL;
      if ( pEditor->GetLastError() == EDITOR_OK && (pszDocName = pDoc->GetFullName()) && *pszDocName ) {
		    ret = pDoc->IsSaved();

      }
    }
  }
  return ret;
}


int dis_integration_read_selection (int get_lineoffset /* == 0 if does not needed */)
{
  int ret = 0;
  gSel.ClearSelection();

  if ( IDE.IsOLEeditor() )
  {
    if ( IDE.InitEditor() == EDITOR_OK )
    {
      LPEDITOR   pEditor = IDE.DefaultEditor();
      LPDOCUMENT pDoc = pEditor->GetActiveDocument();
      char *pszDocName = NULL;
      if ( pEditor->GetLastError() == EDITOR_OK && (pszDocName = pDoc->GetFullName()) && *pszDocName )
      {
        EDITSELECTION sel;
        if ( get_lineoffset == 0 )
          sel.nLineOffset = -1;
        pDoc->GetSelection( &sel );
        gSel.fname = new char [ strlen(pszDocName) +1 ];
        gSel.token = new char [ strlen(sel.pszText) +1 ];
        if ( gSel.fname && gSel.token )
        {
          if ( get_lineoffset )
             gSel.lineoffset = sel.nLineOffset;
          gSel.line = sel.cursor.line;
          gSel.col = sel.cursor.column;
          gSel.tab = sel.nTab;
          strcpy(gSel.token, sel.pszText);
          strcpy(gSel.fname, pszDocName );
          ret = 1;
        }
        else
          gSel.ClearSelection();
      }
    }
  }
  return ret;
}


//-------------------------------------------
int dis_integration_getsel_tabsize ()
{
  return gSel.tab;
}
int dis_integration_getsel_line ()
{
  return gSel.line;
}
int   dis_integration_getsel_lineoffset ()
{
  return gSel.lineoffset;
}
int dis_integration_getsel_col ()
{
  return gSel.col;
}
char* dis_integration_getsel_token ()
{
  return gSel.token;
}

/*
char* dis_integration_getsel_fname ()
{
  return gSel.fname;
}
*/

char* dis_integration_getsel_fname () {
  char *p = gSel.fname;
  while(*p) {
	if (*p=='\\') *p='/';
	p++;
  }
  return gSel.fname;
}
