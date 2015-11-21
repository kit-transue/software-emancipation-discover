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
#include "shelltextitem.h"
#include vdrawHEADER




vkindTYPED_FULL_DEFINITION(shellTextItem, vtextitem, "shellTextItem");

vkindTYPED_FULL_DEFINITION(shellText, vtextitemText, "shellText");




shellHistory::shellHistory( int nSize )
{
  m_nSize = 0;
  m_nCurrent = -1;
  m_nLast = -1;
  m_fTmp = 0;
  if( nSize > 0 )
  {
    m_Strings = new vstr *[nSize];
    if( m_Strings )
      m_nSize = nSize;
    for( int i=0; i<m_nSize; i++ )
      m_Strings[i] = NULL;
  }
}

shellHistory::~shellHistory()
{
  if( m_Strings )
  {
    for( int i=0; i<m_nSize; i++ )
      if( m_Strings[i] )
        vstrDestroy( m_Strings[i] );
    delete m_Strings;
  }
}

int shellHistory::Push( const vchar *pszCmd )
{
  int nRet = 0;
  if( m_Strings && m_nSize > 0)
  {
    int nIndex = 0;
    if( !m_fTmp )
      nIndex = m_nLast + 1;
    else
    {
      nIndex = m_nLast;
      m_fTmp = 0;
    }
    if( nIndex >= m_nSize )
    {
      vstrDestroy( m_Strings[0] );
      nIndex = m_nSize - 1;
      for( int i = 0; i<nIndex; i++ )
        m_Strings[i] = m_Strings[i+1];
      m_Strings[nIndex] = vstrClone( pszCmd );
    }
    else
    {
      if( m_Strings[nIndex] )
        m_Strings[nIndex] = vstrCopy( pszCmd, m_Strings[nIndex] );
      else
        m_Strings[nIndex] = vstrClone( pszCmd );
    }
      
    if( m_Strings[nIndex] )
    {
      nRet = 1;
      m_nLast = nIndex;
      m_nCurrent = nIndex;
    }
  }
  return nRet;
}

int shellHistory::Push_Tmp( const vchar *pszCmd )
{
  int nRet = Push( pszCmd );
  m_fTmp = 1;
  return nRet;
}

const vstr *shellHistory::PopPrev()
{
  const vstr *ret = NULL;
  if( m_Strings && m_nSize > 0 && m_nCurrent > 0)
    ret = m_Strings[ --m_nCurrent ];
  return ret;
}

const vstr *shellHistory::PopNext()
{
  const vstr *ret = NULL;
  if( m_Strings && m_nSize > 0 && m_nCurrent < m_nLast )
    ret = m_Strings[ ++m_nCurrent ];
  return ret;
}

int shellHistory::SetSize(int new_size)
{
  int nRet = 0;
  if( new_size != m_nSize )
  {
    if( new_size > 0 )
    {
      int nCurrent = -1;
      int nLast = -1;
      int nSize = 0;
      vstr **new_array = new vstr *[new_size];
      if( new_array )
      {
        nRet = 1;
        nSize = new_size;
        if( m_Strings && m_nSize > 0)
        {
          for( int i=0; i<new_size; i++ )
            new_array[i] = NULL;
          int nStart = m_nLast + 1 - new_size;
          if( nStart < 0 )
            nStart = 0;
          for( i=0; i<new_size && (nStart + i) <= m_nLast; i++ )
            new_array[i] = m_Strings[nStart + i];
          nCurrent = i-1;
          nLast = nCurrent;
          for( i=0; i < nStart; i++ )
            vstrDestroy( m_Strings[i] );
        }
      }
      m_Strings = new_array;
      m_nSize = nSize;
      m_nLast = nLast;
      m_nCurrent = nCurrent;
    }
  }
  else
    nRet = 1;

  return nRet;
}

vbool shellText::DetermineWrapBreak(vchar *, long)
{
  //returning 0 in any case lets GALAXY break a word
  //for line wrap anywhere (as expected for a shell program)
  return 0;
}

void shellText::LoadInit( vresource res )
{
  vtextitemText::LoadInit( res );
  m_nMinLineNum = 1024;
  m_nCaret = 1;
  m_nStartPos = 0;
  m_LocalClipboard = vstrClone( (const vchar *)"" );
  m_Selection = CreateSelection();
}

