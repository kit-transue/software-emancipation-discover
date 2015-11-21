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
#include "galgen.h"
#include <malloc.h>

class IF_SYM;

/* ======================================================== */
class IF_SMT
{
  IF_SMT*  m_pNext;

  int	   m_nLine;
  int	   m_nColumn;
  IF_SYM*  m_pSym;

public:
  IF_SMT() { m_pNext=0; m_pSym=0; m_nLine=0, m_nColumn=0; }
 ~IF_SMT() { if (m_pNext) delete m_pNext;};
 IF_SMT*  getNext() { return m_pNext; };

static IF_SMT* CreateNew(IF_SMT** pHeader, IF_SYM* pSym, int nLine=0, int nColumn=0);

  IF_SYM*   getSym   (){ return (this ? m_pSym : 0); }
  int	    getLine  (){ return (this ? m_nLine : 0); }
  int	    getColumn(){ return (this ? m_nColumn : 0); }
  int	    getLength();

  IF_SYM*   setSym   (IF_SYM* p) { return (m_pSym=p); }
  int	    setLine  (int n){ return (m_nLine=n); }
  int	    setColumn(int n){ return (m_nColumn=n); }
};

/* ======================================================== */
class IF_SYM
{
  IF_SYM* m_pNext;

  char*   m_pszName;
  int     m_nLength;
  int     m_nIndex;
  IF_SMT* m_pSmt;

public:
  IF_SYM() { m_pszName=0; m_nIndex=0; m_nLength=0; m_pNext=0; m_pSmt=0; } 
 ~IF_SYM() { if (m_pNext) delete m_pNext; if (m_pszName) delete [] m_pszName; }
  IF_SYM*  getNext() { return m_pNext; }

  static IF_SYM*  CreateNew(IF_SYM** pHeader, const char* psz, int nIndex );

  char*   getName()  { return (this ? m_pszName : 0); }
  int     getIndex() { return (this ? m_nIndex : 0); }
  int	  getLine  (){ return (this ? m_pSmt->getLine() : 0); }
  int	  getColumn(){ return (this ? m_pSmt->getColumn() : 0); }
  int     getLength(){ return (this ? m_nLength : 0); }
  IF_SMT* getListSmt(){ return m_pSmt; }

 IF_SMT* CreateNewSMT(int nLine=0, int nColumn=0)
 {
   return IF_SMT::CreateNew(&m_pSmt, this, nLine, nColumn );
 }
};

/* ======================================================== */
class IF_SYMTAB
{
  IF_SYM* m_listSym;
  int     m_nSyms;
  IF_SYM* m_symFinded;

public:
  IF_SYMTAB() { m_listSym = 0; m_nSyms= 0; m_symFinded = 0; }
 ~IF_SYMTAB() { if (m_listSym) delete m_listSym; }

  IF_SYM*  getListSym() { return m_listSym; }
  IF_SYM*  AddNewSym( const char* name );

  IF_SYM*  FindSymByName  ( const char* name );
  IF_SYM*  FindSymByIndex ( int n );
  char*    FindNameByIndex( int n );
  int      getFindedSymIndex(){ return m_symFinded->getIndex(); }

  static int   IsInternalType( const char* typenm );
  static char* getExternalType( const char* typenm, char* ext_typenm );

};


/* ======================================================== */
class IF_FUNCDEF
{
  TYPENAME     m_arg1;
  int          m_maxargs;

  struct tagFUNC_ARGS 
  {
    TYPENAME* ptypename;
    
    char* type_name;
    int   type_lin;
    int   type_col;
    int   type_indx;

    int   name_lin;
    int   name_col;
    int   name_indx;
  }
  m_funcArgs[MAX_ARGS_TYPENAME+1];

public:
  IF_FUNCDEF() { for (int i=0; i<MAX_ARGS_TYPENAME+1; i++) m_funcArgs[i].type_name =0; };
 ~IF_FUNCDEF() 
  { 
   for (int i=0; i<MAX_ARGS_TYPENAME+1; i++) 
   {
     if (m_funcArgs[i].type_name) delete [] m_funcArgs[i].type_name;
   }
  };

  void Init(TYPENAME* args);
  void SetTypeNames(int maxargs, char* arg1_type, char* arg1_name);
  void setNamePos(int iarg);
  void setTypePos(int iarg);

