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
#include "galgen_if.h"
#include <stdlib.h>

/* ======================================================== */
/* ======================================================== */

IF* pIF=0;

/* ======================================================== */
/* ======================================================== */
extern "C" void IF_init_all(int iin, int iif, char* argv[])
{
  if ( iif>1 && iin > 1 )
  {
    if ( pIF )
      delete pIF;
    pIF = new IF;
    if ( pIF )
      pIF->Init( argv[iin], argv[iif] );
  }
}

/* -------------------------------- */
void IF_service_keyword()
{
  if ( pIF ) pIF->ServiceKeyword();
}

/* -------------------------------- */
extern "C" void IF_set_service_name(char* servname)
{
  if ( pIF ) pIF->setInterfaceName((const char*)servname);
}

/* -------------------------------- */
extern "C" void IF_destroy_all()
{
  delete pIF;
  pIF = 0;
}

/* -------------------------------- */
extern "C" void IF_eof()
{
  if ( pIF ) pIF->getIFFile()->ResetFlagOut();
}

/* -------------------------------- */
extern "C" void IF_init_args(TYPENAME* args)
{
  if ( pIF ) pIF->getFuncDef()->Init(args);
}

/* -------------------------------- */
extern "C" void IF_set_type_pos(int iarg)
{
  if ( pIF ) pIF->getFuncDef()->setTypePos(iarg);
}

extern "C" void IF_set_name_pos(int iarg)
{
  if ( pIF ) pIF->getFuncDef()->setNamePos(iarg);
}

/* -------------------------------- */
extern "C" void IF_print_func_def(int maxargs, char* preffix, char* arg1_type, char* arg1_name)
{
  if ( !pIF || !pIF->getIFFile()->getFlagOut() )
    return;

  pIF->getFuncDef()->SetTypeNames(maxargs, arg1_type, arg1_name);
  pIF->PrintFunctDef((const char*)preffix);
}


/* ======================================================== */
/* ======================================================== */

inline int IF_SMT::getLength() { return (this ? m_pSym->getLength() : 0); }

//--------------------
IF_SMT* IF_SMT::CreateNew(IF_SMT** pHeader, IF_SYM* pSym, int nLine, int nColumn)
{
  if ( pHeader == 0 || nLine == 0 || nColumn == 0 )
    return 0;

  IF_SMT* pSmt= new IF_SMT;
  if ( pSmt )
  {
    pSmt->m_pSym   = pSym;
    pSmt->m_nLine  = nLine;
    pSmt->m_nColumn= nColumn;
    pSmt->m_pNext = *pHeader;
    *pHeader = pSmt;
  }
  return pSmt;
}

/* ======================================================== */
IF_SYM* IF_SYM::CreateNew(IF_SYM** pHeader, const char* psz, int nIndex)
{
  if ( pHeader == 0 )
    return 0;

  IF_SYM* pSym = new IF_SYM;
  if ( pSym )
  {
    pSym->m_nIndex  = nIndex;
    if ( psz && *psz )
      pSym->m_pszName = new char [ strlen(psz) +1];
    if ( pSym->m_pszName )
    {
      strcpy( pSym->m_pszName, psz );
      pSym->m_nLength = strlen( pSym->m_pszName);
    }
    pSym->m_pNext = *pHeader;
    *pHeader = pSym;
  }
  return pSym;
}

/* ======================================================== */
IF_SYM* IF_SYMTAB::AddNewSym( const char* name )
{
  m_symFinded = IF_SYM::CreateNew(&m_listSym, name, ++m_nSyms);
  if ( !m_symFinded )
    m_nSyms--;
  return m_symFinded;
}

//-----------------------
IF_SYM* IF_SYMTAB::FindSymByName ( const char* name )
{
  m_symFinded= m_listSym;
  if ( name && *name && m_listSym )
  {
    while ( m_symFinded )
    {
      if ( strcmp(m_symFinded->getName(), name) == 0 )
	break;
      m_symFinded= m_symFinded->getNext();
    }
  }
  return m_symFinded;
}