void shellText::Destroy()
{
  if( m_LocalClipboard )
    vstrDestroy( m_LocalClipboard );
  vtextitemText::Destroy();
  if( m_Selection )
    DestroySelection( m_Selection );
}


void shellText::InsertText( vtextSelection *, const vchar *text, long len)
{
  SelectRange( m_Selection, m_nCaret - 1, m_nCaret - 1 );
  m_nCaret += len;
  if( m_pOwner )
    m_pOwner->SetIgnoreHilite(1);
  vtextitemText::InsertText( m_Selection, text, len );
  if( m_pOwner )
    m_pOwner->SetIgnoreHilite(0);
  CheckLines();
  ScrollToCaret();
  if( m_pOwner )
    m_pOwner->InvalView();
}

void shellText::InsertPrompt(const vchar *pszPrompt )
{
  int nLen = GetLength();
  if( m_pOwner )
    m_pOwner->SetIgnoreHilite(1);
  SelectRange( m_Selection, nLen-1, nLen-1 );
  int nTextLen = strlen((const char *)pszPrompt);
  m_nCaret = nLen + nTextLen;
  vtextitemText::InsertText( m_Selection, pszPrompt, nTextLen );
  m_nStartPos = nLen + nTextLen - 1;
  if( m_pOwner )
    m_pOwner->SetIgnoreHilite(0);
  CheckLines();
  ScrollToCaret();
  if( m_pOwner )
    m_pOwner->InvalView();
}

void shellText::ShowCaret( int fFirstTime )
{
  if( fFirstTime )
  {
    m_nCaret = 1;
    SelectRange( m_Selection, 0, 0 );
    vtextitemText::InsertText( m_Selection, (const vchar *)" ", 1 );
  }
  if( m_pOwner )
    m_pOwner->InvalView();
}

int shellText::MoveLeft()
{
  int nRet = 0;
  int nNew = m_nCaret - 1;
  if( nNew > m_nStartPos )
  {
    MoveCaret( nNew );
    nRet = 1;
  }
  return nRet;
}

int shellText::MoveRight()
{
  int nRet = 0;
  int nNew = m_nCaret + 1;
  if( nNew <= GetLength() )
  {
    MoveCaret( nNew );
    nRet = 1;
  }
  return nRet;
}

int shellText::MoveEnd()
{
  int nRet = 0;
  int nNew = GetLength();
  if( m_nCaret < nNew )
  {
    MoveCaret( nNew );
    nRet = 1;
  }
  return nRet;
}

int shellText::MoveHome()
{
  int nRet = 0;
  int nNew = m_nStartPos + 1;
  if( m_nCaret > nNew )
  {
    MoveCaret( nNew );
    nRet = 1;
  }
  return nRet;
}

int shellText::DoDelete()
{
  int nRet = MoveRight();
  if( nRet )
    DoBackspace();
  return nRet;
}

int shellText::DoBackspace()
{
  int nRet = 0;

  if( m_nCaret > (m_nStartPos + 1) )
  {
    nRet = 1;
    SelectRange( m_Selection, m_nCaret - 2, m_nCaret - 1);
    if( m_pOwner )
      m_pOwner->SetIgnoreHilite(1);
    m_nCaret--;
    DeleteText( m_Selection );
    if( m_pOwner )
      m_pOwner->SetIgnoreHilite(0);
    if( m_pOwner )
      m_pOwner->InvalView();
  }
  return nRet;
}

int shellText::CutBegLocal()
{
  int nRet = 0;
  int nLen = m_nCaret - (m_nStartPos + 1);
  if( nLen > 0 )
  {
    vchar *buff = new vchar[nLen+1];
    if( buff )
    {
      buff[nLen] = '\0';
      vchar *text = GetTextAt( m_nStartPos );
      if( text )
      {
        nRet = 1;
        strncpy( (char *)buff, (const char *)text, nLen );
        m_LocalClipboard = vstrCopy( buff, m_LocalClipboard );
        SelectRange( m_Selection, m_nStartPos, m_nCaret - 1);
        if( m_pOwner )
          m_pOwner->SetIgnoreHilite(1);
        DeleteText( m_Selection );
        if( m_pOwner )
          m_pOwner->SetIgnoreHilite(0);
        m_nCaret-=nLen;
        if( m_pOwner )
          m_pOwner->InvalView();
      }
      delete buff;
    }
    
  }
  return nRet;
}

