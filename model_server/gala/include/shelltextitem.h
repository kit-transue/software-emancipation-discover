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
#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef vtextitemINCLUDED
#include vtextitemHEADER
#endif

#ifndef vstrINCLUDED
#include vstrHEADER
#endif

class shellTextItem;

typedef void (*PROCESSREQFUNC)( shellTextItem *, const vchar * );

class shellHistory
{
public:
  shellHistory(int nSize = 20);
  ~shellHistory();
  int SetSize(int new_size);
  int GetSize() {return m_nSize;}
  int GetStackSize() {return m_nLast+1;}
  int Push( const vchar *cmd );
  int Push_Tmp( const vchar *cmd );
  const vstr *PopPrev();
  const vstr *PopNext();
protected:
  vstr **m_Strings;
  int m_nSize;
  int m_nCurrent;
  int m_nLast;
  int m_fTmp;

};

class shellText;

class shellTextItem : public vtextitem
{
public:
    vkindTYPED_FULL_DECLARATION(shellTextItem);
    vloadableINLINE_CONSTRUCTORS(shellTextItem, vtextitem);

    virtual int HandleKeyDown(vevent *event);
    virtual void LoadInit( vresource res );
    virtual void Destroy();
    virtual void DrawCaret( vrect * );
    virtual void DrawHilite( vrect *, int );
    virtual void Focus();
    virtual void Unfocus();
    virtual void DrawLine( vtextLineMetricRec *);
    virtual int HandleButtonUp( vevent *event );
    virtual void ScrollToStart( int );

    int ProcessAccelerators(vevent *event, vkeyStroke ks, shellText *);
    void SetMinLineNum(int);
    void SetHistorySize(int len){ m_History.SetSize( len ); }
    void SetPrompt(const vchar *new_prompt) {m_Prompt = vstrCopy( new_prompt, m_Prompt );}
    void BlockInput();
    void UnblockInput();
    int IsBlocked() {return m_fIsBlocked;}
    void SetProcessFunc( PROCESSREQFUNC func ){ m_ProcessFunc = func; }
    void Respond( const vchar *text );
    void ScrollIfNecessary( int nLine );
    void SetIgnoreHilite( int is_ignore ){ m_fIgnoreHilite = is_ignore; }
    int GetClientId() {return m_nShellId;}
    void SetClientId( int id ) {m_nShellId = id;}
    void NewDrawCursor();

protected:
    
    void HistoryPrev(shellText *data);
    void HistoryNext(shellText *data);
    void Enter(shellText *data);

    int m_fIgnoreHilite;
    int m_fIsBlocked;

    PROCESSREQFUNC m_ProcessFunc;

    vstr *m_Prompt;
    shellHistory m_History;
    int m_fUsedHistory;
    int m_nShellId;
    vrect m_CursorRect;
};

class shellText: public vtextitemText
{
public:
  vkindTYPED_FULL_DECLARATION(shellText);
  vloadableINLINE_CONSTRUCTORS(shellText, vtextitemText);

  virtual vbool DetermineWrapBreak(vchar *, long);
  virtual void InsertText( vtextSelection *, const vchar *, long );
  virtual void Destroy();
  virtual void ObserveSelect(vtextSelection *, int); 
  
  void InsertPrompt(const vchar * );
  void ShowCaret( int fFirstTime = 0 );
  void SetTextOwner( shellTextItem *owner ){ m_pOwner = owner; }
  void SetMinLineNum( int num ){ m_nMinLineNum = num; }
  int IsEscape();
  void Add(const vchar *);
  int GetCaretPos() {return m_nCaret-1;}

  int MoveLeft();
  int MoveRight();
  int MoveEnd();
  int MoveHome();
  int DoDelete();
  int DoBackspace();
  
  int CutBegLocal();
  int CutEndLocal();
  int PasteLocal();

  void ReplaceUserInput( const vchar * );

  int GetUserInput( vchar * &);

  void ScrollToCaret();
protected:
  virtual void LoadInit( vresource );
  
  void MoveCaret( int nNewPos );
  void CheckLines();
  shellTextItem *m_pOwner;
  vstr *m_LocalClipboard;
  vtextSelection *m_Selection;
  int m_nMinLineNum;
  int m_nCaret;
  int m_nStartPos;
};