  char* getArgType      (int iarg );
  int   getArgTypeLine  (int iarg );
  int   getArgTypeColumn(int iarg );
  int   getArgTypeIndx  (int iarg );
  int   setArgTypeIndx  (int iarg, int indx );

  char* getArgName	(int iarg);
  int   getArgNameLine  (int iarg );
  int   getArgNameColumn(int iarg );
  int   getArgNameIndx  (int iarg );
  int   setArgNameIndx  (int iarg, int indx );

};

/* ======================================================== */

class IF_KEYWORDS
{
  char*		m_pszKeyWord;
  IF_KEYWORDS*	m_pNext;

public:
  IF_KEYWORDS() { m_pszKeyWord =0; m_pNext=0; }
 ~IF_KEYWORDS() 
 { 
   if ( m_pNext ) delete m_pNext;
   m_pNext =0;
   if ( m_pszKeyWord ) delete m_pszKeyWord;
   m_pszKeyWord =0; 
 }

 static void Add( const char* pszKey, IF_KEYWORDS** pHeader );
 int  IsKeyWord( const char* pszKey );
};

/* ======================================================== */

class IF_FILE
{
  FILE* m_file;
  int   m_flagOut;
  char* m_pszPrtBuffer;

public:

  IF_FILE();
 ~IF_FILE();

  int   open  ( const char* fname );
  void  close ();
  int   printf();
  void  printf(char* s) { if (m_file) fprintf(m_file,s); }

  void  printf(char* fmt, int n) { if (m_file) fprintf(m_file,fmt,n); }
  void  printf(char* fmt, int n, char* psz) { if (m_file) fprintf(m_file,fmt,n, psz); }
  void  printf(char* fmt, int n, int m) { if (m_file) fprintf(m_file,fmt,n,m); }
  void  printf(char* fmt, int n, int m, int k) { if (m_file) fprintf(m_file,fmt,n,m,k); }
  void  printf(char* fmt, int n, int m, int k, int l) { if (m_file) fprintf(m_file,fmt,n,m,k,l); }

  void  printf(char* fmt, char* s) { if (m_file) fprintf(m_file,fmt,s); }
  void  printf(char* fmt, char* s, int n) { if (m_file) fprintf(m_file,fmt,s, n); }
  void  printf(char* fmt, char* s, int n, int m) { if (m_file) fprintf(m_file,fmt,s, n,m); }
  void  printf(char* fmt, char* s, int n, int m, int k) { if (m_file) fprintf(m_file,fmt,s, n,m,k); }

  void  ResetFlagOut() { m_flagOut=0; }
  int   getFlagOut() { return m_flagOut; }

  char* getBuf() { return m_pszPrtBuffer; }

};
/* ======================================================== */

class IF
{
  IF_FILE*	m_pIFFile;
  IF_SYMTAB*	m_pSymTab;
  IF_FUNCDEF*	m_pfuncDef;
  IF_KEYWORDS*	m_pKeyWords;

  char*		m_pszInterfaceName;
  int		m_nInterfaceIndx;
  int	        m_service_col; 
  int	        m_service_lin; 

public:

  void Init( const char* pszFileIn, const char* pszFileIF );
  void Destroy();

  IF() { m_pIFFile=0; m_pSymTab=0; m_pfuncDef=0; m_pKeyWords=0;
	 m_pszInterfaceName=0; m_nInterfaceIndx=0; m_service_col=0; m_service_lin=0; }
 ~IF() { Destroy(); }

  IF_FILE*    getIFFile()  { return m_pIFFile; }
  IF_FUNCDEF* getFuncDef() { return m_pfuncDef; };
  IF_SYMTAB*  getSymTab()  { return m_pSymTab; }

  int	getInterfaceIndx() { return m_nInterfaceIndx; }
  char*	getInterfaceName() { return m_pszInterfaceName; }
  void	setInterfaceName(const char* p);

  void	AddKeyWord( const char* pszKey ) { IF_KEYWORDS::Add(pszKey, &m_pKeyWords); }
  int	IsKeyWord ( const char* pszKey ) { return m_pKeyWords->IsKeyWord(pszKey); }
  void	ServiceKeyword();

  int  FindSymIndexOrAdd( const char* name, int nLine, int nColumn, int bPrintFlag=1 );

  int getArgTypeIndex(int iarg);
  int getArgNameIndex(int iarg);

  void PrintFunctDef(const char* prefix);
  void PrintSymSmt();
};