int shellText::CutEndLocal()
{
  int nRet = 0;
  int nLen = GetLength() - m_nCaret;
  if( nLen > 0 )
  {
    vchar *buff = new vchar[nLen+1];
    if( buff )
    {
      buff[nLen] = '\0';
      vchar *text = GetTextAt( m_nCaret - 1 );
      if( text )
      {
        nRet = 1;
        strncpy( (char *)buff, (const char *)text, nLen );
        m_LocalClipboard = vstrCopy( buff, m_LocalClipboard );
        SelectRange( m_Selection, m_nCaret-1, GetLength() - 1 );
        if( m_pOwner )
          m_pOwner->SetIgnoreHilite(1);
        DeleteText( m_Selection );
        if( m_pOwner )
          m_pOwner->SetIgnoreHilite(0);
        if( m_pOwner )
          m_pOwner->InvalView();
      }
      delete buff;
    }
    
  }
  return nRet;
}

int shellText::PasteLocal()
{
  int nRet = 0;
  int nLen = 0;

  if( m_LocalClipboard && (nLen=vstrGetSize( m_LocalClipboard )) > 0 )
  {
    InsertText( NULL, m_LocalClipboard, nLen );
    nRet = 1;
  }
  return nRet;
}

void shellText::Add( const vchar *text )
{
  SelectRange( m_Selection, GetLength() - 1, GetLength() - 1 );
  if( m_pOwner )
    m_pOwner->SetIgnoreHilite(1);
  vtextitemText::InsertText( m_Selection, text, 
                          strlen( (const char *)text ) );
  MoveCaret( GetLength() );
  CheckLines();
  if( m_pOwner )
    m_pOwner->SetIgnoreHilite(0);
  ScrollToCaret();  
}

void shellText::ReplaceUserInput( const vchar *text )
{
  SelectRange( m_Selection, m_nStartPos, GetLength() - 1 );
  if( m_pOwner )
    m_pOwner->SetIgnoreHilite(1);
  vtextitemText::InsertText( m_Selection, text, 
                          strlen( (const char *)text ) );
  m_nCaret = GetLength();
  MoveCaret( m_nCaret );
  CheckLines();
  if( m_pOwner )
    m_pOwner->SetIgnoreHilite(0);
  ScrollToCaret();
  if( m_pOwner )
    m_pOwner->InvalView();

}

void shellText::CheckLines()
{
  int diff = GetLineCount() - m_nMinLineNum;
  if( diff > 1 )
  {
    SelectLines( m_Selection, 1, diff, vtextLINE_ALL );
    if( m_nStartPos > GetSelectionEnd( m_Selection ) )
    {
      m_nCaret -= GetSelectionLength( m_Selection );
      m_nStartPos -= GetSelectionLength( m_Selection );
      DeleteText( m_Selection );
    }
  }
}

void shellText::ScrollToCaret()
{
  if( m_pOwner )
    m_pOwner->ScrollIfNecessary( GetLineFromPosition( m_nCaret ) );
}

void shellText::MoveCaret( int nNewPos )
{
  m_nCaret = nNewPos;
  if( m_pOwner )
    m_pOwner->InvalView();
}

int shellText::IsEscape()
{
  int nRet = 0;
  int nLen = GetLength();
  if( nLen > 1 && *(GetTextAt( nLen - 3 )) == '\\' )
    nRet = 1;
  return nRet;
}

int shellText::GetUserInput( vchar * &text)
{
  int nRet = GetLength() - m_nStartPos - 1;
  text = GetTextAt( m_nStartPos );
  return nRet;
}

void shellTextItem::LoadInit( vresource res )
{
  vtextitem::LoadInit( res );
  SetOffscreenDrawing( 1 );
  m_fIgnoreHilite = 0;
  m_fIsBlocked = 0;
  m_ProcessFunc = NULL;
  m_fUsedHistory = 0;
  m_nShellId = 0;
  m_Prompt = vstrClone( (const vchar *)"% " );
  shellText *t_data = shellText::CastDown( GetTextData() );
  if( t_data )
  {
    t_data->SetTextOwner(this);
    t_data->ShowCaret (1);
    t_data->InsertPrompt( m_Prompt );
  }
}

void shellTextItem::Destroy()
{
  if( m_Prompt )
    vstrDestroy( m_Prompt );
  vtextitem::Destroy();  
}

void shellTextItem::DrawCaret( vrect *t_rect ) {}

void shellTextItem::DrawHilite( vrect *t_rect, int style )
{
  if ( !m_fIgnoreHilite )
    vtextitem::DrawHilite( t_rect, style );
}