//-----------------------
IF_SYM* IF_SYMTAB::FindSymByIndex ( int n )
{
  m_symFinded= m_listSym;
  if ( m_listSym && n > 0 && n <= m_nSyms )
  {
    while ( m_symFinded && m_symFinded->getIndex() != n )
      m_symFinded= m_symFinded->getNext();
  }
  return m_symFinded;
}


//-----------------------
char* IF_SYMTAB::FindNameByIndex( int n )
{
  FindSymByIndex(n);
  if ( m_symFinded )
    return m_symFinded->getName();
  else
    return 0;
}


/* -------------------------------- */
int IF_SYMTAB::IsInternalType( const char* typenm )
{
  int ret = 0;
  int len = strlen(typenm);
  if ( len >0 && strncmp(typenm,PTR_TYPE_PREFIX,PTR_TYPE_PREFIX_LEN)==0 &&
       strcmp( typenm + len - PTR_TYPE_SUFFIX_LEN, PTR_TYPE_SUFFIX) == 0 )
    ret =1;
  return ret;
}

/* -------------------------------- */
char* IF_SYMTAB::getExternalType( const char* typenm, char* ext_typenm )
{
  if ( IsInternalType(typenm) )
  {
    strcpy( ext_typenm, typenm + PTR_TYPE_PREFIX_LEN );
    ext_typenm[ strlen(ext_typenm) - PTR_TYPE_SUFFIX_LEN] = '*';
    ext_typenm[ strlen(ext_typenm) - PTR_TYPE_SUFFIX_LEN +1] = 0;
  }
  else
    strcpy( ext_typenm, typenm );
  return ext_typenm;
}

/* ======================================================== */
void IF_FUNCDEF::Init(TYPENAME* args)
{
  m_funcArgs[0].type_lin  = 0;
  m_funcArgs[0].type_col  = 0;
  m_funcArgs[0].type_indx = 0;
  m_funcArgs[0].name_lin  = 0;
  m_funcArgs[0].name_col  = 0;
  m_funcArgs[0].name_indx = 0;
  m_funcArgs[0].ptypename = args++;
  if ( m_funcArgs[0].type_name )
  {
    delete [] m_funcArgs[0].type_name;
    m_funcArgs[0].type_name= 0;
  }

  m_funcArgs[1].type_lin  = 0;
  m_funcArgs[1].type_col  = 0;
  m_funcArgs[1].type_indx = 0;
  m_funcArgs[1].name_lin  = 0;
  m_funcArgs[1].name_col  = 0;
  m_funcArgs[1].name_indx = 0;
  m_funcArgs[1].ptypename = &m_arg1;
  if ( m_funcArgs[1].type_name )
  {
    delete [] m_funcArgs[1].type_name;
    m_funcArgs[1].type_name= 0;
  }
  m_arg1.ptr = 0;
  m_arg1.ref = 0;
  m_arg1.isstatic = 0;
  m_arg1.isconst = 0;
  m_arg1.output = 0;
  m_arg1.unsign = 0;
  m_arg1.typname = 0;
  m_arg1.name = 0;

  for (int i=2; i<MAX_ARGS_TYPENAME+1; i++) {
      m_funcArgs[i].type_lin  = 0;
      m_funcArgs[i].type_col  = 0;
      m_funcArgs[i].type_indx = 0;
      m_funcArgs[i].name_lin  = 0;
      m_funcArgs[i].name_col  = 0;
      m_funcArgs[i].ptypename = args++;
      m_funcArgs[i].name_indx = 0;
      if ( m_funcArgs[i].type_name )
      {
	delete [] m_funcArgs[i].type_name;
	m_funcArgs[i].type_name= 0;
      }
  }
  m_maxargs = MAX_ARGS_TYPENAME;
}

/* -------------------------------- */
void IF_FUNCDEF::SetTypeNames(int maxargs, char* arg1_type, char* arg1_name)
{
  m_maxargs= maxargs+1;
  m_arg1.typname = arg1_type;
  m_arg1.name = arg1_name;

  char szTypeName[256];
  TYPENAME* thing;
  for ( int iarg=0; iarg< m_maxargs; iarg++ )
  {
    if ( m_funcArgs[iarg].type_name )
    {
      delete [] m_funcArgs[iarg].type_name;
      m_funcArgs[iarg].type_name= 0;
    }

    if ( (thing= m_funcArgs[iarg].ptypename) && thing->typname )
    {
      IF_SYMTAB::getExternalType( (const char*)(thing->typname), szTypeName );
      
      for (int iptr=0; iptr < thing->ptr; iptr++)
	strcat( szTypeName, "*" );
      for (int iref=0; iref <thing->ref; iref++)
	strcat( szTypeName, "&" );

      m_funcArgs[iarg].type_name = new char [ strlen(szTypeName) +1 ];
      if ( m_funcArgs[iarg].type_name )
	strcpy( m_funcArgs[iarg].type_name, szTypeName );
    }
  }
}

/* -------------------------------- */
void IF_FUNCDEF::setTypePos(int iarg)
{
  if ( iarg > 0 )
    iarg++;

  m_funcArgs[iarg].type_col = get_prev_col(); 
  m_funcArgs[iarg].type_lin = get_line(); 
}

void IF_FUNCDEF::setNamePos(int iarg)
{
  if ( iarg > 0 )
    iarg++;

  m_funcArgs[iarg].name_col = get_prev_col(); 
  m_funcArgs[iarg].name_lin = get_line(); 
}

/* -------------------------------- */
char* IF_FUNCDEF::getArgType(int iarg )
{ 
  if ( iarg < m_maxargs )
    return m_funcArgs[iarg].type_name; 
  else
    return 0;
}

int IF_FUNCDEF::getArgTypeLine(int iarg )
{ 
  if ( iarg < m_maxargs )
    return m_funcArgs[iarg].type_lin; 
  else
    return 0;
}

int IF_FUNCDEF::getArgTypeColumn(int iarg )
{ 
  if ( iarg < m_maxargs )
    return m_funcArgs[iarg].type_col; 
  else
    return 0;
}

int IF_FUNCDEF::getArgTypeIndx(int iarg )
{ 
  if ( iarg < m_maxargs )
    return m_funcArgs[iarg].type_indx; 
  else
    return 0;
}

int IF_FUNCDEF::setArgTypeIndx(int iarg, int indx )
{ 
  if ( iarg < m_maxargs )
    return (m_funcArgs[iarg].type_indx=indx); 
  else
    return 0;
}


/* -------------------------------- */
char* IF_FUNCDEF::getArgName(int iarg)
{ 
  if ( iarg < m_maxargs && m_funcArgs[iarg].ptypename )
    return (m_funcArgs[iarg].ptypename)->name; 
  else
    return 0;
}

int IF_FUNCDEF::getArgNameLine(int iarg )
{ 
  if ( iarg < m_maxargs )
    return m_funcArgs[iarg].name_lin; 
  else
    return 0;
}

int IF_FUNCDEF::getArgNameColumn(int iarg )
{ 
  if ( iarg < m_maxargs )
    return m_funcArgs[iarg].name_col; 
  else
    return 0;
}

int IF_FUNCDEF::getArgNameIndx(int iarg )
{ 
  if ( iarg < m_maxargs )
    return m_funcArgs[iarg].name_indx; 
  else
    return 0;
}

int IF_FUNCDEF::setArgNameIndx(int iarg, int indx )
{ 
  if ( iarg < m_maxargs )
    return (m_funcArgs[iarg].name_indx=indx); 
  else
    return 0;
}


/* ======================================================== */
IF_FILE::IF_FILE() 
{ 
  m_file= 0; 
  m_flagOut = 0; 
  m_pszPrtBuffer = new char [1024];
}

IF_FILE::~IF_FILE()
{ 
  if ( m_file ) 
    fclose(m_file);
  m_file = 0;

  if ( m_pszPrtBuffer )
    delete [] m_pszPrtBuffer;
}

//-----------------------
int IF_FILE::open( const char* fname )
{
  if ( fname && *fname )
    m_file = fopen( fname, "w" );
  if ( m_file )
    m_flagOut = 1;

  return (int)m_file;
}

//-----------------------
void IF_FILE::close()
{
  if ( m_file )
    fclose(m_file);
  m_file = 0;
}