void shellTextItem::Focus()
{
  vtextitem::Focus();
  shellText *t_data = shellText::CastDown( GetTextData() );
  if( t_data )
    t_data->ShowCaret ();
}

void shellTextItem::Unfocus()
{
  vtextitem::Unfocus();
  shellText *t_data = shellText::CastDown( GetTextData() );
  if( t_data )
    t_data->ShowCaret ();
}

void shellTextItem::SetMinLineNum(int nLines)
{
  shellText *t_data = shellText::CastDown( GetTextData() );
  if( t_data )
    t_data->SetMinLineNum (nLines);
}

void shellTextItem::BlockInput()
{
  m_fIsBlocked = 1;
  SetModifyable(0);
}

void shellTextItem::UnblockInput()
{
  m_fIsBlocked = 0;
  SetModifyable(1);
}

void shellTextItem::Respond( const vchar *text )
{
  shellText *t_data = shellText::CastDown( GetTextData() );
  if( t_data )
  {
    t_data->InsertPrompt( text );
    //t_data->InsertPrompt( m_Prompt );
    m_fUsedHistory = 0;
  }
}

void shellTextItem::ScrollIfNecessary( int nLine )
{
  long lTop  = GetTopLineNum();
  if( nLine < lTop || nLine > (lTop + Lines() - 2) )
    Scroll( nLine, 0 );
}

int shellTextItem::HandleKeyDown(vevent *event)
{
  shellText *t_data = shellText::CastDown( GetTextData() );
  if( !m_fIsBlocked && t_data )
  {
    if( !ProcessAccelerators( event, event->GetKeyStroke(), t_data )  )
    {
      const vchar* binding = event->GetBinding();
      vchar key = event->GetKeyChar();
      if( !binding || !strcmp( (char *)binding, "Select" ) )
      {
        //sometimes binding for Return key is not set
        if( key == 13 )
        {
          t_data->Add( (const vchar *)"\n" );
          if( !t_data->IsEscape() )
          {
            Enter(t_data);
          }
        }
        else if( isprint(key) && key != '\t' )
        {
          t_data->InsertText( NULL, &key, 1 );
        }
      }
      else
      {
        if( !strcmp( (char *)binding, "Confirm" ) )
        {
          t_data->Add( (const vchar *)"\n" );
          if( !t_data->IsEscape() )
          {
            Enter(t_data);
          }
        }
        else if ( !strcmp( (char *)binding, "Left" ) )
        {
          if( !t_data->MoveLeft() )
            vwindow::Beep();
        }
        else if ( !strcmp( (char *)binding, "Right" ) )
        {
          if( !t_data->MoveRight() )
            vwindow::Beep();
        }
        else if ( !strcmp( (char *)binding, "Delete" ) )
        {
          if( key==8)
          {
            if( !t_data->DoBackspace() )
              vwindow::Beep();
          }
          else
          {
            if( !t_data->DoDelete() )
              vwindow::Beep();
          }
        }
        else if ( !strcmp( (char *)binding, "Up" ) )
        {
          HistoryPrev(t_data);
        }
        else if ( !strcmp( (char *)binding, "Down" ) )
        {
          HistoryNext(t_data);
        }
      }
    }
  }
  else
    vwindow::Beep();
  return 1;
}

int shellTextItem::ProcessAccelerators(vevent *event, vkeyStroke ks, shellText *t_data)
{
  int nRet = 0;
  vkey key = vkeyGetStrokeKey( ks );
  vkeyModifiers mod = vkeyGetStrokeModifiers( ks );
  if( mod & vkeyMODIFIER_KEY_COMMAND )
  {
    nRet = 1;
    switch( key )
    {
    case 'u':
      t_data->ReplaceUserInput((const vchar *)"");
      break;
    case 'k':
      if( !t_data->CutEndLocal() )
        vwindow::Beep();
      break;
    case 'w':
      if( !t_data->CutBegLocal() )
        vwindow::Beep();
      break;
    case 'y':
      if( !t_data->PasteLocal() )
        vwindow::Beep();
      break;
    case 'h':
      if( !t_data->DoBackspace() )
        vwindow::Beep();
    case 'j':case 'm':
      t_data->Add( (const vchar *)"\n" );
      if( !t_data->IsEscape() )
        Enter( t_data );
      break;
    case 'a':
      if( !t_data->MoveHome() )
        vwindow::Beep();
      break;
    case 'l'://cls
      break;
    case 'e':
      if( !t_data->MoveEnd() )
        vwindow::Beep();
      break;
    case 'n':
      HistoryNext( t_data );
      break;
    case 'p':
      HistoryPrev( t_data );
      break;
    case 'f':
      if( !t_data->MoveRight() )
        vwindow::Beep();
      break;
    case 'b':
      if( !t_data->MoveLeft() )
        vwindow::Beep();
      break;
    case 'c':case 'v':case vkeyINSERT:
      vtextitem::HandleKeyDown( event );
      break;
    default:
      nRet = 0;
    }
  }
  else if( (mod & vkeyMODIFIER_KEY_SHIFT) && key == vkeyINSERT )
    vtextitem::HandleKeyDown( event );
  return nRet;
}