//-----------------------
int IF_FILE::printf()
{
  return fprintf(m_file, "%s", m_pszPrtBuffer );
}



/* ======================================================== */
void IF_KEYWORDS::Add( const char* pszKey, IF_KEYWORDS** pHeader )
{
 IF_KEYWORDS* pKeyWord =0;
 if ( pszKey && *pszKey && pHeader )
 {
   pKeyWord= new IF_KEYWORDS;
   if ( pKeyWord )
   {
     pKeyWord->m_pszKeyWord = new char [ strlen(pszKey)+1];
     if ( pKeyWord->m_pszKeyWord )
       strcpy( pKeyWord->m_pszKeyWord, pszKey );
     else
     {
       delete pKeyWord;
       pKeyWord = 0;
     }
   }
 }

 if ( pKeyWord )
 {
   pKeyWord->m_pNext = *pHeader;
   *pHeader = pKeyWord;
 }
}

/* -------------------------------- */
int IF_KEYWORDS::IsKeyWord( const char* pszKey )
{
  int nRet=0;
  IF_KEYWORDS*	pKeyWord= this;
  while ( pKeyWord && pKeyWord->m_pszKeyWord )
  {
    if ( strcmp(pKeyWord->m_pszKeyWord, pszKey) == 0 )
    {
      nRet= 1;
      break;
    }
    pKeyWord= pKeyWord->m_pNext;
  }
  return nRet;
}

/* ======================================================== */
void IF::Destroy()
{
  PrintSymSmt();

  if ( m_pIFFile ) 
    delete m_pIFFile;
  m_pIFFile = 0;
  
  if ( m_pSymTab )
    delete m_pSymTab;
  m_pSymTab = 0;
  
  if ( m_pfuncDef)
    delete m_pfuncDef;
  m_pfuncDef =0;

  if ( m_pKeyWords )
    delete m_pKeyWords;
  m_pKeyWords = 0;

  if ( m_pszInterfaceName )
    delete [] m_pszInterfaceName;
  m_pszInterfaceName = 0;
}

/* -------------------------------- */
void IF::Init( const char* pszFileIn, const char* pszFileIF )
{
  if ( pszFileIn && *pszFileIn && pszFileIF && *pszFileIF )
  {
    m_pIFFile = new IF_FILE;
    m_pSymTab = new IF_SYMTAB;
    m_pfuncDef= new IF_FUNCDEF;

    if ( m_pIFFile && m_pSymTab && m_pfuncDef && getIFFile()->open(pszFileIF) )
    {
      getIFFile()->printf("SMT language \"c\"\n");
  
      char* szRealPath[1000];
#ifndef _WIN32
      ::realpath( (char*)pszFileIn, szRealPath);
#else
      char *p;
      p = _fullpath((char*)szRealPath, pszFileIn, sizeof(szRealPath) );
      if ( p == 0 )
	strcpy( (char*)szRealPath, pszFileIn);
      while ( (p=strchr( (char*)szRealPath,'\\')) ) *p= '/';
#endif

      IF_SYM* pSym= getSymTab()->AddNewSym((char*)szRealPath);
      getIFFile()->printf("SMT file \"%s\"\n", pSym->getName() );
      getIFFile()->printf("SYM [1] file \"%s\"\n", pSym->getName() );

      AddKeyWord("char");
      AddKeyWord("int");
      AddKeyWord("short");
      AddKeyWord("long");
      AddKeyWord("double");
      AddKeyWord("float");
      AddKeyWord("void");
    }
    else
      Destroy();
  }
}

/* -------------------------------- */
void IF::ServiceKeyword()
{
  int col = get_prev_col(); 
  int lin = get_line(); 
  getIFFile()->printf("SMT keyword \"service\" %d/%d %d\n", col, lin, strlen("service") ); 
}

/* -------------------------------- */
void IF::setInterfaceName(const char* p)
{
  if ( m_pszInterfaceName )
    delete [] m_pszInterfaceName;
  m_pszInterfaceName =0;
  if ( p && *p )
  {
    m_pszInterfaceName = new char [ strlen(p) +1 ];
    if ( m_pszInterfaceName )
      strcpy( m_pszInterfaceName, p);

    m_service_col = get_prev_col(); 
    m_service_lin = get_line(); 

    IF_SYM* pSym= getSymTab()->AddNewSym((char*)m_pszInterfaceName);
    m_nInterfaceIndx = pSym->getIndex();
    sprintf(getIFFile()->getBuf(), "SYM [%d] interface \"%s\" %d/%d %d\n", 
	    m_nInterfaceIndx, pSym->getName(), m_service_lin, m_service_col, strlen(m_pszInterfaceName)  );
    getIFFile()->printf();
    getIFFile()->printf("SMT [%d] %d/%d %d\n", 
	    pSym->getIndex(), m_service_lin, m_service_col, strlen(m_pszInterfaceName));
  }
}

/* -------------------------------- */
int IF::getArgTypeIndex(int iarg )
{
  IF_FUNCDEF*  pfuncDef = getFuncDef();
  int nIndex =pfuncDef->getArgTypeIndx(iarg);
  if ( nIndex == 0 )
  {
    char* pszTypeName = pfuncDef->getArgType(iarg);
    if ( pszTypeName && *pszTypeName )
    {
      int nLine  = pfuncDef->getArgTypeLine(iarg);
      int nColumn= pfuncDef->getArgTypeColumn(iarg);

      nIndex = FindSymIndexOrAdd( pszTypeName, nLine, nColumn, 1 );
      pfuncDef->setArgTypeIndx(iarg, nIndex);
    }
  }
  return nIndex;
}

/* -------------------------------- */
int IF::getArgNameIndex(int iarg )
{
  IF_FUNCDEF*  pfuncDef = getFuncDef();
  int nIndex =pfuncDef->getArgNameIndx(iarg);
  if ( nIndex == 0 )
  {
    char* pszName = pfuncDef->getArgName(iarg);
    if ( pszName && *pszName )
    {
      int nLine  = pfuncDef->getArgNameLine(iarg);
      int nColumn= pfuncDef->getArgNameColumn(iarg);

      nIndex = FindSymIndexOrAdd( pszName, nLine, nColumn, 1 );
      pfuncDef->setArgNameIndx(iarg, nIndex);
    }
  }
  return nIndex;
}

//-----------------------
int IF::FindSymIndexOrAdd( const char* name, int nLine, int nColumn, int bPrintFlag )
{
  int nCurIndex = 0;
  if ( name && *name )
  {
    char pszName[1000];
    IF_SYMTAB::getExternalType( name, pszName );

    int len = strlen( pszName );
    char* buff;
    int nRelIndex = 0;
    if ( *(pszName+len-1)== '*'  && (buff=new char[len+1]) )
    {
      strcpy( buff, pszName);
      *(buff+len-1) = 0;
      nRelIndex = FindSymIndexOrAdd( buff, nLine, nColumn, bPrintFlag );
      delete [] buff;
    }

    IF_SYM* pSym = getSymTab()->FindSymByName(pszName);
    if ( !pSym )
    {
      pSym= getSymTab()->AddNewSym( pszName );
      if ( pSym )
      {
        pSym->CreateNewSMT( nLine, nColumn );
	nCurIndex = getSymTab()->getFindedSymIndex();
	if ( bPrintFlag )
	{
	  m_pIFFile->printf( "SYM [%d] type \"%s\"\n", nCurIndex, pSym->getName() );
	  if ( nRelIndex )
	    m_pIFFile->printf( "REL [%d] \"type\" [%d] pointer\n", nRelIndex, nCurIndex ); 
	}
      }
    }
    else
    {
      pSym->CreateNewSMT( nLine, nColumn );
      nCurIndex = getSymTab()->getFindedSymIndex();
    }
  }

  return nCurIndex;
}