void shellTextItem::HistoryNext(shellText *t_data)
{
  const vstr *prev = m_History.PopNext();
  if( prev )
    t_data->ReplaceUserInput( prev );
  else
    vwindow::Beep();
}

void shellTextItem::HistoryPrev(shellText *t_data)
{
  const vstr *prev = NULL;
  if (!m_fUsedHistory && m_History.GetStackSize() > 0)
  {
    vchar *text = NULL;
    int nEntered = t_data->GetUserInput( text );
    vchar *buff = new vchar[nEntered + 1];
    if( buff )
    {
      buff[0] = '\0';
      if( text )
        strncpy( (char *)buff, (const char *)text, nEntered );
      buff[nEntered] = '\0';
      m_History.Push_Tmp( buff );
      m_fUsedHistory = 1;
      delete buff;
    }
  }
  prev = m_History.PopPrev();
  if( prev )
    t_data->ReplaceUserInput( prev );
  else
    vwindow::Beep();

}

void shellTextItem::Enter(shellText *t_data)
{
  vchar *cmd;
  int nLen = t_data->GetUserInput( cmd );
  vchar *buff = new vchar [nLen + 1];
  if( buff )
  {
    strncpy( (char *)buff, (const char *)cmd, nLen );
    buff[nLen] = '\0';
    if( m_ProcessFunc )
      m_ProcessFunc( this, buff );
    if( buff[nLen -1] == '\n' )
      buff[nLen - 1] = '\0';
    nLen = strlen( (char *)buff );
    if( nLen > 0 )
      m_History.Push( buff );
    delete buff;
  }
}

void shellTextItem::DrawLine( vtextLineMetricRec *rec)
{
  vtextitem::DrawLine( rec );
  shellText *t_data = shellText::CastDown( GetTextData() );
  if( t_data && IsFocused() )
  {
    long lCaretPos = t_data->GetCaretPos();
    long lCurrLine = t_data->GetLineFromPosition( rec->position );
    if( t_data->GetLineFromPosition( lCaretPos ) == lCurrLine )
    {
      int x,y;
      vdraw::GetPoint(&x, &y);
      vfont *font = GetFont();
      if( font )
      {
        int nDiff = lCaretPos - t_data->GetLinePosition( lCurrLine );
        int dx, dy, dx1, dy1;
        y-=font->Descent();
        font->SizedStringWidth( rec->text, nDiff, &dx, &dy );
        font->SizedStringWidth( rec->text + nDiff, 1, &dx1, &dy1 );
        x = GetX() + rec->lineOrigin + dx + dy + GetLeftMargin() + GetDefaultBorderInset();
        m_CursorRect.Set(x, y, dx1+dy1, font->Descent()+font->Ascent());
        NewDrawCursor();
      }
    }
  }
}


void shellTextItem::NewDrawCursor()
{
  vcolorSpec spec;
  spec.FSetRGB( 0.75, 0, 0 );
  vcolor *color = vcolor::Intern( &spec );
  vcolor *old_color = vdraw::GetColor();
  vdraw::SetColor( color );
  vdrawRectsStroke(&m_CursorRect, 1);
  vdraw::SetColor( old_color );
}


int shellTextItem::HandleButtonUp( vevent *event )
{
  return vtextitem::HandleButtonUp( event );
}

void shellText::ObserveSelect(vtextSelection *sel, int before)
{
  vtextitemText::ObserveSelect(sel, before);
#if 0
  if( before==vtextSELECTION_HAS_CHANGED && m_pOwner )
    m_pOwner->InvalView();
#endif
}

void shellTextItem::ScrollToStart(int){}