/* ======================================================== */
void IF::PrintFunctDef( const char* preffix )
{
//-------------------------- Function --------------------------
// SYM for return type
  int nRetTypeIndex = getArgTypeIndex(0);
  char* pszRetType  = getSymTab()->FindNameByIndex(nRetTypeIndex);
  char* pszFuncName = getFuncDef()->getArgName(0);

// SYM for function prototype
  char szFProto[1000];
  strcpy( szFProto, pszRetType );
  strcat( szFProto, "(" );

  int nTypeIndex;
  char* pszType;
  int i=0;
  while( (nTypeIndex = getArgTypeIndex(++i)) )
  {
    pszType = getSymTab()->FindNameByIndex(nTypeIndex);
    strcat( szFProto, pszType );
    strcat( szFProto, "," );
  }
  szFProto[ strlen(szFProto) -1 ] = ')';
  int nFuncProto= FindSymIndexOrAdd(szFProto, 0, 0, 1);

// REL for arguments
  i=0;
  while( (nTypeIndex = getArgTypeIndex(++i)) )
  {
    getIFFile()->printf( "REL [%d] \"type\" [%d] argument(%d)\n", nTypeIndex, nFuncProto, i );
  }

// SYM for function name
  int nFName= getArgNameIndex(0);
  IF_SYM* pSym = getSymTab()->FindSymByIndex(nFName);
  char* pszFName = pSym->getName();
  sprintf( getIFFile()->getBuf(), "SYM [%d] function \"%s%s\" %d/%d\n", nFName, 
	   preffix, pszFName, 
	   getFuncDef()->getArgNameLine(0), 
	   getFuncDef()->getArgNameColumn(0) );
  getIFFile()->printf();

// SMT for function name
  char* p = strchr( szFProto, '(' );
  getIFFile()->printf( "ATR [%d] c_proto(\"%s\")\n", nFName, p );

  getIFFile()->printf( "SMT  [%d] %d/%d %d\n", nFName, 
	   getFuncDef()->getArgNameLine(0), 
	   getFuncDef()->getArgNameColumn(0), strlen(pszFName));

  getIFFile()->printf( "REL [%d] \"type\" [%d]\n", nFuncProto, nFName );

//-------------------------- Method --------------------------
  char szMethodProto[1000];
  strcpy( szMethodProto, getInterfaceName() );
  strcat( szMethodProto, "::" );
  strcat( szMethodProto, pszFName );
  p = strchr( szFProto, '(' );
  strcat( szMethodProto, p );

// SYM for Method
  int nMethodProto= FindSymIndexOrAdd(szMethodProto, 0,0, 0);
  
  sprintf( getIFFile()->getBuf(), "SYM [%d] function \"%s\" %d/%d %d\n", nMethodProto, 
	   szMethodProto, 
	   getFuncDef()->getArgNameLine(0), 
	   getFuncDef()->getArgNameColumn(0),
	   strlen(pszFName) );
  getIFFile()->printf();

  getIFFile()->printf( "SMT [%d] %d/%d %d\n", nMethodProto,
	   getFuncDef()->getArgNameLine(0), 
	   getFuncDef()->getArgNameColumn(0),
	   strlen(pszFName) );
  getIFFile()->printf( "ATR [%d] virtual, abstract\n", nMethodProto );
  getIFFile()->printf( "REL [%d] \"context\" [%d] public\n", getInterfaceIndx(), nMethodProto );
}
/* ======================================================== */

//-----------------------
void IF::PrintSymSmt()
{
  if ( m_pSymTab )
  {
    IF_SYM*  pSym = m_pSymTab->getListSym();
  
    while ( pSym )
    {
      IF_SMT* pSmt = pSym->getListSmt();
      char* pszName= pSym->getName();
      int nLength = pSym->getLength();
      int nSymIndx = pSym->getIndex();
      int bKeyWord = IsKeyWord( pszName );

      while ( nLength && pSmt )
      {
	int nLine = pSmt->getLine();
	int nColumn= pSmt->getColumn();
	if ( nLine >0 && nColumn >0 )
	{
	  m_pIFFile->printf( "SMT [%d] %d/%d %d\n", nSymIndx, nLine, nColumn, nLength );
	  if ( bKeyWord )
	    m_pIFFile->printf( "SMT keyword \"%s\" %d/%d %d\n", pszName, nLine, nColumn, nLength );
	}
	pSmt= pSmt->getNext();
      }
      pSym= pSym->getNext();
    }
  }
}







